#ifndef AVIALAST
#define AVIALAST

#include "visitor.h"
// #include <iostream>
#include <mlir/IR/SymbolTable.h>
// #include <set>
#include <vector>

using namespace std;

class ASTNode
{
  public:
    virtual ~ASTNode();
    virtual void Accept(Visitor* visitor) const                                     = 0;
    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const = 0;
};

enum ExpressionKind { KIND_NUMBER, KIND_IDENTIFIER, KIND_KEYWORD, KIND_BOOLEXPR, KIND_MEMBERACCESS, KIND_METHODCALL, KIND_ADDOP, KIND_SUBOP, KIND_MULOP, KIND_DIVOP, KIND_ANDOP, KIND_OROP };

class Identifier : public ASTNode
{
  public:
    Identifier(char* name);

    Identifier();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Identifier();

    char* getname() const;

  private:
    char* name_;
};

class GraphProperties : public ASTNode
{
  public:
    GraphProperties(char* properties);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~GraphProperties();

    char* getPropertyType() const;

  private:
    char* properties_;
};

class Statement : public ASTNode
{
  public:
    Statement();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Statement();

    ASTNode* getstatement() const;

  private:
    ASTNode* statement;
};

class Methodcall : public ASTNode
{
  public:
    Methodcall(Identifier* identifier, ASTNode* paramlist);

    Methodcall(Identifier* identifier);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Methodcall();

    const Identifier* getIdentifier() const;
    const ASTNode* getParamLists() const;
    bool getIsBuiltin() const;

  private:
    const Identifier* identifier_;
    const ASTNode* paramlist_;
    bool _isBuiltin;

    static bool checkIfBuiltin(const Identifier* id);
};

class TupleAssignment : public ASTNode
{
  public:
    TupleAssignment(ASTNode* lhsexpr1, ASTNode* lhsexpr2, ASTNode* rhsexpr1, ASTNode* rhsexpr2);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~TupleAssignment();

    const ASTNode* getlhsexpr1() const;
    const ASTNode* getlhsexpr2() const;
    const ASTNode* getrhsexpr1() const;
    const ASTNode* getrhsexpr2() const;

  private:
    const ASTNode* lhsexpr1_;
    const ASTNode* lhsexpr2_;
    const ASTNode* rhsexpr1_;
    const ASTNode* rhsexpr2_;
};

class MemberacceessStmt : public ASTNode
{
  public:
    MemberacceessStmt(ASTNode* memberAccess);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    const ASTNode* getMemberAccess() const;

    ~MemberacceessStmt();

  private:
    ASTNode* memberAccess_;
};

class Memberaccess : public ASTNode
{
  public:
    Memberaccess(Identifier* identifier1, ASTNode* methodcall);

    Memberaccess(Identifier* identifier1, Identifier* identifier2);

    Memberaccess(ASTNode* memberaccessNode, ASTNode* methodcall);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Memberaccess();

    const Identifier* getIdentifier() const;
    const Identifier* getIdentifier2() const;
    const ASTNode* getMethodCall() const;
    const ASTNode* getMemberAccessNode() const;

  private:
    const Identifier* identifier1_;
    const Identifier* identifier2_;
    const ASTNode* methodcall_;
    const ASTNode* memberaccessNode_;
};

class Statementlist : public ASTNode
{
  public:
    Statementlist();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Statementlist();

    const std::vector<ASTNode*> getStatementList() const;
    void addstmt(ASTNode* param);

  private:
    std::vector<ASTNode*> statementlist;
};

class DeclarationStatement : public ASTNode
{
  public:
    DeclarationStatement(ASTNode* type, ASTNode* identifier, ASTNode* number);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~DeclarationStatement();

    ASTNode* gettype() const;
    ASTNode* getvarname() const;
    ASTNode* getnumber() const;

  private:
    ASTNode* type;
    ASTNode* varname;
    ASTNode* number;
};

class InitialiseAssignmentStmt : public ASTNode
{
  public:
    InitialiseAssignmentStmt(ASTNode* type, ASTNode* identifier, ASTNode* expr);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~InitialiseAssignmentStmt();

    ASTNode* gettype() const;
    ASTNode* getidentifier() const;
    ASTNode* getexpr() const;

  private:
    ASTNode* type_;
    ASTNode* identifier_;
    ASTNode* expr_;
};

