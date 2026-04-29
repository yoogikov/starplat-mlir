// #include "ast.h"
#include "includes/StarPlatDialect.h"
#include "includes/StarPlatOps.h"
#include "includes/StarPlatTypes.h"
#include <starplatIR.h>

// #include "mlir/IR/Builders.h"  // For mlir::OpBuilder
// #include "mlir/IR/Dialect.h"   // For mlir::Dialect
// #include "mlir/IR/Operation.h" // For mlir::Operation
//
// #include "mlir/IR/AsmState.h"
// #include "mlir/IR/BuiltinOps.h"
// #include "mlir/IR/MLIRContext.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Verifier.h"
// #include "mlir/Parser/Parser.h"
// #include <string>
//
// #include "mlir/Interfaces/CallInterfaces.h"
// #include "mlir/Interfaces/FunctionInterfaces.h"
// #include "mlir/Interfaces/SideEffectInterfaces.h"
// #include "mlir/Support/LLVM.h"
// #include "visitor.h"
// #include "llvm/Support/LogicalResult.h"
//

StarPlatCodeGen::StarPlatCodeGen()
    : context(), builder(&context), module(mlir::ModuleOp::create(builder.getUnknownLoc())), globalSymbolTable(module) {
    // Load Dialects here.
    context.getOrLoadDialect<mlir::starplat::StarPlatDialect>();
    context.getOrLoadDialect<mlir::scf::SCFDialect>();
    symbolTables.push_back(&globalSymbolTable);
    const_count = 0;
}

void StarPlatCodeGen::visitDeclarationStmt(const DeclarationStatement* dclstmt, mlir::SymbolTable* symbolTable) {
    TypeExpr* Type = static_cast<TypeExpr*>(dclstmt->gettype());
    Identifier* Id = static_cast<Identifier*>(dclstmt->getvarname());
    Number* Num    = static_cast<Number*>(dclstmt->getnumber());

    mlir::Type type;
    if (std::string(Type->getType()) == "int")
        type = mlir::starplat::SPIntType::get(builder.getContext());
    if (std::string(Type->getType()) == "float")
        type = mlir::starplat::SPFloatType::get(builder.getContext());
    auto declareOp = mlir::starplat::DeclareOp2::create(builder, builder.getUnknownLoc(), type, builder.getStringAttr(Id->getname()),
                                                        builder.getStringAttr("public"));

    if (!globalLookupOp(Id->getname()))
        symbolTable->insert(declareOp);

    if (Num) {
        // Declaration of the type "type IDENTIFIER EQ NUMBER SEMIC"
        mlir::Value lhs = declareOp;

        mlir::Value rhs;

        if (Num->isFloat()) {
            rhs = mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), builder.getF64Type(),
                                                  builder.getF64FloatAttr(Num->getnumberfloat()), "const_" +
                                                  to_string(get_const_count()), builder.getStringAttr("private"));
        }

        else {
            rhs = mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), builder.getI64Type(),
                                                  builder.getI64IntegerAttr(Num->getnumber()), "const_" +
                                                  to_string(get_const_count()), builder.getStringAttr("private"));
        }

        mlir::starplat::AssignmentOp::create(builder, builder.getUnknownLoc(), lhs, rhs);
    }
    // llvm::errs() << (num->getnumber());

    // llvm::StringRef graphID = "g";
    // if(std::string(Type->getType())=="int")
    // llvm::errs() << std::string(static_cast<TemplateType*>(dclstmt->gettype())->getType());
    // auto declareOp = mlir::starplat::DeclareOp::create(builder, builder.getUnknownLoc(), type, builder.getStringAttr(identifier->getname()),
    // visibility, graph); llvm::errs() << "decl happening\n"; mlir::starplat
}

void StarPlatCodeGen::visitTemplateDeclarationStmt(const TemplateDeclarationStatement* templateDeclStmt, mlir::SymbolTable* symbolTable) {

    TemplateType* Type          = static_cast<TemplateType*>(templateDeclStmt->gettype());
    Identifier* identifier      = static_cast<Identifier*>(templateDeclStmt->getvarname());

    const Identifier* graphName = Type->getGraphName();
    llvm::StringRef graphId     = graphName->getname();

    // TODO: Change the function name to getGraphProp().
    if (std::string(Type->getGraphPropNode()->getPropertyType()) == "propNode") {

        mlir::Type type;

        if (std::string(Type->getType()->getType()) == "int")
            type = builder.getType<mlir::starplat::PropNodeType>(builder.getI64Type(), graphId);
        else if (std::string(Type->getType()->getType()) == "bool")
            type = builder.getType<mlir::starplat::PropNodeType>(builder.getI1Type(), graphId);
        else if (std::string(Type->getType()->getType()) == "float")
            type = builder.getType<mlir::starplat::PropNodeType>(builder.getF64Type(), graphId);
        else {
            llvm::errs() << "Error: Type not implemented\n";
            exit(0);
        }

        auto visibility   = builder.getStringAttr("public");
        mlir::Value graph = globalLookupOp(graphId);

        mlir::Operation* declareOp;

        if (graph)
            declareOp = mlir::starplat::DeclareOp::create(builder, builder.getUnknownLoc(), type, builder.getStringAttr(identifier->getname()),
                                                          visibility, graph);
        else
            llvm::errs() << "Error: Undefined Symbol " << graphId << "\n";

        if (globalLookupOp(identifier->getname())) {
        }
        else
            symbolTable->insert(declareOp);
    }

    else if (std::string(Type->getGraphPropNode()->getPropertyType()) == "propEdge") {

        auto type         = builder.getType<mlir::starplat::PropEdgeType>(builder.getI64Type(), graphId);
        mlir::Value graph = NULL;
        auto declare      = mlir::starplat::DeclareOp::create(builder, builder.getUnknownLoc(), type, builder.getStringAttr(identifier->getname()),
                                                              builder.getStringAttr("public"), graph);
        symbolTable->insert(declare);
    }
}

