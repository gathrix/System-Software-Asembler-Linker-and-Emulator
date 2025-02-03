#ifndef __Asembler_hpp__
#define __Asembler_hpp__



#include <iostream>
#include <string.h>
#include <vector>

#include <list>
#include <iomanip>
#include <set>
#include <map>
#include <fstream>
#include "../location.hh"
#include "../bison.tab.hh"
#include <sstream>
#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif
using namespace std;

enum BindingType{
  LOCAL, GLOBAL, WEAK
};

enum Type{
  NOTYPE, OBJECT, FUNC, SECTION
};

enum OperationType{
  NONE, REGDIRECT, REGINDIRECT, LITERALDIRECT, LITERALINDIRECT, SYMBOLDIRECT, SYMBOLINDIRECT, REGLITERALIND, REGSYMBOLIND
};

struct Sections{
  string name;
  int id;
  int size;
};
struct ST_forwardRefs{
  int patch;
  int section;
  bool reloc;
  ST_forwardRefs* nlink;
};
class SectionData{
  public:
    SectionData(){}
    ~SectionData(){}
    int section;
    int offset;
    string code;
    string explanation;

    friend std::ostream& operator<<(std::ostream& os, const SectionData& obj) {
        return os <<  "    " << obj.section << "           " << obj.offset << "          " << obj.code << "      " << obj.explanation;

    }

};


class RelocTable{
  public:
    RelocTable(){}
    ~RelocTable(){}
    int section;
    int offset;
    string type;
    int symbol;
    int addend;
    friend std::ostream& operator<<(std::ostream& os, const RelocTable& obj) {
        return os << "    " << obj.section << "           " << obj.offset << "          " << obj.symbol << "            " << obj.addend << "       " << obj.type;

    }
};

class LiteralPool{
  public:
    LiteralPool(string hexa, int section, bool reloc): literal(hexa), offset(-1), section(section), reloc(reloc) {}
    ~LiteralPool(){}
    string literal;
    int offset;
    int section;
    list<int> usedOn;

    bool reloc;
    friend std::ostream& operator<<(std::ostream& os, const LiteralPool& obj) {
        os << obj.literal << "   " << obj.offset << "    " << obj.section << "   ";
        os << "[";
        for(int i : obj.usedOn){
          os << i << "  ";
        }
        os << "]";
        return os;

    }
};

class ST_entry{
    
      
    public:
      ST_entry(string name, int value, bool defined, int section): name(name), size(0), value(value), section(section), bindType(LOCAL),type(NOTYPE), defined(defined), flink(nullptr){ id = ++ID;}
      ST_entry(string name): name(name), size(0), value(0), section(0), bindType(LOCAL),type(NOTYPE), defined(false), flink(nullptr){ id = ++ID;}
      ST_entry(string name, bool defined, Type type): name(name), size(0), value(0), section(0), bindType(LOCAL),type(type), defined(defined), flink(nullptr){ id = ++ID; section = id;}
      ST_entry(string name, bool defined, BindingType bindType, int section): name(name), size(0), value(0), section(section), bindType(bindType),type(NOTYPE), defined(defined), flink(nullptr){ id = ++ID;}
      ST_entry(string name, bool defined, Type type, int id): name(name),id(id), size(0), value(0), section(0), bindType(LOCAL),type(type), defined(defined), flink(nullptr) { section = id;}
      ~ST_entry() {
        while(flink){
          ST_forwardRefs* temp = flink;
          flink = flink->nlink;
          delete(temp);
          
        }
      }
      string name;

      static int ID;
      int size;
      int id;
      int value;
      int section;
      //0 - local, 1- global
      BindingType bindType;
      Type type;
      bool defined;
      ST_forwardRefs* flink;

      friend std::ostream& operator<<(std::ostream& os, const ST_entry& obj) {
          os << "  " << obj.id << "     "<< obj.name << "       " << obj.value << "           " << obj.bindType << "        " << obj.type << "    " << obj.defined << "     " << obj.section << "     " << obj.size;
          ST_forwardRefs* temp = obj.flink;
          os << "| ";
          if(temp != nullptr ) os << ", ";
          while(temp != nullptr){
            os << temp->patch << " ";
            temp = temp->nlink;
          }
          os << ")";

          return os;
}


};