class FixedpointUntil : public ASTNode
{
  public:
    FixedpointUntil(ASTNode* identifier, ASTNode* expr, ASTNode* stmtlist);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ASTNode* getstmtlist() const;
    ASTNode* getidentifier() const;
    ASTNode* getexpr() const;

    ~FixedpointUntil();

  private:
    ASTNode* expr_;
    ASTNode* identifier_;
    ASTNode* stmtlist_;
};

class ParameterAssignment : public ASTNode
{
  public:
    ParameterAssignment(ASTNode* identifier, ASTNode* keyword);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~ParameterAssignment();

    ASTNode* getidentifier() const;
    ASTNode* getkeyword() const;

  private:
    ASTNode* identifier_;
    ASTNode* keyword_;
};

class TemplateDeclarationStatement : public ASTNode
{
  public:
    TemplateDeclarationStatement(ASTNode* type, ASTNode* identifier);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~TemplateDeclarationStatement();

    ASTNode* gettype() const;
    ASTNode* getvarname() const;

  private:
    ASTNode* type;
    ASTNode* varname;
};

class ForallStatement : public ASTNode
{
  public:
    ForallStatement(Identifier* loopVar, ASTNode* expr, ASTNode* stmtlist);

    ForallStatement();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~ForallStatement();

    Identifier* getLoopVar() const;
    ASTNode* getexpr() const;
    ASTNode* getstmtlist() const;

  private:
    Identifier* loopVar_;
    ASTNode* expr_;
    ASTNode* stmtlist_;
};


class DoWhileStatement : public ASTNode
{
  public:
    DoWhileStatement(ASTNode* expr, ASTNode* stmtlist);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~DoWhileStatement();
    
    ASTNode* getexpr() const;
    ASTNode* getstmtlist() const;

  private:
    ASTNode* expr_;
    ASTNode* stmtlist_;
};

class IfStatement : public ASTNode
{
  public:
    IfStatement(ASTNode* expr, ASTNode* stmt);

    IfStatement();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~IfStatement();

    ASTNode* getexpr() const;
    ASTNode* getstmt() const;

  private:
    ASTNode* expr;
    ASTNode* stmt;
};

class BoolExpr : public ASTNode
{
  public:
    BoolExpr(ASTNode* expr1, char* op, ASTNode* expr2);

    BoolExpr(ASTNode* expr1, char* op);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~BoolExpr();

    ASTNode* getExpr1() const;
    ASTNode* getExpr2() const;
    char* getop() const;

  private:
    ASTNode* expr1_;
    ASTNode* expr2_;
    char* op_;
};

class Add : public ASTNode
{
  public:
    Add(ASTNode* operand1, ASTNode* operand2);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Add();

    ASTNode* getOperand1() const;
    ASTNode* getOperand2() const;

  private:
    ASTNode* operand1_;
    ASTNode* operand2_;
};

class Sub : public ASTNode
{
  public:
    Sub(ASTNode* operand1, ASTNode* operand2);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Sub();

    ASTNode* getOperand1() const;
    ASTNode* getOperand2() const;

  private:
    ASTNode* operand1_;
    ASTNode* operand2_;
};

class Mul : public ASTNode
{
  public:
    Mul(ASTNode* operand1, ASTNode* operand2);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Mul();

    ASTNode* getOperand1() const;
    ASTNode* getOperand2() const;

  private:
    ASTNode* operand1_;
    ASTNode* operand2_;
};

class Div : public ASTNode
{
  public:
    Div(ASTNode* operand1, ASTNode* operand2);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Div();

    ASTNode* getOperand1() const;
    ASTNode* getOperand2() const;

  private:
    ASTNode* operand1_;
    ASTNode* operand2_;
};

class And : public ASTNode
{
  public:
    And(ASTNode* operand1, ASTNode* operand2);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~And();

    ASTNode* getOperand1() const;
    ASTNode* getOperand2() const;

  private:
    ASTNode* operand1_;
    ASTNode* operand2_;
};

class Or : public ASTNode
{
  public:
    Or(ASTNode* operand1, ASTNode* operand2);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Or();

    ASTNode* getOperand1() const;
    ASTNode* getOperand2() const;

  private:
    ASTNode* operand1_;
    ASTNode* operand2_;
};

