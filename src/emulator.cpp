
#include "../inc/emulator.hpp"


uint32_t Emulator::MEMORY_MAPPED_REGISTERS = 0xFFFFFF00;

Emulator::Emulator(std::string inputFile): inputFile(inputFile) {
  for(int i =0; i < 16; i++){
    registers[i] = 0;
  }

  pc = 0x40000000;
  


}

void Emulator::printRegisters(){
  for(int i = 0; i < 16; i++){
    std::cout << registers[i] << std::endl;

  }

  
  
}

void Emulator::initializeMemory(){
  std::ifstream input(inputFile);

  if(!input.is_open()){
    std::cerr << "Could not open the file!" << std::endl;
    exit(-1);
  }

  std::string line;
  while(std::getline(input, line)){
  
    std::vector<std::string> oneLine = split(line, ":");
    std::string data = oneLine.back();
    oneLine.pop_back();
    std::string address = oneLine.back();

    uint32_t addr = std::stoul(address, nullptr, 16);

    std::vector<std::string> bytes = split(data, " ");

    for(int i = 0; i < 4 ; i++){
      memory[addr + i] = bytes[i];
    }
    
  }

  

  

  input.close();

}

void Emulator::printing(){
  std::cout << "---------------------------------------------" << std::endl;
  std::cout << "Emulated processor executed halt instruction" << std::endl;
  std::cout << "Emulated processor state:" << std::endl;
  for (int i = 0; i < 16; i++) {
    std::cout << "r" << std::to_string(i) << "=" << "0x";
    std::cout << std::hex << std::setw(8) << std::setfill('0') << registers[i] << "\t";
    if ((i + 1) % 4 == 0) {
      std::cout << std::endl;
    }
  }

}

