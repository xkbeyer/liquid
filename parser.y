%code requires {

# define YYLTYPE_IS_DECLARED 1 /* alert the parser that we have our own definition */

}

%{
    #include "AstNode.h"
    #include <stdio.h>
    AST::Block *programBlock; /* the top level root node of our final AST */

    extern int yylex();
    int yyerror(char const * s );
    #define YYERROR_VERBOSE
    /*#define YYDEBUG 1*/
%}

/* Represents the many different ways we can access our data */
%union {
    AST::Node *node;
    AST::Block *block;
    AST::Expression *expr;
    AST::Statement *stmt;
    AST::Identifier *ident;
    AST::VariableDeclaration *var_decl;
    AST::VariableDeclarationDeduce *var_decl_deduce;
    std::vector<AST::VariableDeclaration*> *varvec;
    std::vector<AST::Expression*> *exprvec;
    std::string *string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE TSTR TBOOL
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TCOMMA TDOT TCOLON
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TNOT TAND TOR
%token <token> TIF TELSE TWHILE
%token <token> TSQUOTE TDEF TRETURN TRETURN_SIMPLE TVAR IS
%token <token> INDENT UNINDENT 

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (Identifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> literals expr boolean_expr binop_expr unaryop_expr
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl var_decl_deduce func_decl conditional return while class_decl
%type <token> comparison 

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV
%left TAND TNOT

%start program
/* %debug */
/* %verbose */
%locations

%%

program : /* blank */ { programBlock = new AST::Block(); }
        | stmts { programBlock = $1; }
        ;

stmts : stmt { $$ = new AST::Block(); $$->statements.push_back($<stmt>1); }
      | stmts stmt { $1->statements.push_back($<stmt>2); }
      ;

stmt : var_decl
     | var_decl_deduce
     | func_decl
     | class_decl
     | conditional 
     | return
     | while
     | expr { $$ = new AST::ExpressionStatement($1); }
     ;


block : INDENT stmts UNINDENT { $$ = $2; }
      | INDENT UNINDENT { $$ = new AST::Block(); }
      ;

conditional : TIF expr block TELSE block {$$ = new AST::Conditional($2,$3,$5);}
            | TIF expr block {$$ = new AST::Conditional($2,$3);}
            ; 

while : TWHILE expr block TELSE block {$$ = new AST::WhileLoop($2,$3,$5);}
      | TWHILE expr block {$$ = new AST::WhileLoop($2,$3);}
      ; 

var_decl : ident ident { $$ = new AST::VariableDeclaration($1, $2, @2); }
         | ident ident TEQUAL expr { $$ = new AST::VariableDeclaration($1, $2, $4, @2); }
         ;

var_decl_deduce : TVAR ident TEQUAL expr { $$ = new AST::VariableDeclarationDeduce($2, $4, @2); }
         ;

func_decl : TDEF ident TLPAREN func_decl_args TRPAREN TCOLON ident block { $$ = new AST::FunctionDeclaration($7, $2, $4, $8, @2); }
          | TDEF ident TLPAREN func_decl_args TRPAREN block { $$ = new AST::FunctionDeclaration($2, $4, $6, @2); }
          ;

func_decl_args : /*blank*/  { $$ = new AST::VariableList(); }
          | var_decl { $$ = new AST::VariableList(); $$->push_back($<var_decl>1); }
          | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
          ;

class_decl: TDEF ident block {$$ = new AST::ClassDeclaration($2, $3); }
          ;

ident : TIDENTIFIER { $$ = new AST::Identifier(*$1, @1); delete $1; }
      | TIDENTIFIER TDOT TIDENTIFIER { $$ = new AST::Identifier(*$1,*$3, @1); delete $1; delete $3;}
      ;

literals : TINTEGER { $$ = new AST::Integer(atol($1->c_str())); delete $1; }
         | TDOUBLE { $$ = new AST::Double(atof($1->c_str())); delete $1; }
         | TSTR { $$ = new AST::String(*$1); delete $1; }
         | TBOOL { $$ = new AST::Boolean(*$1); delete $1; }
         ;


return :  TRETURN expr { $$ = new AST::Return($2); }
       | TRETURN_SIMPLE { $$ = new AST::Return(); }
       ;

expr : ident TEQUAL expr { $$ = new AST::Assignment($<ident>1, $3, @1); }
     | ident TLPAREN call_args TRPAREN { $$ = new AST::MethodCall($1, $3, @1);  }
     | ident { $<ident>$ = $1; }
     | literals
     | boolean_expr 
     | binop_expr
     | unaryop_expr
     | TLPAREN expr TRPAREN { $$ = $2; }
     ;
/* have to write it explecity to have the right operator precedence */
binop_expr : expr TAND expr { $$ = new AST::BinaryOp($1, $2, $3); }
           | expr TOR expr { $$ = new AST::BinaryOp($1, $2, $3); }
           | expr TPLUS expr { $$ = new AST::BinaryOp($1, $2, $3); }
           | expr TMINUS expr { $$ = new AST::BinaryOp($1, $2, $3); }
           | expr TMUL expr { $$ = new AST::BinaryOp($1, $2, $3); }
           | expr TDIV expr { $$ = new AST::BinaryOp($1, $2, $3); }
           ;

unaryop_expr : TNOT expr { $$ = new AST::UnaryOperator($1, $2); }
             ;

boolean_expr : expr comparison expr { $$ = new AST::CompOperator($1, $2, $3); }
             ;

call_args : /*blank*/  { $$ = new AST::ExpressionList(); }
          | expr { $$ = new AST::ExpressionList(); $$->push_back($1); }
          | call_args TCOMMA expr  { $1->push_back($3); }
          ;
 
comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
           ;

%%
