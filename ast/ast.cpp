// #include "visitor.h"
#include <ast.h>
// #include <iostream>
// #include <mlir/IR/SymbolTable.h>
// #include <set>
// #include <vector>

using namespace std;
using namespace mlir;

ASTNode::~ASTNode() {}

Identifier::Identifier(char* name) : name_(name) {}

Identifier::Identifier() {}

void Identifier::Accept(Visitor* visitor) const { visitor->visitIdentifier(this); }

void Identifier::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitIdentifier(this, symbolTable); }

Identifier::~Identifier() { delete name_; }

char* Identifier::getname() const { return name_; }

// GraphProperties
GraphProperties::GraphProperties(char* properties) : properties_(properties) {}
GraphProperties::~GraphProperties() { delete properties_; }
char* GraphProperties::getPropertyType() const { return properties_; }
void GraphProperties::Accept(Visitor* visitor) const { visitor->visitGraphProperties(this); }
void GraphProperties::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitGraphProperties(this, symbolTable); }

// Statement
Statement::Statement() {}
Statement::~Statement() { delete statement; }
ASTNode* Statement::getstatement() const { return statement; }
void Statement::Accept(Visitor* visitor) const { visitor->visitStatement(this); }
void Statement::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitStatement(this, symbolTable); }

// Methodcall
Methodcall::Methodcall(Identifier* identifier, ASTNode* paramlist)
    : identifier_(identifier), paramlist_(paramlist), _isBuiltin(checkIfBuiltin(identifier)) {}
Methodcall::Methodcall(Identifier* identifier) : identifier_(identifier), paramlist_(nullptr), _isBuiltin(checkIfBuiltin(identifier)) {}
Methodcall::~Methodcall() {
    delete identifier_;
    delete paramlist_;
}
const Identifier* Methodcall::getIdentifier() const { return identifier_; }
const ASTNode* Methodcall::getParamLists() const { return paramlist_; }
bool Methodcall::getIsBuiltin() const { return _isBuiltin; }
void Methodcall::Accept(Visitor* visitor) const { visitor->visitMethodcall(this); }
void Methodcall::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitMethodcall(this, symbolTable); }
bool Methodcall::checkIfBuiltin(const Identifier* id) {
    static const std::set<std::string> builtins = {"print", "attachNodeProperty", "filter", "get_edge", "neighbors", "nodes", "Min", "num_nodes", "count_outNbrs", "is_an_edge"};
    return builtins.find(id->getname()) != builtins.end();
}

// TupleAssignment
TupleAssignment::TupleAssignment(ASTNode* lhsexpr1, ASTNode* lhsexpr2, ASTNode* rhsexpr1, ASTNode* rhsexpr2)
    : lhsexpr1_(lhsexpr1), lhsexpr2_(lhsexpr2), rhsexpr1_(rhsexpr1), rhsexpr2_(rhsexpr2) {}
TupleAssignment::~TupleAssignment() {
    delete lhsexpr1_;
    delete lhsexpr2_;
    delete rhsexpr1_;
    delete rhsexpr2_;
}
const ASTNode* TupleAssignment::getlhsexpr1() const { return lhsexpr1_; }
const ASTNode* TupleAssignment::getlhsexpr2() const { return lhsexpr2_; }
const ASTNode* TupleAssignment::getrhsexpr1() const { return rhsexpr1_; }
const ASTNode* TupleAssignment::getrhsexpr2() const { return rhsexpr2_; }
void TupleAssignment::Accept(Visitor* visitor) const { visitor->visitTupleAssignment(this); }
void TupleAssignment::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitTupleAssignment(this, symbolTable); }

// MemberacceessStmt
MemberacceessStmt::MemberacceessStmt(ASTNode* memberAccess) : memberAccess_(memberAccess) {}
MemberacceessStmt::~MemberacceessStmt() { delete memberAccess_; }
const ASTNode* MemberacceessStmt::getMemberAccess() const { return memberAccess_; }
void MemberacceessStmt::Accept(Visitor* visitor) const { visitor->visitMemberaccessStmt(this); }
void MemberacceessStmt::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitMemberaccessStmt(this, symbolTable); }

