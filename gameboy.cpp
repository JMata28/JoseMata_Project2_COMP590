#include "Z80.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;
//unsigned char rom[]={0x06,0x06,0x3e,0x00,0x80,0x05,0xc2,0x04,0x00,0x76};
char* rom;  //Needed later to read the testrom.gb file
int romSize; //Needed later to read the testrom.gb file

//The following declarations were copied-and-pasted from the instructions:
unsigned char graphicsRAM[8192];
int palette[4];
int tileset, tilemap, scrollx, scrolly;

//The declaration of "Screen" below was taken from Dr.Black's "gameboy.cpp" file in her "db" folder.
int Screen[160][144];

unsigned char memoryRead(int address){
     return rom[address];
}

void memoryWrite(int address, unsigned char b){}

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
    ifstream romfile("../testrom.gb", ios::in|ios::binary|ios::ate); //The modes here specify that the file is open for input operations, is opened in binary mode instead of text mode, and that the initial position is set at the end of the file
    streampos size=romfile.tellg();
    rom=new char[size]; //Should there be a "delete" after this line to avoid memory leaks since it includes the new operator? https://stackoverflow.com/questions/655065/when-should-i-use-the-new-keyword-in-c
    romSize=size;
    romfile.seekg(0,ios::beg);
    romfile.read(rom,size);
    romfile.close();

    Z80 *z80 = new Z80(memoryRead,
                       memoryWrite); //Should there be a "delete" after this line to avoid memory leaks since it includes the new operator? https://stackoverflow.com/questions/655065/when-should-i-use-the-new-keyword-in-c

    z80->reset();
//    while (z80->halted != true) {
//        z80->doInstruction();
//        cout << "PC: " << z80->PC << endl;
//        cout << "A: " << z80->A << endl;
//        cout << "B: " << z80->B << endl;
//    }

    //Read the first 8192 integers from screendump into graphicsRAM. (This section of code below was copied-and pasted from project instructions)
    int n;
    ifstream vidfile("../screendump.txt",ios::in);
    for(int i=0; i<8192; i++){
        int n;
        vidfile>>n;
        graphicsRAM[i]=(unsigned char)n;
    }

//Read the other variables (This section of code below was copied-and pasted from project instructions)
    vidfile >> tileset;
    vidfile >> tilemap;
    vidfile >> scrollx;
    vidfile >> scrolly;
    vidfile >> palette[0];
    vidfile >> palette[1];
    vidfile >> palette[2];
    vidfile >> palette[3];

    renderScreen();
}
