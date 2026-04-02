#ifndef STARPLAT2OMP
#define STARPLAT2OMP

#include "includes/StarPlatOps.h"
#include "includes/StarPlatTypes.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/Types.h"
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
        auto llvmFuncTy =
            LLVM::LLVMFunctionType::get(convertedResultTypes.empty() ? LLVM::LLVMVoidType::get(rewriter.getContext()) : convertedResultTypes.front(),
                                        sigConversion.getConvertedTypes(), false);

        auto funcName = op.getSymName();
        //
        // // Create the LLVM function
        auto funcOp2 = LLVM::LLVMFuncOp::create(rewriter, loc, funcName, llvmFuncTy);
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

struct ConvertDeclareOp : public OpConversionPattern<mlir::starplat::DeclareOp2>
{
    ConvertDeclareOp(mlir::MLIRContext* context) : OpConversionPattern<mlir::starplat::DeclareOp2>(context) {}

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
        auto storeop = LLVM::StoreOp::create(rewriter, op.getLoc(), adaptor.getOperands()[1], adaptor.getOperands()[0]);

        rewriter.replaceOp(op, storeop);

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
        auto par = op->getParentOp();
        if (isa<scf::ForallOp>(par)) {
            // llvm::errs() << "hi\n";
        }
        auto module                = op->getParentOfType<mlir::ModuleOp>();
        mlir::MLIRContext* context = getContext();
        FailureOr<LLVM::LLVMFuncOp> graphCountNeighbours =
            LLVM::lookupOrCreateFn(rewriter, module, "get_num_neighbours", {LLVM::LLVMPointerType::get(context), LLVM::LLVMPointerType::get(context)},
                                   rewriter.getI64Type());
        FailureOr<LLVM::LLVMFuncOp> graphNeighbourArray =
            LLVM::lookupOrCreateFn(rewriter, module, "get_neighbours", {LLVM::LLVMPointerType::get(context), LLVM::LLVMPointerType::get(context)},
                                   LLVM::LLVMPointerType::get(context));
        auto num_neighbours =
            LLVM::CallOp::create(rewriter, loc, *graphCountNeighbours, ValueRange{adaptor.getOperands()[0], adaptor.getOperands()[1]});
        auto neighbour_array =
            LLVM::CallOp::create(rewriter, loc, *graphNeighbourArray, ValueRange{adaptor.getOperands()[0], adaptor.getOperands()[1]});
        // Value five        = arith::ConstantIndexOp::create(rewriter, loc, num_nodes);
        Value node_count = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), num_neighbours.getResult());
        auto forallOp    = scf::ForallOp::create(rewriter, loc, ArrayRef<OpFoldResult>{OpFoldResult(zero)}, // lower bounds
                                                 ArrayRef<OpFoldResult>{OpFoldResult(node_count)},          // uper bounds
                                                 ArrayRef<OpFoldResult>{OpFoldResult(one)},                 // steps
                                                 ValueRange{},                                              // shared outputs (none)
                                                 std::nullopt,                                              // mapping attr (none = no device mapping)
                                                 [&](OpBuilder& b, Location loc, ValueRange args)
                                                 {
                                                  mlir::Value iv      = args[0];
                                                  mlir::Value ivI64   = arith::IndexCastOp::create(b, loc, b.getI64Type(), iv);

                                                  mlir::Value elemPtr = LLVM::GEPOp::create(b, loc, LLVM::LLVMPointerType::get(context),
                                                                                               LLVM::LLVMPointerType::get(context),
                                                                                               neighbour_array.getResult(), ValueRange{ivI64});
                                                  mlir::Value node    = LLVM::LoadOp::create(b, loc, LLVM::LLVMPointerType::get(context), elemPtr);
                                                  LLVM::StoreOp::create(b, loc, node, adaptor.getOperands()[1]);

                                                  scf::InParallelOp::create(b, loc);
                                              });

        // auto nodes        = LLVM::CallOp::create(rewriter, loc, *graphAddEdgeFn, ValueRange{adaptor.getOperands()[0]});
        // auto storeop = LLVM::StoreOp::create(rewriter, op.getLoc(), adaptor.getOperands()[1], adaptor.getOperands()[0]);
        Block* forallBody = forallOp.getBody();
        rewriter.setInsertionPoint(forallBody->getTerminator());

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
            rewriter.moveOpBefore(&innerOp, forallBody->getTerminator());
        }

        rewriter.eraseOp(op);

        return success();
    }
};

