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
            auto elemType      = rewriter.getI1Type();
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
        if (isa<starplat::SPIntType>(resType)) {
            auto loc                   = op->getLoc();
            mlir::MLIRContext* context = getContext();
            auto allocaop = LLVM::AllocaOp::create(rewriter, loc, LLVM::LLVMPointerType::get(context), LLVM::LLVMPointerType::get(context),
                                                   LLVM::ConstantOp::create(rewriter, loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1)), 0);
            rewriter.replaceOp(op, allocaop);
        }
        else if (isa<starplat::NodeType>(resType)) {
            auto loc                   = op->getLoc();
            mlir::MLIRContext* context = getContext();
            auto allocaop = LLVM::AllocaOp::create(rewriter, loc, LLVM::LLVMPointerType::get(context), LLVM::LLVMPointerType::get(context),
                                                   LLVM::ConstantOp::create(rewriter, loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1)), 0);
            rewriter.replaceOp(op, allocaop);
        }
        else if (isa<starplat::EdgeType>(resType)) {
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
            else if (attr.getValue() == "0") {
                auto newOp = LLVM::ConstantOp::create(rewriter, loc, op.getResult().getType(), 0);
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
        adaptor.getOperands()[0].dump();
        adaptor.getOperands()[1].dump();
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

struct ConvertForAllNeighboursOp : public OpConversionPattern<mlir::starplat::ForAllNeighboursOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::ForAllNeighboursOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        auto loc   = op.getLoc();
        Value zero = arith::ConstantIndexOp::create(rewriter, loc, 0);
        Value one  = arith::ConstantIndexOp::create(rewriter, loc, 1);
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
        // auto storeop = LLVM::StoreOp::create(rewriter, op.getLoc(), adaptor.getOperands()[1], adaptor.getOperands()[0]);
        //
        // auto par = op->getParentOp();
        // if (isa<scf::ForallOp>(par)) {
        //     llvm::errs() << "hi\n";
        // }
        auto module                                      = op->getParentOfType<mlir::ModuleOp>();
        auto parparent                                   = op->getParentOfType<mlir::scf::ParallelOp>();
        mlir::MLIRContext* context                       = getContext();
        FailureOr<LLVM::LLVMFuncOp> graphCountNeighbours = LLVM::lookupOrCreateFn(
            rewriter, module, "get_num_neighbours", {LLVM::LLVMPointerType::get(context), rewriter.getI32Type()}, rewriter.getI64Type());
        FailureOr<LLVM::LLVMFuncOp> graphNeighbourArray = LLVM::lookupOrCreateFn(
            rewriter, module, "get_neighbours", {LLVM::LLVMPointerType::get(context), rewriter.getI32Type()}, LLVM::LLVMPointerType::get(context));
        auto graph           = adaptor.getOperands()[0];
        auto node            = adaptor.getOperands()[1];
        auto nodeLoadOp      = LLVM::LoadOp::create(rewriter, loc, rewriter.getI32Type(), node);
        node                 = nodeLoadOp.getResult();
        auto num_neighbours  = LLVM::CallOp::create(rewriter, loc, *graphCountNeighbours, ValueRange{graph, node});
        auto neighbour_array = LLVM::CallOp::create(rewriter, loc, *graphNeighbourArray, ValueRange{graph, node});
        // Value five        = arith::ConstantIndexOp::create(rewriter, loc, num_nodes);
        Value node_count = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), num_neighbours.getResult());

        Block* body;
        if (parparent) {
            auto forOp = scf::ForOp::create(rewriter, loc, zero, node_count, one, {},
                                            [&](OpBuilder& b, Location loc, ValueRange args, ValueRange initVals)
                                            {
                                                mlir::Value iv    = args[0];
                                                mlir::Value ivI64 = arith::IndexCastOp::create(b, loc, b.getI64Type(), iv);

                                                mlir::Value elemPtr =
                                                    LLVM::GEPOp::create(b, loc, LLVM::LLVMPointerType::get(context), rewriter.getI32Type(),
                                                                        neighbour_array.getResult(), ValueRange{ivI64});
                                                mlir::Value nodeid = LLVM::LoadOp::create(b, loc, rewriter.getI32Type(), elemPtr);
                                                LLVM::StoreOp::create(b, loc, nodeid, adaptor.getOperands()[2]);

                                                scf::YieldOp::create(b, loc);
                                            });
            body       = forOp.getBody();
        }
        else {
            // parparent.dump();
            auto parallelOp = scf::ParallelOp::create(rewriter, loc, zero, // lower bounds
                                                      node_count,          // uper bounds
                                                      one,                 // steps
                                                      ValueRange{},        // shared outputs (none)
                                                      [&](OpBuilder& b, Location loc, ValueRange args, ValueRange initVals)
                                                      {
                                                          mlir::Value iv    = args[0];
                                                          mlir::Value ivI64 = arith::IndexCastOp::create(b, loc, b.getI64Type(), iv);

                                                          mlir::Value elemPtr =
                                                              LLVM::GEPOp::create(b, loc, LLVM::LLVMPointerType::get(context), rewriter.getI32Type(),
                                                                                  neighbour_array.getResult(), ValueRange{ivI64});
                                                          mlir::Value nodeid = LLVM::LoadOp::create(b, loc, rewriter.getI32Type(), elemPtr);
                                                          LLVM::StoreOp::create(b, loc, nodeid, adaptor.getOperands()[2]);

                                                          // scf::InParallelOp::create(b, loc);
                                                      });

            // auto nodes        = LLVM::CallOp::create(rewriter, loc, *graphAddEdgeFn, ValueRange{adaptor.getOperands()[0]});
            // auto storeop = LLVM::StoreOp::create(rewriter, op.getLoc(), adaptor.getOperands()[1], adaptor.getOperands()[0]);
            body = parallelOp.getBody();
        }

        rewriter.setInsertionPoint(body->getTerminator());

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
            rewriter.moveOpBefore(&innerOp, body->getTerminator());
        }

        rewriter.eraseOp(op);
        return success();
    }
};

