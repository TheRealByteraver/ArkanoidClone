/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	D3DGraphics.h																		  *
 *	Copyright 2012 PlanetChili <http://www.planetchili.net>								  *
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
#pragma once

#include "globaldefs.h"
#include "font.h"
#include "EvoConsole.h"
#include "loadbmp.h"

#include <d3d9.h>
#include <D3dx9core.h>

#define DEFAULT_FRAME_WIDTH     3
#define DEFAULT_FRAME_COLOR     0xD8B8A8
#define DEFAULT_TEXT_COLOR      0xFFFFFF

class D3DGraphics
{
public:
	D3DGraphics( HWND hWnd );
	~D3DGraphics();
	void    BeginFrame();
	void    EndFrame();
    int     Darken( int src, int darken )
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
	void    PutPixel( int x,int y,int color );
	void    DrawLine( int x1,int y1,int x2,int y2,int color );
    void    DrawHorLine( int x1,int y,int x2,int color );
    void    DrawVerLine( int x,int y1,int y2,int color );
    void    DrawMathLine( int x,int y,float m,int length,int color );
	void    DrawCircle( int cx,int cy,int radius,int color );
	void    DrawDisc( int cx,int cy,int r,int color );
    void    DrawBox( const Rect& coords,int color );
    void    DrawBox( int x1,int y1,int x2,int y2,int color );
    void    DrawBoxClipped( Rect coords,Rect clippedArea,int color );
    void    DrawBlock( Rect coords,int color );
    void    DrawBlock( int x1,int y1,int x2,int y2,int color );
    void    setTextColor( int color ) { textColor = color; }
    void    setFont( Font *f )  { font = f;     }
    Font    *getFont()          { return font;  }
    int     getStrLen( const char *s,Font *font );
    void    PrintXY( int x,int y,const char *s );
    void    PrintXY( int x,int y,const char *s, int opacity );
    void    PrintXY( int x,int y,const char *s,int opacity,Font& font );
    //void    PrintXYSolid( int x,int y,int xSpacer,char *s, HRESULT *hres );
    void    PrintXYSolid( int x,int y,int xSpacer,char *s,int color /*, HRESULT *hres*/ );
    void    PaintConsole( int x,int y,EvoConsole *console );
    void    PaintBMP( int x,int y,Bitmap &bitmap );
    //void    PaintBMPWrapAround( Rect r,Bitmap &bitmap );
    void    PaintFromBMP(int x,int y,Rect area,Bitmap &bitmap);
    void    PaintBMPKey( int x,int y,Bitmap &bitmap, int keyColor );
    void    PaintBMPClearType( int x,int y,Bitmap &bitmap,int keyColor );
    void    PaintBMPClearType( int x,int y,Bitmap &bitmap,int keyColor,int opacity );
    void    DrawNiceBlock( Rect r );
    void    DrawNiceBlockInv( Rect r );
    void    DrawButton( Rect r );
    void    DrawButtonPlusMinus( Rect r,int width );
    void    PrintText( int x,int y,const char *text[] );
    void    DrawBlockShadowClipped( Rect r,Bitmap &source,int darken );
    void    DrawBallShadowClipped( int centerX,int centerY,int radius,Bitmap &source,int darken,int xOfs,int yOfs );
private:
	IDirect3D9*			pDirect3D;
	IDirect3DDevice9*	pDevice;
	IDirect3DSurface9*	pBackBuffer;
	D3DLOCKED_RECT		backRect;
    // User defined variables:
    int                 textColor;
    int                 frameWidth;
    int                 frameColor;
    Font                *font;
};