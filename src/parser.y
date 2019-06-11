%code requires {

# define YYLTYPE_IS_DECLARED 1 /* alert the parser that we have our own definition */

}

%{
    #include "AstNode.h"
	#include "FunctionDeclaration.h"
	#include "ClassDeclaration.h"
    #include <stdio.h>
    liquid::Block *programBlock; /* the top level root node of our final AST */

    extern int yylex();
    int yyerror(char const * s );
    #define YYERROR_VERBOSE
    #define YYDEBUG 1

    extern std::stack<std::string> fileNames;

    # define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
          (Current).file_name = fileNames.top();            \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
          (Current).file_name = fileNames.top();            \
        }                                                               \
    while (0)

%}

/* Represents the many different ways we can access our data */
%union {
    liquid::Node *node;
    liquid::Block *block;
    liquid::Expression *expr;
    liquid::Statement *stmt;
    liquid::Identifier *ident;
    liquid::VariableDeclaration *var_decl;
    liquid::VariableDeclarationDeduce *var_decl_deduce;
    std::vector<liquid::VariableDeclaration*> *varvec;
    std::vector<liquid::Expression*> *exprvec;
    std::string *string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE TSTR TBOOL
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL TLTLT
%token <token> TCOMMA TDOT TCOLON TRANGE
%token <token> TLPAREN TRPAREN TLBRACKET TRBRACKET
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TNOT TAND TOR
%token <token> TIF TELSE TWHILE TTO 
%token <token> TSQUOTE TDEF TRETURN TRETURN_SIMPLE TVAR IS
%token <token> INDENT UNINDENT 

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (Identifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> literals expr boolean_expr binop_expr unaryop_expr array_expr array_access  range_expr
%type <varvec> func_decl_args
%type <exprvec> call_args array_elemets_expr 
%type <block> program stmts block
%type <stmt> stmt var_decl var_decl_deduce func_decl conditional return while class_decl array_add_element
%type <token> comparison 

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV
%left TAND TNOT

%start program
%debug 
%verbose 
%locations /* track locations: @n of component N; @$ of entire range */

%%

program : /* blank */ { programBlock = new liquid::Block(); }
        | stmts { programBlock = $1; }
        ;

stmts : stmt { $$ = new liquid::Block(); $$->statements.push_back($<stmt>1); }
      | stmts stmt { $1->statements.push_back($<stmt>2); }
      ;

stmt : var_decl
     | var_decl_deduce
     | func_decl
     | class_decl
     | conditional 
     | return
     | while
     | array_add_element
     | expr { $$ = new liquid::ExpressionStatement($1); }
     ;


block : INDENT stmts UNINDENT { $$ = $2; }
      | INDENT UNINDENT { $$ = new liquid::Block(); }
      ;

conditional : TIF expr block TELSE block {$$ = new liquid::Conditional($2,$3,$5);}
            | TIF expr block {$$ = new liquid::Conditional($2,$3);}
            ; 

while : TWHILE expr block TELSE block {$$ = new liquid::WhileLoop($2,$3,$5);}
      | TWHILE expr block {$$ = new liquid::WhileLoop($2,$3);}
      ; 

var_decl : ident ident { $$ = new liquid::VariableDeclaration($1, $2, @$); }
         | ident ident TEQUAL expr { $$ = new liquid::VariableDeclaration($1, $2, $4, @$); }
         ;

var_decl_deduce : TVAR ident TEQUAL expr { $$ = new liquid::VariableDeclarationDeduce($2, $4, @$); }
         ;

func_decl : TDEF ident TLPAREN func_decl_args TRPAREN TCOLON ident block { $$ = new liquid::FunctionDeclaration($7, $2, $4, $8, @$); }
          | TDEF ident TLPAREN func_decl_args TRPAREN block { $$ = new liquid::FunctionDeclaration($2, $4, $6, @$); }
          ;

func_decl_args : /*blank*/  { $$ = new liquid::VariableList(); }
          | var_decl { $$ = new liquid::VariableList(); $$->push_back($<var_decl>1); }
          | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
          ;

class_decl: TDEF ident block {$$ = new liquid::ClassDeclaration($2, $3); }
          ;

ident : TIDENTIFIER { $$ = new liquid::Identifier(*$1, @1); delete $1; }
      | TIDENTIFIER TDOT TIDENTIFIER { $$ = new liquid::Identifier(*$1,*$3, @$); delete $1; delete $3;}
      ;

literals : TINTEGER { $$ = new liquid::Integer(atol($1->c_str())); delete $1; }
         | TDOUBLE { $$ = new liquid::Double(atof($1->c_str())); delete $1; }
         | TSTR { $$ = new liquid::String(*$1); delete $1; }
         | TBOOL { $$ = new liquid::Boolean(*$1); delete $1; }
         ;


return : TRETURN expr { $$ = new liquid::Return(@$, $2); }
       | TRETURN_SIMPLE { $$ = new liquid::Return(@$); }
       ;

expr : ident TEQUAL expr { $$ = new liquid::Assignment($<ident>1, $3, @$); }
     | ident TLPAREN call_args TRPAREN { $$ = new liquid::MethodCall($1, $3, @$);  }
     | ident { $<ident>$ = $1; }
     | literals
     | boolean_expr 
     | binop_expr
     | unaryop_expr
     | TLPAREN expr TRPAREN { $$ = $2; }
     | range_expr
     | array_expr
     | array_access
     ;
/* have to write it explecity to have the right operator precedence */
binop_expr : expr TAND expr { $$ = new liquid::BinaryOp($1, $2, $3, @$); }
           | expr TOR expr { $$ = new liquid::BinaryOp($1, $2, $3, @$); }
           | expr TPLUS expr { $$ = new liquid::BinaryOp($1, $2, $3, @$); }
           | expr TMINUS expr { $$ = new liquid::BinaryOp($1, $2, $3, @$); }
           | expr TMUL expr { $$ = new liquid::BinaryOp($1, $2, $3, @$); }
           | expr TDIV expr { $$ = new liquid::BinaryOp($1, $2, $3, @$); }
           ;

unaryop_expr : TNOT expr { $$ = new liquid::UnaryOperator($1, $2); }
             ;

boolean_expr : expr comparison expr { $$ = new liquid::CompOperator($1, $2, $3); }
             ;

call_args : /*blank*/  { $$ = new liquid::ExpressionList(); }
          | expr { $$ = new liquid::ExpressionList(); $$->push_back($1); }
          | call_args TCOMMA expr  { $1->push_back($3); }
          ;
 
comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
           ;
          
array_elemets_expr: /* blank */ {$$ = new liquid::ExpressionList(); }
                 | expr {$$ = new liquid::ExpressionList(); $$->push_back($1);}
                 | array_elemets_expr TCOMMA expr {$$->push_back($3); }
                 ; 
                 
array_expr : TLBRACKET array_elemets_expr TRBRACKET {$$ = new liquid::Array($2, @$);}
          ;
          
array_add_element: ident TLTLT expr { $$ = new liquid::ArrayAddElement($1, $3, @$); }
                ;
                
array_access: ident TLBRACKET TINTEGER TRBRACKET { $$ = new liquid::ArrayAccess($1,atol($3->c_str()), @$); delete $3;}
           | array_access TLBRACKET TINTEGER TRBRACKET { $$ = new liquid::ArrayAccess($1,atol($3->c_str()), @$); delete $3;}
           ;
           
range_expr : TLBRACKET expr TRANGE expr TRBRACKET {$$ = new liquid::Range($2, $4, @$);}
           ;

%%
