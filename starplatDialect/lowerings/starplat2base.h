#ifndef STARPLAT2BASE
#define STARPLAT2BASE

#include "includes/StarPlatOps.h"
#include "includes/StarPlatTypes.h"
#include "mlir/Conversion/LLVMCommon/LoweringOptions.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/IR/BuiltinTypeInterfaces.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/Types.h"
#include "mlir/IR/Value.h"
#include "mlir/IR/ValueRange.h"
#include "mlir/Pass/PassManager.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/GPU/Transforms/ParallelLoopMapper.h" // for setMappingAttr
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "llvm/Support/LogicalResult.h"

#include "mlir/Dialect/LLVMIR/FunctionCallUtils.h"
#include "mlir/Transforms/DialectConversion.h" // from @llvm-project

#include "typeconverter.h"

// mlir::Operation *generateGraphStruct(mlir::MLIRContext *context)
// {
//     auto structType = LLVM::LLVMStructType::getIdentified(context, "Graph");

//     return;
// }

static constexpr llvm::StringLiteral kInReductionAttr("__in_reduction__");
inline mlir::LLVM::LLVMStructType createGraphStruct(mlir::MLIRContext* context) {

    auto structType = mlir::LLVM::LLVMStructType::getIdentified(context, "Graph");
    // Create Node struct type

    // auto ptrType = LLVM::LLVMPointerType::get(context);
    // Define Graph struct body with (Node*, int)
    // structType.setBody({ptrType, mlir::IntegerType::get(context, 32), ptrType}, /*isPacked=*/false);

    llvm::LogicalResult lr = structType.setBody({mlir::IntegerType::get(context, 64)}, /*isPacked=*/false);
    if (llvm::succeeded(lr))
        return structType;
    // structType.setBody({rewriter->getI64Type()}, false);

    return structType;
}

inline mlir::LLVM::LLVMStructType createNodeStruct(mlir::IRRewriter* rewriter, mlir::MLIRContext* context) {
    auto structType = mlir::LLVM::LLVMStructType::getIdentified(context, "Node");

    // Create a ptr type
    // auto ptr = mlir::LLVM::LLVMPointerType::get(rewriter->getI64Type());

    llvm::LogicalResult lr = structType.setBody({rewriter->getI64Type()}, false);
    if (llvm::succeeded(lr))
        return structType;

    return structType;
}

using namespace mlir;

struct ConvertFunc : public OpConversionPattern<mlir::starplat::FuncOp>
{
    using OpConversionPattern<mlir::starplat::FuncOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::FuncOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        // llvm::errs() << "I came here!!\n";
        auto loc = op.getLoc();

        TypeConverter::SignatureConversion sigConversion(op.getArgNames()->size());

        for (auto [idx, argType] : llvm::enumerate(op.getArgumentTypes())) {
            auto convertedType = getTypeConverter()->convertType(argType);
            if (!convertedType) {
                llvm::errs() << "Invalid conversion\n";
                return failure();
            }
            sigConversion.addInputs(idx, convertedType);
        }

        SmallVector<Type> convertedResultTypes;
        if (failed(getTypeConverter()->convertTypes(op.getFunctionType().getResults(), convertedResultTypes)))
            return failure();
        // auto returnType = LLVM::LLVMPointerType::get(op.getContext());
        // // auto returnType =  MemRefType::get({mlir::ShapedType::kDynamic}, rewriter.getI1Type());
        //
        // // auto returnType = rewriter.getI64Type(); // up up
        //
        // // auto i64Type = rewriter.getI64Type();
        //
        // // Function signature: (i32, i32) -> i32
        auto oldFuncType = op.getFunctionType();
        //
        // // SmallVector<Type> paramTypes = {oldFuncType.getInput(0), oldFuncType.getInput(1), oldFuncType.getInput(2),
        // oldFuncType.getInput(3)};
        SmallVector<Type> paramTypes = {oldFuncType.getInput(0)};
        //
        // auto funcType = mlir::FunctionType::get(rewriter.getContext(), paramTypes, returnType);
        // auto funcType = FunctionType::get(rewriter.getContext(), {}, false);
        // auto llvmFuncTy = LLVM::LLVMFunctionType::get(LLVM::LLVMVoidType::get(rewriter.getContext()), paramTypes, false);
        auto funcType = mlir::FunctionType::get(rewriter.getContext(),
                                                sigConversion.getConvertedTypes(), // inputs
                                                convertedResultTypes.empty()       // results
                                                    ? TypeRange{}
                                                    : TypeRange{convertedResultTypes.front()});

        auto funcOp2  = func::FuncOp::create(rewriter, loc, op.getSymName(), funcType);
        rewriter.inlineRegionBefore(op.getRegion(), funcOp2.getBody(), funcOp2.end());
        // // auto funcOp = LLVM::LLVMFuncOp::create(rewriter,loc, funcName, funcType);
        //
        // // Create an entry block with the right number of arguments
        // // Block *entryBlock = rewriter.createBlock(&funcOp.getBody(), {}, paramTypes, {loc, loc});
        //
        // Block& entryBlock = funcOp2.getBody().front();
        // for (BlockArgument arg : entryBlock.getArguments())
        //     assert(arg.use_empty() && "expected no uses of function arguments");
        //
        // llvm::BitVector argsToErase(entryBlock.getNumArguments(), true);
        // entryBlock.eraseArguments(argsToErase);
        //
        if (failed(rewriter.convertRegionTypes(&funcOp2.getBody(), *getTypeConverter(), &sigConversion)))
            return failure();

        // TODO: add attributes to generated function
        rewriter.eraseOp(op);
        // auto module = op->getParentOfType<mlir::ModuleOp>();
        // FailureOr<LLVM::LLVMFuncOp> graphAddEdgeFn = LLVM::lookupOrCreateFn(rewriter, module, "graph_add_edge", {}, rewriter.getI64Type());

        // Replace original starplat.func
        return success();
    }
};

struct ConvertDeclareOp : public OpConversionPattern<mlir::starplat::DeclareOp>
{
    ConvertDeclareOp(mlir::MLIRContext* context) : OpConversionPattern<mlir::starplat::DeclareOp>(context) {}

    using OpConversionPattern<mlir::starplat::DeclareOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::DeclareOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {

        auto resType = op->getResult(0).getType();
        auto ctx     = op.getContext();
        auto loc     = op.getLoc();
        // resType.dump();
        if (auto pnodetype = dyn_cast<starplat::PropNodeType>(resType)) {
            auto module = op->getParentOfType<mlir::ModuleOp>();
            FailureOr<LLVM::LLVMFuncOp> graphCountNodes =
                LLVM::lookupOrCreateFn(rewriter, module, "get_num_nodes", {LLVM::LLVMPointerType::get(ctx)}, rewriter.getI32Type());
            auto num_nodes = LLVM::CallOp::create(rewriter, loc, *graphCountNodes, ValueRange{adaptor.getOperands()[0]});
            // auto elemType = pnodetype.getParameter();
            // auto num_nodes     = LLVM::ConstantOp::create(rewriter, loc, rewriter.getI32Type(), rewriter.getI32IntegerAttr(10));
            // NOTE: propNode<i1,"g"> is widened to memref<?xi8> at the type-converter level
            // (i1 is not a legal atomicrmw operand type in LLVM IR). Every read/write site
            // inserts the i1<->i8 conversion.
            auto elemType      = rewriter.getI8Type();
            auto arrayType     = MemRefType::get({mlir::ShapedType::kDynamic}, elemType);
            Value sizeIdx      = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), num_nodes.getResult());
            auto arrayCreation = memref::AllocaOp::create(rewriter, loc, arrayType, ValueRange{sizeIdx});
            rewriter.replaceOp(op, arrayCreation);
            // TODO need to get the graph.
        }
        // if (isa<starplat::SPIntType>(resType)) {
        //     auto loc                   = op->getLoc();
        //     mlir::MLIRContext* context = getContext();
        //     auto allocaop = LLVM::AllocaOp::create(rewriter, loc, LLVM::LLVMPointerType::get(context), LLVM::LLVMPointerType::get(context),
        //                                            LLVM::ConstantOp::create(rewriter, loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1)),
        //                                            0);
        //     rewriter.replaceOp(op, allocaop);
        // }
        // else if (isa<starplat::NodeType>(resType)) {
        //     auto loc                   = op->getLoc();
        //     mlir::MLIRContext* context = getContext();
        //     auto allocaop = LLVM::AllocaOp::create(rewriter, loc, LLVM::LLVMPointerType::get(context), LLVM::LLVMPointerType::get(context),
        //                                            LLVM::ConstantOp::create(rewriter, loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1)),
        //                                            0);
        //     rewriter.replaceOp(op, allocaop);
        // }
        // else {
        //     llvm::outs() << "Error: This DeclareOp lowering not yet implemented.";
        //     exit(0);
        // }
        // auto resType = op->getResult(0).getType();
        // if (isa<mlir::starplat::PropNodeType>(resType)) {
        //     auto loc          = op->getLoc();
        //
        //     auto rescast      = dyn_cast<mlir::starplat::PropNodeType>(resType);
        //
        //     auto field0       = LLVM::ExtractValueOp::create(rewriter, loc, mlir::IntegerType::get(op.getContext(), 64),
        // adaptor.getOperands()[0],
        //                                                      rewriter.getDenseI64ArrayAttr({0}));
        //     Value dynamicSize = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), field0);
        //
        //     MemRefType memrefType;
        //     if (rescast.getParameter() == mlir::IntegerType::get(rewriter.getContext(), 1))
        //         memrefType = MemRefType::get({mlir::ShapedType::kDynamic}, rewriter.getI1Type());
        //
        //     else if (rescast.getParameter() == mlir::IntegerType::get(rewriter.getContext(), 64))
        //         memrefType = MemRefType::get({mlir::ShapedType::kDynamic}, rewriter.getI64Type());
        //
        //     else {
        //         llvm::outs() << "Error: MemrefType not implemented\n";
        //         exit(0);
        //     }
        //     Value allocated = memref::AllocOp::create(rewriter, loc, memrefType, mlir::ValueRange({dynamicSize}));
        //
        //     rewriter.replaceOp(op, allocated);
        // }
        //
        // else if (isa<mlir::IntegerType>(resType)) {
        //     auto loc     = op->getLoc();
        //     auto resCast = dyn_cast<mlir::IntegerType>(resType);
        //
        //     MemRefType memrefType;
        //
        //     if (resCast.getWidth() == 64)
        //         memrefType = MemRefType::get({}, rewriter.getI64Type());
        //     else if (resCast.getWidth() == 1)
        //         memrefType = MemRefType::get({}, rewriter.getI1Type());
        //
        //     else {
        //         llvm::outs() << "Error: MemrefType Integer type not implemented.\n";
        //         exit(0);
        //     }
        //
        //     Value allocated = memref::AllocOp::create(rewriter, loc, memrefType);
        //     rewriter.replaceOp(op, allocated);
        // }
        //
        // else if (isa<mlir::starplat::NodeType>(resType)) {
        //     llvm::outs() << "Hello\n";
        // }
        //
        // else {
        //     llvm::outs() << "Error: This DeclareOp lowering not yet implemented.";
        //     return failure();
        // }

        return success();
    }
};

