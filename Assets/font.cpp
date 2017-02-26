#pragma once

#include "font.h"
#include <fstream>
#include <assert.h>

Font::Font() 
{
    for (int i = 0; i < NR_OF_CHARS; i++)
    {
        charData_[i] = nullptr;
        bmpData_[i] =  nullptr;
    }
    width_       = 0;
    height_      = 0;
    isBitmap_    = false;
}
Font::~Font()
{
    for (int i = 0; i < NR_OF_CHARS; i++)
    {
        if ( charData_[i] != nullptr ) delete charData_[i];
        if ( ((Bitmap *)bmpData_[i]) != nullptr ) delete ((Bitmap *)bmpData_[i]); 
    }    
}
void *Font::getBmpData( unsigned char c )  
{ 
    /*
        return an empty char if the requested char is not available
    */
    void *t = bmpData_[c];
    if ( t != nullptr ) return t;  
    else return bmpData_[0];
}
int Font::loadFont(char *fileName)
{
    TftHeader       header;
    std::ifstream   fontFile;
    fontFile.open( fileName, std::ios_base::in | std::ios_base::binary );
    if ( ! fontFile ) return - 1;

    fontFile.read( (char *)(&header), sizeof(TftHeader) );
    if( ( header.isCompressed ) || 
        ( header.isPaletteStored ) ||
        ( header.nrScanLines > MAX_SCANLINES ) ||
        ( header.nrCharacters > NR_OF_CHARS ) ) return - 1;
    
    width_  = header.horizontalRes;
    height_ = header.verticalRes;
    isBitmap_ = header.isBitmap != 0;

    if ( ! isBitmap_ )
    {
        for (int i = 0; i < NR_OF_CHARS; i++)    
        {
            charData_[i] = new char[height_ * header.nrBytesScanline];
            memset( charData_[i],0,height_ * header.nrBytesScanline );
        }
        for (int iChar = 0; iChar < header.nrCharacters; iChar++)    
        {
            unsigned char index;
            fontFile.read( (char *)(&index),sizeof(char) );
            fontFile.read( charData_[index],height_ * header.nrBytesScanline );
        }
    } else {
        /*
            Create an empty bitmap and let every character point to it:
        */   
        /*
        bmpData_[0] = new Bitmap;
        Bitmap *bmpData = (Bitmap *)bmpData_[0];
        bmpData->width = 1;
        bmpData->height = 1;
        bmpData->pixelData = new int[bmpData->width];
        memset( bmpData->pixelData,0,bmpData->width * sizeof(int) );
        for ( int i = 1; i < NR_OF_CHARS; i++ ) bmpData_[i] = nullptr;
        */
        bmpData_[0] = new Bitmap;
        ((Bitmap *)bmpData_[0])->createEmptyBMP( 1,1,0x0 );
        for ( int i = 1; i < NR_OF_CHARS; i++ ) bmpData_[i] = nullptr;
        /*
            Now load the bitmapped font:    
        */
        for ( int iChar = 0; iChar < header.nrCharacters; iChar++ )
        {
            unsigned char ascii;
            unsigned short charHeight;
            unsigned short charWidth;
            fontFile.read( (char *)(&ascii),sizeof(char) );
            fontFile.read( (char *)(&charWidth ),sizeof(unsigned short) );
            fontFile.read( (char *)(&charHeight),sizeof(unsigned short) );
            if ( ascii >= NR_OF_CHARS ) return - 1;

            bmpData_[ascii] = new Bitmap;
            Bitmap *bmpData = (Bitmap *)bmpData_[ascii];
            /*
            bmpData->height = charHeight;
            bmpData->width = charWidth;
            bmpData->pixelData = new int[charHeight * charWidth];
            */
            bmpData->createEmptyBMP( charWidth,charHeight,0x0 );
            for( int c = 0; c < charHeight * charWidth; c++ )
            {
                unsigned char r;
                unsigned char g;
                unsigned char b;
                fontFile.read( (char *)(&r),sizeof(char) );
                fontFile.read( (char *)(&g),sizeof(char) );
                fontFile.read( (char *)(&b),sizeof(char) );
                unsigned int color = r;
                color <<= 8;
                color += g;
                color <<= 8;
                color += b;
                bmpData->putPixel( c,color );
                //bmpData->pixelData[c] = color;
            }
        }
    }
    return 0;
}