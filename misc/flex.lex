%{
  #include <list>
  #include <cstdio>
  #include <cstdlib>
  #include "./inc/asembler.hpp"
  #include "bison.tab.hh"
  #undef  YY_DECL
  #define YY_DECL int MC::Scanner::yylex( MC::MC_Parser::semantic_type * const lval, MC::MC_Parser::location_type *loc )
  #define YY_USER_ACTION loc->step(); loc->columns(yyleng);
  
  int isExpr = 0;
  using token = MC::MC_Parser::token;

%}




%option noyywrap c++
%option yyclass="MC::Scanner"
%%

%{          
    yylval = lval;
%}


" "		{ }
"\t" 	{ }
"\n" 	{
    loc->lines(); 
    if(isExpr == 1){
      isExpr = 0;
      return (token::EOL);
    }
    else { }
}
"\b"    { }
"\f"    { }

":" { return (token::COLON);}
"$" { return (token::DOLLAR);}
"[" { return (token::LBRACKET);}
"]" { return (token::RBRACKET);}
"+" { return (token::PLUS);}
"," { return (token::COMMA);}
"%" { return (token::PERCENT);}

".global" { isExpr = 1; return (token::GLOBAL);}
".extern" { isExpr = 1; return (token::EXTERN);} 
".section" { isExpr = 1; return (token::SECTION);}
".word" { isExpr = 1; return (token::WORD); }
".skip" { isExpr = 1; return (token::SKIP);}
".end" { return (token::END); }




"halt" { isExpr = 1;  return (token::HALT);}
"int" { isExpr = 1;  return (token::INT);}
"iret" { isExpr = 1;  return (token::IRET); }
"call" { isExpr = 1;  return (token::CALL);}
"ret" { isExpr = 1;  return (token::RET); }
"jmp" { isExpr = 1; return (token::JMP);}
"beq" { isExpr = 1; return (token::BEQ);}
"bne" { isExpr = 1; return (token::BNE);}
"bgt" { isExpr = 1; return (token::BGT);}
"push" { isExpr = 1; return (token::PUSH);}
"pop" { isExpr = 1; return (token::POP);}
"xchg" { isExpr = 1; return (token::XCHG);}
"add" { isExpr = 1; return (token::ADD);}
"sub" { isExpr = 1; return (token::SUB);}
"mul" { isExpr = 1; return (token::MUL);}
"div" { isExpr = 1; return (token::DIV);}
"not" { isExpr = 1; return (token::NOT);}
"and" { isExpr = 1; return (token::AND);}
"or" { isExpr = 1; return (token::OR);}
"xor" { isExpr = 1; return (token::XOR);}
"shl" { isExpr = 1; return (token::SHL);}
"shr" { isExpr = 1; return (token::SHR);}
"ld" { isExpr = 1; return (token::LD);}
"st" { isExpr = 1; return (token::ST);}
"csrrd" { isExpr = 1; return (token::CSRRD);}
"csrwr" { isExpr = 1; return (token::CSRWR);}


r(1[0-5]|[0-9]) {yylval->build< std::string >( yytext ); return (token::GPR);}
(pc|sp) {yylval->build< std::string >( yytext ); return (token::GPR);}
(status|handler|cause) {yylval->build< std::string >( yytext ); return (token::CSR);}
([a-z]|[A-Z])[a-z|A-Z|0-9|_]*  {yylval->build< std::string >( yytext ); isExpr = 1; return (token::IDENT); }
[0-9]+ {yylval->build< std::string >( yytext ); return (token::NUM);}
0[Xx][0-9A-Fa-f]+  { yylval->build< std::string >( yytext ); return (token::HEXA);}



"#".*  {} 

. {} 

%%


