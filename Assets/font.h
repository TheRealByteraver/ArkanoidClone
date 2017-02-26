/*

  What follows is the description of the fileformat of
  TNT's internal font files.


Offset:  Size(in bytes):  Description:
-------  ---------------  ---------------------------------------------------

0        20               ID string: "TNT FONT FILE 1.00: "
20       30               FONT Name string (ex: "5x5 STANDARD BITMASK FONT"),
50       1                ASCIIZ (byte 0)
51       1                EOF char (^Z)
52       2                File version word: Major, Minor version nr (2 bytes)
54       2                compat w/ version: Major, Minor version nr (2 bytes)
56       1                Font Type: 0 = bit mask, no fixed colors
                                     1 = bit maps, fixed colors
57       2                Word: "NOCM", nr of character maps saved. (max 512?)
59       1                byte: "NOBS", nr of bytes per scanline per row/char
                          ( == horiz. res. in pixels if fixed colors)
60       1                byte: "NORC", nr of row stored/char (vertical res.)
61       1                byte: Horiz res. in pixels
62       1                byte: vert. -------------- (same as "NORC")
63       1                byte: data compression type. 0 = storage.
64       30               reserved: 30 bytes (set to 0).
94       1                byte: Palette option. 0 -=> no palette was saved.
                                                1 -=> ColorPalette was saved.

    ----------------------------------------------------------
    if the precedent field was 1, the following data follows:

    95   1                byte: "NOC", nr of colors saved.
    96   1                byte: start of color sequence in pa-
                                lette table
    97   NOC*3            RGB data, three bytes/color (R G B)
    ----------------------------------------------------------

?   (1+NOBS*NORC)*NOCM    the data: all the characters.
     ^
     ³
     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ  before each character, a byte that indicates
                          the equivalent ascii code.


EXAMPLE:
--------

          ÚÄÄÄÄÄÄÄÄÄÄÄÄÄ Used Bits   -\
          ³                            > the mostleft bits are used first.
          ³    ÚÄÄÄÄÄÄÄÄ UnUsed Bits -/
          ³    ³
          ³    ³
          ³    ³   ÚÄÄÄÄ byte values: the data stored.
        ÚÄÁÄ¿ ÚÁ¿  ³
bit   : 76543 210  ³
                  ÚÁ¿
Byte 0: 00000 000   0 Ä¿               ÚÄÄ     ÄÄ¿
Byte 1: 01110 000 112  ³               ³   111   ³
Byte 2: 10001 000 136  ³               ³  1   1  ³
Byte 3: 11111 000 248  ÃÄÄÄÄÄ Rows     ³  11111  ³
Byte 4: 10001 000 136  ³               ³  1   1  ³
Byte 5: 10001 000 136  ³               ³  1   1  ³
Byte 6: 00000 000   0  ³               ³         ³
Byte 7: 00000 000   0 ÄÙ               ÀÄÄ     ÄÄÙ


   Letter "A" data sequence: 65 or 97, 0, 112, 136, 248, 136, 136, 0, 0
                             ÀÄÄÂÄÄÄÙ
                                ÀÄÄÄÄÄ byte index in ascii table.

  NOBS = 1 (5 bits fit in ONE byte)
  NORC = 8 (EIGHT rows)

  *****************************
  * Additions for version 1.1 *
  *****************************

  - nr of bytes per scanline can be bigger than 1
  - bitmaps are now allowed
  - variable width & variable height (bitmaps are drawn top down so this is not
    an issue)
  - when bitmaps are stored, the ascii index is followed by one 32bit int:
    the width & the height of the bitmap. The height is in the upper 16 bits.
    The 24bit bitmap follows right after and is in rgb format.
  - Key color?
 

  that's it for the file format description. have fun,

                     BYTERAVER/TNT.


*/
#pragma once

#include "loadbmp.h"

#define CHAR_SPACE      32
#define NR_OF_CHARS     256
#define MAX_SCANLINES   256

#pragma pack(push)
#pragma pack (1)
struct TftHeader {
    char        id[20];          // file ID string
    char        name[30];        // name of font
    char        asciiz;          // asciiz end of string marker
    char        eofSign ;        // end of file marker
    short int   fVersion;        // actual file format version
    short int   cVersion;        // compatible with file format version
    char        isBitmap;        // 0 -=> bit mask, 1 -=> bit maps
    short int   nrCharacters;    // nr of character maps saved.
    char        nrBytesScanline; // nr of bytes per scanline per row/char
    char        nrScanLines;     // nr of row stored/char (vertical res.)
    char        horizontalRes;   // Horiz res. in pixels
    char        verticalRes;     // Vert. res. in pixels
    char        isCompressed;    // compression type. 0 = storage, no comp.
    char        reserved[30];    // reserved for future developments
    char        isPaletteStored; // if 1, a palette is stored
};

struct TftPalette {
    char          nrColors    ; // nr of colors saved
    unsigned char paletteIndex; // index in color table of first color
};
#pragma pack(pop)


class Font {
public:
    Font();
    ~Font();
    int         width  () { return width_;  } 
    int         height () { return height_; }
    bool        isBitmap () { return isBitmap_; }
    int         loadFont( char *fileName );
    char       *getCharData( unsigned char c ) { return charData_[c]; }
    void       *getBmpData( unsigned char c );
private:
    char        width_;
    char        height_;   // width & height in pixels of one char
    bool        isBitmap_;
    char       *charData_[NR_OF_CHARS];
    void       *bmpData_[NR_OF_CHARS];
};