void StarPlatCodeGen::visitTemplateType(const TemplateType* templateType, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitForallStmt(const ForallStatement* forAllStmt, mlir::SymbolTable* symbolTable) {

    Identifier* loopVar           = static_cast<Identifier*>(forAllStmt->getLoopVar());
    const Expression* expr        = static_cast<const Expression*>(forAllStmt->getexpr());
    const Statementlist* stmtlist = static_cast<const Statementlist*>(forAllStmt->getstmtlist());

    if (globalLookupOp(loopVar->getname())) {
        llvm::outs() << "Error: Identifier '" << loopVar->getname() << "' already declared.\n";
        exit(0);
    }

    if (expr->getKind() != ExpressionKind::KIND_MEMBERACCESS) {
        llvm::outs() << "Error: Expected member access expression in forall.\n";
        return;
    }

    const Memberaccess* memberaccess  = static_cast<const Memberaccess*>(expr->getExpression());
    const Methodcall* outermethodcall = static_cast<const Methodcall*>(memberaccess->getMethodCall());

    bool isNeighbours  = false;
    bool hasFilter     = false;
    bool lhsIsLoopVar  = false;
    bool rhsIsLoopVar  = false;
    mlir::Value graphSymbol;
    mlir::Value nodeSymbol;
    mlir::Value lhsPropVal;
    std::string filterOp;
    std::string lhsPropName;
    std::string lhsFilterName;
    std::string rhsFilterName;
    ExpressionKind lhsExprKind = ExpressionKind::KIND_IDENTIFIER;

    mlir::Type nodeType = mlir::starplat::NodeType::get(builder.getContext());

    // ---- CHAINED: g.nodes().filter(...) or g.neighbors(v).filter(...) ----
    if (memberaccess->getMemberAccessNode()) {
        const Memberaccess* inner     = static_cast<const Memberaccess*>(memberaccess->getMemberAccessNode());
        const Methodcall* innerMethod = static_cast<const Methodcall*>(inner->getMethodCall());

        graphSymbol = globalLookupOp(inner->getIdentifier()->getname());
        if (!graphSymbol) {
            llvm::outs() << "Error: Graph not found.\n";
            return;
        }

        if (strcmp(innerMethod->getIdentifier()->getname(), "neighbors") == 0) {
            isNeighbours = true;
            const Expression* paramExpr = static_cast<const Expression*>(innerMethod->getParamLists());
            const Identifier* paramId   = static_cast<const Identifier*>(paramExpr->getExpression());
            nodeSymbol = globalLookupOp(paramId->getname());
            if (!nodeSymbol) {
                llvm::outs() << "Error: Node '" << paramId->getname() << "' not found.\n";
                return;
            }
        } else if (strcmp(innerMethod->getIdentifier()->getname(), "nodes") == 0) {
            isNeighbours = false;
        } else {
            llvm::outs() << "Error: Unknown method '" << innerMethod->getIdentifier()->getname() << "'.\n";
            return;
        }

        // Check for filter — only store names, resolve after block creation
        if (outermethodcall && outermethodcall->getIsBuiltin() &&
            strcmp(outermethodcall->getIdentifier()->getname(), "filter") == 0) {
            hasFilter = true;
            const Expression* filterExpr = static_cast<const Expression*>(outermethodcall->getParamLists());
            const BoolExpr* boolExpr     = static_cast<const BoolExpr*>(filterExpr->getExpression());
            const Expression* lhsExpr    = static_cast<const Expression*>(boolExpr->getExpr1());
            const Expression* rhsExpr    = static_cast<const Expression*>(boolExpr->getExpr2());
            filterOp                     = std::string(boolExpr->getop());
            lhsExprKind                  = lhsExpr->getKind();

            // LHS
            if (lhsExpr->getKind() == ExpressionKind::KIND_MEMBERACCESS) {
                const Memberaccess* lhsMem = static_cast<const Memberaccess*>(lhsExpr->getExpression());
                const Identifier* nodeId   = static_cast<const Identifier*>(lhsMem->getIdentifier());
                const Identifier* propId   = static_cast<const Identifier*>(lhsMem->getIdentifier2());
                if (!nodeId || !propId) {
                    llvm::outs() << "Error: Malformed member access in filter.\n";
                    return;
                }
                lhsPropName = propId->getname();
                lhsPropVal  = globalLookupOp(propId->getname());
                if (!lhsPropVal) {
                    llvm::outs() << "Error: Undefined property '" << propId->getname() << "' in filter.\n";
                    return;
                }
                if (strcmp(nodeId->getname(), loopVar->getname()) == 0) {
                    lhsIsLoopVar = true;
                } else {
                    lhsFilterName = nodeId->getname();
                }
            } else if (lhsExpr->getKind() == ExpressionKind::KIND_IDENTIFIER) {
                const Identifier* lhsId = static_cast<const Identifier*>(lhsExpr->getExpression());
                if (strcmp(lhsId->getname(), loopVar->getname()) == 0) {
                    lhsIsLoopVar = true;
                } else {
                    lhsFilterName = lhsId->getname();
                }
            } else {
                llvm::outs() << "Error: Unsupported filter LHS expression.\n";
                return;
            }

            // RHS
            if (rhsExpr->getKind() == ExpressionKind::KIND_IDENTIFIER) {
                const Identifier* rhsId = static_cast<const Identifier*>(rhsExpr->getExpression());
                if (strcmp(rhsId->getname(), loopVar->getname()) == 0) {
                    rhsIsLoopVar = true;
                } else {
                    rhsFilterName = rhsId->getname();
                }
            } else if (rhsExpr->getKind() == ExpressionKind::KIND_KEYWORD) {
                const Keyword* rhsKw = static_cast<const Keyword*>(rhsExpr->getExpression());
                rhsFilterName = rhsKw->getKeyword();
            } else {
                llvm::outs() << "Error: Unsupported filter RHS expression.\n";
                return;
            }
        }
    }

    // ---- SIMPLE: g.nodes() or g.neighbors(v) ----
    else if (memberaccess->getMethodCall()) {
        const Methodcall* method = static_cast<const Methodcall*>(memberaccess->getMethodCall());

        graphSymbol = globalLookupOp(memberaccess->getIdentifier()->getname());
        if (!graphSymbol) {
            llvm::outs() << "Error: Graph not found.\n";
            return;
        }

        if (strcmp(method->getIdentifier()->getname(), "nodes") == 0) {
            isNeighbours = false;
        } else if (strcmp(method->getIdentifier()->getname(), "neighbors") == 0) {
            isNeighbours = true;
            const Expression* paramExpr = static_cast<const Expression*>(method->getParamLists());
            const Identifier* paramId   = static_cast<const Identifier*>(paramExpr->getExpression());
            nodeSymbol = globalLookupOp(paramId->getname());
            if (!nodeSymbol) {
                llvm::outs() << "Error: Node '" << paramId->getname() << "' not found.\n";
                return;
            }
        } else {
            llvm::outs() << "Error: Unknown method '" << method->getIdentifier()->getname() << "'.\n";
            return;
        }
    }

    // ---- Create the ForAll op ----
    mlir::Operation* loopOp;
    mlir::StringAttr loopa = builder.getStringAttr("loopa");

    if (!isNeighbours) {
        loopOp = mlir::starplat::ForAllNodesOp::create(builder, builder.getUnknownLoc(),
                                                        graphSymbol, loopa);
    } else {
        loopOp = mlir::starplat::ForAllNeighboursOp::create(builder, builder.getUnknownLoc(),
                                                             graphSymbol, nodeSymbol, loopa);
    }

    // ---- Set up the body ----
    auto& loopBlock = (isNeighbours ?
        llvm::cast<mlir::starplat::ForAllNeighboursOp>(loopOp).getBody() :
        llvm::cast<mlir::starplat::ForAllNodesOp>(loopOp).getBody()).emplaceBlock();

    loopBlock.addArgument(nodeType, builder.getUnknownLoc());
    nameToArgMap[loopVar->getname()] = loopBlock.getArgument(0);

    builder.setInsertionPointToStart(&loopBlock);

    mlir::SymbolTable forAllSymbolTable(loopOp);
    symbolTables.push_back(&forAllSymbolTable);

    if (hasFilter) {
        // ---- Resolve filter symbols ----
        mlir::Value lhsFilterSymbol;
        mlir::Value rhsFilterSymbol;

        // Resolve LHS
        if (lhsExprKind == ExpressionKind::KIND_MEMBERACCESS) {
            mlir::Value nodeVal = lhsIsLoopVar ? loopBlock.getArgument(0) : globalLookupOp(lhsFilterName);
            if (!nodeVal) {
                llvm::outs() << "Error: Undefined node '" << lhsFilterName << "' in filter.\n";
                return;
            }
            mlir::Type propElemType = builder.getI64Type();
            if (auto propNodeType = mlir::dyn_cast<mlir::starplat::PropNodeType>(lhsPropVal.getType()))
                propElemType = propNodeType.getParameter();
            lhsFilterSymbol = mlir::starplat::GetNodePropertyOp::create(
                builder, builder.getUnknownLoc(), propElemType,
                nodeVal, lhsPropVal,
                builder.getStringAttr(lhsPropName))->getResult(0);
        } else if (lhsIsLoopVar) {
            lhsFilterSymbol = loopBlock.getArgument(0);
        } else {
            lhsFilterSymbol = globalLookupOp(lhsFilterName);
            if (!lhsFilterSymbol) {
                llvm::outs() << "Error: Undefined identifier '" << lhsFilterName << "' in filter.\n";
                return;
            }
        }

        // Resolve RHS
        if (rhsIsLoopVar) {
            rhsFilterSymbol = loopBlock.getArgument(0);
        } else {
            rhsFilterSymbol = globalLookupOp(rhsFilterName);
            if (!rhsFilterSymbol) {
                llvm::outs() << "Error: Undefined identifier '" << rhsFilterName << "' in filter.\n";
                return;
            }
        }

        // ---- Emit filter cmp + spif ----
        auto cmpOp = mlir::starplat::CmpOp::create(builder, builder.getUnknownLoc(),
                                                    mlir::IntegerType::get(builder.getContext(), 1),
                                                    lhsFilterSymbol, rhsFilterSymbol,
                                                    builder.getStringAttr(filterOp));

        auto ifOp = mlir::starplat::StarPlatIfOp::create(builder, builder.getUnknownLoc(),
                                                          cmpOp->getResult(0),
                                                          builder.getStringAttr("spif"));
        auto& ifBlock = ifOp.getBody().emplaceBlock();
        builder.setInsertionPointToStart(&ifBlock);

        stmtlist->Accept(this, &forAllSymbolTable);

        mlir::starplat::endOp::create(builder, builder.getUnknownLoc());
        builder.setInsertionPointAfter(ifOp);
    } else {
        stmtlist->Accept(this, &forAllSymbolTable);
    }

    mlir::starplat::endOp::create(builder, builder.getUnknownLoc());
    builder.setInsertionPointAfter(loopOp);
}

void StarPlatCodeGen::visitForStmt(const ForStatement* forStmt, mlir::SymbolTable* symbolTable) {
    Identifier* loopVar           = static_cast<Identifier*>(forStmt->getLoopVar());
    const Expression* expr        = static_cast<const Expression*>(forStmt->getexpr());
    const Statementlist* stmtlist = static_cast<const Statementlist*>(forStmt->getstmtlist());

    if (expr->getKind() != ExpressionKind::KIND_MEMBERACCESS) {
        llvm::outs() << "Error: Expected member access in for stmt.\n";
        return;
    }

    const Memberaccess* memberaccess = static_cast<const Memberaccess*>(expr->getExpression());
    const Methodcall* methodcall     = static_cast<const Methodcall*>(memberaccess->getMethodCall());

    mlir::Value graphSymbol = globalLookupOp(memberaccess->getIdentifier()->getname());
    if (!graphSymbol) {
        llvm::outs() << "Error: Graph not found in for stmt.\n";
        return;
    }

    mlir::Value nodeSymbol;
    if (strcmp(methodcall->getIdentifier()->getname(), "nodes_to") == 0) {
        const Expression* paramExpr = static_cast<const Expression*>(methodcall->getParamLists());
        const Identifier* paramId   = static_cast<const Identifier*>(paramExpr->getExpression());
        nodeSymbol = globalLookupOp(paramId->getname());
        if (!nodeSymbol) {
            llvm::outs() << "Error: Node '" << paramId->getname() << "' not found.\n";
            return;
        }
    } else {
        llvm::outs() << "Error: Unknown method '" << methodcall->getIdentifier()->getname() << "' in for stmt.\n";
        return;
    }

    // Create ForOp
    mlir::StringAttr loopa = builder.getStringAttr("loopa");
    llvm::SmallVector<mlir::Value> forOperands = {graphSymbol, nodeSymbol};
    mlir::ArrayAttr loopAttr  = builder.getArrayAttr({builder.getStringAttr("nodes_to")});
    mlir::BoolAttr filterAttr = builder.getBoolAttr(false);

    auto forOp = mlir::starplat::ForOp::create(builder, builder.getUnknownLoc(),
                                                forOperands, loopAttr, filterAttr, loopa);

    // Set up body block
    auto& loopBlock = forOp.getBody().emplaceBlock();

    // Add loop var as block argument
    mlir::Type nodeType = mlir::starplat::NodeType::get(builder.getContext());
    loopBlock.addArgument(nodeType, builder.getUnknownLoc());
    nameToArgMap[loopVar->getname()] = loopBlock.getArgument(0);

    builder.setInsertionPointToStart(&loopBlock);

    mlir::SymbolTable forSymbolTable(forOp);
    symbolTables.push_back(&forSymbolTable);

    stmtlist->Accept(this, &forSymbolTable);

    mlir::starplat::endOp::create(builder, builder.getUnknownLoc());
    builder.setInsertionPointAfter(forOp);
}


void StarPlatCodeGen::visitDoWhileStmt(const DoWhileStatement* doWhileStmt, mlir::SymbolTable* symbolTable) {
    const Expression* expr        = static_cast<const Expression*>(doWhileStmt->getexpr());
    const Statementlist* stmtlist = static_cast<const Statementlist*>(doWhileStmt->getstmtlist());

    auto doWhileOp = mlir::starplat::DoWhileOp::create(builder, builder.getUnknownLoc(),
                                                        builder.getStringAttr("DoWhile"));
    auto& bodyBlock = doWhileOp.getBody().emplaceBlock();
    builder.setInsertionPointToStart(&bodyBlock);

    mlir::SymbolTable doWhileSymbolTable(doWhileOp);
    symbolTables.push_back(&doWhileSymbolTable);

    stmtlist->Accept(this, &doWhileSymbolTable);

    mlir::Value cond = resolveExpr(expr, &doWhileSymbolTable);
    if (!cond) { llvm::outs() << "Error: Failed to resolve do-while condition.\n"; return; }

    mlir::starplat::DoWhileCondOp::create(builder, builder.getUnknownLoc(), cond);
    builder.setInsertionPointAfter(doWhileOp);
}

void StarPlatCodeGen::visitMemberaccessStmt(const MemberacceessStmt* MemberacceessStmt, mlir::SymbolTable* symbolTable) {
    const Memberaccess* memberaccessnode = static_cast<const Memberaccess*>(MemberacceessStmt->getMemberAccess());
    const Methodcall* methodcallnode     = static_cast<const Methodcall*>(memberaccessnode->getMethodCall());
    
    if (!methodcallnode) {
        llvm::outs() << "Error: null methodcall\n";
        return;
    }

    const Paramlist* paramlist = static_cast<const Paramlist*>(methodcallnode->getParamLists());
    
    if (!paramlist) {
        llvm::outs() << "Error: null paramlist\n";
        return;
    }

    methodcallnode->Accept(this, symbolTable);
    std::vector<Param*> paramListVector = paramlist->getParamList();
    const Identifier* identifier1 = memberaccessnode->getIdentifier();
    auto graph                    = globalLookupOp(identifier1->getname());
    if (!graph) {
        llvm::outs() << "Error: Graph not defined!\n";
        return;
    }

    if (methodcallnode && methodcallnode->getIsBuiltin()) {
        if (std::strcmp(methodcallnode->getIdentifier()->getname(), "attachNodeProperty") == 0) {
            llvm::SmallVector<mlir::Value> operandsForAttachNodeProperty;
            operandsForAttachNodeProperty.push_back(graph);
            for (Param* param : paramListVector) {
                if (!param->getParamAssignment())
                    continue;
                const auto* paramAssignment = static_cast<const ParameterAssignment*>(param->getParamAssignment());
                if (!paramAssignment)
                    continue;
                const auto* identifier = static_cast<const Identifier*>(paramAssignment->getidentifier());
                auto idSymbol = globalLookupOp(identifier->getname());
                if (!idSymbol) {
                    llvm::outs() << "Error: Identifier '" << identifier->getname() << "' not declared.\n";
                    return;
                }

                mlir::Value kwSymbol;
                if (paramAssignment->isKeyword()) {
                    const auto* keyword = static_cast<const Keyword*>(paramAssignment->getkeyword());
                    keyword->Accept(this, symbolTable);
                    kwSymbol = globalLookupOp(keyword->getKeyword());
                } else {
                    const auto* expr = static_cast<const Expression*>(paramAssignment->getexpr());
                    kwSymbol = resolveExpr(expr, symbolTable);
                }

                if (!idSymbol || !kwSymbol)
                    return;

                operandsForAttachNodeProperty.push_back(idSymbol);
                operandsForAttachNodeProperty.push_back(kwSymbol);
            }
            mlir::starplat::AttachNodePropertyOp::create(builder, builder.getUnknownLoc(), operandsForAttachNodeProperty);
        }
        else {
            llvm::errs() << methodcallnode->getIdentifier()->getname() << " is not implemented yet\n";
        }
    }
}

void StarPlatCodeGen::visitIfStmt(const IfStatement* ifStmt, mlir::SymbolTable* symbolTable) {
    const Expression* condExpr = static_cast<const Expression*>(ifStmt->getexpr());

    mlir::Value cond;

    // Handle is_an_edge specially since it's a method call on graph
    if (condExpr->getKind() == ExpressionKind::KIND_MEMBERACCESS) {
        const Memberaccess* memberaccess = static_cast<const Memberaccess*>(condExpr->getExpression());
        const Methodcall* methodcall     = static_cast<const Methodcall*>(memberaccess->getMethodCall());

        if (methodcall && methodcall->getIsBuiltin() &&
            strcmp(methodcall->getIdentifier()->getname(), "is_an_edge") == 0) {
            auto graphSymbol = globalLookupOp(memberaccess->getIdentifier()->getname());
            if (!graphSymbol) {
                llvm::outs() << "Error: Graph not found in visitIfStmt\n";
                return;
            }
            const Paramlist* paramlist = static_cast<const Paramlist*>(methodcall->getParamLists());
            auto params                = paramlist->getParamList();
            const Identifier* id1     = static_cast<const Identifier*>(params[0]->getExpr()->getExpression());
            const Identifier* id2     = static_cast<const Identifier*>(params[1]->getExpr()->getExpression());
            auto node1Symbol          = globalLookupOp(id1->getname());
            auto node2Symbol          = globalLookupOp(id2->getname());
            if (!node1Symbol || !node2Symbol) {
                llvm::outs() << "Error: Node symbols not found in visitIfStmt\n";
                return;
            }
            cond = mlir::starplat::IsEdgeOp::create(builder, builder.getUnknownLoc(),
                                                     mlir::IntegerType::get(builder.getContext(), 1),
                                                     graphSymbol, node1Symbol, node2Symbol)->getResult(0);
        } else {
            cond = resolveExpr(condExpr, symbolTable);
        }
    } else {
        cond = resolveExpr(condExpr, symbolTable);
    }

    if (!cond) {
        llvm::outs() << "Error: Failed to resolve if condition.\n";
        return;
    }

    auto ifOp     = mlir::starplat::StarPlatIfOp::create(builder, builder.getUnknownLoc(),
                                                          cond, builder.getStringAttr("spif"));
    auto& ifBlock = ifOp.getBody().emplaceBlock();
    builder.setInsertionPointToStart(&ifBlock);

    mlir::SymbolTable ifSymbolTable(ifOp);
    symbolTables.push_back(&ifSymbolTable);

    const ASTNode* body = ifStmt->getstmt();
    body->Accept(this, &ifSymbolTable);

    mlir::starplat::endOp::create(builder, builder.getUnknownLoc());
    builder.setInsertionPointAfter(ifOp);
}

void StarPlatCodeGen::visitIfElseStmt(const IfElseStatement* ifElseStmt, mlir::SymbolTable* symbolTable) {
    const Expression* condExpr = static_cast<const Expression*>(ifElseStmt->getexpr());
    
    // Resolve condition using resolveExpr
    mlir::Value cond = resolveExpr(condExpr, symbolTable);
    if (!cond) {
        llvm::outs() << "Error: Failed to resolve if-else condition.\n";
        return;
    }

    // Create IfElseOp
    auto ifElseOp = mlir::starplat::StarPlatIfElseOp::create(builder, builder.getUnknownLoc(),
                                                               cond, builder.getStringAttr("ifelse"));

    // ---- If body ----
    auto& ifBlock = ifElseOp.getIfBody().emplaceBlock();
    builder.setInsertionPointToStart(&ifBlock);

    mlir::SymbolTable ifSymbolTable(ifElseOp);
    symbolTables.push_back(&ifSymbolTable);

    const ASTNode* stmt1 = ifElseStmt->getstmt1();
    stmt1->Accept(this, &ifSymbolTable);

    mlir::starplat::endOp::create(builder, builder.getUnknownLoc());

    // ---- Else body ----
    auto& elseBlock = ifElseOp.getElseBody().emplaceBlock();
    builder.setInsertionPointToStart(&elseBlock);

    const ASTNode* stmt2 = ifElseStmt->getstmt2();
    stmt2->Accept(this, &ifSymbolTable);

    mlir::starplat::endOp::create(builder, builder.getUnknownLoc());

    builder.setInsertionPointAfter(ifElseOp);
}

void StarPlatCodeGen::visitBoolExpr(const BoolExpr* boolExpr, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitIncandassignstmt(const Incandassignstmt* stmt, mlir::SymbolTable* symbolTable) {
    auto lhsSymbol = globalLookupOp(stmt->getIdentifier()->getname());
    if (!lhsSymbol) {
        llvm::outs() << "Error: Identifier '" << stmt->getIdentifier()->getname() << "' not declared.\n";
        return;
    }
    const Expression* rhsExpr = static_cast<const Expression*>(stmt->getexpr());
    mlir::Value rhs = resolveExpr(rhsExpr, symbolTable);
    if (!rhs) {
        llvm::outs() << "Error: Failed to resolve RHS of inc assign.\n";
        return;
    }
    mlir::starplat::IncAndAssignOp::create(builder, builder.getUnknownLoc(), lhsSymbol, rhs);
}

void StarPlatCodeGen::visitIncstmt(const Incstmt* incstmt, mlir::SymbolTable* symbolTable) {
    auto lhsSymbol = globalLookupOp(incstmt->getIdentifier()->getname());
    if (!lhsSymbol) {
        llvm::outs() << "Error: Identifier '" << incstmt->getIdentifier()->getname() << "' not declared.\n";
        return;
    }
    mlir::Type intType = mlir::IntegerType::get(builder.getContext(), 64);
    auto constAttr     = mlir::IntegerAttr::get(intType, 1);
    auto constOp       = mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), intType, constAttr,
                                                          builder.getStringAttr("inc_" + std::string(incstmt->getIdentifier()->getname())),
                                                          builder.getStringAttr("private"));
    mlir::starplat::IncAndAssignOp::create(builder, builder.getUnknownLoc(), lhsSymbol, constOp->getResult(0));

}