struct ForallNodesOpLowering : public OpConversionPattern<starplat::ForAllNodesOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(starplat::ForAllNodesOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        auto loc        = op.getLoc();
        auto* ctx       = rewriter.getContext();
        ModuleOp module = op->getParentOfType<ModuleOp>();

        Value graphPtr  = adaptor.getOperands()[0]; // !llvm.ptr

        auto i32Ty      = rewriter.getI32Type();
        auto idxTy      = rewriter.getIndexType();
        auto ptrTy      = LLVM::LLVMPointerType::get(ctx);

        // --- Declare + call get_num_nodes(ptr) -> i32 ---------------------------
        FailureOr<LLVM::LLVMFuncOp> getNumNodesFn = LLVM::lookupOrCreateFn(rewriter, module, "get_num_nodes", {ptrTy}, i32Ty);
        if (failed(getNumNodesFn))
            return failure();

        Value nI32    = LLVM::CallOp::create(rewriter, loc, *getNumNodesFn, ValueRange{graphPtr}).getResult();
        Value nIdx    = arith::IndexCastOp::create(rewriter, loc, idxTy, nI32);

        Value zeroIdx = arith::ConstantIndexOp::create(rewriter, loc, 0);
        Value oneIdx  = arith::ConstantIndexOp::create(rewriter, loc, 1);
        Value oneI32  = LLVM::ConstantOp::create(rewriter, loc, i32Ty, rewriter.getI32IntegerAttr(1));

        // --- Nesting check ------------------------------------------------------
        bool nestedInParallel = op->getParentOfType<scf::ParallelOp>() || op->getParentOfType<scf::ForallOp>();

        // Helper: given the IV (index), materialize an llvm.ptr-to-i32 slot
        // holding the current index, to replace the starplat region's block arg.
        auto materializeNodePtr = [&](OpBuilder& b, Location l, Value iv) -> Value
        {
            Value iI32 = arith::IndexCastOp::create(b, l, i32Ty, iv);
            Value slot = LLVM::AllocaOp::create(b, l, ptrTy, i32Ty, oneI32,
                                                /*alignment=*/0);
            LLVM::StoreOp::create(b, l, iI32, slot);
            return slot;
        };

        // The body region of the starplat op. We'll splice its single block
        // into whichever loop we generate, dropping the starplat.end terminator.
        Region& srcRegion = op.getRegion();
        Block& srcBlock   = srcRegion.front();

        if (nestedInParallel) {
            // ---------------- scf.for (sequential) ------------------------------
            auto forOp = scf::ForOp::create(rewriter, loc, zeroIdx, nIdx, oneIdx);

            // Build the body.
            {
                OpBuilder::InsertionGuard g(rewriter);
                rewriter.setInsertionPointToStart(forOp.getBody());

                Value nodePtr = materializeNodePtr(rewriter, loc, forOp.getInductionVar());

                // Splice starplat body ops before the auto-inserted scf.yield.
                // Remove starplat.end from the source block first.
                auto endOp = cast<starplat::endOp>(srcBlock.getTerminator());
                rewriter.eraseOp(endOp);

                // Map srcBlock's single block arg (the !starplat.node ptr) to our slot.
                rewriter.mergeBlocks(&srcBlock, forOp.getBody(), ValueRange{nodePtr});
            }
        }
        else {
            // ---------------- scf.parallel --------------------------------------
            auto parOp = scf::ParallelOp::create(rewriter, loc, ValueRange{zeroIdx}, ValueRange{nIdx}, ValueRange{oneIdx},
                                                 /*initVals=*/ValueRange{});

            {
                OpBuilder::InsertionGuard g(rewriter);
                rewriter.setInsertionPointToStart(parOp.getBody());

                Value nodePtr = materializeNodePtr(rewriter, loc, parOp.getInductionVars()[0]);

                auto endOp    = cast<starplat::endOp>(srcBlock.getTerminator());
                rewriter.eraseOp(endOp);

                rewriter.mergeBlocks(&srcBlock, parOp.getBody(), ValueRange{nodePtr});
            }
        }

        rewriter.eraseOp(op);
        return success();
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

        // Look at the ORIGINAL op's operand types to decide what we're comparing.
        // (adaptor operands are already type-converted; op operands still carry
        // the original starplat types.)
        Type lhsOrigTy = op->getOperand(0).getType();
        Type rhsOrigTy = op->getOperand(1).getType();

        //
        // Type lhsOrigTy = lhs.getType();
        // Type rhsOrigTy = rhs.getType();

        bool lhsIsNode = isa<mlir::starplat::NodeType>(lhsOrigTy);
        bool rhsIsNode = isa<mlir::starplat::NodeType>(rhsOrigTy);

        // If either side is a node, dereference it to get the underlying i32 index.
        auto derefIfNode = [&](Value converted, bool isNode) -> Value
        {
            if (!isNode)
                return converted;
            return LLVM::LoadOp::create(rewriter, loc, rewriter.getI32Type(), converted);
        };

        Value lhsVal = derefIfNode(lhs, lhsIsNode);
        Value rhsVal = derefIfNode(rhs, rhsIsNode);

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
        auto loc      = op.getLoc();
        auto operand1 = adaptor.getOperands()[0];
        // operand1.getType().dump();
        auto operand2 = adaptor.getOperands()[1];

        // if (isa<LLVM::LLVMPointerType>(operand1.getType())) {
        //     auto loadOp1 = LLVM::LoadOp::create(rewriter, loc, rewriter.getI64Type(), operand1);
        //     operand1     = loadOp1.getResult();
        // }

        if (isa<LLVM::LLVMPointerType>(operand2.getType())) {
            auto loadOp2 = LLVM::LoadOp::create(rewriter, loc, rewriter.getI64Type(), operand2);
            operand2     = loadOp2.getResult();
        }
        // operand2.getType().dump();

        auto addOp = LLVM::AtomicRMWOp::create(rewriter, op.getLoc(), LLVM::AtomicBinOp::add, operand1, operand2, LLVM::AtomicOrdering::monotonic);
        // adaptor.getOperand1();
        // auto addOp = LLVM::LoadOp::create(rewriter, op.getLoc(), rewriter.getI64Type(), adaptor.getOperands()[0]);
        //
        rewriter.replaceOp(op, addOp);

        // rewriter.eraseOp(op);
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

struct ConvertAttachOp : public OpConversionPattern<mlir::starplat::AttachNodePropertyOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::AttachNodePropertyOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {

        // auto retVal = LLVM::ConstantOp::create(rewriter, op.getLoc(), rewriter.getI32Type(), rewriter.getI32IntegerAttr(0));
        // auto operand = adaptor.getOperands()[0];
        //
        // if (isa<LLVM::LLVMPointerType>(operand.getType())) {
        //     auto loadOp1 = LLVM::LoadOp::create(rewriter, op.getLoc(), rewriter.getI64Type(), operand);
        //     operand      = loadOp1.getResult();
        // }
        //
        // LLVM::ReturnOp::create(rewriter, op.getLoc(), mlir::ValueRange({operand}));

        // auto loc = op.getLoc();
        //
        // auto fillop = linalg::FillOp::create(rewriter, loc, ValueRange{adaptor.getOperands()[2]}, ValueRange{op.getOperands()[1]});
        //
        // rewriter.replaceOp(op, fillop);
        Location loc = op.getLoc();

        // 1. Get the converted memref operand via adaptor
        Value memref = adaptor.getOperands()[1]; // memref<?x!starplat.spint>

        // 2. Get the fill value — cast it to the memref's element type if needed
        Value fillVal = adaptor.getOperands()[2]; // the i64 constant in your case
                                                  //

        // Type elemTy = cast<MemRefType>(memref.getType()).getElementType();
        //
        // // 3. Cast the fill value to match element type if they differ
        // //    e.g. your constant is i64 but elemTy might be i32/spint
        // if (fillVal.getType() != elemTy) {
        //     fillVal = arith::TruncIOp::create(rewriter, loc, elemTy, fillVal);
        //     // use ExtSIOp/ExtUIOp if upcasting, TruncIOp if downcasting
        //     // use FPToSIOp/SIToFPOp if crossing int<->float boundary
        // }

        // 4. Create linalg.fill
        linalg::FillOp::create(rewriter, loc,
                               /*inputs=*/ValueRange{fillVal},
                               /*outputs=*/ValueRange{memref});

        // 5. Erase the original op
        rewriter.eraseOp(op);
        return success();
    }
};

