
//===----------------------------------------------------------------------===//
// ForallReductionTransform
//
// One-shot Option-B rewrite. Detects reduction-eligible incAndAssigns nested
// inside an outermost starplat forall, and rewrites the entire subtree into
// scf.parallel + scf.for + scf.if + arith.addi + scf.reduce, followed by a
// post-loop store back to the counter pointer.
//
// Ops not part of a recognized reduction pattern are left alone; the existing
// conversion patterns handle them (atomic / sequential fallback).
//===----------------------------------------------------------------------===//

namespace
{

//===--- Analysis helpers -------------------------------------------------===//

static Region* forallRegion(Operation* op) {
    if (auto f = dyn_cast<starplat::ForAllNodesOp>(op))
        return &f.getRegion();
    if (auto f = dyn_cast<starplat::ForAllNeighboursOp>(op))
        return &f.getRegion();
    return nullptr;
}

static bool isStarplatForall(Operation* op) { return isa_and_nonnull<starplat::ForAllNodesOp, starplat::ForAllNeighboursOp>(op); }

static Operation* getOutermostStarplatForall(Operation* op) {
    Operation* cur   = op->getParentOp();
    Operation* outer = nullptr;
    while (cur) {
        if (isStarplatForall(cur))
            outer = cur;
        cur = cur->getParentOp();
    }
    return outer;
}

static bool isDefinedOutside(Value v, Region& r) {
    if (auto ba = dyn_cast<BlockArgument>(v))
        return !r.isAncestor(ba.getOwner()->getParent());
    Operation* d = v.getDefiningOp();
    return d && !r.isAncestor(d->getParentRegion());
}

/// An incAndAssign is "reduction-eligible" relative to an anchor forall if:
///   - its counter ptr is defined outside the anchor;
///   - every op on the path from the inc up to the anchor is either a
///     starplat forall or a starplat.spif (no other region-bearing ops).
static bool isReductionEligible(starplat::IncAndAssignOp inc, Operation* anchor) {
    Region* anchorRegion = forallRegion(anchor);
    if (!anchorRegion)
        return false;
    if (!isDefinedOutside(inc.getOperand(0), *anchorRegion))
        return false;
    Operation* cur = inc->getParentOp();
    while (cur && cur != anchor) {
        if (!isa<starplat::ForAllNodesOp, starplat::ForAllNeighboursOp, starplat::StarPlatIfOp>(cur))
            return false;
        cur = cur->getParentOp();
    }
    return cur == anchor;
}

//===--- The recursive builder --------------------------------------------===//

/// Recursively walks a starplat body region's ops in order. For each op:
///   - If it's the target incAndAssign, produces its increment as the
///     contribution and does NOT clone the op.
///   - If it's a starplat.spif on the path, builds an scf.if yielding i64;
///     recurses into the then-region; else-region yields 0.
///   - If it's a nested starplat forall on the path, builds scf.for with an
///     i64 iter_arg accumulator; recurses.
///   - Otherwise clones the op verbatim (type-converting ops will rewrite it
///     later via the conversion patterns).
///
/// Returns the total contribution value of the block (i64).
class ReductionBuilder
{
  public:
    ReductionBuilder(OpBuilder& b, Location loc, Type elemTy, starplat::IncAndAssignOp targetInc, IRMapping& mapping)
        : b(b), loc(loc), elemTy(elemTy), targetInc(targetInc), mapping(mapping) {
        zero = arith::ConstantOp::create(b, loc, elemTy, b.getIntegerAttr(elemTy, 0));
    }

    /// Build contributions for all ops in `srcBlock` (in order), cloning
    /// non-reduction-relevant ops and threading reduction contributions.
    /// Returns the accumulated i64 contribution for this block.
    Value buildBlock(Block& srcBlock, Value acc) {
        for (Operation& op : llvm::make_early_inc_range(srcBlock)) {
            if (isa<starplat::endOp>(op))
                continue;
            acc = buildOp(&op, acc);
        }
        return acc;
    }

