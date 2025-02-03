#ifndef __Emulator_hpp__
#define __Emulator_hpp__
#include <string.h>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cassert>
#include <iomanip>
#include <cstdio>


class Emulator{
  private:
    std::map<uint32_t, std::string> memory;
    int64_t  registers[16];
    int64_t handler;
    int64_t cause;
    int64_t status;
    bool stop;
    int oc;
    int mod;
    int regA, regB, regC;
    int64_t disp;
    int64_t &pc = registers[15];
    int64_t &sp = registers[14];
   
    std::string inputFile;

    
    std::vector<std::string> split(std::string s, std::string delimiter);
  public:
    Emulator(std::string);
    void initializeMemory();
    void fetch_and_decode();
    void start_emulation();
    
    
    //Instrukcije
    void haltInstruction();
    void intInstruction();
    void callInstruction();
    void jumpInstructions();
    void xchgInstruction();
    void arithInstructions();
    void logicInstructions();
    void shiftInstructions();
    void stInstruction();
    void ldInstruction();
    void printing();


    void push(int64_t data);


    int64_t readFourBytes(int start);

    void readInstruction();
    void storeToMemory(int64_t addr, int64_t data);
    void printMemory();
    static uint32_t MEMORY_MAPPED_REGISTERS;
    void printRegisters();
};



#endif