struct ConvertDeclareOp2 : public OpConversionPattern<mlir::starplat::DeclareOp2>
{
    ConvertDeclareOp2(mlir::MLIRContext* context) : OpConversionPattern<mlir::starplat::DeclareOp2>(context) {}

    using OpConversionPattern<mlir::starplat::DeclareOp2>::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::DeclareOp2 op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {

        auto resType = op->getResult(0).getType();
        if (isa<starplat::SPIntType>(resType) || isa<starplat::SPFloatType>(resType) || isa<starplat::NodeType>(resType) ||
            isa<starplat::EdgeType>(resType)) {
            auto loc                   = op->getLoc();
            mlir::MLIRContext* context = getContext();
            auto allocaop = LLVM::AllocaOp::create(rewriter, loc, LLVM::LLVMPointerType::get(context), LLVM::LLVMPointerType::get(context),
                                                   LLVM::ConstantOp::create(rewriter, loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1)), 0);
            rewriter.replaceOp(op, allocaop);
        }
        else {
            llvm::outs() << "Error: This DeclareOp lowering not yet implemented.";
            exit(0);
        }
        // auto resType = op->getResult(0).getType();
        // if (isa<mlir::starplat::PropNodeType>(resType)) {
        //     auto loc          = op->getLoc();
        //
        //     auto rescast      = dyn_cast<mlir::starplat::PropNodeType>(resType);
        //
        //     auto field0       = LLVM::ExtractValueOp::create(rewriter, loc, mlir::IntegerType::get(op.getContext(), 64), adaptor.getOperands()[0],
        //                                                      rewriter.getDenseI64ArrayAttr({0}));
        //     Value dynamicSize = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), field0);
        //
        //     MemRefType memrefType;
        //     if (rescast.getParameter() == mlir::IntegerType::get(rewriter.getContext(), 1))
        //         memrefType = MemRefType::get({mlir::ShapedType::kDynamic}, rewriter.getI1Type());
        //
        //     else if (rescast.getParameter() == mlir::IntegerType::get(rewriter.getContext(), 64))
        //         memrefType = MemRefType::get({mlir::ShapedType::kDynamic}, rewriter.getI64Type());
        //
        //     else {
        //         llvm::outs() << "Error: MemrefType not implemented\n";
        //         exit(0);
        //     }
        //     Value allocated = memref::AllocOp::create(rewriter, loc, memrefType, mlir::ValueRange({dynamicSize}));
        //
        //     rewriter.replaceOp(op, allocated);
        // }
        //
        // else if (isa<mlir::IntegerType>(resType)) {
        //     auto loc     = op->getLoc();
        //     auto resCast = dyn_cast<mlir::IntegerType>(resType);
        //
        //     MemRefType memrefType;
        //
        //     if (resCast.getWidth() == 64)
        //         memrefType = MemRefType::get({}, rewriter.getI64Type());
        //     else if (resCast.getWidth() == 1)
        //         memrefType = MemRefType::get({}, rewriter.getI1Type());
        //
        //     else {
        //         llvm::outs() << "Error: MemrefType Integer type not implemented.\n";
        //         exit(0);
        //     }
        //
        //     Value allocated = memref::AllocOp::create(rewriter, loc, memrefType);
        //     rewriter.replaceOp(op, allocated);
        // }
        //
        // else if (isa<mlir::starplat::NodeType>(resType)) {
        //     llvm::outs() << "Hello\n";
        // }
        //
        // else {
        //     llvm::outs() << "Error: This DeclareOp lowering not yet implemented.";
        //     return failure();
        // }

        return success();
    }
};

struct ConvertConstOp : public OpConversionPattern<mlir::starplat::ConstOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::ConstOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {

        auto loc   = op.getLoc();

        auto value = op.getValueAttr();

        if (isa<mlir::IntegerAttr>(value)) {
            auto newOp = LLVM::ConstantOp::create(rewriter, loc, rewriter.getI64Type(), value);
            rewriter.replaceOp(op, newOp);
        }
        else if (auto attr = dyn_cast<mlir::StringAttr>(value)) {
            if (attr.getValue() == "INF") {
                auto newOp = LLVM::ConstantOp::create(rewriter, loc, op.getResult().getType(), rewriter.getI64IntegerAttr(2147483647));
                rewriter.replaceOp(op, newOp);
            }
            else if (attr.getValue() == "False") {
                auto newOp = LLVM::ConstantOp::create(rewriter, loc, op.getResult().getType(), false);
                rewriter.replaceOp(op, newOp);
            }
            else if (attr.getValue() == "True") {
                auto newOp = LLVM::ConstantOp::create(rewriter, loc, op.getResult().getType(), true);
                rewriter.replaceOp(op, newOp);
            }
            else if (std::all_of(attr.getValue().begin(), attr.getValue().end(), ::isdigit)) {
                auto newOp = LLVM::ConstantOp::create(rewriter, loc, op.getResult().getType(), std::stoi(attr.getValue().str()));
                rewriter.replaceOp(op, newOp);
            }
            else {
                llvm::outs() << "Error: This ConstantOp lowering not yet implemented.";
                exit(0);
            }
        }
        else {
            llvm::outs() << "Error: This ConstantOp lowering not yet implemented.";
            exit(0);
        }

        // if (cast<mlir::StringAttr>(value).getValue() == "False") {
        // auto newOp = LLVM::ConstantOp::create(rewriter, loc, mlir::IntegerType::get(op.getContext(), 1), rewriter.getBoolAttr(0));
        //     rewriter.replaceOp(op, newOp);
        // }
        //
        // else if (cast<mlir::StringAttr>(value).getValue() == "True") {
        //     auto newOp = LLVM::ConstantOp::create(rewriter, loc, mlir::IntegerType::get(op.getContext(), 1), rewriter.getBoolAttr(1));
        //     rewriter.replaceOp(op, newOp);
        // }
        // else {
        //     llvm::outs() << "Error: This ConstantOp lowering not yet implemented.";
        //     exit(0);
        // }

        return success();
    }
};

struct ConvertAssignOp : public OpConversionPattern<mlir::starplat::AssignmentOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::AssignmentOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        // llvm::errs() << "=== ConvertAssignOp firing ===\n";
        // llvm::errs() << "Original operand 0 type: " << op->getOperand(0).getType() << "\n";
        // llvm::errs() << "Original operand 1 type: " << op->getOperand(1).getType() << "\n";
        // llvm::errs() << "Adaptor operand 0 type:  " << adaptor.getOperands()[0].getType() << "\n";
        // llvm::errs() << "Adaptor operand 1 type:  " << adaptor.getOperands()[1].getType() << "\n";
        //
        // // also dump the defining op of operand 0
        // llvm::errs() << "Operand 0 defined by: ";
        // adaptor.getOperands()[0].getDefiningOp()->dump();
        // llvm::errs() << "Operand 1 defined by: ";
        // adaptor.getOperands()[1].getDefiningOp()->dump();
        //
        //
        // adaptor.getOperands()[0].dump();
        // adaptor.getOperands()[1].dump();
        auto storeop = LLVM::StoreOp::create(rewriter, op.getLoc(), adaptor.getOperands()[1], adaptor.getOperands()[0]);

        rewriter.replaceOp(op, storeop);

        return success();
    }
};

struct ConvertGetEdge : public OpConversionPattern<mlir::starplat::GetEdgeOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::GetEdgeOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        Location loc         = op.getLoc();
        MLIRContext* context = rewriter.getContext();

        // Locate the enclosing module so we can look up / create the runtime decl.
        auto module = op->getParentOfType<ModuleOp>();
        if (!module)
            return rewriter.notifyMatchFailure(op, "no enclosing module");

        // --- 1. Declare (or fetch) the runtime helper ---
        //   i8* get_edge(i8* graph, i32 src, i32 dst)
        auto ptrTy = LLVM::LLVMPointerType::get(context);
        FailureOr<LLVM::LLVMFuncOp> getEdgeFn =
            LLVM::lookupOrCreateFn(rewriter, module, "get_edge", {ptrTy, rewriter.getI32Type(), rewriter.getI32Type()}, ptrTy);
        if (failed(getEdgeFn))
            return rewriter.notifyMatchFailure(op, "failed to declare get_edge");

        // --- 2. Pull the converted operands ---
        Value graphPtr = adaptor.getOperands()[0]; // !llvm.ptr (graph)
        Value srcPtr   = adaptor.getOperands()[1]; // !llvm.ptr (to i32 index)
        Value dstPtr   = adaptor.getOperands()[2]; // !llvm.ptr (to i32 index)

        // --- 3. Load the i32 indices from the two node pointers ---
        Type i32Ty   = rewriter.getI32Type();
        Value srcIdx = LLVM::LoadOp::create(rewriter, loc, i32Ty, srcPtr);
        Value dstIdx = LLVM::LoadOp::create(rewriter, loc, i32Ty, dstPtr);

        // --- 4. Call get_edge(graph, srcIdx, dstIdx) ---
        auto callOp = LLVM::CallOp::create(rewriter, loc, getEdgeFn.value(), ValueRange{graphPtr, srcIdx, dstIdx});

        rewriter.replaceOp(op, callOp.getResult());
        return success();
    }
};

