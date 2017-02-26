#pragma once

//#define DEBUG_MODE
//#define GOD_MODE         // invincible if defined, for debugging
#define HOR_SCREEN_RES   1024  // You can try to increase the resolution for fun   
#define VER_SCREEN_RES   768   // however levels created in hires are not compatible with lowres
#define WINDOW_TITLE     (L"Arkanoid by Byteraver - Powered by Chili's directX Framework")
#define _CRT_SECURE_NO_WARNINGS 1 // doesn't work in VS 2015
#pragma warning(disable:4996)     // so we need this

#include <assert.h>

struct Rect 
{ 
    int x1;
    int y1;
    int x2;
    int y2;
    Rect () {}
    Rect ( int x1_,int y1_,int x2_,int y2_ )
    {
        x1 = x1_;
        y1 = y1_;
        x2 = x2_;
        y2 = y2_;
    }
    bool contains( int x,int y ) const
    {
        if ( x < x1 ) return false;
        if ( x > x2 ) return false;
        if ( y < y1 ) return false;
        if ( y > y2 ) return false;
        return true;
    }
    int width() const
    {
        return x2 - x1 + 1;
    }
    int height() const
    {
        return y2 - y1 + 1;
    }
};

void getNrStr( char *dest, int source );


