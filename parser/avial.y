%{
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>
	  #include "../../ast/ast.h"
    extern int yylex();
    void yyerror(const char* s);

	extern ASTNode *root;


%}

%union {
	ASTNode* astNode;
	char* id;
}


%token<id> FUNCTION LPAREN RPAREN LCURLY RCURLY RETURN IDENTIFIER ASGN NUMBER LTxx GT FORALL FOR EQUALS EDGE NODE
%token<id> INT IF SEMICLN DOT IN COMMA EQUAL GRAPH PLUSEQUAL PROPNODE PROPEDGE FALSE INF FIXEDPOINT UNTIL COLON PLUS TRUE NOT BOOL FLOAT DO WHILE MINUS TIMES DIVIDE AND OR

%type<astNode>  methodcall memberaccess expr type paramlist arglist arg function boolexpr declarationstmt stmt 
stmtlist ifstmt forstmt returnstmt forallstmt dowhileStmt incandassignstmt assignment initializestmt fixedPointStmt tuppleAssignmentstmt memberaccessstmt assignmentStmt
addExpr properties templateType templateDecl paramAssignment param memberaccessAssignment KEYWORDS subExpr mulExpr divExpr andExpr orExpr


%%

prgm  : function            {root = $1;;}
        | stmtlist           {root = $1;}   
        ;

function : FUNCTION IDENTIFIER LPAREN arglist RPAREN LCURLY stmtlist RCURLY     {
                                                                                        Identifier* funcName = new Identifier($2);
                                                                                        Arglist* arglist = static_cast<Arglist*>($4);
                                                                                        $$ = new Function(funcName, arglist, $7);
                                                                                }
         ;

stmtlist :                  {
                                  $$ = new Statementlist();
                                  
                                }

         | stmtlist stmt        {
                                  Statementlist* stmtlist =static_cast<Statementlist*>($1);
                                  stmtlist->addstmt($2);
                                  $$ = stmtlist;
                                }
         ;

stmt :  assignmentStmt                              {$$ = $1;}
        |   declarationstmt                             {$$ = $1;}
        |   ifstmt			                {$$ = $1;}
        |   forstmt 			                {$$ = $1;}
        |   returnstmt 			                {$$ = $1;}
        |   forallstmt			                {$$ = $1;}
        |   incandassignstmt	                        {$$ = $1;}
        |   templateDecl                                {$$ = $1;}
        |   /*epsilon*/                                 {$$ = nullptr;}
        |   memberaccessstmt                            {$$ = $1;}
        |   initializestmt                              {$$ = $1;}
        |   memberaccessAssignment                      {$$ = $1;}
        |   fixedPointStmt                              {$$ = $1;}
        |   tuppleAssignmentstmt                        {$$ = $1;}
        |   dowhileStmt                                 {$$ = $1;}
        ;



memberaccessAssignment : memberaccess EQUAL expr SEMICLN        {$$ = new MemberAccessAssignment($1, $3);}
                        ;


declarationstmt : type IDENTIFIER SEMICLN                   {
		Identifier* identifier = new Identifier($2);
		Number* number = nullptr;
		$$ = new DeclarationStatement($1,identifier,number);
		// printf("Declaration statement\n");
		}

                | type IDENTIFIER EQUAL NUMBER SEMICLN  {
                                                                Identifier* identifier = new Identifier($2);
                                                                Number* number =   new Number($4);
                                                                $$ = new DeclarationStatement($1, identifier, number);
                                                        
                                                        }
            ;

templateDecl : templateType IDENTIFIER SEMICLN {      
                                                      Identifier *identifier = new Identifier($2);  
                                                      $$ = new TemplateDeclarationStatement($1, identifier); 
                                                }
              ;

assignmentStmt : assignment SEMICLN     {$$ = new AssignmentStmt($1);}

assignment : IDENTIFIER EQUAL expr      {$$ = new Assignment($1, $3);}
            ;

initializestmt : type IDENTIFIER EQUAL expr SEMICLN {
                                                        Identifier *identifier = new Identifier($2);
                                                        $$ = new InitialiseAssignmentStmt($1, identifier, $4);
                                                }

paramAssignment : IDENTIFIER EQUAL KEYWORDS  {
                                                Identifier *identifier = new Identifier($1);
                                                $$ = new ParameterAssignment(identifier, $3);


}

