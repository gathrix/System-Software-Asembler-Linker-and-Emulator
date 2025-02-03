#ifndef __Linker_hpp__
#define __Linker_hpp__

#include <iostream>
#include <string.h>
#include <vector>
#include <list>
#include <fstream>
#include <set>
#include <iomanip>
#include <map>
#include <sstream>
#include <algorithm>


enum BindingType{
  LOCAL, GLOBAL, WEAK
};

enum Type{
  NOTYPE, OBJECT, FUNC, SECTION
};


class ST_entry{
  public:

    std::string name;

    static int ID;
    int size;
    int id;
    long long int value;
    int section;
        //0 - local, 1- global
    BindingType bindType;
    Type type;
    bool defined;
    int updated;
    
    ST_entry(std::string name, int size, int value, Type type, bool defined):name(name), size(size), value(value), type(type), defined(defined), updated(0) {id = ++ID; section = id;}
    ST_entry(std::string name, int size,int id,long long int value,int section, Type type, bool defined): name(name), size(size), value(value), type(type), defined(defined), section(section), id(id), updated(0) {}
    ST_entry(std::string name, int size,int id,long long int value,int section, BindingType bindType, Type type,  bool defined): name(name), size(size), value(value), type(type), defined(defined), section(section), id(id), bindType(bindType), updated(0) {}
    ST_entry(std::string name, int size, int value, int section, Type type, bool defined):name(name), size(size), value(value), section(section), type(type), defined(defined), updated(0) {id = ++ID;}
};

class Code{
  public:
    
    int offset;
    std::string fourBytes;


};
class RelaData{
  public:
    int offset;
    int addend;
    std::string symbol;
};



class SectionData{

  public:
    SectionData() {}
    SectionData(std::string name): name(name) {}
    ~SectionData(){
      for(RelaData* r : rela){
        delete(r);
      }
    }
    std::string name;
    std::vector<std::string> code;
    int size;
    long long int start;
    bool place;
    std::list<RelaData*> rela;

};

class Linker{
  private:
    std::list<ST_entry*> symbolTable;
    std::map<std::string, SectionData*> sections;    
    std::map<std::string, long long int> sectionSizes;
    std::map<int, std::string> oldSectionsMap; 
    std::map<std::string, int> newSectionsMap;
  
  public:
    void parseFiles(int argc, char* argv[]);
    void relocation();
    void generateCode();
    void updateSectionId();
    void checkOverlap();
    void updateValues();
    void updateSymTab(std::string section, long long int size, int newOffset, std::list<ST_entry*> old);
    std::vector<std::string> split(std::string s, std::string delimiter);
    Type intToType(int);
    BindingType intToBindType(int);
    ST_entry* isInSymTab(std::string sym);
    static std::string outputFile;
    static int sectionStart;
    static int processing;
};


#endif