void StarPlatCodeGen::visitAssignment(const Assignment* assignment, mlir::SymbolTable* symbolTable) {
    static_cast<Expression*>(assignment->getexpr())->Accept(this, symbolTable);
}

void StarPlatCodeGen::visitAssignmentStmt(const AssignmentStmt* assignemntStmt, mlir::SymbolTable* symbolTable) {
    Assignment* assignment = static_cast<Assignment*>(assignemntStmt->getAssignment());
    char* identifier       = assignment->getIdentifier();
    Expression* expr       = static_cast<Expression*>(assignment->getexpr());

    auto lhs = globalLookupOp(identifier);
    if (!lhs) {
        llvm::errs() << "Error: Undefined identifier '" << identifier << "'\n";
        return;
    }

    mlir::Value rhs = resolveExpr(expr, symbolTable);
    if (!rhs) {
        llvm::errs() << "Error: Failed to resolve RHS of assignment.\n";
        return;
    }

    mlir::starplat::StoreOp::create(builder, builder.getUnknownLoc(), lhs, rhs);
}

void StarPlatCodeGen::visitIdentifier(const Identifier* identifier, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitReturnStmt(const ReturnStmt* returnStmt, mlir::SymbolTable* symbolTable) {
    const Expression* expr = static_cast<const Expression*>(returnStmt->getexpr());

    if (expr->getKind() == KIND_NUMBER) {
        const Number* number = static_cast<const Number*>(expr->getExpression());
        mlir::Type intType   = mlir::IntegerType::get(builder.getContext(), 64);
        auto constAttr       = mlir::IntegerAttr::get(intType, number->getnumber());
        int const_count      = get_const_count();
        auto constOp =
            mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), intType, constAttr,
                                            builder.getStringAttr(std::string("const_") + to_string(const_count)), builder.getStringAttr("public"));
        auto retSymbol = constOp->getResult(0);
        mlir::starplat::ReturnOp::create(builder, builder.getUnknownLoc(), mlir::ValueRange{retSymbol});
    }

    else if (expr->getKind() == KIND_IDENTIFIER) {
        const Identifier* identifier = static_cast<const Identifier*>(expr->getExpression());

        auto retSymbol               = globalLookupOp(identifier->getname());
        if (!retSymbol) {
            llvm::outs() << "Error: Return value '" << identifier->getname() << "' not found.\n";
            mlir::starplat::ReturnOp::create(builder, builder.getUnknownLoc(), mlir::ValueRange{});
            return;
        }

        mlir::starplat::ReturnOp::create(builder, builder.getUnknownLoc(), mlir::ValueRange{retSymbol});
    }
}
void StarPlatCodeGen::visitParameterAssignment(const ParameterAssignment* paramAssignment, mlir::SymbolTable* symbolTable) {
    Identifier* identifier = static_cast<Identifier*>(paramAssignment->getidentifier());
    if (paramAssignment->isKeyword()) {
        Keyword* keyword = static_cast<Keyword*>(paramAssignment->getkeyword());
        if (keyword) keyword->Accept(this, symbolTable);
    }

    if (globalLookupOp(identifier->getname())) {
        // auto lhs = globalLookupOp(identifier->getname());
        // auto rhs = globalLookupOp(keyword->getKeyword());
        //
        // auto assign =
        // mlir::starplat::AssignmentOp::create(builder,builder.getUnknownLoc(),
        // lhs->getResult(0), rhs->getResult(0)); symbolTable->rename(assign,
        // builder.getStringAttr(identifier->getname()));
    }

    else if (globalLookupOp(identifier->getname()) != nullptr) {

        // auto lhs = globalLookupOp(identifier->getname());
        // auto rhs = globalLookupOp(keyword->getKeyword());

        // auto assign =
        // mlir::starplat::AssignmentOp::create(builder,builder.getUnknownLoc(),
        // lhs->getResult(0), rhs->getResult(0)); symbolTable->rename(assign,
        // builder.getStringAttr(identifier->getname()));
    }
    else {
        llvm::outs() << "error: " << identifier->getname() << " not declared -_-\n";
        exit(1);
    }
}

