/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	D3DGraphics.cpp																		  *
 *	Copyright 2012 PlanetChili.net														  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include "D3DGraphics.h"
#include "font.h"
#include "EvoConsole.h"
#include "levelDesigner.h"
#include <math.h>
#include <assert.h>
#include <string.h>

D3DGraphics::D3DGraphics( HWND hWnd )
{
	HRESULT result;

	backRect.pBits = NULL;
	
	pDirect3D = Direct3DCreate9( D3D_SDK_VERSION );
	assert( pDirect3D != NULL );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp,sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    result = pDirect3D->CreateDevice( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,&d3dpp,&pDevice );
	assert( !FAILED( result ) );

	result = pDevice->GetBackBuffer( 0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer );
	assert( !FAILED( result ) );

    // initialize user variables:
    frameWidth = DEFAULT_FRAME_WIDTH;
    frameColor = DEFAULT_FRAME_COLOR;
    setTextColor( DEFAULT_TEXT_COLOR );
    font = nullptr;
}
D3DGraphics::~D3DGraphics()
{
	if( pDevice )
	{
		pDevice->Release();
		pDevice = NULL;
	}
	if( pDirect3D )
	{
		pDirect3D->Release();
		pDirect3D = NULL;
	}
	if( pBackBuffer )
	{
		pBackBuffer->Release();
		pBackBuffer = NULL;
	}
}
void D3DGraphics::BeginFrame()
{
	HRESULT result;

	result = pDevice->Clear( 0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,0,0),0.0f,0 );
	assert( !FAILED( result ) );

    //pDevice->UpdateSurface
    //pBackBuffer->


	result = pBackBuffer->LockRect( &backRect,NULL,NULL );
	assert( !FAILED( result ) );
}
void D3DGraphics::EndFrame()
{
	HRESULT result;

	result = pBackBuffer->UnlockRect();
	assert( !FAILED( result ) );

	result = pDevice->Present( NULL,NULL,NULL,NULL );
    //pDevice->

	assert( !FAILED( result ) );
}
/*
int D3DGraphics::Darken( int src, int darken )  // inlined it
{
    int r = (src & 0x00FF0000) >> 16;
    int g = (src & 0x0000FF00) >> 8;
    int b = (src & 0x000000FF);
    return D3DCOLOR_XRGB( 
        (r > darken) ? r - darken : 0,
        (g > darken) ? g - darken : 0,
        (b > darken) ? b - darken : 0
    );
}
*/
inline void D3DGraphics::PutPixel( int x,int y,int color )
{	
#ifdef DEBUG_MODE
    if ( x < 0 ) return; 
    if ( y < 0 ) return; 
    if ( x >= HOR_SCREEN_RES ) return; 
    if ( y >= VER_SCREEN_RES ) return; 
#else
    assert( x >= 0 );
	assert( y >= 0 );
	assert( x < HOR_SCREEN_RES );
	assert( y < VER_SCREEN_RES );
#endif
	((D3DCOLOR*)backRect.pBits)[ x + (backRect.Pitch >> 2) * y ] = color;
}
void D3DGraphics::DrawDisc( int cx,int cy,int r,int color )
{
	int rSquared = r * r;
	int xPivot = (int)(r * 0.707107f + 0.5f);
	for( int x = 0; x <= xPivot; x++ )
	{
		int y = (int)(sqrt( (float)( rSquared - x*x ) ) + 0.5f);
        int yHi = cy - y;
        int yLo = cy + y;
        for ( int ix = cx - x; ix <= cx + x; ix++ ) PutPixel ( ix,yHi,color );
        for ( int ix = cx - x; ix <= cx + x; ix++ ) PutPixel ( ix,yLo,color );
        yHi = cy - x;
        yLo = cy + x;
        for ( int ix = cx - y; ix <= cx + y; ix++ ) PutPixel ( ix,yHi,color );
        for ( int ix = cx - y; ix <= cx + y; ix++ ) PutPixel ( ix,yLo,color );
	}
}
void D3DGraphics::DrawLine( int x1,int y1,int x2,int y2,int color )
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	if( dy == 0 && dx == 0 )
	{
		PutPixel( x1,y1,color );
	}
	else if( abs( dy ) > abs( dx ) )
	{
		if( dy < 0 )
		{
			int temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		float m = (float)dx / (float)dy;
		float b = x1 - m*y1;
		for( int y = y1; y <= y2; y = y + 1 )
		{
			int x = (int)(m*y + b + 0.5f);
			PutPixel( x,y,color );
		}
	}
	else
	{
		if( dx < 0 )
		{
			int temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		float m = (float)dy / (float)dx;
		float b = y1 - m*x1;
		for( int x = x1; x <= x2; x = x + 1 )
		{
			int y = (int)(m*x + b + 0.5f);
			PutPixel( x,y,color );
		}
	}
}
void D3DGraphics::DrawHorLine( int x1,int y,int x2,int color )
{
    for ( int x = x1; x <= x2; x++ ) PutPixel( x,y,color );
}
void D3DGraphics::DrawVerLine( int x,int y1,int y2,int color )
{
    for ( int y = y1; y <= y2; y++ ) PutPixel( x,y,color );
}
// can be slow since only meant for debugging
void D3DGraphics::DrawMathLine( int x,int y,float m,int length,int color )
{
    int l = length / 2;
    int b = (int)(y - m * x);
    int x1;
    int x2;
    int y1;
    int y2;
    if ( abs( m ) < 1.0f )
    {
        x1 = x - l;
        x2 = x + l;
        y1 = (int)(m * x1 + b);
        y2 = (int)(m * x2 + b);
    } else
    {
        y1 = y - l;
        y2 = y + l;
        x1 = (int)((y1 - b) / m);
        x2 = (int)((y2 - b) / m);
    }
    //Draw the Line ( x1,y1,x2,y2 ) :
    int dx = x2 - x1;
    int dy = y2 - y1;
    if ( dy == 0 && dx == 0 )
    {
        if ( x >= 0 && x < HOR_SCREEN_RES && y >= 0 && y < VER_SCREEN_RES )
            PutPixel( x1,y1,color );
    } else if ( abs( dy ) > abs( dx ) )
    {
        if ( dy < 0 )
        {
            int temp = x1;
            x1 = x2;
            x2 = temp;
            temp = y1;
            y1 = y2;
            y2 = temp;
        }
        float m = (float)dx / (float)dy;
        float b = x1 - m*y1;
        for ( int y = y1; y <= y2; y = y + 1 )
        {
            int x = (int)(m*y + b + 0.5f);
            if( x >= 0 && x < HOR_SCREEN_RES && y >=0 && y < VER_SCREEN_RES )
                PutPixel( x,y,color );
        }
    } else
    {
        if ( dx < 0 )
        {
            int temp = x1;
            x1 = x2;
            x2 = temp;
            temp = y1;
            y1 = y2;
            y2 = temp;
        }
        float m = (float)dy / (float)dx;
        float b = y1 - m*x1;
        for ( int x = x1; x <= x2; x = x + 1 )
        {
            int y = (int)(m*x + b + 0.5f);
            if ( x >= 0 && x < HOR_SCREEN_RES && y >= 0 && y < VER_SCREEN_RES )
                PutPixel( x,y,color );
        }
    }
}
void D3DGraphics::DrawCircle( int centerX,int centerY,int radius,int color )
{
	int rSquared = radius*radius;
	int xPivot = (int)(radius * 0.707107f + 0.5f);
	for( int x = 0; x <= xPivot; x++ )
	{
		int y = (int)(sqrt( (float)( rSquared - x*x ) ) + 0.5f);
		PutPixel( centerX + x,centerY + y,color );
		PutPixel( centerX - x,centerY + y,color );
		PutPixel( centerX + x,centerY - y,color );
		PutPixel( centerX - x,centerY - y,color );
		PutPixel( centerX + y,centerY + x,color );
		PutPixel( centerX - y,centerY + x,color );
		PutPixel( centerX + y,centerY - x,color );
		PutPixel( centerX - y,centerY - x,color );
	}
}
template <class V> inline void swap (V &a, V &b)
{
    V temp = a;
    a = b;
    b = temp;
}
void D3DGraphics::DrawBox( const Rect& coords,int color )
{
    DrawBox( coords.x1,coords.y1,coords.x2,coords.y2,color );
}
void D3DGraphics::DrawBox( int x1,int y1,int x2,int y2,int color )
{
    for (int x = x1; x <= x2; x++) 
    {
        PutPixel( x,y1,color );
        PutPixel( x,y2,color );
    }
    for (int y = y1; y <= y2; y++) 
    {
        PutPixel( x1,y,color );
        PutPixel( x2,y,color );
    }
}
void D3DGraphics::DrawBoxClipped( Rect coords, Rect clippedArea,int color )
{
    int x1;
    int y1;
    int x2;
    int y2;
    if ( coords.x1 >= clippedArea.x1 ) x1 = coords.x1;
    else x1 = clippedArea.x1;
    if ( x1 > clippedArea.x2 ) x1 = clippedArea.x2;
    if ( coords.x2 <= clippedArea.x2 ) x2 = coords.x2;
    else x2 = clippedArea.x2;
    if ( x2 < clippedArea.x1 ) x2 = clippedArea.x1;
    if ( coords.y1 >= clippedArea.y1 ) y1 = coords.y1;
    else y1 = clippedArea.y1;
    if ( y1 > clippedArea.y2 ) y1 = clippedArea.y2;
    if ( coords.y2 <= clippedArea.y2 ) y2 = coords.y2;
    else y2 = clippedArea.y2;
    if ( y2 < clippedArea.y1 ) y2 = clippedArea.y1;
    if( coords.y1 == y1 ) DrawHorLine( x1,y1,x2,color );
    if( coords.y2 == y2 ) DrawHorLine( x1,y2,x2,color );
    if( coords.x1 == x1 ) DrawVerLine( x1,y1,y2,color );
    if( coords.x2 == x2 ) DrawVerLine( x2,y1,y2,color );
}
void D3DGraphics::DrawBlock( Rect coords,int color )
{
    DrawBlock( coords.x1,coords.y1,coords.x2,coords.y2,color );
}
void D3DGraphics::DrawBlock( int x1,int y1,int x2,int y2,int color )
{
    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++)
            PutPixel( x,y,color );
}
void D3DGraphics::PrintXY( int x,int y,const char *s )
{
    if( s == nullptr ) return;
    int slen = strlen( s );
    int height = font->height ();
    if( ! font->isBitmap() )
    {
        int width  = font->width  ();
        int stringWidth = width * slen;

        assert( (x + stringWidth) < HOR_SCREEN_RES );
        assert( (y + height     ) < VER_SCREEN_RES );

        D3DCOLOR *s1 = (D3DCOLOR *)backRect.pBits;
        s1 += x + (backRect.Pitch >> 2) * y;
        for ( int iChar = 0; iChar < slen; iChar++ )
        {
            D3DCOLOR    *s2 = s1;
            char *iData = font->getCharData( s[iChar] );
            for ( int j = 0; j < height; j++ )
            {
                int iByte;            
                for ( int i = 0; i < width; i++ )
                {
                    iByte = i % 8;
                    if ( (iByte == 0) && (i > 0) ) iData++;
                    if ( ((*iData) & (1 << (7 - iByte))) != 0 ) 
                        *s2 = textColor;
                    s2++;
                }
                if ( iByte > 0 ) iData++;
                s2 += (backRect.Pitch >> 2) - width;
            }
            s1 += width;
        }
    } else {
        int startX = x;
        for (int i = 0; i < slen; i++ )
        {
            PaintBMPClearType( startX,y,*((Bitmap *)(font->getBmpData( s[i] ))),0 );
            startX += ((Bitmap *)(font->getBmpData( s[i] )))->getWidth();
        }        
    }
}
void D3DGraphics::PrintXY( int x,int y,const char *s, int opacity )
{
    if( s == nullptr ) return;
    int slen = strlen( s );
    int height = font->height ();
    if( font->isBitmap() == false )
    {
        int width  = font->width  ();
        int stringWidth = width * slen;
        int destNextLine = (backRect.Pitch >> 2) - width;

        assert( (x + stringWidth) < HOR_SCREEN_RES );
        assert( (y + height     ) < VER_SCREEN_RES );

        D3DCOLOR *s1 = (D3DCOLOR *)backRect.pBits;
        s1 += x + (backRect.Pitch >> 2) * y;
        for ( int iChar = 0; iChar < slen; iChar++ )
        {
            D3DCOLOR    *s2 = s1;
            char *iData = font->getCharData( s[iChar] );
            for ( int j = 0; j < height; j++ )
            {
                int iByte;            
                for ( int i = 0; i < width; i++ )
                {
                    iByte = i % 8;
                    if ( (iByte == 0) && (i > 0) ) iData++;
                    if ( ((*iData) & (1 << (7 - iByte))) != 0 ) 
                        *s2 = textColor;
                    s2++;
                }
                if ( iByte > 0 ) iData++;
                s2 += destNextLine;
            }
            s1 += width;
        }
    } else {
        int startX = x;
        for (int i = 0; i < slen; i++ )
        {
            PaintBMPClearType( startX,y,*((Bitmap *)(font->getBmpData(s[i]))),0,opacity );
            startX += ((Bitmap *)(font->getBmpData(s[i])))->getWidth();
        }        
    }
}
int D3DGraphics::getStrLen( const char *s,Font *font )
{
    if ( s == nullptr ) return 0;
    int width = font->width();
    int slen = strlen( s );
    if ( font->isBitmap() == false ) return slen * width;
    else {
        width = 0;
        for ( int i = 0; i < slen; i++ ) 
            width += ((Bitmap *)font->getBmpData( s[i] ))->getWidth();
        return width;
    }
}
void D3DGraphics::PrintXY( int x,int y,const char *s,int opacity,Font& font )
{
    Font *oldFont = getFont();
    setFont( &font );
    PrintXY( x,y,s,opacity );
    setFont( oldFont );
}
void D3DGraphics::PrintXYSolid( int x,int y,int xSpacer,char *s,int color /*, HRESULT *hres*/ )
{
    if( s == nullptr ) return;
    int width  = font->width  ();
    int height = font->height ();
    int slen = strlen( s );
    int stringWidth = (width  + xSpacer) * slen;

    assert( ( x + stringWidth) < HOR_SCREEN_RES );
    assert( ( y + height )     < VER_SCREEN_RES );

    int i = x;
    D3DCOLOR *s1 = (D3DCOLOR *)backRect.pBits;
    s1 += x + (backRect.Pitch >> 2) * y;
    for ( int iChar = 0; iChar < slen; iChar++ )
    {
        unsigned char c = s[iChar];
        char *charMask = font->getCharData( c );
        D3DCOLOR    *s2 = s1;
        for ( int py = 0; py < height; py++ )
        {
            unsigned char d = charMask[py];
            for ( int px = 0; px < width; px++ )
            {
                if (d & (1 << (7 - px))) *s2 = D3DCOLOR_XRGB( 255,255,255 );
                else                     *s2 = color;
                s2++;
            } 
            s2 += (backRect.Pitch >> 2) - width;
        }
        s1 += width + xSpacer;
    }
/*
//http://msdn2.microsoft.com/en-us/library/bb172773.aspx


    RECT        lineRect;
    ID3DXFont   *pDefaultFont;
    D3DCOLOR    defaultFontColor;

    lineRect.left   = 0;
    lineRect.top    = 0;
    lineRect.right  = HOR_SCREEN_RES - 1;
    lineRect.bottom = VER_SCREEN_RES - 1;

    D3DXCreateFont(
        pDevice, 
        30,  
        0, 
        FW_BOLD, 
        0, 
        FALSE,
        DEFAULT_CHARSET, 
        OUT_DEFAULT_PRECIS,
        DEFAULT_QUALITY, 
        DEFAULT_PITCH | FF_DONTCARE,
        TEXT("Calibri"), 
        &pDefaultFont );
    defaultFontColor = D3DCOLOR_ARGB(255, 255, 255, 255);
    *hres = pDefaultFont->DrawTextA(NULL, s, -1, &lineRect, DT_RIGHT, defaultFontColor);
*/
}
void D3DGraphics::PaintBMP( int x,int y,Bitmap &bitmap )
{
    //if( bitmap.pixelData == nullptr ) return;
    if ( ! bitmap.isImagePresent() ) return;
    if( (x >= HOR_SCREEN_RES) || (y >= VER_SCREEN_RES) ) return;
    int xStart, yStart;
    int xEnd, yEnd;
    int xOffset, yOffset;
    if ( x < 0 ) { xStart = 0; xOffset = - x; } 
    else         { xStart = x; xOffset =   0; }
    if ( y < 0 ) { yStart = 0; yOffset = - y; } 
    else         { yStart = y; yOffset =   0; }
    xEnd = x + bitmap.getWidth();
    yEnd = y + bitmap.getHeight();
    if ( (xEnd <= 0) || (yEnd <= 0) ) return;
    if ( xEnd > HOR_SCREEN_RES ) xEnd = HOR_SCREEN_RES;
    if ( yEnd > VER_SCREEN_RES ) yEnd = VER_SCREEN_RES;
    int *data = bitmap.getPixelData() + yOffset * bitmap.getWidth() + xOffset;
    D3DCOLOR *s = ((D3DCOLOR*)backRect.pBits) + (backRect.Pitch >> 2) * yStart + xStart;
    int sNextLine = (backRect.Pitch >> 2) - (xEnd - xStart);
    int dataNextLine = bitmap.getWidth() - (xEnd - xStart);
    for ( int j = yStart; j < yEnd; j++ )
    {
        for ( int i = xStart; i < xEnd; i++ ) { *s++ = *data++; }
        data += dataNextLine;
        s += sNextLine;
    }
}
/*
void D3DGraphics::PaintBMPWrapAround( Rect r,Bitmap &bitmap )
{
    if ( !bitmap.isImagePresent() ) return;
    int columns = r.width() / bitmap.getWidth();
    int colLeft = r.width() % bitmap.getWidth();
    int rows    = r.height() / bitmap.getHeight();
    int rwsLeft = r.height() % bitmap.getHeight();
    int i,j;
    for ( j = 0; j < rows; j++ )
    {
        for ( i = 0; i < columns; i++ )
        {
            PaintBMP(
                r.x1 + i * bitmap.getWidth(),
                r.y1 + j * bitmap.getHeight(),
                bitmap );
        }        
        PaintFromBMP(
            r.x1 + i * bitmap.getWidth(),
            r.y1 + j * bitmap.getHeight(),
            Rect( 0,0,colLeft - 1,bitmap.getHeight() - 1 ),
            bitmap );
    }
    for ( i = 0; i < columns; i++ )
    {        
        PaintFromBMP(
            r.x1 + i * bitmap.getWidth(),
            r.y1 + j * bitmap.getHeight(),
            Rect( 0,0,bitmap.getWidth() - 1,rwsLeft - 1 ),
            bitmap );
    }    
    PaintFromBMP(
        r.x1 + i * bitmap.getWidth(),
        r.y1 + j * bitmap.getHeight(),
        Rect( 0,0,colLeft - 1,rwsLeft - 1 ),
        bitmap );
}
*/
void D3DGraphics::PaintFromBMP( int x,int y,Rect area,Bitmap &bitmap)
{
    assert(area.x1 >= 0);
    assert(area.y1 >= 0);
    if ( ! bitmap.isImagePresent() ) return;
    if ((x >= HOR_SCREEN_RES) || (y >= VER_SCREEN_RES)) return;
    int xOffset = 0;
    int yOffset = 0;
    if (x < 0) { xOffset = -x; x = 0; }
    if (y < 0) { yOffset = -y; y = 0; }
    int sourceWidth  = area.x2 - area.x1 + 1;
    int sourceHeight = area.y2 - area.y1 + 1;
    int drawWidth  = sourceWidth  - xOffset;
    int drawHeight = sourceHeight - yOffset;
    if (x + drawWidth  > HOR_SCREEN_RES) drawWidth  = HOR_SCREEN_RES - x;
    if (y + drawHeight > VER_SCREEN_RES) drawHeight = VER_SCREEN_RES - y;
    if (area.x1 + xOffset + drawWidth  > bitmap.getWidth() )
        drawWidth  = bitmap.getWidth() - xOffset - area.x1;
    if (area.y1 + yOffset + drawHeight > bitmap.getHeight() )
        drawHeight = bitmap.getHeight() - yOffset - area.y1;
    int *src = bitmap.getPixelData() + area.x1 + xOffset
                + (area.y1 + yOffset) * bitmap.getWidth();
    D3DCOLOR *dst = ((D3DCOLOR *)backRect.pBits) + (backRect.Pitch >> 2) * y + x;
    int srcNextLine = bitmap.getWidth() - drawWidth;
    int dstNextLine = HOR_SCREEN_RES - drawWidth;
    for(int j = 0; j < drawHeight; j++)
    {
        for (int i = 0; i < drawWidth; i++) *dst++ = *src++;
        src += srcNextLine;
        dst += dstNextLine;
    }
}
void D3DGraphics::PaintBMPKey( int x,int y,Bitmap &bitmap, int keyColor )  // not tested
{
    //if( bitmap.pixelData == nullptr ) return;
    if ( ! bitmap.isImagePresent() ) return;
    if ( (x >= HOR_SCREEN_RES) || (y >= VER_SCREEN_RES) ) return;
    int xStart, yStart;
    int xEnd, yEnd;
    int xOffset, yOffset;
    if ( x < 0 ) { xStart = 0; xOffset = - x; } 
    else         { xStart = x; xOffset =   0; }
    if ( y < 0 ) { yStart = 0; yOffset = - y; } 
    else         { yStart = y; yOffset =   0; }
    xEnd = x + bitmap.getWidth();
    yEnd = y + bitmap.getHeight();
    if ( (xEnd < 0) || (yEnd < 0) ) return;
    if ( xEnd > HOR_SCREEN_RES ) xEnd = HOR_SCREEN_RES;
    if ( yEnd > VER_SCREEN_RES ) yEnd = VER_SCREEN_RES;
    int *data = bitmap.getPixelData() + yOffset * bitmap.getWidth() + xOffset;
    D3DCOLOR *s = ((D3DCOLOR*)backRect.pBits) + (backRect.Pitch >> 2) * yStart + xStart;
    int sNextLine = (backRect.Pitch >> 2) - (xEnd - xStart);
    int dataNextLine = bitmap.getWidth() - (xEnd - xStart);
    for ( int j = yStart; j < yEnd; j++ )
    {
        for ( int i = xStart; i < xEnd; i++ ) 
        { 
            int c = *s++;
            if ( c != keyColor ) *data = c;
            data++;
        }
        data += dataNextLine;
        s += sNextLine;
    }
}
void D3DGraphics::PaintBMPClearType( int x,int y,Bitmap &bitmap, int keyColor )
{
    //if( bitmap.pixelData == nullptr ) return;
    if ( ! bitmap.isImagePresent() ) return;
    if ( (x >= HOR_SCREEN_RES) || (y >= VER_SCREEN_RES) ) return;
    int xStart, yStart;
    int xEnd, yEnd;
    int xOffset, yOffset;
    if ( x < 0 ) { xStart = 0; xOffset = - x; } 
    else         { xStart = x; xOffset =   0; }
    if ( y < 0 ) { yStart = 0; yOffset = - y; } 
    else         { yStart = y; yOffset =   0; }
    xEnd = x + bitmap.getWidth();
    yEnd = y + bitmap.getHeight();
    if ( (xEnd < 0) || (yEnd < 0) ) return;
    if ( xEnd > HOR_SCREEN_RES ) xEnd = HOR_SCREEN_RES;
    if ( yEnd > VER_SCREEN_RES ) yEnd = VER_SCREEN_RES;
    int *bmpData = bitmap.getPixelData() + yOffset * bitmap.getWidth() + xOffset;
    D3DCOLOR *dest = (D3DCOLOR*)backRect.pBits + (backRect.Pitch >> 2) * yStart + xStart;
    int destNextLine = (backRect.Pitch >> 2) - (xEnd - xStart);
    int bmpDataNextLine = bitmap.getWidth() - (xEnd - xStart);
    for ( int j = yStart; j < yEnd; j++ )
    {
        int pixelCnt = xOffset;
        for ( int i = xStart; i < xEnd; i++ )
        {
            int c = *bmpData++;
            if ( c != keyColor ) 
            {   
                if ( c == 0xFFFFFF ) *dest = 0xFFFFFF;
                else {
                    // recycle MS cleartype technology a little ;)
                    int s = *dest;
                    int sr = (s & 0xFF0000) >> 16;
                    int sg = (s & 0x00FF00) >> 8;
                    int sb = (s & 0x0000FF);
                    int dr = (c & 0xFF0000) >> 16;
                    int dg = (c & 0x00FF00) >> 8;
                    int db = (c & 0x0000FF);
                    sr = sr + dr; if ( sr > 255 ) sr = 255;
                    sg = sg + dg; if ( sg > 255 ) sg = 255;
                    sb = sb + db; if ( sb > 255 ) sb = 255;
                    *dest = (sr << 16) + (sg << 8) + sb;
                }
            }
            dest++;
        }
        bmpData += bmpDataNextLine;
        dest += destNextLine;
    }
}
void D3DGraphics::PaintBMPClearType( int x,int y,Bitmap &bitmap, int keyColor, int opacity )
{
    if ( ! bitmap.isImagePresent() ) return;
    if ( (x >= HOR_SCREEN_RES) || (y >= VER_SCREEN_RES) ) return;
    int xStart, yStart;
    int xEnd, yEnd;
    int xOffset, yOffset;
    if ( x < 0 ) { xStart = 0; xOffset = - x; } 
    else         { xStart = x; xOffset =   0; }
    if ( y < 0 ) { yStart = 0; yOffset = - y; } 
    else         { yStart = y; yOffset =   0; }
    xEnd = x + bitmap.getWidth();
    yEnd = y + bitmap.getHeight();
    if ( (xEnd < 0) || (yEnd < 0) ) return;
    if ( xEnd > HOR_SCREEN_RES ) xEnd = HOR_SCREEN_RES;
    if ( yEnd > VER_SCREEN_RES ) yEnd = VER_SCREEN_RES;
    int *bmpData = bitmap.getPixelData() + yOffset * bitmap.getWidth() + xOffset;
    D3DCOLOR *dest = (D3DCOLOR*)backRect.pBits + (backRect.Pitch >> 2) * yStart + xStart;
    int destNextLine = (backRect.Pitch >> 2) - (xEnd - xStart);
    int bmpDataNextLine = bitmap.getWidth() - (xEnd - xStart);
    for ( int j = yStart; j < yEnd; j++ )
    {
        int pixelCnt = xOffset;
        for ( int i = xStart; i < xEnd; i++ )
        {
            int c = *bmpData++;
            if ( c != keyColor ) 
            {   
                int s = *dest;  // video memory read == slow!
                int sr = (s & 0xFF0000) >> 16;
                int sg = (s & 0x00FF00) >> 8;
                int sb = (s & 0x0000FF);
                int dr = (c & 0xFF0000) >> 16;
                int dg = (c & 0x00FF00) >> 8;
                int db = (c & 0x0000FF);
                if ( c != 0xFFFFFF ) {
                    // recycle MS cleartype technology a little ;)
                    dr = sr + dr; if ( dr > 255 ) dr = 255;
                    dg = sg + dg; if ( dg > 255 ) dg = 255;
                    db = sb + db; if ( db > 255 ) db = 255;
                }
                // blend it with the background based on the opacity
                *dest = (((dr << 16) + ((opacity  * (sr - dr)) << 8)) & 0xFF0000)
                        | (dg <<  8) + ((opacity  * (sg - dg))        &   0xFF00)
                        | (db        + ((opacity  * (sb - db)) >> 8));                
            }
            dest++;
        }
        bmpData += bmpDataNextLine;
        dest += destNextLine;
    }
}
/*
    Specialized drawing functions for interface
*/
void D3DGraphics::DrawNiceBlock( Rect r )
{
    int rd = ((frameColor >> 16) & 0xFF) / frameWidth;
    int gd = ((frameColor >>  8) & 0xFF) / frameWidth;
    int bd = ((frameColor      ) & 0xFF) / frameWidth;
    int red = rd;
    int grn = gd;
    int blu = bd;
    int i;
    for ( i = 0; i < frameWidth; i++ )
    {
        DrawBox( r.x1 + i,r.y1 + i,r.x2 - i,r.y2 - i,D3DCOLOR_XRGB( red,grn,blu ) );
        red += rd;
        grn += gd;
        blu += bd;
    }
    DrawBlock( r.x1 + i,r.y1 + i,r.x2 - i,r.y2 - i,frameColor );
}
void D3DGraphics::DrawNiceBlockInv( Rect r )
{
    int red = ((frameColor >> 16) & 0xFF);
    int grn = ((frameColor >>  8) & 0xFF);
    int blu = ((frameColor      ) & 0xFF);
    int rd = red / frameWidth;
    int gd = grn / frameWidth;
    int bd = blu / frameWidth;
    int i;
    int c;
    for ( i = 0; i < frameWidth; i++ )
    {
        c = D3DCOLOR_XRGB( red,grn,blu );
        DrawBox( r.x1 + i,r.y1 + i,r.x2 - i,r.y2 - i,c );
        red -= rd;
        grn -= gd;
        blu -= bd;
    }
    DrawBlock( r.x1 + i,r.y1 + i,r.x2 - i,r.y2 - i,c );
}
void D3DGraphics::DrawButton( Rect r )
{
    DrawBox( r.x1,r.y1,r.x2,r.y2,0 );
    int colorDelta = 60;
    int red = ((frameColor >> 16) & 0xFF);
    int grn = ((frameColor >>  8) & 0xFF);
    int blu = ((frameColor      ) & 0xFF);
    int rd = red - colorDelta; if ( rd < 0 ) rd = 0;
    int gd = grn - colorDelta; if ( gd < 0 ) gd = 0;
    int bd = blu - colorDelta; if ( bd < 0 ) bd = 0;
    int darkColor = (rd << 16) + (gd << 8) + bd;
    int rl = red + colorDelta; if ( rl > 0xFF ) rl = 0xFF;
    int gl = grn + colorDelta; if ( gl > 0xFF ) gl = 0xFF;
    int bl = blu + colorDelta; if ( bl > 0xFF ) bl = 0xFF;
    int lightColor = (rl << 16) + (gl << 8) + bl;
    for( int i = 1; i < frameWidth - 1; i++ )
    {
        DrawHorLine(r.x1 + i, r.y1 + i, r.x2 - i,lightColor );
        DrawVerLine(r.x2 - i, r.y1 + i, r.y2 - i,lightColor );
        DrawHorLine(r.x1 + i, r.y2 - i, r.x2 - i,darkColor );
        DrawVerLine(r.x1 + i, r.y1 + i, r.y2 - i,darkColor );
    }
}
void D3DGraphics::DrawButtonPlusMinus( Rect r,int width )
{
    int ySplit = r.y1 + (r.y2 - r.y1) / 2 + 1;
    Rect rUp = r;
    rUp.y2 = ySplit;
    Rect rDown = r;
    rDown.y1 = ySplit;
    DrawButton( rUp   );
    DrawButton( rDown );
    for( int i = 0; i < width; i++ )
    {
        int x = r.x1 + 5 + i * font->width();
        PrintXY( x,r.y1   + 1,"+" );
        PrintXY( x,ySplit    ,"-" );
    }
}
void D3DGraphics::PaintConsole( int x,int y,EvoConsole *console )
{
    assert( console->columns > 0 );
    assert( console->rows    > 0 );

    char buf[2];
    buf[1] = '\0';

    for (int r = 0; r < console->rows; r++)
        for (int c = 0; c < console->columns; c++)
        {
            buf[0] = console->data[r][c];
            if ( ! buf[0] ) buf[0] = ' ';
            PrintXY( x + c * font->width(),y + r * font->height(),buf );
        }
}
void D3DGraphics::PrintText( int x,int y,const char *text[] )
{
    int iLine = 0;
    for ( int j = y; text[iLine] != nullptr; j += font->height() )
        PrintXY( x,j,text[iLine++] );
}

