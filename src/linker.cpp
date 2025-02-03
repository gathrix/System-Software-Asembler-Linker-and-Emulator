#include <iostream>
#include "../inc/linker.hpp"
using namespace std;


int ST_entry::ID = 0;
int Linker::sectionStart = 0;
int Linker::processing = 0;
string Linker::outputFile = "";



void Linker::updateSectionId(){
  for(ST_entry* s : this->symbolTable){
    
    if(s->type == Type::SECTION) {
      continue;
    }

    if(s->updated == 1) {
      continue;
    }

    if(s->section == 0){
      continue;
    }
    
    int id = s->section;
    if(oldSectionsMap.count(id) == 0){
      continue;
    }
    string name = oldSectionsMap[id];
    int newID = newSectionsMap[name];
    s->section = newID;
    s->updated = 1;
    
  


  }
}



void Linker::parseFiles(int argc, char* argv[]){
  map<string, long long int> placedSections;
  std::list<std::string> files;
  if(argc < 2){
    cerr << "Nedovoljno parametara komandne linije" << endl;
    exit(-1);
  }

  if(strncmp(argv[1], "-hex", 4) == 0){
    int i = 2;
    
    while(strncmp(argv[i], "-place", 6) == 0){
      
      string arguments = argv[i];
      // Dohvatanje imena i start addr
      std::vector<std::string> nameAndStart = split(arguments.substr(7), "@");
      string sectionStart = nameAndStart.back();
      nameAndStart.pop_back();
      string sectionName = nameAndStart.back();

      placedSections[sectionName] = std::stoll(sectionStart,0,16);
  


      i++;

    }
    if(strncmp(argv[i], "-o", 2) == 0){
      i++;
      this->outputFile = argv[i];
      i++;
    }
    else{
      this->outputFile = "mem_content.hex";
    }
    while(i < argc){
      files.push_back(argv[i]);
      i++;
    }

  }
  else{
    cerr << "Nedovoljno parametara komandne linije" << endl;
    exit(-1);
  }
  std::string undString = "UND";
  ST_entry* undEntry = new ST_entry(undString, 0, 0, 0, 0, Type::SECTION, true);
  this->symbolTable.push_back(undEntry);
  oldSectionsMap[0] = "UND";
  newSectionsMap["UND"] = 0;

  
  for(string f : files){
    std::map<string, vector<string>> processed;
    std::ifstream inputFile(f);
    std::list<ST_entry*> old_symTab;
    
    
    // Check if the file opened successfully
    if (!inputFile.is_open()) {
      std::cerr << "Could not open the file!" << std::endl;
      exit(-1);
    }

    // Read the file line by line
    std::string line;
    std::getline(inputFile, line);
    if(line == "symbol_table"){
      processing = 1;
    }
    else{
      cerr << "Greska u ulaznom fajlu" << endl;
      exit(-1);
    }
    string section = "";
    while(std::getline(inputFile, line)){
      
      int newOffset;
      switch(processing){
        case 1: {
          // tabela simbola
          
          if(line.substr(0,5) == "rela."){
            //OVDE DODATI LOGIKU ZA SECTION_ID
            //PROCITANA CELA TABELA SIMBOLA za dati FAJL -> VRSI SE UPDATE SECTION ID

            


            processing = 2;
            section = line.substr(5);
            int sectionId;
            newOffset = 0;
            int size;
            for(ST_entry* s : this->symbolTable){
              if(s->name == section){
                sectionId = s->id;
                size = s->size;
              }
            }

            if(processed.count(f) == 0){
              
            
                auto it = sections.find(section);
              

              
              
              
                if(it != this->sections.end()){

                    
                    
                  newOffset = it->second->size;
                
                  it->second->size = sectionSizes[section];
                  this->updateSymTab(section, it->second->size, newOffset, old_symTab);
                    
                    
                  }
                  else{
                    SectionData* newSec = new SectionData(section);
                    newSec->size = size;
                    if(placedSections.count(section) > 0){
                      newSec->start = placedSections[section];
                    }
                    else{
                      newSec->start = -1;
                    }
                    
                    
                    this->sections[section] = newSec;
                    
                    
                  }
                  processed[f].push_back(section);
              
            }
            else{
              auto found = std::find(processed[f].begin(), processed[f].end(), section);
              if(found != processed[f].end()){
                break;
              }
              else{
                auto it = sections.find(section);
              

              
              
              
                if(it != this->sections.end()){

                    
                    
                  newOffset = it->second->size;
                  it->second->size = sectionSizes[section];
                  this->updateSymTab(section, it->second->size, newOffset, old_symTab);
                    
                    
                  }
                  else{
                    SectionData* newSec = new SectionData(section);
                    newSec->size = size;
                    if(placedSections.count(section) > 0){
                      newSec->start = placedSections[section];
                    }
                    else{
                      newSec->start = -1;
                    }
                    
                    
                    this->sections[section] = newSec;
                    
                    
                  }
                  processed[f].push_back(section);
              }
            }
            
            
            
            
            break;
          }
          if(line.substr(0,5) == "code."){
            // OVDE DODATI LOGIKU ZA SECTION ID
            
            section = line.substr(5);
            processing = 3;
            int sectionId;
            newOffset = 0;
            int size;
            for(ST_entry* s : this->symbolTable){
              if(s->name == section){
                sectionId = s->id;
                size = s->size;
              }
            }
            if(processed.count(f) == 0){
              
                auto it = sections.find(section);
              

              
              
              
                if(it != this->sections.end()){

                    
                    
                  newOffset = it->second->size;
                  it->second->size = sectionSizes[section];
                  this->updateSymTab(section, it->second->size, newOffset, old_symTab);
                    
                    
                  }
                  else{
                    SectionData* newSec = new SectionData(section);
                    newSec->size = size;
                    if(placedSections.count(section) > 0){
                      newSec->start = placedSections[section];
                    }
                    else{
                      newSec->start = -1;
                    }
                    
                    
                    this->sections[section] = newSec;
                    
                    
                  }
                  processed[f].push_back(section);
              

            }
            else{
              auto found = std::find(processed[f].begin(), processed[f].end(), section);
              if(found != processed[f].end()){
                break;
              }
              else{
                auto it = sections.find(section);
              

              
              
              
                if(it != this->sections.end()){

                    
                    
                  newOffset = it->second->size;
                  it->second->size = sectionSizes[section];
                  this->updateSymTab(section, it->second->size, newOffset, old_symTab);
                    
                    
                  }
                  else{
                    SectionData* newSec = new SectionData(section);
                    newSec->size = size;
                    if(placedSections.count(section) > 0){
                      newSec->start = placedSections[section];
                    }
                    else{
                      newSec->start = -1;
                    }
                    
                    
                    this->sections[section] = newSec;
                    
                    
                  }
                  processed[f].push_back(section);
              }
            }
            
            break;
          }
          string newSectionName = "";
          std::vector<string> entry = split(line, ":");
          int size = stoi(entry.back());
          
          entry.pop_back();
          int section = stoi(entry.back());
          
          entry.pop_back();
          bool defined = stoi(entry.back());
          entry.pop_back();
          Type type = this->intToType(stoi(entry.back()));
          entry.pop_back();

          BindingType bindType = this->intToBindType(stoi(entry.back()));
          entry.pop_back();
          int value = stoi(entry.back());
          entry.pop_back();
          string name = entry.back();
          entry.pop_back();
          int id = stoi(entry.back());
          if(id == section){
            sectionSizes[name] += size;
            oldSectionsMap[section] = name; 
          }
          
          ST_entry* oldEntry = new ST_entry(name, size, id, value, section, bindType, type, defined);
            
          old_symTab.push_back(oldEntry);
            
          if(defined && id != 0){
            ST_entry* test = isInSymTab(name);
            if(test){
              
              if(section != id && section != 0 && test->section != 0){
              
                cerr << "Greska, simbol " <<  name << " je vec definisan" << endl;
                exit(-1);
              }
            
              if(test->section == 0){
                test->section = section;
              }
              

            }
            else{
              if(section == id){
              ST_entry* newEntry = new ST_entry(name, size, value, type, defined);

              newSectionsMap[newEntry->name] = newEntry->id; 

              this->symbolTable.push_back(newEntry);
              }
              else{
                ST_entry* newEntry = new ST_entry(name, size, value, section, type, defined);
                          
                this->symbolTable.push_back(newEntry);
              }
            }

            
            
            
            
            
            
            
          }
          break;
        }
        case 2: {
          // relokacioni zapisi
          this->updateSectionId();
          if(line.substr(0,5) == "code."){
            
            section = line.substr(5);
            processing = 3;
            break;
          }
         
          std::vector<string> relaRecord = split(line, ":");
          int addend = stoi(relaRecord.back());
          relaRecord.pop_back();
          int symbol = stoi(relaRecord.back());
          relaRecord.pop_back();
          int offset = stoi(relaRecord.back());

          RelaData* newRela = new RelaData();
          newRela->addend = addend;
          newRela->offset = offset + newOffset;
          for(ST_entry* entry : old_symTab){
            if(entry->id == symbol){
              newRela->symbol = entry->name;
            }
          }
          auto it = this->sections.find(section);
          it->second->rela.push_back(newRela); 
          break;
        }
        case 3: {
          // code
          
          this->updateSectionId();
          if(line.substr(0,5) == "rela."){
            section = line.substr(5);
            processing = 2;
            int sectionId;
            newOffset = 0;
            int size;
            for(ST_entry* s : this->symbolTable){
              if(s->name == section){
                sectionId = s->id;
                size = s->size;
              }
            }
            if(processed.count(f) == 0){
              
            
                auto it = sections.find(section);
              

              
              
              
                if(it != this->sections.end()){

                    
                    
                  newOffset = it->second->size;
                  it->second->size = sectionSizes[section];
                  this->updateSymTab(section, it->second->size, newOffset, old_symTab);
                    
                    
                  }
                  else{
                    SectionData* newSec = new SectionData(section);
                    newSec->size = size;
                    if(placedSections.count(section) > 0){
                      newSec->start = placedSections[section];
                    }
                    else{
                      newSec->start = -1;
                    }
                    
                    
                    this->sections[section] = newSec;
                    
                    
                  }
                  processed[f].push_back(section);
              
            }
            else{
              auto found = std::find(processed[f].begin(), processed[f].end(), section);
              if(found != processed[f].end()){
                break;
              }
              else{
                auto it = sections.find(section);
              

              
              
              
                if(it != this->sections.end()){

                    
                    
                  newOffset = it->second->size;
                  it->second->size = sectionSizes[section];
                  this->updateSymTab(section, it->second->size, newOffset, old_symTab);
                    
                    
                  }
                  else{
                    SectionData* newSec = new SectionData(section);
                    newSec->size = size;
                    if(placedSections.count(section) > 0){
                      newSec->start = placedSections[section];
                    }
                    else{
                      newSec->start = -1;
                    }
                    
                    
                    this->sections[section] = newSec;
                    
                    
                  }
                  processed[f].push_back(section);
              }
            }
            
            break;
          }
          if(line.substr(0,5) == "code."){
            section = line.substr(5);
            processing = 3;
            int sectionId;
            newOffset = 0;
            int size;
            for(ST_entry* s : this->symbolTable){
              if(s->name == section){
                sectionId = s->id;
                size = s->size;
              }
            }
            if(processed.count(f) == 0){
             
            
                auto it = sections.find(section);
              

              
              
              
                if(it != this->sections.end()){

                    
                    
                  newOffset = it->second->size;
                  it->second->size = sectionSizes[section];
                  this->updateSymTab(section, it->second->size, newOffset, old_symTab);
                    
                    
                  }
                  else{
                    SectionData* newSec = new SectionData(section);
                    newSec->size = size;
                    if(placedSections.count(section) > 0){
                      newSec->start = placedSections[section];
                    }
                    else{
                      newSec->start = -1;
                    }
                    
                    
                    this->sections[section] = newSec;
                    
                    
                  }
                  processed[f].push_back(section);
              
              
            }
            else{
              auto found = std::find(processed[f].begin(), processed[f].end(), section);
              if(found != processed[f].end()){
                break;
              }
              else{
                auto it = sections.find(section);
              

              
              
              
                if(it != this->sections.end()){

                    
                    
                  newOffset = it->second->size;
                  it->second->size = sectionSizes[section];
                  this->updateSymTab(section, it->second->size, newOffset, old_symTab);
                    
                    
                  }
                  else{
                    SectionData* newSec = new SectionData(section);
                    newSec->size = size;
                    if(placedSections.count(section) > 0){
                      newSec->start = placedSections[section];
                    }
                    else{
                      newSec->start = -1;
                    }
                    
                    
                    this->sections[section] = newSec;
                    
                    
                  }
                  processed[f].push_back(section);
              }
            }
            
            break;
          }
          
          auto it = this->sections.find(section);
          it->second->code.push_back(line);
          

          break;
        }
      }
      
          //POCINJE C
          //PROVERA PREKLAPANJA
          
      
          /*for(ST_entry* s: old_symTab){
            cout << s->id << ":"
                << s->name << ":"
                << s->value << ":"
                << s->bindType << ":"
                << s->type << ":"
                << s->defined << ":"
                << s->section << ":"
                << s->size  << endl;
          }*/
    }
      
    inputFile.close();
    /*for(ST_entry* s : old_symTab){
      cout << s->id << ":"
                << s->name << ":"
                << s->value << ":"
                << s->bindType << ":"
                << s->type << ":"
                << s->defined << ":"
                << s->section << ":"
                << s->size  << endl;
    }
    cout << "===============================" << endl;*/
    for(ST_entry* s : old_symTab){
      delete(s);
    }
    // cout << "NAKON FAJLA: " << f << endl;
    /*for (const auto& pair : oldSectionsMap) {
      std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }
  
    for (const auto& pair : newSectionsMap) {
        std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }*/
    

    
    
    
  }
  /*for(ST_entry* s: this->symbolTable){
            cout << s->id << ":"
                << s->name << ":"
                << s->value << ":"
                << s->type << ":"
                << s->defined << ":"
                << s->section << ":"
                << s->size  << endl;

  }*/
  
  

  
  for(ST_entry* st : this->symbolTable){
    if(st->section == 0 && st->id != 0){
      cerr << "Greska, simbol " <<  st->name << " nije definisan" << endl;
      exit(-1);
    }
  }

  checkOverlap();
  SectionData* max = new SectionData();
  max->start = 0;
  for(const auto& pair : this->sections){
    if(pair.second->start >= max->start){
      max = pair.second;
    }
  }
  
  for(const auto& pair : this->sections){
    if(pair.second->start == -1){
      pair.second->start = max->start + max->size;
      max = pair.second;
    }
  }

 for(const auto& pair : this->sections){
    cout << pair.first << ":" << pair.second->start << endl;
  }
 this->updateValues();
  for(ST_entry* s: this->symbolTable){
            cout << s->id << ":"
                << s->name << ":"
                << s->value << ":"
                << s->type << ":"
                << s->defined << ":"
                << s->section << ":"
                << s->size  << endl;

  }

  
  
 
}