Memberaccess::Memberaccess(Identifier* identifier1, ASTNode* methodcall)
    : identifier1_(identifier1), identifier2_(nullptr), methodcall_(methodcall), memberaccessNode_(nullptr) {}
Memberaccess::Memberaccess(Identifier* identifier1, Identifier* identifier2)
    : identifier1_(identifier1), identifier2_(identifier2), methodcall_(nullptr), memberaccessNode_(nullptr) {}
Memberaccess::Memberaccess(ASTNode* memberaccessNode, ASTNode* methodcall)
    : identifier1_(nullptr), identifier2_(nullptr), methodcall_(methodcall), memberaccessNode_(memberaccessNode) {}
Memberaccess::~Memberaccess() {
    delete identifier1_;
    delete identifier2_;
    delete memberaccessNode_;
    delete methodcall_;
}
const Identifier* Memberaccess::getIdentifier() const { return identifier1_; }
const Identifier* Memberaccess::getIdentifier2() const { return identifier2_; }
const ASTNode* Memberaccess::getMethodCall() const { return methodcall_; }
const ASTNode* Memberaccess::getMemberAccessNode() const { return memberaccessNode_; }
void Memberaccess::Accept(Visitor* visitor) const { visitor->visitMemberaccess(this); }
void Memberaccess::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitMemberaccess(this, symbolTable); }

// Statementlist
Statementlist::Statementlist() {}
Statementlist::~Statementlist() {
    for (ASTNode* param : statementlist)
        delete param;
}
const std::vector<ASTNode*> Statementlist::getStatementList() const { return statementlist; }
void Statementlist::addstmt(ASTNode* param) { statementlist.push_back(param); }
void Statementlist::Accept(Visitor* visitor) const { visitor->visitStatementlist(this); }
void Statementlist::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const {
    for (ASTNode* stmt : statementlist)
        stmt->Accept(visitor, symbolTable);
}

// DeclarationStatement
DeclarationStatement::DeclarationStatement(ASTNode* type, ASTNode* identifier, ASTNode* number) : type(type), varname(identifier), number(number) {}
DeclarationStatement::~DeclarationStatement() {
    delete type;
    delete varname;
    delete number;
}
ASTNode* DeclarationStatement::gettype() const { return type; }
ASTNode* DeclarationStatement::getvarname() const { return varname; }
ASTNode* DeclarationStatement::getnumber() const { return number; }
void DeclarationStatement::Accept(Visitor* visitor) const { visitor->visitDeclarationStmt(this); }
void DeclarationStatement::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitDeclarationStmt(this, symbolTable); }

// InitialiseAssignmentStmt
InitialiseAssignmentStmt::InitialiseAssignmentStmt(ASTNode* type, ASTNode* identifier, ASTNode* expr)
    : type_(type), identifier_(identifier), expr_(expr) {}
InitialiseAssignmentStmt::~InitialiseAssignmentStmt() {
    delete type_;
    delete identifier_;
    delete expr_;
}
ASTNode* InitialiseAssignmentStmt::gettype() const { return type_; }
ASTNode* InitialiseAssignmentStmt::getidentifier() const { return identifier_; }
ASTNode* InitialiseAssignmentStmt::getexpr() const { return expr_; }
void InitialiseAssignmentStmt::Accept(Visitor* visitor) const { visitor->visitInitialiseAssignmentStmt(this); }
void InitialiseAssignmentStmt::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const {
    visitor->visitInitialiseAssignmentStmt(this, symbolTable);
}

// FixedpointUntil
FixedpointUntil::FixedpointUntil(ASTNode* identifier, ASTNode* expr, ASTNode* stmtlist) : expr_(expr), identifier_(identifier), stmtlist_(stmtlist) {}
FixedpointUntil::~FixedpointUntil() {
    delete stmtlist_;
    delete identifier_;
    delete expr_;
}
ASTNode* FixedpointUntil::getstmtlist() const { return stmtlist_; }
ASTNode* FixedpointUntil::getidentifier() const { return identifier_; }
ASTNode* FixedpointUntil::getexpr() const { return expr_; }
void FixedpointUntil::Accept(Visitor* visitor) const { visitor->visitFixedpointUntil(this); }
void FixedpointUntil::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitFixedpointUntil(this, symbolTable); }

