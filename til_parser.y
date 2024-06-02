%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                                            i;          /* integer value */
  double                                         d;          /* double value */
  std::string                                   *s;          /* symbol name or string literal */
  cdk::basic_node                               *node;       /* node pointer */
  cdk::sequence_node                            *sequence;   /* sequence node */
  std::vector<std::shared_ptr<cdk::basic_type>> *vtype;      /* basic_type vector */
  til::block_node                               *block;      /* block node */
  cdk::expression_node                          *expression; /* expression node */
  cdk::lvalue_node                              *lvalue;     /* lvalue node */
};

%token <i> tINT
%token <d> tDOUBLE
%token <s> tIDENTIFIER tSTRING
%token tNULL
%token tTINT tTDOUBLE tTSTRING tTVOID
%token tPROGRAM tBLOCK tFUNCTION tWITH
%token tPUBLIC tPRIVATE tFORWARD tEXTERNAL tVAR
%token tPRINT tPRINTLN tSTOP tNEXT tRETURN
%token tIF tLOOP
%token tSET tREAD tSIZEOF tINDEX tOBJECTS
%token tGE tLE tEQ tNE tAND tOR

%type <node> program decl decl_global function_decl inst loop with if
%type <sequence> decls decls_global insts exprs function_decls
%type <type> type type_pointer type_function function_type
%type <vtype> types
%type <block> block
%type <expression> expr expr_comp function
%type <lvalue> lval

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file : decls_global program    { compiler->ast(new cdk::sequence_node(LINE, $2, $1)); }
     | decls_global            { compiler->ast($1); }
     | program                 { compiler->ast(new cdk::sequence_node(LINE, $1)); }
     | /* empty */             { compiler->ast(new cdk::sequence_node(LINE)); }
     ;

decl_global : '(' tPUBLIC type tIDENTIFIER expr ')'    { $$ = new til::declaration_node(LINE, tPUBLIC, *$4, $5, $3); delete $4; }
            | '(' tPUBLIC type tIDENTIFIER ')'         { $$ = new til::declaration_node(LINE, tPUBLIC, *$4, nullptr, $3); delete $4;}
            | '(' tFORWARD type tIDENTIFIER ')'        { $$ = new til::declaration_node(LINE, tFORWARD, *$4, nullptr, $3); delete $4; }
            | '(' tEXTERNAL type tIDENTIFIER ')'       { $$ = new til::declaration_node(LINE, tEXTERNAL, *$4, nullptr, $3); delete $4; }
            | '(' tPUBLIC tVAR tIDENTIFIER expr ')'    { $$ = new til::declaration_node(LINE, tPUBLIC, *$4, $5, nullptr); delete $4; }
            | '(' tPUBLIC tIDENTIFIER expr ')'         { $$ = new til::declaration_node(LINE, tPUBLIC, *$3, $4, nullptr); delete $3; }
            | decl                                     { $$ = $1; }
            ;