  private:
    Value buildOp(Operation* op, Value acc) {
        // --- Target incAndAssign: contribute its increment value. ---
        if (op == targetInc.getOperation()) {
            Value inc       = targetInc.getOperand(1);
            Value incMapped = mapping.lookupOrDefault(inc);
            // If the inc value's defining op is inside the original forall
            // region and hasn't been cloned yet, we need to clone it. For
            // simplicity we handle the common case: the inc is either a
            // block argument, defined outside, or a constant we've cloned.
            Value contrib = incMapped;
            if (contrib.getType() != elemTy) {
                // Best-effort cast; extend as needed.
                contrib = arith::ExtSIOp::create(b, loc, elemTy, contrib);
            }
            return arith::AddIOp::create(b, loc, acc, contrib);
        }

        // --- starplat.spif on the reduction path: scf.if returning i64. ---
        if (auto spif = dyn_cast<starplat::StarPlatIfOp>(op)) {
            if (pathContainsTarget(spif)) {
                Value cond = mapping.lookupOrDefault(spif.getCondition());
                auto ifOp  = scf::IfOp::create(b, loc, TypeRange{elemTy}, cond,
                                               /*withElseRegion=*/true);

                // Then region.
                {
                    OpBuilder::InsertionGuard g(b);
                    Block& thenBlock = ifOp.getThenRegion().front();
                    b.setInsertionPointToStart(&thenBlock);
                    ReductionBuilder inner(b, loc, elemTy, targetInc, mapping);
                    Value innerAcc = inner.zero;
                    Block& srcThen = spif.getBody().front(); // or spif.getRegion(0).front()
                    innerAcc       = inner.buildBlock(srcThen, innerAcc);
                    scf::YieldOp::create(b, loc, ValueRange{innerAcc});
                }

                // Else region.
                {
                    OpBuilder::InsertionGuard g(b);
                    Block& elseBlock = ifOp.getElseRegion().front();
                    b.setInsertionPointToStart(&elseBlock);
                    scf::YieldOp::create(b, loc, ValueRange{zero});
                }

                return arith::AddIOp::create(b, loc, acc, ifOp.getResult(0));
            }
            // spif not on path → clone verbatim.
            cloneVerbatim(op);
            return acc;
        }

        // --- Nested starplat forall on the reduction path: scf.for with iter_arg. ---
        if (isStarplatForall(op) && pathContainsTarget(op)) {
            return buildNestedForall(op, acc);
        }

        // --- Anything else: clone verbatim. ---
        cloneVerbatim(op);
        return acc;
    }

    Value buildNestedForall(Operation* forallOp, Value acc) {
        auto* ctx     = b.getContext();
        auto i32Ty    = b.getI32Type();
        auto idxTy    = b.getIndexType();
        auto ptrTy    = LLVM::LLVMPointerType::get(ctx);
        Value oneI32  = LLVM::ConstantOp::create(b, loc, i32Ty, b.getI32IntegerAttr(1));
        Value zeroIdx = arith::ConstantIndexOp::create(b, loc, 0);
        Value oneIdx  = arith::ConstantIndexOp::create(b, loc, 1);

        // Resolve the trip count depending on forall kind.
        ModuleOp module = forallOp->getParentOfType<ModuleOp>();
        Value nIdx;
        if (auto fn = dyn_cast<starplat::ForAllNodesOp>(forallOp)) {
            FailureOr<LLVM::LLVMFuncOp> fnDecl = LLVM::lookupOrCreateFn(b, module, "get_num_nodes", {ptrTy}, i32Ty);
            Value graph                        = mapping.lookupOrDefault(fn.getOperand());
            Value nI32                         = LLVM::CallOp::create(b, loc, *fnDecl, ValueRange{graph}).getResult();
            nIdx                               = arith::IndexCastOp::create(b, loc, idxTy, nI32);
        }
        else {
            auto fn2                            = cast<starplat::ForAllNeighboursOp>(forallOp);
            FailureOr<LLVM::LLVMFuncOp> fn2Decl = LLVM::lookupOrCreateFn(b, module, "get_num_neighbours", {ptrTy, i32Ty}, i32Ty);
            Value graph                         = mapping.lookupOrDefault(fn2.getOperand(0));
            Value uPtr                          = mapping.lookupOrDefault(fn2.getOperand(1));
            Value uI32                          = LLVM::LoadOp::create(b, loc, i32Ty, uPtr);
            Value nI32                          = LLVM::CallOp::create(b, loc, *fn2Decl, ValueRange{graph, uI32}).getResult();
            nIdx                                = arith::IndexCastOp::create(b, loc, idxTy, nI32);
            // Also need neighbours pointer for per-iter materialization.
            // Stash it for the body builder via the closure below.
            FailureOr<LLVM::LLVMFuncOp> nbrsDecl = LLVM::lookupOrCreateFn(b, module, "get_neighbours", {ptrTy, i32Ty}, ptrTy);
            Value nbrsPtr                        = LLVM::CallOp::create(b, loc, *nbrsDecl, ValueRange{graph, uI32}).getResult();
            pendingNbrsPtr                       = nbrsPtr;
        }

        bool isNodes          = isa<starplat::ForAllNodesOp>(forallOp);
        Value capturedNbrsPtr = pendingNbrsPtr;
        pendingNbrsPtr        = Value{};

        auto forOp            = scf::ForOp::create(b, loc, zeroIdx, nIdx, oneIdx, ValueRange{zero},
                                                   [&](OpBuilder& fb, Location fl, Value iv, ValueRange iters)
                                                   {
                                            // Materialize the iteration variable as !llvm.ptr to i32.
                                            Value ivI32 = arith::IndexCastOp::create(fb, fl, i32Ty, iv);
                                            Value slot;
                                            if (isNodes) {
                                                slot = LLVM::AllocaOp::create(fb, fl, ptrTy, i32Ty, oneI32, 0);
                                                LLVM::StoreOp::create(fb, fl, ivI32, slot);
                                            }
                                            else {
                                                // neighbours[iv]
                                                Value ivI64 = arith::IndexCastOp::create(fb, fl, fb.getI64Type(), iv);
                                                Value addr  = LLVM::GEPOp::create(fb, fl, ptrTy, i32Ty, capturedNbrsPtr,
                                                                                             ArrayRef<LLVM::GEPArg>{LLVM::GEPArg(ivI64)});
                                                Value vI32  = LLVM::LoadOp::create(fb, fl, i32Ty, addr);
                                                slot        = LLVM::AllocaOp::create(fb, fl, ptrTy, i32Ty, oneI32, 0);
                                                LLVM::StoreOp::create(fb, fl, vI32, slot);
                                            }

                                            // Bind the source forall's block argument to this slot in the mapping.
                                            Block& srcBlock = forallRegion(forallOp)->front();
                                            mapping.map(srcBlock.getArgument(0), slot);

                                            // Recurse into the body.
                                            ReductionBuilder inner(fb, fl, elemTy, targetInc, mapping);
                                            Value innerAcc = iters[0];
                                            innerAcc       = inner.buildBlock(srcBlock, innerAcc);

                                            scf::YieldOp::create(fb, fl, ValueRange{innerAcc});
                                        });

        return arith::AddIOp::create(b, loc, acc, forOp.getResult(0));
    }