// --- Shared helpers for lowering starplat forall ops ------------------------
//
// Both forallnodes and forallneighbours lower the same way: scf.parallel when
// they sit at the top, scf.for when they nest inside another forall. Only the
// trip count and the per-iteration body-arg materialization differ:
//
//   forallnodes(g):
//     N = get_num_nodes(g); iv -> alloca i32, store iv, yield ptr
//
//   forallneighbours(g, u):
//     uId = *u; N = get_num_neighbours(g, uId); nbrs = get_neighbours(g, uId);
//     iv -> alloca i32, store nbrs[iv], yield ptr
//
// planForallLoop computes the trip count and returns a materializer closure
// for the body-arg pointer. lowerOutermostForall drives the rest — walking
// for a reduction, emitting scf.parallel, and threading any reduction through
// nested scf.fors/scf.ifs.

struct ForallLoopPlan
{
    Value upperBound; // index-typed loop trip count
    std::function<Value(OpBuilder&, Location, Value)> materializeBodyArg;
};

static FailureOr<ForallLoopPlan> planForallLoop(Operation* forallOp, ConversionPatternRewriter& rewriter, Location loc, Value graphPtr,
                                                Value nodePtrOrNull) {
    auto* ctx       = rewriter.getContext();
    auto ptrTy      = LLVM::LLVMPointerType::get(ctx);
    auto i32Ty      = rewriter.getI32Type();
    auto i64Ty      = rewriter.getI64Type();
    auto idxTy      = rewriter.getIndexType();
    ModuleOp module = forallOp->getParentOfType<ModuleOp>();
    Value oneI32    = LLVM::ConstantOp::create(rewriter, loc, i32Ty, rewriter.getI32IntegerAttr(1));

    if (isa<starplat::ForAllNodesOp>(forallOp)) {
        FailureOr<LLVM::LLVMFuncOp> getNumNodesFn = LLVM::lookupOrCreateFn(rewriter, module, "get_num_nodes", {ptrTy}, i32Ty);
        if (failed(getNumNodesFn))
            return failure();

        Value nI32        = LLVM::CallOp::create(rewriter, loc, *getNumNodesFn, ValueRange{graphPtr}).getResult();
        Value nIdx        = arith::IndexCastOp::create(rewriter, loc, idxTy, nI32);

        auto materializer = [=](OpBuilder& b, Location l, Value iv) -> Value
        {
            Value ivI32 = arith::IndexCastOp::create(b, l, i32Ty, iv);
            Value slot  = LLVM::AllocaOp::create(b, l, ptrTy, i32Ty, oneI32, /*alignment=*/0);
            LLVM::StoreOp::create(b, l, ivI32, slot);
            return slot;
        };
        return ForallLoopPlan{nIdx, materializer};
    }

    if (isa<starplat::ForAllNeighboursOp>(forallOp)) {
        assert(nodePtrOrNull && "ForAllNeighbours requires a node pointer");
        FailureOr<LLVM::LLVMFuncOp> getNumNbrsFn = LLVM::lookupOrCreateFn(rewriter, module, "get_num_neighbours", {ptrTy, i32Ty}, i32Ty);
        if (failed(getNumNbrsFn))
            return failure();
        FailureOr<LLVM::LLVMFuncOp> getNbrsFn = LLVM::lookupOrCreateFn(rewriter, module, "get_neighbours", {ptrTy, i32Ty}, ptrTy);
        if (failed(getNbrsFn))
            return failure();

        Value uId         = LLVM::LoadOp::create(rewriter, loc, i32Ty, nodePtrOrNull);
        Value nI32        = LLVM::CallOp::create(rewriter, loc, *getNumNbrsFn, ValueRange{graphPtr, uId}).getResult();
        Value nIdx        = arith::IndexCastOp::create(rewriter, loc, idxTy, nI32);
        Value nbrsPtr     = LLVM::CallOp::create(rewriter, loc, *getNbrsFn, ValueRange{graphPtr, uId}).getResult();

        auto materializer = [=](OpBuilder& b, Location l, Value iv) -> Value
        {
            Value ivI64    = arith::IndexCastOp::create(b, l, i64Ty, iv);
            Value elemAddr = LLVM::GEPOp::create(b, l, ptrTy, /*elementType=*/i32Ty, nbrsPtr, ArrayRef<LLVM::GEPArg>{LLVM::GEPArg(ivI64)});
            Value vI32     = LLVM::LoadOp::create(b, l, i32Ty, elemAddr);
            Value slot     = LLVM::AllocaOp::create(b, l, ptrTy, i32Ty, oneI32, /*alignment=*/0);
            LLVM::StoreOp::create(b, l, vI32, slot);
            return slot;
        };
        return ForallLoopPlan{nIdx, materializer};
    }

    return failure();
}

// Resolve an operand of a nested forall to an SSA value usable in the outer
// scf.parallel body. If the operand is an outer forall's body arg we remapped
// locally, pick up the mapping; otherwise fall back to the type-converted
// value (function arg, declared node, etc).
static Value resolveNestedOperand(Value origVal, const IRMapping& mapping, ConversionPatternRewriter& rewriter) {
    Value mapped = mapping.lookupOrDefault(origVal);
    if (mapped != origVal)
        return mapped;
    if (Value remapped = rewriter.getRemappedValue(origVal))
        return remapped;
    return origVal;
}

// Whether this forall sits at the top of the forall nest (i.e., not nested
// inside another ForAllNodesOp/ForAllNeighboursOp). Only the outermost one
// emits scf.parallel; nested ones are consumed by cloneBlock below.
static bool isOutermostStarplatForall(Operation* op) {
    Operation* parent = op->getParentOp();
    while (parent) {
        if (isa<starplat::ForAllNodesOp, starplat::ForAllNeighboursOp>(parent))
            return false;
        parent = parent->getParentOp();
    }
    return true;
}

