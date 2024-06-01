%option c++ prefix="til_scanner_" outfile="til_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{
// make relevant includes before including the parser's tab file
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "til_parser.tab.h"

// don't change this
#define yyerror LexerError
%}

EXP [Ee][+-]?[0-9]+

%x X_STRING X_STRING_END X_COMMENT

%%
  yydebug=1; set_debug(1);

";".*                    ; /* ignore comments */

"/*"                     yy_push_state(X_COMMENT);
<X_COMMENT>"/*"          yy_push_state(X_COMMENT);
<X_COMMENT>"*/"          yy_pop_state();
<X_COMMENT>.|\n          ; /* ignore comments */

"null"                   return tNULL;

"int"                    return tTINT;
"double"                 return tTDOUBLE;
"string"                 return tTSTRING;
"void"                   return tTVOID;

"program"                return tPROGRAM;
"block"                  return tBLOCK;
"function"               return tFUNCTION;

"public"                 return tPUBLIC;
"forward"                return tFORWARD;
"external"               return tEXTERNAL;
"var"                    return tVAR;

"print"                  return tPRINT;
"println"                return tPRINTLN;
"stop"                   return tSTOP;
"next"                   return tNEXT;
"return"                 return tRETURN;

"if"                     return tIF;
"loop"                   return tLOOP;
"with"                   return tWITH;

"set"                    return tSET;
"read"                   return tREAD;
"sizeof"                 return tSIZEOF;
"index"                  return tINDEX;
"objects"                return tOBJECTS;

">="                     return tGE;
"<="                     return tLE;
"=="                     return tEQ;
"!="                     return tNE;
"&&"                     return tAND;
"||"                     return tOR;

[A-Za-z][A-Za-z0-9]*     yylval.s = new std::string(yytext); return tIDENTIFIER;

\"                       yy_push_state(X_STRING); yylval.s = new std::string("");
<X_STRING>\"             yy_pop_state(); return tSTRING;
<X_STRING>\0             yyerror("ASCII NUL (0x00) in string");
<X_STRING>\n             yyerror("ASCII LF (0x0A) in string");
<X_STRING>\\0            yy_push_state(X_STRING_END);
<X_STRING>\\t            *yylval.s += "\t";
<X_STRING>\\n            *yylval.s += "\n";
<X_STRING>\\r            *yylval.s += "\r";
<X_STRING>\\\"           *yylval.s += yytext + 1;
<X_STRING>\\\\           *yylval.s += yytext + 1;
<X_STRING>\\[0-7]{1,3}   {
                             int i8 = std::stoi(yytext + 1, nullptr, 8);
                             if (i8 > 255) yyerror("escape sequence bigger than 8 bits in string");
                             *yylval.s += (char) i8;
                         }
<X_STRING>\\.            *yylval.s += yytext + 1;
<X_STRING>.              *yylval.s += yytext;

<X_STRING_END>\"         yy_pop_state(); yy_pop_state(); return tSTRING;
<X_STRING_END>\0         yyerror("ASCII NUL (0x00) in string");
<X_STRING_END>\n         yyerror("ASCII LF (0x0A) in string");
<X_STRING_END>\\\"       ;
<X_STRING_END>\\\\       ;
<X_STRING_END>.          ;

[1-9][0-9]*|0            {
                             try {
                               yylval.i = std::stoi(yytext, nullptr, 10); return tINT;
                             } catch (const std::out_of_range&) {
                               yyerror("INT bigger than 4 bytes");
                             }
                         }

0x[0-9a-fA-F]*[1-9a-fA-F]+[0]* {
                                   try {
                                       yylval.i = std::stoi(yytext, nullptr, 16); return tINT;
                                   } catch (const std::out_of_range&) {
                                       yyerror("INT bigger than 4 bytes");
                                   }
                               }

[0-9]*\.[0-9]+{EXP}?|[0-9]+\.[0-9]*{EXP}?|[0-9]+{EXP}    {
                                                             try {
                                                                 yylval.d = std::stod(yytext); return tDOUBLE;
                                                             } catch (const std::out_of_range&) {
                                                                 yyerror("DOUBLE bigger than 8 bytes");
                                                             }
                                                         }

[-!()+?*/%<>~@]           return *yytext;

[ \n\r\t]+                ; /* ignore whitespace */

.                         yyerror("Unknown character");

%%
