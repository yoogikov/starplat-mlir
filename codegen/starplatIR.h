#ifndef STARPLATIR
#define STARPLATIR

// Next Iteration TODOs
// 1. Revmap the parser file. It's very messy.
// 2. Think if we need to cast here or at parser file.
// 3. Return String instead of const char *
// 4. Remove ArgList Code gen from Funntion and write it in VisitArgList.
// 5. Check if the type is node propert inorder to do attachnode.
// 6. Add return to the Accept Function.
// 7. Rewrite visitForAllStmt.
// 8. Add switch instead of IF-ELSE.
// 9. Change edge -> Edge type.
// 10. Handle all the special function seperately.
// 11. Rewrite Add to Arithematic OP
// 12. For now creating a new operation for argument. Later make use og block
// arguments.
#include "ast.h"
// #include "includes/StarPlatDialect.h"
// #include "includes/StarPlatOps.h"
// #include "includes/StarPlatTypes.h"

#include "mlir/IR/Builders.h" // For mlir::OpBuilder
#include "mlir/IR/Dialect.h"  // For mlir::Dialect
// #include "mlir/IR/Operation.h" // For mlir::Operation

// #include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
// #include "mlir/IR/Verifier.h"
#include "mlir/Parser/Parser.h"
// #include <string>

// #include "mlir/Interfaces/CallInterfaces.h"
// #include "mlir/Interfaces/FunctionInterfaces.h"
// #include "mlir/Interfaces/SideEffectInterfaces.h"
#include "mlir/Support/LLVM.h"
#include "visitor.h"
// #include "llvm/Support/LogicalResult.h"

class StarPlatCodeGen : public MLIRVisitor
{

  public:
    StarPlatCodeGen();

    virtual void visitDeclarationStmt(const DeclarationStatement* dclstmt, mlir::SymbolTable* symbolTable) override;

    virtual void visitTemplateDeclarationStmt(const TemplateDeclarationStatement* templateDeclStmt, mlir::SymbolTable* symbolTable) override;

    virtual void visitTemplateType(const TemplateType* templateType, mlir::SymbolTable* symbolTable) override;

    virtual void visitForallStmt(const ForallStatement* forAllStmt, mlir::SymbolTable* symbolTable) override;

    virtual void visitDoWhileStmt(const DoWhileStatement* doWhileStmt, mlir::SymbolTable* symbolTable) override;

    virtual void visitMemberaccessStmt(const MemberacceessStmt* MemberacceessStmt, mlir::SymbolTable* symbolTable) override;

    virtual void visitIfStmt(const IfStatement* ifStmt, mlir::SymbolTable* symbolTable) override;

    virtual void visitBoolExpr(const BoolExpr* boolExpr, mlir::SymbolTable* symbolTable) override;

    virtual void visitIncandassignstmt(const Incandassignstmt* incandassignstmt, mlir::SymbolTable* symbolTable) override;

    void visitAssignment(const Assignment* assignment, mlir::SymbolTable* symbolTable) override;

    void visitAssignmentStmt(const AssignmentStmt* assignemntStmt, mlir::SymbolTable* symbolTable) override;

    virtual void visitIdentifier(const Identifier* identifier, mlir::SymbolTable* symbolTable) override;

    virtual void visitReturnStmt(const ReturnStmt* returnStmt, mlir::SymbolTable* symbolTable) override;

    virtual void visitParameterAssignment(const ParameterAssignment* paramAssignment, mlir::SymbolTable* symbolTable) override;

    virtual void visitParam(const Param* param, mlir::SymbolTable* symbolTable) override;

    virtual void visitTupleAssignment(const TupleAssignment* tupleAssignment, mlir::SymbolTable* symbolTable) override;

    virtual void visitAdd(const Add* add, mlir::SymbolTable* symbolTable) override;

    virtual void visitFunction(const Function* function, mlir::SymbolTable* symbolTable) override;

    virtual void visitParamlist(const Paramlist* paramlist, mlir::SymbolTable* symbolTable) override;

    virtual void visitFixedpointUntil(const FixedpointUntil* fixedpointuntil, mlir::SymbolTable* symbolTable) override;

    virtual void visitInitialiseAssignmentStmt(const InitialiseAssignmentStmt* initialiseAssignmentStmt, mlir::SymbolTable* symbolTable) override;

    virtual void visitMemberAccessAssignment(const MemberAccessAssignment* memberAccessAssignment, mlir::SymbolTable* symbolTable) override;

    virtual void visitKeyword(const Keyword* keyword, mlir::SymbolTable* symbolTable) override;

    virtual void visitGraphProperties(const GraphProperties* graphproperties, mlir::SymbolTable* symbolTable) override;

    virtual void visitMethodcall(const Methodcall* methodcall, mlir::SymbolTable* symbolTable) override;

    virtual void visitMemberaccess(const Memberaccess* memberaccess, mlir::SymbolTable* symbolTable) override;

    virtual void visitArglist(const Arglist* arglist, mlir::SymbolTable* symbolTable) override;

    virtual void visitArg(const Arg* arg, mlir::SymbolTable* symbolTable) override;

    virtual void visitStatement(const Statement* statement, mlir::SymbolTable* symbolTable) override;

    virtual void visitStatementlist(const Statementlist* stmtlist, mlir::SymbolTable* symbolTable) override;

    virtual void visitType(const TypeExpr* type, mlir::SymbolTable* symbolTable) override;

    virtual void visitNumber(const Number* number, mlir::SymbolTable* symbolTable) override;

    virtual void visitExpression(const Expression* expr, mlir::SymbolTable* symbolTable) override;

    void print();

    mlir::SymbolTable* getSymbolTable();

    mlir::MLIRContext* getContext();

    mlir::ModuleOp* getModule();

  private:
    mlir::MLIRContext context;
    mlir::OpBuilder builder;
    mlir::ModuleOp module;
    mlir::SymbolTable globalSymbolTable;

    std::vector<mlir::SymbolTable*> symbolTables;
    llvm::DenseMap<llvm::StringRef, mlir::Value> nameToArgMap;

    mlir::Value globalLookupOp(llvm::StringRef name);

    uint64_t const_count;

    int get_const_count() {
      return const_count++;
    }
};

#endif