static LogicalResult lowerOutermostForall(Operation* op, ConversionPatternRewriter& rewriter, Value convertedGraphPtr, Value convertedNodePtrOrNull) {
    auto loc = op->getLoc();

    // Walk transitively — IncAndAssignOp may sit inside any mix of nested
    // forall levels and wrapper ops (StarPlatIfOp, etc) in the outermost
    // forall's subtree.
    starplat::IncAndAssignOp reductionOp = nullptr;
    op->walk(
        [&](starplat::IncAndAssignOp inc)
        {
            if (!reductionOp)
                reductionOp = inc;
        });
    bool hasReduction = reductionOp != nullptr;

    Value initVal;
    Value reductionPtr;
    if (hasReduction) {
        reductionPtr = rewriter.getRemappedValue(reductionOp->getOperand(0));
        initVal      = LLVM::LoadOp::create(rewriter, loc, rewriter.getI64Type(), reductionPtr);
    }

    auto outerPlanOr = planForallLoop(op, rewriter, loc, convertedGraphPtr, convertedNodePtrOrNull);
    if (failed(outerPlanOr))
        return failure();
    ForallLoopPlan outerPlan = *outerPlanOr;

    Value zeroIdx            = arith::ConstantIndexOp::create(rewriter, loc, 0);
    Value oneIdx             = arith::ConstantIndexOp::create(rewriter, loc, 1);

    auto parallelOp          = scf::ParallelOp::create(rewriter, loc,
                                                       /*lowerBounds=*/ValueRange{zeroIdx},
                                                       /*upperBounds=*/ValueRange{outerPlan.upperBound},
                                                       /*steps=*/ValueRange{oneIdx},
                                              /*initVals=*/hasReduction ? ValueRange{initVal} : ValueRange{});

             {
        OpBuilder::InsertionGuard g(rewriter);
        rewriter.setInsertionPointToStart(parallelOp.getBody());

        IRMapping mapping;
        Value outerBodyArg = outerPlan.materializeBodyArg(rewriter, loc, parallelOp.getInductionVars()[0]);
        mapping.map(op->getRegion(0).front().getArgument(0), outerBodyArg);

        Value zero = arith::ConstantOp::create(rewriter, loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(0));

        // cloneBlock: walks a source block and emits it at the current
        // insertion point, seeing through region-bearing wrapper ops and
        // emitting scf.for for nested starplat forall levels (nodes OR
        // neighbours — both get the same treatment, just different bounds
        // and body-arg materialization via planForallLoop).
        //
        // The reduction is threaded as an SSA accumulator via `iterArg`
        // (passed by reference):
        //   * IncAndAssignOp -> iterArg = iterArg + delta.
        //   * Load of the reduction pointer -> result remapped to iterArg.
        //   * starplat.spif whose body transitively contains the reduction
        //     is promoted to scf.if that yields the updated/unchanged
        //     accumulator; iterArg is rebound to the scf.if's result.
        //   * Nested starplat forall is emitted as scf.for. If its subtree
        //     transitively contains the reduction, the scf.for takes iterArg
        //     as its iter_arg, yields the final accumulator, and iterArg is
        //     rebound to the scf.for's result.
        //   * Any other region-bearing wrapper: clone shell and recurse.
        // cloneBlock never emits scf.yield itself — each emit site (nested
        // scf.for/scf.if and the outermost scf.parallel caller) emits its
        // own terminator using the final iterArg.
        std::function<void(Block*, Value&)> cloneBlock;
        cloneBlock = [&](Block* srcBlock, Value& iterArg)
        {
            for (auto& bodyOp : *srcBlock) {
                if (isa<starplat::endOp>(bodyOp))
                    continue;

                if (isa<starplat::IncAndAssignOp>(bodyOp)) {
                    Value delta = mapping.lookupOrDefault(bodyOp.getOperand(1));
                    iterArg     = arith::AddIOp::create(rewriter, loc, iterArg, delta);
                    continue;
                }

                // Load of the spint reduction pointer — remap its result
                // to the current accumulator so users see the SSA value
                // instead of a stale memory read.
                if (bodyOp.getNumResults() > 0 && hasReduction &&
                    llvm::any_of(bodyOp.getOperands(), [&](Value v) { return v == reductionOp->getOperand(0); })) {
                    for (Value res : bodyOp.getResults())
                        mapping.map(res, iterArg);
                    continue;
                }

                // Nested starplat forall (nodes or neighbours) -> scf.for.
                // Thread iterArg through the scf.for's iter_args iff the
                // nested subtree transitively contains the reduction.
                if (isa<starplat::ForAllNodesOp, starplat::ForAllNeighboursOp>(&bodyOp)) {
                    Operation* nested      = &bodyOp;
                    bool containsReduction = false;
                    if (hasReduction) {
                        nested->walk([&](starplat::IncAndAssignOp) { containsReduction = true; });
                    }

                    Value nestedGraphPtr = resolveNestedOperand(nested->getOperand(0), mapping, rewriter);
                    Value nestedNodePtr  = nullptr;
                    if (isa<starplat::ForAllNeighboursOp>(nested))
                        nestedNodePtr = resolveNestedOperand(nested->getOperand(1), mapping, rewriter);

                    auto nestedPlanOr = planForallLoop(nested, rewriter, loc, nestedGraphPtr, nestedNodePtr);
                    if (failed(nestedPlanOr))
                        continue;
                    ForallLoopPlan nestedPlan = *nestedPlanOr;

                    if (containsReduction) {
                        auto forOp = scf::ForOp::create(rewriter, loc, zeroIdx, nestedPlan.upperBound, oneIdx, ValueRange{iterArg});
                        OpBuilder::InsertionGuard fg(rewriter);
                        rewriter.setInsertionPointToStart(forOp.getBody());
                        Value argPtr = nestedPlan.materializeBodyArg(rewriter, loc, forOp.getInductionVar());
                        mapping.map(nested->getRegion(0).front().getArgument(0), argPtr);
                        Value innerIter = forOp.getRegionIterArgs()[0];
                        cloneBlock(&nested->getRegion(0).front(), innerIter);
                        scf::YieldOp::create(rewriter, loc, ValueRange{innerIter});
                        iterArg = forOp.getResult(0);
                    }
                    else {
                        auto forOp = scf::ForOp::create(rewriter, loc, zeroIdx, nestedPlan.upperBound, oneIdx);
                        OpBuilder::InsertionGuard fg(rewriter);
                        rewriter.setInsertionPointToStart(forOp.getBody());
                        Value argPtr = nestedPlan.materializeBodyArg(rewriter, loc, forOp.getInductionVar());
                        mapping.map(nested->getRegion(0).front().getArgument(0), argPtr);
                        Value dummy;
                        cloneBlock(&nested->getRegion(0).front(), dummy);
                    }
                    continue;
                }

                // Other region-bearing wrappers: either promote spif to
                // scf.if (on the reduction path) or clone the shell and
                // recurse.
                if (bodyOp.getNumRegions() > 0) {
                    bool containsReduction = false;
                    if (hasReduction) {
                        bodyOp.walk([&](starplat::IncAndAssignOp) { containsReduction = true; });
                    }

                    if (auto spif = dyn_cast<starplat::StarPlatIfOp>(&bodyOp); spif && containsReduction) {
                        // Promote spif to scf.if so the accumulator update
                        // is conditional on spif's condition. The scf.if
                        // yields the (possibly-updated) accumulator and
                        // iterArg is rebound to scf.if's result for the
                        // rest of the block.
                        Value cond = mapping.lookupOrDefault(spif.getCondition());
                        auto ifOp  = scf::IfOp::create(rewriter, loc, TypeRange{iterArg.getType()}, cond, /*withElseRegion=*/true);
                        {
                            OpBuilder::InsertionGuard g(rewriter);
                            rewriter.setInsertionPointToStart(ifOp.thenBlock());
                            Value thenIter = iterArg;
                            cloneBlock(&spif.getBody().front(), thenIter);
                            scf::YieldOp::create(rewriter, loc, ValueRange{thenIter});
                        }
                        {
                            OpBuilder::InsertionGuard g(rewriter);
                            rewriter.setInsertionPointToStart(ifOp.elseBlock());
                            scf::YieldOp::create(rewriter, loc, ValueRange{iterArg});
                        }
                        iterArg = ifOp.getResult(0);
                        continue;
                    }

                    // Non-reduction wrapper: build the shell via
                    // OperationState (cloning the whole op and clearing
                    // regions would bypass the ConversionPatternRewriter's
                    // tracking and leave stale pointers in its worklist —
                    // crash in isLegal), then recurse into each region's
                    // block.
                    SmallVector<Value, 4> remappedOperands;
                    for (Value v : bodyOp.getOperands())
                        remappedOperands.push_back(mapping.lookupOrDefault(v));
                    OperationState state(bodyOp.getLoc(), bodyOp.getName());
                    state.addOperands(remappedOperands);
                    state.addTypes(bodyOp.getResultTypes());
                    state.addAttributes(bodyOp.getAttrs());
                    state.addSuccessors(bodyOp.getSuccessors());
                    for (unsigned i = 0, e = bodyOp.getNumRegions(); i < e; ++i)
                        state.addRegion();
                    Operation* clonedWrapper = rewriter.create(state);
                    for (auto [origRegion, clonedRegion] : llvm::zip(bodyOp.getRegions(), clonedWrapper->getRegions())) {
                        for (Block& origBlk : origRegion) {
                            SmallVector<Type, 4> argTypes;
                            SmallVector<Location, 4> argLocs;
                            for (BlockArgument arg : origBlk.getArguments()) {
                                argTypes.push_back(arg.getType());
                                argLocs.push_back(arg.getLoc());
                            }
                            Block* clonedBlk = rewriter.createBlock(&clonedRegion, clonedRegion.end(), argTypes, argLocs);
                            for (auto [origArg, clonedArg] : llvm::zip(origBlk.getArguments(), clonedBlk->getArguments()))
                                mapping.map(origArg, clonedArg);
                            OpBuilder::InsertionGuard wg(rewriter);
                            rewriter.setInsertionPointToStart(clonedBlk);
                            cloneBlock(&origBlk, iterArg);
                        }
                    }
                    continue;
                }

                // Everything else: clone verbatim. The driver fires the op's
                // own lowering pattern on the clone in a later iteration.
                rewriter.clone(bodyOp, mapping);
            }
        };

        Value iterArg = zero;
        cloneBlock(&op->getRegion(0).front(), iterArg);

        if (hasReduction) {
            auto reduceOp = scf::ReduceOp::create(rewriter, loc, ValueRange{iterArg});
            Block& rb     = reduceOp.getReductions()[0].front();
            OpBuilder::InsertionGuard rg(rewriter);
            rewriter.setInsertionPointToStart(&rb);
            Value sum = arith::AddIOp::create(rewriter, loc, rb.getArgument(0), rb.getArgument(1));
            scf::ReduceReturnOp::create(rewriter, loc, sum);
        }
    }

    if (hasReduction) {
        rewriter.setInsertionPointAfter(parallelOp);
        LLVM::StoreOp::create(rewriter, loc, parallelOp.getResult(0), reductionPtr);
    }

    rewriter.eraseOp(op);
    return success();
}

struct ForallNodesOpLoweringCPU : public OpConversionPattern<starplat::ForAllNodesOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(starplat::ForAllNodesOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        if (!isOutermostStarplatForall(op))
            return failure();
        Value graphPtr = adaptor.getOperands()[0];
        return lowerOutermostForall(op, rewriter, graphPtr, /*nodePtr=*/nullptr);
    }
};

struct ConvertIfOp : public OpConversionPattern<mlir::starplat::StarPlatIfOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::StarPlatIfOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        auto loc = op.getLoc();
        // mlir::MLIRContext* context = getContext();
        auto ifop = scf::IfOp::create(rewriter, loc, op.getCondition(),
                                      [&](OpBuilder& b, Location loc)
                                      {
                                          // IRMapping mapping;
                                          // Block& srcBlock = op.getBody().front();
                                          //
                                          // for (auto& op : srcBlock.without_terminator())
                                          //     b.clone(op, mapping);
                                          scf::YieldOp::create(b, loc);
                                      });
        // op->getParentOp()->dump();
        Block* ifBody = ifop.getBody();
        rewriter.setInsertionPoint(ifBody->getTerminator());

        // rewriter.moveOpBefore(nodes, forallBody->getTerminator());

        // rewriter.setInsertionPointToStart(forallBody);

        // llvm::errs() << "\n";
        // llvm::errs() << adaptor.getOperands()[0] << "\n";
        // llvm::errs() << adaptor.getOperands()[1] << "\n";
        // llvm::errs() << "\n";
        // LLVM::CallOp::create(rewriter, loc, *graphAddEdgeFn, ValueRange{});

        // rewriter.inlineBlockBefore(&op.getBody().front(), forallBody, forallBody->end());

        Block& srcBlock = op.getBody().front();
        for (auto& innerOp : llvm::make_early_inc_range(srcBlock)) {

            // llvm::errs() << "hi\n";
            if (isa<starplat::endOp>(&innerOp)) {
                rewriter.eraseOp(&innerOp); // erase starplat.end, keep scf terminator
                continue;
            }
            rewriter.moveOpBefore(&innerOp, ifBody->getTerminator());
        }

        rewriter.eraseOp(op);
        return success();
    }
};

