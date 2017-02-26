#pragma once

#include "font.h"
#include "globaldefs.h"
//#include "D3DGraphics.h"

#include <fstream>
#include <assert.h>

// for changeing the egdes of a bitmap to create a block like effect
#define BLOCK_EDGE_COLOR_DELTA              60

#pragma pack(push)
#pragma pack(1)
struct BMPHeader
{
    char            charB;
    char            charM;
    unsigned        fileSize;
    unsigned short  reserved[2];
    unsigned        imageDataOffset;
};
struct BitmapInfoHeader
{
    unsigned        headerSize;             
    int             width;
    int             height;
    unsigned short  nrOfColorPlanes;
    unsigned short  bitsPerPixel;
    unsigned        compressionType;
    unsigned        imageSize;
    unsigned        horizontalResolution;
    unsigned        verticalResolution;
    unsigned        nrColorsInPalette;
    unsigned        nrOfImportantColorsUsed;
};
#pragma pack(pop)

class Bitmap
{
public:
    Bitmap()
    {
        width = 0;
        height = 0;
        pixelData = nullptr;
    }
    ~Bitmap()
    {
        unLoadBMP();    
    }
    int     getWidth() const { return width; }
    int     getHeight() const { return height; }
    bool    isImagePresent() const { return pixelData != nullptr; }
    int     *getPixelData() const { return pixelData; }
    void    unLoadBMP()
    {
        width = 0;
        height = 0;
        if ( pixelData != nullptr ) delete pixelData;
        pixelData = nullptr;
    }
    /*
        It will only load 24 bit uncompressed bitmaps
    */
    int     loadBMP( const char *fileName );
    void    loadFromMemory( int width_,int height_,int *pixelData_ );
    void    createEmptyBMP( int width_,int height_ );
    void    createEmptyBMP( int width_,int height_,int fillColor );
    int     loadPNG( const char *filename );
    int     loadFromBMP( const char *fileName, Rect area );
    bool    isEmpty( Rect area,int color );
    void    fill( int color );
    void    putPixel( int i,int c );
    void    putPixel( int x,int y,int c );
    int     getPixel( int x,int y ) const;
    void    copyFromBMP(Bitmap& source, Rect area);
    void    insertFromBMP( int x,int y,Bitmap& source );
    //void    PrintXY( int x,int y,const char *s,void *font );
    //void    PrintXY( void *gfx,int x,int y,const char *s,void *font );
    void    PrintXY( int x,int y,const char *s,void *font );
    Rect    getBlockBorders( int x,int y );
    void    MakeButtonEdges();
    /*
    int saveBitmapTNTFontFile();
    int saveMonoCTNTFontFile();
    */
private:
    int     width;
    int     height;
    int     *pixelData;
};





