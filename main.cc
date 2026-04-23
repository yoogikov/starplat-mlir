#include <cstdio>
#include <cstdlib>
#include <execinfo.h>
#include <signal.h>

#include "ast/ast.h"
#include "avial.tab.h"
#include "starplatIR.h"

extern int yyparse();
extern FILE* yyin;
ASTNode* root;

void signalHandler(int sig) {
    void* array[10];
    size_t size = backtrace(array, 10);
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, 0);
    exit(1);
}

int main(int argc, char* argv[]) {
    root = nullptr;
    signal(SIGSEGV, signalHandler);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.avl>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return 1;
    }

    yyin = file;
    yyparse();
    fclose(file);

    if (!root) {
        llvm::errs() << "Error parsing DSL\n";
        return 1;
    }

    StarPlatCodeGen codegen;
    root->Accept(&codegen, codegen.getSymbolTable());

    codegen.getModule()->print(llvm::outs());
    return 0;
}
