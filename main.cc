#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <signal.h>

#include "ast/ast.h"
// #include "ast/visitor.h"
#include "astdump.h"
#include "avial.tab.h"
#include "starplatIR.h"

// #include "mlir/IR/Builders.h"
// #include "mlir/IR/BuiltinOps.h"
// #include "mlir/IR/MLIRContext.h"

// #include "tblgen2/StarPlatOps.cpp.inc"

// #include "mlir/Pass/PassManager.h"
// #include "transforms/reachingDef.h"
// #include "transforms/vertexToEdge.h"

// #include "starplatDialect/includes/StarPlatDialect.h"

// #include "mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
// #include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
// #include "mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
// #include "mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"

#include "lowerings/starplat2llvmconversions.h"

// #include "lowerings/starplat2omp.h"

#define DEBUG_TYPE "dialect-conversion"

extern int yyparse();
extern FILE* yyin;
ASTNode* root;

void signalHandler(int sig) {
    void* array[10];
    size_t size;

    size = backtrace(array, 10);

    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, 0);
    exit(1);
}

int main(int argc, char* argv[]) {
    root = nullptr;
    signal(SIGSEGV, signalHandler);

    if (argc < 2) {
        printf("%s usage\n%s <file name>\n", argv[0], argv[0]);
        return 0;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Cannot open file. \n");
        return 0;
    }

    yyin = file;
    yyparse();
    fclose(file);

    printf("Parsing Complete\n");

    CodeGen* codegen = new CodeGen;

    if (root != nullptr)
        // Accept(codegen) just dumps the AST
        root->Accept(codegen);

    StarPlatCodeGen* starplatcodegen = new StarPlatCodeGen;

    if (root != nullptr)
        root->Accept(starplatcodegen, starplatcodegen->getSymbolTable());
    else {
        llvm::errs() << "Error in DSL Code\n";
        exit(0);
    }

    starplatcodegen->print();

    PassManager pm(starplatcodegen->getContext());
    pm.addPass(mlir::starplat::createConvertStarPlatIRToOMPPass());

    // // RUN the pass on the module
    if (mlir::failed(pm.run(starplatcodegen->getModule()->getOperation()))) {
        llvm::errs() << "StarPlat → OMP lowering failed\n";
        return 0;
    }

    starplatcodegen->print();

    // Work on Conversion of OMP
    // Working on Generating a hello world program in LLVM - Done
    // Workign on generating OMP - Done
    // Donw with the Blog and Repo

    return 0;
}
