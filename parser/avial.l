%{
    #include "../../ast/ast.h"
    #include "avial.tab.h"

    extern YYSTYPE yylval;
%}

%option nounput

%%


"function"              {return FUNCTION;}
"return"                {return RETURN;}        
"if"                    {return IF;}
"else"                  {return ELSE;}
"forall"                {return FORALL;}
"for"                   {return FOR;}
"Graph"                 {yylval.id = strdup(yytext); return GRAPH;}
"Node"                  {yylval.id = strdup(yytext); return NODE;}
"int"                   {yylval.id = strdup(yytext); return INT;}
"bool"                  {yylval.id = strdup(yytext); return BOOL;}
"float"                 {yylval.id = strdup(yytext); return FLOAT; }
"do"                    {yylval.id = strdup(yytext); return DO; }
"while"                 {yylval.id = strdup(yytext); return WHILE; }
"propNode"              {yylval.id = strdup(yytext); return PROPNODE; }
"propEdge"              {yylval.id = strdup(yytext); return PROPEDGE; }
"False"                 {yylval.id = strdup(yytext); return FALSE; }
"True"                  {yylval.id = strdup(yytext); return TRUE; }
"INF"                   {yylval.id = strdup(yytext); return INF; }
"fixedPoint"            {yylval.id = strdup(yytext); return FIXEDPOINT; }
"until"                 {yylval.id = strdup(yytext); return UNTIL; }
"edge"                  {yylval.id = strdup(yytext); return EDGE; }



"{"                     {return LCURLY;}
"}"                     {return RCURLY;}
"("                     {return LPAREN;}
")"                     {return RPAREN;} 
"<"                     {return LTxx;}
">"                     {return GT;}
"="                     {return EQUAL;}
"<="                    {return LTE;}
">="                    {return GTE;}
";"                     {return SEMICLN;}
"."                     {return DOT;}
"in"                    {return IN;}
","                     {return COMMA;}
"+="                    {return PLUSEQUAL;}
":"                     {return COLON;}
"=="                    {return EQUALS;}
"+"                     {return PLUS;}
"++"                    {return INCREMENT;}
"!"                     {return NOT;}
"-"                     {return MINUS;}
"*"                     {return TIMES;}
"/"                     {return DIVIDE;}
"&&"                    {return AND;}
"||"                    {return OR;}

[ \t\n]+                ;
#.*                    { /* Ignore single-line comments */ }

[a-zA-Z_][a-zA-Z0-9_]*  {yylval.id = strdup(yytext); return IDENTIFIER;}
[0-9]*                  {yylval.id = strdup(yytext); return NUMBER;}
[0-9]*[.][0-9]*         {yylval.id = strdup(yytext); return DECIMAL;}

%%

int yywrap(void)
{
    return 1;
}


void yyerror(const char* s) {
    fprintf(stderr, "Error: %s\n", s);
}
