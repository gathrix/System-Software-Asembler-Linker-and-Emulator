# System-Software-Asembler-Linker-and-Emulator

Assembler:  OnePass assembler for modified Intel Assembly language for educational 
purposes, written in C++ with parser and lexer implemented in Bison and Flex respectively. 
Result of assembling is a binary file with all the information about instruction and variables 

Linker: Static linker for linking two or more binary files provided by the Assembler. Written 
in C++. Provides either a text file similar to .hex file type which represents memory 
representation after linking all assembler files. It allows -relocatable option which then 
provides another binary file for further linking with other files 

Emulator: Simple emulator made for emulating files provided by linker and assembler. 
Contains 16 32-bit registers used in executing instructions as well as representing its pc and 
sp.Starts executing instruction from a given address which is written in pc on reset. Ends 
emulating when it reaches HALT instruction. 
