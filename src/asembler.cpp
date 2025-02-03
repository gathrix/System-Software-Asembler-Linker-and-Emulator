#include "../inc/asembler.hpp"

#include <cctype>
#include <fstream>
#include <cassert>
#include <iostream>
#include <cstdio>

int ST_entry::ID = 0;
int MC::Asembler::sOrL = 0;
int MC::Asembler::currSection = 0;
int MC::Asembler::locationCounter = 0;
int MC::Asembler::isInSection = 0;
int MC::Asembler::symbolOrliteral = 0;
OperationType MC::Asembler::opType = NONE;
MC::Asembler::~Asembler()
{
   delete(scanner);
   
   scanner = nullptr;
   delete(parser);
   parser = nullptr;
   for(ST_entry* s : this->symbolTable){
    delete(s);
   }
   for(SectionData* s : this->code){
    delete(s);
   }
   for(LiteralPool* s : this->literalPool){
    delete(s);
   }
}
void MC::Asembler::parse( const char * const filename )
{
   assert( filename != nullptr );
   std::ifstream in_file( filename );
   if( ! in_file.good() )
   {
       exit( EXIT_FAILURE );
   }
   parse_helper( in_file );
   return;
}
void MC::Asembler::parse_helper( std::istream &stream )
{
   
   delete(scanner);
   try
   {
      scanner = new MC::Scanner( &stream );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << "Failed to allocate scanner: (" <<
         ba.what() << "), exiting!!\n";
      exit( EXIT_FAILURE );
   }
   
   delete(parser); 
   try
   {
      parser = new MC::MC_Parser( (*scanner) /* scanner */, 
                                  (*this) /* driver */ );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << "Failed to allocate parser: (" << 
         ba.what() << "), exiting!!\n";
      exit( EXIT_FAILURE );
   }
   const int accept( 0 );
   if( parser->parse() != accept )
   {
      std::cerr << "Parse failed!!\n";
   }
   return;
}







void MC::Asembler::printSymTab(){
  
  
  cout << "         SYMBOL TABLE          " << endl;
  cout << "========================================" << endl;
  cout << "|ID|    |Name|    |Value|    |BindType|    |Defined|   |Section|" << endl;
  for(ST_entry* s: this->symbolTable){
    cout << *s << endl;
  }


}

void MC::Asembler::stRegCode(string operand, string gpr){
  operand = getRegisterNumber(operand);
  gpr = getRegisterNumber(gpr);
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "00";
  fourthByte->explanation = "Fourth Byte of ST";
  locationCounter++;
  this->code.push_back(fourthByte);


  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = gpr + "0";    
  thirdByte->explanation = "Third Byte of ST";
  locationCounter++;
  this->code.push_back(thirdByte);

  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = operand + "0";
  secondByte->explanation = "Second Byte of ST";
  locationCounter++;
  this->code.push_back(secondByte);
          
}


void MC::Asembler::ldRegCode(string operand, string gpr){
  operand = getRegisterNumber(operand);
  gpr = getRegisterNumber(gpr);
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "00";
  fourthByte->explanation = "Fourth Byte of LD";
  locationCounter++;
  this->code.push_back(fourthByte);


  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "00";    
  thirdByte->explanation = "Third Byte of LD";
  locationCounter++;
  this->code.push_back(thirdByte);

  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = gpr + operand;
  secondByte->explanation = "Second Byte of LD";
  locationCounter++;
  this->code.push_back(secondByte);
          
}

