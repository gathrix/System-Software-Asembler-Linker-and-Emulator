%{
  /* definitions */
  #include <cstdio>
  #include <iostream>
  #include <cstdlib>
  #include <fstream>
  #include "./inc/asembler.hpp"
  
  void yyerror(const char* s);
  int i = 0;
  extern int yyparse();
  extern "C" FILE* yyin;
%}

%define api.namespace {MC}

%define api.parser.class {MC_Parser}

%code requires {
      namespace MC {
            class Asembler;
            class Scanner;
      }
      # ifndef YY_NULLPTR
      #  if defined __cplusplus && 201103L <= __cplusplus
      #define YY_NULLPTR nullptr
      #  else
      #   define YY_NULLPTR 0
      #  endif
      # endif
}

%parse-param { Scanner  &scanner  }
%parse-param { Asembler  &asembler  }

%code{
      #undef yylex
      #define yylex scanner.yylex
}


%language "c++"
%define api.value.type variant
%define parse.assert
%locations

%start program

%token GLOBAL EXTERN SECTION WORD SKIP END EOL
       COLON DOLLAR LBRACKET RBRACKET PLUS COMMA PERCENT
       HALT INT IRET CALL RET JMP BEQ BNE BGT PUSH POP XCHG ADD SUB MUL DIV NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR

%token <std::string> IDENT
%token <std::string> HEXA
%token <std::string> NUM
%token <std::string> GPR
%token <std::string> CSR
%type <std::string> gpr
%type <std::string> operand
%type <std::string> symbolList
%type <std::string> literalList
%type <std::string> wordParameters
%type <std::string> literal
%type <std::string> symbol
%type <std::string> jmpOperand
%type <std::list<std::string>> dataOperand
%type <std::list<std::string>> jumpOperand
%type <std::string> csr

%%

program: lines END {asembler.updateSectionPool(); asembler.backpatching();}      
      |
      END;

lines: line lines  
      |
      line
      ;
line: expr EOL  
      |
      label expr EOL 
      |
      label EOL 
      ;


label: symbol COLON
      {
            if(!asembler.isInSymTab($1)){
                  
                  ST_entry* newEntry = new ST_entry($1, asembler.locationCounter, true, asembler.currSection);
                  asembler.addToSymTab(newEntry);
                  
            }
            else{
                  asembler.updateSymbol($1);
            }
           
            

      } 
      ;



expr: directive
      |
      instruction {}
      ;

directive: GLOBAL symbolList  {
                                    
                                    asembler.addGlobalSymbols($2);
                                    i = 0;
                              } 
          |
          EXTERN symbolList  
                              {
                                    
                                    asembler.addExternSymbols($2);
                                   
                              }
          |
          SECTION IDENT  {
                              if(!asembler.isInSection){
                                    ST_entry* newEntry = new ST_entry("UND", true, Type::SECTION, 0);
                                    asembler.addToSymTab(newEntry);
                              }
                              if(asembler.isInSection){asembler.updateSectionPool();}
                              asembler.isInSection = 1;
                              ST_entry* newEntry = new ST_entry($2, true, Type::SECTION);
                              asembler.currSection = newEntry->id;
                              asembler.addToSymTab(newEntry);
                              asembler.locationCounter = 0;


                         }
          |
          WORD wordParameters {

                                    asembler.wordDirective($2);
                                    

                              }  
          |
          SKIP literal 
                        {
                              asembler.skipDirective($2);

                        } 
          ;

symbolList: symbolList COMMA symbol { $$ = $1 +  $3 + " ";}
          |
          symbol  {$$ = $$ + $1 + " ";}
          ;
symbol: IDENT 
            {
                 
                 $$ = $1;
            }
      |
      CSR {$$ = $1;} 
      ;

wordParameters: symbolList {$$ = $1; asembler.sOrL = 1;}
              |
              literalList {$$ = $1; asembler.sOrL = -1;}
              ;
