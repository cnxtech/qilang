/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

%{
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include "parser_p.hpp"
#include <boost/make_shared.hpp>

// stuff from flex that bison needs to know about:


%}

%locations
%defines
%error-verbose

//use C++ type, instead of old union C type
%define api.value.type variant
// Instead of "yytoken yylex(yylval, yylloc)", use "symbol_type yylex()".
%define api.token.constructor

%language "c++"


//We tell Bison that yyparse should take an extra parameter context
//and that yylex (LanAB_lex) takes an additional argument scanner
%parse-param { qilang::Parser* context }
%lex-param   { qilang::Parser* context }

%code {

  typedef void* yyscan_t;
  yy::parser::symbol_type qilang_lex(yyscan_t lex);

  yy::parser::symbol_type yylex(qilang::Parser* context)
  {
    return qilang_lex(context->scanner);
  }


}


%define api.token.prefix {TOK_}
%token
  //better error reporting
  END_OF_FILE          0 "end of file"
  // Operators
  BANG                "!"
  PERCENT             "%"
  STAR                "*"
  PLUS                "+"
  MINUS               "-"
  SLASH               "/"
  EQ_EQ               "=="
  GT                  ">"
  GT_EQ               ">="
  LT                  "<"
  LT_EQ               "<="
  NOT_EQ              "!="
  AMPERSAND_AMPERSAND "&&"
  PIPE_PIPE           "||"
  LPAREN              "("
  RPAREN              ")"
  TILDA               "~"
  LBRACKET            "["
  RBRACKET            "]"
  COMMA               ","
  AND                 "&"
  OR                  "|"
  XOR                 "^"
  ARO                 "@"

  // Blocks Types
  OBJECT              "object"
  INTERFACE           "interface"
  STRUCT              "struct"
  TYPE                "type"

  // Core Keywords
  AT                  "at"
  END                 "end"
  FOR                 "for"
  IF                  "if"

%token <qilang::NodePtr> CONSTANT
%token <qilang::NodePtr> ID IDSLASH STRING

// the first item here is the last to evaluate, the last item is the first
%left  "||"
%left  "&&"
%left  "==" "!="
%left  "<" "<=" ">" ">="
%left  "|" "&" "^"
%left  "+" "-"
%left  "*" "/" "%"
%left  "~" "@"
%right "!"
%left  "["

%%

%start toplevel;

%type<qilang::NodePtr> toplevel;

toplevel:
  exp    { context->root = boost::make_shared<qilang::ExprNode>($1); }

//comma.opt: /* empty */ | ",";

//exps:
//  /* empty */       { $$ = new Call; }
//| exps.1 comma.opt  { std::swap($$, $1); }
//;

//exps.1:
//  exp             { $$ = new Call; $$->args.push_back($1);}
//| exps.1 "," exp  { std::swap($$, $1); $$->args.push_back($3); }
//;

//call:
//  ID "(" exps ")" { $$ = $3; $$->functionName = $1; free($1);}

//exp:
//  call   { $$ = new CallNode($1->functionName, $1->args); delete $1;}

//exp:
//  "[" exps "]" { $$ = new CallNode("", $2->args); delete $2;}

%type<qilang::NodePtr> exp;
exp:
  exp "+" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Plus);}
| exp "-" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Minus);}
| exp "/" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Divide);}
| exp "*" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Multiply);}
| exp "%" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Modulus);}
| exp "^" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Xor);}
| exp "|" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Or);}
| exp "&" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_And);}

exp:
  "!" exp { $$ = boost::make_shared<qilang::UnaryOpNode>($2, qilang::UnaryOpCode_Negate);}
| "-" exp { $$ = boost::make_shared<qilang::UnaryOpNode>($2, qilang::UnaryOpCode_Minus);}

exp:
  exp "||" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_BoolOr);}
| exp "&&" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_BoolAnd);}

exp:
  CONSTANT { $$ = $1; }
| STRING   { $$ = $1; }

exp:
  ID       { $$ = $1; }

exp:
   exp "==" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_EqEq);}
|  exp "<"  exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Lt);}
|  exp "<=" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Le);}
|  exp ">"  exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Gt);}
|  exp ">=" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Ge);}
|  exp "!=" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Ne);}

// The PersistNode has been removed (all keys data are persistent now). But we
//  need to keep this here so we don't get parse errors on existing conditions
exp:
  exp "@" exp { $$ = $1;}

exp:
  "(" exp ")" { $$ = $2;}

exp:
exp "[" exp "]" { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_FetchArray);}

%%

void yy::parser::error(const yy::parser::location_type& loc,
                       const std::string& msg)
{
  std::stringstream ss;

  //ss << context->filename << ":" << loc.first_line << ":" << loc.first_column << ": error:" << err;
  ss << "error: " << loc << ":" << msg;
  throw std::runtime_error(ss.str());
}