#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "starplatDialect/includes/StarPlatDialect.h"
#include "starplatDialect/lowerings/starplat2base.h"

int main(int argc, char **argv) {
    mlir::DialectRegistry registry;
    mlir::registerAllDialects(registry);
    registry.insert<mlir::starplat::StarPlatDialect>();

    mlir::registerAllPasses();
    ::mlir::registerPass([]() -> std::unique_ptr<::mlir::Pass> {
        return mlir::starplat::createConvertStarPlatIRToBasePass();
    });

    return mlir::asMainReturnCode(
        mlir::MlirOptMain(argc, argv, "StarPlat optimizer driver\n", registry));
}