decls_global : decl_global                 { $$ = new cdk::sequence_node(LINE, $1); };
             | decls_global decl_global    { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;

decl : '(' type tIDENTIFIER expr ')'            { $$ = new til::declaration_node(LINE, tPRIVATE, *$3, $4, $2); delete $3; }
     | '(' type tIDENTIFIER ')'                 { $$ = new til::declaration_node(LINE, tPRIVATE, *$3, nullptr, $2); delete $3; }
     | '(' tVAR tIDENTIFIER expr ')'            { $$ = new til::declaration_node(LINE, tPRIVATE, *$3, $4, nullptr); delete $3; }
     ;

decls : decl          { $$ = new cdk::sequence_node(LINE, $1); }
      | decls decl    { $$ = new cdk::sequence_node(LINE, $2, $1); }
      ;

program : '(' tPROGRAM decls insts ')' { $$ = new til::program_node(LINE, new til::block_node(LINE, $3, $4)); }
        | '(' tPROGRAM decls ')'       { $$ = new til::program_node(LINE, new til::block_node(LINE, $3, new cdk::sequence_node(LINE))); }
        | '(' tPROGRAM insts ')'       { $$ = new til::program_node(LINE, new til::block_node(LINE, new cdk::sequence_node(LINE), $3)); }
        | '(' tPROGRAM ')'             { $$ = new til::program_node(LINE, new til::block_node(LINE, new cdk::sequence_node(LINE), new cdk::sequence_node(LINE))); }
        ;

function : '(' tFUNCTION '(' function_type ')' decls insts ')'       { $$ = new til::function_def_node(LINE, new cdk::sequence_node(LINE), $4, new til::block_node(LINE, $6, $7)); }
         | '(' tFUNCTION '(' function_type ')' decls ')'             { $$ = new til::function_def_node(LINE, new cdk::sequence_node(LINE), $4, new til::block_node(LINE, $6, new cdk::sequence_node(LINE))); }
         | '(' tFUNCTION '(' function_type ')' insts ')'             { $$ = new til::function_def_node(LINE, new cdk::sequence_node(LINE), $4, new til::block_node(LINE, new cdk::sequence_node(LINE), $6)); }
         | '(' tFUNCTION '(' function_type ')' ')'                   { $$ = new til::function_def_node(LINE, new cdk::sequence_node(LINE), $4, new til::block_node(LINE, new cdk::sequence_node(LINE), new cdk::sequence_node(LINE))); }
         | '(' tFUNCTION '(' function_type function_decls ')' decls insts ')' { $$ = new til::function_def_node(LINE, $5, $4, new til::block_node(LINE, $7, $8)); }
         | '(' tFUNCTION '(' function_type function_decls ')' decls ')'       { $$ = new til::function_def_node(LINE, $5, $4, new til::block_node(LINE, $7, new cdk::sequence_node(LINE))); }
         | '(' tFUNCTION '(' function_type function_decls ')' insts ')'       { $$ = new til::function_def_node(LINE, $5, $4, new til::block_node(LINE, new cdk::sequence_node(LINE), $7)); }
         | '(' tFUNCTION '(' function_type function_decls ')' ')'             { $$ = new til::function_def_node(LINE, $5, $4, new til::block_node(LINE, new cdk::sequence_node(LINE), new cdk::sequence_node(LINE))); }
         ;

function_type : type      { $$ = $1; }
              | tTVOID    { $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID); }

function_decl : '(' type tIDENTIFIER ')'         { $$ = new til::declaration_node(LINE, tPRIVATE, *$3, nullptr, $2); delete $3; }
              ;

function_decls : function_decl                   { $$ = new cdk::sequence_node(LINE, $1); }
               | function_decls function_decl    { $$ = new cdk::sequence_node(LINE, $2, $1); }
               ;

type : tTINT            { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT); }
     | tTDOUBLE         { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE); }
     | tTSTRING         { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
     | type_pointer     { $$ = $1; }
     | type_function    { $$ = $1; }
     ;

type_pointer : type '!'      { $$ = cdk::reference_type::create(4, $1); }
             | tTVOID '!'    { $$ = cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_VOID)); }
             ;

type_function : '(' type ')'                    { $$ = cdk::functional_type::create($2); }
              | '(' tTVOID ')'                  { $$ = cdk::functional_type::create(cdk::primitive_type::create(0, cdk::TYPE_VOID)); }
              | '(' type '(' types ')' ')'      { $$ = cdk::functional_type::create(*$4, $2); delete $4; }
              | '(' tTVOID '(' types ')' ')'    { $$ = cdk::functional_type::create(*$4, cdk::primitive_type::create(0, cdk::TYPE_VOID)); delete $4; }
              ;

types : type          { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(1, $1); }
      | types type    { $1->push_back($2); $$ = $1; }
      ;

block : '(' tBLOCK decls insts ')'    { $$ = new til::block_node(LINE, $3, $4); }
      | '(' tBLOCK decls ')'          { $$ = new til::block_node(LINE, $3, new cdk::sequence_node(LINE)); }
      | '(' tBLOCK insts ')'          { $$ = new til::block_node(LINE, new cdk::sequence_node(LINE), $3); }
      | '(' tBLOCK ')'                { $$ = new til::block_node(LINE, new cdk::sequence_node(LINE), new cdk::sequence_node(LINE)); }
      ;

inst : expr                           { $$ = new til::evaluation_node(LINE, $1); }
     | '(' tPRINT exprs ')'           { $$ = new til::print_node(LINE, $3); }
     | '(' tPRINTLN exprs ')'         { $$ = new til::print_node(LINE, $3, true); }
     | '(' tSTOP tINT ')'             { $$ = new til::stop_node(LINE, $3); }
     | '(' tSTOP ')'                  { $$ = new til::stop_node(LINE, 1); }
     | '(' tNEXT tINT ')'             { $$ = new til::next_node(LINE, $3); }
     | '(' tNEXT ')'                  { $$ = new til::next_node(LINE, 1); }
     | '(' tRETURN expr ')'           { $$ = new til::return_node(LINE, $3); }
     | '(' tRETURN ')'                { $$ = new til::return_node(LINE, nullptr); }
     | if                             { $$ = $1; }
     | loop                           { $$ = $1; }
     | with                           { $$ = $1; }
     | block                          { $$ = $1; }
     ;