literalList: literalList COMMA literal  { $$ = $1 +  $3 + " ";}
            |
            literal {$$ = $$ + $1 + " ";}
            ;
literal: HEXA 
                  {
                        $$ = $1;
                  }
         |
         NUM { $$ = $1; }
         ;



instruction: HALT     
                  {
                        asembler.haltInstruction();

                  }
            |
            INT
                  {
                        asembler.intInstruction();
                  }
            |
            IRET
                  {
                        asembler.iretInstruction();
                  }   
            |
            CALL jmpOperand  {asembler.callInstruction($2);}
            |
            RET  { asembler.retInstruction();}
            |
            JMP jmpOperand  {asembler.jmpInstruction($2);}  
            |
            BEQ jumpOperand  {asembler.beqInstruction($2);}
            |
            BNE jumpOperand  {asembler.bneInstruction($2);}
            |
            BGT jumpOperand  {asembler.bgtInstruction($2);}
            |
            PUSH gpr  {asembler.pushInstruction($2);}
            |
            POP gpr  {asembler.popInstruction($2);}
            |
            XCHG dataOperand  {asembler.xchgInstruction($2);}
            |
            ADD dataOperand  {asembler.addInstruction($2);}
            |
            SUB dataOperand  {asembler.subInstruction($2);}
            |
            MUL dataOperand  {asembler.mulInstruction($2);}
            |
            DIV dataOperand  {asembler.divInstruction($2);}
            |
            NOT gpr  {asembler.notInstruction($2);}
            |
            AND dataOperand  {asembler.andInstruction($2);}
            |
            OR dataOperand  {asembler.orInstruction($2);}
            |
            XOR dataOperand  {asembler.xorInstruction($2);}
            |
            SHL dataOperand  {asembler.shlInstruction($2);}
            |
            SHR dataOperand  {asembler.shrInstruction($2);}
            |
            LD operand COMMA gpr   {asembler.ldInstruction($2, $4);}
            |
            ST gpr COMMA operand  {asembler.stInstruction($2, $4);}
            |
            CSRRD csr COMMA gpr  {asembler.csrrdInstruction($2, $4);}
            |
            CSRWR gpr COMMA csr   {asembler.csrwrInstruction($2, $4);}
            ;
jumpOperand: gpr COMMA gpr COMMA jmpOperand  {$$.push_back($1); $$.push_back($3); $$.push_back($5);};
jmpOperand: literal
                  {
                      asembler.symbolOrliteral = -1;
                      $$ = asembler.jmpLiteral($1);
                  }
            |
            symbol 
                  {
                      asembler.symbolOrliteral = 1;
                      $$ = $1;


                  }
            ;
dataOperand: gpr COMMA gpr {$$.push_back($1); $$.push_back($3);} ;
gpr: PERCENT GPR {$$ = $2;};
csr: PERCENT CSR {$$ = $2;};
operand: DOLLAR literal {asembler.opType = OperationType::LITERALDIRECT; $$ = $2;}
        |
        DOLLAR symbol {asembler.opType = OperationType::SYMBOLDIRECT; $$ = $2;}
        |
        literal {asembler.opType = OperationType::LITERALINDIRECT; $$ = $1;}
        |
        symbol {asembler.opType = OperationType::SYMBOLINDIRECT; $$ = $1;}
        |
        gpr {asembler.opType = OperationType::REGDIRECT; $$ = $1;}
        |
        LBRACKET gpr RBRACKET {asembler.opType = OperationType::REGINDIRECT; $$ = $2;}
        |
        LBRACKET gpr PLUS literal RBRACKET {asembler.opType = OperationType::REGLITERALIND; if($4.length() > 5){cout << "GRESKA PRI ASEMBLIRANJU" << endl; return -1;} $$ = $2 + " " + $4;}
        |
        LBRACKET gpr PLUS symbol RBRACKET {asembler.opType = OperationType::REGSYMBOLIND; $$ = $2 + " " + $4;}
        ;


%%





void MC::MC_Parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