// ParameterAssignment
ParameterAssignment::ParameterAssignment(ASTNode* identifier, ASTNode* rhs, bool iskeyword) : identifier_(identifier) {
    iskeyword_ = iskeyword;
    if(iskeyword) {
        keyword_ = rhs;
        expr_ = nullptr;
    }
    else {
        keyword_ = nullptr;
        expr_ = rhs;
    }
}
ParameterAssignment::~ParameterAssignment() {
    delete identifier_;
    delete keyword_;
}
ASTNode* ParameterAssignment::getidentifier() const { return identifier_; }
ASTNode* ParameterAssignment::getkeyword() const { return keyword_; }
ASTNode* ParameterAssignment::getexpr() const { return expr_; }
bool ParameterAssignment::isKeyword() const { return iskeyword_; }
void ParameterAssignment::Accept(Visitor* visitor) const { visitor->visitParameterAssignment(this); }
void ParameterAssignment::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitParameterAssignment(this, symbolTable); }

// TemplateDeclarationStatement
TemplateDeclarationStatement::TemplateDeclarationStatement(ASTNode* type, ASTNode* identifier) : type(type), varname(identifier) {}
TemplateDeclarationStatement::~TemplateDeclarationStatement() {
    delete type;
    delete varname;
}
ASTNode* TemplateDeclarationStatement::gettype() const { return type; }
ASTNode* TemplateDeclarationStatement::getvarname() const { return varname; }
void TemplateDeclarationStatement::Accept(Visitor* visitor) const { visitor->visitTemplateDeclarationStmt(this); }
void TemplateDeclarationStatement::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const {
    visitor->visitTemplateDeclarationStmt(this, symbolTable);
}

// ForallStatement
ForallStatement::ForallStatement(Identifier* loopVar, ASTNode* expr, ASTNode* stmtlist) : loopVar_(loopVar), expr_(expr), stmtlist_(stmtlist) {}
ForallStatement::ForallStatement() {}
ForallStatement::~ForallStatement() {
    delete loopVar_;
    delete expr_;
    delete stmtlist_;
}
Identifier* ForallStatement::getLoopVar() const { return loopVar_; }
ASTNode* ForallStatement::getexpr() const { return expr_; }
ASTNode* ForallStatement::getstmtlist() const { return stmtlist_; }
void ForallStatement::Accept(Visitor* visitor) const { visitor->visitForallStmt(this); }
void ForallStatement::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitForallStmt(this, symbolTable); }

// ForStatement
ForStatement::ForStatement(Identifier* loopVar, ASTNode* expr, ASTNode* stmtlist) : loopVar_(loopVar), expr_(expr), stmtlist_(stmtlist) {}
ForStatement::ForStatement() {}
ForStatement::~ForStatement() {
    delete loopVar_;
    delete expr_;
    delete stmtlist_;
}
Identifier* ForStatement::getLoopVar() const { return loopVar_; }
ASTNode* ForStatement::getexpr() const { return expr_; }
ASTNode* ForStatement::getstmtlist() const { return stmtlist_; }
void ForStatement::Accept(Visitor* visitor) const { visitor->visitForStmt(this); }
void ForStatement::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitForStmt(this, symbolTable); }

// DoWhileStatment
DoWhileStatement::DoWhileStatement(ASTNode* expr, ASTNode* stmtlist) : expr_(expr), stmtlist_(stmtlist) {}
DoWhileStatement::~DoWhileStatement() {
    delete expr_;
    delete stmtlist_;
}
ASTNode* DoWhileStatement::getexpr() const { return expr_; }
ASTNode* DoWhileStatement::getstmtlist() const { return stmtlist_; }
void DoWhileStatement::Accept(Visitor* visitor) const { visitor->visitDoWhileStmt(this); }
void DoWhileStatement::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitDoWhileStmt(this, symbolTable); }

// IfStatement
IfStatement::IfStatement(ASTNode* expr, ASTNode* stmt) : expr(expr), stmt(stmt) {}
IfStatement::IfStatement() {}
IfStatement::~IfStatement() {
    delete expr;
    delete stmt;
}
ASTNode* IfStatement::getexpr() const { return expr; }
ASTNode* IfStatement::getstmt() const { return stmt; }
void IfStatement::Accept(Visitor* visitor) const { visitor->visitIfStmt(this); }
void IfStatement::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitIfStmt(this, symbolTable); }