fixedPointStmt : FIXEDPOINT UNTIL LPAREN IDENTIFIER COLON expr RPAREN LCURLY stmtlist RCURLY         { 
                                                                                                                Identifier *identifier = new Identifier($4);
                                                                                                                $$ = new FixedpointUntil(identifier, $6, $9);
                                                                                                        }

tuppleAssignmentstmt : LTxx expr COMMA expr GT EQUAL LTxx expr COMMA expr GT SEMICLN               {$$ = new TupleAssignment($2, $4, $8, $10);}

dowhileStmt : DO LCURLY stmtlist RCURLY WHILE LPAREN expr RPAREN SEMICLN               {$$ = new DoWhileStatement($7, $3);}

boolexpr : expr LTxx expr 		{$$ = new BoolExpr($1, strdup("<"), $3);}

         | expr GT expr                 {$$ = new BoolExpr($1, strdup(">"), $3);}

         | expr EQUALS expr             {$$ = new BoolExpr($1, const_cast<char*>("=="), $3);} 

         | NOT expr                     {$$ = new BoolExpr($2, const_cast<char*>("!"));} 
         ;

ifstmt : IF LPAREN expr RPAREN stmt         {$$ = new IfStatement($3, $5);}
        | IF LPAREN expr RPAREN LCURLY stmtlist RCURLY     {$$ = new IfStatement($3, $6);}
        ;


forstmt : FOR LPAREN IDENTIFIER IN expr RPAREN LCURLY stmtlist RCURLY      { $$ = new ForallStatement(); }

forallstmt : FORALL LPAREN IDENTIFIER IN expr RPAREN LCURLY stmtlist RCURLY    {
                                                                  Identifier* identifier = new Identifier($3);
                                                                  $$ = new ForallStatement(identifier, $5, $8);
                                                                }

expr :  IDENTIFIER              {$$ = new Expression( new Identifier($1), KIND_IDENTIFIER);} 
     |  boolexpr                {$$ = new Expression ($1, KIND_BOOLEXPR);}
     |  NUMBER                  {$$ = new Expression( new Number($1), KIND_NUMBER);}
     |  memberaccess            {$$ = new Expression ($1, KIND_MEMBERACCESS);}
     |  KEYWORDS                {$$ = new Expression ($1, KIND_KEYWORD);}
     |  methodcall              {$$ = new Expression ($1, KIND_METHODCALL);}
     |  addExpr                 {$$ = new Expression ($1, KIND_ADDOP);}
     |  subExpr                 {$$ = new Expression ($1, KIND_SUBOP);}
     |  mulExpr                 {$$ = new Expression ($1, KIND_MULOP);}
     |  divExpr                 {$$ = new Expression ($1, KIND_DIVOP);}
     |  andExpr                 {$$ = new Expression ($1, KIND_ANDOP);}
     |  orExpr                  {$$ = new Expression ($1, KIND_OROP);}
     |  LPAREN expr RPAREN      {$$ = $2;}       
     ;

addExpr : expr PLUS expr      {$$ = new Add($1, $3);}

subExpr : expr MINUS expr      {$$ = new Sub($1, $3);}

mulExpr : expr TIMES expr      {$$ = new Mul($1, $3);}

divExpr : expr DIVIDE expr      {$$ = new Div($1, $3);}

andExpr : expr AND expr      {$$ = new And($1, $3);}

orExpr : expr OR expr      {$$ = new Or($1, $3);}

incandassignstmt : IDENTIFIER PLUSEQUAL expr SEMICLN  {
                                                        Identifier* identifier = new Identifier($1);
                                                        $$ = new Incandassignstmt(identifier, $2, $3);
                                                      }
             ;

returnstmt : RETURN expr SEMICLN        {$$ = new ReturnStmt($2);}
           ;

methodcall : IDENTIFIER LPAREN paramlist RPAREN {
                                                  
                                                  Identifier* identifier = new Identifier($1);
                                                  $$ = new Methodcall(identifier, $3);
                                                }

             | IDENTIFIER LPAREN expr RPAREN     {
                                                  Identifier* identifier = new Identifier($1);
                                                  $$ = new Methodcall(identifier, $3);
                                                } 

            | IDENTIFIER LPAREN RPAREN          {
                                                  Identifier* identifier = new Identifier($1);
                                                  $$ = new Methodcall(identifier);
                                                }
            ;