void StarPlatCodeGen::visitParam(const Param* param, mlir::SymbolTable* symbolTable) {
    const ParameterAssignment* paramAssignment = static_cast<const ParameterAssignment*>(param->getParamAssignment());
    if (paramAssignment != nullptr) {
        paramAssignment->Accept(this, symbolTable);
    }
}

void StarPlatCodeGen::visitTupleAssignment(const TupleAssignment* tupleAssignment, mlir::SymbolTable* symbolTable) {

    const Expression* lhsexpr1 = static_cast<const Expression*>(tupleAssignment->getlhsexpr1());
    const Expression* lhsexpr2 = static_cast<const Expression*>(tupleAssignment->getlhsexpr2());
    const Expression* rhsexpr1 = static_cast<const Expression*>(tupleAssignment->getrhsexpr1());
    const Expression* rhsexpr2 = static_cast<const Expression*>(tupleAssignment->getrhsexpr2());

    mlir::Value lhsNode1, lhsProp1, lhsNode2, lhsProp2;

    // ---- LHS1 ----
    if (lhsexpr1->getKind() == ExpressionKind::KIND_MEMBERACCESS) {
        const Memberaccess* lhs1 = static_cast<const Memberaccess*>(lhsexpr1->getExpression());
        if (!lhs1->getIdentifier() || !lhs1->getIdentifier2()) {
            llvm::outs() << "Error: Tuple Assignment failed.\n"; return;
        }
        lhsNode1 = globalLookupOp(lhs1->getIdentifier()->getname());
        lhsProp1 = globalLookupOp(lhs1->getIdentifier2()->getname());
        if (!lhsNode1 || !lhsProp1) { llvm::outs() << "Error: Undefined var/prop in lhs1.\n"; return; }
    } else { llvm::outs() << "Error: Tuple Assignment failed.\n"; return; }

    // ---- LHS2 ----
    if (lhsexpr2->getKind() == ExpressionKind::KIND_MEMBERACCESS) {
        const Memberaccess* lhs2 = static_cast<const Memberaccess*>(lhsexpr2->getExpression());
        if (!lhs2->getIdentifier() || !lhs2->getIdentifier2()) {
            llvm::outs() << "Error: Tuple Assignment failed.\n"; return;
        }
        lhsNode2 = globalLookupOp(lhs2->getIdentifier()->getname());
        lhsProp2 = globalLookupOp(lhs2->getIdentifier2()->getname());
        if (!lhsNode2 || !lhsProp2) { llvm::outs() << "Error: Undefined var/prop in lhs2.\n"; return; }
    } else { llvm::outs() << "Error: Tuple Assignment failed.\n"; return; }

    // ---- RHS1: Min(...) ----
    if (rhsexpr1->getKind() != ExpressionKind::KIND_METHODCALL) {
        llvm::outs() << "Error: Tuple Assignment failed.\n"; return;
    }
    const Methodcall* rhsMethodCall = static_cast<const Methodcall*>(rhsexpr1->getExpression());
    if (!rhsMethodCall->getIsBuiltin() || strcmp(rhsMethodCall->getIdentifier()->getname(), "Min") != 0) {
        llvm::outs() << rhsMethodCall->getIdentifier()->getname() << " Not implemented yet.\n"; return;
    }
    const Paramlist* rhsParamList = static_cast<const Paramlist*>(rhsMethodCall->getParamLists());
    const Expression* minArg1     = static_cast<const Expression*>(rhsParamList->getParamList()[0]->getExpr());
    const Expression* minArg2     = static_cast<const Expression*>(rhsParamList->getParamList()[1]->getExpr());

    mlir::Value rhsValue1 = resolveExpr(minArg1, symbolTable);
    mlir::Value rhsValue2 = resolveExpr(minArg2, symbolTable);
    if (!rhsValue1 || !rhsValue2) { llvm::outs() << "Error: Failed to resolve Min args.\n"; return; }

    // ---- RHS2: updateValue ----
    mlir::Value updateValue = resolveExpr(rhsexpr2, symbolTable);
    if (!updateValue) { llvm::outs() << "Error: Failed to resolve update value.\n"; return; }

    // ---- Create MinOp ----
    mlir::starplat::MinOp::create(builder, builder.getUnknownLoc(),
        lhsNode1, lhsProp1,
        lhsNode2, lhsProp2,
        rhsValue1, rhsValue2,
        updateValue);
}