struct ConvertSetNodePropOp : public OpConversionPattern<mlir::starplat::SetNodePropertyOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::SetNodePropertyOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {

        auto loc     = op.getLoc();

        auto nodeval = LLVM::LoadOp::create(rewriter, loc, rewriter.getI32Type(), adaptor.getOperands()[1]);

        auto index   = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), nodeval.getResult());

        auto storeop = memref::StoreOp::create(rewriter, loc, adaptor.getOperands()[3], adaptor.getOperands()[2], index.getResult());

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
        Value propMemref = adaptor.getOperands()[1]; // memref<?xi1>

        // --- 1. Load the i32 index stored at the node pointer ---
        Value loadedIdxI32 = LLVM::LoadOp::create(rewriter, loc, rewriter.getI32Type(), nodePtr);

        // memref.load wants an `index`-typed subscript.
        Value idx = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), loadedIdxI32);

        // --- 2. Load propMemref[idx]; result is i1. ---
        Value loadedBit = memref::LoadOp::create(rewriter, loc, propMemref, ValueRange{idx});

        rewriter.replaceOp(op, loadedBit);
        return success();
    }
};

struct ConvertStoreOp : public OpConversionPattern<mlir::starplat::StoreOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::StoreOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        Location loc = op.getLoc();

        // After type conversion, both operands are memref<?xi1> (or similar).
        Value dst = adaptor.getOperands()[0]; // destination propNode
        Value src = adaptor.getOperands()[1]; // source propNode

        // Sanity: both must be memrefs.
        if (!isa<MemRefType>(dst.getType()) || !isa<MemRefType>(src.getType())) {
            return rewriter.notifyMatchFailure(op, "expected both operands to be memref after conversion");
        }

        // memref.copy semantics: copy src into dst, elementwise.
        // Sizes must match at runtime (you've guaranteed this).
        memref::CopyOp::create(rewriter, loc, src, dst);

        rewriter.eraseOp(op);
        return success();
    }
};