struct ConvertNodeCmpOp : public OpConversionPattern<mlir::starplat::CmpOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::CmpOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        Location loc = op.getLoc();

        Value lhs    = adaptor.getOperands()[0];
        Value rhs    = adaptor.getOperands()[1];

        // A starplat.node lowers to !llvm.ptr holding the i32 index. If the
        // (converted) operand is a pointer, dereference to the i32 index.
        // (We can't rely on op->getOperand(i).getType() being a NodeType —
        // when this op is the product of cloneBlock inside an outer forall
        // lowering, the operand type is already !llvm.ptr.)
        auto derefIfPtr = [&](Value v) -> Value
        {
            if (isa<LLVM::LLVMPointerType>(v.getType()))
                return LLVM::LoadOp::create(rewriter, loc, rewriter.getI32Type(), v);
            return v;
        };

        Value lhsVal = derefIfPtr(lhs);
        Value rhsVal = derefIfPtr(rhs);

        // Sanity: both sides must now be the same integer type for arith.cmpi.
        // If one side was a node (now i32) and the other was, say, i64, widen.
        if (lhsVal.getType() != rhsVal.getType()) {
            auto lhsIntTy = dyn_cast<IntegerType>(lhsVal.getType());
            auto rhsIntTy = dyn_cast<IntegerType>(rhsVal.getType());
            if (!lhsIntTy || !rhsIntTy) {
                return rewriter.notifyMatchFailure(op, "cmp operands must be integer-like after deref");
            }
            unsigned targetW = std::max(lhsIntTy.getWidth(), rhsIntTy.getWidth());
            Type targetTy    = rewriter.getIntegerType(targetW);
            if (lhsIntTy.getWidth() < targetW)
                lhsVal = arith::ExtSIOp::create(rewriter, loc, targetTy, lhsVal);
            if (rhsIntTy.getWidth() < targetW)
                rhsVal = arith::ExtSIOp::create(rewriter, loc, targetTy, rhsVal);
        }

        // Map starplat predicate to arith::CmpIPredicate.
        // Adjust the accessor (getPredicate / getCmpType / etc.) to match your ODS.
        StringRef predStr = mlir::cast<mlir::StringAttr>(op->getAttr("op")).getValue();

        arith::CmpIPredicate pred;
        if (predStr == "==")
            pred = arith::CmpIPredicate::eq;
        else if (predStr == "!=")
            pred = arith::CmpIPredicate::ne;
        else if (predStr == "<")
            pred = arith::CmpIPredicate::slt;
        else if (predStr == "<=")
            pred = arith::CmpIPredicate::sle;
        else if (predStr == ">")
            pred = arith::CmpIPredicate::sgt;
        else if (predStr == ">=")
            pred = arith::CmpIPredicate::sge;
        else {
            return rewriter.notifyMatchFailure(op, "unknown cmp predicate string: " + predStr);
        }

        Value cmp = arith::CmpIOp::create(rewriter, loc, pred, lhsVal, rhsVal);
        rewriter.replaceOp(op, cmp);
        return success();
    }
};

struct ConvertIsEdgeOp : public OpConversionPattern<mlir::starplat::IsEdgeOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::IsEdgeOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        auto loc                           = op.getLoc();
        auto graph                         = adaptor.getOperands()[0];
        auto node1                         = adaptor.getOperands()[1];
        auto node2                         = adaptor.getOperands()[2];

        auto node1id                       = LLVM::LoadOp::create(rewriter, loc, rewriter.getI32Type(), node1);
        auto node2id                       = LLVM::LoadOp::create(rewriter, loc, rewriter.getI32Type(), node2);

        mlir::MLIRContext* context         = getContext();
        auto module                        = op->getParentOfType<mlir::ModuleOp>();
        FailureOr<LLVM::LLVMFuncOp> nodeId = LLVM::lookupOrCreateFn(
            rewriter, module, "isEdge", {LLVM::LLVMPointerType::get(context), rewriter.getI32Type(), rewriter.getI32Type()}, rewriter.getI1Type());
        auto condOp = LLVM::CallOp::create(rewriter, loc, *nodeId, ValueRange{graph, node1id, node2id});
        // auto

        rewriter.replaceOp(op, condOp);
        return success();
    }
};

struct ConvertIncAndAssign : public OpConversionPattern<mlir::starplat::IncAndAssignOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::IncAndAssignOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {

        rewriter.eraseOp(op);
        return success();
    }
};

struct ConvertAdd : public OpConversionPattern<mlir::starplat::AddOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::AddOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        auto loc      = op.getLoc();
        auto operand1 = adaptor.getOperands()[0];
        // operand1.getType().dump();
        auto operand2 = adaptor.getOperands()[1];

        if (isa<LLVM::LLVMPointerType>(operand1.getType())) {
            auto loadOp1 = LLVM::LoadOp::create(rewriter, loc, rewriter.getI64Type(), operand1);
            operand1     = loadOp1.getResult();
        }

        if (isa<LLVM::LLVMPointerType>(operand2.getType())) {
            auto loadOp2 = LLVM::LoadOp::create(rewriter, loc, rewriter.getI64Type(), operand2);
            operand2     = loadOp2.getResult();
        }
        // operand2.getType().dump();

        auto addOp = LLVM::AddOp::create(rewriter, op.getLoc(), operand1, operand2);
        // adaptor.getOperand1();
        // auto addOp = LLVM::LoadOp::create(rewriter, op.getLoc(), rewriter.getI64Type(), adaptor.getOperands()[0]);
        //
        rewriter.replaceOp(op, addOp);

        // rewriter.eraseOp(op);
        return success();
    }
};

struct ConvertReturnOp : public OpConversionPattern<mlir::starplat::ReturnOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::ReturnOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {

        // auto retVal = LLVM::ConstantOp::create(rewriter, op.getLoc(), rewriter.getI32Type(), rewriter.getI32IntegerAttr(0));
        auto operand = adaptor.getOperands()[0];

        if (isa<LLVM::LLVMPointerType>(operand.getType())) {
            auto loadOp1 = LLVM::LoadOp::create(rewriter, op.getLoc(), rewriter.getI64Type(), operand);
            operand      = loadOp1.getResult();
        }
        LLVM::ReturnOp::create(rewriter, op.getLoc(), mlir::ValueRange({operand}));

        rewriter.eraseOp(op);

        return success();
    }
};

// struct ForallNodesOpLoweringGPU : public OpConversionPattern<starplat::ForAllNodesOp>
// {
//     using OpConversionPattern::OpConversionPattern;
//
//     // Block size (threads per block along thread_x). 256 is a sane default for
//     // vertex-parallel graph kernels; tune per-algorithm later if needed.
//     static constexpr int64_t kBlockSize = 256;
//
//     LogicalResult matchAndRewrite(starplat::ForAllNodesOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//         auto loc        = op.getLoc();
//         auto* ctx       = rewriter.getContext();
//         ModuleOp module = op->getParentOfType<ModuleOp>();
//
//         Value graphPtr  = adaptor.getOperands()[0]; // !llvm.ptr
//
//         auto i32Ty      = rewriter.getI32Type();
//         auto idxTy      = rewriter.getIndexType();
//         auto ptrTy      = LLVM::LLVMPointerType::get(ctx);
//
//         // --- N = get_num_nodes(graph) ------------------------------------------------
//         FailureOr<LLVM::LLVMFuncOp> getNumNodesFn = LLVM::lookupOrCreateFn(rewriter, module, "get_num_nodes", {ptrTy}, i32Ty);
//         if (failed(getNumNodesFn))
//             return failure();
//
//         Value nI32       = LLVM::CallOp::create(rewriter, loc, *getNumNodesFn, ValueRange{graphPtr}).getResult();
//         Value nIdx       = arith::IndexCastOp::create(rewriter, loc, idxTy, nI32);
//
//         Value zeroIdx    = arith::ConstantIndexOp::create(rewriter, loc, 0);
//         Value oneIdx     = arith::ConstantIndexOp::create(rewriter, loc, 1);
//         Value oneI32     = LLVM::ConstantOp::create(rewriter, loc, i32Ty, rewriter.getI32IntegerAttr(1));
//         Value blockSzIdx = arith::ConstantIndexOp::create(rewriter, loc, kBlockSize);
//
//         // numBlocks = ceildiv(N, kBlockSize)
//         Value numBlocksIdx = arith::CeilDivUIOp::create(rewriter, loc, nIdx, blockSzIdx);
//
//         // If we're already inside a parallel region (outer scf.parallel / scf.forall
//         // / gpu.launch), degrade to a sequential scf.for — same rule as the CPU path,
//         // just now it also prevents nested kernel launches.
//         bool nestedInParallel =
//             op->getParentOfType<scf::ParallelOp>() || op->getParentOfType<scf::ForallOp>() || op->getParentOfType<gpu::LaunchOp>();
//
//         // Per-iteration: cast IV index -> i32, alloca an i32 slot, store, return ptr.
//         // Identical to the CPU lowering — the body still sees a !llvm.ptr node.
//         auto materializeNodePtr = [&](OpBuilder& b, Location l, Value iv) -> Value
//         {
//             Value iI32 = arith::IndexCastOp::create(b, l, i32Ty, iv);
//             Value slot = LLVM::AllocaOp::create(b, l, ptrTy, i32Ty, oneI32,
//                                                 /*alignment=*/0);
//             LLVM::StoreOp::create(b, l, iI32, slot);
//             return slot;
//         };
//
//         Region& srcRegion = op.getRegion();
//         Block& srcBlock   = srcRegion.front();
//
//         if (nestedInParallel) {
//             // Sequential fallback — identical to CPU path.
//             auto forOp = scf::ForOp::create(rewriter, loc, zeroIdx, nIdx, oneIdx);
//
//             OpBuilder::InsertionGuard g(rewriter);
//             rewriter.setInsertionPointToStart(forOp.getBody());
//
//             Value nodePtr = materializeNodePtr(rewriter, loc, forOp.getInductionVar());
//
//             auto endOp    = cast<starplat::endOp>(srcBlock.getTerminator());
//             rewriter.eraseOp(endOp);
//
//             Block* dstBlock = forOp.getBody();
//             rewriter.inlineBlockBefore(&srcBlock, dstBlock->getTerminator(), ValueRange{nodePtr});
//         }
//         else {
//             // -----------------------------------------------------------------------
//             // GPU path: two nested scf.parallel loops tagged with #gpu.loop_dim_map
//             // so `-convert-parallel-loops-to-gpu` turns them into a gpu.launch with
//             // one block per tile and kBlockSize threads per block.
//             //
//             //   scf.parallel (b) in [0, numBlocks)        { mapping = block_x }
//             //     scf.parallel (t) in [0, kBlockSize)     { mapping = thread_x }
//             //       v = b * kBlockSize + t
//             //       if (v < N) { <body with nodePtr = &v> }
//             //
//             // This is the standard CUDA grid-stride tile. One logical parallel loop
//             // in StarPlat becomes two levels here purely because the hardware has a
//             // two-level execution hierarchy — the DSL doesn't need to know.
//             // -----------------------------------------------------------------------
//
//             // Outer (grid) loop over blocks.
//             auto blockLoop = scf::ParallelOp::create(rewriter, loc, ValueRange{zeroIdx}, ValueRange{numBlocksIdx}, ValueRange{oneIdx},
//                                                      /*initVals=*/ValueRange{});
//
//             // Attach #gpu.loop_dim_map<processor=block_x, map=(d0)->(d0), bound=(d0)->(d0)>
//             {
//                 auto identityMap  = AffineMap::getMultiDimIdentityMap(1, ctx);
//                 auto blockMapping = gpu::ParallelLoopDimMappingAttr::get(ctx, gpu::Processor::BlockX, identityMap, identityMap);
//                 if (failed(gpu::setMappingAttr(blockLoop, ArrayRef<gpu::ParallelLoopDimMappingAttr>{blockMapping})))
//                     return rewriter.notifyMatchFailure(op, "failed to set block_x mapping");
//             }
//
//             {
//                 OpBuilder::InsertionGuard gOuter(rewriter);
//                 rewriter.setInsertionPointToStart(blockLoop.getBody());
//
//                 // Inner (block) loop over threads.
//                 auto threadLoop    = scf::ParallelOp::create(rewriter, loc, ValueRange{zeroIdx}, ValueRange{blockSzIdx}, ValueRange{oneIdx},
//                                                              /*initVals=*/ValueRange{});
//
//                 auto identityMap   = AffineMap::getMultiDimIdentityMap(1, ctx);
//                 auto threadMapping = gpu::ParallelLoopDimMappingAttr::get(ctx, gpu::Processor::ThreadX, identityMap, identityMap);
//                 if (failed(gpu::setMappingAttr(threadLoop, ArrayRef<gpu::ParallelLoopDimMappingAttr>{threadMapping})))
//                     return rewriter.notifyMatchFailure(op, "failed to set thread_x mapping");
//
//                 OpBuilder::InsertionGuard gInner(rewriter);
//                 rewriter.setInsertionPointToStart(threadLoop.getBody());
//
//                 Value bIv = blockLoop.getInductionVars()[0];
//                 Value tIv = threadLoop.getInductionVars()[0];
//
//                 // v = b * kBlockSize + t
//                 Value bTimesBS = arith::MulIOp::create(rewriter, loc, bIv, blockSzIdx);
//                 Value vIdx     = arith::AddIOp::create(rewriter, loc, bTimesBS, tIv);
//
//                 // Tail guard: if (v < N) { body }
//                 // Needed because numBlocks * kBlockSize rounds up past N.
//                 Value inBounds = arith::CmpIOp::create(rewriter, loc, arith::CmpIPredicate::ult, vIdx, nIdx);
//
//                 auto ifOp      = scf::IfOp::create(rewriter, loc, inBounds,
//                                                    /*withElseRegion=*/false);
//
//                 OpBuilder::InsertionGuard gIf(rewriter);
//                 rewriter.setInsertionPointToStart(&ifOp.getThenRegion().front());
//
//                 Value nodePtr = materializeNodePtr(rewriter, loc, vIdx);
//
//                 auto endOp    = cast<starplat::endOp>(srcBlock.getTerminator());
//                 rewriter.eraseOp(endOp);
//
//                 // Splice the original body into the `then` block, before its yield.
//                 Block* thenBlock = &ifOp.getThenRegion().front();
//                 rewriter.inlineBlockBefore(&srcBlock, thenBlock->getTerminator(), ValueRange{nodePtr});
//             }
//         }
//
//         rewriter.eraseOp(op);
//         return success();
//     }
// };