void Linker::updateValues(){
  map<int, string> idToName;
  for(const auto& pair : this->newSectionsMap){
    idToName[pair.second] = pair.first;
  }
  for(const auto& pair : idToName){
    cout << pair.first << ":" << pair.second << endl;
  }
  for(ST_entry* s : this->symbolTable){
    if(s->id != s->section){
      string sec = idToName[s->section];
      s->value += this->sections[sec]->start;
    }
  }
}

void Linker::updateSymTab(string section, long long int size, int newOffset, std::list<ST_entry*> oldSymTab){
  int sectionId = -1;
  for(ST_entry* s : this->symbolTable){
    if(s->name == section){
      s->size = size;
      sectionId = s->section;

      
    }
  }


  for(ST_entry* old : oldSymTab){
    if(old->defined && old->section != 0 && old->section != old->id){
      for(ST_entry* curr : this->symbolTable){
        if(old->name == curr->name && curr->section == sectionId){
          curr->value = old->value + newOffset;
        }
      }
    }
  }
  


}

void Linker::checkOverlap(){
  map<long long int, long long int> sortedSections;
  for(const auto& pair : this->sections){
    sortedSections[pair.second->start] = pair.second->size; 
  }
  for (auto it = sortedSections.begin(); it != std::prev(sortedSections.end()); ++it) {
        auto next_it = std::next(it);
        if((it->first + it->second) > next_it->first){
          cerr << "Doslo je do preklapanja sekcija" << endl;
          exit(-1);
        }
  }

}

    
    

  



  