namespace MC{

  class Asembler
  {
  private:
    MC::MC_Parser  *parser  = nullptr;
    MC::Scanner *scanner = nullptr;
    set<ST_entry*> symbolTable;
    list<SectionData*> code;
    list<LiteralPool*> literalPool;
    list<RelocTable*>  relocTable;
    list<SectionData*> unresolvedCode;
  public:
    Asembler() = default;
    virtual ~Asembler();

    ST_entry* isInSymTab(string symbol);
    void addToSymTab(ST_entry* newSym);
    void parse(const char* const filename);
    std::vector<std::string> split(std::string s, std::string delimiter);
    void addGlobalSymbols(std::string symbolString);
    void addExternSymbols(std::string symbolString);
    void updateSymbol(string symbol);
    void addToForwardRefs(ST_entry* symbol, bool reloc);
    void printSymTab();
    void parse_helper( std::istream &stream );
    void printToFile(string filename);
    void printBinToFile(string filename);
    //HELPER
    int getSectionSize(int section);
    void dataOperandCode(list<string> operands);
    string getRegisterNumber(string reg);
    void jmpRelocCode();
    int addToLiteralPool(string hexa, bool reloc);
    void printLiteralPool();
    void printRelocData();
    void updateSectionPool();
    void jmpCode(string code);
    void jmpLiteralCode();
    void relocCode(string instruction);
    void callLiteralCode();
    void callCode(string code);
    void beqLiteralCode(string gpr1);
    void bgtLiteralCode(string gpr1);
    void bneLiteralCode(string gpr1);
    void ldLiteralCode(string gpr);
    void ldCode(string gpr, string operand);
    void condJmpRelocCode(string instruction, string gpr2);
    void beqCode(string code, string gpr1, string gpr2);
    void bneCode(string code, string gpr1, string gpr2);
    void bgtCode(string code, string gpr1, string gpr2);
    void ldRegCode(string operand, string gpr);
    void stRegCode(string operand, string gpr);
    string jmpLiteral(string literal);
    void ldRegSymIndCode(string operand, string gpr, string reg);
    void stRegSymIndCode(string operand, string gpr, string reg);
    void backpatching();
    void stRelocCode(string gpr);
    void addPoolRelocCode();
    void addLiteralToCode(string literal);

    //INSTRUCTION FUNCTIONS
    void xchgInstruction(list<string> operands);
    void haltInstruction();
    void intInstruction();
    void iretInstruction();
    void notInstruction(string gpr);
    void orInstruction(list<string> operands);
    void xorInstruction(list<string> operands);
    void andInstruction(list<string> operands);
    void shlInstruction(list<string> operands);
    void shrInstruction(list<string> operands);
    void csrrdInstruction(string csr, string gpr);
    void csrwrInstruction(string gpr, string csr);
    void retInstruction();
    void pushInstruction(string gpr);
    void popInstruction(string gpr);
    void addInstruction(list<string> operands);
    void subInstruction(list<string> operands);
    void mulInstruction(list<string> operands);
    void divInstruction(list<string> operands);
    void callInstruction(string operand);
    void jmpInstruction(string operand);
    void beqInstruction(list<string> operands);
    void bgtInstruction(list<string> operands);
    void bneInstruction(list<string> operands);
    void ldInstruction(string operand, string gpr);
    void stInstruction(string gpr, string operand);
    //END OF INS FUNCTIONS

    //DIRECTIVE FUNCTIONS
    void skipDirective(string literal);
    void wordDirective(string list);
    //END OF DIRECTIVE FUNCTIONS


    void printSecData();
    static int locationCounter;
    static int isInSection;
    static int currSection;
    static int symbolOrliteral;
    static int sOrL;
    static OperationType opType;
  };



  class Scanner : public yyFlexLexer {

    public:
      Scanner(std::istream *in) : yyFlexLexer(in) {};

      virtual ~Scanner() {} ;
      
      
      using FlexLexer::yylex;

      virtual int yylex( MC::MC_Parser::semantic_type * const lval, MC::MC_Parser::location_type *location );


      private:
    /* yyval ptr */
        MC::MC_Parser::semantic_type *yylval = nullptr;
  };

}





#endif