//IfElseStatement
IfElseStatement::IfElseStatement(ASTNode* expr, ASTNode* stmt1, ASTNode* stmt2) : expr(expr), stmt1(stmt1), stmt2(stmt2) {}
IfElseStatement::IfElseStatement() {}
IfElseStatement::~IfElseStatement() {
    delete expr;
    delete stmt1;
    delete stmt2;
}
ASTNode* IfElseStatement::getexpr() const { return expr; }
ASTNode* IfElseStatement::getstmt1() const { return stmt1; }
ASTNode* IfElseStatement::getstmt2() const { return stmt2; }
void IfElseStatement::Accept(Visitor* visitor) const { visitor->visitIfElseStmt(this); }
void IfElseStatement::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitIfElseStmt(this, symbolTable); }

// BoolExpr
BoolExpr::BoolExpr(ASTNode* expr1, char* op, ASTNode* expr2) : expr1_(expr1), expr2_(expr2), op_(op) {}
BoolExpr::BoolExpr(ASTNode* expr1, char* op) : expr1_(expr1), expr2_(nullptr), op_(op) {}
BoolExpr::~BoolExpr() {
    delete expr1_;
    delete expr2_;
    delete op_;
}
ASTNode* BoolExpr::getExpr1() const { return expr1_; }
ASTNode* BoolExpr::getExpr2() const { return expr2_; }
char* BoolExpr::getop() const { return op_; }
void BoolExpr::Accept(Visitor* visitor) const { visitor->visitBoolExpr(this); }
void BoolExpr::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitBoolExpr(this, symbolTable); }

// Add
Add::Add(ASTNode* operand1, ASTNode* operand2) : operand1_(operand1), operand2_(operand2) {}
Add::~Add() {
    delete operand1_;
    delete operand2_;
}
ASTNode* Add::getOperand1() const { return operand1_; }
ASTNode* Add::getOperand2() const { return operand2_; }
void Add::Accept(Visitor* visitor) const { visitor->visitAdd(this); }
void Add::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitAdd(this, symbolTable); }

// Sub
Sub::Sub(ASTNode* operand1, ASTNode* operand2) : operand1_(operand1), operand2_(operand2) {}
Sub::~Sub() {
    delete operand1_;
    delete operand2_;
}
ASTNode* Sub::getOperand1() const { return operand1_; }
ASTNode* Sub::getOperand2() const { return operand2_; }
void Sub::Accept(Visitor* visitor) const { visitor->visitSub(this); }
void Sub::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitSub(this, symbolTable); }

// Mul
Mul::Mul(ASTNode* operand1, ASTNode* operand2) : operand1_(operand1), operand2_(operand2) {}
Mul::~Mul() {
    delete operand1_;
    delete operand2_;
}
ASTNode* Mul::getOperand1() const { return operand1_; }
ASTNode* Mul::getOperand2() const { return operand2_; }
void Mul::Accept(Visitor* visitor) const { visitor->visitMul(this); }
void Mul::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitMul(this, symbolTable); }

// Div
Div::Div(ASTNode* operand1, ASTNode* operand2) : operand1_(operand1), operand2_(operand2) {}
Div::~Div() {
    delete operand1_;
    delete operand2_;
}
ASTNode* Div::getOperand1() const { return operand1_; }
ASTNode* Div::getOperand2() const { return operand2_; }
void Div::Accept(Visitor* visitor) const { visitor->visitDiv(this); }
void Div::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitDiv(this, symbolTable); }

// And
And::And(ASTNode* operand1, ASTNode* operand2) : operand1_(operand1), operand2_(operand2) {}
And::~And() {
    delete operand1_;
    delete operand2_;
}
ASTNode* And::getOperand1() const { return operand1_; }
ASTNode* And::getOperand2() const { return operand2_; }
void And::Accept(Visitor* visitor) const { visitor->visitAnd(this); }
void And::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitAnd(this, symbolTable); }

// Or
Or::Or(ASTNode* operand1, ASTNode* operand2) : operand1_(operand1), operand2_(operand2) {}
Or::~Or() {
    delete operand1_;
    delete operand2_;
}
ASTNode* Or::getOperand1() const { return operand1_; }
ASTNode* Or::getOperand2() const { return operand2_; }
void Or::Accept(Visitor* visitor) const { visitor->visitOr(this); }
void Or::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitOr(this, symbolTable); }