struct ForAllNeighboursOpLoweringCPU : public OpConversionPattern<starplat::ForAllNeighboursOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(starplat::ForAllNeighboursOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        if (!isOutermostStarplatForall(op))
            return failure();
        Value graphPtr = adaptor.getOperands()[0];
        Value nodePtr  = adaptor.getOperands()[1];
        return lowerOutermostForall(op, rewriter, graphPtr, nodePtr);
    }
};

struct ConvertAttachOp : public OpConversionPattern<mlir::starplat::AttachNodePropertyOp>
{
    using OpConversionPattern::OpConversionPattern;
    LogicalResult matchAndRewrite(mlir::starplat::AttachNodePropertyOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        Location loc  = op.getLoc();
        auto operands = adaptor.getOperands();

        // Operands layout: %0 (graph/node handle), then pairs of (%dst_memref, %fill_val)
        // i.e. operands[1] filled with operands[2], operands[3] filled with operands[4], ...
        // Start at index 1, step by 2: dst at i, fill at i+1
        for (size_t i = 1; i + 1 < operands.size(); i += 2) {
            Value memref  = operands[i];     // destination memref<?xT>
            Value fillVal = operands[i + 1]; // fill value (may need widening/narrowing)

            auto memrefTy = dyn_cast<MemRefType>(memref.getType());
            if (!memrefTy)
                return rewriter.notifyMatchFailure(op, "expected memref type for destination operand");

            Type elemTy = memrefTy.getElementType();

            // Widen or narrow fill value to match memref element type if needed.
            // Common case: i1 fill into memref<?xi8> after type conversion widens.
            if (fillVal.getType() != elemTy) {
                auto fillIntTy = dyn_cast<IntegerType>(fillVal.getType());
                auto elemIntTy = dyn_cast<IntegerType>(elemTy);
                if (fillIntTy && elemIntTy) {
                    if (fillIntTy.getWidth() < elemIntTy.getWidth())
                        fillVal = arith::ExtUIOp::create(rewriter, loc, elemTy, fillVal);
                    else
                        fillVal = arith::TruncIOp::create(rewriter, loc, elemTy, fillVal);
                }
                else {
                    return rewriter.notifyMatchFailure(op, "attach fill value / memref element type mismatch (non-integer)");
                }
            }

            linalg::FillOp::create(rewriter, loc,
                                   /*inputs=*/ValueRange{fillVal},
                                   /*outputs=*/ValueRange{memref});
        }

        rewriter.eraseOp(op);
        return success();
    }
};

struct ConvertSetNodePropOp : public OpConversionPattern<mlir::starplat::SetNodePropertyOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::SetNodePropertyOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {

        auto loc        = op.getLoc();

        auto nodeval    = LLVM::LoadOp::create(rewriter, loc, rewriter.getI32Type(), adaptor.getOperands()[1]);

        auto index      = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), nodeval.getResult());

        Value storedVal = adaptor.getOperands()[3];
        Value memref    = adaptor.getOperands()[2];
        Type elemTy     = cast<MemRefType>(memref.getType()).getElementType();

        // Handle the i1 propNode -> memref<?xi8> widening case: the stored
        // value is still i1, widen it to i8 before the store.
        // For matching types (i64 -> memref<?xi64> etc.), store as-is.
        if (storedVal.getType() != elemTy) {
            if (storedVal.getType().isInteger(1) && elemTy.isInteger(8)) {
                storedVal = arith::ExtUIOp::create(rewriter, loc, elemTy, storedVal);
            }
            else {
                // Fall back to generic int ext/trunc for any other width mismatch.
                auto vTy = dyn_cast<IntegerType>(storedVal.getType());
                auto eTy = dyn_cast<IntegerType>(elemTy);
                if (!vTy || !eTy) {
                    return rewriter.notifyMatchFailure(op, "set-node-prop value / memref element type mismatch (non-integer)");
                }
                if (vTy.getWidth() < eTy.getWidth())
                    storedVal = arith::ExtUIOp::create(rewriter, loc, elemTy, storedVal);
                else
                    storedVal = arith::TruncIOp::create(rewriter, loc, elemTy, storedVal);
            }
        }

        auto storeop = memref::StoreOp::create(rewriter, loc, storedVal, memref, index.getResult());

        rewriter.replaceOp(op, storeop);

        return success();
    }
};

struct ConvertGetNodePropOp : public OpConversionPattern<mlir::starplat::GetNodePropertyOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::GetNodePropertyOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        Location loc = op.getLoc();

        // Operands after type conversion, by positional index.
        Value nodePtr    = adaptor.getOperands()[0]; // !llvm.ptr (to i32 index)
        Value propMemref = adaptor.getOperands()[1]; // memref<?xT> — T is converter-dependent

        // --- 1. Load the i32 index stored at the node pointer ---
        Value loadedIdxI32 = LLVM::LoadOp::create(rewriter, loc, rewriter.getI32Type(), nodePtr);

        // memref.load wants an `index`-typed subscript.
        Value idx = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), loadedIdxI32);

        // --- 2. Load propMemref[idx]. ---
        Value loaded = memref::LoadOp::create(rewriter, loc, propMemref, ValueRange{idx});

        // If the starplat-level result type was i1 and the memref element type
        // was widened to i8 by the type converter, narrow back via (!= 0).
        // In every other case (i64 distance arrays, etc.) just return the
        // loaded value directly — it already has the expected type.
        Type origResTy = op.getResult().getType();
        Type elemTy    = cast<MemRefType>(propMemref.getType()).getElementType();
        Value result   = loaded;

        if (origResTy.isInteger(1) && elemTy.isInteger(8)) {
            Value zeroI8 = arith::ConstantOp::create(rewriter, loc, rewriter.getI8Type(), rewriter.getI8IntegerAttr(0));
            result       = arith::CmpIOp::create(rewriter, loc, arith::CmpIPredicate::ne, loaded, zeroI8);
        }

        rewriter.replaceOp(op, result);
        return success();
    }
};

