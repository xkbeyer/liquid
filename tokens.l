%{
#include <string>
#include <stack>
#include "AstNode.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)

#ifdef _MSC_VER
int isatty(int) {return 0;};
#endif

extern "C" int yywrap() { return 1; }

#define YYERROR_VERBOSE 1
int yyerror(char const * s )
{
    printf("ERROR %s in %s at line %d, col ??\n", s, yytext, yylineno);
    return 1;
}

std::string g_str;

/* globals to track current indentation */
int current_line_indent = 0;   /* indentation of the current line */
int indent_level = 0;          /* indentation level passed to the parser */
std::stack<int> curr_indents;
int first_time = 1 ;

int yycolumn = 1;

#define YY_USER_ACTION \
    if( yylloc.last_line < yylineno ) yycolumn = 1 ; \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng; 
//    printf("fl=%d fc=%d lc=%d l=%d c=%d t=%s\n",yylloc.first_line, yylloc.last_line,yylloc.first_column,yylloc.last_column,yyleng,yycolumn, yytext);

%}

%option yylineno

%x indent 
%s normal 
%x str qstr
%x comment
%x incl
%%

            if( first_time ) {
                first_time = 0;
                curr_indents.push(0);
                BEGIN indent;
            }

import             BEGIN(incl);

<incl>[ \t]*      /* eat the whitespace */
<incl>[^ \t\n\r]+   { /* got the include file name */
                    yyin = fopen( yytext, "r" );
     
                    if ( ! yyin ) {
                       yyerror( "Failed to load import." );
                    } else {
                       yypush_buffer_state(yy_create_buffer( yyin, YY_BUF_SIZE ));
                    }
                    BEGIN(normal);
                  }
     

\@\{             BEGIN(comment);
<comment>\@\}.*    BEGIN(normal);
<comment>.*  ;/* eat everything */
<comment>\n  ;/* eat everything */

\"      g_str = ""; BEGIN(str);
'       g_str = ""; BEGIN(qstr);

<str>\" {   BEGIN(normal);
            yylval.string = new std::string(g_str);
            return TSTR;
        }
<qstr>' {   BEGIN(normal);
            yylval.string = new std::string(g_str);
            return TSTR;
        }

<str,qstr>\\n   g_str += "\n";
<str,qstr>\\t   g_str += "\t";
<str,qstr>\\r   g_str += "\r";
<str>\\\"       g_str += "\"";
<qstr>\\\'      g_str += "'";

<str,qstr>\\(.|\n)  g_str += yytext[1];

<str>[^\\\"]+  g_str += std::string(yytext);
<qstr>[^\\']+  g_str += std::string(yytext);

<indent>" "      { current_line_indent++; }
<indent>"\t"     { current_line_indent = (current_line_indent + 8) & ~7; }
<indent>"\n"     { current_line_indent = 0; yycolumn = 1;/*ignoring blank line */ }
<indent>"\r"     { current_line_indent = 0; yycolumn = 1;/*ignoring blank line */ }
<indent>.        {
                   unput(*yytext);
                   yycolumn--;
                   if (current_line_indent > curr_indents.top()) {
                       curr_indents.push(current_line_indent);
                       return TOKEN(INDENT);
                   } else if (current_line_indent < curr_indents.top()) {
                       curr_indents.pop();
                       return TOKEN(UNINDENT);
                   } else {
                       BEGIN(normal);
                   }
                 }

<normal>"\n"     { current_line_indent = 0; BEGIN( indent); yycolumn = 1; }
<<EOF>>          { if( curr_indents.size() > 1 ) {
                        curr_indents.pop();
                        return TOKEN(UNINDENT);
                   }
     		   yypop_buffer_state();
     
                   if ( !YY_CURRENT_BUFFER ) {
                        yyterminate();
                   }
                 }
                  

\r                      /* cr are ignored */
"if"                    return TOKEN(TIF);
"else"                  return TOKEN(TELSE);
"return"[ \t]*[\n]      return TOKEN(TRETURN_SIMPLE);
"return"                return TOKEN(TRETURN);
"not"                   return TOKEN(TNOT);
"and"                   return TOKEN(TAND);
"or"                    return TOKEN(TOR);
"def"                   return TOKEN(TDEF);
"var"                   return TOKEN(TVAR);
"while"                 return TOKEN(TWHILE);
"is"                    return TOKEN(IS);
"true"                  SAVE_TOKEN; return TBOOL;
"false"                 SAVE_TOKEN; return TBOOL;
#.*                     /* comments one line til nl */
[ \t\n]                 /* ignore */;
[a-zA-Z_][a-zA-Z0-9_&%\$\?\-]*  SAVE_TOKEN; return TIDENTIFIER;
-?[0-9]+\.[0-9]*        SAVE_TOKEN; return TDOUBLE;
-?[0-9]+                SAVE_TOKEN; return TINTEGER;
"="                     return TOKEN(TEQUAL);
"=="                    return TOKEN(TCEQ);
"!="                    return TOKEN(TCNE);
"<"                     return TOKEN(TCLT);
"<="                    return TOKEN(TCLE);
">"                     return TOKEN(TCGT);
">="                    return TOKEN(TCGE);
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"."                     return TOKEN(TDOT);
","                     return TOKEN(TCOMMA);
"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
":"                     return TOKEN(TCOLON);
.                       printf("line %d, len %d Unknown token %s !\n", yylineno, yyleng, yytext); yyterminate();

%%