void MC::Asembler::stInstruction(string gpr, string operand){
  switch(this->opType){
    case REGDIRECT: {
      this->stRegCode(operand,gpr);
      SectionData* firstByte = new SectionData();
      firstByte->section = currSection;
      firstByte->offset = locationCounter;
      firstByte->code = "80";
      firstByte->explanation = "First Byte of ST";
      locationCounter++;
      this->code.push_back(firstByte);
      break;
    }
    case REGINDIRECT: {
      this->stRegCode(operand,gpr);
      SectionData* firstByte = new SectionData();
      firstByte->section = currSection;
      firstByte->offset = locationCounter;
      firstByte->code = "82";
      firstByte->explanation = "First Byte of ST";
      locationCounter++;
      this->code.push_back(firstByte);
      break;
    }
    case LITERALDIRECT: {
      operand = jmpLiteral(operand);
      gpr = getRegisterNumber(gpr);
      if(operand.length() > 3){
        //LITERAL POOL
        this->addToLiteralPool(operand, false);
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = "??";
        fourthByte->explanation = "Fourth Byte of ST";
        locationCounter++;
        this->code.push_back(fourthByte);
        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = gpr + "?";
        thirdByte->explanation = "Third Byte of ST" ;
        locationCounter++;
        this->code.push_back(thirdByte);
        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "0F";
        secondByte->explanation = "Second Byte of ST";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "80";
        firstByte->explanation = "First Byte of ST";
        locationCounter++;
        this->code.push_back(firstByte);




      }else{
        // NO LITERAL POOL
        while(operand.length()< 3){
          operand.insert(0, 1, '0');

        }
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of ST";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = gpr + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of ST";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "00";
        secondByte->explanation = "Second Byte of ST";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        
        
        firstByte->code = "80";
        
        firstByte->explanation = "First Byte of ST";
        locationCounter++;
        this->code.push_back(firstByte);

      }

      break;
    }
    case LITERALINDIRECT: {
      operand = jmpLiteral(operand);
      gpr = getRegisterNumber(gpr);
      if(operand.length() > 3){
        //LITERAL POOL
        this->addToLiteralPool(operand, false);
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = "??";
        fourthByte->explanation = "Fourth Byte of ST";
        locationCounter++;
        this->code.push_back(fourthByte);
        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = gpr + "?";
        thirdByte->explanation = "Third Byte of ST" ;
        locationCounter++;
        this->code.push_back(thirdByte);
        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "0F";
        secondByte->explanation = "Second Byte of ST";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "82";
        firstByte->explanation = "First Byte of ST";
        locationCounter++;
        this->code.push_back(firstByte);




      }else{
        // NO LITERAL POOL
        while(operand.length()< 3){
          operand.insert(0, 1, '0');

        }
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of ST";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = gpr + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of ST";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "00";
        secondByte->explanation = "Second Byte of ST";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        
        
        firstByte->code = "82";
        
        firstByte->explanation = "First Byte of ST";
        locationCounter++;
        this->code.push_back(firstByte);

      }
      break;
    }
    case SYMBOLDIRECT: {
      ST_entry* symbol = isInSymTab(operand);
      gpr = getRegisterNumber(gpr);
      if(symbol){
        //Postoji u TABELI
        if(symbol->defined){
          //DEFINISAN
          if(symbol->section == 0){
            //GLOBALAN ILI EXTERN
            this->addToLiteralPool(operand, true);

            this->stRelocCode(gpr);

            SectionData* secondByte = new SectionData();
            secondByte->section = currSection;
            secondByte->offset = locationCounter;
            secondByte->code = "0F";
            secondByte->explanation = "Second Byte of ST";
            locationCounter++;
            this->code.push_back(secondByte);
                
            SectionData* firstByte = new SectionData();
            firstByte->section = currSection;
            firstByte->offset = locationCounter;
            
            firstByte->code = "80";
            
            
            firstByte->explanation = "First Byte of ST";
            locationCounter++;
            this->code.push_back(firstByte);

          }else{
            if(symbol->section == currSection){
              // U TRENUNTOJ SEKCIJI
              int displacement = symbol->value - (locationCounter + 4);
              std::stringstream ss;
              ss << std::hex << std::uppercase << displacement;
              string sym_code = ss.str();
                
              if(displacement >= -2048 && displacement <= 2047)
              {
                  // STAJE U 12 bita

                if(displacement < 0){
                  sym_code = sym_code.substr(sym_code.length() - 3);

                }
                else{
                  while(sym_code.length() < 3){
                    sym_code.insert(0,1, '0');
                  }
                }

                SectionData* fourthByte = new SectionData();
                fourthByte->section = currSection;
                fourthByte->offset = locationCounter;
                fourthByte->code = sym_code.substr(1);
                fourthByte->explanation = "Fourth Byte of ST";
                locationCounter++;
                this->code.push_back(fourthByte);


                SectionData* thirdByte = new SectionData();
                thirdByte->section = currSection;
                thirdByte->offset = locationCounter;
                thirdByte->code = gpr + sym_code.substr(0,1);
                thirdByte->explanation = "Third Byte of ST";
                locationCounter++;
                this->code.push_back(thirdByte);

                SectionData* secondByte = new SectionData();
                secondByte->section = currSection;
                secondByte->offset = locationCounter;
                secondByte->code = "0F";
                secondByte->explanation = "Second Byte of ST";
                locationCounter++;
                this->code.push_back(secondByte);
                    
                SectionData* firstByte = new SectionData();
                firstByte->section = currSection;
                firstByte->offset = locationCounter;
                
                firstByte->code = "80";
                
                firstByte->explanation = "First Byte of ST";
                locationCounter++;
                this->code.push_back(firstByte);   

                }
                else{
                  // Ne staje u 12 bita
                  this->addToLiteralPool(operand, true);
                  this->stRelocCode(gpr);
                  SectionData* secondByte = new SectionData();
                  secondByte->section = currSection;
                  secondByte->offset = locationCounter;
                  secondByte->code = "0F";
                  secondByte->explanation = "Second Byte of ST";
                  locationCounter++;
                  this->code.push_back(secondByte);
                      
                  SectionData* firstByte = new SectionData();
                  firstByte->section = currSection;
                  firstByte->offset = locationCounter;
                  
                  firstByte->code = "80";
                  firstByte->explanation = "First Byte of ST";
                  locationCounter++;
                  this->code.push_back(firstByte);   
                  
                }
            }else{
              // NIJE U TRENUTNOJ SEKCIJI
              this->addToLiteralPool(operand, true);

              this->stRelocCode(gpr);

              SectionData* secondByte = new SectionData();
              secondByte->section = currSection;
              secondByte->offset = locationCounter;
              secondByte->code = "0F";
              secondByte->explanation = "Second Byte of ST";
              locationCounter++;
              this->code.push_back(secondByte);
                      
              SectionData* firstByte = new SectionData();
              firstByte->section = currSection;
              firstByte->offset = locationCounter;
                  
              firstByte->code = "80";
              firstByte->explanation = "First Byte of ST";
              locationCounter++;
              this->code.push_back(firstByte);
            }
          }
        }
        else{
          //NEDEFINISAN
          this->addToForwardRefs(symbol, false);
          this->stRelocCode(gpr);
            

          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = "0F";
          secondByte->explanation = "Second Byte of ST";
          locationCounter++;
          this->code.push_back(secondByte);
                      
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
                  
          firstByte->code = "80";
          firstByte->explanation = "First Byte of ST";
          locationCounter++;
          this->code.push_back(firstByte);
        }
      }
      else{
        //NEMA GA U TABELI
          ST_entry* newSym = new ST_entry(operand);
          this->addToForwardRefs(newSym, false);
          this->addToSymTab(newSym);
          this->stRelocCode(gpr);
            

          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = "0F";
          secondByte->explanation = "Second Byte of ST";
          locationCounter++;
          this->code.push_back(secondByte);
                      
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
                  
          firstByte->code = "80";
          firstByte->explanation = "First Byte of ST";
          locationCounter++;
          this->code.push_back(firstByte);
      }

      break;
    }
    case SYMBOLINDIRECT: {
      ST_entry* symbol = isInSymTab(operand);
      gpr = getRegisterNumber(gpr);
      if(symbol){
        //Postoji u TABELI
        if(symbol->defined){
          //DEFINISAN
          if(symbol->section == 0){
            //GLOBALAN ILI EXTERN
            this->addToLiteralPool(operand, true);

            this->stRelocCode(gpr);

            SectionData* secondByte = new SectionData();
            secondByte->section = currSection;
            secondByte->offset = locationCounter;
            secondByte->code = "0F";
            secondByte->explanation = "Second Byte of ST";
            locationCounter++;
            this->code.push_back(secondByte);
                
            SectionData* firstByte = new SectionData();
            firstByte->section = currSection;
            firstByte->offset = locationCounter;
            
            firstByte->code = "82";
            
            
            firstByte->explanation = "First Byte of ST";
            locationCounter++;
            this->code.push_back(firstByte);

          }else{
            if(symbol->section == currSection){
              // U TRENUNTOJ SEKCIJI
              int displacement = symbol->value - (locationCounter + 4);
              std::stringstream ss;
              ss << std::hex << std::uppercase << displacement;
              string sym_code = ss.str();
                
              if(displacement >= -2048 && displacement <= 2047)
              {
                  // STAJE U 12 bita

                if(displacement < 0){
                  sym_code = sym_code.substr(sym_code.length() - 3);

                }
                else{
                  while(sym_code.length() < 3){
                    sym_code.insert(0,1, '0');
                  }
                }

                SectionData* fourthByte = new SectionData();
                fourthByte->section = currSection;
                fourthByte->offset = locationCounter;
                fourthByte->code = sym_code.substr(1);
                fourthByte->explanation = "Fourth Byte of ST";
                locationCounter++;
                this->code.push_back(fourthByte);


                SectionData* thirdByte = new SectionData();
                thirdByte->section = currSection;
                thirdByte->offset = locationCounter;
                thirdByte->code = gpr + sym_code.substr(0,1);
                thirdByte->explanation = "Third Byte of ST";
                locationCounter++;
                this->code.push_back(thirdByte);

                SectionData* secondByte = new SectionData();
                secondByte->section = currSection;
                secondByte->offset = locationCounter;
                secondByte->code = "0F";
                secondByte->explanation = "Second Byte of ST";
                locationCounter++;
                this->code.push_back(secondByte);
                    
                SectionData* firstByte = new SectionData();
                firstByte->section = currSection;
                firstByte->offset = locationCounter;
                
                firstByte->code = "82";
                
                firstByte->explanation = "First Byte of ST";
                locationCounter++;
                this->code.push_back(firstByte);   

                }
                else{
                  // Ne staje u 12 bita
                  this->addToLiteralPool(operand, true);
                  this->stRelocCode(gpr);
                  SectionData* secondByte = new SectionData();
                  secondByte->section = currSection;
                  secondByte->offset = locationCounter;
                  secondByte->code = "0F";
                  secondByte->explanation = "Second Byte of ST";
                  locationCounter++;
                  this->code.push_back(secondByte);
                      
                  SectionData* firstByte = new SectionData();
                  firstByte->section = currSection;
                  firstByte->offset = locationCounter;
                  
                  firstByte->code = "82";
                  firstByte->explanation = "First Byte of ST";
                  locationCounter++;
                  this->code.push_back(firstByte);   
                  
                }
            }else{
              // NIJE U TRENUTNOJ SEKCIJI
              this->addToLiteralPool(operand, true);

              this->stRelocCode(gpr);

              SectionData* secondByte = new SectionData();
              secondByte->section = currSection;
              secondByte->offset = locationCounter;
              secondByte->code = "0F";
              secondByte->explanation = "Second Byte of ST";
              locationCounter++;
              this->code.push_back(secondByte);
                      
              SectionData* firstByte = new SectionData();
              firstByte->section = currSection;
              firstByte->offset = locationCounter;
                  
              firstByte->code = "82";
              firstByte->explanation = "First Byte of ST";
              locationCounter++;
              this->code.push_back(firstByte);
            }
          }
        }
        else{
          //NEDEFINISAN
          this->addToForwardRefs(symbol,false);
          this->stRelocCode(gpr);
            

          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = "0F";
          secondByte->explanation = "Second Byte of ST";
          locationCounter++;
          this->code.push_back(secondByte);
                      
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
                  
          firstByte->code = "82";
          firstByte->explanation = "First Byte of ST";
          locationCounter++;
          this->code.push_back(firstByte);
        }
      }
      else{
        //NEMA GA U TABELI
          ST_entry* newSym = new ST_entry(operand);
          this->addToForwardRefs(newSym,false);
          this->addToSymTab(newSym);
          this->stRelocCode(gpr);
            

          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = "0F";
          secondByte->explanation = "Second Byte of ST";
          locationCounter++;
          this->code.push_back(secondByte);
                      
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
                  
          firstByte->code = "82";
          firstByte->explanation = "First Byte of ST";
          locationCounter++;
          this->code.push_back(firstByte);
      }

      break;
    }
    case REGLITERALIND: {
      vector<string> operands = split(operand, " ");
      string literal = jmpLiteral(operands.back());
      operands.pop_back();
      string reg = getRegisterNumber(operands.back());
      gpr = getRegisterNumber(gpr);
      
     
      while(literal.length() < 3){
        literal.insert(0, 1, '0');
      }
      SectionData* fourthByte = new SectionData();
      fourthByte->section = currSection;
      fourthByte->offset = locationCounter;
      fourthByte->code = literal.substr(1);
      fourthByte->explanation = "Fourth Byte of ST";
      locationCounter++;
      this->code.push_back(fourthByte);


      SectionData* thirdByte = new SectionData();
      thirdByte->section = currSection;
      thirdByte->offset = locationCounter;
      thirdByte->code = gpr + literal.substr(0,1);
      thirdByte->explanation = "Third Byte of ST";
      locationCounter++;
      this->code.push_back(thirdByte);

      SectionData* secondByte = new SectionData();
      secondByte->section = currSection;
      secondByte->offset = locationCounter;
      secondByte->code = reg + "0";
      secondByte->explanation = "Second Byte of ST";
      locationCounter++;
      this->code.push_back(secondByte);
          
      SectionData* firstByte = new SectionData();
      firstByte->section = currSection;
      firstByte->offset = locationCounter;
      firstByte->code = "82";
      firstByte->explanation = "First Byte of ST";
      locationCounter++;
      this->code.push_back(firstByte);

      break;
    }
    case REGSYMBOLIND: {
      vector<string> operands = split(operand, " ");
      
      string symbol = operands.back();
      operands.pop_back();
      string reg = getRegisterNumber(operands.back());
      gpr = getRegisterNumber(gpr);
      ST_entry* sym = isInSymTab(symbol);
      if(sym){
        // U TABELI
        if(sym->defined){
          if(sym->section == 0){
            // GLOBAL I EXTERN
            this->addToForwardRefs(sym,false);
            SectionData* fourthByte = new SectionData();
            fourthByte->section = currSection;
            fourthByte->offset = locationCounter;
            fourthByte->code = "??";
            fourthByte->explanation = "Fourth Byte of ST";
            locationCounter++;
            this->code.push_back(fourthByte);
            SectionData* thirdByte = new SectionData();
            thirdByte->section = currSection;
            thirdByte->offset = locationCounter;
            thirdByte->code = gpr + "?";
            thirdByte->explanation = "Third Byte of ST";
            locationCounter++;
            this->code.push_back(thirdByte);
            SectionData* secondByte = new SectionData();
            secondByte->section = currSection;
            secondByte->offset = locationCounter;
            secondByte->code = reg + "F";
            secondByte->explanation = "Second Byte of ST";
            locationCounter++;
            this->code.push_back(secondByte);
        
            SectionData* firstByte = new SectionData();
            firstByte->section = currSection;
            firstByte->offset = locationCounter;
            firstByte->code = "82";
            firstByte->explanation = "First Byte of ST";
            locationCounter++;
            this->code.push_back(firstByte);

          }
          else{
            //LOCAL
            if(sym->section == this->currSection){
              // trenutna sekcija
              int disp = sym->value - (locationCounter + 4);
              
                
              if(disp >= -2048 && disp <= 2047)
              {
                  // STAJE U 12 bita
                std::stringstream ss;
                ss << std::hex << std::uppercase << disp;
                string sym_code = ss.str();  

                if(disp < 0){
                  sym_code = sym_code.substr(sym_code.length() - 3);

                }
                else{
                  while(sym_code.length() < 3){
                    sym_code.insert(0,1, '0');
                  }
                }

                this->stRegSymIndCode(sym_code, gpr, reg);    

                }
                else{
                  // Ne staje u 12 bita
                 cout << "GRESKA SIMBOL NE STAJE U 12 Bita" << endl;
                 exit(-1);
                  
                }
            }
            else{
              // DRUGA SEKCIJA
              cout << "GRESKA, VREDNOST SIMBOLA NIJE POZNATA NA KRAJU ASEMBLIRANJA" << endl;
              exit(-1);
            }
          }
        }
        else{
          // SIMBOL NIJE DEFINISAN
          this->addToForwardRefs(sym, false);
          SectionData* fourthByte = new SectionData();
          fourthByte->section = currSection;
          fourthByte->offset = locationCounter;
          fourthByte->code = "??";
          fourthByte->explanation = "Fourth Byte of ST";
          locationCounter++;
          this->code.push_back(fourthByte);
          SectionData* thirdByte = new SectionData();
          thirdByte->section = currSection;
          thirdByte->offset = locationCounter;
          thirdByte->code = gpr + "?";
          thirdByte->explanation = "Third Byte of ST";
          locationCounter++;
          this->code.push_back(thirdByte);
            

          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = reg + "F";
          secondByte->explanation = "Second Byte of ST";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "82";
          firstByte->explanation = "First Byte of ST";
          locationCounter++;
          this->code.push_back(firstByte);
        }
      }else{
        //NIJE U TABELI SIMBOLA
          ST_entry* newSym = new ST_entry(symbol);
          this->addToForwardRefs(newSym, false);
          this->addToSymTab(newSym);
          SectionData* fourthByte = new SectionData();
          fourthByte->section = currSection;
          fourthByte->offset = locationCounter;
          fourthByte->code = "??";
          fourthByte->explanation = "Fourth Byte of ST";
          locationCounter++;
          this->code.push_back(fourthByte);
          SectionData* thirdByte = new SectionData();
          thirdByte->section = currSection;
          thirdByte->offset = locationCounter;
          thirdByte->code = gpr + "?";
          thirdByte->explanation = "Third Byte of ST";
          locationCounter++;
          this->code.push_back(thirdByte);
          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code =  reg + "F";
          secondByte->explanation = "Second Byte of ST";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "82";
          firstByte->explanation = "First Byte of ST";
          locationCounter++;
          this->code.push_back(firstByte);
      }
      
      
      
      break;
    }

  
  }

}