struct ConvertStoreOp : public OpConversionPattern<mlir::starplat::StoreOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::StoreOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        Location loc = op.getLoc();
        // MLIRContext* context = rewriter.getContext();

        Value dst    = adaptor.getOperands()[0]; // destination (converted)
        Value src    = adaptor.getOperands()[1]; // source (converted)

        auto dstType = dst.getType();
        auto srcType = src.getType();

        // ── Case 1: both memrefs → memref.copy ──────────────────────────────
        if (isa<MemRefType>(dstType) && isa<MemRefType>(srcType)) {
            // memref.copy(src, dst) — note argument order: source first, dest second.
            memref::CopyOp::create(rewriter, loc, src, dst);
            rewriter.eraseOp(op);
            return success();
        }

        // ── Case 2: dst is llvm.ptr (spint), src is an integer ──────────────
        // !starplat.spint lowered to !llvm.ptr (pointing at i64).
        // Materialise an i64 from whatever integer type src has, then store.
        if (isa<LLVM::LLVMPointerType>(dstType) && isa<IntegerType>(srcType)) {
            auto i64Ty         = rewriter.getI64Type();

            Value valueToStore = src;

            // Widen / narrow to i64 as needed.
            unsigned srcWidth = cast<IntegerType>(srcType).getWidth();
            if (srcWidth < 64) {
                valueToStore = arith::ExtSIOp::create(rewriter, loc, i64Ty, src);
            }
            else if (srcWidth > 64) {
                valueToStore = arith::TruncIOp::create(rewriter, loc, i64Ty, src);
            }
            // srcWidth == 64 → use as-is.

            LLVM::StoreOp::create(rewriter, loc, valueToStore, dst);
            rewriter.eraseOp(op);
            return success();
        }

        // ── Case 3: dst is llvm.ptr (spint), src is index ───────────────────
        // index can appear if the RHS was, e.g., a loop induction variable.
        if (isa<LLVM::LLVMPointerType>(dstType) && isa<IndexType>(srcType)) {
            auto i64Ty  = rewriter.getI64Type();
            Value asI64 = arith::IndexCastOp::create(rewriter, loc, i64Ty, src);
            LLVM::StoreOp::create(rewriter, loc, asI64, dst);
            rewriter.eraseOp(op);
            return success();
        }

        return rewriter.notifyMatchFailure(op, "unhandled StoreOp operand type combination after conversion");
    }
};

struct ConvertFixedPointUntil : public OpConversionPattern<mlir::starplat::FixedPointUntilOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::FixedPointUntilOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        Location loc = op.getLoc();

        Type i8Ty    = rewriter.getI8Type();
        Type idxTy   = rewriter.getIndexType();

        // propNode<i1,"g"> -> memref<?xi1>
        Value propArray = adaptor.getOperands()[1];

        // --- Build scf.while skeleton ---
        auto whileOp = scf::WhileOp::create(rewriter, loc, /*resultTypes=*/TypeRange{}, /*operands=*/ValueRange{});

        // ----- "before" region: body, then OR-reduce, then condition. -----
        Block* beforeBlock = rewriter.createBlock(&whileOp.getBefore());
        {
            OpBuilder::InsertionGuard g(rewriter);
            rewriter.setInsertionPointToStart(beforeBlock);

            // Inline the original op's region.
            Region& bodyRegion = op.getRegion();
            if (!bodyRegion.empty()) {
                Block& bodyBlock = bodyRegion.front();
                if (!bodyBlock.empty() && isa<mlir::starplat::endOp>(bodyBlock.back())) {
                    rewriter.eraseOp(&bodyBlock.back());
                }
                beforeBlock->getOperations().splice(beforeBlock->end(), bodyBlock.getOperations());
            }

            // --- OR-reduce propArray. ---
            Value cFalse  = arith::ConstantOp::create(rewriter, loc, i8Ty, rewriter.getI8IntegerAttr(0));
            Value zeroIdx = arith::ConstantOp::create(rewriter, loc, idxTy, rewriter.getIndexAttr(0));
            Value oneIdx  = arith::ConstantOp::create(rewriter, loc, idxTy, rewriter.getIndexAttr(1));
            Value len     = memref::DimOp::create(rewriter, loc, propArray, zeroIdx);

            // Check if we're already nested inside a parallel region.
            bool insideParallel = false;
            Operation* ancestor = whileOp->getParentOp();
            while (ancestor) {
                if (isa<scf::ParallelOp, scf::ForallOp>(ancestor)) {
                    insideParallel = true;
                    break;
                }
                ancestor = ancestor->getParentOp();
            }

            Value anySet;
            if (insideParallel) {
                // Sequential fallback: scf.for with an i1 iter_arg.
                // Load byte (i8) from propArray, narrow to i1 via (!= 0), then OR-accumulate.
                Value zeroI8 = arith::ConstantOp::create(rewriter, loc, rewriter.getI8Type(), rewriter.getI8IntegerAttr(0));
                auto reduce  = scf::ForOp::create(rewriter, loc, zeroIdx, len, oneIdx, ValueRange{cFalse},
                                                  [&](OpBuilder& b, Location l, Value iv, ValueRange iters)
                                                  {
                                                     Value acc  = iters[0];
                                                     Value byte = memref::LoadOp::create(b, l, propArray, ValueRange{iv});
                                                     Value bit  = arith::CmpIOp::create(b, l, arith::CmpIPredicate::ne, byte, zeroI8);
                                                     Value next = arith::OrIOp::create(b, l, acc, bit);
                                                     scf::YieldOp::create(b, l, ValueRange{next});
                                                 });
                anySet       = reduce.getResult(0);
            }
            else {
                // Parallel reduction via scf.parallel + scf.reduce.
                // Load byte (i8) from propArray, narrow to i1 via (!= 0), then contribute to the OR reduction.
                auto parallelOp = scf::ParallelOp::create(rewriter, loc,
                                                          /*lowerBounds=*/ValueRange{zeroIdx},
                                                          /*upperBounds=*/ValueRange{len},
                                                          /*steps=*/ValueRange{oneIdx},
                                                          /*initVals=*/ValueRange{cFalse},
                                                          [&](OpBuilder& b, Location l, ValueRange ivs, ValueRange /*initVals*/)
                                                          {
                                                              Value byte = memref::LoadOp::create(b, l, propArray, ivs);

                                                              // Emit scf.reduce contributing `bit`; its region
                                                              // defines the combiner (OR).
                                                              auto reduceOp = scf::ReduceOp::create(b, l, ValueRange{byte});

                                                              Block& rBlock = reduceOp.getReductions()[0].front();
                                                              OpBuilder::InsertionGuard g2(b);
                                                              b.setInsertionPointToStart(&rBlock);
                                                              Value lhs      = rBlock.getArgument(0);
                                                              Value rhs      = rBlock.getArgument(1);
                                                              Value combined = arith::OrIOp::create(b, l, lhs, rhs);
                                                              scf::ReduceReturnOp::create(b, l, combined);

                                                              // scf.parallel body has an implicit terminator after
                                                              // scf.reduce; nothing else to emit here.
                                                          });
                Value zeroI8    = arith::ConstantOp::create(rewriter, loc, rewriter.getI8Type(), rewriter.getI8IntegerAttr(0));
                anySet          = parallelOp.getResult(0);
                anySet          = arith::CmpIOp::create(rewriter, loc, arith::CmpIPredicate::ne, anySet, zeroI8);
            }

            scf::ConditionOp::create(rewriter, loc, anySet, ValueRange{});
        }

        // ----- "after" region: empty. -----
        Block* afterBlock = rewriter.createBlock(&whileOp.getAfter());
        {
            OpBuilder::InsertionGuard g(rewriter);
            rewriter.setInsertionPointToStart(afterBlock);
            scf::YieldOp::create(rewriter, loc, ValueRange{});
        }

        rewriter.eraseOp(op);
        return success();
    }
};

struct MinOpLowering : public OpConversionPattern<starplat::MinOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(starplat::MinOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        auto loc          = op.getLoc();

        auto operands     = adaptor.getOperands();
        Value nodeAPtr    = operands[0]; // llvm.ptr to i32
        Value propValArr  = operands[1]; // memref<?xi64>
        Value nodeBPtr    = operands[2]; // llvm.ptr to i32
        Value propFlagArr = operands[3]; // memref<?xi8>  (was i1 at starplat level; widened)
        Value candA       = operands[4]; // i64
        Value candB       = operands[5]; // i64
        if (isa<LLVM::LLVMPointerType>(candB.getType()))
            candB = LLVM::LoadOp::create(rewriter, loc, rewriter.getI64Type(), candB);
        Value flagVal = operands[6]; // i1

        auto i8Ty     = rewriter.getI8Type();
        auto i32Ty    = rewriter.getI32Type();
        auto i64Ty    = rewriter.getI64Type();
        auto idxTy    = rewriter.getIndexType();

        // A = *nodeAPtr ; B = *nodeBPtr
        Value aI32 = LLVM::LoadOp::create(rewriter, loc, i32Ty, nodeAPtr);
        Value bI32 = LLVM::LoadOp::create(rewriter, loc, i32Ty, nodeBPtr);
        Value aIdx = arith::IndexCastOp::create(rewriter, loc, idxTy, aI32);
        Value bIdx = arith::IndexCastOp::create(rewriter, loc, idxTy, bI32);

        // newVal = min(candA, candB) — signed
        Value newVal = arith::MinSIOp::create(rewriter, loc, candA, candB);

        // old = atomic { propValArr[A] = min(propValArr[A], newVal); yield old; }
        // memref.atomic_rmw returns the *pre-update* value.
        Value oldVal = memref::AtomicRMWOp::create(rewriter, loc, i64Ty, arith::AtomicRMWKind::mins, newVal, propValArr, ValueRange{aIdx});

        // changed = (oldVal != newVal)  — i.e. the atomic min actually moved the slot.
        Value changed = arith::CmpIOp::create(rewriter, loc, arith::CmpIPredicate::ne, oldVal, newVal);

        // if (changed) { atomic write flag[B] = flagVal (widened to i8); }
        // i1 is not a legal LLVM atomicrmw operand type, hence the ext to i8.
        scf::IfOp::create(rewriter, loc, changed,
                          /*thenBuilder=*/
                          [&](OpBuilder& b, Location l)
                          {
                              Value flagI8 = arith::ExtUIOp::create(b, l, i8Ty, flagVal);
                              memref::AtomicRMWOp::create(b, l, i8Ty, arith::AtomicRMWKind::assign, flagI8, propFlagArr, ValueRange{bIdx});
                              scf::YieldOp::create(b, l);
                          });

        rewriter.eraseOp(op);
        return success();
    }
};

