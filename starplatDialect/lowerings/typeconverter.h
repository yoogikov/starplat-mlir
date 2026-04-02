
#ifndef STARPLAT_TO_LLVM_TYPE_CONVERTER
#define STARPLAT_TO_LLVM_TYPE_CONVERTER

#include "includes/StarPlatOps.h"
#include "includes/StarPlatTypes.h"
#include "mlir/Conversion/Passes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Pass/PassManager.h"

#include "includes/StarPlatDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/ValueRange.h"

#include "mlir/Transforms/DialectConversion.h" // from @llvm-project

using namespace mlir;
class StarPlatTypeConverter : public TypeConverter
{
  public:
    StarPlatTypeConverter(MLIRContext* ctx) {
        addConversion(
            [](Type type) -> std::optional<Type>
            {
                if (LLVM::isCompatibleType(type))
                    return type;
                return std::nullopt;
            });

        addConversion([ctx](starplat::GraphType type) -> std::optional<Type> { return LLVM::LLVMPointerType::get(ctx); });
        addConversion([ctx](starplat::NodeType type) -> std::optional<Type> { return LLVM::LLVMPointerType::get(ctx); });
        addConversion([ctx](starplat::SPIntType type) -> std::optional<Type> { return LLVM::LLVMPointerType::get(ctx); });
        addConversion([ctx](starplat::PropNodeType type) -> std::optional<Type>
                      { // llvm::errs() << type.getGraphId();
                          // return type.getParameter();
                          return LLVM::LLVMPointerType::get(ctx);
                      });
        addConversion([ctx](starplat::PropEdgeType type) -> std::optional<Type>
                      { // llvm::errs() << type.getGraphId();
                          // return type.getParameter();
                          return LLVM::LLVMPointerType::get(ctx);
                      });

        addSourceMaterialization(
            [](OpBuilder& builder, Type resultType, ValueRange inputs, Location loc) -> Value
            {
                if (inputs.size() != 1)
                    return nullptr;
                return UnrealizedConversionCastOp::create(builder, loc, resultType, inputs).getResult(0);
            });

        addTargetMaterialization(
            [](OpBuilder& builder, Type resultType, ValueRange inputs, Location loc) -> Value
            {
                if (inputs.size() != 1)
                    return nullptr;
                return UnrealizedConversionCastOp::create(builder, loc, resultType, inputs).getResult(0);
            });
    }
};

#endif