class Assignment : public ASTNode
{
  public:
    Assignment(char* identifier, ASTNode* expr);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Assignment();

    char* getIdentifier() const;
    ASTNode* getexpr() const;

  private:
    char* identifier;
    ASTNode* expr;
};

class AssignmentStmt : public ASTNode
{
  public:
    AssignmentStmt(ASTNode* assignment);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~AssignmentStmt();

    ASTNode* getAssignment() const;

  private:
    ASTNode* assignment;
};

class Incandassignstmt : public ASTNode
{
  public:
    Incandassignstmt(Identifier* identifier, char* op, ASTNode* expr);

    Incandassignstmt();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Incandassignstmt();

    Identifier* getIdentifier() const;
    ASTNode* getexpr() const;

  private:
    Identifier* identifier;
    char* op;
    ASTNode* expr;
};

class Number : public ASTNode
{
  public:
    Number(char* number);

    Number();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    int getnumber() const;

  private:
    int number_;
};

class ReturnStmt : public ASTNode
{
  public:
    ReturnStmt(ASTNode* expr);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~ReturnStmt();

    ASTNode* getexpr() const;

  private:
    ASTNode* expr;
};

class Function : public ASTNode
{
  public:
    Function(Identifier* functionname, Arglist* arglist, ASTNode* stmtlist);

    // Function();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Function();

    ASTNode* getfuncname() const;
    Arglist* getparams() const;
    ASTNode* getstmtlist() const;
    string getfuncNameIdentifier() const; // Returns the function name as a string.

  private:
    ASTNode* functionname;
    Arglist* arglist;
    ASTNode* stmtlist;
    string funcName;
};

class TemplateType : public ASTNode
{
  public:
    TemplateType(GraphProperties* graphprop, TypeExpr* type, Identifier* graphName);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    const GraphProperties* getGraphPropNode() const;
    const TypeExpr* getType() const;
    const Identifier* getGraphName() const;

  private:
    GraphProperties* graphproperties_;
    TypeExpr* type_;
    Identifier* graphName_;
};

class TypeExpr : public ASTNode
{
  public:
    TypeExpr(char* type);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~TypeExpr();

    const char* getType() const;

  private:
    char* type_;
};

class MemberAccessAssignment : public ASTNode
{
  public:
    MemberAccessAssignment(ASTNode* memberAccess, ASTNode* expr);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~MemberAccessAssignment();

    const ASTNode* getMemberAccess() const;
    const ASTNode* getExpr() const;

  private:
    ASTNode* memberAccess_;
    ASTNode* expr_;
};

class Keyword : public ASTNode
{
  public:
    Keyword(char* keyword);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Keyword();

    const char* getKeyword() const;

  private:
    char* keyword_;
};

class Arg : public ASTNode
{
  public:
    Arg(TypeExpr* type, Identifier* identifier);

    Arg(TemplateType* type, Identifier* identifier);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Arg();

    const TypeExpr* getType() const;
    const TemplateType* getTemplateType() const;
    const Identifier* getVarName() const;

  private:
    TypeExpr* type;
    TemplateType* templatetype;
    Identifier* varname;
};

class Arglist : public ASTNode
{
  public:
    Arglist();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Arglist();

    const std::vector<Arg*> getArgList() const;
    void addarg(Arg* arg);

  private:
    std::vector<Arg*> arglist;
};

class Expression : public ASTNode
{
  public:
    Expression(ASTNode* node, ExpressionKind kind);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    const ASTNode* getExpression() const;
    ExpressionKind getKind() const;

  private:
    ASTNode* node_;
    ExpressionKind kind_;
};

class Param : public ASTNode
{
  public:
    Param(Expression* expr);

    Param(ParameterAssignment* paramAssignment);

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Param();

    const Expression* getExpr() const;
    const ParameterAssignment* getParamAssignment() const;

  private:
    Expression* expr_;
    ParameterAssignment* paramAssignment_;
};

class Paramlist : public ASTNode
{
  public:
    Paramlist();

    virtual void Accept(Visitor* visitor) const override;

    virtual void Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const override;

    ~Paramlist();

    const std::vector<Param*> getParamList() const;
    void addparam(Param* param);

  private:
    std::vector<Param*> paramlist;
};

#endif