memberaccess : IDENTIFIER DOT methodcall        {
                                                  Identifier* identifier = new Identifier($1);
                                                  $$ = new Memberaccess(identifier, $3);
                                                }
             | memberaccess DOT methodcall      { $$ = new Memberaccess($1, $3);}
             | IDENTIFIER DOT IDENTIFIER        { 
                                                    Identifier* identifier_1 = new Identifier($1);    
                                                    Identifier* identifier_2 = new Identifier($3);
                                                    $$ = new Memberaccess(identifier_1, identifier_2);

                                                }
             ;

memberaccessstmt : memberaccess SEMICLN {$$ = new MemberacceessStmt($1);}

arg : type IDENTIFIER           {
                                  Identifier* varname = new Identifier($2);
                                  TypeExpr* type = static_cast<TypeExpr*>($1);
                                  $$ = new Arg(type, varname);
                                }
      | templateType IDENTIFIER {

                                  Identifier* varname = new Identifier($2);
                                  TemplateType* type = static_cast<TemplateType*>($1);
                                  $$ = new Arg(type, varname);
                                }
      
      | /*epsilon*/             { $$ = nullptr; }

      | IDENTIFIER EQUAL IDENTIFIER   {}
    
      ;


arglist : arg                   {
                                  Arglist* arglist = new Arglist();
                                  Arg* arg = static_cast<Arg*>($1);
                                  arglist->addarg(arg);
                                  $$ = arglist;
                                }
        
        | arglist COMMA arg     
                                {
                                  Arglist* arglist =static_cast<Arglist*>($1);
                                  Arg* arg = static_cast<Arg*>($3);
                                  arglist->addarg(arg);
                                  $$ = arglist;
                                }
        ;


param : expr                    {       Expression *expr = static_cast<Expression *>($1);
                                        $$ = new Param(expr);

                                        if(expr->getKind() == KIND_IDENTIFIER) {
                                                const Identifier *identifier = static_cast<const Identifier *>(expr->getExpression());
                                                printf("Param: %s\n", identifier->getname());
                                        }
                                                
                                }

         | paramAssignment      {
                                        ParameterAssignment *paramAssignment = static_cast<ParameterAssignment *>($1);
                                        $$ = new Param(paramAssignment);
                                }
         ;


paramlist :   param                                          {
                                                                Paramlist *paramlist = new Paramlist();
                                                                Param *param = static_cast<Param *>($1);
                                                                paramlist->addparam(param);
                                                                $$ = paramlist;
                                                                

                                                           }

          | paramlist COMMA param                          {
                                                                Paramlist *paramlist = static_cast<Paramlist *>($1);
                                                                Param *param = static_cast<Param *>($3);
                                                                paramlist->addparam(param);
                                                                $$ = paramlist;

                                                           }
          ;

templateType : properties LTxx type COMMA IDENTIFIER GT   {
                                                GraphProperties *graphproperties = static_cast<GraphProperties *>($1);
                                                TypeExpr *type = static_cast<TypeExpr *>($3);
                                                Identifier *graphName = new Identifier($5);
                                                $$ = new TemplateType(graphproperties, type, graphName);


                                        }
                ;

properties : PROPEDGE                   {$$ = new GraphProperties($1);} 
             | PROPNODE                 {$$ = new GraphProperties($1);}
             ;


type : INT              {       
                                $$ = new TypeExpr($1);
                        }
      | BOOL            {
                                $$ = new TypeExpr($1);
                        }

     | GRAPH            {
                                $$ = new TypeExpr($1);
                        }

     |  EDGE            {
                                $$ = new TypeExpr($1);
                        }
     |  NODE            {
                                $$ = new TypeExpr($1);
                        }
     | FLOAT            {
                                $$ = new TypeExpr($1);
                        }
     ;

KEYWORDS : FALSE         {
                                $$ = new Keyword($1);
                         }

          | INF          {
                                $$ = new Keyword($1);
                         }

          | TRUE        {
                                $$ = new Keyword($1);
                        }
          ;


%%