void MC::Asembler::ldInstruction(string operand, string gpr){
  switch(this->opType){
    case REGDIRECT: {
      this->ldRegCode(operand,gpr);
      SectionData* firstByte = new SectionData();
      firstByte->section = currSection;
      firstByte->offset = locationCounter;
      firstByte->code = "91";
      firstByte->explanation = "First Byte of LD";
      locationCounter++;
      this->code.push_back(firstByte);
      break;
    }
    case REGINDIRECT: {
      this->ldRegCode(operand,gpr);
      SectionData* firstByte = new SectionData();
      firstByte->section = currSection;
      firstByte->offset = locationCounter;
      firstByte->code = "92";
      firstByte->explanation = "First Byte of LD";
      locationCounter++;
      this->code.push_back(firstByte);
      break;
    }
    case LITERALDIRECT: {
      operand = jmpLiteral(operand);
      gpr = getRegisterNumber(gpr);
      if(operand.length() > 3){
        //LITERAL POOL
        this->addToLiteralPool(operand, false);
        this->relocCode("LD");
        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = gpr + "F";
        secondByte->explanation = "Second Byte of LD";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "92";
        firstByte->explanation = "First Byte of LD";
        locationCounter++;
        this->code.push_back(firstByte);




      }else{
        // NO LITERAL POOL
        while(operand.length()< 3){
          operand.insert(0, 1, '0');

        }
        this->ldCode(gpr, operand);

      }

      break;
    }
    case LITERALINDIRECT: {
      operand = jmpLiteral(operand);
      gpr = getRegisterNumber(gpr);
      if(operand.length() > 3){
        //LITERAL POOL
        this->addToLiteralPool(operand, false);
        this->relocCode("LD");
        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = gpr + "F";
        secondByte->explanation = "Second Byte of LD";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "92";
        firstByte->explanation = "First Byte of LD";
        locationCounter++;
        this->code.push_back(firstByte);




      }else{
        // NO LITERAL POOL
        while(operand.length()< 3){
          operand.insert(0, 1, '0');

        }
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of LD";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = "0" + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of LD";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = gpr + "0";
        secondByte->explanation = "Second Byte of LD";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "92";
        firstByte->explanation = "First Byte of LD";
        locationCounter++;
        this->code.push_back(firstByte);
      }
      break;
    }
    case SYMBOLDIRECT: {
      ST_entry* symbol = isInSymTab(operand);
      gpr = getRegisterNumber(gpr);
      if(symbol){
        //Postoji u TABELI
        if(symbol->defined){
          //DEFINISAN
          if(symbol->section == 0){
            //GLOBALAN ILI EXTERN
            this->addToLiteralPool(operand, true);

            this->relocCode("LD");

            this->ldLiteralCode(gpr);

          }else{
            if(symbol->section == currSection){
              // U TRENUNTOJ SEKCIJI
              int displacement = symbol->value - (locationCounter + 4);
              std::stringstream ss;
              ss << std::hex << std::uppercase << displacement;
              string sym_code = ss.str();
                
              if(displacement >= -2048 && displacement <= 2047)
              {
                  // STAJE U 12 bita

                if(displacement < 0){
                  sym_code = sym_code.substr(sym_code.length() - 3);

                }
                else{
                  while(sym_code.length() < 3){
                    sym_code.insert(0,1, '0');
                  }
                }

                this->ldCode(gpr,sym_code);    

                }
                else{
                  // Ne staje u 12 bita
                  this->addToLiteralPool(operand, true);
                  this->relocCode("LD");

                  this->ldLiteralCode(gpr);
                  
                }
            }else{
              // NIJE U TRENUTNOJ SEKCIJI
              this->addToLiteralPool(operand, true);

              this->relocCode("LD");

              this->ldLiteralCode(gpr);
            }
          }
        }
        else{
          //NEDEFINISAN
          this->addToForwardRefs(symbol,false);
          this->relocCode("LD");
            

          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = gpr + "F";
          secondByte->explanation = "Second Byte of LD";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "92";
          firstByte->explanation = "First Byte of LD";
          locationCounter++;
          this->code.push_back(firstByte);
        }
      }
      else{
        //NEMA GA U TABELI
          ST_entry* newSym = new ST_entry(operand);
          this->addToForwardRefs(newSym, false);
          this->addToSymTab(newSym);
          this->relocCode("LD");
          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = gpr + "F";
          secondByte->explanation = "Second Byte of LD";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "92";
          firstByte->explanation = "First Byte of LD";
          locationCounter++;
          this->code.push_back(firstByte);
      }

      break;
    }
    case SYMBOLINDIRECT: {
      ST_entry* symbol = isInSymTab(operand);
      gpr = getRegisterNumber(gpr);
      if(symbol){
        //Postoji u TABELI
        if(symbol->defined){
          //DEFINISAN
          if(symbol->section == 0){
            //GLOBALAN ILI EXTERN
            this->addToLiteralPool(operand, true);

            this->relocCode("LD");

            this->ldLiteralCode(gpr);

          }else{
            if(symbol->section == currSection){
              // U TRENUNTOJ SEKCIJI
              int displacement = symbol->value - (locationCounter + 4);
              std::stringstream ss;
              ss << std::hex << std::uppercase << displacement;
              string sym_code = ss.str();
                
              if(displacement >= -2048 && displacement <= 2047)
              {
                  // STAJE U 12 bita

                if(displacement < 0){
                  sym_code = sym_code.substr(sym_code.length() - 3);

                }
                else{
                  while(sym_code.length() < 3){
                    sym_code.insert(0,1, '0');
                  }
                }

                this->ldCode(gpr,sym_code);    

                }
                else{
                  // Ne staje u 12 bita
                  this->addToLiteralPool(operand, true);
                  this->relocCode("LD");

                  this->ldLiteralCode(gpr);
                  
                }
            }else{
              // NIJE U TRENUTNOJ SEKCIJI
              this->addToLiteralPool(operand, true);

              this->relocCode("LD");

              this->ldLiteralCode(gpr);
            }
          }
        }
        else{
          //NEDEFINISAN
          this->addToForwardRefs(symbol,false);
          this->relocCode("LD");
            

          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = gpr + "F";
          secondByte->explanation = "Second Byte of LD";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "92";
          firstByte->explanation = "First Byte of LD";
          locationCounter++;
          this->code.push_back(firstByte);
        }
      }
      else{
        //NEMA GA U TABELI
          ST_entry* newSym = new ST_entry(operand);
          this->addToForwardRefs(newSym, false);
          this->addToSymTab(newSym);
          this->relocCode("LD");
          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = gpr + "F";
          secondByte->explanation = "Second Byte of LD";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "92";
          firstByte->explanation = "First Byte of LD";
          locationCounter++;
          this->code.push_back(firstByte);
      }
      break;
    }
    case REGLITERALIND: {
      vector<string> operands = split(operand, " ");
      string literal = jmpLiteral(operands.back());
      operands.pop_back();
      string operand = getRegisterNumber(operands.back());
      gpr = getRegisterNumber(gpr);
     
      while(literal.length() < 3){
        literal.insert(0, 1, '0');
      }
      SectionData* fourthByte = new SectionData();
      fourthByte->section = currSection;
      fourthByte->offset = locationCounter;
      fourthByte->code = literal.substr(1);
      fourthByte->explanation = "Fourth Byte of LD";
      locationCounter++;
      this->code.push_back(fourthByte);


      SectionData* thirdByte = new SectionData();
      thirdByte->section = currSection;
      thirdByte->offset = locationCounter;
      thirdByte->code = "0" + literal.substr(0,1);
      thirdByte->explanation = "Third Byte of LD";
      locationCounter++;
      this->code.push_back(thirdByte);

      SectionData* secondByte = new SectionData();
      secondByte->section = currSection;
      secondByte->offset = locationCounter;
      secondByte->code = gpr + operand;
      secondByte->explanation = "Second Byte of LD";
      locationCounter++;
      this->code.push_back(secondByte);
          
      SectionData* firstByte = new SectionData();
      firstByte->section = currSection;
      firstByte->offset = locationCounter;
      firstByte->code = "92";
      firstByte->explanation = "First Byte of LD";
      locationCounter++;
      this->code.push_back(firstByte);

      break;
    }
    case REGSYMBOLIND: {
      vector<string> operands = split(operand, " ");
      
      string symbol = operands.back();
      operands.pop_back();
      string reg = getRegisterNumber(operands.back());
      gpr = getRegisterNumber(gpr);
      ST_entry* sym = isInSymTab(symbol);
      if(sym){
        // U TABELI
        if(sym->defined){
          if(sym->section == 0){
            // GLOBAL I EXTERN
            this->addToForwardRefs(sym, false);
            SectionData* fourthByte = new SectionData();
            fourthByte->section = currSection;
            fourthByte->offset = locationCounter;
            fourthByte->code = "??";
            fourthByte->explanation = "Fourth Byte of LD";
            locationCounter++;
            this->code.push_back(fourthByte);
            SectionData* thirdByte = new SectionData();
            thirdByte->section = currSection;
            thirdByte->offset = locationCounter;
            thirdByte->code = "F?";
            thirdByte->explanation = "Third Byte of LD";
            locationCounter++;
            this->code.push_back(thirdByte);
            SectionData* secondByte = new SectionData();
            secondByte->section = currSection;
            secondByte->offset = locationCounter;
            secondByte->code = gpr + reg;
            secondByte->explanation = "Second Byte of LD";
            locationCounter++;
            this->code.push_back(secondByte);
        
            SectionData* firstByte = new SectionData();
            firstByte->section = currSection;
            firstByte->offset = locationCounter;
            firstByte->code = "92";
            firstByte->explanation = "First Byte of LD";
            locationCounter++;
            this->code.push_back(firstByte);

          }
          else{
            //LOCAL
            if(sym->section == this->currSection){
              // trenutna sekcija
              int disp = sym->value - (locationCounter + 4);
              
                
              if(disp >= -2048 && disp <= 2047)
              {
                  // STAJE U 12 bita
                std::stringstream ss;
                ss << std::hex << std::uppercase << disp;
                string sym_code = ss.str();  

                if(disp < 0){
                  sym_code = sym_code.substr(sym_code.length() - 3);

                }
                else{
                  while(sym_code.length() < 3){
                    sym_code.insert(0,1, '0');
                  }
                }

                this->ldRegSymIndCode(sym_code, gpr, reg);    

                }
                else{
                  // Ne staje u 12 bita
                 cout << "GRESKA SIMBOL NE STAJE U 12 Bita" << endl;
                 exit(-1);
                  
                }
            }
            else{
              // DRUGA SEKCIJA
              cout << "GRESKA, VREDNOST SIMBOLA NIJE POZNATA NA KRAJU ASEMBLIRANJA" << endl;
              exit(-1);
            }
          }
        }
        else{
          // SIMBOL NIJE DEFINISAN
          this->addToForwardRefs(sym,false);
          SectionData* fourthByte = new SectionData();
          fourthByte->section = currSection;
          fourthByte->offset = locationCounter;
          fourthByte->code = "??";
          fourthByte->explanation = "Fourth Byte of LD";
          locationCounter++;
          this->code.push_back(fourthByte);
          SectionData* thirdByte = new SectionData();
          thirdByte->section = currSection;
          thirdByte->offset = locationCounter;
          thirdByte->code = "F?";
          thirdByte->explanation = "Third Byte of LD";
          locationCounter++;
          this->code.push_back(thirdByte);
            

          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = gpr + reg;
          secondByte->explanation = "Second Byte of LD";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "92";
          firstByte->explanation = "First Byte of LD";
          locationCounter++;
          this->code.push_back(firstByte);
        }
      }else{
        //NIJE U TABELI SIMBOLA
          ST_entry* newSym = new ST_entry(symbol);
          this->addToForwardRefs(newSym,false);
          this->addToSymTab(newSym);
          SectionData* fourthByte = new SectionData();
          fourthByte->section = currSection;
          fourthByte->offset = locationCounter;
          fourthByte->code = "??";
          fourthByte->explanation = "Fourth Byte of LD";
          locationCounter++;
          this->code.push_back(fourthByte);
          SectionData* thirdByte = new SectionData();
          thirdByte->section = currSection;
          thirdByte->offset = locationCounter;
          thirdByte->code = "F?";
          thirdByte->explanation = "Third Byte of LD";
          locationCounter++;
          this->code.push_back(thirdByte);
          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = gpr + reg;
          secondByte->explanation = "Second Byte of LD";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "92";
          firstByte->explanation = "First Byte of LD";
          locationCounter++;
          this->code.push_back(firstByte);
      }
      
      
      
      break;
    }

  }
}
void MC::Asembler::stRegSymIndCode(string operand, string gpr, string reg){
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of ST";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = gpr + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of ST";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = reg + "F";
        secondByte->explanation = "Second Byte of ST";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "82";
        firstByte->explanation = "First Byte of ST";
        locationCounter++;
        this->code.push_back(firstByte);

}

