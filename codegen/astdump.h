#ifndef ASTDUMP
#define ASTDUMP

#include "../ast/ast.h"
#include "../ast/visitor.h"
#include <map>
#include <string>

using namespace std;

// for neat printing

class Beautifier {
public:
  Beautifier();
  void operator++();
  void operator--();
  void print_tab();

private:
  size_t tab_space;
  size_t tab_size;

};

class CodeGen : public Visitor
{

  public:
    CodeGen();

    virtual void visitDeclarationStmt(const DeclarationStatement* dclstmt) override;

    virtual void visitAdd(const Add* add) override;

    virtual void visitSub(const Sub* sub) override;

    virtual void visitMul(const Mul* mul) override;

    virtual void visitDiv(const Div* div) override;

    virtual void visitAnd(const And* _and) override;

    virtual void visitOr(const Or* _or) override;

    virtual void visitMemberaccessStmt(const MemberacceessStmt* memberAccess) override;

    virtual void visitTemplateDeclarationStmt(const TemplateDeclarationStatement* templateDeclStmt) override;

    virtual void visitTemplateType(const TemplateType* templateType) override;

    virtual void visitForallStmt(const ForallStatement* forAllStmt) override;

    virtual void visitForStmt(const ForStatement* forStmt) override;

    virtual void visitDoWhileStmt(const DoWhileStatement* doWhileStmt) override;

    virtual void visitIfStmt(const IfStatement* ifStmt) override;

    virtual void visitIfElseStmt(const IfElseStatement* ifElseStmt) override;

    virtual void visitBoolExpr(const BoolExpr* boolExpr) override;

    virtual void visitIncandassignstmt(const Incandassignstmt* incandassignstmt) override;

    virtual void visitIncstmt(const Incstmt* incstmt) override;

    virtual void visitAssignment(const Assignment* assignment) override;

    virtual void visitAssignmentStmt(const AssignmentStmt* assignemntStmt) override;

    virtual void visitIdentifier(const Identifier* identifier) override;

    virtual void visitReturnStmt(const ReturnStmt* returnStmt) override;

    virtual void visitParameterAssignment(const ParameterAssignment* paramAssignment) override;

    virtual void visitParam(const Param* param) override;

    virtual void visitTupleAssignment(const TupleAssignment* tupleAssignment) override;

    virtual void visitFunction(const Function* function) override;

    virtual void visitParamlist(const Paramlist* paramlist) override;

    virtual void visitFixedpointUntil(const FixedpointUntil* fixedpointuntil) override;

    virtual void visitInitialiseAssignmentStmt(const InitialiseAssignmentStmt* initialiseAssignmentStmt) override;

    virtual void visitMemberAccessAssignment(const MemberAccessAssignment* memberAccessAssignment) override;

    virtual void visitKeyword(const Keyword* keyword) override;

    virtual void visitGraphProperties(const GraphProperties* graphproperties) override;

    virtual void visitMethodcall(const Methodcall* methodcall) override;

    virtual void visitMemberaccess(const Memberaccess* memberaccess) override;

    virtual void visitArglist(const Arglist* arglist) override;

    virtual void visitArg(const Arg* arg) override;

    virtual void visitStatement(const Statement* statement) override;

    virtual void visitStatementlist(const Statementlist* stmtlist) override;

    virtual void visitType(const TypeExpr* type) override;

    virtual void visitNumber(const Number* number) override;

    virtual void visitExpression(const Expression* expr) override;

  private:
    int result;
    map<string, int> variables;
    Beautifier beautifier;
};

#endif