struct ConvertGetEdgeProperty : public OpConversionPattern<mlir::starplat::GetEdgePropertyOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::GetEdgePropertyOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        Location loc         = op.getLoc();
        MLIRContext* context = rewriter.getContext();

        auto module          = op->getParentOfType<ModuleOp>();
        if (!module)
            return rewriter.notifyMatchFailure(op, "no enclosing module");

        // Read the property name off the op's attribute dictionary.
        auto propAttr = op->getAttrOfType<StringAttr>("property");
        if (!propAttr)
            return rewriter.notifyMatchFailure(op, "missing 'property' attribute");
        StringRef property = propAttr.getValue();

        // Pull the converted edge operand. The propEdge operand is intentionally ignored.
        Value edgePtr = adaptor.getOperands()[0]; // !llvm.ptr
        edgePtr       = LLVM::LoadOp::create(rewriter, loc, LLVM::LLVMPointerType::get(context), edgePtr);

        auto ptrTy    = LLVM::LLVMPointerType::get(context);
        auto i64Ty    = rewriter.getI64Type();

        if (property == "weight") {
            FailureOr<LLVM::LLVMFuncOp> getEdgeWeightFn = LLVM::lookupOrCreateFn(rewriter, module, "get_edge_weight", {ptrTy}, i64Ty);
            if (failed(getEdgeWeightFn))
                return rewriter.notifyMatchFailure(op, "failed to declare get_edge_weight");

            auto callOp = LLVM::CallOp::create(rewriter, loc, *getEdgeWeightFn, ValueRange{edgePtr});
            rewriter.replaceOp(op, callOp.getResult());
            return success();
        }

        return rewriter.notifyMatchFailure(op, "unknown edge property: " + property);
    }
};

struct ConvertNumNodesOp : public OpConversionPattern<mlir::starplat::NumNodesOp>
{
    using OpConversionPattern<mlir::starplat::NumNodesOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::NumNodesOp op, OpAdaptor adaptor,
                                  ConversionPatternRewriter& rewriter) const override {
        auto loc    = op.getLoc();
        auto ctx    = op.getContext();
        auto module = op->getParentOfType<mlir::ModuleOp>();

        auto ptrTy = LLVM::LLVMPointerType::get(ctx);
        auto i32Ty = rewriter.getI32Type();

        FailureOr<LLVM::LLVMFuncOp> getNumNodesFn =
            LLVM::lookupOrCreateFn(rewriter, module, "get_num_nodes", {ptrTy}, i32Ty);
        if (failed(getNumNodesFn))
            return rewriter.notifyMatchFailure(op, "failed to declare get_num_nodes");

        Value graphPtr = adaptor.getOperands()[0];
        Value nI32     = LLVM::CallOp::create(rewriter, loc, *getNumNodesFn, ValueRange{graphPtr}).getResult();

        auto resultType = op.getResult().getType();
        Value result;
        if (resultType == i32Ty) {
            result = nI32;
        } else if (auto intTy = dyn_cast<IntegerType>(resultType)) {
            if (intTy.getWidth() > 32)
                result = arith::ExtSIOp::create(rewriter, loc, resultType, nI32);
            else
                result = arith::TruncIOp::create(rewriter, loc, resultType, nI32);
        } else {
            return rewriter.notifyMatchFailure(op, "unsupported num_nodes result type");
        }

        rewriter.replaceOp(op, result);
        return success();
    }
};

struct ConvertCastOp : public OpConversionPattern<mlir::starplat::CastOp>
{
    using OpConversionPattern<mlir::starplat::CastOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::CastOp op, OpAdaptor adaptor,
                                  ConversionPatternRewriter& rewriter) const override {
        auto loc     = op.getLoc();
        Value input  = adaptor.getOperands()[0];
        auto srcType = input.getType();
        auto dstType = op.getResult().getType();

        if (srcType == dstType) {
            rewriter.replaceOp(op, input);
            return success();
        }

        bool srcIsInt   = isa<IntegerType>(srcType);
        bool srcIsFloat = isa<FloatType>(srcType);
        bool dstIsInt   = isa<IntegerType>(dstType);
        bool dstIsFloat = isa<FloatType>(dstType);

        Value result;
        if (srcIsInt && dstIsFloat) {
            result = arith::SIToFPOp::create(rewriter, loc, dstType, input);
        } else if (srcIsFloat && dstIsInt) {
            result = arith::FPToSIOp::create(rewriter, loc, dstType, input);
        } else if (srcIsInt && dstIsInt) {
            unsigned srcW = cast<IntegerType>(srcType).getWidth();
            unsigned dstW = cast<IntegerType>(dstType).getWidth();
            if (dstW > srcW)
                result = arith::ExtSIOp::create(rewriter, loc, dstType, input);
            else
                result = arith::TruncIOp::create(rewriter, loc, dstType, input);
        } else if (srcIsFloat && dstIsFloat) {
            unsigned srcW = cast<FloatType>(srcType).getWidth();
            unsigned dstW = cast<FloatType>(dstType).getWidth();
            if (dstW > srcW)
                result = arith::ExtFOp::create(rewriter, loc, dstType, input);
            else
                result = arith::TruncFOp::create(rewriter, loc, dstType, input);
        } else {
            return rewriter.notifyMatchFailure(op, "unsupported cast type combination");
        }

        rewriter.replaceOp(op, result);
        return success();
    }
};

namespace mlir
{
namespace starplat
{
#define GEN_PASS_DEF_CONVERTSTARPLATIRTOBASEPASS
#include "Passes.h.inc"

struct ConvertStarPlatIRToBasePass : public mlir::starplat::impl::ConvertStarPlatIRToBasePassBase<ConvertStarPlatIRToBasePass>
{
    using ConvertStarPlatIRToBasePassBase::ConvertStarPlatIRToBasePassBase;

    void getDependentDialects(mlir::DialectRegistry& registry) const override {
        registry.insert<mlir::gpu::GPUDialect>();
        registry.insert<mlir::linalg::LinalgDialect>();
        registry.insert<mlir::arith::ArithDialect>();
        registry.insert<mlir::memref::MemRefDialect>();
        registry.insert<mlir::func::FuncDialect>();
        registry.insert<mlir::scf::SCFDialect>();
        registry.insert<mlir::LLVM::LLVMDialect>();
    }

    void runOnOperation() override {

        // bool useGPU                = false;

        mlir::MLIRContext* context = &getContext();
        auto* module               = getOperation();

        if (!module)
            llvm::errs() << "Module not found!\n";

        if (!context)
            llvm::errs() << "Context not found!\n";

        ConversionTarget target(getContext());

        target.addLegalDialect<mlir::gpu::GPUDialect>();
        target.addLegalDialect<mlir::LLVM::LLVMDialect>();
        target.addLegalDialect<mlir::scf::SCFDialect>();
        target.addLegalDialect<mlir::memref::MemRefDialect>();
        target.addLegalDialect<mlir::func::FuncDialect>();
        target.addLegalDialect<mlir::arith::ArithDialect>();
        target.addLegalDialect<mlir::linalg::LinalgDialect>();

        target.addIllegalOp<mlir::starplat::StoreOp>();
        // target.addIllegalOp<mlir::starplat::FuncOp>();
        // target.addIllegalOp<mlir::starplat::DeclareOp2>();
        // target.addIllegalOp<mlir::starplat::AttachNodePropertyOp>();
        // target.addIllegalOp<mlir::starplat::ConstOp>();
        // target.addIllegalOp<mlir::starplat::AssignmentOp>();
        target.addIllegalOp<mlir::starplat::ForAllNodesOp>();
        target.addIllegalOp<mlir::starplat::NumNodesOp>();
        target.addIllegalOp<mlir::starplat::CastOp>();

        // target.addIllegalOp<mlir::starplat::SetNodePropertyOp>();
        // target.addIllegalOp<mlir::starplat::FixedPointUntilOp>();

        RewritePatternSet patterns(context);
        StarPlatTypeConverter typeConverter(context);
        // StarplatToLLVMTypeConverter typeConverter(context);

        patterns.add<ConvertFunc>(typeConverter, context);
        patterns.add<ConvertDeclareOp>(typeConverter, context);
        patterns.add<ConvertDeclareOp2>(typeConverter, context);
        patterns.add<ConvertConstOp>(typeConverter, context);
        patterns.add<ConvertAssignOp>(context);
        patterns.add<ConvertIfOp>(typeConverter, context);
        patterns.add<ForallNodesOpLoweringCPU>(typeConverter, context);
        patterns.add<ForAllNeighboursOpLoweringCPU>(typeConverter, context);
        patterns.add<ConvertNodeCmpOp>(typeConverter, context);
        patterns.add<ConvertIsEdgeOp>(typeConverter, context);
        patterns.add<ConvertAdd>(context);
        patterns.add<ConvertIncAndAssign>(context);
        patterns.add<ConvertAttachOp>(typeConverter, context);
        patterns.add<ConvertSetNodePropOp>(typeConverter, context);
        patterns.add<ConvertGetNodePropOp>(typeConverter, context);
        patterns.add<ConvertGetEdge>(typeConverter, context);
        patterns.add<ConvertGetEdgeProperty>(typeConverter, context);
        patterns.add<ConvertStoreOp>(typeConverter, context);
        patterns.add<ConvertFixedPointUntil>(typeConverter, context);
        patterns.add<MinOpLowering>(typeConverter, context);
        patterns.add<ConvertReturnOp>(typeConverter, context);
        patterns.add<ConvertNumNodesOp>(typeConverter, context);
        patterns.add<ConvertCastOp>(typeConverter, context);

        // populateFunctionOpInterfaceTypeConversionPattern<mlir::starplat::FuncOp>(patterns, typeConverter);
        // target.addDynamicallyLegalOp<mlir::starplat::FuncOp>([&](starplat::FuncOp op)
        //                                    {

        //     auto isSignatureLegal = typeConverter.isSignatureLegal(op.getFunctionType());
        //     auto isLegal = typeConverter.isLegal(&op.getBody());

        //     return isSignatureLegal && isLegal; });

        // module->dump();
        if (failed(applyPartialConversion(module, target, std::move(patterns)))) {
            signalPassFailure();
        }
    }
};
} // namespace starplat
} // namespace mlir

#endif