void MC::Asembler::ldRegSymIndCode(string operand, string gpr, string reg){
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of LD";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = "F" + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of LD";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = gpr + reg;
        secondByte->explanation = "Second Byte of LD";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "92";
        firstByte->explanation = "First Byte of LD";
        locationCounter++;
        this->code.push_back(firstByte);

}

std::vector<std::string> MC::Asembler::split(std::string s, std::string delimiter){
  
  
  
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

void MC::Asembler::wordDirective(string list){
  std::vector<std::string> symbols = split(list, " ");
  switch(this->sOrL){
    case 1:{
      //symbolList
      for(string symbol : symbols){
        if(symbol == "") continue;
        ST_entry* sym = isInSymTab(symbol);
        if(sym){
          if(sym->defined){
            if(sym->section == 0){
              // GLOBAL ili EXTERN
              RelocTable* newReloc = new RelocTable();
              newReloc->offset = locationCounter;
              newReloc->section = currSection;
              newReloc->addend = 0;
              newReloc->symbol = sym->id;
              newReloc->type = "SYMBOL";
              this->relocTable.push_back(newReloc);

            }
            else{
              //LOKALNI
              RelocTable* newReloc = new RelocTable();
              newReloc->offset = locationCounter;
              newReloc->section = currSection;
              newReloc->addend = sym->value;
              newReloc->symbol = sym->section;
              newReloc->type = "SYMBOL";
              this->relocTable.push_back(newReloc);
            }
          }
          else{
            //NIJE DEFINISAN
            this->addToForwardRefs(sym, true);
          }

        }
        else{
          // nije u tabeli simbola
          ST_entry* newSym = new ST_entry(symbol);
          this->addToForwardRefs(newSym, true);
          this->addToSymTab(newSym);
          
        }
        
        for(int i = 0; i < 4; i++){
          SectionData* relocCode = new SectionData();
          relocCode->section = currSection;
          relocCode->offset = locationCounter;
          relocCode->code = "??";
          relocCode->explanation = "WORD";
          locationCounter++;
          this->code.push_back(relocCode);
        }
      }
      
      break;
    }
    case -1: {
      //Literal List
      
      for(string literal : symbols){
        if(literal == "") continue;
        string code = jmpLiteral(literal);
        
        while(code.length() < 8){
          code.insert(0,1,'0');
        }
        
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = code.substr(6,2);
        fourthByte->explanation = "WORD4";
        locationCounter++;
        this->code.push_back(fourthByte);
        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = code.substr(4,2);
        thirdByte->explanation = "WORD3";
        locationCounter++;
        this->code.push_back(thirdByte);
        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code =  code.substr(2,2);
        secondByte->explanation = "WORD2";
        locationCounter++;
        this->code.push_back(secondByte);
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = code.substr(0,2);
        firstByte->explanation = "WORD1";
        locationCounter++;
        this->code.push_back(firstByte);


      }
      
      break;
    }

  }

}


void MC::Asembler::addExternSymbols(std::string symbolString){
    std::vector<std::string> symbols = split(symbolString, " ");
    for(std::string symName: symbols){
    if(!isInSymTab(symName)){
        if(symName != ""){
          ST_entry* newEntry = new ST_entry(symName, true, BindingType::GLOBAL, 0);
          addToSymTab(newEntry);  
        }
      }
      
      
    }
}



void MC::Asembler::addGlobalSymbols(std::string symbolString){

  std::vector<std::string> symbols = split(symbolString, " ");
  for(std::string symName: symbols){
      if(!isInSymTab(symName)){
        if(symName != ""){
          ST_entry* newEntry = new ST_entry(symName, true, BindingType::GLOBAL, 0);
          addToSymTab(newEntry);  
        } 
      }
      
      
    }


}




void MC::Asembler::addToForwardRefs(ST_entry* symbol, bool reloc){
  ST_forwardRefs* newNode = new ST_forwardRefs();
  newNode->patch = this->locationCounter;
  newNode->nlink = nullptr;
  newNode->section = this->currSection;
  newNode->reloc = reloc;
  if(symbol->flink == nullptr){
    symbol->flink = newNode;
    return;
  }

  ST_forwardRefs* temp = symbol->flink;
  while(temp->nlink != nullptr){
    temp = temp->nlink;
  }

  temp->nlink = newNode;


}


void MC::Asembler::haltInstruction(){
  for(int i = 0 ; i < 4; i++){
    SectionData* newSecData = new SectionData();
    newSecData->section = currSection;
    newSecData->offset = locationCounter;
    newSecData->code = "00";
    newSecData->explanation = "Halt instruction";
    this->code.push_back(newSecData);
    locationCounter++;
  }
}
void MC::Asembler::intInstruction(){
    for(int i = 0 ; i < 3; i++){
      SectionData* newSecData = new SectionData();
      newSecData->section = currSection;
      newSecData->offset = locationCounter;
      newSecData->code = "00";
      newSecData->explanation = "Int instruction";
      this->code.push_back(newSecData);
      locationCounter++;
    }
    SectionData* newSecData = new SectionData();
    newSecData->section = currSection;
    newSecData->offset = locationCounter;
    newSecData->code = "10";
    newSecData->explanation = "Int instruction";
    this->code.push_back(newSecData);
    locationCounter++;
    
}

void MC::Asembler::retInstruction(){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "04";
  fourthByte->explanation = "Fourth Byte of POP PC";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "00";
  thirdByte->explanation = "Third Byte of POP PC";
  locationCounter++;
  this->code.push_back(thirdByte);
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code =  "FE";
  secondByte->explanation = "Second Byte of POP PC";
  locationCounter++;
  this->code.push_back(secondByte);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "93";
  firstByte->explanation = "First Byte of POP";
  locationCounter++;
  this->code.push_back(firstByte);
}

void MC::Asembler::iretInstruction(){

  //pop pc
  SectionData* firstFourthByte = new SectionData();
  firstFourthByte->section = currSection;
  firstFourthByte->offset = locationCounter;
  firstFourthByte->code = "04";
  firstFourthByte->explanation = "Fourth Byte of POP PC";
  locationCounter++;
  this->code.push_back(firstFourthByte);
  SectionData* firstThirdByte = new SectionData();
  firstThirdByte->section = currSection;
  firstThirdByte->offset = locationCounter;
  firstThirdByte->code = "00";
  firstThirdByte->explanation = "Third Byte of POP PC";
  locationCounter++;
  this->code.push_back(firstThirdByte);
  SectionData* firstSecondByte = new SectionData();
  firstSecondByte->section = currSection;
  firstSecondByte->offset = locationCounter;
  firstSecondByte->code =  "FE";
  firstSecondByte->explanation = "Second Byte of POP PC";
  locationCounter++;
  this->code.push_back(firstSecondByte);
  SectionData* firstFirstByte = new SectionData();
  firstFirstByte->section = currSection;
  firstFirstByte->offset = locationCounter;
  firstFirstByte->code = "93";
  firstFirstByte->explanation = "First Byte of POP PC";
  locationCounter++;
  this->code.push_back(firstFirstByte);

  //pop status
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "04";
  fourthByte->explanation = "Fourth Byte of POP STATUS";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "00";
  thirdByte->explanation = "Third Byte of POP STATUS";
  locationCounter++;
  this->code.push_back(thirdByte);
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "0E";
  secondByte->explanation = "Second Byte of POP STATUS";
  locationCounter++;
  this->code.push_back(secondByte);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "97";
  firstByte->explanation = "First Byte of POP STATUS";
  locationCounter++;
  this->code.push_back(firstByte);
}
string MC::Asembler::getRegisterNumber(string reg){
  if(reg == "r0" || reg == "status"){
    return "0";
  }
  else if(reg == "r1" || reg == "handler"){
    return "1";
  }
  else if(reg == "r2" || reg == "cause"){
    return "2";
  }
  else if(reg == "r3"){
    return "3";
  }
  else if(reg == "r4"){
    return "4";
  }
  else if(reg == "r5"){
    return "5";
  }
  else if(reg == "r6"){
    return "6";
  }
  else if(reg == "r7"){
    return "7";
  }
  else if(reg == "r8"){
    return "8";
  }
  else if(reg == "r9"){
    return "9";
  }
  else if(reg == "r10"){
    return "A";
  }
  else if(reg == "r11"){
    return "B";
  }
  else if(reg == "r12"){
    return "C";
  }
  else if(reg == "r13"){
    return "D";
  }
  else if(reg == "r14" || reg == "sp"){
    return "E";
  }
  else {
    return "F";
  }
}

void MC::Asembler::xchgInstruction(list<string> operands){
  string firstReg = getRegisterNumber(operands.front());
  operands.pop_front();
  string secondReg = getRegisterNumber(operands.front());

  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "00";
  fourthByte->explanation = "Fourth Byte of XCHG";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = firstReg + "0";
  thirdByte->explanation = "Third Byte of XCHG";
  locationCounter++;
  this->code.push_back(thirdByte);
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "0" + secondReg;
  secondByte->explanation = "Second Byte of XCHG";
  locationCounter++;
  this->code.push_back(secondByte);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "40";
  firstByte->explanation = "First Byte of XCHG";
  locationCounter++;
  this->code.push_back(firstByte);

}
void MC::Asembler::addInstruction(list<string> operands){
  dataOperandCode(operands);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "50";
  firstByte->explanation = "First Byte of ADD";
  locationCounter++;
  this->code.push_back(firstByte);

}
void MC::Asembler::subInstruction(list<string> operands){
  dataOperandCode(operands);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "51";
  firstByte->explanation = "First Byte of SUB";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::mulInstruction(list<string> operands){
  dataOperandCode(operands);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "52";
  firstByte->explanation = "First Byte of MUL";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::divInstruction(list<string> operands){
  dataOperandCode(operands);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "53";
  firstByte->explanation = "First Byte of DIV";
  locationCounter++;
  this->code.push_back(firstByte);
}

void MC::Asembler::pushInstruction(string gpr){
  string regCode = getRegisterNumber(gpr);
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "FC";
  fourthByte->explanation = "Fourth Byte of PUSH";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = regCode + "F";
  thirdByte->explanation = "Third Byte of PUSH";
  locationCounter++;
  this->code.push_back(thirdByte);
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "E0";
  secondByte->explanation = "Second Byte of PUSH";
  locationCounter++;
  this->code.push_back(secondByte);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "81";
  firstByte->explanation = "First Byte of PUSH";
  locationCounter++;
  this->code.push_back(firstByte);


}

void MC::Asembler::popInstruction(string gpr){
  string regCode = getRegisterNumber(gpr);
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "04";
  fourthByte->explanation = "Fourth Byte of POP";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "00";
  thirdByte->explanation = "Third Byte of POP";
  locationCounter++;
  this->code.push_back(thirdByte);
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = regCode + "E";
  secondByte->explanation = "Second Byte of POP";
  locationCounter++;
  this->code.push_back(secondByte);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "93";
  firstByte->explanation = "First Byte of POP";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::dataOperandCode(list<string> operands){
  string gprS = getRegisterNumber(operands.front());
  operands.pop_front();
  string gprD = getRegisterNumber(operands.front());
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "00";
  fourthByte->explanation = "Fourth Byte";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = gprS + "0";
  thirdByte->explanation = "Third Byte";
  locationCounter++;
  this->code.push_back(thirdByte);
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = gprD + gprD;
  secondByte->explanation = "Second Byte";
  locationCounter++;
  this->code.push_back(secondByte);
}

void MC::Asembler::notInstruction(string gpr){
  string regCode = getRegisterNumber(gpr);
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "00";
  fourthByte->explanation = "Fourth Byte of NOT";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "00";
  thirdByte->explanation = "Third Byte of NOT";
  locationCounter++;
  this->code.push_back(thirdByte);
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = regCode + regCode;
  secondByte->explanation = "Second Byte of NOT";
  locationCounter++;
  this->code.push_back(secondByte);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "60";
  firstByte->explanation = "First Byte of NOT";
  locationCounter++;
  this->code.push_back(firstByte);

}
void MC::Asembler::orInstruction(list<string> operands){
  dataOperandCode(operands);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "62";
  firstByte->explanation = "First Byte of OR";
  locationCounter++;
  this->code.push_back(firstByte);

  
}
void MC::Asembler::xorInstruction(list<string> operands){
  dataOperandCode(operands);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "63";
  firstByte->explanation = "First Byte of OR";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::andInstruction(list<string> operands){
  dataOperandCode(operands);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "61";
  firstByte->explanation = "First Byte of OR";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::shlInstruction(list<string> operands){
  dataOperandCode(operands);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "70";
  firstByte->explanation = "First Byte of SHL";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::shrInstruction(list<string> operands){
  dataOperandCode(operands);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "71";
  firstByte->explanation = "First Byte of SHR";
  locationCounter++;
  this->code.push_back(firstByte);
}

void MC::Asembler::csrrdInstruction(string csr, string gpr){
  string gprCode = getRegisterNumber(gpr);
  string csrCode = getRegisterNumber(csr);
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "00";
  fourthByte->explanation = "Fourth Byte of CSRRD";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "00";
  thirdByte->explanation = "Third Byte of CSRRD";
  locationCounter++;
  this->code.push_back(thirdByte);
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = gprCode + csrCode;
  secondByte->explanation = "Second Byte of CSRRD";
  locationCounter++;
  this->code.push_back(secondByte);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "90";
  firstByte->explanation = "First Byte of CSRRD";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::csrwrInstruction(string gpr, string csr){
  string gprCode = getRegisterNumber(gpr);
  string csrCode = getRegisterNumber(csr);
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "00";
  fourthByte->explanation = "Fourth Byte of CSRWR";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "00";
  thirdByte->explanation = "Third Byte of CSRWR";
  locationCounter++;
  this->code.push_back(thirdByte);
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = csrCode + gprCode;
  secondByte->explanation = "Second Byte of CSRWR";
  locationCounter++;
  this->code.push_back(secondByte);
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "94";
  firstByte->explanation = "First Byte of CSRWR";
  locationCounter++;
  this->code.push_back(firstByte);
}


void MC::Asembler::skipDirective(string literal)
{
  int l = -1;
  if(literal[0] == '0' && (literal[1] == 'x' or literal[1] == 'X')){
    
    l = stoi(literal, 0, 16);

  }
  else{
    l = stoi(literal, 0, 10);
  }
  for(int i = 0; i < l; i++){
      SectionData* newSecData = new SectionData();
      newSecData->section = currSection;
      newSecData->offset = locationCounter;
      newSecData->code = "00";
      newSecData->explanation = ".skip directive";
      this->code.push_back(newSecData);
      locationCounter++;
    }
}
void MC::Asembler::printSecData()
{
  cout << "         SECTION CODE DATA          " << endl;
  cout << "========================================" << endl;
  cout << "|Section|    |Offset|    |Code|    |Explanation|" << endl;
  for(SectionData* s : code){
    cout << *s << endl;
  }
}


void MC::Asembler::printLiteralPool(){
  for(LiteralPool* p : literalPool){
    cout << *p << endl;
  }
}



int MC::Asembler::addToLiteralPool(string hexa, bool reloc){
  int ret = this->literalPool.size();
  for(LiteralPool* i : this->literalPool){
    if(i->literal == hexa && i->section == currSection){
      i->usedOn.push_back(locationCounter);
      return -1;
    }
  }
  LiteralPool* newLiteral = new LiteralPool(hexa, currSection, reloc);
  newLiteral->usedOn.push_back(locationCounter);
  this->literalPool.push_back(newLiteral);

  return ret * 4;

}

void MC::Asembler::addLiteralToCode(string literal){
  while(literal.length() < 8){
      literal.insert(0,1,'0');
  }
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = literal.substr(6,2);
  fourthByte->explanation = "Fourth Byte of Literal";
  locationCounter++;
  this->code.push_back(fourthByte);


  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code =  literal.substr(4,2);
  thirdByte->explanation = "Third Byte of Literal";
  locationCounter++;
  this->code.push_back(thirdByte);

  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = literal.substr(2,2);
  secondByte->explanation = "Second Byte of literal";
  locationCounter++;
  this->code.push_back(secondByte);
            
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
        
  firstByte->code = literal.substr(0,2);
        


  firstByte->explanation = "First Byte of literal";
  locationCounter++;
  this->code.push_back(firstByte);
}

void MC::Asembler::addPoolRelocCode(){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "??";
  fourthByte->explanation = "Fourth Byte of Literal";
  locationCounter++;
  this->code.push_back(fourthByte);


  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code =  "??";
  thirdByte->explanation = "Third Byte of Literal";
  locationCounter++;
  this->code.push_back(thirdByte);

  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "??";
  secondByte->explanation = "Second Byte of literal";
  locationCounter++;
  this->code.push_back(secondByte);
            
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
        
  firstByte->code = "??";
        


  firstByte->explanation = "First Byte of literal";
  locationCounter++;
  this->code.push_back(firstByte);
}


void MC::Asembler::updateSectionPool(){
  //Update section size
  

  //Update LiteralPool
  for(LiteralPool* i : this->literalPool){
    if(i->section == currSection){

      i->offset = locationCounter;
      if(i->reloc){
        this->addPoolRelocCode();
        RelocTable* newReloc = new RelocTable();
        newReloc->section = this->currSection;
        newReloc->offset = i->offset;
        
        ST_entry* symbol = isInSymTab(i->literal);
        if(symbol->bindType == GLOBAL){
          newReloc->symbol = symbol->id;
          newReloc->addend = 0;
        }
        else{
          newReloc->symbol = symbol->section;
          newReloc->addend = symbol->value;
        }

        this->relocTable.push_back(newReloc);

      }
      else{
        this->addLiteralToCode(i->literal);
      }
      for(int j : i->usedOn){
        
        for(auto s = this->code.begin(); s != this->code.end(); ++s){
            if((*s)->offset == j && (*s)->section == i->section){
                int disp = i->offset - (j+4);
                std::stringstream ss;
                ss << std::hex << std::uppercase << disp;
                string sym_code = ss.str();
                while(sym_code.length() < 3){
                  sym_code.insert(0, 1, '0');
                }

                (*s)->code = sym_code.substr(1);
                auto s2 = next(s);
                (*s2)->code = (*s2)->code[0] +  sym_code.substr(0,1); 
            }

          
        }
        
      }
      
    }
  }
  for(ST_entry* st : this->symbolTable){
    if(st->id == this->currSection){
      st->size = this->locationCounter;
    }
  }
}



ST_entry* MC::Asembler::isInSymTab(string symbol){
    
  for(ST_entry* s : this->symbolTable){
    if(s->name.compare(symbol) == 0) return s ;
  }

  return nullptr;
}


void MC::Asembler::addToSymTab(ST_entry* newSym){

  this->symbolTable.insert(newSym);
}

void MC::Asembler::updateSymbol(string symbol){

  for(ST_entry* s : this->symbolTable){
    if(s->name.compare(symbol) == 0){
      s->defined = true;
      s->value = locationCounter;
      s->section = currSection;
      //for(ST_forwardRefs* forw = s->flink; forw != nullptr; forw = forw->nlink){
        /*REGULISATI BACKPATCHING*/  
      //}
    } 
  } 
}

string MC::Asembler::jmpLiteral(string literal){ 
  string a = "";
  if(literal[0] == '0' && (literal[1] == 'X' ||  literal[1] == 'x')){
      
      a.append(literal, 2, literal.length());
      if(a.length() < 2){
        return "0" + a;
      }
      else{
        return a;
      }
  }else{
    if (literal.length() < 2){
      return "0" + literal;
    }
    else{
      int num = std::stoi(literal);
      std::stringstream ss;
      ss << std::hex << std::uppercase << num;
      if(ss.str().length() < 2){
          return "0" + ss.str();
      }
      return ss.str();
    }
  }


  
  

  

}


void MC::Asembler::ldCode(string gpr, string operand){
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of LD";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = "0" + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of LD";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = gpr + "0";
        secondByte->explanation = "Second Byte of LD";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        
        
        firstByte->code = "92";
        

        firstByte->explanation = "First Byte of LD";
        locationCounter++;
        this->code.push_back(firstByte);

}

void MC::Asembler::ldLiteralCode(string gpr){
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = gpr + "F";
  secondByte->explanation = "Second Byte of LD";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  
  firstByte->code = "92";
  
  
  firstByte->explanation = "First Byte of LD";
  locationCounter++;
  this->code.push_back(firstByte);
}

void MC::Asembler::callLiteralCode(){
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F0";
  secondByte->explanation = "Second Byte of CALL";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "20";
  firstByte->explanation = "First Byte of CALL";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::bneLiteralCode(string gpr1){
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F" + gpr1;
  secondByte->explanation = "Second Byte of BNE";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "32";
  firstByte->explanation = "First Byte of BNE";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::bgtLiteralCode(string gpr1){
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F" + gpr1;
  secondByte->explanation = "Second Byte of BGT";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "33";
  firstByte->explanation = "First Byte of BGT";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::beqLiteralCode(string gpr1){
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F" + gpr1;
  secondByte->explanation = "Second Byte of BEQ";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "31";
  firstByte->explanation = "First Byte of BEQ";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::beqCode(string code, string gpr1, string gpr2){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = code.substr(1);
  fourthByte->explanation = "Fourth Byte of BEQ";
  locationCounter++;
  this->code.push_back(fourthByte);


  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = gpr2 + code.substr(0,1);
  thirdByte->explanation = "Third Byte of BEQ";
  locationCounter++;
  this->code.push_back(thirdByte);

  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F" + gpr1;
  secondByte->explanation = "Second Byte of BEQ";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "31";
  firstByte->explanation = "First Byte of BEQ";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::bgtCode(string code, string gpr1, string gpr2){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = code.substr(1);
  fourthByte->explanation = "Fourth Byte of BGT";
  locationCounter++;
  this->code.push_back(fourthByte);


  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = gpr2 + code.substr(0,1);
  thirdByte->explanation = "Third Byte of BGT";
  locationCounter++;
  this->code.push_back(thirdByte);

  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F" + gpr1;
  secondByte->explanation = "Second Byte of BGT";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "33";
  firstByte->explanation = "First Byte of BGT";
  locationCounter++;
  this->code.push_back(firstByte);
}
void MC::Asembler::bneCode(string code, string gpr1, string gpr2){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = code.substr(1);
  fourthByte->explanation = "Fourth Byte of BNE";
  locationCounter++;
  this->code.push_back(fourthByte);


  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = gpr2 + code.substr(0,1);
  thirdByte->explanation = "Third Byte of BNE";
  locationCounter++;
  this->code.push_back(thirdByte);

  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F" + gpr1;
  secondByte->explanation = "Second Byte of BNE";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "32";
  firstByte->explanation = "First Byte of BEQ";
  locationCounter++;
  this->code.push_back(firstByte);
}

void MC::Asembler::callCode(string code){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = code.substr(1);
  fourthByte->explanation = "Fourth Byte of CALL";
  locationCounter++;
  this->code.push_back(fourthByte);


  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "0" + code.substr(0,1);
  thirdByte->explanation = "Third Byte of CALL";
  locationCounter++;
  this->code.push_back(thirdByte);

  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F0";
  secondByte->explanation = "Second Byte of CALL";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "20";
  firstByte->explanation = "First Byte of CALL";
  locationCounter++;
  this->code.push_back(firstByte);
}

void MC::Asembler::callInstruction(string operand){
  switch (this->symbolOrliteral)
  {
    case 1: {
      /* SYMBOL */
      ST_entry* symbol = isInSymTab(operand);
        if(symbol){
          // U TABELI SIMBOLA
          if(symbol->defined){
            //DEFINISAN SIMBOL
            if(symbol->section == 0){
              //GLOBAL ILI EXTERN
              this->addToLiteralPool(operand, true);

              this->relocCode("CALL");

              this->callLiteralCode();
            }
            else{
              //LOKAALAN
              if(symbol->section == this->currSection){
                //U TRENUTNOJ SEKCIJI
                int displacement = symbol->value - (locationCounter + 4);
                std::stringstream ss;
                ss << std::hex << std::uppercase << displacement;
                string sym_code = ss.str();
                
                if(displacement >= -2048 && displacement <= 2047)
                {
                  // STAJE U 12 bita

                  if(displacement < 0){
                    sym_code = sym_code.substr(sym_code.length() - 3);

                  }
                  else{
                    while(sym_code.length() < 3){
                      sym_code.insert(0,1, '0');
                    }
                  }

                  this->callCode(sym_code);    

                }
                else{
                  // Ne staje u 12 bita
                  this->addToLiteralPool(operand, true);
                  this->relocCode("CALL");

                  this->callLiteralCode();
                  
                }
                
                
              }
              else{
                // NIJE U TRENUTNOJ SEKCIJI
                this->addToLiteralPool(operand, true);

                this->relocCode("CALL");

                this->callLiteralCode();
              }
            }
          }
          else{
            // SIMBOL NIJE DEFINISAN
            this->addToForwardRefs(symbol,false);
            this->relocCode("CALL");
            

            SectionData* secondByte = new SectionData();
            secondByte->section = currSection;
            secondByte->offset = locationCounter;
            secondByte->code = "F0";
            secondByte->explanation = "Second Byte of CALL";
            locationCounter++;
            this->code.push_back(secondByte);
        
            SectionData* firstByte = new SectionData();
            firstByte->section = currSection;
            firstByte->offset = locationCounter;
            firstByte->code = "30";
            firstByte->explanation = "First Byte of CALL";
            locationCounter++;
            this->code.push_back(firstByte);
          }
        }
        else{
          // SIMBOL NIJE U TABELI SIMBOLA
          ST_entry* newSym = new ST_entry(operand);
          this->addToForwardRefs(newSym, false);
          this->addToSymTab(newSym);
          this->relocCode("CALL");
          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = "F0";
          secondByte->explanation = "Second Byte of CALL";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "20";
          firstByte->explanation = "First Byte of CALL";
          locationCounter++;
          this->code.push_back(firstByte);
        }
      
      break;
    }
    case -1: {
      /* LITERAL */
      //operand je literal

      if(operand.length() > 3){
       //LITERAL POOL
       this->addToLiteralPool(operand, false); 
       this->relocCode("CALL");
       this->callLiteralCode();

      }
      else{
        // Ne mora LITERAL POOL
        while(operand.length() < 3){
          operand.insert(0,1, '0');
        }
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of CALL";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = "0" + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of CALL";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "00";
        secondByte->explanation = "Second Byte of CALL";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "20";
        firstByte->explanation = "First Byte of CALL";
        locationCounter++;
        this->code.push_back(firstByte);

      
      }
      break;
    }
    default:
      break;
  
  }


}

void::MC::Asembler::jmpCode(string code){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = code.substr(1);
  fourthByte->explanation = "Fourth Byte of JMP";
  locationCounter++;
  this->code.push_back(fourthByte);


  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "0" + code.substr(0,1);
  thirdByte->explanation = "Third Byte of JMP";
  locationCounter++;
  this->code.push_back(thirdByte);

  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F0";
  secondByte->explanation = "Second Byte of JMP";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "30";
  firstByte->explanation = "First Byte of JMP";
  locationCounter++;
  this->code.push_back(firstByte);
}

void MC::Asembler::jmpLiteralCode(){
  SectionData* secondByte = new SectionData();
  secondByte->section = currSection;
  secondByte->offset = locationCounter;
  secondByte->code = "F0";
  secondByte->explanation = "Second Byte of JMP";
  locationCounter++;
  this->code.push_back(secondByte);
      
  SectionData* firstByte = new SectionData();
  firstByte->section = currSection;
  firstByte->offset = locationCounter;
  firstByte->code = "38";
  firstByte->explanation = "First Byte of JMP";
  locationCounter++;
  this->code.push_back(firstByte);


}

void MC::Asembler::jmpInstruction(string operand){
  switch (this->symbolOrliteral){
    case 1: {
      //SYMBOL
      ST_entry* symbol = isInSymTab(operand);
      if(symbol){
        // U TABELI SIMBOLA
        if(symbol->defined){
          //DEFINISAN SIMBOL
          if(symbol->section == 0){
            //SYMBOL JE GLOBAL ILI EXTERN
            this->addToLiteralPool(operand, true);

            this->relocCode("JMP");

            this->jmpLiteralCode();
            

          }else{
            //LOKALAN SIMBOL
            if(symbol->section == this->currSection){
              //U TRENUTNOJ SEKCIJI
              int displacement = symbol->value - (locationCounter + 4);
              std::stringstream ss;
              ss << std::hex << std::uppercase << displacement;
              string sym_code = ss.str();
              
              if(displacement >= -2048 && displacement <= 2047)
              {
                // STAJE U 12 bita

                if(displacement < 0){
                  sym_code = sym_code.substr(sym_code.length() - 3);

                }
                else{
                  while(sym_code.length() < 3){
                    sym_code.insert(0,1, '0');
                  }
                }

                this->jmpCode(sym_code);    

              }
              else{
                // Ne staje u 12 bita
                this->addToLiteralPool(operand, true);
                this->relocCode("JMP");

                this->jmpLiteralCode();
                
              }

              
            }else{
              //DEFINISAN U NEKOJ DRUGOJ SEKCIJI, TREBA RELOKACIONI ZAPIS
              this->addToLiteralPool(operand, true);

              this->relocCode("JMP");

              this->jmpLiteralCode();



            }
          }

        }else{
          //SIMBOL NIJE DEFINISAN
          this->addToForwardRefs(symbol, false);
          this->relocCode("JMP");

          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = "F0";
          secondByte->explanation = "Second Byte of JMP";
          locationCounter++;
          this->code.push_back(secondByte);
      
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "30";
          firstByte->explanation = "First Byte of JMP";
          locationCounter++;
          this->code.push_back(firstByte);


        }
      }
      else{
        // NIJE U TABELI SIMBOLA
        ST_entry* newSym = new ST_entry(operand);
        this->addToForwardRefs(newSym, false);
        this->addToSymTab(newSym);
        this->relocCode("JMP");
        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "F0";
        secondByte->explanation = "Second Byte of JMP";
        locationCounter++;
        this->code.push_back(secondByte);
      
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "30";
        firstByte->explanation = "First Byte of JMP";
        locationCounter++;
        this->code.push_back(firstByte);


      }



      break;
    }
    case -1:{
      //Literal
      if(operand.length() > 3){
       //LITERAL POOL
       this->addToLiteralPool(operand, false); 
       this->relocCode("JMP");
       this->jmpLiteralCode();

      }
      else{
        // Ne mora LITERAL POOL
        while(operand.length() < 3){
          operand.insert(0,1, '0');
        }
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of JMP";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = "0" + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of JMP";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "00";
        secondByte->explanation = "Second Byte of JMP";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "30";
        firstByte->explanation = "First Byte of JMP";
        locationCounter++;
        this->code.push_back(firstByte);

      }

      



      break;
    }

  }


}

void MC::Asembler::condJmpRelocCode(string instruction, string gpr2){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "??";
  fourthByte->explanation = "Fourth Byte of " + instruction;
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = gpr2 + "?";
  thirdByte->explanation = "Third Byte of " + instruction;
  locationCounter++;
  this->code.push_back(thirdByte);
}

void MC::Asembler::relocCode(string instruction){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "??";
  fourthByte->explanation = "Fourth Byte of " + instruction;
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = "0?";
  thirdByte->explanation = "Third Byte of " + instruction;
  locationCounter++;
  this->code.push_back(thirdByte);
}
void MC::Asembler::stRelocCode(string gpr){
  SectionData* fourthByte = new SectionData();
  fourthByte->section = currSection;
  fourthByte->offset = locationCounter;
  fourthByte->code = "??";
  fourthByte->explanation = "Fourth Byte of ST";
  locationCounter++;
  this->code.push_back(fourthByte);
  SectionData* thirdByte = new SectionData();
  thirdByte->section = currSection;
  thirdByte->offset = locationCounter;
  thirdByte->code = gpr + "?";
  thirdByte->explanation = "Third Byte of ST";
  locationCounter++;
  this->code.push_back(thirdByte);
}

void MC::Asembler::printRelocData(){
  cout << "         RELOC DATA          " << endl;
  cout << "========================================" << endl;
  cout << "|Section|    |Offset|    |Symbol|    |Addend|    |Type|" << endl;
  for(RelocTable* r : this->relocTable){
    cout << *r << endl;
  }
}

void MC::Asembler::beqInstruction(list<string> operands){
  string gpr1 = getRegisterNumber(operands.front());
  operands.pop_front();
  string gpr2 = getRegisterNumber(operands.front());
  operands.pop_front();
  string operand = operands.front();
  operands.pop_front();
  switch(this->symbolOrliteral){
    case 1: {

      // SIMBOL
      ST_entry* symbol = isInSymTab(operand);
        if(symbol){
          // U TABELI SIMBOLA
          if(symbol->defined){
            //DEFINISAN SIMBOL
            if(symbol->section == 0){
              //GLOBAL ILI EXTERN
              this->addToLiteralPool(operand, true);

              this->condJmpRelocCode("BEQ", gpr2);

              this->beqLiteralCode(gpr1);
            }
            else{
              //LOKAALAN
              if(symbol->section == this->currSection){
                //U TRENUTNOJ SEKCIJI
                int displacement = symbol->value - (locationCounter + 4);
                std::stringstream ss;
                ss << std::hex << std::uppercase << displacement;
                string sym_code = ss.str();
                
                if(displacement >= -2048 && displacement <= 2047)
                {
                  // STAJE U 12 bita

                  if(displacement < 0){
                    sym_code = sym_code.substr(sym_code.length() - 3);

                  }
                  else{
                    while(sym_code.length() < 3){
                      sym_code.insert(0,1, '0');
                    }
                  }

                  this->beqCode(sym_code, gpr1, gpr2);    

                }
                else{
                  // Ne staje u 12 bita
                  this->addToLiteralPool(operand, true);
                  this->condJmpRelocCode("BEQ", gpr2);

                  this->beqLiteralCode(gpr1);
                  
                }
                
                
              }
              else{
                // NIJE U TRENUTNOJ SEKCIJI
                this->addToLiteralPool(operand, true);

                this->condJmpRelocCode("BEQ", gpr2);

                this->beqLiteralCode(gpr1);
              }
            }
          }
          else{
            // SIMBOL NIJE DEFINISAN
            this->addToForwardRefs(symbol, false);
            this->condJmpRelocCode("BEQ", gpr2);
            

            SectionData* secondByte = new SectionData();
            secondByte->section = currSection;
            secondByte->offset = locationCounter;
            secondByte->code = "F" + gpr1;
            secondByte->explanation = "Second Byte of BEQ";
            locationCounter++;
            this->code.push_back(secondByte);
        
            SectionData* firstByte = new SectionData();
            firstByte->section = currSection;
            firstByte->offset = locationCounter;
            firstByte->code = "31";
            firstByte->explanation = "First Byte of BEQ";
            locationCounter++;
            this->code.push_back(firstByte);
          }
        }
        else{
          // SIMBOL NIJE U TABELI SIMBOLA
          ST_entry* newSym = new ST_entry(operand);
          this->addToForwardRefs(newSym, false);
          this->addToSymTab(newSym);
          this->condJmpRelocCode("BEQ", gpr2);
          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = "F" + gpr1;
          secondByte->explanation = "Second Byte of BEQ";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "31";
          firstByte->explanation = "First Byte of BEQ";
          locationCounter++;
          this->code.push_back(firstByte);
        }

      break;
    }
    case -1:{
      // LITERAL

      if(operand.length() > 3){
       //LITERAL POOL
       this->addToLiteralPool(operand, false); 
       this->condJmpRelocCode("BEQ", gpr2);
       this->beqLiteralCode(gpr1);

      }
      else{
        // Ne mora LITERAL POOL
        while(operand.length() < 3){
          operand.insert(0,1, '0');
        }
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of BEQ";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = gpr2 + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of BEQ";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "0" + gpr1;
        secondByte->explanation = "Second Byte of BEQ";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "31";
        firstByte->explanation = "First Byte of BEQ";
        locationCounter++;
        this->code.push_back(firstByte);

      
      }
      break;
    }
  }
  

}


void MC::Asembler::bneInstruction(list<string> operands){
  string gpr1 = getRegisterNumber(operands.front());
  operands.pop_front();
  string gpr2 = getRegisterNumber(operands.front());
  operands.pop_front();
  string operand = operands.front();
  operands.pop_front();
  switch(this->symbolOrliteral){
    case 1: {

      // SIMBOL
      ST_entry* symbol = isInSymTab(operand);
        if(symbol){
          // U TABELI SIMBOLA
          if(symbol->defined){
            //DEFINISAN SIMBOL
            if(symbol->section == 0){
              //GLOBAL ILI EXTERN
              this->addToLiteralPool(operand, true);

              this->condJmpRelocCode("BNE", gpr2);

              this->bneLiteralCode(gpr1);
            }
            else{
              //LOKAALAN
              if(symbol->section == this->currSection){
                //U TRENUTNOJ SEKCIJI
                int displacement = symbol->value - (locationCounter + 4);
                std::stringstream ss;
                ss << std::hex << std::uppercase << displacement;
                string sym_code = ss.str();
                
                if(displacement >= -2048 && displacement <= 2047)
                {
                  // STAJE U 12 bita

                  if(displacement < 0){
                    sym_code = sym_code.substr(sym_code.length() - 3);

                  }
                  else{
                    while(sym_code.length() < 3){
                      sym_code.insert(0,1, '0');
                    }
                  }

                  this->bneCode(sym_code, gpr1, gpr2);    

                }
                else{
                  // Ne staje u 12 bita
                  this->addToLiteralPool(operand, true);
                  this->condJmpRelocCode("BNE", gpr2);

                  this->bneLiteralCode(gpr1);
                  
                }
                
                
              }
              else{
                // NIJE U TRENUTNOJ SEKCIJI
                this->addToLiteralPool(operand, true);

                this->condJmpRelocCode("BNE", gpr2);

                this->bneLiteralCode(gpr1);
              }
            }
          }
          else{
            // SIMBOL NIJE DEFINISAN
            this->addToForwardRefs(symbol, false);
            this->condJmpRelocCode("BNE", gpr2);
            

            SectionData* secondByte = new SectionData();
            secondByte->section = currSection;
            secondByte->offset = locationCounter;
            secondByte->code = "F" + gpr1;
            secondByte->explanation = "Second Byte of BNE";
            locationCounter++;
            this->code.push_back(secondByte);
        
            SectionData* firstByte = new SectionData();
            firstByte->section = currSection;
            firstByte->offset = locationCounter;
            firstByte->code = "32";
            firstByte->explanation = "First Byte of BEQ";
            locationCounter++;
            this->code.push_back(firstByte);
          }
        }
        else{
          // SIMBOL NIJE U TABELI SIMBOLA
          ST_entry* newSym = new ST_entry(operand);
          this->addToForwardRefs(newSym, false);
          this->addToSymTab(newSym);
          this->condJmpRelocCode("BNE", gpr2);
          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = "F" + gpr1;
          secondByte->explanation = "Second Byte of BNE";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "32";
          firstByte->explanation = "First Byte of BNE";
          locationCounter++;
          this->code.push_back(firstByte);
        }

      break;
    }
    case -1:{
      // LITERAL

      if(operand.length() > 3){
       //LITERAL POOL
       this->addToLiteralPool(operand, false); 
       this->condJmpRelocCode("BNE", gpr2);
       this->bneLiteralCode(gpr1);

      }
      else{
        // Ne mora LITERAL POOL
        while(operand.length() < 3){
          operand.insert(0,1, '0');
        }
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of BNE";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = gpr2 + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of BNE";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "0" + gpr1;
        secondByte->explanation = "Second Byte of BNE";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "32";
        firstByte->explanation = "First Byte of BNE";
        locationCounter++;
        this->code.push_back(firstByte);

      
      }
      break;
    }
  }
   
}


void MC::Asembler::bgtInstruction(list<string> operands){
  string gpr1 = getRegisterNumber(operands.front());
  operands.pop_front();
  string gpr2 = getRegisterNumber(operands.front());
  operands.pop_front();
  string operand = operands.front();
  operands.pop_front();
  switch(this->symbolOrliteral){
    case 1: {

      // SIMBOL
      ST_entry* symbol = isInSymTab(operand);
        if(symbol){
          // U TABELI SIMBOLA
          if(symbol->defined){
            //DEFINISAN SIMBOL
            if(symbol->section == 0){
              //GLOBAL ILI EXTERN
              this->addToLiteralPool(operand, true);

              this->condJmpRelocCode("BGT", gpr2);

              this->bgtLiteralCode(gpr1);
            }
            else{
              //LOKAALAN
              if(symbol->section == this->currSection){
                //U TRENUTNOJ SEKCIJI
                int displacement = symbol->value - (locationCounter + 4);
                std::stringstream ss;
                ss << std::hex << std::uppercase << displacement;
                string sym_code = ss.str();
                
                if(displacement >= -2048 && displacement <= 2047)
                {
                  // STAJE U 12 bita

                  if(displacement < 0){
                    sym_code = sym_code.substr(sym_code.length() - 3);

                  }
                  else{
                    while(sym_code.length() < 3){
                      sym_code.insert(0,1, '0');
                    }
                  }

                  this->bgtCode(sym_code, gpr1, gpr2);    

                }
                else{
                  // Ne staje u 12 bita
                  this->addToLiteralPool(operand, true);
                  this->condJmpRelocCode("BGT", gpr2);

                  this->bgtLiteralCode(gpr1);
                  
                }
                
                
              }
              else{
                // NIJE U TRENUTNOJ SEKCIJI
                this->addToLiteralPool(operand, true);

                this->condJmpRelocCode("BGT", gpr2);

                this->bgtLiteralCode(gpr1);
              }
            }
          }
          else{
            // SIMBOL NIJE DEFINISAN
            this->addToForwardRefs(symbol, false);
            this->condJmpRelocCode("BGT", gpr2);
            

            SectionData* secondByte = new SectionData();
            secondByte->section = currSection;
            secondByte->offset = locationCounter;
            secondByte->code = "F" + gpr1;
            secondByte->explanation = "Second Byte of BGT";
            locationCounter++;
            this->code.push_back(secondByte);
        
            SectionData* firstByte = new SectionData();
            firstByte->section = currSection;
            firstByte->offset = locationCounter;
            firstByte->code = "33";
            firstByte->explanation = "First Byte of BGT";
            locationCounter++;
            this->code.push_back(firstByte);
          }
        }
        else{
          // SIMBOL NIJE U TABELI SIMBOLA
          ST_entry* newSym = new ST_entry(operand);
          this->addToForwardRefs(newSym, false);
          this->addToSymTab(newSym);
          this->condJmpRelocCode("BGT", gpr2);
          SectionData* secondByte = new SectionData();
          secondByte->section = currSection;
          secondByte->offset = locationCounter;
          secondByte->code = "F" + gpr1;
          secondByte->explanation = "Second Byte of BGT";
          locationCounter++;
          this->code.push_back(secondByte);
        
          SectionData* firstByte = new SectionData();
          firstByte->section = currSection;
          firstByte->offset = locationCounter;
          firstByte->code = "33";
          firstByte->explanation = "First Byte of BGT";
          locationCounter++;
          this->code.push_back(firstByte);
        }

      break;
    }
    case -1:{
      // LITERAL

      if(operand.length() > 3){
       //LITERAL POOL
       this->addToLiteralPool(operand, false); 
       this->condJmpRelocCode("BGT", gpr2);
       this->bgtLiteralCode(gpr1);

      }
      else{
        // Ne mora LITERAL POOL
        while(operand.length() < 3){
          operand.insert(0,1, '0');
        }
        SectionData* fourthByte = new SectionData();
        fourthByte->section = currSection;
        fourthByte->offset = locationCounter;
        fourthByte->code = operand.substr(1);
        fourthByte->explanation = "Fourth Byte of BGT";
        locationCounter++;
        this->code.push_back(fourthByte);


        SectionData* thirdByte = new SectionData();
        thirdByte->section = currSection;
        thirdByte->offset = locationCounter;
        thirdByte->code = gpr2 + operand.substr(0,1);
        thirdByte->explanation = "Third Byte of BGT";
        locationCounter++;
        this->code.push_back(thirdByte);

        SectionData* secondByte = new SectionData();
        secondByte->section = currSection;
        secondByte->offset = locationCounter;
        secondByte->code = "0" + gpr1;
        secondByte->explanation = "Second Byte of BGT";
        locationCounter++;
        this->code.push_back(secondByte);
            
        SectionData* firstByte = new SectionData();
        firstByte->section = currSection;
        firstByte->offset = locationCounter;
        firstByte->code = "33";
        firstByte->explanation = "First Byte of BGT";
        locationCounter++;
        this->code.push_back(firstByte);

      
      }
      break;
    }
  }
   
}

void MC::Asembler::backpatching(){
  for(ST_entry* s : this->symbolTable){

    ST_forwardRefs* temp = s->flink;
    while(temp){
      for(auto c = this->code.begin(); c != this->code.end(); ++c){
        if((*c)->offset == temp->patch && (*c)->section == temp->section){
          if(s->defined == false){
            cout << "Greska pri asembliranju, simbol nije definisan, a koriscen je" << endl;
            exit(-1);
          }
          else{
            if(s->bindType == GLOBAL){
              //GLOBAL ZA REGSYMIND
              if(s->section == 0){
                cout << "GRESKA pri asembliranju, simbol kod registarskog indirektnog sa pomerajem nije poznat u vreme asembliranja" << endl;
                exit(-1);
              }
              else{
                int disp = s->value - (temp->patch + 4);
                if(disp >= -2048 && disp <= 2047){
                  //STAJE
                  std::stringstream ss;
                  ss << std::hex << std::uppercase << disp;
                  string sym_code = ss.str();
                  if(disp < 0){
                  sym_code = sym_code.substr(sym_code.length() - 3);

                  }
                  else{
                    while(sym_code.length() < 3){
                      sym_code.insert(0,1, '0');
                    }
                  }

                  (*c)->code = sym_code.substr(1);
                  auto c2 = next(c);
                  (*c2)->code = (*c2)->code[0] +  sym_code.substr(0,1); 






                }else{
                  cout << "GRESKA pri asembliranju, pomeraj kod registarskog indirektnog sa pomerajem je veci od 12 bita" << endl;
                  exit(-1);
                }
              }
            }
            else{
              //LOKALAN SYMBOL
              if(temp->reloc){
                RelocTable* newReloc = new RelocTable();
                newReloc->offset = temp->patch;
                newReloc->section = temp->section;
                newReloc->addend = s->value;
                newReloc->symbol = s->section;
                newReloc->type = "SYMBOL";
                this->relocTable.push_back(newReloc);


              }
              else{
                if(s->section == temp->section){
                  // SIMBOL IZ ISTE SEKCIJE
                  int displacement = s->value - (temp->patch + 4);
                  std::stringstream ss;
                  ss << std::hex << std::uppercase << displacement;
                  string sym_code = ss.str();
                    
                  if(displacement >= -2048 && displacement <= 2047)
                  {
                      // STAJE U 12 bita

                    if(displacement < 0){
                      sym_code = sym_code.substr(sym_code.length() - 3);

                    }
                    else{
                      while(sym_code.length() < 3){
                        sym_code.insert(0,1, '0');
                      }
                    }
                    (*c)->code = sym_code.substr(1);
                    auto c2 = next(c);
                    (*c2)->code = (*c2)->code[0] +  sym_code.substr(0,1); 
                  

                    }
                    else{
                      // Ne staje u 12 bita
                      LiteralPool* newLit = new LiteralPool(s->name, s->section, false);
                      
                      std::list<LiteralPool*>::iterator it;
                      for(it = this->literalPool.begin(); it != this->literalPool.end(); ++it){
                        if((*it)->section == s->section){
                          break;
                        }
                      }
                      if(it == this->literalPool.end()){
                        newLit->offset = this->getSectionSize(s->section);
                      }
                      else{
                        while((*next(it))->section == s->section){
                          ++it;
                        }
                        newLit->offset = (*it)->offset + 4; 
                      }

                      if(it != this->literalPool.end()){
                        ++it;
                      }

                      this->literalPool.insert(it, newLit);

                      RelocTable* newReloc = new RelocTable();
                      newReloc->offset = newLit->offset;
                      newReloc->section = s->section;
                      newReloc->symbol = s->section;
                      newReloc->addend = s->value;

                      this->relocTable.push_back(newReloc);

                      int displacement = newLit->offset - (temp->patch + 4);
                      std::stringstream ss;
                      ss << std::hex << std::uppercase << displacement;
                      string sym_code = ss.str();
                      if(displacement < 0){
                        sym_code = sym_code.substr(sym_code.length() - 3);

                      }
                      else{
                        while(sym_code.length() < 3){
                          sym_code.insert(0,1, '0');
                        }
                      }
                      (*c)->code = sym_code.substr(1);
                      auto c2 = next(c);
                      (*c2)->code = (*c2)->code[0] +  sym_code.substr(0,1); 
                      
                    }
                  
                }
                else{
                  // SIMBOL IZ DRUGE SEKCIJE ISTOG FAJLA
                      LiteralPool* newLit = new LiteralPool(s->name, temp->section, false);
                      
                      std::list<LiteralPool*>::iterator it;
                      for(it = this->literalPool.begin(); it != this->literalPool.end(); ++it){
                        if((*it)->section == temp->section){
                          break;
                        }
                      }
                      if(it == this->literalPool.end()){
                        newLit->offset = this->getSectionSize(temp->section);
                      }
                      else{
                        while((*next(it))->section == temp->section){
                          ++it;
                        }
                        newLit->offset = (*it)->offset + 4; 
                      }

                      if(it != this->literalPool.end()){
                        ++it;
                      }

                      this->literalPool.insert(it, newLit);
                      RelocTable* newReloc = new RelocTable();
                      newReloc->offset = newLit->offset;
                      newReloc->section = temp->section;
                      newReloc->symbol = s->section;
                      newReloc->addend = s->value;

                      this->relocTable.push_back(newReloc);

                      int displacement = newLit->offset - (temp->patch + 4);
                      std::stringstream ss;
                      ss << std::hex << std::uppercase << displacement;
                      string sym_code = ss.str();
                      if(displacement < 0){
                        sym_code = sym_code.substr(sym_code.length() - 3);

                      }
                      else{
                        while(sym_code.length() < 3){
                          sym_code.insert(0,1, '0');
                        }
                      }
                      (*c)->code = sym_code.substr(1);
                      auto c2 = next(c);
                      (*c2)->code = (*c2)->code[0] +  sym_code.substr(0,1); 
                }

                      




              }

            }
          }
        }
      }


      temp = temp->nlink;
    }


  }
}
int MC::Asembler::getSectionSize(int section){
  for(ST_entry* s : this->symbolTable){
    if(s->id == section){
      return s->size;

    }
  
    
  }
  return -1;
}

void MC::Asembler::printToFile(string filename){
  std::ofstream outFile;
  outFile.open(filename);
  list<Sections> sections;
  
  if (!outFile) {
      std::cerr << "Error opening file!" << std::endl;
      return;
  }


  const int idWidth = 5;
  const int nameWidth = 10;
  const int valueWidth = 8;
  const int bindTypeWidth = 10;
  const int typeWidth = 6;
  const int definedWidth = 9;
  const int sectionWidth = 10;
  const int sizeWidth = 6;
  
  //TABELA SIMBOLA
  outFile << "|   SYMBOL TABLE    |" << endl;
  outFile << std::left
          << std::setw(idWidth) << "ID" 
          << std::setw(nameWidth) << "Name" 
          << std::setw(valueWidth) << "Value" 
          << std::setw(bindTypeWidth) << "BindType" 
          << std::setw(typeWidth) << "Type" 
          << std::setw(definedWidth) << "Defined" 
          << std::setw(sectionWidth) << "Section" 
          << std::setw(sizeWidth) << "Size"  
          << std::endl;

  outFile << std::string(idWidth + nameWidth + valueWidth + bindTypeWidth + typeWidth + definedWidth + sectionWidth + sizeWidth, '=') << std::endl;
  for(ST_entry* s : this->symbolTable){
      if(s->type == SECTION && s->name != "UND"){
        Sections newSec = {s->name, s->id, s->size};
        sections.push_back(newSec);

      }
      outFile   << std::left
                << std::setw(idWidth) << s->id 
                << std::setw(nameWidth) << s->name 
                << std::setw(valueWidth) << s->value 
                << std::setw(bindTypeWidth) << s->bindType 
                << std::setw(typeWidth) << s->type 
                << std::setw(definedWidth) << s->defined 
                << std::setw(sectionWidth) << s->section 
                << std::setw(sizeWidth) << s->size  << endl;
                
  }

  for(Sections s : sections){
    //RELOC DATA
    int i = 0;
   
    const int offsetWidth = 14;
    
    const int symbolWidth = 9;
    const int addendWidth = 14;
    
    for(RelocTable* r : this->relocTable){
      if(r->section == s.id){
        if(i == 0){
          outFile << endl;
          outFile << "rela." << s.name << endl;
          outFile << std::left
            << std::setw(offsetWidth) << "Offset" 
            << std::setw(symbolWidth) << "Symbol" 
            << std::setw(addendWidth) << "Addend" << std::endl;
          outFile << std::string(offsetWidth + symbolWidth + addendWidth, '=') << std::endl;
          i = 1;
        }
        outFile << std::left
                << std::setw(offsetWidth) << r->offset 
                << std::setw(symbolWidth) << r->symbol 
                << std::setw(addendWidth) << r->addend 
                << endl;


      }
    }
    outFile << endl;
    outFile << "code." + s.name << endl;
    outFile << "======================" << endl;
    int j = 0;
    //SECTION DATA
    for(SectionData* c : this->code){
      if(c->section == s.id){
        if( j == 4){
          outFile << endl;
          j = 0;
        }
        outFile << c->code << " ";
        j++;
      }
    }
    outFile << endl;
    int k = 0;
    const int literalWidth = 15;
    for(LiteralPool* c : this->literalPool){
      if(c->section == s.id){
        if( k == 0){
          outFile << endl;
          outFile << "pool." << s.name << endl;
          outFile << std::left
                  << std::setw(offsetWidth) << "Offset" 
                  << std::setw(literalWidth) << "Literal" 
                  << std::endl;
          outFile << std::string(offsetWidth + literalWidth, '=') << std::endl;
          k = 1;
        }
        outFile << std::left
                << std::setw(offsetWidth) << c->offset 
                << std::setw(symbolWidth) << c->literal << endl;
      }
    }
    outFile << endl;

  }

  outFile.close();

  
}

void MC::Asembler::printBinToFile(string filename){
  cout << "USO" << endl;
  std::ofstream outFile;
  outFile.open(filename);
  list<Sections> sections;
  if (!outFile) {
    std::cerr << "Error opening file!" << std::endl;
    return;
  }
  outFile << "symbol_table" << endl;
  for(ST_entry* s : this->symbolTable){
      if(s->type == SECTION && s->name != "UND"){
        Sections newSec = {s->name, s->id, s->size};
        sections.push_back(newSec);

      }

      outFile << s->id << ":"
              << s->name << ":"
              << s->value << ":"
              << s->bindType << ":"
              << s->type << ":"
              << s->defined << ":"
              << s->section << ":"
              << s->size  << endl;

  }
  int i = 0;
  for(Sections s : sections){
    
    for(RelocTable* r: this->relocTable){

      if(r->section == s.id){
        if(i == 0){
          outFile << "rela." + s.name << endl;
          i++;
        }
        outFile << r->offset << ":"
                << r->symbol << ":"
                << r->addend 
                << endl;
      }
    }
    outFile << "code." + s.name << endl;
    int j = 0;
    for(SectionData* c: this->code){
      if(c->section == s.id){
        if(j == 4){
          outFile << endl;
          j = 0;
        }
        outFile << c->code << " ";
        j++;
      }

    }
    outFile << endl;
    
  }



  outFile.close();
}


int main(int argc, char* argv[]){ 
   
   MC::Asembler asembler;
   string outputFile;

   if(argc < 2){
    cout << "GRESKA" << endl;
    exit(-1);
   }

   if(strncmp(argv[1], "-o", 2) == 0){
    
    outputFile = argv[2];
    asembler.parse(argv[3]);
   }
   else{
    outputFile = "izlaz.o";
    asembler.parse(argv[1]);
   } 
   string outputTxtFile = outputFile.substr(0, outputFile.length()-2) + ".txt";
   
   
   

   asembler.printToFile(outputTxtFile);
   asembler.printBinToFile(outputFile); 
   return 0;
}