void StarPlatCodeGen::visitAdd(const Add* add, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitSub(const Sub* sub, mlir::SymbolTable* symbolTable) {}
    
void StarPlatCodeGen::visitMul(const Mul* mul, mlir::SymbolTable* symbolTable) {}
    
void StarPlatCodeGen::visitDiv(const Div* div, mlir::SymbolTable* symbolTable) {}
    
void StarPlatCodeGen::visitAnd(const And* _and, mlir::SymbolTable* symbolTable) {}
    
void StarPlatCodeGen::visitOr(const Or* _or, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitFunction(const Function* function, mlir::SymbolTable* symbolTable) {
    // Create function type.
    llvm::SmallVector<mlir::Type> argTypes;
    llvm::SmallVector<mlir::Attribute> argNames;
    llvm::SmallVector<mlir::Type> arg4Builder;

    std::vector<mlir::Operation*> ops;

    auto args = function->getparams()->getArgList();

    for (auto arg : args) {
        if (arg->getType() != nullptr) {
            if (std::string(arg->getType()->getType()) == "Graph") {
                argTypes.push_back(builder.getType<mlir::starplat::GraphType>());
            }
            else if (std::string(arg->getType()->getType()) == "Node") {
                argTypes.push_back(builder.getType<mlir::starplat::NodeType>());
            }
            else if (std::string(arg->getType()->getType()) == "float") {
                argTypes.push_back(mlir::starplat::SPFloatType::get(builder.getContext()));
            }
            else if (std::string(arg->getType()->getType()) == "int") {
                argTypes.push_back(mlir::starplat::SPIntType::get(builder.getContext()));
            }
        }
        else if (arg->getTemplateType() != nullptr) {
            llvm::StringRef graphId = arg->getTemplateType()->getGraphName()->getname();

            // Determine element type from template parameter
            mlir::Type elemType = builder.getI64Type(); // default
            if (std::string(arg->getTemplateType()->getType()->getType()) == "float")
                elemType = builder.getF64Type();
            else if (std::string(arg->getTemplateType()->getType()->getType()) == "bool")
                elemType = builder.getI1Type();

            if (std::string(arg->getTemplateType()->getGraphPropNode()->getPropertyType()) == "propNode") {
                argTypes.push_back(builder.getType<mlir::starplat::PropNodeType>(elemType, graphId));
            }
            else if (std::string(arg->getTemplateType()->getGraphPropNode()->getPropertyType()) == "propEdge") {
                argTypes.push_back(builder.getType<mlir::starplat::PropEdgeType>(elemType, graphId));
            }
        }
        argNames.push_back(builder.getStringAttr(arg->getVarName()->getname()));
    }
    // auto ret = function->

    auto funcType                = builder.getFunctionType(argTypes, {builder.getI64Type()});
    mlir::ArrayAttr argNamesAttr = builder.getArrayAttr(argNames);

    auto func = mlir::starplat::FuncOp::create(builder, builder.getUnknownLoc(), function->getfuncNameIdentifier(), funcType, argNamesAttr);
    // func.setNested();

    module.push_back(func);
    auto& entryBlock = func.getBody().emplaceBlock();

    int idx          = 0;
    for (auto arg : funcType.getInputs()) {
        auto argval                                                        = entryBlock.addArgument(arg, builder.getUnknownLoc());
        auto argName                                                       = argNames[idx++];

        nameToArgMap[mlir::dyn_cast<mlir::StringAttr>(argName).getValue()] = argval;
    }

    // Visit the function body.

    Statementlist* stmtlist = static_cast<Statementlist*>(function->getstmtlist());

    mlir::SymbolTable funcSymbolTable(func);

    symbolTables.push_back(&funcSymbolTable);

    builder.setInsertionPointToStart(&entryBlock);

    // for (auto argItr : args)
    // {

    //     auto argOp =
    //     mlir::starplat::ArgOp::create(builder,builder.getUnknownLoc(),
    //     builder.getI64Type(), argTypes[idx++],
    //     builder.getStringAttr(argItr->getVarName()->getname()),
    //     builder.getStringAttr("public")); funcSymbolTable.insert(argOp);
    // }

    // printf("Visiting Body!!\n");

    if (stmtlist->getStatementList().size() != 0) {
        // printf("Inside If %zu\n", stmtlist->getStatementList().size());
        stmtlist->Accept(this, &funcSymbolTable);
    }

    // Create end operation.
    // auto returnOp =
    // mlir::starplat::ReturnOp::create(builder, builder.getUnknownLoc());
}

void StarPlatCodeGen::visitParamlist(const Paramlist* paramlist, mlir::SymbolTable* symbolTable) {
    std::vector<Param*> paramListVecvtor = paramlist->getParamList();

    for (Param* param : paramListVecvtor)
        param->Accept(this, symbolTable);
}

void StarPlatCodeGen::visitFixedpointUntil(const FixedpointUntil* fixedpointuntil, mlir::SymbolTable* symbolTable) {
    // Create new region.
    // Create new symbol table.
    // Pass everything in symbol table to new symbol table.
    // Then Generate Code.

    Identifier* identifier  = static_cast<Identifier*>(fixedpointuntil->getidentifier());
    Expression* expr        = static_cast<Expression*>(fixedpointuntil->getexpr());
    Statementlist* stmtlist = static_cast<Statementlist*>(fixedpointuntil->getstmtlist());

    if (!globalLookupOp(identifier->getname())) {
        llvm::errs() << "Error: " << identifier->getname() << " not declared.\n";
        return;
    }

    mlir::StringAttr opAttr;
    const BoolExpr* boolExpr;
    if (expr->getKind() == ExpressionKind::KIND_BOOLEXPR) {
        boolExpr = static_cast<const BoolExpr*>(expr->getExpression());

        if (strcmp(boolExpr->getop(), "!") == 0)
            opAttr = builder.getStringAttr("NOT");
    }

    mlir::ArrayAttr condAttrArray = builder.getArrayAttr({opAttr});

    mlir::Value lhs               = globalLookupOp(identifier->getname());
    if (!lhs) {
        llvm::errs() << "Error: " << identifier->getname() << " not declared.\n";
        return;
    }

    const Expression* innerBoolExpr = static_cast<const Expression*>(boolExpr->getExpr1());
    mlir::Value rhs;
    if (innerBoolExpr->getKind() == ExpressionKind::KIND_IDENTIFIER) {
        rhs = globalLookupOp(static_cast<const Identifier*>(innerBoolExpr->getExpression())->getname());
        if (!rhs) {
            llvm::errs() << "Error: " << static_cast<const Identifier*>(innerBoolExpr->getExpression())->getname() << " not declared.\n";
            return;
        }
    }

    llvm::SmallVector<mlir::Value> condArgs = {lhs, rhs};
    mlir::StringAttr fixPntAttr             = builder.getStringAttr("FixedPnt");
    auto fixedPointUntil = mlir::starplat::FixedPointUntilOp::create(builder, builder.getUnknownLoc(), condArgs, condAttrArray, fixPntAttr);

    fixedPointUntil.setNested();

    // Change block.
    auto& loopBlock = fixedPointUntil.getBody().emplaceBlock();
    builder.setInsertionPointToStart(&loopBlock);

    mlir::SymbolTable fixedSymbolTable(fixedPointUntil);
    symbolTables.push_back(&fixedSymbolTable);

    stmtlist->Accept(this, &fixedSymbolTable);

    mlir::starplat::endOp::create(builder, builder.getUnknownLoc());
    builder.setInsertionPointAfter(fixedPointUntil);
}

void StarPlatCodeGen::visitInitialiseAssignmentStmt(const InitialiseAssignmentStmt* initialiseAssignmentStmt, mlir::SymbolTable* symbolTable) {
    const TypeExpr* type         = static_cast<const TypeExpr*>(initialiseAssignmentStmt->gettype());
    const Identifier* identifier = static_cast<const Identifier*>(initialiseAssignmentStmt->getidentifier());
    const Expression* expr       = static_cast<const Expression*>(initialiseAssignmentStmt->getexpr());

    mlir::Type typeAttr;

    mlir::Value lhs = NULL;
    mlir::Value rhs = NULL;

    if (strcmp(type->getType(), "int") == 0)
        typeAttr = mlir::starplat::SPIntType::get(builder.getContext());

    else if (strcmp(type->getType(), "bool") == 0)
        typeAttr = builder.getI1Type();

    else if (strcmp(type->getType(), "edge") == 0)
        typeAttr = mlir::starplat::EdgeType::get(builder.getContext());

    else if (strcmp(type->getType(), "float") == 0)
        typeAttr = mlir::starplat::SPFloatType::get(builder.getContext());

    else {
            llvm::errs() << "Error: Type not implemented\n";
            exit(0);
    }

    auto idDecl       = mlir::starplat::DeclareOp2::create(builder, builder.getUnknownLoc(), typeAttr, builder.getStringAttr(identifier->getname()),
                                                          builder.getStringAttr("public"));
    lhs               = idDecl.getResult();

    symbolTable->insert(idDecl);

    expr->Accept(this, symbolTable);

    mlir::Value op;
    if (expr->getKind() == ExpressionKind::KIND_KEYWORD) {
        const Keyword* keyword = static_cast<const Keyword*>(expr->getExpression());
        if (globalLookupOp(keyword->getKeyword()))
            op = globalLookupOp(keyword->getKeyword());

        // auto asgOp =
        mlir::starplat::AssignmentOp::create(builder, builder.getUnknownLoc(), idDecl.getResult(), op);
    }

    else if (expr->getKind() == ExpressionKind::KIND_MEMBERACCESS) {
        const Memberaccess* memberAccessIn = static_cast<const Memberaccess*>(expr->getExpression());
        const Identifier* identifierIn     = static_cast<const Identifier*>(memberAccessIn->getIdentifier());
        const Methodcall* methodcallIn     = static_cast<const Methodcall*>(memberAccessIn->getMethodCall());

        if (!globalLookupOp(identifierIn->getname())) {
            llvm::outs() << "Error: Undefined variable " << identifierIn->getname() << "\n";
            return;
        }
        auto accessIdentifier = globalLookupOp(identifierIn->getname());

        if (methodcallIn->getIsBuiltin()) {
            if (strcmp(methodcallIn->getIdentifier()->getname(), "get_edge") == 0) {
                // Visit ParamList
                const Paramlist* paramlist = static_cast<const Paramlist*>(methodcallIn->getParamLists());
                paramlist->Accept(this, symbolTable);

                std::vector<Param*> paramlistvector = paramlist->getParamList();
                const Identifier* node1             = static_cast<const Identifier*>(paramlistvector[0]->getExpr()->getExpression());
                const Identifier* node2             = static_cast<const Identifier*>(paramlistvector[1]->getExpr()->getExpression());

                auto node1Op                        = globalLookupOp(node1->getname());
                auto node2Op                        = globalLookupOp(node2->getname());

                // Create a get_edge Op.
                auto getedgeOp = mlir::starplat::GetEdgeOp::create(builder, builder.getUnknownLoc(), typeAttr, accessIdentifier, node1Op, node2Op);
                rhs            = getedgeOp.getResult();
                // Assign getedgeOp to iDecl.
                // Work here tomorrow.
                mlir::starplat::AssignmentOp::create(builder, builder.getUnknownLoc(), lhs, rhs);
            }
            else if (strcmp(methodcallIn->getIdentifier()->getname(), "num_nodes") == 0) {
                auto numNodesOp = mlir::starplat::NumNodesOp::create(
                    builder, builder.getUnknownLoc(),
                    builder.getI64Type(),  // always returns i64 from lower layer
                    accessIdentifier);
                
                // Cast to target type if different
                if (typeAttr == mlir::starplat::SPFloatType::get(builder.getContext())) {
                    auto castOp = mlir::starplat::CastOp::create(
                        builder, builder.getUnknownLoc(),
                        builder.getF64Type(), 
                        numNodesOp.getResult());
                    rhs = castOp.getResult();
                } else {
                    rhs = numNodesOp.getResult();
                }
                
                mlir::starplat::AssignmentOp::create(builder, builder.getUnknownLoc(), lhs, rhs);
            }
        }

    }
    else {
    // General expression — use resolveExpr
        rhs = resolveExpr(expr, symbolTable);
        if (!rhs) {
            llvm::outs() << "Error: Failed to resolve expr in init assignment.\n";
            return;
        }
        mlir::starplat::AssignmentOp::create(builder, builder.getUnknownLoc(), lhs, rhs);
    }
}

void StarPlatCodeGen::visitMemberAccessAssignment(const MemberAccessAssignment* memberAccessAssignment, mlir::SymbolTable* symbolTable) {

    const Memberaccess* memberAccess = static_cast<const Memberaccess*>(memberAccessAssignment->getMemberAccess());
    memberAccess->Accept(this, symbolTable);

    const Expression* expr = static_cast<const Expression*>(memberAccessAssignment->getExpr());
    expr->Accept(this, symbolTable);

    const Identifier* identifier  = memberAccess->getIdentifier();
    const Identifier* identifier2 = memberAccess->getIdentifier2();

    auto id1                      = globalLookupOp(identifier->getname());
    auto id2                      = globalLookupOp(identifier2->getname());

    if (!id1) {
        llvm::errs() << "Error: " << identifier->getname() << " not declared.\n";
        return;
    }

    if (!id2) {
        llvm::errs() << "Error: " << identifier2->getname() << " not declared.\n";
        return;
    }

    mlir::Type type = id1.getType();

    if (mlir::isa<mlir::starplat::NodeType>(type)) {
        // Set Node Property
        if (expr->getKind() == ExpressionKind::KIND_NUMBER) {
            const Number* number = static_cast<const Number*>(expr->getExpression());
            auto numberVal       = globalLookupOp(std::to_string(number->getnumber()));

            if (id2.getDefiningOp() != nullptr)
                // auto setNodeProp =
                mlir::starplat::SetNodePropertyOp::create(builder, builder.getUnknownLoc(), id2.getDefiningOp()->getOperand(0), id1, id2, numberVal);
            else
                // auto setNodeProp =
                mlir::starplat::SetNodePropertyOp::create(builder, builder.getUnknownLoc(), id2, id1, id2, numberVal);
        }

        else if (expr->getKind() == ExpressionKind::KIND_KEYWORD) {
            const Keyword* keyword = static_cast<const Keyword*>(expr->getExpression());
            auto keywordVal        = globalLookupOp(keyword->getKeyword());
            // auto setNodeProp =
            mlir::starplat::SetNodePropertyOp::create(builder, builder.getUnknownLoc(), id2.getDefiningOp()->getOperand(0), id1, id2, keywordVal);
        }

        else {
            mlir::Value rhs = resolveExpr(expr, symbolTable);
            if (!rhs) {
                llvm::outs() << "Error: Failed to resolve RHS in member access assignment.\n";
                return;
            }
            mlir::starplat::SetNodePropertyOp::create(builder, builder.getUnknownLoc(),
                id2.getDefiningOp()->getOperand(0), id1, id2, rhs);
        }
    }
}

void StarPlatCodeGen::visitKeyword(const Keyword* keyword, mlir::SymbolTable* symbolTable) {

    if (globalLookupOp(keyword->getKeyword()))
        return;

    mlir::Operation* keywordSSA;

    if (strcmp(keyword->getKeyword(), "False") == 0)
        keywordSSA =
            mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), builder.getI1Type(), builder.getStringAttr(keyword->getKeyword()),
                                            builder.getStringAttr(keyword->getKeyword()), builder.getStringAttr("public"));

    else if (strcmp(keyword->getKeyword(), "True") == 0)
        keywordSSA =
            mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), builder.getI1Type(), builder.getStringAttr(keyword->getKeyword()),
                                            builder.getStringAttr(keyword->getKeyword()), builder.getStringAttr("public"));
    else
        keywordSSA =
            mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), builder.getI64Type(), builder.getStringAttr(keyword->getKeyword()),
                                            builder.getStringAttr(keyword->getKeyword()), builder.getStringAttr("public"));

    symbolTable->insert(keywordSSA);
}