    /// Clone `op` into the current builder with operand remapping. Also maps
    /// its results. Recursively handles regions (conservatively clones them
    /// verbatim, which is fine for ops with no reduction paths through them).
    void cloneVerbatim(Operation* op) {
        Operation* cloned = b.clone(*op, mapping);
        for (auto [oldR, newR] : llvm::zip(op->getResults(), cloned->getResults()))
            mapping.map(oldR, newR);
    }

    /// Does the (region-bearing) op transitively contain the target inc?
    bool pathContainsTarget(Operation* op) {
        bool found = false;
        op->walk(
            [&](starplat::IncAndAssignOp cand)
            {
                if (cand.getOperation() == targetInc.getOperation()) {
                    found = true;
                    return WalkResult::interrupt();
                }
                return WalkResult::advance();
            });
        return found;
    }

    OpBuilder& b;
    Location loc;
    Type elemTy;
    starplat::IncAndAssignOp targetInc;
    IRMapping& mapping;
    Value zero;
    Value pendingNbrsPtr; // closure-passed from buildNestedForall outer scope
};

//===--- The pass ---------------------------------------------------------===//

struct ForallReductionTransform : public PassWrapper<ForallReductionTransform, OperationPass<ModuleOp>>
{

    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ForallReductionTransform)

    StringRef getArgument() const final { return "starplat-forall-reduction-transform"; }
    StringRef getDescription() const final {
        return "Rewrite reduction-eligible starplat forall subtrees into "
               "scf.parallel + scf.for + scf.reduce (Option B).";
    }

    void getDependentDialects(DialectRegistry& registry) const override {
        registry.insert<arith::ArithDialect, scf::SCFDialect, memref::MemRefDialect, LLVM::LLVMDialect>();
    }

    void runOnOperation() override {
        ModuleOp module  = getOperation();
        MLIRContext* ctx = &getContext();

        // Collect (anchor, incAndAssign, counterPtr, elemTy) groups. One
        // reduction counter per anchor for now.
        struct Group
        {
            Operation* anchor;
            starplat::IncAndAssignOp inc;
            Value counterPtr;
            Type elemTy;
        };
        SmallVector<Group> groups;

        module.walk(
            [&](starplat::IncAndAssignOp inc)
            {
                Operation* anchor = getOutermostStarplatForall(inc);
                if (!anchor)
                    return;
                if (!isReductionEligible(inc, anchor))
                    return;

                // Skip if this anchor already has a registered reduction (one-counter limit).
                for (auto& g : groups)
                    if (g.anchor == anchor)
                        return;

                Value counterPtr = inc.getOperand(0);
                Value incVal     = inc.getOperand(1);
                Type elemTy;
                if (isa<LLVM::LLVMPointerType>(incVal.getType()))
                    elemTy = IntegerType::get(ctx, 64);
                else if (auto intTy = dyn_cast<IntegerType>(incVal.getType()))
                    elemTy = intTy;
                else
                    return;

                groups.push_back({anchor, inc, counterPtr, elemTy});
            });

        // Rewrite each group.
        for (auto& g : groups) {
            if (failed(rewriteGroup(g.anchor, g.inc, g.counterPtr, g.elemTy)))
                return signalPassFailure();
        }
    }