// Assignment
Assignment::Assignment(char* identifier, ASTNode* expr) : identifier(identifier), expr(expr) {}
Assignment::~Assignment() {
    delete identifier;
    delete expr;
}
char* Assignment::getIdentifier() const { return identifier; }
ASTNode* Assignment::getexpr() const { return expr; }
void Assignment::Accept(Visitor* visitor) const { visitor->visitAssignment(this); }
void Assignment::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitAssignment(this, symbolTable); }

// AssignmentStmt
AssignmentStmt::AssignmentStmt(ASTNode* assignment) : assignment(assignment) {}
AssignmentStmt::~AssignmentStmt() { delete assignment; }
ASTNode* AssignmentStmt::getAssignment() const { return assignment; }
void AssignmentStmt::Accept(Visitor* visitor) const { visitor->visitAssignmentStmt(this); }
void AssignmentStmt::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitAssignmentStmt(this, symbolTable); }

// Incandassignstmt
Incandassignstmt::Incandassignstmt(Identifier* identifier, char* op, ASTNode* expr) : identifier(identifier), op(op), expr(expr) {}
Incandassignstmt::Incandassignstmt() {}
Incandassignstmt::~Incandassignstmt() {
    delete identifier;
    delete op;
    delete expr;
}
Identifier* Incandassignstmt::getIdentifier() const { return identifier; }
ASTNode* Incandassignstmt::getexpr() const { return expr; }
void Incandassignstmt::Accept(Visitor* visitor) const { visitor->visitIncandassignstmt(this); }
void Incandassignstmt::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitIncandassignstmt(this, symbolTable); }

// Incstmt
Incstmt::Incstmt(Identifier* identifier) : identifier(identifier) {}
Incstmt::Incstmt() {}
Incstmt::~Incstmt() {
    delete identifier;
}
Identifier* Incstmt::getIdentifier() const { return identifier; }
void Incstmt::Accept(Visitor* visitor) const { visitor->visitIncstmt(this); }
void Incstmt::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitIncstmt(this, symbolTable); }
// Number
Number::Number(char* number) {
    std::string s(number);
    if (s.find('.') != std::string::npos) {
        isFloat_      = true;
        numberfloat_  = atof(number);
        number_       = 0;
    } else {
        isFloat_      = false;
        number_       = atoi(number);
        numberfloat_  = 0.0;
    }
}
Number::Number() {}
int Number::getnumber() const { return number_; }
double Number::getnumberfloat() const { return numberfloat_; }
bool Number::isFloat() const { return isFloat_; }
void Number::Accept(Visitor* visitor) const { visitor->visitNumber(this); }
void Number::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitNumber(this, symbolTable); }

// ReturnStmt
ReturnStmt::ReturnStmt(ASTNode* expr) : expr(expr) {}
ReturnStmt::~ReturnStmt() { delete expr; }
ASTNode* ReturnStmt::getexpr() const { return expr; }
void ReturnStmt::Accept(Visitor* visitor) const { visitor->visitReturnStmt(this); }
void ReturnStmt::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitReturnStmt(this, symbolTable); }

// Function
Function::Function(Identifier* functionname, Arglist* arglist, ASTNode* stmtlist) : functionname(functionname), arglist(arglist), stmtlist(stmtlist) {
    funcName = functionname->getname();
}
Function::~Function() {
    delete functionname;
    delete stmtlist;
}
ASTNode* Function::getfuncname() const { return functionname; }
Arglist* Function::getparams() const { return arglist; }
ASTNode* Function::getstmtlist() const { return stmtlist; }
string Function::getfuncNameIdentifier() const { return funcName; }
void Function::Accept(Visitor* visitor) const { visitor->visitFunction(this); }
void Function::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitFunction(this, symbolTable); }

// TemplateType
TemplateType::TemplateType(GraphProperties* graphprop, TypeExpr* type, Identifier* graphName)
    : graphproperties_(graphprop), type_(type), graphName_(graphName) {}