void StarPlatCodeGen::visitGraphProperties(const GraphProperties* graphproperties, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitMethodcall(const Methodcall* methodcall, mlir::SymbolTable* symbolTable) {
    const Paramlist* paramlist = static_cast<const Paramlist*>(methodcall->getParamLists());
    paramlist->Accept(this, symbolTable);
}

void StarPlatCodeGen::visitMemberaccess(const Memberaccess* memberaccess, mlir::SymbolTable* symbolTable) {

    if (!globalLookupOp(builder.getStringAttr(memberaccess->getIdentifier()->getname()))) {
        if (globalLookupOp(memberaccess->getIdentifier()->getname()) == nullptr) {
            llvm::outs() << "Error1: " << memberaccess->getIdentifier()->getname() << " not defined!\n";
            exit(0);
        }
    }

    if (memberaccess->getIdentifier2()) {
        if (!globalLookupOp(builder.getStringAttr(memberaccess->getIdentifier2()->getname()))) {
            if (globalLookupOp(memberaccess->getIdentifier()->getname()) == nullptr) {
                llvm::outs() << "Error2: " << memberaccess->getIdentifier2()->getname() << " not defined!\n";
                exit(0);
            }
        }
    }
}

void StarPlatCodeGen::visitArglist(const Arglist* arglist, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitArg(const Arg* arg, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitStatement(const Statement* statement, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitStatementlist(const Statementlist* stmtlist, mlir::SymbolTable* symbolTable) {
    printf("Inside Stmtlist\n");

    for (ASTNode* stmt : stmtlist->getStatementList()) {
        stmt->Accept(this, symbolTable);
    }
}

void StarPlatCodeGen::visitType(const TypeExpr* type, mlir::SymbolTable* symbolTable) {}

void StarPlatCodeGen::visitNumber(const Number* number, mlir::SymbolTable* symbolTable) {
    if (number->isFloat()) {
        std::string key = std::to_string(number->getnumberfloat());
        if (globalLookupOp(key)) return;
        auto constant = mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), builder.getF64Type(),
                                                        mlir::FloatAttr::get(builder.getF64Type(), number->getnumberfloat()),
                                                        builder.getStringAttr(key),
                                                        builder.getStringAttr("public"));
        symbolTable->insert(constant);
    } else {
        std::string key = std::to_string(number->getnumber());
        if (globalLookupOp(key)) return;
        auto constant = mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), builder.getI64Type(),
                                                        builder.getStringAttr(key),
                                                        builder.getStringAttr(key),
                                                        builder.getStringAttr("public"));
        symbolTable->insert(constant);
    }
}