insts : inst          { $$ = new cdk::sequence_node(LINE, $1); }
      | insts inst    { $$ = new cdk::sequence_node(LINE, $2, $1); }

if : '(' tIF expr inst inst ')' { $$ = new til::if_else_node(LINE, $3, $4, $5); }
   | '(' tIF expr inst ')'      { $$ = new til::if_node(LINE, $3, $4); }
   ;

loop : '(' tLOOP expr inst ')'  { $$ = new til::loop_node(LINE, $3, $4); }
     ;

with : tWITH expr expr expr expr  { $$ = new til::with_node(LINE, $2, $3, $4, $5); }
     | tWITH '@' expr expr expr   { $$ = new til::with_node(LINE, nullptr, $3, $4, $5); }
     ;

expr : tINT                  { $$ = new cdk::integer_node(LINE, $1); }
     | tDOUBLE               { $$ = new cdk::double_node(LINE, $1); }
     | tSTRING               { $$ = new cdk::string_node(LINE, *$1); delete $1; }
     | tNULL                 { $$ = new til::nullptr_node(LINE); }
     | lval                  { $$ = new cdk::rvalue_node(LINE, $1); }
     | function              { $$ = $1; }
     | '(' expr_comp ')'     { $$ = $2; }
     ;

expr_comp : '-' expr              { $$ = new cdk::unary_minus_node(LINE, $2); }
          | '+' expr              { $$ = new cdk::unary_plus_node(LINE, $2); }
          | '?' lval              { $$ = new til::address_node(LINE, $2); }
          | '*' expr expr         { $$ = new cdk::mul_node(LINE, $2, $3); }
          | '/' expr expr         { $$ = new cdk::div_node(LINE, $2, $3); }
          | '%' expr expr         { $$ = new cdk::mod_node(LINE, $2, $3); }
          | '+' expr expr         { $$ = new cdk::add_node(LINE, $2, $3); }
          | '-' expr expr         { $$ = new cdk::sub_node(LINE, $2, $3); }
          | '<' expr expr         { $$ = new cdk::lt_node(LINE, $2, $3); }
          | '>' expr expr         { $$ = new cdk::gt_node(LINE, $2, $3); }
          | tGE expr expr         { $$ = new cdk::ge_node(LINE, $2, $3); }
          | tLE expr expr         { $$ = new cdk::le_node(LINE, $2, $3); }
          | tEQ expr expr         { $$ = new cdk::eq_node(LINE, $2, $3); }
          | tNE expr expr         { $$ = new cdk::ne_node(LINE, $2, $3); }
          | '~' expr              { $$ = new cdk::not_node(LINE, $2); }
          | tAND expr expr        { $$ = new cdk::and_node(LINE, $2, $3); }
          | tOR expr expr         { $$ = new cdk::or_node(LINE, $2, $3); }
          | tSET lval expr        { $$ = new cdk::assignment_node(LINE, $2, $3); }
          | expr exprs            { $$ = new til::call_node(LINE, $1, $2); }
          | expr                  { $$ = new til::call_node(LINE, $1, new cdk::sequence_node(LINE)); }
          | '@' exprs             { $$ = new til::call_node(LINE, nullptr, $2); }
          | '@'                   { $$ = new til::call_node(LINE, nullptr, new cdk::sequence_node(LINE)); }
          | tREAD                 { $$ = new til::read_node(LINE); }
          | tOBJECTS expr         { $$ = new til::objects_node(LINE, $2); }
          | tSIZEOF expr          { $$ = new til::sizeof_node(LINE, $2); }
          // | '(' expr ')'          { $$ = $2; } shitft/reduce conflict. I dont even know if this is supposed to exist. this enables things like (+ (1) 2)
          ;

lval : tIDENTIFIER                 { $$ = new cdk::variable_node(LINE, *$1); delete $1; }
     | '(' tINDEX expr expr ')'    { $$ = new til::index_node(LINE, $3, $4); }
     ;

exprs : expr          { $$ = new cdk::sequence_node(LINE, $1); }
      | exprs expr    { $$ = new cdk::sequence_node(LINE, $2, $1); }
      ;

%%