std::vector<std::string> Emulator::split(std::string s, std::string delimiter){
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


void Emulator::printMemory(){
  for(const auto& pair : memory){
    std::cout << pair.first << ":" << pair.second << std::endl;
  }
}

void Emulator::readInstruction(){
  std::cout << pc << std::endl;
  std::string fourthByte = memory[pc++];
  std::string thirdByte = memory[pc++];
  std::string secondByte = memory[pc++];
  std::string firstByte = memory[pc++];
  
  std::cout << firstByte + secondByte + thirdByte + fourthByte << std::endl;

  oc = std::stoi(firstByte.substr(0,1), nullptr, 16);
  mod = std::stoi(firstByte.substr(1), nullptr, 16);
  regA = std::stoi(secondByte.substr(0,1), nullptr, 16);
  regB = std::stoi(secondByte.substr(1), nullptr, 16);
  regC = std::stoi(thirdByte.substr(0,1), nullptr, 16);
  std::string displacement = thirdByte.substr(1) + fourthByte;
  if(displacement.substr(0,1) == "F" || displacement.substr(0,1) == "E" || displacement.substr(0,1) == "D" || displacement.substr(0,1) == "C" 
  || displacement.substr(0,1) == "B" || displacement.substr(0,1) == "A" || displacement.substr(0,1) == "9" || displacement.substr(0,1) == "8" ){
    while(displacement.size() < 8){
      displacement.insert(0,1, 'F');
    }
  }
  else{
    while(displacement.size() < 8){
      displacement.insert(0,1, '0');
    }
  }

  disp = std::stoi(displacement, nullptr, 16);
  
}

void Emulator::push(int64_t data){
  sp = sp - 4;
  std::stringstream ss;
  ss << std::uppercase << std::hex << data;
  std::string hexa = ss.str();
  while(hexa.size() < 8){
    hexa.insert(0, 1, '0');
  }

  memory[sp] = hexa.substr(6);
  memory[sp+1] = hexa.substr(4,2);
  memory[sp+2] = hexa.substr(2,2);
  memory[sp+3] = hexa.substr(0,2);




}

void Emulator::haltInstruction(){
  std::cout << "ENTER HALT" << std::endl;
  stop = true;
}

void Emulator::intInstruction(){
  std::cout << handler << std::endl;
  std::cout << "INT" << std::endl;
  push(status);
  push(pc);
  cause = 4;
  status = status & (~0x1);
  pc = handler;
}
void Emulator::callInstruction(){
  switch (mod)
  {
  case 0: {
    push(pc);
    pc = registers[regA] + registers[regB] + disp;
    break;
  }
  case 1: {
    push(pc);
    pc = readFourBytes(registers[regA] + registers[regB] + disp);
    break;
  }
  default: {
    std::cerr << "Doslo je do greske kod call instrukcije" << std::endl;
    exit(-1);
  }
    break;
  }
}


void Emulator::jumpInstructions(){
  switch (mod)
  {
  case 0: {
    pc = registers[regA] + disp;
    break;
  }
  case 1: {
    if(registers[regB] == registers[regC]){
      pc = registers[regA] + disp;
    }
    break;
  }
  case 2: {
    if(registers[regB] != registers[regC]){
      pc = registers[regA] + disp;
    }
  
    break;
  }
  case 3: {
    if(registers[regB] > registers[regC]){
      pc = registers[regA] + disp;
    }
    break;
  }
  case 8: {
    pc = readFourBytes(registers[regA] + disp);
    break;
  }
  case 9: {
    if(registers[regB] == registers[regC]){
      pc = readFourBytes(registers[regA] + disp);
    }
    break;
  }
  case 10: {
     if(registers[regB] != registers[regC]){
      pc = readFourBytes(registers[regA] + disp);
    }
    break;
  }
  case 11: {
    if(registers[regB] > registers[regC]){
      pc = readFourBytes(registers[regA] + disp);
    }
    break;
  }
  default: {
    std::cerr << "greska u jump" << std::endl;
    break;
  }
  }
}

int64_t Emulator::readFourBytes(int start){
  std::string fourthByte = memory[start];
  std::string thirdByte = memory[start + 1];
  std::string secondByte = memory[start + 2];
  std::string firstByte = memory[start + 3];

  std::string hexa = firstByte + secondByte + thirdByte + fourthByte;

  int64_t result = std::stoull(hexa, nullptr, 16);

  return result;
}

void Emulator::fetch_and_decode(){
  readInstruction();

  switch (oc)
  {
  case 0: {
    //HALT INSTRUKCIJA
    haltInstruction();
    break;
  }
  case 1: {
    //INT INSTRUKCIJA

    intInstruction();
    break;
  }
  case 2: { 
    callInstruction();

    break;
  }
  
  case 3: {
    jumpInstructions();
    break;
  }

  case 4: {
    xchgInstruction();
    break;
  }
  case 5: {
    arithInstructions();
    break;
  }
  case 6: {
    logicInstructions();
    break;
  }
  case 7: {
    shiftInstructions();
    break;
  }
  case 8: {
    stInstruction();
    break;
  }
  case 9: {
    ldInstruction();
    break;
  }
  default: {
    std::cerr << "Doslo je do greske pri dekoodovanju" << std::endl;
    exit(-1);
    break;
  }
  }
  

}

void Emulator::xchgInstruction(){
  int64_t temp = registers[regB];
  registers[regB] = registers[regC];
  registers[regC] = temp;
  
}

void Emulator::arithInstructions(){

  switch (mod)
  {
  case 0: {
    registers[regA] = registers[regB] + registers[regC];
    break;
  }
  case 1: {
    registers[regA] = registers[regB] - registers[regC];
    break;
  }
  case 2: {
    registers[regA] = registers[regB] * registers[regC];
    break;
  }
  case 3: {
    if(registers[regC] == 0 ){
      cause = 1;
      
      pc = handler; 
    }
    else{
      registers[regA] = registers[regB] / registers[regC];
    }
    break;
  }
  default:
    break;
  }

}

void Emulator::logicInstructions(){
  switch (mod)
  {
  case 0: {
    registers[regA] = ~registers[regB];
    break;
  }
  case 1: {
    registers[regA] = registers[regB] & registers[regC];
    break;
  }
  case 2: {
    registers[regA] = registers[regB] | registers[regC];
    break;
  }
  case 3: {
    registers[regA] = registers[regB] ^ registers[regC];
    
    break;
  }
  default:
    break;
  }
}

void Emulator::shiftInstructions(){
  switch (mod)
  {
  case 0: {
    registers[regA] = registers[regB] << registers[regC];
    break;
  }
  case 1: {
    registers[regA] = registers[regB] >> registers[regC];
    break;
  }
  default:
    break;
  }
}

void Emulator::storeToMemory(int64_t addr, int64_t data){
  std::stringstream ss;
  ss << std::uppercase << std::hex << data;
  std::string hexa = ss.str();
  while(hexa.size() < 8){
    hexa.insert(0, 1, '0');
  }
  memory[addr] = hexa.substr(6);
  memory[addr + 1] = hexa.substr(4,2);
  memory[addr + 2] = hexa.substr(2,2);
  memory[addr + 3] = hexa.substr(0,2);
  //00 00 00 0F
}

void Emulator::stInstruction(){
  switch (mod)
  {
  case 0: {
    int64_t addr = registers[regA] + registers[regB] + disp;
    storeToMemory(addr, registers[regC]);
    break;
  }
  
  case 1: {
    registers[regA] += disp;
    storeToMemory(registers[regA], registers[regC]);
    break;
  }
  case 2: {
    int64_t addr = readFourBytes(registers[regA] + registers[regB] + disp);
    storeToMemory(addr, registers[regC]);
    break;
  }
  default:
    break;
  }
}

void Emulator::ldInstruction(){
  switch (mod)
  {
  case 0: {

    switch(regB){
      case 0:
        registers[regA] = status;
        break;
      case 1:
        registers[regA] = handler;
        break;
      case 2:
        registers[regA] = cause;
        break;
    }
    
    break;
  }
  case 1: {
    registers[regA] = registers[regB] + disp;
    break;
  }
  case 2: {
    registers[regA] = readFourBytes(registers[regB] + registers[regC] + disp);
    break;
  }
  case 3: {
    registers[regA] = readFourBytes(registers[regB]);
    registers[regB] += disp;
    break;
  }
  case 4: {
    switch(regA){
      case 0:
        status = registers[regB];
        break;
      case 1:
        handler = registers[regB];
        break;
      case 2:
        cause = registers[regB];
        break;
    }
    break;
  }
  case 5: {
    switch(regA){
      case 0:
        switch(regB){
          case 0:
            status = status | disp;
            break;
          case 1:
            status = handler | disp;
            break;
          case 2:
            status = cause | disp;
            break;
        }
        
        break;
      case 1:
        switch(regB){
          case 0:
            handler = status | disp;
            break;
          case 1:
            handler = handler | disp;
            break;
          case 2:
            handler = cause | disp;
            break;
        }
        break;
      case 2:
        switch(regB){
          case 0:
            cause = status | disp;
            break;
          case 1:
            cause = handler | disp;
            break;
          case 2:
            cause = cause | disp;
            break;
        }
        break;
    }
    break;
  }
  case 6: {
    int64_t loaded = readFourBytes(registers[regB] + registers[regC] + disp);
    switch(regA){
          case 0:
            status = loaded;
            break;
          case 1:
            handler = loaded;
            break;
          case 2:
            cause = loaded;
            break;
        }
    break;

  }


  case 7: {
    int64_t loaded = readFourBytes(registers[regB]);
    
    switch(regA){
          case 0:
            status = loaded;
            break;
          case 1:
            handler = loaded;
            break;
          case 2:
            cause = loaded;
            break;
    }
    registers[regB] += disp;
    break;
  }
  default:
    break;
  }
}

void Emulator::start_emulation(){

  while(!stop){

    fetch_and_decode();


  }





}

int main(int argc, char* argv[]){
  std::string memoryFile;
  if(argc < 2){
    std::cerr << "Nije moguce emuliranje, nedostaje fajl sa memorijom!" << std::endl;
    exit(-1);
  }
  else{
    memoryFile = argv[1];
  }

  Emulator* emu = new Emulator(memoryFile);

  emu->initializeMemory();
  emu->start_emulation();
  emu->printing();
 
  return 0;
}