    LogicalResult rewriteGroup(Operation* anchorOp, starplat::IncAndAssignOp inc, Value counterPtr, Type elemTy) {
        // The anchor must be a ForAllNodesOp (top-level forall over nodes).
        // A top-level ForAllNeighboursOp is possible but rare; extend as needed.
        auto anchor = dyn_cast<starplat::ForAllNodesOp>(anchorOp);
        if (!anchor)
            return anchorOp->emitError("Option-B rewrite currently requires an outer ForAllNodesOp anchor");

        OpBuilder b(anchor);
        Location loc     = anchor.getLoc();
        MLIRContext* ctx = b.getContext();
        auto i32Ty       = b.getI32Type();
        auto idxTy       = b.getIndexType();
        auto ptrTy       = LLVM::LLVMPointerType::get(ctx);

        // Trip count for outer forall.
        ModuleOp module                         = anchor->getParentOfType<ModuleOp>();
        FailureOr<LLVM::LLVMFuncOp> getNumNodes = LLVM::lookupOrCreateFn(b, module, "get_num_nodes", {ptrTy}, i32Ty);
        if (failed(getNumNodes))
            return failure();

        Value graph    = anchor.getOperand();
        Value nI32     = LLVM::CallOp::create(b, loc, *getNumNodes, ValueRange{graph}).getResult();
        Value nIdx     = arith::IndexCastOp::create(b, loc, idxTy, nI32);
        Value zeroIdx  = arith::ConstantIndexOp::create(b, loc, 0);
        Value oneIdx   = arith::ConstantIndexOp::create(b, loc, 1);
        Value oneI32   = LLVM::ConstantOp::create(b, loc, i32Ty, b.getI32IntegerAttr(1));
        Value zeroElem = arith::ConstantOp::create(b, loc, elemTy, b.getIntegerAttr(elemTy, 0));

        auto parOp     = scf::ParallelOp::create(b, loc, ValueRange{zeroIdx}, ValueRange{nIdx}, ValueRange{oneIdx}, ValueRange{zeroElem},
                                                 [&](OpBuilder& pb, Location pl, ValueRange ivs, ValueRange /*inits*/)
                                                 {
                                                 // Materialize u = *slot(ivs[0]) as !llvm.ptr to i32.
                                                 Value uI32  = arith::IndexCastOp::create(pb, pl, i32Ty, ivs[0]);
                                                 Value uSlot = LLVM::AllocaOp::create(pb, pl, ptrTy, i32Ty, oneI32, 0);
                                                 LLVM::StoreOp::create(pb, pl, uI32, uSlot);

                                                 // Map the source forall's block arg to uSlot.
                                                 IRMapping mapping;
                                                 Block& srcBlock = anchor.getRegion().front();
                                                 mapping.map(srcBlock.getArgument(0), uSlot);

                                                 // Build the body with reduction threading.
                                                 ReductionBuilder builder(pb, pl, elemTy, inc, mapping);
                                                 Value localZero = arith::ConstantOp::create(pb, pl, elemTy, pb.getIntegerAttr(elemTy, 0));
                                                 Value local     = builder.buildBlock(srcBlock, localZero);

                                                 // Contribute to outer reduction.
                                                 auto reduceOp = scf::ReduceOp::create(pb, pl, ValueRange{local});
                                                 Block& rBlock = reduceOp.getReductions()[0].front();
                                                 {
                                                     OpBuilder::InsertionGuard g(pb);
                                                     pb.setInsertionPointToStart(&rBlock);
                                                     Value lhs = rBlock.getArgument(0);
                                                     Value rhs = rBlock.getArgument(1);
                                                     Value s   = arith::AddIOp::create(pb, pl, lhs, rhs);
                                                     scf::ReduceReturnOp::create(pb, pl, s);
                                                 }
                                             });

        // Post-loop: counter += result.
        b.setInsertionPointAfter(parOp);
        Value prior = LLVM::LoadOp::create(b, loc, elemTy, counterPtr);
        Value sum   = LLVM::AddOp::create(b, loc, prior, parOp.getResult(0));
        LLVM::StoreOp::create(b, loc, sum, counterPtr);

        // Erase the original anchor (and everything inside it).
        anchor.erase();
        return success();
    }
};

} // namespace

inline std::unique_ptr<Pass> createForallReductionTransformPass() { return std::make_unique<ForallReductionTransform>(); }