void StarPlatCodeGen::visitExpression(const Expression* expr, mlir::SymbolTable* symbolTable) { expr->getExpression()->Accept(this, symbolTable); }

void StarPlatCodeGen::print() {
    mlir::LogicalResult lr = verify(module);
    if (llvm::succeeded(lr))
        module.dump();
}

mlir::SymbolTable* StarPlatCodeGen::getSymbolTable() { return &globalSymbolTable; }

mlir::MLIRContext* StarPlatCodeGen::getContext() { return &context; }

mlir::ModuleOp* StarPlatCodeGen::getModule() { return &module; }

mlir::Value StarPlatCodeGen::globalLookupOp(llvm::StringRef name) {
    // llvm::outs() << "Looking up: " << name << " in " << symbolTables.size() << " symbol tables\n";
    auto it = nameToArgMap.find(name);
    if (it != nameToArgMap.end()) {
        mlir::Value value = it->second;

        return value;
    }

    for (auto symbolTable : symbolTables) {
        if (symbolTable->lookup(name))
            return symbolTable->lookup(name)->getResult(0);
    }

    return nullptr;
}

mlir::Value StarPlatCodeGen::resolveExpr(const Expression* expr, mlir::SymbolTable* symbolTable) {
    if (!expr) { llvm::outs() << "Error: null expression\n"; return nullptr; }
    // llvm::outs() << "resolveExpr: kind = " << expr->getKind() << "\n";

    switch (expr->getKind()) {

        case ExpressionKind::KIND_IDENTIFIER: {
            const Identifier* id = static_cast<const Identifier*>(expr->getExpression());
            auto val = globalLookupOp(id->getname());
            if (!val) llvm::outs() << "Error: Undefined identifier '" << id->getname() << "'\n";
            return val;
        }

        case ExpressionKind::KIND_NUMBER: {
            const Number* num = static_cast<const Number*>(expr->getExpression());
            if (num->isFloat()) {
                mlir::Type floatType = builder.getF64Type();
                auto constAttr = mlir::FloatAttr::get(floatType, num->getnumberfloat());
                auto constOp   = mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), floatType, constAttr,
                                                                builder.getStringAttr("const_" + std::to_string(get_const_count())),
                                                                builder.getStringAttr("private"));
                return constOp->getResult(0);
            } else {
                mlir::Type intType = mlir::IntegerType::get(builder.getContext(), 64);
                auto constAttr     = mlir::IntegerAttr::get(intType, num->getnumber());
                auto constOp       = mlir::starplat::ConstOp::create(builder, builder.getUnknownLoc(), intType, constAttr,
                                                                    builder.getStringAttr("const_" + std::to_string(get_const_count())),
                                                                    builder.getStringAttr("private"));
                return constOp->getResult(0);
            }
        }

        case ExpressionKind::KIND_KEYWORD: {
            const Keyword* kw = static_cast<const Keyword*>(expr->getExpression());
            auto val = globalLookupOp(kw->getKeyword());
            if (!val) {
                kw->Accept(this, symbolTable);
                val = globalLookupOp(kw->getKeyword());
            }
            if (!val) llvm::outs() << "Error: Undefined keyword '" << kw->getKeyword() << "'\n";
            return val;
        }

        case ExpressionKind::KIND_MEMBERACCESS: {
            const Memberaccess* mem = static_cast<const Memberaccess*>(expr->getExpression());
            const Identifier* id1   = static_cast<const Identifier*>(mem->getIdentifier());

            if (!id1) { llvm::outs() << "Error: Malformed member access\n"; return nullptr; }

            // Case 1: g.method(params) — method call on a variable
            if (mem->getMethodCall()) {
                const Methodcall* method = static_cast<const Methodcall*>(mem->getMethodCall());
                auto objVal = globalLookupOp(id1->getname());
                if (!objVal) { llvm::outs() << "Error: Undefined variable '" << id1->getname() << "'\n"; return nullptr; }

                if (method->getIsBuiltin()) {
                    if (strcmp(method->getIdentifier()->getname(), "count_outNbrs") == 0) {
                        const Expression* paramExpr = static_cast<const Expression*>(method->getParamLists());
                        const Identifier* paramId   = static_cast<const Identifier*>(paramExpr->getExpression());
                        auto nodeVal = globalLookupOp(paramId->getname());
                        if (!nodeVal) { llvm::outs() << "Error: Undefined node in count_outNbrs\n"; return nullptr; }
                        return mlir::starplat::CountOutNbrsOp::create(
                            builder, builder.getUnknownLoc(),
                            builder.getI64Type(),
                            objVal, nodeVal)->getResult(0);
                    }
                    else if (strcmp(method->getIdentifier()->getname(), "is_an_edge") == 0) {
                        const Paramlist* paramlist = static_cast<const Paramlist*>(method->getParamLists());
                        auto params    = paramlist->getParamList();
                        const Identifier* id1 = static_cast<const Identifier*>(params[0]->getExpr()->getExpression());
                        const Identifier* id2 = static_cast<const Identifier*>(params[1]->getExpr()->getExpression());
                        auto node1 = globalLookupOp(id1->getname());
                        auto node2 = globalLookupOp(id2->getname());
                        if (!node1 || !node2) {
                            llvm::outs() << "Error: Undefined nodes in is_an_edge\n";
                            return nullptr;
                        }
                        return mlir::starplat::IsEdgeOp::create(
                            builder, builder.getUnknownLoc(),
                            mlir::IntegerType::get(builder.getContext(), 1),
                            objVal, node1, node2)->getResult(0);
                    }
                }
                llvm::outs() << "Error: Unsupported method call '" << method->getIdentifier()->getname() << "' in resolveExpr\n";
                return nullptr;
            }

            // Case 2: node.property or edge.property
            const Identifier* id2 = static_cast<const Identifier*>(mem->getIdentifier2());
            if (!id2) { llvm::outs() << "Error: Malformed member access\n"; return nullptr; }

            auto nodeVal = globalLookupOp(id1->getname());
            auto propVal = globalLookupOp(id2->getname());
            if (!nodeVal || !propVal) { 
                llvm::outs() << "Error: Undefined member access '" << id1->getname() << "." << id2->getname() << "'\n"; 
                return nullptr; 
            }

            if (mlir::isa<mlir::starplat::NodeType>(nodeVal.getType())) {
                mlir::Type propElemType = builder.getI64Type();
                if (auto propNodeType = mlir::dyn_cast<mlir::starplat::PropNodeType>(propVal.getType()))
                    propElemType = propNodeType.getParameter();
                return mlir::starplat::GetNodePropertyOp::create(
                    builder, builder.getUnknownLoc(), propElemType,
                    nodeVal, propVal,
                    builder.getStringAttr(id2->getname()))->getResult(0);
            }

            if (mlir::isa<mlir::starplat::EdgeType>(nodeVal.getType())) {
                mlir::Type propElemType = builder.getI64Type();
                if (auto propEdgeType = mlir::dyn_cast<mlir::starplat::PropEdgeType>(propVal.getType()))
                    propElemType = propEdgeType.getParameter();
                return mlir::starplat::GetEdgePropertyOp::create(
                    builder, builder.getUnknownLoc(), propElemType,
                    nodeVal, propVal,
                    builder.getStringAttr(id2->getname()))->getResult(0);
            }

            llvm::outs() << "Error: Unknown member access type for '" << id1->getname() << "'\n";
            return nullptr;
        }

        case ExpressionKind::KIND_ADDOP: {
            const Add* op = static_cast<const Add*>(expr->getExpression());
            auto lhs = resolveExpr(static_cast<const Expression*>(op->getOperand1()), symbolTable);
            auto rhs = resolveExpr(static_cast<const Expression*>(op->getOperand2()), symbolTable);
            if (!lhs || !rhs) return nullptr;
            return mlir::starplat::AddOp::create(builder, builder.getUnknownLoc(),
                builder.getI64Type(), lhs, rhs)->getResult(0);
        }

        case ExpressionKind::KIND_SUBOP: {
            const Sub* op = static_cast<const Sub*>(expr->getExpression());
            auto lhs = resolveExpr(static_cast<const Expression*>(op->getOperand1()), symbolTable);
            auto rhs = resolveExpr(static_cast<const Expression*>(op->getOperand2()), symbolTable);
            if (!lhs || !rhs) return nullptr;
            return mlir::starplat::SubOp::create(builder, builder.getUnknownLoc(),
                builder.getI64Type(), lhs, rhs)->getResult(0);
        }

        case ExpressionKind::KIND_MULOP: {
            const Mul* op = static_cast<const Mul*>(expr->getExpression());
            auto lhs = resolveExpr(static_cast<const Expression*>(op->getOperand1()), symbolTable);
            auto rhs = resolveExpr(static_cast<const Expression*>(op->getOperand2()), symbolTable);
            if (!lhs || !rhs) return nullptr;
            return mlir::starplat::MulOp::create(builder, builder.getUnknownLoc(),
                builder.getI64Type(), lhs, rhs)->getResult(0);
        }

        case ExpressionKind::KIND_DIVOP: {
            const Div* op = static_cast<const Div*>(expr->getExpression());
            auto lhs = resolveExpr(static_cast<const Expression*>(op->getOperand1()), symbolTable);
            auto rhs = resolveExpr(static_cast<const Expression*>(op->getOperand2()), symbolTable);
            if (!lhs || !rhs) return nullptr;
            return mlir::starplat::DivOp::create(builder, builder.getUnknownLoc(),
                builder.getF64Type(), lhs, rhs)->getResult(0);
        }

        case ExpressionKind::KIND_ANDOP: {
            const And* op = static_cast<const And*>(expr->getExpression());
            auto lhs = resolveExpr(static_cast<const Expression*>(op->getOperand1()), symbolTable);
            auto rhs = resolveExpr(static_cast<const Expression*>(op->getOperand2()), symbolTable);
            if (!lhs || !rhs) return nullptr;
            return mlir::starplat::AndOp::create(builder, builder.getUnknownLoc(),
                builder.getI1Type(), lhs, rhs)->getResult(0);
        }

        case ExpressionKind::KIND_OROP: {
            const Or* op = static_cast<const Or*>(expr->getExpression());
            auto lhs = resolveExpr(static_cast<const Expression*>(op->getOperand1()), symbolTable);
            auto rhs = resolveExpr(static_cast<const Expression*>(op->getOperand2()), symbolTable);
            if (!lhs || !rhs) return nullptr;
            return mlir::starplat::OrOp::create(builder, builder.getUnknownLoc(),
                builder.getI1Type(), lhs, rhs)->getResult(0);
        }

        case ExpressionKind::KIND_BOOLEXPR: {
            const BoolExpr* boolExpr  = static_cast<const BoolExpr*>(expr->getExpression());
            const Expression* lhsExpr = static_cast<const Expression*>(boolExpr->getExpr1());
            const Expression* rhsExpr = static_cast<const Expression*>(boolExpr->getExpr2());
            auto lhs = resolveExpr(lhsExpr, symbolTable);
            auto rhs = resolveExpr(rhsExpr, symbolTable);
            if (!lhs || !rhs) return nullptr;
            return mlir::starplat::CmpOp::create(builder, builder.getUnknownLoc(),
                mlir::IntegerType::get(builder.getContext(), 1),
                lhs, rhs,
                builder.getStringAttr(boolExpr->getop()))->getResult(0);
        }

        default:
            llvm::outs() << "Error: Unsupported expression kind " << expr->getKind() << "\n";
            return nullptr;
    }
}