struct ConvertForAllNodesOp : public OpConversionPattern<mlir::starplat::ForAllNodesOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::ForAllNodesOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
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
        auto module                = op->getParentOfType<mlir::ModuleOp>();
        mlir::MLIRContext* context = getContext();
        FailureOr<LLVM::LLVMFuncOp> graphCountNodes =
            LLVM::lookupOrCreateFn(rewriter, module, "get_num_nodes", {LLVM::LLVMPointerType::get(context)}, rewriter.getI64Type());
        FailureOr<LLVM::LLVMFuncOp> graphNodeArray =
            LLVM::lookupOrCreateFn(rewriter, module, "get_nodes", {LLVM::LLVMPointerType::get(context)}, LLVM::LLVMPointerType::get(context));
        auto num_nodes  = LLVM::CallOp::create(rewriter, loc, *graphCountNodes, ValueRange{adaptor.getOperands()[0]});
        auto node_array = LLVM::CallOp::create(rewriter, loc, *graphNodeArray, ValueRange{adaptor.getOperands()[0]});
        // Value five        = arith::ConstantIndexOp::create(rewriter, loc, num_nodes);
        Value node_count = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), num_nodes.getResult());
        auto forallOp    = scf::ForallOp::create(rewriter, loc, ArrayRef<OpFoldResult>{OpFoldResult(zero)}, // lower bounds
                                                 ArrayRef<OpFoldResult>{OpFoldResult(node_count)},          // uper bounds
                                                 ArrayRef<OpFoldResult>{OpFoldResult(one)},                 // steps
                                                 ValueRange{},                                              // shared outputs (none)
                                                 std::nullopt,                                              // mapping attr (none = no device mapping)
                                                 [&](OpBuilder& b, Location loc, ValueRange args)
                                                 {
                                                  mlir::Value iv      = args[0];
                                                  mlir::Value ivI64   = arith::IndexCastOp::create(b, loc, b.getI64Type(), iv);

                                                  mlir::Value elemPtr = LLVM::GEPOp::create(b, loc, LLVM::LLVMPointerType::get(context),
                                                                                               LLVM::LLVMPointerType::get(context),
                                                                                               node_array.getResult(), ValueRange{ivI64});
                                                  mlir::Value node    = LLVM::LoadOp::create(b, loc, LLVM::LLVMPointerType::get(context), elemPtr);
                                                  LLVM::StoreOp::create(b, loc, node, adaptor.getOperands()[1]);

                                                  scf::InParallelOp::create(b, loc);
                                              });

        // auto nodes        = LLVM::CallOp::create(rewriter, loc, *graphAddEdgeFn, ValueRange{adaptor.getOperands()[0]});
        // auto storeop = LLVM::StoreOp::create(rewriter, op.getLoc(), adaptor.getOperands()[1], adaptor.getOperands()[0]);
        Block* forallBody = forallOp.getBody();
        rewriter.setInsertionPoint(forallBody->getTerminator());

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
            rewriter.moveOpBefore(&innerOp, forallBody->getTerminator());
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

struct ConvertNodeCmpOp : public OpConversionPattern<mlir::starplat::NodeCmpOp>
{
    using OpConversionPattern::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::starplat::NodeCmpOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
        auto loc                   = op.getLoc();
        auto node1                 = adaptor.getOperands()[0];
        auto node2                 = adaptor.getOperands()[1];

