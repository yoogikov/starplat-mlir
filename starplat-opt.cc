#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/IndexToLLVM/IndexToLLVM.h"
#include "mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
#include "mlir/Conversion/OpenMPToLLVM/ConvertOpenMPToLLVM.h"
#include "mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"
#include "mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "mlir/Conversion/SCFToOpenMP/SCFToOpenMP.h"
#include "mlir/Dialect/Linalg/Passes.h"
#include "mlir/Pass/PassManager.h"

#include "starplatDialect/includes/StarPlatDialect.h"
#include "starplatDialect/lowerings/starplat2base.h"
#include "starplatDialect/lowerings/scfparallel2for.h"

int main(int argc, char** argv) {
    mlir::DialectRegistry registry;
    mlir::registerAllDialects(registry);
    registry.insert<mlir::starplat::StarPlatDialect>();

    mlir::registerAllPasses();
    ::mlir::registerPass([]() -> std::unique_ptr<::mlir::Pass> { return mlir::starplat::createConvertStarPlatIRToBasePass(); });
    ::mlir::registerPass([]() -> std::unique_ptr<::mlir::Pass> { return createConvertSCFParallelToForPass(); });

    mlir::PassPipelineRegistration<>("starplat-to-base", "Lower StarPlat ops to base dialects (SCF/MemRef/Arith/Func), stop there",
                                     [](mlir::OpPassManager& pm)
                                     {
                                         pm.addPass(mlir::starplat::createConvertStarPlatIRToBasePass());
                                     });

    mlir::PassPipelineRegistration<>("starplat-to-llvm", "Lower StarPlat all the way to LLVM dialect (ready for mlir-translate)",
                                     [](mlir::OpPassManager& pm)
                                     {
                                         pm.addPass(mlir::starplat::createConvertStarPlatIRToBasePass());
                                         pm.addPass(mlir::createConvertLinalgToLoopsPass());
                                         pm.addPass(mlir::createConvertSCFToOpenMPPass());
                                         pm.addPass(mlir::createFinalizeMemRefToLLVMConversionPass());
                                         pm.addPass(mlir::createSCFToControlFlowPass());
                                         pm.addPass(mlir::createArithToLLVMConversionPass());
                                         pm.addPass(mlir::createConvertIndexToLLVMPass());
                                         pm.addPass(mlir::createConvertFuncToLLVMPass());
                                         pm.addPass(mlir::createConvertOpenMPToLLVMPass());
                                         pm.addPass(mlir::createConvertControlFlowToLLVMPass());
                                         pm.addPass(mlir::createReconcileUnrealizedCastsPass());
                                     });

    mlir::PassPipelineRegistration<>("starplat-to-llvm-seq", "Lower StarPlat to LLVM via sequential for loops (no OpenMP)",
                                     [](mlir::OpPassManager& pm)
                                     {
                                         pm.addPass(mlir::starplat::createConvertStarPlatIRToBasePass());
                                         pm.addPass(mlir::createConvertLinalgToLoopsPass());
                                         pm.addPass(createConvertSCFParallelToForPass());
                                         pm.addPass(mlir::createFinalizeMemRefToLLVMConversionPass());
                                         pm.addPass(mlir::createSCFToControlFlowPass());
                                         pm.addPass(mlir::createArithToLLVMConversionPass());
                                         pm.addPass(mlir::createConvertIndexToLLVMPass());
                                         pm.addPass(mlir::createConvertFuncToLLVMPass());
                                         pm.addPass(mlir::createConvertControlFlowToLLVMPass());
                                         pm.addPass(mlir::createReconcileUnrealizedCastsPass());
                                     });

    return mlir::asMainReturnCode(mlir::MlirOptMain(argc, argv, "StarPlat optimizer driver\n", registry));
}
