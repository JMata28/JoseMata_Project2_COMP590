#include "Z80.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;
unsigned char rom[]={0x06,0x06,0x3e,0x00,0x80,0x05,0xc2,0x04,0x00,0x76};
//char* rom[10];

unsigned char memoryRead(int address){
    unsigned char byte = rom[address];
    return byte;
}

void memoryWrite(int address, unsigned char b){}

int main() {

    Z80 *z80 = new Z80(memoryRead, memoryWrite); //Should there be a "delete" after this line to avoid memory leaks since it includes the new operator? https://stackoverflow.com/questions/655065/when-should-i-use-the-new-keyword-in-c

    z80->reset();
    z80->PC=0;
    while (z80->halted != true) {
        z80->doInstruction();
        cout << "PC: " << z80->PC << endl;
        cout << "A: " << z80->A << endl;
        cout << "B: " << z80->B << endl;
    }
    return 0;
}