        mlir::MLIRContext* context = getContext();
        auto module                = op->getParentOfType<mlir::ModuleOp>();
        FailureOr<LLVM::LLVMFuncOp> nodeId =
            LLVM::lookupOrCreateFn(rewriter, module, "get_node_id", {LLVM::LLVMPointerType::get(context)}, rewriter.getI64Type());
        auto node1id = LLVM::CallOp::create(rewriter, loc, *nodeId, ValueRange{node1});
        auto node2id = LLVM::CallOp::create(rewriter, loc, *nodeId, ValueRange{node2});
        // auto
        if (mlir::cast<mlir::StringAttr>(op->getAttr("op")).getValue() == "<") {
            auto compareOp = LLVM::ICmpOp::create(rewriter, loc, LLVM::ICmpPredicate::ult, node1id.getResult(), node2id.getResult());
            rewriter.replaceOp(op, compareOp);
        }
        else if (mlir::cast<mlir::StringAttr>(op->getAttr("op")).getValue() == ">") {
            auto compareOp = LLVM::ICmpOp::create(rewriter, loc, LLVM::ICmpPredicate::ugt, node1id.getResult(), node2id.getResult());
            rewriter.replaceOp(op, compareOp);
        }
        // rewriter.eraseOp(op);
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

        mlir::MLIRContext* context         = getContext();
        auto module                        = op->getParentOfType<mlir::ModuleOp>();
        FailureOr<LLVM::LLVMFuncOp> nodeId = LLVM::lookupOrCreateFn(
            rewriter, module, "checkEdge",
            {LLVM::LLVMPointerType::get(context), LLVM::LLVMPointerType::get(context), LLVM::LLVMPointerType::get(context)}, rewriter.getI1Type());
        auto condOp = LLVM::CallOp::create(rewriter, loc, *nodeId, ValueRange{graph, node1, node2});
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

namespace mlir
{
namespace starplat
{
#define GEN_PASS_DEF_CONVERTSTARPLATIRTOOMPPASS
#include "Passes.h.inc"

struct ConvertStarPlatIRToOMPPass : public mlir::starplat::impl::ConvertStarPlatIRToOMPPassBase<ConvertStarPlatIRToOMPPass>
{
    using ConvertStarPlatIRToOMPPassBase::ConvertStarPlatIRToOMPPassBase;

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

        // target.addIllegalOp<mlir::starplat::AddOp>();
        target.addIllegalOp<mlir::starplat::FuncOp>();
        target.addIllegalOp<mlir::starplat::DeclareOp2>();
        // target.addIllegalOp<mlir::starplat::AttachNodePropertyOp>();
        target.addIllegalOp<mlir::starplat::ConstOp>();
        target.addIllegalOp<mlir::starplat::AssignmentOp>();
        // target.addIllegalOp<mlir::starplat::SetNodePropertyOp>();
        // target.addIllegalOp<mlir::starplat::FixedPointUntilOp>();

        RewritePatternSet patterns(context);
        StarPlatTypeConverter typeConverter(context);
        // StarplatToLLVMTypeConverter typeConverter(context);

        // patterns.add<ConvertAdd>(context);
        patterns.add<ConvertFunc>(typeConverter, context);
        patterns.add<ConvertDeclareOp>(typeConverter, context);
        patterns.add<ConvertConstOp>(typeConverter, context);
        patterns.add<ConvertAssignOp>(context);
        patterns.add<ConvertForAllNodesOp>(typeConverter, context);
        patterns.add<ConvertForAllNeighboursOp>(typeConverter, context);
        patterns.add<ConvertIfOp>(typeConverter, context);
        patterns.add<ConvertNodeCmpOp>(typeConverter, context);
        patterns.add<ConvertIsEdgeOp>(typeConverter, context);
        patterns.add<ConvertAdd>(context);
        patterns.add<ConvertIncAndAssign>(context);
        patterns.add<ConvertReturnOp>(typeConverter, context);
        // patterns.add<ConvertDeclareOp>(typeConverter, context);

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
