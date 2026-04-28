#pragma once

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/IRMapping.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {

// Converts a single-dimension scf.parallel + scf.reduce into an scf.for with
// iter_args. The combiner region of each scf.reduce is inlined directly into
// the for body, and scf.yield carries the running accumulators.
//
// Before:
//   scf.parallel (%i) = (%lb) to (%ub) step (%step) init (%init) {
//     ...
//     scf.reduce(%val) : T {
//       ^bb0(%lhs: T, %rhs: T):
//         %r = <combiner> %lhs, %rhs : T
//         scf.reduce.return %r : T
//     }
//   } : T
//
// After:
//   %result = scf.for %i = %lb to %ub step %step iter_args(%acc = %init) -> T {
//     ...
//     %new_acc = <combiner> %acc, %val : T
//     scf.yield %new_acc : T
//   }
struct ParallelToForPattern : public OpRewritePattern<scf::ParallelOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult matchAndRewrite(scf::ParallelOp parallelOp,
                                  PatternRewriter& rewriter) const override {
        // Only handle single-IV parallel loops.
        if (parallelOp.getNumLoops() != 1)
            return failure();

        Location loc = parallelOp.getLoc();
        Value lb     = parallelOp.getLowerBound()[0];
        Value ub     = parallelOp.getUpperBound()[0];
        Value step   = parallelOp.getStep()[0];

        auto reduceOp =
            dyn_cast<scf::ReduceOp>(parallelOp.getBody()->getTerminator());
        if (!reduceOp)
            return failure();

        SmallVector<Value> initVals(parallelOp.getInitVals());

        // scf.for with iter_args seeded by the parallel's init values.
        auto forOp =
            scf::ForOp::create(rewriter, loc, lb, ub, step, initVals);

        Block* parallelBody = parallelOp.getBody();
        Block* forBody      = forOp.getBody();

        // When initVals is empty, scf::ForOp::build runs ensureTerminator and
        // emits an scf.yield itself. We always emit our own yield below, so
        // drop the auto-emitted one to avoid two terminators in the block.
        if (!forBody->empty())
            rewriter.eraseOp(forBody->getTerminator());

        rewriter.setInsertionPointToStart(forBody);

        // Map parallel IV → for IV (both are argument 0 of their blocks).
        IRMapping mapping;
        mapping.map(parallelBody->getArgument(0), forBody->getArgument(0));

        // Clone every op from the parallel body except the reduce terminator.
        for (auto& op : *parallelBody) {
            if (isa<scf::ReduceOp>(op))
                break;
            rewriter.clone(op, mapping);
        }

        // For each reduction, inline the combiner and collect new iter values.
        SmallVector<Value> yieldVals;
        for (auto [reduceVal, reductionRegion, iterArg] :
             llvm::zip(reduceOp.getOperands(),
                       reduceOp.getReductions(),
                       forOp.getRegionIterArgs())) {
            Block& combinerBlock = reductionRegion.front();

            IRMapping combinerMap;
            // lhs = running accumulator (iter_arg), rhs = per-iteration value.
            combinerMap.map(combinerBlock.getArgument(0), iterArg);
            combinerMap.map(combinerBlock.getArgument(1),
                            mapping.lookupOrDefault(reduceVal));

            Value newAcc;
            for (auto& op : combinerBlock) {
                if (auto retOp = dyn_cast<scf::ReduceReturnOp>(op)) {
                    newAcc = combinerMap.lookupOrDefault(retOp.getResult());
                    break;
                }
                rewriter.clone(op, combinerMap);
            }
            assert(newAcc && "combiner block must contain scf.reduce.return");
            yieldVals.push_back(newAcc);
        }

        scf::YieldOp::create(rewriter, loc, yieldVals);
        rewriter.replaceOp(parallelOp, forOp.getResults());
        return success();
    }
};

struct ConvertSCFParallelToForPass
    : public PassWrapper<ConvertSCFParallelToForPass,
                         OperationPass<ModuleOp>> {

    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ConvertSCFParallelToForPass)

    StringRef getArgument() const final { return "scf-parallel-to-for"; }
    StringRef getDescription() const final {
        return "Convert scf.parallel + scf.reduce to scf.for with iter_args";
    }

    void getDependentDialects(DialectRegistry& registry) const override {
        registry.insert<arith::ArithDialect, scf::SCFDialect>();
    }

    void runOnOperation() override {
        RewritePatternSet patterns(&getContext());
        patterns.add<ParallelToForPattern>(&getContext());
        if (failed(applyPatternsGreedily(getOperation(),
                                         std::move(patterns))))
            signalPassFailure();
    }
};

} // namespace

inline std::unique_ptr<Pass> createConvertSCFParallelToForPass() {
    return std::make_unique<ConvertSCFParallelToForPass>();
}
