#include "Z80.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;
//PART OF STEP 1: unsigned char rom[]={0x06,0x06,0x3e,0x00,0x80,0x05,0xc2,0x04,0x00,0x76};
char* rom;  //Needed later to read the testrom.gb file
int romSize; //Needed later to read the testrom.gb file

//PART OF STEP 2: The following declarations were copied-and-pasted from the instructions:
unsigned char graphicsRAM[8192];
int palette[4];
int tileset, tilemap, scrollx, scrolly;

//PART OF STEP 2: The declaration of "Screen" below was taken from Dr.Black's "gameboy.cpp" file in her "db" folder.
int Screen[160][144];

//PART OF STEP 3: The following declarations were copied-and-pasted from the instructions:
int HBLANK=0, VBLANK=1, SPRITE=2, VRAM=3;
unsigned char workingRAM[0x2000];

unsigned char page0RAM[0x80];

int line=0, cmpline=0, videostate=0, keyboardColumn=0, horizontal=0;
int gpuMode=HBLANK;
int romOffset = 0x4000;
long totalInstructions=0;

//PART OF STEP 3: The following functions were copied-and-pasted from the instructions:
unsigned char getKey() { return 0xf; }
void setRomMode(int address, unsigned char b) { }
void setControlByte(unsigned char b) {
    tilemap=(b&8)!=0?1:0;
    tileset=(b&16)!=0?1:0;
}
void setPalette(unsigned char b) {
    palette[0]=b&3; palette[1]=(b>>2)&3; palette[2]=(b>>4)&3; palette[3]=(b>>6)&3;}

unsigned char getVideoState() {
    int by=0;
    if(line==cmpline) by|=4;
    if(gpuMode==VBLANK) by|=1;
    if(gpuMode==SPRITE) by|=2;
    if(gpuMode==VRAM) by|=3;
    return (unsigned char)((by|(videostate&0xf8))&0xff);
}


    unsigned char memoryRead(int address){
     //PART OF STEP 1: return rom[address];

     //PART OF STEP 3:
     if (address <=0x3FFF){
         return rom[address];
     }
    if (address >= 0x4000 && address <= 0x7FFF){
        return rom[romOffset + address%0x4000];
    }
    if (address >= 0x8000 && address <= 0x9FFF){
        return graphicsRAM[address%0x2000];
    }
    if (address >= 0xC000 && address <= 0xDFFF){
        return workingRAM[address%0x2000];
    }
    if (address >= 0xFF80 && address <= 0xFFFF){
        return page0RAM[address%0x80];
    }
    if (address == 0xFF00){
        return getKey();
    }
    if (address == 0xFF41){
        return getVideoState();
    }
    if (address == 0xFF42){
        return scrolly;
    }
    if (address == 0xFF43){
        return scrollx;
    }
    if (address == 0xFF44){
        return line;
    }
    if (address == 0xFF45){
        return cmpline;
    }
    return 0;
}

void memoryWrite(int address, unsigned char b){
    //PART OF STEP3:
    if (address <= 0x3FFF){
        setRomMode(address, b);
    }
    if (address >= 0x8000 && address <= 0x9FFF){
        graphicsRAM[address%0x2000]=b;
    }
    if (address >= 0xC000 && address <= 0xDFFF){
        workingRAM[address%0x2000]=b;
    }
    if (address >= 0xFF80 && address <= 0xFFFF){
        page0RAM[address%0x80]=b;
    }
    if (address == 0xFF40){
        setControlByte(b);
    }
    if (address == 0xFF00){
        keyboardColumn = b;
    }
    if (address == 0xFF41){
        videostate=b;
    }
    if (address == 0xFF42){
        scrolly=b;
    }
    if (address == 0xFF43){
        scrollx = b;
    }
    if (address == 0xFF44){
        line = b;
    }
    if (address == 0xFF45){
        cmpline=b;
    }
    if (address == 0xFF47){
        setPalette(b);
    }
}

