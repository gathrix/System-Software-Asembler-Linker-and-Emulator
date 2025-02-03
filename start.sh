ASSEMBLER=./assembler
LINKER=./linker
EMULATOR=./emulator

${ASSEMBLER} -o main.o test/main.s
${ASSEMBLER} -o math.o test/math.s
${ASSEMBLER} -o handler.o test/handler.s
${ASSEMBLER} -o isr_timer.o test/isr_timer.s
${ASSEMBLER} -o isr_terminal.o test/isr_terminal.s
${ASSEMBLER} -o isr_software.o test/isr_software.s
${LINKER} -hex \
  -place=my_code@0x40000000 -place=math@0xF0000000 \
  -o program.hex \
  handler.o math.o main.o isr_terminal.o isr_timer.o isr_software.o
${EMULATOR} program.hex

#bison -d -v ./misc/bison.y 
#flex ./misc/flex.lex
#g++ -o asembler  bison.tab.cc lex.yy.cc ./src/asembler.cpp -lfl

#g++ -g src/linker.cpp -o linker
#g++ src/emulator.cpp -o emulator
