#ifndef BASE2OMP
#define BASE2OMP

// #include "includes/StarPlatOps.h"
// #include "includes/StarPlatTypes.h"
// #include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
// #include "mlir/IR/OpDefinition.h"
// #include "mlir/IR/Types.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/OperationSupport.h"
#include "mlir/Pass/PassManager.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"

// #include "mlir/IR/BuiltinAttributes.h"
// #include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
// #include "llvm/Support/LogicalResult.h"

// #include "mlir/Dialect/LLVMIR/FunctionCallUtils.h"
#include "mlir/Transforms/DialectConversion.h" // from @llvm-project

#include "typeconverter.h"
#include "llvm/ADT/ArrayRef.h"

// mlir::Operation *generateGraphStruct(mlir::MLIRContext *context)
// {
//     auto structType = LLVM::LLVMStructType::getIdentified(context, "Graph");

//     return;
// }
//
// struct SCFForallToOMPLowering : OpConversionPattern<scf::ForallOp>
// {
//     using OpConversionPattern::OpConversionPattern;
//
//     LogicalResult matchAndRewrite(scf::ForallOp op, OpAdaptor adaptor, ConversionPatternRewriter& rewriter) const override {
//
//         auto loc = op.getLoc();
//
//         // Check if the scf.parallel was annotated with a schedule hint
//         omp::ClauseScheduleKind schedKind = omp::ClauseScheduleKind::Dynamic;
//         Value chunkSize                   = nullptr;
//
//         if (auto schedAttr = op->getAttrOfType<StringAttr>("omp.schedule")) {
//             if (schedAttr.getValue() == "static")
//                 schedKind = omp::ClauseScheduleKind::Static;
//             else if (schedAttr.getValue() == "guided")
//                 schedKind = omp::ClauseScheduleKind::Guided;
//         }
//
//         if (auto chunkAttr = op->getAttrOfType<IntegerAttr>("omp.chunk_size")) {
//             chunkSize = arith::ConstantOp::create(rewriter, loc, chunkAttr);
//         }
//
//         auto schedKindAttr = omp::ClauseScheduleKindAttr::get(rewriter.getContext(), schedKind);
//
//         // Build parallel + wsloop with chosen schedule
//         auto parallelOp2     = omp::ParallelOp::create(rewriter, loc);
//         Block* parallelBlock = rewriter.createBlock(&parallelOp2.getRegion());
//         rewriter.setInsertionPointToStart(parallelBlock);
//
//         // auto wsloopOp = omp::WsloopOp::create(rewriter, loc, llvm::ArrayRef<NamedAttribute>({schedKindAttr, chunkSize}));
//
//         // ... rest of the lowering
//     }
// };

namespace mlir
{
namespace starplat
{
#define GEN_PASS_DEF_CONVERTBASETOOMPPASS
#include "Passes.h.inc"

struct ConvertBaseToOMPPass : public mlir::starplat::impl::ConvertBaseToOMPPassBase<ConvertBaseToOMPPass>
{
    using ConvertBaseToOMPPassBase::ConvertBaseToOMPPassBase;

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

        // patterns.add<ConvertFunc>(typeConverter, context);
        // patterns.add<ConvertDeclareOp>(typeConverter, context);
        // patterns.add<ConvertConstOp>(typeConverter, context);
        // patterns.add<ConvertAssignOp>(context);
        // patterns.add<ConvertForAllNodesOp>(typeConverter, context);
        // patterns.add<ConvertForAllNeighboursOp>(typeConverter, context);
        // patterns.add<ConvertIfOp>(typeConverter, context);
        // patterns.add<ConvertNodeCmpOp>(typeConverter, context);
        // patterns.add<ConvertIsEdgeOp>(typeConverter, context);
        // patterns.add<ConvertAdd>(context);
        // patterns.add<ConvertIncAndAssign>(context);
        // patterns.add<ConvertReturnOp>(typeConverter, context);

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
