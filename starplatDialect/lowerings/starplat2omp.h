// #ifndef STARPLAT2OMP
// #define STARPLAT2OMP
//
// #include "includes/StarPlatOps.h"
// #include "mlir/Conversion/Passes.h"
// #include "mlir/Pass/PassManager.h"
//
// #include "includes/StarPlatDialect.h"
// #include "mlir/Dialect/Arith/IR/Arith.h"
// #include "mlir/Dialect/Func/IR/FuncOps.h"
// #include "mlir/Dialect/LLVMIR/LLVMDialect.h"
// #include "mlir/Dialect/MemRef/IR/MemRef.h"
// #include "mlir/Dialect/OpenMP/OpenMPDialect.h"
// #include "mlir/Dialect/SCF/IR/SCF.h"
//
// #include "mlir/IR/BuiltinAttributes.h"
// #include "mlir/IR/BuiltinDialect.h"
// #include "mlir/IR/BuiltinOps.h"
// #include "mlir/IR/BuiltinTypes.h"
// #include "mlir/IR/Diagnostics.h"
// #include "mlir/IR/DialectRegistry.h"
// #include "mlir/IR/PatternMatch.h"
// #include "mlir/IR/ValueRange.h"
// #include "mlir/Support/LLVM.h"
// #include "mlir/Support/LogicalResult.h"
// #include "mlir/Support/TypeID.h"
// #include "llvm/Support/Casting.h"
// #include "llvm/Support/LogicalResult.h"
//
// #include "mlir/Transforms/DialectConversion.h" // from @llvm-project
//
// // mlir::Operation *generateGraphStruct(mlir::MLIRContext *context)
// // {
// //     auto structType = LLVM::LLVMStructType::getIdentified(context, "Graph");
//
// //     return;
// // }
//
// mlir::LLVM::LLVMStructType createGraphStruct(mlir::MLIRContext* context);
// mlir::LLVM::LLVMStructType createNodeStruct(mlir::IRRewriter* rewriter, mlir::MLIRContext* context);
// using namespace mlir;
//
// class StarplatToLLVMTypeConverter : public TypeConverter
// {
//   public:
//     StarplatToLLVMTypeConverter(MLIRContext* ctx) {
//
//         // addConversion([](Type type)
//         //               { return type; });
//
//         addConversion([ctx](mlir::starplat::GraphType graph) -> Type { return createGraphStruct(ctx); });
//
//         addTargetMaterialization(
//             [](OpBuilder& builder, Type type, ValueRange inputs, Location loc) -> Value
//             {
//                 if (isa<mlir::starplat::GraphType>(type) && inputs.size() == 1 && isa<MemRefType>(inputs[0].getType())) {
//                     return inputs[0]; // just forward the memref
//                 }
//                 return nullptr;
//             });
//
//         addConversion([ctx](mlir::starplat::NodeType node) -> Type { return mlir::IntegerType::get(ctx, 64); });
//
//         addConversion([ctx](mlir::IntegerType intType) -> Type { return LLVM::LLVMPointerType::get(ctx); });
//
//         addConversion([ctx](mlir::starplat::PropNodeType intType) -> Type { return LLVM::LLVMPointerType::get(ctx); });
//     }
// };
//
// struct ConvertFunc : public OpConversionPattern<mlir::starplat::FuncOp>
// {
//     using OpConversionPattern<mlir::starplat::FuncOp>::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(mlir::starplat::FuncOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//         llvm::errs() << "I came here!!\n";
//         auto loc        = op.getLoc();
//
//         auto returnType = LLVM::LLVMPointerType::get(op.getContext());
//         // auto returnType =  MemRefType::get({mlir::ShapedType::kDynamic}, rewriter.getI1Type());
//
//         // auto returnType = rewriter.getI64Type(); // up up
//
//         // auto i64Type = rewriter.getI64Type();
//
//         // Function signature: (i32, i32) -> i32
//         auto oldFuncType = op.getFunctionType();
//
//         // SmallVector<Type> paramTypes = {oldFuncType.getInput(0), oldFuncType.getInput(1), oldFuncType.getInput(2), oldFuncType.getInput(3)};
//         SmallVector<Type> paramTypes = {oldFuncType.getInput(0)};
//
//         auto funcType                = mlir::FunctionType::get(rewriter.getContext(), paramTypes, returnType);
//         auto funcName                = op.getSymName();
//
//         // Create the LLVM function
//         auto funcOp2 = mlir::func::FuncOp::create(rewriter, loc, funcName, funcType);
//         // auto funcOp = LLVM::LLVMFuncOp::create(rewriter,loc, funcName, funcType);
//
//         // Create an entry block with the right number of arguments
//         // Block *entryBlock = rewriter.createBlock(&funcOp.getBody(), {}, paramTypes, {loc, loc});
//
//         rewriter.inlineRegionBefore(op.getRegion(), funcOp2.getBody(), funcOp2.end());
//         rewriter.eraseOp(op);
//
//         // Replace original starplat.func
//         return success();
//     }
// };
//
// struct ConvertAttachNode : public OpConversionPattern<mlir::starplat::AttachNodePropertyOp>
// {
//     using OpConversionPattern::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(mlir::starplat::AttachNodePropertyOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//
//         // "starplat.attachNodeProperty"(%arg0, %0, %1) : (!starplat.graph, !starplat.propNode<i32, "g">, i32) -> ()}
//         // Get num of nodes from arg0 done
//         // Loop from 0 to num of nodes  done
//         // Assign %0[index] = %1
//
//         auto loc        = op.getLoc();
//
//         auto constZero  = LLVM::ConstantOp::create(rewriter, loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(0));
//         auto numOfNodes = LLVM::ExtractValueOp::create(rewriter, loc, mlir::IntegerType::get(op.getContext(), 64), adaptor.getOperands()[0],
//                                                        rewriter.getDenseI64ArrayAttr({0}));
//         auto step       = LLVM::ConstantOp::create(rewriter, loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1));
//
//         mlir::scf::ForOp::create(rewriter, loc, constZero, numOfNodes, step, mlir::ValueRange{},
//                                  [&](mlir::OpBuilder& builder, mlir::Location nestedLoc, mlir::Value iv, mlir::ValueRange iterArgs)
//                                  {
//                                      auto indexIv = arith::IndexCastOp::create(builder, nestedLoc, builder.getIndexType(), iv);
//
//                                      mlir::memref::StoreOp::create(builder, nestedLoc, adaptor.getOperands()[2], adaptor.getOperands()[1],
//                                                                    mlir::ValueRange{indexIv});
//                                      mlir::scf::YieldOp::create(builder, nestedLoc);
//                                  });
//
//         // auto algnIdx = mlir::memref::ExtractAlignedPointerAsIndexOp::create(rewriter,op.getLoc(),adaptor.getOperands()[1]);
//         // auto algnPtrToInt = arith::IndexCastOp::create(rewriter,op.getLoc(), rewriter.getI64Type(),  algnIdx);
//         // auto IdxtoPtr = LLVM::IntToPtrOp::create(rewriter,op.getLoc(),LLVM::LLVMPointerType::get(op.getContext()), algnPtrToInt);
//
//         rewriter.eraseOp(op);
//
//         return success();
//     }
// };
//
// struct ConvertSetNodeProp : public OpConversionPattern<mlir::starplat::SetNodePropertyOp>
// {
//     using OpConversionPattern::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(mlir::starplat::SetNodePropertyOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//
//         // Check if the node id is greater than the total number of nodes using operand(0) & operand(1)
//         // If no, then assign operand(3) to operand(2)[operand(1)]
//         auto numOfNodes = LLVM::ExtractValueOp::create(rewriter, op.getLoc(), mlir::IntegerType::get(op.getContext(), 64),
//         adaptor.getOperands()[0],
//                                                        rewriter.getDenseI64ArrayAttr({0}));
//
//         auto cond = mlir::arith::CmpIOp::create(rewriter, op.getLoc(), rewriter.getI1Type(), mlir::arith::CmpIPredicate::sgt,
//         numOfNodes.getResult(),
//                                                 adaptor.getOperands()[1]);
//
//         mlir::scf::IfOp::create(rewriter, op.getLoc(), cond,
//                                 [&](mlir::OpBuilder& builder, mlir::Location nestedLoc)
//                                 {
//                                     auto index = arith::IndexCastOp::create(builder, nestedLoc, builder.getIndexType(), adaptor.getOperands()[1]);
//
//                                     mlir::memref::StoreOp::create(builder, nestedLoc, adaptor.getOperands()[3], adaptor.getOperands()[2],
//                                                                   mlir::ValueRange{index});
//                                     mlir::scf::YieldOp::create(builder, nestedLoc);
//                                 });
//
//         auto algnIdx = mlir::memref::ExtractAlignedPointerAsIndexOp::create(rewriter, op.getLoc(), adaptor.getOperands()[2]);
//         // auto algnPtrToInt =
//         arith::IndexCastOp::create(rewriter, op.getLoc(), rewriter.getI64Type(), algnIdx);
//         // auto IdxtoPtr = LLVM::IntToPtrOp::create(rewriter,op.getLoc(),LLVM::LLVMPointerType::get(op.getContext()), algnPtrToInt);
//
//         // func::ReturnOp::create(rewriter,op.getLoc(), IdxtoPtr.getResult());
//
//         rewriter.eraseOp(op);
//
//         return success();
//     }
// };
// struct ConvertReturnOp : public OpConversionPattern<mlir::starplat::ReturnOp>
// {
//     using OpConversionPattern::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(mlir::starplat::ReturnOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//
//         // auto retVal = LLVM::ConstantOp::create(rewriter,op.getLoc(), rewriter.getI32Type(), rewriter.getI32IntegerAttr(0));
//         // LLVM::ReturnOp::create(rewriter,op.getLoc(), retVal);
//
//         rewriter.eraseOp(op);
//
//         return success();
//     }
// };
//
// struct ConvertAssignOp : public OpConversionPattern<mlir::starplat::AssignmentOp>
// {
//     using OpConversionPattern::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(mlir::starplat::AssignmentOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//
//         auto assign = mlir::memref::StoreOp::create(rewriter, op.getLoc(), adaptor.getOperands()[1], adaptor.getOperands()[0]);
//         rewriter.replaceOp(op, assign);
//
//         auto algnIdx = mlir::memref::ExtractAlignedPointerAsIndexOp::create(rewriter, op.getLoc(), adaptor.getOperands()[0]);
//         // auto algnPtrToInt =
//         arith::IndexCastOp::create(rewriter, op.getLoc(), rewriter.getI64Type(), algnIdx);
//         // auto IdxtoPtr = LLVM::IntToPtrOp::create(rewriter,op.getLoc(),LLVM::LLVMPointerType::get(op.getContext()), algnPtrToInt);
//         // func::ReturnOp::create(rewriter,op.getLoc(), IdxtoPtr.getResult());
//
//         return success();
//     }
// };
//
// struct ConvertDeclareOp : public OpConversionPattern<mlir::starplat::DeclareOp>
// {
//     ConvertDeclareOp(mlir::MLIRContext* context) : OpConversionPattern<mlir::starplat::DeclareOp>(context) {}
//
//     using OpConversionPattern<mlir::starplat::DeclareOp>::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(mlir::starplat::DeclareOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//
//         auto resType = op->getResult(0).getType();
//         if (isa<mlir::starplat::PropNodeType>(resType)) {
//             auto loc          = op->getLoc();
//
//             auto rescast      = dyn_cast<mlir::starplat::PropNodeType>(resType);
//
//             auto field0       = LLVM::ExtractValueOp::create(rewriter, loc, mlir::IntegerType::get(op.getContext(), 64), adaptor.getOperands()[0],
//                                                              rewriter.getDenseI64ArrayAttr({0}));
//             Value dynamicSize = arith::IndexCastOp::create(rewriter, loc, rewriter.getIndexType(), field0);
//
//             MemRefType memrefType;
//             if (rescast.getParameter() == mlir::IntegerType::get(rewriter.getContext(), 1))
//                 memrefType = MemRefType::get({mlir::ShapedType::kDynamic}, rewriter.getI1Type());
//
//             else if (rescast.getParameter() == mlir::IntegerType::get(rewriter.getContext(), 64))
//                 memrefType = MemRefType::get({mlir::ShapedType::kDynamic}, rewriter.getI64Type());
//
//             else {
//                 llvm::outs() << "Error: MemrefType not implemented\n";
//                 exit(0);
//             }
//             Value allocated = memref::AllocOp::create(rewriter, loc, memrefType, mlir::ValueRange({dynamicSize}));
//
//             rewriter.replaceOp(op, allocated);
//         }
//
//         else if (isa<mlir::IntegerType>(resType)) {
//             auto loc     = op->getLoc();
//             auto resCast = dyn_cast<mlir::IntegerType>(resType);
//
//             MemRefType memrefType;
//
//             if (resCast.getWidth() == 64)
//                 memrefType = MemRefType::get({}, rewriter.getI64Type());
//             else if (resCast.getWidth() == 1)
//                 memrefType = MemRefType::get({}, rewriter.getI1Type());
//
//             else {
//                 llvm::outs() << "Error: MemrefType Integer type not implemented.\n";
//                 exit(0);
//             }
//
//             Value allocated = memref::AllocOp::create(rewriter, loc, memrefType);
//             rewriter.replaceOp(op, allocated);
//         }
//
//         else if (isa<mlir::starplat::NodeType>(resType)) {
//             llvm::outs() << "Hello\n";
//         }
//
//         else {
//             llvm::outs() << "Error: This DeclareOp lowering not yet implemented.";
//             return failure();
//         }
//
//         return success();
//     }
// };
//
// struct ConvertConstOp : public OpConversionPattern<mlir::starplat::ConstOp>
// {
//     using OpConversionPattern::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(mlir::starplat::ConstOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//
//         auto loc   = op.getLoc();
//
//         auto value = op.getValueAttr();
//
//         if (cast<mlir::StringAttr>(value).getValue() == "False") {
//             auto newOp = LLVM::ConstantOp::create(rewriter, loc, mlir::IntegerType::get(op.getContext(), 1), rewriter.getBoolAttr(0));
//             rewriter.replaceOp(op, newOp);
//         }
//
//         else if (cast<mlir::StringAttr>(value).getValue() == "True") {
//             auto newOp = LLVM::ConstantOp::create(rewriter, loc, mlir::IntegerType::get(op.getContext(), 1), rewriter.getBoolAttr(1));
//             rewriter.replaceOp(op, newOp);
//         }
//         else {
//             llvm::outs() << "Error: Constant Not Implemented.";
//             exit(0);
//         }
//
//         return success();
//     }
// };
//
// struct ConvertFixedPointOp : public OpConversionPattern<mlir::starplat::FixedPointUntilOp>
// {
//     using OpConversionPattern::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(mlir::starplat::FixedPointUntilOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//
//         auto cond = op.getTerminationConditionAttr();
//         cond.dump();
//
//         // What to do for fixed point?
//         // Operand 1 will always be an I1, Operand 2 can be any type! Convert it to
//         auto op1     = adaptor.getOperands()[0];
//         auto op2     = adaptor.getOperands()[1];
//
//         auto op1Type = op1.getType();
//         auto op2Type = op2.getType();
//         int width    = op1Type.getIntOrFloatBitWidth();
//
//         if (isa<mlir::IntegerType>(op1Type) && width == 1) {
//
//             if (isa<mlir::starplat::PropNodeType>(op2Type)) {
//                 // Or all the values in the propnode and get an I1
//                 auto graph      = op2.getDefiningOp()->getOperands()[0];
//
//                 auto constZero  = LLVM::ConstantOp::create(rewriter, op.getLoc(), rewriter.getI64Type(), rewriter.getI64IntegerAttr(0));
//                 auto numOfNodes = LLVM::ExtractValueOp::create(rewriter, op.getLoc(), mlir::IntegerType::get(op.getContext(), 64), graph,
//                                                                rewriter.getDenseI64ArrayAttr({0}));
//                 auto step       = LLVM::ConstantOp::create(rewriter, op.getLoc(), rewriter.getI64Type(), rewriter.getI64IntegerAttr(1));
//
//                 mlir::scf::ForOp::create(rewriter, op.getLoc(), constZero, numOfNodes, step, mlir::ValueRange{},
//                                          [&](mlir::OpBuilder& builder, mlir::Location nestedLoc, mlir::Value iv, mlir::ValueRange iterArgs)
//                                          {
//                                              auto indexIv = arith::IndexCastOp::create(builder, nestedLoc, builder.getIndexType(), iv);
//
//                                              // auto val     =
//                                              mlir::memref::LoadOp::create(builder, nestedLoc, adaptor.getOperands()[1], mlir::ValueRange{indexIv});
//                                              mlir::scf::YieldOp::create(builder, nestedLoc);
//                                          });
//             }
//
//             else {
//                 llvm::outs() << "Error: This FixedPoint Operand Type not Implemented\n";
//                 exit(0);
//             }
//         }
//
//         else {
//             llvm::errs() << "Error: Fixedpoint Variable type error\n";
//             exit(0);
//         }
//         // I1 through, or-ing if it is a propNode.
//         // And make use of the termination condition.
//
//         // mlir::scf::WhileOp::create(rewriter,op.getLoc(), );
//
//         rewriter.eraseOp(op);
//
//         return success();
//     }
// };
//
// struct ConvertAdd : public OpConversionPattern<mlir::starplat::AddOp>
// {
//     using OpConversionPattern::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(mlir::starplat::AddOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//
//         auto addOp = LLVM::AddOp::create(rewriter, op.getLoc(), adaptor.getOperands()[0], adaptor.getOperands()[1]);
//
//         rewriter.replaceOp(op, addOp.getResult());
//         return success();
//     }
// };
//
// namespace mlir
// {
// namespace starplat
// {
// #define GEN_PASS_DEF_CONVERTSTARPLATIRTOOMPPASS
// #include "Passes.h.inc"
//
// struct ConvertStarPlatIRToOMPPass : public mlir::starplat::impl::ConvertStarPlatIRToOMPPassBase<ConvertStarPlatIRToOMPPass>
// {
//     using ConvertStarPlatIRToOMPPassBase::ConvertStarPlatIRToOMPPassBase;
//
//     void runOnOperation() override {
//
//         mlir::MLIRContext* context = &getContext();
//         auto* module               = getOperation();
//
//         if (!module)
//             llvm::errs() << "Module not found!\n";
//
//         if (!context)
//             llvm::errs() << "Context not found!\n";
//
//         ConversionTarget target(getContext());
//
//         target.addLegalDialect<mlir::LLVM::LLVMDialect>();
//         target.addLegalDialect<mlir::scf::SCFDialect>();
//         target.addLegalDialect<mlir::memref::MemRefDialect>();
//         target.addLegalDialect<mlir::func::FuncDialect>();
//         target.addLegalDialect<mlir::arith::ArithDialect>();
//
//         target.addIllegalOp<mlir::starplat::AddOp>();
//         // target.addIllegalOp<mlir::starplat::DeclareOp>();
//         // target.addIllegalOp<mlir::starplat::AttachNodePropertyOp>();
//         // target.addIllegalOp<mlir::starplat::ConstOp>();
//         // target.addIllegalOp<mlir::starplat::AssignmentOp>();
//         // target.addIllegalOp<mlir::starplat::SetNodePropertyOp>();
//         // target.addIllegalOp<mlir::starplat::FixedPointUntilOp>();
//
//         RewritePatternSet patterns(context);
//         StarplatToLLVMTypeConverter typeConverter(context);
//
//         patterns.add<ConvertAdd>(context);
//         // patterns.add<ConvertFunc>(typeConverter, context);
//
//         // populateFunctionOpInterfaceTypeConversionPattern<mlir::starplat::FuncOp>(patterns, typeConverter);
//         // target.addDynamicallyLegalOp<mlir::starplat::FuncOp>([&](starplat::FuncOp op)
//         //                                    {
//
//         //     auto isSignatureLegal = typeConverter.isSignatureLegal(op.getFunctionType());
//         //     auto isLegal = typeConverter.isLegal(&op.getBody());
//
//         //     return isSignatureLegal && isLegal; });
//
//         if (failed(applyPartialConversion(module, target, std::move(patterns)))) {
//             signalPassFailure();
//         }
//     }
// };
// } // namespace starplat
// } // namespace mlir
//
// inline mlir::LLVM::LLVMStructType createGraphStruct(mlir::MLIRContext* context) {
//
//     auto structType = LLVM::LLVMStructType::getIdentified(context, "Graph");
//     // Create Node struct type
//
//     // auto ptrType = LLVM::LLVMPointerType::get(context);
//     // Define Graph struct body with (Node*, int)
//     // structType.setBody({ptrType, mlir::IntegerType::get(context, 32), ptrType}, /*isPacked=*/false);
//
//     LogicalResult lr = structType.setBody({mlir::IntegerType::get(context, 64)}, /*isPacked=*/false);
//     if (llvm::succeeded(lr))
//         return structType;
//     // structType.setBody({rewriter->getI32Type()}, false);
//
//     return structType;
// }
//
// inline mlir::LLVM::LLVMStructType createNodeStruct(mlir::IRRewriter* rewriter, mlir::MLIRContext* context) {
//     auto structType = LLVM::LLVMStructType::getIdentified(context, "Node");
//
//     // Create a ptr type
//     // auto ptr = mlir::LLVM::LLVMPointerType::get(rewriter->getI32Type());
//
//     LogicalResult lr = structType.setBody({rewriter->getI32Type()}, false);
//     if (llvm::succeeded(lr))
//         return structType;
//
//     return structType;
// }
//
// #endif