struct ConvertFixedPointUntil : public OpConversionPattern<mlir::starplat::FixedPointUntilOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::FixedPointUntilOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        Location loc = op.getLoc();

        Type i1Ty    = rewriter.getI1Type();
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
            Value cFalse  = arith::ConstantOp::create(rewriter, loc, i1Ty, rewriter.getBoolAttr(false));
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
                auto reduce = scf::ForOp::create(rewriter, loc, zeroIdx, len, oneIdx, ValueRange{cFalse},
                                                 [&](OpBuilder& b, Location l, Value iv, ValueRange iters)
                                                 {
                                                     Value acc  = iters[0];
                                                     Value bit  = memref::LoadOp::create(b, l, propArray, ValueRange{iv});
                                                     Value next = arith::OrIOp::create(b, l, acc, bit);
                                                     scf::YieldOp::create(b, l, ValueRange{next});
                                                 });
                anySet      = reduce.getResult(0);
            }
            else {
                // Parallel reduction via scf.parallel + scf.reduce.
                auto parallelOp = scf::ParallelOp::create(rewriter, loc,
                                                          /*lowerBounds=*/ValueRange{zeroIdx},
                                                          /*upperBounds=*/ValueRange{len},
                                                          /*steps=*/ValueRange{oneIdx},
                                                          /*initVals=*/ValueRange{cFalse},
                                                          [&](OpBuilder& b, Location l, ValueRange ivs, ValueRange /*initVals*/)
                                                          {
                                                              Value bit = memref::LoadOp::create(b, l, propArray, ivs);

                                                              // Emit scf.reduce contributing `bit`; its region
                                                              // defines the combiner (OR).
                                                              auto reduceOp = scf::ReduceOp::create(b, l, ValueRange{bit});

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
                anySet          = parallelOp.getResult(0);
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
        Value propFlagArr = operands[3]; // memref<?xi1>
        Value candA       = operands[4]; // i64
        Value candB       = operands[5]; // i64
        Value flagVal     = operands[6]; // i1

        auto i32Ty        = rewriter.getI32Type();
        auto i64Ty        = rewriter.getI64Type();
        auto idxTy        = rewriter.getIndexType();

        // A = *nodeAPtr ; B = *nodeBPtr
        Value aI32 = LLVM::LoadOp::create(rewriter, loc, i32Ty, nodeAPtr);
        Value bI32 = LLVM::LoadOp::create(rewriter, loc, i32Ty, nodeBPtr);
        Value aIdx = arith::IndexCastOp::create(rewriter, loc, idxTy, aI32);
        Value bIdx = arith::IndexCastOp::create(rewriter, loc, idxTy, bI32);

        // newVal = min(candA, candB) — signed
        Value newVal = arith::MinSIOp::create(rewriter, loc, candA, candB);

        // old = atomic { propValArr[A] = min(propValArr[A], newVal); yield old; }
        // memref.atomic_rmw returns the *pre-update* value.
        Value oldVal = memref::AtomicRMWOp::create(rewriter, loc, i64Ty,
                                                   arith::AtomicRMWKind::mins, // name varies by LLVM version; see note
                                                   newVal, propValArr, ValueRange{aIdx});

        // changed = (oldVal != newVal)  — i.e. the atomic min actually moved the slot.
        Value changed = arith::CmpIOp::create(rewriter, loc, arith::CmpIPredicate::ne, oldVal, newVal);

        // if (changed) { atomic write flag[B] = flagVal; }
        // Use scf.if with no results. Inside, do an atomic_rmw "assign" so the
        // write is well-defined under concurrent execution (OMP / GPU).
        scf::IfOp::create(rewriter, loc, changed,
                          /*thenBuilder=*/
                          [&](OpBuilder& b, Location l)
                          {
                              memref::AtomicRMWOp::create(b, l, b.getI1Type(), arith::AtomicRMWKind::assign, flagVal, propFlagArr, ValueRange{bIdx});
                              scf::YieldOp::create(b, l);
                          });

        rewriter.eraseOp(op);
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
        registry.insert<mlir::linalg::LinalgDialect>();
        registry.insert<mlir::arith::ArithDialect>();
        registry.insert<mlir::memref::MemRefDialect>();
        registry.insert<mlir::func::FuncDialect>();
        registry.insert<mlir::scf::SCFDialect>();
        registry.insert<mlir::LLVM::LLVMDialect>();
    }

    void runOnOperation() override {

        mlir::MLIRContext* context = &getContext();
        auto* module               = getOperation();

        if (!module)
            llvm::errs() << "Module not found!\n";

        if (!context)
            llvm::errs() << "Context not found!\n";

        ConversionTarget target(getContext());

        target.addLegalDialect<mlir::LLVM::LLVMDialect>();
        target.addLegalDialect<mlir::scf::SCFDialect>();
        target.addLegalDialect<mlir::memref::MemRefDialect>();
        target.addLegalDialect<mlir::func::FuncDialect>();
        target.addLegalDialect<mlir::arith::ArithDialect>();
        target.addLegalDialect<mlir::linalg::LinalgDialect>();

        // target.addIllegalOp<mlir::starplat::AddOp>();
        // target.addIllegalOp<mlir::starplat::FuncOp>();
        // target.addIllegalOp<mlir::starplat::DeclareOp2>();
        // target.addIllegalOp<mlir::starplat::AttachNodePropertyOp>();
        // target.addIllegalOp<mlir::starplat::ConstOp>();
        // target.addIllegalOp<mlir::starplat::AssignmentOp>();
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
        patterns.add<ForallNodesOpLowering>(typeConverter, context);
        patterns.add<ConvertForAllNeighboursOp>(typeConverter, context);
        patterns.add<ConvertIfOp>(typeConverter, context);
        patterns.add<ConvertNodeCmpOp>(typeConverter, context);
        patterns.add<ConvertIsEdgeOp>(typeConverter, context);
        patterns.add<ConvertAdd>(context);
        patterns.add<ConvertIncAndAssign>(context);
        patterns.add<ConvertAttachOp>(typeConverter, context);
        patterns.add<ConvertSetNodePropOp>(typeConverter, context);
        patterns.add<ConvertGetNodePropOp>(typeConverter, context);
        patterns.add<ConvertGetEdge>(typeConverter, context);
        patterns.add<ConvertStoreOp>(typeConverter, context);
        patterns.add<ConvertFixedPointUntil>(typeConverter, context);
        patterns.add<MinOpLowering>(typeConverter, context);
        patterns.add<ConvertReturnOp>(typeConverter, context);

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
