/*
    This effect is a little too intrusive to be used as a background during
    gameplay but maybe it'll do just fine as menu background?
*/

#pragma once

#include "globaldefs.h"
#include "D3DGraphics.h"

#include <stdlib.h>  
#include <time.h>  

#define NR_OF_BARS          5
#define BAR_WIDTH           10
#define BAR_DISTANCE        ((BAR_WIDTH * 3) / 5)
#define MAX_BRIGHTNESS      255
#define GAMMA_DELTA         (MAX_BRIGHTNESS / NR_OF_BARS)
#define SPEED               0.04f

enum barflydirection { up, down };

class RGBColor
{
public:
    int r;
    int g;
    int b;
    RGBColor()
    {
        r = 0;
        g = 0;
        b = 0;
    }
    int getXRGB()
    {
        return D3DCOLOR_XRGB( r,g,b );    
    }
};

class CopperBar
{
private:  
    D3DGraphics         *gfx;
    int                 frameCounter;
    float               speed[NR_OF_BARS];
    int                 barPositions[NR_OF_BARS];
    barflydirection     flyDirection[NR_OF_BARS];
    RGBColor            color;
    RGBColor            destinationColor;    
public:
    CopperBar() { gfx = nullptr; }
    void    init( D3DGraphics *gfx_ )
    {
        srand ((unsigned int)time(NULL));
        gfx = gfx_;
        frameCounter = 0;
        color.r = 80;
        color.b = 250;
        color.g = 140;
        for( int iBar = 0; iBar < NR_OF_BARS; iBar++ ) 
        {
            barPositions[iBar] = BAR_WIDTH + iBar * BAR_DISTANCE;
            flyDirection[iBar] = up;
            speed[iBar] = 1.0f;
        }
    }
    void    updateFrame()
    {
        frameCounter++;
        if ( (frameCounter % 3) == 0 )
        {
            if ( color.r < destinationColor.r  ) color.r++;
            else if ( color.r > destinationColor.r  ) color.r--;
            if ( color.g < destinationColor.g  ) color.g++;
            else if ( color.g > destinationColor.g  ) color.g--;
            if ( color.b < destinationColor.b  ) color.b++;
            else if ( color.b > destinationColor.b  ) color.b--;
            if ( (color.r == destinationColor.r) && 
                 (color.g == destinationColor.g) && 
                 (color.b == destinationColor.b) ) 
            {
                destinationColor.r = 70 + rand() % 185;
                destinationColor.g = 70 + rand() % 185;
                destinationColor.b = 70 + rand() % 185;
            }
        }
        for( int iBar = 0; iBar < NR_OF_BARS; iBar++ )
        {
            int y = barPositions[iBar];
            if ( flyDirection[iBar] == down )
            {
                if ( y < VER_SCREEN_RES / 2 ) speed[iBar] += SPEED;
                else if (speed[iBar] > (1.0f + SPEED))  speed[iBar] -= SPEED;
                y += (int)(speed[iBar] * speed[iBar]);
                if ( y >= (VER_SCREEN_RES - BAR_WIDTH - 1) )
                {
                    y = VER_SCREEN_RES - BAR_WIDTH - 1;
                    flyDirection[iBar] = up;
                }
            } else {
                if ( y > VER_SCREEN_RES / 2 ) speed[iBar] += SPEED;
                else if (speed[iBar] > (1.0f + SPEED))  speed[iBar] -= SPEED;
                y -= (int)(speed[iBar] * speed[iBar]);
                if ( y <= (BAR_WIDTH - 1) )
                {
                    y = BAR_WIDTH - 1;
                    flyDirection[iBar] = down;
                }
            }
            barPositions[iBar] = y;
        }
        Draw();
    }
private:  
    void                Draw()
    {
        for( int iBar = NR_OF_BARS - 1; iBar >= 0; iBar-- )
        {
            int gamma = ((NR_OF_BARS - iBar) << 8) / NR_OF_BARS;
            RGBColor centerColor;
            centerColor.r = (MAX_BRIGHTNESS * gamma) >> 8;
            centerColor.g = (MAX_BRIGHTNESS * gamma) >> 8;
            centerColor.b = (MAX_BRIGHTNESS * gamma) >> 8;
            RGBColor borderColor = color;
            borderColor.r = (borderColor.r * gamma) >> 8;
            borderColor.g = (borderColor.g * gamma) >> 8;
            borderColor.b = (borderColor.b * gamma) >> 8;
            int rDelta = ((centerColor.r - borderColor.r) << 8) / BAR_WIDTH;
            int gDelta = ((centerColor.g - borderColor.g) << 8) / BAR_WIDTH;
            int bDelta = ((centerColor.b - borderColor.b) << 8) / BAR_WIDTH;
            gfx->DrawHorLine( 0,barPositions[iBar],HOR_SCREEN_RES - 1,centerColor.getXRGB() );
            gfx->DrawHorLine( 0,barPositions[iBar] - (BAR_WIDTH - 1),HOR_SCREEN_RES - 1,0 );
            gfx->DrawHorLine( 0,barPositions[iBar] + (BAR_WIDTH - 1),HOR_SCREEN_RES - 1,0 );
            for( int w = BAR_WIDTH - 2; w > 0; w-- )
            {
                int c = borderColor.getXRGB();
                gfx->DrawHorLine( 0,barPositions[iBar] - w,HOR_SCREEN_RES - 1,c );
                gfx->DrawHorLine( 0,barPositions[iBar] + w,HOR_SCREEN_RES - 1,c );
                borderColor.r = ((borderColor.r << 8) + rDelta) >> 8;
                borderColor.g = ((borderColor.g << 8) + gDelta) >> 8;
                borderColor.b = ((borderColor.b << 8) + bDelta) >> 8;
            }
        }
    }
};