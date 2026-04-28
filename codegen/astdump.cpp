#include "astdump.h"
#include <iostream>

using namespace std;

Beautifier::Beautifier() {
    tab_space = 0;
    tab_size = 4;
}

void Beautifier::operator++() {tab_space++;}

void Beautifier::operator--() {tab_space--;}

void Beautifier::print_tab() {
    for(size_t x = 0 ; x < tab_size*tab_space ; x++) {
        printf(" ");
    }
}

CodeGen::CodeGen() : result(0) {}

void CodeGen::visitDeclarationStmt(const DeclarationStatement* dclstmt) {
    beautifier.print_tab();
    cout << "Declaration Statememt: {\n";
    ++beautifier;

    TypeExpr* type         = static_cast<TypeExpr*>(dclstmt->gettype());
    Identifier* identifier = static_cast<Identifier*>(dclstmt->getvarname());
    Number* number         = static_cast<Number*>(dclstmt->getnumber());

    type->Accept(this);

    identifier->Accept(this);

    if(number) number->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitAdd(const Add* add) {
    beautifier.print_tab();
    cout << "Add Expression: {\n";
    ++beautifier;

    Expression* expr1 = static_cast<Expression*>(add->getOperand1());
    Expression* expr2 = static_cast<Expression*>(add->getOperand2());

    expr1->Accept(this);
    expr2->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitSub(const Sub* sub) {
    beautifier.print_tab();
    cout << "Subtraction Expression: {\n";
    ++beautifier;

    Expression* expr1 = static_cast<Expression*>(sub->getOperand1());
    Expression* expr2 = static_cast<Expression*>(sub->getOperand2());

    expr1->Accept(this);
    expr2->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitMul(const Mul* mul) {
    beautifier.print_tab();
    cout << "Multiplication Expression: {\n";
    ++beautifier;

    Expression* expr1 = static_cast<Expression*>(mul->getOperand1());
    Expression* expr2 = static_cast<Expression*>(mul->getOperand2());

    expr1->Accept(this);
    expr2->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitDiv(const Div* div) {
    beautifier.print_tab();
    cout << "Division Expression: {\n";
    ++beautifier;

    Expression* expr1 = static_cast<Expression*>(div->getOperand1());
    Expression* expr2 = static_cast<Expression*>(div->getOperand2());

    expr1->Accept(this);
    expr2->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitAnd(const And* _and) {
    beautifier.print_tab();
    cout << "And Expression: {\n";
    ++beautifier;

    Expression* expr1 = static_cast<Expression*>(_and->getOperand1());
    Expression* expr2 = static_cast<Expression*>(_and->getOperand2());

    expr1->Accept(this);
    expr2->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitOr(const Or* _or) {
    beautifier.print_tab();
    cout << "Or Expression: {\n";
    ++beautifier;

    Expression* expr1 = static_cast<Expression*>(_or->getOperand1());
    Expression* expr2 = static_cast<Expression*>(_or->getOperand2());

    expr1->Accept(this);
    expr2->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitMemberaccessStmt(const MemberacceessStmt* memberAccess) {
    memberAccess->getMemberAccess()->Accept(this);
    // std::abort();
}

void CodeGen::visitTemplateDeclarationStmt(const TemplateDeclarationStatement* templateDeclStmt) { cout << "Template Declaration\n\n"; }

void CodeGen::visitTemplateType(const TemplateType* templateType) { cout << "Template Type\n"; }

void CodeGen::visitForallStmt(const ForallStatement* forAllStmt) {
    beautifier.print_tab();
    std::cout << "Forall: {\n";
    ++beautifier;

    beautifier.print_tab();
    std::cout << "Loop Var: {\n";
    ++beautifier;

    forAllStmt->getLoopVar()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";

    beautifier.print_tab();
    std::cout << "Loop Expr: {\n";
    ++beautifier;

    forAllStmt->getexpr()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";

    beautifier.print_tab();
    std::cout << "Loop Body: {\n";
    ++beautifier;

    forAllStmt->getstmtlist()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitForStmt(const ForStatement* forStmt) {
    beautifier.print_tab();
    std::cout << "Forall: {\n";
    ++beautifier;

    beautifier.print_tab();
    std::cout << "Loop Var: {\n";
    ++beautifier;

    forStmt->getLoopVar()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";

    beautifier.print_tab();
    std::cout << "Loop Expr: {\n";
    ++beautifier;

    forStmt->getexpr()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";

    beautifier.print_tab();
    std::cout << "Loop Body: {\n";
    ++beautifier;

    forStmt->getstmtlist()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitDoWhileStmt(const DoWhileStatement* doWhileStmt) {
    beautifier.print_tab();
    std::cout << "DoWhile: {\n";
    ++beautifier;

    beautifier.print_tab();
    std::cout << "Loop Expr: {\n";
    ++beautifier;

    doWhileStmt->getexpr()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";

    beautifier.print_tab();
    std::cout << "Loop Body: {\n";
    ++beautifier;

    doWhileStmt->getstmtlist()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitIfStmt(const IfStatement* ifStmt) {
    beautifier.print_tab();
    std::cout << "If Statement: {\n";
    ++beautifier;

    ifStmt->getexpr()->Accept(this);
    ifStmt->getstmt()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";
}

void CodeGen::visitIfElseStmt(const IfElseStatement* ifElseStmt) {
    beautifier.print_tab();
    std::cout << "Linked If Statement: {\n";
    ++beautifier;
    ifElseStmt->getexpr()->Accept(this);
    ifElseStmt->getstmt1()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";

    beautifier.print_tab();
    std::cout << "Else Statement: {\n";
    ++beautifier;

    ifElseStmt->getstmt2()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";
}

void CodeGen::visitBoolExpr(const BoolExpr* boolExpr) { 
    beautifier.print_tab();
    std::cout << "Bool Expression {\n";
    ++beautifier;

    boolExpr->getExpr1()->Accept(this);
    beautifier.print_tab();
    std::cout << "Operator: " << boolExpr->getop() << "\n";
    boolExpr->getExpr2()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    std::cout << "}\n";
}

void CodeGen::visitIncandassignstmt(const Incandassignstmt* incandassignstmt) {
    beautifier.print_tab();
    std::cout << "Increment and Assign Statement: {\n";
    ++beautifier;

    incandassignstmt->getIdentifier()->Accept(this);
    incandassignstmt->getexpr()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitIncstmt(const Incstmt* incstmt) {
    beautifier.print_tab();
    std::cout << "Increment Statement: {\n";
    ++beautifier;

    incstmt->getIdentifier()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitAssignment(const Assignment* assignment) {
    beautifier.print_tab();
    std::cout << "Identifier: " << assignment->getIdentifier() << "\n";
    assignment->getexpr() -> Accept(this);
}

void CodeGen::visitAssignmentStmt(const AssignmentStmt* assignemntStmt) {
    beautifier.print_tab();
    std::cout << "Assignment Statement: {\n";
    ++beautifier;

    assignemntStmt->getAssignment()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitIdentifier(const Identifier* identifier) { 
    beautifier.print_tab();
    cout << "Identifier: " << identifier->getname() << "\n"; 
}

void CodeGen::visitReturnStmt(const ReturnStmt* returnStmt) {
    ASTNode* expr = returnStmt->getexpr();

    beautifier.print_tab();
    cout << "Return: {\n";
    ++beautifier;

    expr->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitParameterAssignment(const ParameterAssignment* paramAssignment) { 
    beautifier.print_tab();
    cout << "Parameter Assignment\n"; 
}

void CodeGen::visitParam(const Param* param) {
    beautifier.print_tab();
    cout << "Params\n\n"; 
}

void CodeGen::visitTupleAssignment(const TupleAssignment* tupleAssignment) { 
    beautifier.print_tab();
    cout << "Tuple Assignment\n"; 
}

void CodeGen::visitFunction(const Function* function) {
    Arglist* arglist        = static_cast<Arglist*>(function->getparams());
    Identifier* funcName    = static_cast<Identifier*>(function->getfuncname());
    Statementlist* stmtlist = static_cast<Statementlist*>(function->getstmtlist());

    beautifier.print_tab();
    cout << "Function: " << funcName->getname() << " {\n";
    ++beautifier;

    arglist->Accept(this);

    beautifier.print_tab();
    cout << "Function Body: {\n";
    ++beautifier;

    stmtlist->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitParamlist(const Paramlist* paramlist) { 
    beautifier.print_tab();
    cout << "ParamList\n\n"; 
}

void CodeGen::visitFixedpointUntil(const FixedpointUntil* fixedpointuntil) {
    Statementlist* stmtlist = static_cast<Statementlist*>(fixedpointuntil->getstmtlist());

    beautifier.print_tab();
    cout << "Fixed Point Until{\n";
    ++beautifier;

    stmtlist->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitInitialiseAssignmentStmt(const InitialiseAssignmentStmt* initialiseAssignmentStmt) { 
    beautifier.print_tab();
    cout << "Init Assignment Stmt {\n";
    ++beautifier;
    
    initialiseAssignmentStmt->gettype()->Accept(this);
    initialiseAssignmentStmt->getidentifier()->Accept(this);
    initialiseAssignmentStmt->getexpr()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitMemberAccessAssignment(const MemberAccessAssignment* memberAccessAssignment) { 
    beautifier.print_tab();
    cout << "Member Access Assignment\n\n"; 
}

void CodeGen::visitKeyword(const Keyword* keyword) {}

void CodeGen::visitGraphProperties(const GraphProperties* graphproperties) { 
    beautifier.print_tab();
    cout << graphproperties->getPropertyType() << " "; 
}

void CodeGen::visitMethodcall(const Methodcall* methodcall) {
    beautifier.print_tab();
    cout << "Methodcall: {\n";
    ++beautifier;

    methodcall->getIdentifier()->Accept(this);

    if (methodcall->getParamLists() != nullptr)
        methodcall->getParamLists()->Accept(this);
    
    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitMemberaccess(const Memberaccess* memberaccess) {
    beautifier.print_tab();
    cout << "Member Access: {\n";
    ++beautifier;

    if (memberaccess->getIdentifier() != nullptr)
        memberaccess->getIdentifier()->Accept(this);
    else if (memberaccess->getMemberAccessNode() != nullptr)
        memberaccess->getMemberAccessNode()->Accept(this);

    if (memberaccess->getIdentifier2() != nullptr)
        memberaccess->getIdentifier2()->Accept(this);

    if (memberaccess->getMethodCall() != nullptr)
        memberaccess->getMethodCall()->Accept(this);

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitArglist(const Arglist* arglist) {
    vector<Arg*> arglistV = arglist->getArgList();

    beautifier.print_tab();
    cout << "Arguments: {\n";
    ++beautifier;

    for (Arg* arg : arglistV) {
        beautifier.print_tab();
        cout << "Arg: {\n";
        ++beautifier;

        arg->Accept(this);

        --beautifier;
        beautifier.print_tab();
        cout << "}\n";
    }

    --beautifier;
    beautifier.print_tab();
    cout << "}\n";
}

void CodeGen::visitArg(const Arg* arg) {
    if (arg->getType()) {
        arg->getType()->Accept(this);

        arg->getVarName()->Accept(this);
    }

    else if (arg->getTemplateType()) {
        beautifier.print_tab();
        cout << "{";
        ++beautifier;
        beautifier.print_tab();
        cout << "PROPxx";
        --beautifier;
        beautifier.print_tab();
        cout << "}";
    }
}

void CodeGen::visitStatement(const Statement* statement) {
    statement->getstatement()->Accept(this);
}

void CodeGen::visitStatementlist(const Statementlist* stmtlist) {
    for (ASTNode* stmt : stmtlist->getStatementList()) {
        stmt->Accept(this);
    }
}

void CodeGen::visitType(const TypeExpr* type) { 
    beautifier.print_tab();
    cout << "Type: " << type->getType() << "\n"; 
}

void CodeGen::visitNumber(const Number* number) { 
    beautifier.print_tab();
    if(number->isFloat()) cout << "Decimal: " << number->getnumberfloat() << "\n";
    else cout << "Number: " << number->getnumber() << "\n"; 
}

void CodeGen::visitExpression(const Expression* expr) {
    expr->getExpression()->Accept(this);
}