std::vector<std::string> Linker::split(std::string s, std::string delimiter){
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr (pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back (token);
  }

  res.push_back (s.substr (pos_start));
  return res;
}

ST_entry* Linker::isInSymTab(string symbol){
    
  for(ST_entry* s : this->symbolTable){
    if(s->name.compare(symbol) == 0) return s; 
  }

  return nullptr;
}

Type Linker::intToType(int i){
  switch(i){
    case 0:
      return NOTYPE;
    case 1:
      return OBJECT;
    case 2:
      return FUNC;
    case 3:
      return SECTION;
    default:
      return NOTYPE;
  }
}

BindingType Linker::intToBindType(int i){
  switch(i){
    case 0:
      return LOCAL;
    case 1:
      return GLOBAL;
    case 2:
      return WEAK;
    default:
      return WEAK;
  }
}

void Linker::relocation(){

  for(auto it : this->sections){
    
    for(RelaData* s : it.second->rela){
       auto i = it.second->code.begin() + (s->offset / 4);
       for(ST_entry* entry : this->symbolTable){
        if(entry->name == s->symbol){
          std::stringstream ss;
          ss << std::uppercase << std::hex << (entry->value + s->addend);
          string hexa = ss.str();
          
          string finished;
          while(hexa.size() < 8){
            hexa.insert(0, 1, '0');
          }
      
          finished = hexa.substr(6) + " " + hexa.substr(4,2) + " " + hexa.substr(2,2) + " " +  hexa.substr(0,2);
          *i = finished;
          break;
        }
       }
           
    }
  }
  



}

void Linker::generateCode(){
  map<string, string> outCode;
  for(const auto& pair : this->sections){
    int start = pair.second->start;
  
    
    for(string s : pair.second->code){
      std::stringstream ss;
      ss << std::setw(4) << std::setfill('0') << std::uppercase << std::hex << start;
      outCode[ss.str()] = s;
      start += 4;

    }
  }
  std::ofstream outFile;
  outFile.open(this->outputFile);
  if (!outFile) {
    std::cerr << "Error opening file!" << std::endl;
    return;
  }
  for(const auto& p : outCode){
    outFile << p.first << ":" << p.second << endl;
  }

  outFile.close();
 
}

int main(int argc, char* argv[]){
  Linker linker;

  linker.parseFiles(argc, argv);
  linker.relocation();
  linker.generateCode();
  
  return 0;


}