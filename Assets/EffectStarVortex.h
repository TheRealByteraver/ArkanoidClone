/*
    
*/
#pragma once

#include "globaldefs.h"
#include "D3DGraphics.h"

#include <stdlib.h>  
#include <time.h> 
#include <math.h>

#define NR_OF_STARS     300
#define X_CENTER        (HOR_SCREEN_RES / 2)
#define Y_CENTER        (VER_SCREEN_RES / 2)
#define ANGULAR_INC     ( - 0.008f)
#define OUTWARD_INC     1.008f
#ifndef PI
    #define PI          3.141592f
#endif

class Star
{
private:
    float x;
    float y;
    float dist;
    float angle;
public:
    Star()
    {
        x = (float)((rand() % (HOR_SCREEN_RES - 1)) - X_CENTER);
        y = (float)((rand() % (VER_SCREEN_RES - 1)) - Y_CENTER);
        dist = sqrt( x * x + y * y );
        angle = atan( y / x );
        if ( x < 0.0f ) angle += PI; 
    }
    void Move()
    {
        angle += ANGULAR_INC;
        dist *= OUTWARD_INC;
        x = dist * cos( angle );
        y = dist * sin( angle );
        if (( x <= - (X_CENTER - 1) ) ||
            ( x >=   (X_CENTER - 1) ))
        {
            x = (float)((rand() % 200) - 100);
            y = (float)((rand() % 200) - 100);
            dist = sqrt( x * x + y * y );
            angle = atan( y / x );
            if ( x < 0.0f ) angle += PI; 
        }
    }
    int    GetX()       { return (int)x;        }
    int    GetY()       { return (int)y;        }
    int    GetDist()    { return (int)dist;     }
};


class StarVortex 
{
public:
    void    Init( D3DGraphics *gfx_ )
    {
        srand ((unsigned int)time(NULL));
        gfx = gfx_;
    }
    void    UpdateFrame()
    {
        for( int i = 0; i < NR_OF_STARS; i++ )
        {
            int gamma = stars[i].GetDist();
            if ( gamma < 0 ) gamma = - gamma;
            gamma *= 205;
            gamma /= Y_CENTER;
            gamma += 50;
            if ( gamma > 255 ) gamma = 255;
            int x = X_CENTER + stars[i].GetX();
            int y = Y_CENTER + stars[i].GetY();
            if ( (y >= 1) && (y < VER_SCREEN_RES - 1) )
            {
                gfx->PutPixel( x,y,D3DCOLOR_XRGB( gamma,gamma,gamma ) );
                if( stars[i].GetDist() > ((Y_CENTER * 2) / 3) )
                {
                    gfx->PutPixel( x + 1,y,
                                    D3DCOLOR_XRGB( gamma,gamma,gamma ) );
                    gfx->PutPixel( x,y + 1,
                                    D3DCOLOR_XRGB( gamma,gamma,gamma ) );
                    gfx->PutPixel( x + 1,y + 1,
                                    D3DCOLOR_XRGB( gamma,gamma,gamma ) );
                }

            }
            stars[i].Move();
        }
    }
private:
    D3DGraphics     *gfx;
    Star            stars[NR_OF_STARS];
};