const GraphProperties* TemplateType::getGraphPropNode() const { return graphproperties_; }
const TypeExpr* TemplateType::getType() const { return type_; }
const Identifier* TemplateType::getGraphName() const { return graphName_; }
void TemplateType::Accept(Visitor* visitor) const { visitor->visitTemplateType(this); }
void TemplateType::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitTemplateType(this, symbolTable); }

// TypeExpr
TypeExpr::TypeExpr(char* type) : type_(type) {}
TypeExpr::~TypeExpr() { delete type_; }
const char* TypeExpr::getType() const { return type_; }
void TypeExpr::Accept(Visitor* visitor) const { visitor->visitType(this); }
void TypeExpr::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitType(this, symbolTable); }

// MemberAccessAssignment
MemberAccessAssignment::MemberAccessAssignment(ASTNode* memberAccess, ASTNode* expr) : memberAccess_(memberAccess), expr_(expr) {}
MemberAccessAssignment::~MemberAccessAssignment() {
    delete memberAccess_;
    delete expr_;
}
const ASTNode* MemberAccessAssignment::getMemberAccess() const { return memberAccess_; }
const ASTNode* MemberAccessAssignment::getExpr() const { return expr_; }
void MemberAccessAssignment::Accept(Visitor* visitor) const { visitor->visitMemberAccessAssignment(this); }
void MemberAccessAssignment::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const {
    visitor->visitMemberAccessAssignment(this, symbolTable);
}

// Keyword
Keyword::Keyword(char* keyword) : keyword_(keyword) {}
Keyword::~Keyword() { delete keyword_; }
const char* Keyword::getKeyword() const { return keyword_; }
void Keyword::Accept(Visitor* visitor) const { visitor->visitKeyword(this); }
void Keyword::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitKeyword(this, symbolTable); }

// Arg
Arg::Arg(TypeExpr* type, Identifier* identifier) : type(type), templatetype(nullptr), varname(identifier) {}
Arg::Arg(TemplateType* type, Identifier* identifier) : type(nullptr), templatetype(type), varname(identifier) {}
Arg::~Arg() {
    delete type;
    delete varname;
}
const TypeExpr* Arg::getType() const { return type; }
const TemplateType* Arg::getTemplateType() const { return templatetype; }
const Identifier* Arg::getVarName() const { return varname; }
void Arg::Accept(Visitor* visitor) const { visitor->visitArg(this); }
void Arg::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitArg(this, symbolTable); }

// Arglist
Arglist::Arglist() {}
Arglist::~Arglist() {
    for (Arg* arg : arglist)
        delete arg;
}
const std::vector<Arg*> Arglist::getArgList() const { return arglist; }
void Arglist::addarg(Arg* arg) { arglist.push_back(arg); }
void Arglist::Accept(Visitor* visitor) const { visitor->visitArglist(this); }
void Arglist::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitArglist(this, symbolTable); }

// Expression
Expression::Expression(ASTNode* node, ExpressionKind kind) : node_(node), kind_(kind) {}
const ASTNode* Expression::getExpression() const { return node_; }
ExpressionKind Expression::getKind() const { return kind_; }
void Expression::Accept(Visitor* visitor) const { visitor->visitExpression(this); }
void Expression::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitExpression(this, symbolTable); }

// Param
Param::Param(Expression* expr) : expr_(expr), paramAssignment_(nullptr) {}
Param::Param(ParameterAssignment* paramAssignment) : expr_(nullptr), paramAssignment_(paramAssignment) {}
Param::~Param() {
    delete expr_;
    delete paramAssignment_;
}
const Expression* Param::getExpr() const { return expr_; }
const ParameterAssignment* Param::getParamAssignment() const { return paramAssignment_; }
void Param::Accept(Visitor* visitor) const { visitor->visitParam(this); }
void Param::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitParam(this, symbolTable); }

// Paramlist
Paramlist::Paramlist() {}
Paramlist::~Paramlist() {
    for (Param* param : paramlist)
        delete param;
}
const std::vector<Param*> Paramlist::getParamList() const { return paramlist; }
void Paramlist::addparam(Param* param) { paramlist.push_back(param); }
void Paramlist::Accept(Visitor* visitor) const { visitor->visitParamlist(this); }
void Paramlist::Accept(MLIRVisitor* visitor, mlir::SymbolTable* symbolTable) const { visitor->visitParamlist(this, symbolTable); }