/*
void D3DGraphics::DrawBlockShadowClipped( Rect d,int sx,int sy,Bitmap &source,int darken )
{
    
}
*/
void D3DGraphics::DrawBallShadowClipped( int centerX,int centerY,int radius,Bitmap &source,int darken,int xOfs,int yOfs )
{
    if ( (centerX > radius) && (centerX < HOR_SCREEN_RES - 1 - radius) &&
         (centerY > radius) && (centerY < VER_SCREEN_RES - 1 - radius) )
    {
        int rSquared = radius * radius;
        int xPivot = (int)(radius * 0.707107f + 0.5f);
        for ( int x = 0; x <= xPivot; x++ )
        {
            int y = (int)(sqrt( (float)(rSquared - x * x) ) + 0.5f);
            int j = centerY + y;
            for ( int i = centerX - x; i <= centerX + x; i++ )
                PutPixel( i,j,Darken( 
                    source.getPixel( 
                        (i + xOfs) % source.getWidth(),
                        (j + yOfs) % source.getHeight()),darken ) );
            j = centerY - y;
            for ( int i = centerX - x; i <= centerX + x; i++ )
                PutPixel( i,j,Darken(
                    source.getPixel(
                        (i + xOfs) % source.getWidth(),
                        (j + yOfs) % source.getHeight() ),darken ) );
                    j = centerY + x;
            for ( int i = centerX - y; i <= centerX + y; i++ )
                PutPixel( i,j,Darken(
                    source.getPixel(
                        (i + xOfs) % source.getWidth(),
                        (j + yOfs) % source.getHeight() ),darken ) );
                    j = centerY - x;
            for ( int i = centerX - y; i <= centerX + y; i++ )
                PutPixel( i,j,Darken(
                    source.getPixel(
                        (i + xOfs) % source.getWidth(),
                        (j + yOfs) % source.getHeight() ),darken ) );
        }
    }
}