//The template for the function renderScreen was taken from Dr.Black's "gameboy.cpp" file in her "db" folder.
void renderScreen()
{
    for(int xpixel=0; xpixel<160; xpixel++)
    {
        for(int ypixel=0; ypixel<144; ypixel++)
        {
            //your code here: figure out color in terms of xpixel,ypixel...
            int color=0;
            int x =xpixel;
            int y = ypixel;
            //Instructions: First apply a scroll to shift the image by some amount.  Let x = (x + scrollx)&255 and y =( y + scrolly)&255
            x = (x+scrollx)&255; //Doing the & operation with 255 (0b11111111) ensures that x remains 8 bits in size.
            y = (y+scrolly)&255; //Doing the & operation with 255 (0b11111111) ensures that x remains 8 bits in size.

            //Instructions:	Next figure out which tile the pixel x,y belongs to.  The 160 x 144 screen is divided into 8x8 tiles.  Given a pixel x, y, the tile coordinate is tilex = x/8 , tiley = y/8.
            int tilex=x/8; //this value will be an integer because we only care about the whole number and want to discard the remainder. That means when x=0 to x=7, tilex is 0, and so on and so forth.
            int tiley=y/8;  //this value will be an integer because we only care about the whole number and want to discard the remainder. That means when x=0 to x=7, tilex is 0, and so on and so forth.

            //Instructions: These tiles are organized in rows from left to right and then from top to bottom.  There are 32 tiles in each row -- more than can be seen on the screen.  The list of tile numbers is called the "tile map".  The tile's position in the map is tileposition = tiley * 32 + tilex
            int tileposition = (tiley*32)+tilex;

            // There are two tile maps.  Map 0 is located in the graphicsRAM at address 0x1800 (6144), Map 1 is located at address 0x1c00 (7168).  The map actually used depends on the value of the tilemap variable.  The tile index is thus:
            int tileindex;
            if (tilemap == 0){
                tileindex = graphicsRAM[0x1800+tileposition];
            }
            if (tilemap == 1){
                tileindex = graphicsRAM[0x1c00+tileposition];
            }

            //Instructions: The tile encoding itself -- the pixel colors in the tile -- is located in the bottom of graphicsRAM.  Every tile entry is 16 bytes in size.  The location of the entry, based on the index, is different for  tilesets 0 and 1.
            int tileaddress;
            if (tileset == 1 ){
                tileaddress = tileindex*16;
            }
            if (tileset == 0){
                if (tileindex >= 128){
                    tileindex = tileindex-256;
                }
                tileaddress = tileindex*16+0x1000;
            }

            //Each 8x8 tile is encoded as 8 rows, each row consisting of two bytes.  To get the two bytes for the pixel we want within the tile, calculate xoffset = x % 8 and yoffset = y % 8   The two bytes -- we'll call them row0 and row1 -- are found at
            int xoffset = x % 8;
            int yoffset = y % 8;
            int row0 = graphicsRAM[tileaddress + yoffset * 2];
            int row1 = graphicsRAM[tileaddress + yoffset * 2 + 1];

            //Instructions: So how do you get pixel x's color?  Use the bit shifting operator >> to push all the bits to the right.  Then use bitwise AND (&) to zero out all the bits except the one you're interested in.
            int row0shifted = row0 >> (7-xoffset);
            int row0capturedpixel = row0shifted & 1;

            int row1shifted = row1 >> (7-xoffset);
            int row1capturedpixel = row1shifted & 1;

            color = (row1capturedpixel*2)+row0capturedpixel;
            Screen[xpixel][ypixel]=color;
        }
    }

    //demo: turn on pixels 2,3 ==> 3 (black)
    //demo: 100,100 ==> 2 (gray)
//	Screen[2][3]=3;
//	Screen[100][100]=2;

    //this clears the terminal screen
    printf("\033c");
    for(int y=0; y<144; y++)
    {
        for(int x=0; x<160; x++)
        {
            if(Screen[x][y]==3)
                printf("*");
            if(Screen[x][y]==2)
                printf("+");
            if(Screen[x][y]==1)
                printf(".");
            if(Screen[x][y]==0)
                printf(" ");
        }
        printf("\n");
    }
}

int main() {
    //The set of lines below was copied and pasted from the Word document with the instructions of Project 2
    ifstream romfile("../ttt.gb", ios::in|ios::binary|ios::ate); //The modes here specify that the file is open for input operations, is opened in binary mode instead of text mode, and that the initial position is set at the end of the file
    streampos size=romfile.tellg();
    rom=new char[size]; //Should there be a "delete" after this line to avoid memory leaks since it includes the new operator? https://stackoverflow.com/questions/655065/when-should-i-use-the-new-keyword-in-c
    romSize=size;
    romfile.seekg(0,ios::beg);
    romfile.read(rom,size);
    romfile.close();

    Z80 *z80 = new Z80(memoryRead,
                       memoryWrite); //Should there be a "delete" after this line to avoid memory leaks since it includes the new operator? https://stackoverflow.com/questions/655065/when-should-i-use-the-new-keyword-in-c

    z80->reset();
    //PART OF STEP 1 (no longer needed):
//    while (z80->halted != true) {
//        z80->doInstruction();
//        cout << "PC: " << z80->PC << endl;
//        cout << "A: " << z80->A << endl;
//        cout << "B: " << z80->B << endl;
//    }

//PART OF STEP 3B:
    while (z80->halted != true){
        //do an instruction
        z80 -> doInstruction();

        //check for and handle interrupts
        if(z80->interrupt_deferred>0)
        {
            z80->interrupt_deferred--;
            if(z80->interrupt_deferred==1)
            {
                z80->interrupt_deferred=0;
                z80->FLAG_I=1;
            }
        }
        z80->checkForInterrupts();

        //	figure out the screen position and set the video mode
        horizontal = (int) ((totalInstructions+1)%61);
        if (line >= 145){gpuMode = VBLANK;}
        else if (horizontal <= 30){gpuMode = HBLANK;}
        else if (horizontal >= 31 && horizontal <= 40){gpuMode=SPRITE;}
        else gpuMode = VRAM;

        if (horizontal ==0 ){
            line++;
            if (line == 144){
                z80->throwInterrupt(1);
            }
            if ((line%153 == cmpline) && ((videostate&0x40)!=0)){
                z80->throwInterrupt(2);
            }
            if (line ==153){
                line = 0;
                renderScreen();
            }
        }
        totalInstructions++;
    }
// PART2:
//    //Read the first 8192 integers from screendump into graphicsRAM. (This section of code below was copied-and pasted from project instructions)
//    int n;
//    ifstream vidfile("../screendump.txt",ios::in);
//    for(int i=0; i<8192; i++){
//        int n;
//        vidfile>>n;
//        graphicsRAM[i]=(unsigned char)n;
//    }
//
// //Read the other variables (This section of code below was copied-and pasted from project instructions)
//    vidfile >> tileset;
//    vidfile >> tilemap;
//    vidfile >> scrollx;
//    vidfile >> scrolly;
//    vidfile >> palette[0];
//    vidfile >> palette[1];
//    vidfile >> palette[2];
//    vidfile >> palette[3];
//
//    renderScreen();
}
