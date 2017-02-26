/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	Game.cpp																			  *
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
#include "globaldefs.h"
#include "EffectBar.h"
#include "Game.h"
#include "Arkanoid.h"
#include "EvoConsole.h"
#include "levelDesigner.h"
#include <stdlib.h>
#include <iomanip>

char *mainmenu[] = 
{
    "Start Game", 
    "Choose starting Level",
    "Level Designer",
    "Credits",
    "Quit",
    nullptr
};

char *showcreditsmenu[] =
{
    "Hello There!",
    "",
    "Thank you for trying out this little game.",
    "This game was written after the snake game",
    "and was a whole  lot more  challenging  to",
    "create. Especially the collision detection",
    "was a total  nightmare.  I can't recommend",
    "it really.  I designed it so  that  anyone",
    "can easily make  their own  set of levels.",
    "Many thanks must go to Chilli  for writing",
    "the DirectX framework!  It lowered the bar",
    "enough for me to finally make a game in C.",
    "",
    "Press space to return to the main menu.",
    nullptr
};

char *powerupmenu[] =
{
    "No powerup",
    "Double ball",
    "Triple ball",
    "Ball storm",
    "Golden ball",
    "Guns",
    "bigpad",
    "smallpad",
    nullptr
};

Game::Game( HWND hWnd,const KeyboardServer& kServer,const MouseServer& mServer )
:	gfx( hWnd ),
	audio( hWnd ),
	kbd( kServer ),
	mouse( mServer )
{
#ifdef DEBUG_MODE
    evoConsole.initialize( 80,1 );
#endif
    font.loadFont( SOURCE_FILES_PATH"Courier.TFT" );
    neuropolXBMP.loadFont( SOURCE_FILES_PATH"neuropolX.tft" );
    gfx.setFont( &font );
    levelDesigner.setFont( &font );
    levelDesigner.initBlockLibrary();
    levelDesigner.switchLevelDesignerMode();
    // load default level data into the data required for gameplay:
    levelDesigner.loadInGameLevel( 0 );
    // make sure the ball class has access to the level data:
#ifdef DEBUG_MODE
    arkanoid.Init( &gfx,&levelDesigner.inGameLevel,&powerUpLogic );
    pausegame = false;
#else
    arkanoid.Init( &levelDesigner.inGameLevel,&powerUpLogic );
#endif
    // load Sounds:
    levelDesigner.loadSounds( audio );
    // Initialize background animation for menu's and so on
    copperBar.init( &gfx );
    starVortex.Init( &gfx );
    // for the power up menu:
    puMenuX = 0;
    puMenuY = 0;
    puMenuActive = false;
    puNrOfPowerUps = 0;
    int longestStr = 0;
    for ( ; powerupmenu[puNrOfPowerUps] != nullptr; puNrOfPowerUps++ )
    {
        int s = strlen( powerupmenu[puNrOfPowerUps] );
        if ( s > longestStr ) longestStr = s;
    }
    puMenuHeight = font.height() * (puNrOfPowerUps + 1);
    puMenuWidth = (longestStr + 2) * font.width();
    // Miscellaneous initializations:
    frameNr             = 0;
    seconds             = 0; 
    gameState           = gamemenu;
    gameLevel           = 0;
    levelDesigner.setMaxStartLevel( 11 ); // 11 for debugging
}

Game::~Game()
{
}

void Game::Go()
{
	gfx.BeginFrame();
	ComposeFrame();
	gfx.EndFrame();
}

enum colordirection { increasing, decreasing, min, max };

// the function below is not needed in the actual game and was only made
// to draw block library bitmaps
void Game::DrawBlockLibrary()
// red .. yellow .. green .. cyan .. blue .. magenta .. red 
// definitions below are only here to draw blocks from within the code
#define COLOR_STEPS             6
#define BLOCK_COLUMNS           12 
#define BLOCK_ROWS              12
#define TBLOCK_WIDTH            (HOR_SCREEN_RES / (BLOCK_COLUMNS + 2) - 2 * SPACER) 
#define TBLOCK_HEIGHT           ((VER_SCREEN_RES / 2) / BLOCK_ROWS    - 2 * SPACER)
#define COLOR_INTERVAL          ((BLOCK_COLUMNS / COLOR_STEPS) + 1)
#define COLOR_OFFSET            16
#define COLOR_MAX               255
#define COLOR_RANGE             (COLOR_MAX - COLOR_OFFSET)
#define ATTENUATION_INTERVAL    (COLOR_RANGE / BLOCK_ROWS + 1)
#define BORDER_WIDTH            1

{
    const colordirection R[] = { max, decreasing, min, min, increasing, max, max, decreasing };
    const colordirection G[] = { increasing, max, max, decreasing, min, min, increasing, max };
    const colordirection B[] = { min, min, increasing, max, max, decreasing, min, min };
    int cellNr = 0;
    for (int y = 0; y < BLOCK_ROWS; y++ )
    {
        for (int x = 0; x < (BLOCK_COLUMNS + 2); x++ )
        {
            int r;
            int g;
            int b;            
            int iDir = x / ((BLOCK_COLUMNS / COLOR_STEPS) + 1);
            int iOfs = x % ((BLOCK_COLUMNS / COLOR_STEPS) + 1);
            colordirection redDir = R[iDir];
            colordirection grnDir = G[iDir];
            colordirection bluDir = B[iDir];
            switch( redDir )
            {
                case increasing:
                { r = COLOR_OFFSET + (iOfs * COLOR_RANGE) / COLOR_INTERVAL; break; }
                case decreasing:
                { r = COLOR_MAX    - (iOfs * COLOR_RANGE) / COLOR_INTERVAL; break; }
                case min:
                { r = COLOR_OFFSET; break; }
                case max:
                { r = COLOR_MAX; break; }
            }
            switch( grnDir )
                {
                case increasing:
                { g = COLOR_OFFSET + (iOfs * COLOR_RANGE) / COLOR_INTERVAL; break; }
                case decreasing:
                { g = COLOR_MAX    - (iOfs * COLOR_RANGE) / COLOR_INTERVAL; break; }
                case min:
                { g = COLOR_OFFSET; break; }
                case max:
                { g = COLOR_MAX; break; }
            }
            switch( bluDir )
                {
                case increasing:
                { b = COLOR_OFFSET + (iOfs * COLOR_RANGE) / COLOR_INTERVAL; break; }
                case decreasing:
                { b = COLOR_MAX    - (iOfs * COLOR_RANGE) / COLOR_INTERVAL; break; }
                case min:
                { b = COLOR_OFFSET; break; }
                case max:
                { b = COLOR_MAX; break; }
            }
            r = COLOR_OFFSET + ((r - COLOR_OFFSET) * (BLOCK_ROWS - 1 - y)) / BLOCK_ROWS;
            g = COLOR_OFFSET + ((g - COLOR_OFFSET) * (BLOCK_ROWS - 1 - y)) / BLOCK_ROWS;
            b = COLOR_OFFSET + ((b - COLOR_OFFSET) * (BLOCK_ROWS - 1 - y)) / BLOCK_ROWS;
            int color = (r << 16) + (g << 8) + b;            
            int x1 = SPACER + x * (TBLOCK_WIDTH  + SPACER * 2);
            int y1 = SPACER + y * (TBLOCK_HEIGHT + SPACER * 2);
            int x2 = x1 + TBLOCK_WIDTH  - 1;
            int y2 = y1 + TBLOCK_HEIGHT - 1;
            int delta = 60;
            r += delta; if ( r > 255 ) r = 255;
            g += delta; if ( g > 255 ) g = 255;
            b += delta; if ( b > 255 ) b = 255;
            int lightColor = D3DCOLOR_XRGB( r,g,b );
            int i = 0;
            for( i = 0; i < BORDER_WIDTH; i++ )
            {
                gfx.DrawLine ( x1 + i,y1 + i,x2 - i,y1 + i,lightColor );
                gfx.DrawLine ( x2 - i,y1 + i,x2 - i,y2 - 1 - i,lightColor );
            }
            r = (color & 0x00FF0000) >> 16;
            g = (color & 0x0000FF00) >> 8;
            b = (color & 0x000000FF);
            r -= delta; if ( r < 0 ) r = 0;
            g -= delta; if ( g < 0 ) g = 0;
            b -= delta; if ( b < 0 ) b = 1; // full black is not possible!
            int darkColor = D3DCOLOR_XRGB( r,g,b );
            i = 0;
            for( i = 0; i < BORDER_WIDTH; i++ )
            {
                gfx.DrawLine ( x1 + i,y1 + 1 + i,x1 + i,y2 - i,darkColor );
                gfx.DrawLine ( x1 + 1 + i,y2 - i,x2 - i,y2 - i,darkColor );
            }
            gfx.DrawBlock( x1 + i,y1 + i,x2 - i,y2 - i,color );
            cellNr++;
        }
    }
}

void Game::DrawPlayField()
{
    Rect paddle = arkanoid.getPaddleCoords();
#ifndef DEBUG_MODE
    // for the scroll function:
    if ( levelDesigner.inGameLevel.backGroundScrolltype != staticbackground )
    {
        switch ( levelDesigner.inGameLevel.backGroundScrolltype )
        {
            case scrollup:
            {
                if ( frameNr & 0x1 ) bgScrollY -= bgScrollVerSpeed;
                break;
            }
            case scrolldown:
            {
                if ( frameNr & 0x1 ) bgScrollY += bgScrollVerSpeed;
                break;
            }
            case scrollleft:
            {
                if ( frameNr & 0x1 ) bgScrollX -= bgScrollHorSpeed;
                break;
            }
            case scrollright:
            {
                if ( frameNr & 0x1 ) bgScrollX += bgScrollHorSpeed;
                break;
            }
            case randomscroll:
            {
                if ( ((seconds % 10) == 0) && (frameNr == 0) )
                {
                    if ( rand() & 1 ) bgScrollHorSpeed = 3;
                    else              bgScrollHorSpeed = 0;
                    if ( (rand() & 1) || (bgScrollHorSpeed == 0) ) 
                                                        bgScrollVerSpeed = 3;
                    else              bgScrollVerSpeed = 0;
                    if ( rand() & 1 ) bgScrollHorSpeed = -bgScrollHorSpeed;
                    if ( rand() & 1 ) bgScrollVerSpeed = -bgScrollVerSpeed;
                }
                if ( frameNr & 0x1 )
                {
                    bgScrollX += bgScrollHorSpeed;
                    bgScrollY += bgScrollVerSpeed;
                }
                break;
            }
        }
        if ( bgScrollX < 0 ) bgScrollX += levelDesigner.inGameLevel.backgroundBMP->getWidth();
        else
            if ( bgScrollX > levelDesigner.inGameLevel.backgroundBMP->getWidth() )
                bgScrollX = bgScrollX % levelDesigner.inGameLevel.backgroundBMP->getWidth();
        if ( bgScrollY < 0 ) bgScrollY += levelDesigner.inGameLevel.backgroundBMP->getHeight();
        else
            if ( bgScrollY > levelDesigner.inGameLevel.backgroundBMP->getHeight() )
                bgScrollY = bgScrollY % levelDesigner.inGameLevel.backgroundBMP->getHeight();
    }
    // Draw Background
    Bitmap& bitmap = *levelDesigner.inGameLevel.backgroundBMP;
    if ( bitmap.isImagePresent() ) // no shadows if no background is present
    {
        for ( int j = 0; j < VER_SCREEN_RES / bitmap.getHeight() + 2; j++ )
            for ( int i = 0; i < HOR_SCREEN_RES / bitmap.getWidth() + 2; i++ )
                gfx.PaintBMP( -bgScrollX + i * bitmap.getWidth(),
                    -bgScrollY + j * bitmap.getHeight(),bitmap );
        /*
            Shadow Drawing logic
        */
        // some constants used for the shadows:
        const int sxOffset = -10;
        const int syOffset = 10;
        const int darken = 40;
        // Draw Block Shadows: start at 5 so we skip the outer walls & paddle
        Bitmap& source = *levelDesigner.inGameLevel.backgroundBMP;
        for (   int blockNr = SPECIAL_BLOCKS; 
                blockNr < levelDesigner.inGameLevel.nrOfBlocks; blockNr++ )
            if ( levelDesigner.inGameLevel.blocks[blockNr].isAlive )
            {
                Rect b( levelDesigner.inGameLevel.blocks[blockNr].xOrig,
                    levelDesigner.inGameLevel.blocks[blockNr].yOrig,
                    levelDesigner.inGameLevel.blocks[blockNr].x2,
                    levelDesigner.inGameLevel.blocks[blockNr].y2 );
                Rect s( b.x1 + sxOffset,b.y1 + syOffset,
                    b.x2 + sxOffset,b.y2 + syOffset );
                // works only if shadow is bottom left from the block:
                if ( s.x1 < 0 ) s.x1 = 0;
                if ( s.x2 >= HOR_SCREEN_RES ) s.x2 = HOR_SCREEN_RES - 1;
                if ( s.y1 < 0 ) s.y1 = 0;
                if ( s.y2 >= VER_SCREEN_RES ) s.y2 = VER_SCREEN_RES - 1;
                int y = s.y1 + bgScrollY;
                for ( int j = s.y1; j <= s.y2; j++ )
                {
                    if ( y >= source.getHeight() ) y %= source.getHeight();
                    int x = s.x1 + bgScrollX;
                    for ( int i = s.x1; i <= b.x1; i++ )
                    {
                        if ( x >= source.getWidth() ) x %= source.getWidth();
                        gfx.PutPixel( i,j,gfx.Darken( source.getPixel( x,y ),darken ) );
                        x++;
                    }
                    y++;
                }
                y = b.y2 + bgScrollY;
                for ( int j = b.y2; j <= s.y2; j++ )
                {
                    if ( y >= source.getHeight() ) y %= source.getHeight();
                    int x = b.x1 + bgScrollX;
                    for ( int i = b.x1; i <= s.x2; i++ )
                    {
                        if ( x >= source.getWidth() ) x %= source.getWidth();
                        gfx.PutPixel( i,j,gfx.Darken( source.getPixel( x,y ),darken ) );
                        x++;
                    }
                    y++;
                }
            }
        // Draw shadows of the balls:
        for ( int iBall = 0; iBall < MAX_BALLS; iBall++ )
            if ( arkanoid.getBall( iBall ).isActive )
            {
                gfx.DrawBallShadowClipped(
                    (int)arkanoid.getBall( iBall ).ballX + sxOffset,
                    (int)arkanoid.getBall( iBall ).ballY + syOffset,
                    (int)arkanoid.getBall( iBall ).radius - 1,source,darken,bgScrollX,bgScrollY );
            }
        // Draw shadow of the paddle:
        for ( int j = paddle.y1 + syOffset; j <= paddle.y2 + syOffset; j++ )
            for ( int i = paddle.x1 + sxOffset; i <= paddle.x2 + sxOffset; i++ )
                if ( i > 0 ) // && j < VER_SCREEN_RES - 1
                    gfx.PutPixel( i,j,gfx.Darken( source.getPixel(
                    (i + bgScrollX) % levelDesigner.inGameLevel.backgroundBMP->getWidth(),
                        (j + bgScrollY) % levelDesigner.inGameLevel.backgroundBMP->getHeight() ),darken ) );
        gfx.DrawBallShadowClipped(
            paddle.x1 + sxOffset,
            arkanoid.getPaddleYCenter() + syOffset,
            PADDLE_HEIGHT / 2,source,darken,bgScrollX,bgScrollY );
        gfx.DrawBallShadowClipped(
            paddle.x2 + sxOffset,
            arkanoid.getPaddleYCenter() + syOffset,
            PADDLE_HEIGHT / 2,source,darken,bgScrollX,bgScrollY );
        /*
            End of shadow Drawing logic
        */
    }
    // Draw PowerUps:
    char pwrUpStr[2];
    pwrUpStr[1] = '\0';
    for ( int pwrUp = 0; pwrUp < MAX_POWER_UPS; pwrUp++ )
    {
        PowerUp& powerUp = powerUpLogic.powerUps[pwrUp];
        if ( ! powerUp.isAlive ) continue;
        pwrUpStr[0] = powerUp.symbol;
        const int r = 3;
        Rect puExt( 
            powerUp.x,
            powerUp.y,
            powerUp.x + POWERUP_WIDTH - 1,
            powerUp.y + POWERUP_HEIGHT - 1 );
        Rect puInt(
            powerUp.x + r,
            powerUp.y + r,
            powerUp.x + POWERUP_WIDTH - 1 - r,
            powerUp.y + POWERUP_HEIGHT - 1 - r );
        int color = powerUp.color;
        gfx.DrawDisc( puInt.x1,puInt.y1,r,color );
        gfx.DrawDisc( puInt.x2,puInt.y1,r,color );
        gfx.DrawDisc( puInt.x1,puInt.y2,r,color );
        gfx.DrawDisc( puInt.x2,puInt.y2,r,color );
        for ( int i = 0; i < r * 2 - 1; i++ )
        {
            gfx.DrawHorLine( puInt.x1,puInt.y1 - i,puInt.x2,color );
            gfx.DrawHorLine( puInt.x1,puInt.y2 + i,puInt.x2,color );
            gfx.DrawVerLine( puInt.x1 - i,puInt.y1,puInt.y2,color );
            gfx.DrawVerLine( puInt.x2 + i,puInt.y1,puInt.y2,color );
            color = gfx.Darken( color,20 );
        }
        gfx.PrintXY(
            powerUp.x + (POWERUP_WIDTH  - font.width() ) / 2,
            powerUp.y + (POWERUP_HEIGHT - font.height()) / 2,
            pwrUpStr );
    }
    // Draw Blocks:
    for ( int blockNr = SPECIAL_BLOCKS; blockNr < levelDesigner.inGameLevel.nrOfBlocks; blockNr++ )
        if ( levelDesigner.inGameLevel.blocks[blockNr].isAlive )
            gfx.PaintBMP(
                levelDesigner.inGameLevel.blocks[blockNr].xOrig,
                levelDesigner.inGameLevel.blocks[blockNr].yOrig,
               *levelDesigner.inGameLevel.blocks[blockNr].skin );
    // Draw Bullets:
    for ( int b = 0;b < MAX_BULLETS;b++ )
    {
        if ( arkanoid.isBulletAlive( b ) )
        {
            const int f = 4;
            int x;
            int y;
            arkanoid.getBulletXY( b,x,y );
            gfx.DrawVerLine( x    ,y    ,y + BULLET_HEIGHT - 1 - f,BULLET_COLOR );
            gfx.DrawVerLine( x - 1,y + f,y + BULLET_HEIGHT - 1    ,BULLET_COLOR );
            gfx.DrawVerLine( x + 1,y + f,y + BULLET_HEIGHT - 1    ,BULLET_COLOR );
        }
    }
    // Draw Balls:
    for ( int iBall = 0; iBall < MAX_BALLS; iBall++ )
        if( arkanoid.getBall( iBall ).isActive )
        {
            int bx = (int)arkanoid.getBall( iBall ).ballX;
            int by = (int)arkanoid.getBall( iBall ).ballY;
            int br = (int)arkanoid.getBall( iBall ).radius - 1;
            // safety net:
            if ( bx < br ) bx = br; 
            if ( bx >= HOR_SCREEN_RES - br ) bx = HOR_SCREEN_RES - 1 - br;
            if ( by < br ) by = br;
            if ( by >= VER_SCREEN_RES - br ) by = VER_SCREEN_RES - 1 - br;
            gfx.DrawDisc( bx,by,br,arkanoid.getBall( iBall ).color );
            // gfx.DrawCircle( bx,by,br,0x0 );  // add some contrast
        }
    // Draw Paddle  
    gfx.DrawBlock( paddle,PADDLE_COLOR );
    gfx.DrawDisc( paddle.x1,arkanoid.getPaddleYCenter(),PADDLE_HEIGHT / 2,PADDLE_COLOR );
    gfx.DrawDisc( paddle.x2,arkanoid.getPaddleYCenter(),PADDLE_HEIGHT / 2,PADDLE_COLOR );
    if ( arkanoid.isGunPad() )
    {
        gfx.DrawVerLine( paddle.x1    ,paddle.y1 + 6,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x1 + 1,paddle.y1 + 4,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x1 + 2,paddle.y1 + 2,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x1 + 3,paddle.y1 + 0,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x1 + 4,paddle.y1 + 2,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x1 + 5,paddle.y1 + 4,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x1 + 6,paddle.y1 + 6,paddle.y2,PADDLE_GUN_COLOR );

        gfx.DrawVerLine( paddle.x2    ,paddle.y1 + 6,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x2 - 1,paddle.y1 + 4,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x2 - 2,paddle.y1 + 2,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x2 - 3,paddle.y1 + 0,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x2 - 4,paddle.y1 + 2,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x2 - 5,paddle.y1 + 4,paddle.y2,PADDLE_GUN_COLOR );
        gfx.DrawVerLine( paddle.x2 - 6,paddle.y1 + 6,paddle.y2,PADDLE_GUN_COLOR );
    }
    // Draw lives left
    for ( int l = 0; l < livesLeft; l++ )
        gfx.PrintXY( 
            l * 52,
            VER_SCREEN_RES - 1 - neuropolXBMP.height() + 3,
            "_",72,neuropolXBMP );
#else
    // Draw PowerUps:
    char pwrUpStr[2];
    pwrUpStr[1] = '\0';
    for ( int pwrUp = 0; pwrUp < MAX_POWER_UPS; pwrUp++ )
    {
        PowerUp& powerUp = powerUpLogic.powerUps[pwrUp];
        if ( !powerUp.isAlive ) continue;
        pwrUpStr[0] = powerUp.symbol;
        const int r = 3;
        Rect puExt(
            powerUp.x,
            powerUp.y,
            powerUp.x + POWERUP_WIDTH - 1,
            powerUp.y + POWERUP_HEIGHT - 1 );
        Rect puInt(
            powerUp.x + r,
            powerUp.y + r,
            powerUp.x + POWERUP_WIDTH - 1 - r,
            powerUp.y + POWERUP_HEIGHT - 1 - r );
        int color = powerUp.color;
        gfx.DrawDisc( puInt.x1,puInt.y1,r,color );
        gfx.DrawDisc( puInt.x2,puInt.y1,r,color );
        gfx.DrawDisc( puInt.x1,puInt.y2,r,color );
        gfx.DrawDisc( puInt.x2,puInt.y2,r,color );
        for ( int i = 0; i < r * 2 - 1; i++ )
        {
            gfx.DrawHorLine( puInt.x1,puInt.y1 - i,puInt.x2,color );
            gfx.DrawHorLine( puInt.x1,puInt.y2 + i,puInt.x2,color );
            gfx.DrawVerLine( puInt.x1 - i,puInt.y1,puInt.y2,color );
            gfx.DrawVerLine( puInt.x2 + i,puInt.y1,puInt.y2,color );
            color = gfx.Darken( color,20 );
        }
        gfx.PrintXY(
            powerUp.x + (POWERUP_WIDTH - font.width()) / 2,
            powerUp.y + (POWERUP_HEIGHT - font.height()) / 2,
            pwrUpStr );
    }
    // Draw Blocks
    for ( int blockNr = 0; blockNr < levelDesigner.inGameLevel.nrOfBlocks; blockNr++ )
        if ( levelDesigner.inGameLevel.blocks[blockNr].isAlive &&
            (levelDesigner.inGameLevel.blocks[blockNr].skin->isImagePresent()) )
        {
            gfx.DrawBox(
                levelDesigner.inGameLevel.blocks[blockNr].xOrig,
                levelDesigner.inGameLevel.blocks[blockNr].yOrig,
                levelDesigner.inGameLevel.blocks[blockNr].x2,
                levelDesigner.inGameLevel.blocks[blockNr].y2,
                (blockNr << 16) + ((0xFFF / (blockNr + 1)) << 8) + (blockNr * 2) );
            char buf[4];
            _itoa( blockNr,buf,10 );
            buf[3] = '\0';
            gfx.PrintXY(
                levelDesigner.inGameLevel.blocks[blockNr].xOrig,
                levelDesigner.inGameLevel.blocks[blockNr].yOrig,
                buf );
        }
    /*
    gfx.DrawBox( ball.cGrid.x1,ball.cGrid.y1,ball.cGrid.x2,ball.cGrid.y2,0xFFFF );
    gfx.DrawCircle(
        (int)(ball.ballX),
        (int)(ball.ballY),
        ball.radius,
        COLOR_GRAY );
    // Draw Ball at code location whilst debugging:
    gfx.DrawCircle(
        (int)(ball.ballX + ball.horSpeed),
        (int)(ball.ballY + ball.verSpeed),
        ball.radius,
        COLOR_YELLOW );
    */
    // Draw Balls:
    for ( int iBall = 0; iBall < MAX_BALLS; iBall++ )
    {
        int bx = (int)arkanoid.getBall( iBall ).ballX;
        int by = (int)arkanoid.getBall( iBall ).ballY;
        int br = (int)arkanoid.getBall( iBall ).radius - 1;
        gfx.DrawCircle( bx,by,br,0xFF0000 );
        // safety net:
        if ( bx < br ) 
        { 
            bx = br; 
            arkanoid.getBall( iBall ).freeze = true; 
        }
        if ( bx >= HOR_SCREEN_RES - br ) 
        { 
            bx = HOR_SCREEN_RES - 1 - br; 
            arkanoid.getBall( iBall ).freeze = true; 
        }
        if ( by < br ) 
        {
            by = br; 
            arkanoid.getBall( iBall ).freeze = true;
        }
        if ( by >= VER_SCREEN_RES - br ) 
        {
            by = VER_SCREEN_RES - 1 - br; 
            arkanoid.getBall( iBall ).freeze = true;
        }
        gfx.DrawCircle( bx,by,br,0xFF40FF );
    }     
    // Draw Paddle  
    /*
    gfx.DrawBox( paddle.x1,paddle.y1,paddle.x2,paddle.y2,COLOR_WHITE );
    gfx.DrawCircle( paddle.x1,paddle.yCenter,PADDLE_HEIGHT / 2,COLOR_WHITE );
    gfx.DrawCircle( paddle.x2,paddle.yCenter,PADDLE_HEIGHT / 2,COLOR_WHITE );
    */
    gfx.DrawBlock( paddle,PADDLE_COLOR );
    gfx.DrawDisc( paddle.x1,arkanoid.getPaddleYCenter(),PADDLE_HEIGHT / 2,PADDLE_COLOR );
    gfx.DrawDisc( paddle.x2,arkanoid.getPaddleYCenter(),PADDLE_HEIGHT / 2,PADDLE_COLOR );
    //gfx.DrawMathLine( (int)ball2.ballX,(int)ball2.ballY,ball2.m1,100,0xFF00FF );
    //gfx.DrawMathLine(mouse.GetMouseX(),mouse.GetMouseY(),ball2.m1,100,0xFF00FF);
    /*
    std::stringstream mouseInfo;
    mouseInfo << "Mouse x: " << mouse.GetMouseX() << ", Mouse y: " << mouse.GetMouseY();
    gfx.PrintXY( 0,VER_SCREEN_RES - 1 - font.height(),mouseInfo.str().c_str() );
    */
    std::stringstream debugText;
    debugText << "HorSpeed: ";
    debugText << std::setw( 13 ) << arkanoid.getBall( 0 ).horSpeed;
    debugText << ", VerSpeed: ";
    debugText << std::setw( 13 ) << arkanoid.getBall( 0 ).verSpeed;
    debugText << ", xPos: ";
    debugText << std::setw( 13 ) << arkanoid.getBall( 0 ).ballX;
    debugText << ", yPos: ";
    debugText << std::setw( 13 ) << arkanoid.getBall( 0 ).ballY;
    //debugText << ", Speed: ";
    //debugText << std::setw( 10 ) << sqrt( ball.horSpeed * ball.horSpeed + ball.verSpeed * ball.verSpeed );
    debugText << ", m1 = ";
    debugText << std::setw( 12 ) << arkanoid.getBall( 0 ).m1;
    //gfx.PrintXY( 0,VER_SCREEN_RES - 1 - font.height(),debugText.str().c_str() );
    /*
    evoConsole.ScrollUp();
    evoConsole.Print( debugText.str().c_str() );
    gfx.PaintConsole( 1,1,&evoConsole );
    */
#endif
}

void Game::DrawGrid()
{
    if( levelDesigner.isGridVisible() )
    {
        for ( int x = 1; x < levelDesigner.getGridColumns(); x++ )
            for( int i = 0; i < levelDesigner.getGridSpacer(); i++ )
            {
                gfx.DrawVerLine( 
                    gameWindowRect.x1 + levelDesigner.getGridXOffset() 
                    + x * levelDesigner.getGridCellWidth() + i,
                    gameWindowRect.y1 + levelDesigner.getGridYOffset(),
                    levelDesigner.getGridMaxY(),
                    LEVEL_DESIGN_GRID_COLOR );
                gfx.DrawVerLine( 
                    gameWindowRect.x1 + levelDesigner.getGridXOffset() 
                    + x * levelDesigner.getGridCellWidth() - i - 1,
                    gameWindowRect.y1 + levelDesigner.getGridYOffset(),
                    levelDesigner.getGridMaxY(),
                    LEVEL_DESIGN_GRID_COLOR );
            }
        for ( int y = 1; y < levelDesigner.getGridRows(); y++ )
            for( int j = 0; j < levelDesigner.getGridSpacer(); j++ )
            {
                gfx.DrawHorLine( 
                    gameWindowRect.x1 + levelDesigner.getGridXOffset(),
                    gameWindowRect.y1 + levelDesigner.getGridYOffset() 
                    + y * levelDesigner.getGridCellHeight() + j,
                    levelDesigner.getGridMaxX(),
                    LEVEL_DESIGN_GRID_COLOR );
                gfx.DrawHorLine( 
                    gameWindowRect.x1 + levelDesigner.getGridXOffset(),
                    gameWindowRect.y1 + levelDesigner.getGridYOffset() 
                    + y * levelDesigner.getGridCellHeight() - j - 1,
                    levelDesigner.getGridMaxX(),
                    LEVEL_DESIGN_GRID_COLOR );
            }
    }
}

// Draw animated scroll type:
void Game::DrawScrollLogo()
{
    const int bw = 15; // should be a divisor of 60
    int xOrig;
    int yOrig;
    switch ( levelDesigner.getBackgroundScrollType() )
    {
        case scrollup:
        {
            xOrig = 0;
            yOrig = -frameNr % (bw * 2);
            break;
        }
        case scrolldown:
        {
            xOrig = 0;
            yOrig = frameNr % (bw * 2);
            break;
        }
        case scrollleft:
        {
            xOrig = -frameNr % (bw * 2);
            yOrig = 0;
            break;
        }
        case scrollright:
        {
            xOrig = frameNr % (bw * 2);
            yOrig = 0;
            break;
        }
        case randomscroll:
        {
            xOrig = frameNr % bw;
            yOrig = frameNr % bw;
            break;
        }
        default://case staticbackground:
        {
            xOrig = 0;
            yOrig = 0;
            break;
        }
    }
    for ( int j = 0; j < backgroundNrRect.height() / bw + 5; j++ )
        for ( int i = 0; i < backgroundNrRect.width() / bw + 5; i++ )
        {
            int x1 = backgroundNrRect.x1 + i * bw - bw * 2 + xOrig;
            int x2 = x1 + bw;
            int y1 = backgroundNrRect.y1 + j * bw - bw * 2 + yOrig;
            int y2 = y1 + bw;
            if ( x1 > backgroundNrRect.x2 - 2 || y1 > backgroundNrRect.y2 - 2 ||
                 x2 < backgroundNrRect.x1 + 2 || y2 < backgroundNrRect.y1 + 2 )
                continue;
            if ( x1 < backgroundNrRect.x1 + 2 ) x1 = backgroundNrRect.x1 + 2;
            if ( y1 < backgroundNrRect.y1 + 2 ) y1 = backgroundNrRect.y1 + 2;
            if ( x2 > backgroundNrRect.x2 - 2 ) x2 = backgroundNrRect.x2 - 2;
            if ( y2 > backgroundNrRect.y2 - 2 ) y2 = backgroundNrRect.y2 - 2;
            if ( ((j & 0x1) && ((i & 0x1) == 0)) ||
                (((j & 0x1) == 0) && (i & 0x1)) )
            {
                gfx.DrawBlock( Rect( x1,y1,x2,y2 ),DEFAULT_FRAME_COLOR );
            }
        }
/*
    for ( int j = 0; j < backgroundNrRect.height() / bw + 5; j++ )
        for ( int i = 0; i < backgroundNrRect.width() / bw + 5; i++ )
        {
            int x1 = backgroundNrRect.x1 + i * bw - bw * 2 + xOrig;
            int x2 = x1 + bw;
            int y1 = backgroundNrRect.y1 + j * bw - bw * 2 + yOrig;
            int y2 = y1 + bw;
            if ( x1 > backgroundNrRect.x2 || y1 > backgroundNrRect.y2 ||
                x2 < backgroundNrRect.x1 || y2 < backgroundNrRect.y1 )
                continue;
            if ( x1 < backgroundNrRect.x1 ) x1 = backgroundNrRect.x1;
            if ( y1 < backgroundNrRect.y1 ) y1 = backgroundNrRect.y1;
            if ( x2 > backgroundNrRect.x2 ) x2 = backgroundNrRect.x2;
            if ( y2 > backgroundNrRect.y2 ) y2 = backgroundNrRect.y2;
            if ( ((j & 0x1) && ((i & 0x1) == 0)) ||
                (((j & 0x1) == 0) && (i & 0x1)) )
            {
                gfx.DrawBlock( Rect( x1,y1,x2,y2 ),0xA0A000 );
            }
        }
*/
}

void Game::DrawPowerUpMenu()
{
    gfx.DrawNiceBlock( puMenuRect );
    bool mouseOnMenu = false;
    int mx = mouse.GetMouseX();
    int my = mouse.GetMouseY();
    int highLightNr = -1;
    if ( mx > puMenuRect.x1 && mx < puMenuRect.x2 &&
         my > puMenuRect.y1 && my < puMenuRect.y2 )
    {
        mouseOnMenu = true;
        highLightNr = (mouse.GetMouseY() - puMenuRect.y1 - font.height() / 2) / font.height();
        if ( highLightNr >= puNrOfPowerUps ) highLightNr = puNrOfPowerUps - 1;
        int y = puMenuRect.y1 + font.height() / 2 + highLightNr * font.height();
        gfx.DrawBlock( puMenuRect.x1 + 2,y,puMenuRect.x2 - 2,y + font.height() - 1,0xA0A0A0 );
    }
    for ( int i = 0; i < puNrOfPowerUps; i++ )
    {
        gfx.PrintXY(
            puMenuX + font.width(),
            puMenuY + font.height() / 2 + font.height() * i,
            powerupmenu[i] );
    }
    if ( mouseOnMenu && leftButtonPressed )
    {
        powerup& p = levelDesigner.getBlockList()[puNrOfBlock].powerUp;
        switch ( highLightNr )
        {
            case POWERUP_DOUBLEBALL:
            { p = doubleball; break; }
            case POWERUP_MULTIBALL:
            { p = multiball; break; }
            case POWERUP_BALLSTORM:
            { p = ballstorm; break; }
            case POWERUP_GOLDENBALL:
            { p = goldenball; break; }
            case POWERUP_GUNS:
            { p = guns; break; }
            case POWERUP_BIGPAD:
            { p = bigpad; break; }
            case POWERUP_SMALLPAD:
            { p = smallpad; break; }
            default:
            { p = nopowerup; break; }
        } 
        puMenuActive = false;
    }
}

void Game::DrawBlockDesigner()
{
    /*
    Draw Level first:
    */
    gfx.PaintBMP( 
        levelDesigner.getBackGroundX(),
        levelDesigner.getBackGroundY(),
        levelDesigner.getBackground() );
    DrawGrid();
    for ( int blockNr = 0; blockNr < MAX_BLOCKS_PER_LEVEL; blockNr++ )
        if ( levelDesigner.getBlockList()[blockNr].xOrig >= 0 )
            gfx.PaintBMP(   
                levelDesigner.getBlockList()[blockNr].xOrig,
                levelDesigner.getBlockList()[blockNr].yOrig,
                levelDesigner.getBlock( levelDesigner.getBlockList()[blockNr].blockNr ).skin );
    /*
    Now paint the block designers' control panel. First the static content:
    */
    gfx.DrawNiceBlock( panelRect );
    gfx.DrawNiceBlockInv( levelNrRect );
    gfx.DrawNiceBlockInv( levelTitleRect );
    gfx.DrawNiceBlockInv( backgroundNrRect );
    gfx.DrawNiceBlockInv( blockNrRect );
    gfx.DrawNiceBlockInv( colorRRect );
    gfx.DrawNiceBlockInv( colorGRect );
    gfx.DrawNiceBlockInv( colorBRect );
    gfx.DrawNiceBlockInv( colorSampleRect );
    gfx.DrawNiceBlockInv( skinLibraryRect );
    gfx.DrawNiceBlockInv( skinPreviewRect ); 
    gfx.DrawNiceBlockInv( helpWindowRect ); 
    gfx.DrawNiceBlockInv( blockPreviewRect );
    gfx.DrawButton( blockSkinOrColorRect );
    gfx.DrawButton( hitsToDestructRect );
    gfx.DrawButton( toLevelEditorSwitchRect );
    gfx.DrawButtonPlusMinus( levelSelectorRect,1 );
    gfx.DrawButtonPlusMinus( backgroundSelectorRect,1 );
    gfx.DrawButtonPlusMinus( blockNrSelectorRect,1 );
    gfx.DrawButtonPlusMinus( blockNrFastSelectorRect,2 );
    gfx.DrawButtonPlusMinus( colorRSelectorRect,1 );
    gfx.DrawButtonPlusMinus( colorGSelectorRect,1 );
    gfx.DrawButtonPlusMinus( colorBSelectorRect,1 );
    gfx.DrawButtonPlusMinus( skinLibrarySelectorRect,1 );
    DrawScrollLogo(); // draw it BEFORE we write on top of it ;)
    gfx.PrintXY( levelNrRect.x1 + TEXT_SPACER,levelNrRect.y1 + FONT_Y_OFFSET,"Level " );
    gfx.PrintXY( backgroundNrRect.x1 + TEXT_SPACER,backgroundNrRect.y1 + FONT_Y_OFFSET,"Background " );
    gfx.PrintXY( blockNrRect.x1 + TEXT_SPACER,blockNrRect.y1 + FONT_Y_OFFSET,"Block " );
    gfx.PrintXY( colorRRect.x1 + TEXT_SPACER,colorRRect.y1 + FONT_Y_OFFSET,"R:" );
    gfx.PrintXY( colorGRect.x1 + TEXT_SPACER,colorGRect.y1 + FONT_Y_OFFSET,"G:" );
    gfx.PrintXY( colorBRect.x1 + TEXT_SPACER,colorBRect.y1 + FONT_Y_OFFSET,"B:" );
    gfx.PrintXY( skinLibraryRect.x1 + TEXT_SPACER,skinLibraryRect.y1 + FONT_Y_OFFSET,"Skin Library" );
    gfx.PrintXY( toLevelEditorSwitchRect.x1 + TEXT_SPACER,
        toLevelEditorSwitchRect.y1 + FONT_Y_OFFSET,"[Block Designer]" );
    /*
    And now the dynamic content:
    */
    char buf[128];
    getNrStr( buf,levelDesigner.getCurrentLevelNr() );
    gfx.PrintXY( levelNrRect.x1 + TEXT_SPACER + FONT_WIDTH * 6,levelNrRect.y1 + FONT_Y_OFFSET,buf );
    gfx.PrintXY( levelSelectorRect.x2 + TEXT_SPACER,levelSelectorRect.y1 + FONT_Y_OFFSET,levelDesigner.getLevelTitle() );
    getNrStr( buf,levelDesigner.getCurrentBackgroundNr() );
    gfx.PrintXY( backgroundNrRect.x1 + TEXT_SPACER + FONT_WIDTH * 11,backgroundNrRect.y1 + FONT_Y_OFFSET,buf );
    getNrStr( buf,levelDesigner.getCurrentBlockNr() );
    gfx.PrintXY( blockNrRect.x1 + TEXT_SPACER + FONT_WIDTH * 6,blockNrRect.y1 + FONT_Y_OFFSET,buf );
    // Print the Skin / Color block type selector button text:
    char *t;
    if ( levelDesigner.isBlockSkinned() ) t = "Skin: bitmap";
    else                                  t = "Skin: color";
    gfx.PrintXY( blockNrRect.x1 + 1 + TEXT_SPACER,blockSkinOrColorRect.y1 + FONT_Y_OFFSET,t );
    // Print how many hits this button requires before it is destroyed (max 4, or infinite)
    switch ( levelDesigner.getHitsToDestruct() )
    {
        case 1:     { t = "Block hits: 1"; break; }
        case 2:     { t = "Block hits: 2"; break; }
        case 3:     { t = "Block hits: 3"; break; }
        case 4:     { t = "Block hits: 4"; break; }
        default:    
        { 
            buf[0] = '\0';
            strcat( buf,"Block hits:  " );
            buf[12] = (signed char)INFINITE_SYMBOL;
            t = buf;
        }
    }
    gfx.PrintXY( hitsToDestructRect.x1 + 1 + TEXT_SPACER,hitsToDestructRect.y1 + FONT_Y_OFFSET,t );
    // draw the block:
    int blockX = blockPreviewRect.x1 + 
        (blockPreviewRect.x2 - blockPreviewRect.x1 + 1 -
            levelDesigner.getBlock( levelDesigner.getCurrentBlockNr() ).width) / 2;
    int blockY = blockPreviewRect.y1 + 
        (blockPreviewRect.y2 - blockPreviewRect.y1 + 1 -
            levelDesigner.getBlock( levelDesigner.getCurrentBlockNr() ).height) / 2;
    gfx.PaintBMP( blockX,blockY,levelDesigner.getBlock( levelDesigner.getCurrentBlockNr() ).skin );
    // draw the R, G, B levels:
    int c = levelDesigner.getBlock( levelDesigner.getCurrentBlockNr() ).color;
    getNrStr( buf,(c >> 16) & 0xFF );
    gfx.PrintXY( colorRRect.x1 + TEXT_SPACER + FONT_WIDTH * 3,colorRRect.y1 + FONT_Y_OFFSET,buf );
    getNrStr( buf,(c >>  8) & 0xFF );
    gfx.PrintXY( colorGRect.x1 + TEXT_SPACER + FONT_WIDTH * 3,colorGRect.y1 + FONT_Y_OFFSET,buf );
    getNrStr( buf,(c      ) & 0xFF );
    gfx.PrintXY( colorBRect.x1 + TEXT_SPACER + FONT_WIDTH * 3,colorBRect.y1 + FONT_Y_OFFSET,buf );
    gfx.DrawBlock( colorSampleRect.x1 + FRAME_F,colorSampleRect.y1 + FRAME_F,
        colorSampleRect.x2 - FRAME_F,colorSampleRect.y2 - FRAME_F,c );
    // draw block skin library number:
    getNrStr( buf,levelDesigner.getCurrentBlockSkinLibNr() );
    gfx.PrintXY( skinLibraryRect.x1 + TEXT_SPACER + FONT_WIDTH * 13,skinLibraryRect.y1 + FONT_Y_OFFSET,buf );
    // draw block skin library & cursor:
    Rect r;
    r.x1 = levelDesigner.getBlockSkinLibXOrig();
    r.x2 = r.x1 + skinPreviewClientRect.x2 - skinPreviewClientRect.x1;
    r.y1 = levelDesigner.getBlockSkinLibYOrig();
    r.y2 = r.y1 + skinPreviewClientRect.y2 - skinPreviewClientRect.y1;
    gfx.PaintFromBMP(skinPreviewClientRect.x1, skinPreviewClientRect.y1, r, levelDesigner.getBlockSkin());
    if (frameNr >= 30) c = 0xF0 - 6 * (frameNr - 30);
    else c = 6 * frameNr;
    c = 0x800000 + (c << 8) + c;
    Rect boxCoords;
    Block& block = levelDesigner.getBlock( levelDesigner.getCurrentBlockNr() );
    boxCoords.x1 = skinPreviewClientRect.x1 + block.skinXLocation - levelDesigner.getBlockSkinLibXOrig() - 2;
    boxCoords.y1 = skinPreviewClientRect.y1 + block.skinYLocation - levelDesigner.getBlockSkinLibYOrig() - 2;
    boxCoords.x2 = boxCoords.x1 + block.width + 3;
    boxCoords.y2 = boxCoords.y1 + block.height + 3;
    gfx.DrawBoxClipped( boxCoords,skinPreviewClientRect,c );
    boxCoords.x1++;
    boxCoords.y1++;
    boxCoords.x2--;
    boxCoords.y2--;
    gfx.DrawBoxClipped( boxCoords,skinPreviewClientRect,c );
    if ( mouse.IsInArea( gameWindowRect )  )
    {
        ShowHelp( gameWindowText );
        // Draw the potential block at mouse cursor position: 
        Block& block = levelDesigner.getBlock( levelDesigner.getCurrentBlockNr() );
        Rect r = levelDesigner.getGridCellClientRect( mouse.GetMouseX(),mouse.GetMouseY() );
        if( (r.x1 >= levelDesigner.getGridXOffset()) &&
            (r.x2 <= levelDesigner.getGridMaxX()   ) && 
            (r.y1 >= levelDesigner.getGridYOffset()) && 
            (r.y2 <= levelDesigner.getGridMaxY()   ) )
        {
            gfx.DrawBox( r.x1,r.y1,r.x2,r.y2,c );
            gfx.DrawBox( r.x1 + 1,r.y1 + 1,r.x2 - 1,r.y2 - 1,c );
            if( kbd.SpaceIsPressed() )
            {
                gfx.DrawLine( r.x1,r.y1,r.x2,r.y2,c );
                gfx.DrawLine( r.x2,r.y1,r.x1,r.y2,c );
            }
        }
    }
    if ( puMenuActive ) DrawPowerUpMenu();
}

void Game::DrawLevelDesigner()
{
    /*
        Draw Level first:
    */   
    gfx.PaintBMP( 
        levelDesigner.getBackGroundX(),
        levelDesigner.getBackGroundY(),
        levelDesigner.getBackground() );
    DrawGrid();
    for ( int blockNr = 0; blockNr < MAX_BLOCKS_PER_LEVEL; blockNr++ )
        if ( levelDesigner.getBlockList()[blockNr].xOrig >= 0 )
        {
            BlockData& currentBlock = levelDesigner.getBlockList()[blockNr];
            Block& blockType = levelDesigner.getBlock( currentBlock.blockNr );
            gfx.PaintBMP( currentBlock.xOrig,currentBlock.yOrig,blockType.skin );
            // print the blocks hit to destruct number
            char buf[2];
            buf[1] = '\0';
            if ( blockType.hitsToDestruct < PERSISTENT_BLOCK )
                 buf[0] = '0' + blockType.hitsToDestruct;
            else buf[0] = (signed char)INFINITE_SYMBOL;
            gfx.PrintXY( currentBlock.xOrig + 3,currentBlock.yOrig + 3,buf );
            switch ( currentBlock.powerUp )
            {
                case doubleball:
                { buf[0] = 'D'; break; }
                case multiball:
                { buf[0] = 'T'; break; }
                case ballstorm:
                { buf[0] = 'S'; break; }
                case goldenball:
                { buf[0] = 'O'; break; }
                case guns:
                { buf[0] = 'G'; break; }
                case bigpad:
                { buf[0] = 'L'; break; }
                case smallpad:
                { buf[0] = '-'; break; }
                default:
                { buf[0] = ' '; break; }
            }
            gfx.PrintXY( 
                currentBlock.xOrig + blockType.width - 4 - font.width(),
                currentBlock.yOrig + 3,buf );
        }
    /*
        Now paint the level designers' control panel. First the static content:
    */
    gfx.DrawNiceBlock( panelRect );
    gfx.DrawNiceBlockInv( levelNrRect );
    gfx.DrawNiceBlockInv( levelTitleRect );
    gfx.DrawNiceBlockInv( backgroundNrRect );
    gfx.DrawNiceBlockInv( blockPickerRect );  
    gfx.DrawNiceBlockInv( gridWidthRect );
    gfx.DrawNiceBlockInv( gridHeightRect );
    gfx.DrawNiceBlockInv( gridSpacerRect );
    gfx.DrawButton( gridOnOffSwitchRect );
    gfx.DrawButton( toBlockEditorSwitchRect );
    gfx.DrawButton( saveAndExitButtonRect );
    gfx.DrawButtonPlusMinus( levelSelectorRect,1 );
    gfx.DrawButtonPlusMinus( backgroundSelectorRect,1 );
    gfx.DrawButtonPlusMinus( gridWidthSelectorRect,1 );
    gfx.DrawButtonPlusMinus( gridHeightSelectorRect,1 );
    gfx.DrawButtonPlusMinus( gridSpacerSelectorRect,1 );
    DrawScrollLogo(); // draw it BEFORE we write on top of it ;)
    gfx.PrintXY( levelNrRect.x1 + TEXT_SPACER,levelNrRect.y1 + FONT_Y_OFFSET,"Level " );
    gfx.PrintXY( backgroundNrRect.x1 + TEXT_SPACER,backgroundNrRect.y1 + FONT_Y_OFFSET,"Background " );
    gfx.PrintXY( toBlockEditorSwitchRect.x1 + TEXT_SPACER,
                 toBlockEditorSwitchRect.y1 + FONT_Y_OFFSET,"[Level Designer]" );
    gfx.PrintXY( gridWidthRect.x1  + TEXT_SPACER,gridWidthRect.y1  + FONT_Y_OFFSET,"W: " );
    gfx.PrintXY( gridHeightRect.x1 + TEXT_SPACER,gridHeightRect.y1 + FONT_Y_OFFSET,"H: " );
    gfx.PrintXY( gridSpacerRect.x1 + TEXT_SPACER,gridSpacerRect.y1 + FONT_Y_OFFSET,"S: " );
    gfx.PrintXY( saveAndExitButtonRect.x1 + TEXT_SPACER,saveAndExitButtonRect.y1 + FONT_Y_OFFSET," Save and exit" );
    /*
        And now the dynamic content:
    */
    char buf[128];
    // Grid variables:
    if( levelDesigner.isGridVisible() )
        gfx.PrintXY ( gridOnOffSwitchRect.x1 + TEXT_SPACER,gridOnOffSwitchRect.y1 + FONT_Y_OFFSET,"Grid On" );
    else
        gfx.PrintXY ( gridOnOffSwitchRect.x1 + TEXT_SPACER,gridOnOffSwitchRect.y1 + FONT_Y_OFFSET,"Grid Off" );
    _itoa( levelDesigner.getGridCellWidth(),buf,10 );
    gfx.PrintXY( gridWidthRect.x1  + TEXT_SPACER + FONT_WIDTH * 3,gridWidthRect.y1  + FONT_Y_OFFSET,buf );
    _itoa( levelDesigner.getGridCellHeight(),buf,10 );
    gfx.PrintXY( gridHeightRect.x1 + TEXT_SPACER + FONT_WIDTH * 3,gridHeightRect.y1 + FONT_Y_OFFSET,buf );
    _itoa( levelDesigner.getGridSpacer(),buf,10 );
    gfx.PrintXY( gridSpacerRect.x1 + TEXT_SPACER + FONT_WIDTH * 3,gridSpacerRect.y1 + FONT_Y_OFFSET,buf );
    // Level nr, background nr and so on:
    getNrStr( buf,levelDesigner.getCurrentLevelNr() );
    gfx.PrintXY( levelNrRect.x1 + TEXT_SPACER + FONT_WIDTH * 6,levelNrRect.y1 + FONT_Y_OFFSET,buf );
    gfx.PrintXY( levelSelectorRect.x2 + TEXT_SPACER,levelSelectorRect.y1 + FONT_Y_OFFSET,levelDesigner.getLevelTitle() );
    getNrStr( buf,levelDesigner.getCurrentBackgroundNr() );
    gfx.PrintXY( backgroundNrRect.x1 + TEXT_SPACER + FONT_WIDTH * 11,backgroundNrRect.y1 + FONT_Y_OFFSET,buf );
    // draw block skin library & cursor:
    Rect r;
    r.x1 = levelDesigner.getBlockLibraryXOrig();
    r.x2 = r.x1 + blockPickerClientRect.x2 - blockPickerClientRect.x1;
    r.y1 = levelDesigner.getBlockLibraryYOrig();
    r.y2 = r.y1 + blockPickerClientRect.y2 - blockPickerClientRect.y1;
    gfx.PaintFromBMP( blockPickerClientRect.x1, blockPickerClientRect.y1, r, levelDesigner.getBlockLibrary () );
    int c;
    if (frameNr >= 30) c = 0xF0 - 6 * (frameNr - 30);
    else c = 6 * frameNr;
    c = 0x800000 + (c << 8) + c;
    Rect boxCoords;
    boxCoords = levelDesigner.blockLibrary.getBlockBorders(
        (levelDesigner.getCurrentBlockNr() % levelDesigner.getBlockLibraryColumns())
        * levelDesigner.getBlockLibraryCellWidth()  + levelDesigner.getBlockLibrarySpacer(),
        (levelDesigner.getCurrentBlockNr() / levelDesigner.getBlockLibraryColumns())
        * levelDesigner.getBlockLibraryCellHeight() + levelDesigner.getBlockLibrarySpacer()
    );
    boxCoords.x1 += blockPickerClientRect.x1 - levelDesigner.getBlockLibraryXOrig() 
                    - levelDesigner.getBlockLibrarySpacer();
    boxCoords.x2 += blockPickerClientRect.x1 - levelDesigner.getBlockLibraryXOrig() 
                    + levelDesigner.getBlockLibrarySpacer();
    boxCoords.y1 += blockPickerClientRect.y1 - levelDesigner.getBlockLibraryYOrig() 
                    - levelDesigner.getBlockLibrarySpacer();
    boxCoords.y2 += blockPickerClientRect.y1 - levelDesigner.getBlockLibraryYOrig() 
                    + levelDesigner.getBlockLibrarySpacer();
    gfx.DrawBoxClipped( boxCoords,blockPickerClientRect,c );
    boxCoords.x1++;
    boxCoords.x2--;
    boxCoords.y1++;
    boxCoords.y2--;
    gfx.DrawBoxClipped( boxCoords,blockPickerClientRect,c );
    if ( mouse.IsInArea( gameWindowRect )  )
    {
        // Draw the potential block at mouse cursor position: 
        Rect r = levelDesigner.getGridCellClientRect( mouse.GetMouseX(),mouse.GetMouseY() );
        if( (r.x1 >= levelDesigner.getGridXOffset()) &&
            (r.x2 <= levelDesigner.getGridMaxX()   ) &&
            (r.y1 >= levelDesigner.getGridYOffset()) &&
            (r.y2 <= levelDesigner.getGridMaxY()   ) )
        {
            gfx.DrawBox( r.x1,r.y1,r.x2,r.y2,c );
            gfx.DrawBox( r.x1 + 1,r.y1 + 1,r.x2 - 1,r.y2 - 1,c );
            if( kbd.SpaceIsPressed() )
            {
                gfx.DrawLine( r.x1,r.y1,r.x2,r.y2,c );
                gfx.DrawLine( r.x2,r.y1,r.x1,r.y2,c );
            }
        }
    }
    if ( puMenuActive ) DrawPowerUpMenu();
}

void Game::DoMainMenu()
{
    int slmax = 0;
    int menuCnt;
    for ( menuCnt = 0; mainmenu[menuCnt] != nullptr ; menuCnt++ )
    {
        int s = strlen(mainmenu[menuCnt]);
        if ( s > slmax ) slmax = s;
    }
    const int w = (slmax + 4) * font.width();
    const int h = menuCnt * 2 * font.height();
    const int x1 = (HOR_SCREEN_RES - w) / 2;
    const int x2 = x1 + w;
    const int y1 = (VER_SCREEN_RES - h) / 2;
    const int y2 = y1 + h;
    const int vspacer = font.height() / 3;
    int mx = mouse.GetMouseX();
    int my = mouse.GetMouseY();
    int choosenMenu = 0;
    for ( int menuNr = 0; menuNr < menuCnt; menuNr++ )
    {        
        int y = y1 + menuNr * 2 * font.height();
        Rect r( x1,y - vspacer,
                x2,y + font.height() + vspacer );
        gfx.DrawBlock( r,DEFAULT_FRAME_COLOR );    
        gfx.DrawButton( r );
        gfx.PrintXY( 
            x1 + (w - strlen( mainmenu[menuNr] ) * font.width()) / 2,
            y,
            mainmenu[menuNr] );
        if ( r.contains( mx,my ) ) choosenMenu = menuNr;
    }
    if ( leftButtonPressed )
    {
        switch ( choosenMenu )
        {
            case MENU_START_GAME:
            {
                //ShowCursor( false ); // hide mouse cursor
                gameScore = 0;
                bgScrollX = 0;
                bgScrollY = 0;
                bgScrollHorSpeed = -2;
                bgScrollVerSpeed = -2;
                levelDesigner.setLevelNr( levelDesigner.getCurrentLevelNr() );
                levelDesigner.loadInGameLevel( levelDesigner.getCurrentLevelNr() );                
                powerUpLogic.ResetPowerUps();
                arkanoid.resetPaddle();
                arkanoid.resetBullets();
                arkanoid.DeActivateAllBalls();
                arkanoid.CreateBall( 
                    arkanoid.getPaddleXCenter(),PADDLE_Y1 - BALL_RADIUS,2.0f,-8.0f );
                livesLeft = 2;
                arkanoid.stopBall();
                countDown = COUNTDOWN;
                gameState = gamenextlevel;
                break;
            }
            case MENU_CHOOSE_LEVEL:
            {
                gameState = gamechooselevel;
                break;
            }
            case MENU_LEVELDESIGNER:
            {
                gameState = gameleveldesigner;
                break;
            }
            case MENU_CREDITS:
            {
                gameState = gamecredits;
                break;
            }
            case MENU_QUIT:
            {
                PostQuitMessage( 0 ); // clean exit! Destructors need to be called.
                break;
            }
        }
    }
}

#define LEVEL_TITLE_COLUMS      (MAX_NR_OF_LEVELS )
void Game::DoChooseLevelMenu()
{
    /*
        four columns of 25 levels each
    */
    const int titleOffset = 36;
    const int nrColumns = 4;
    const int nrRows = 25;
    const int titleWidth = (LEVEL_TITLE_MAX_LENGTH + 4) * font.width();
    const int totalWidth = titleWidth * nrColumns;
    const int totalHeight = font.height() * 2 * nrRows;
    const int x1 = (HOR_SCREEN_RES - totalWidth) / 2;
    const int y1 = (VER_SCREEN_RES - totalHeight) / 2 + titleOffset;
    const int title_x = 132;
    const int title_y = 6;
    int mx = mouse.GetMouseX();
    int my = mouse.GetMouseY();
    int selectedLevel = -1;
    // Print Title
    Font *oldFont = gfx.getFont();
    gfx.setFont( &neuropolXBMP );
    gfx.PrintXY( title_x,title_y,"Choose your level",100 );
    gfx.setFont( oldFont );
    for ( int i = 0; i < 25; i++ )
    {
        int x = x1 + font.width() * 2;
        int y = y1 + i * 2 * font.height();
        Rect r( x1 + font.width() * 1,
                y  - font.height() / 3,
                x1 - font.width() * 1 + titleWidth,
                y  + font.height() + font.height() / 3 );
        for ( int i2 = 0; i2 < nrColumns; i2++ )
        {
            int levelNr = i + i2 * 25;
            if ( levelNr > levelDesigner.getMaxStartLevel() )
            {
                gfx.DrawBlock( r,0xA0A0A0 );
            } else if ( r.contains( mx,my ) ) {
                selectedLevel = levelNr;
                gfx.DrawBlock( r,0xFFB0B0 );
            } else { 
                gfx.DrawBlock( r,DEFAULT_FRAME_COLOR );
            }
            gfx.DrawButton( r );
            gfx.PrintXY( x,y,levelDesigner.getLevelTitle( levelNr ) );
            r.x1 += titleWidth;
            r.x2 += titleWidth;
            x += titleWidth;
        }
    }
    if ( leftButtonPressed && ( selectedLevel >= 0 ) )
    {
        levelDesigner.setLevelNr( selectedLevel );
        levelDesigner.loadInGameLevel( selectedLevel );
        gameState = gamemenu;
    }
}

void Game::DrawCredits()
{
    int slmax = 0;
    int lineCnt;
    for ( lineCnt = 0; showcreditsmenu[lineCnt] != nullptr; lineCnt++ )
    {
        int s = strlen( showcreditsmenu[lineCnt] );
        if ( s > slmax ) slmax = s;
    }
    const int w = (slmax + 4) * font.width();
    const int h = (lineCnt + 2) * 2 * font.height();
    const int x1 = (HOR_SCREEN_RES - w) / 2;
    const int x2 = x1 + w;
    int y1 = (VER_SCREEN_RES - h) / 2;
    const int y2 = y1 + h;
    const int vspacer = font.height() / 3;
    Rect r( x1,y1,x2,y2 );
    gfx.DrawBlock( r,DEFAULT_FRAME_COLOR );
    gfx.DrawButton( r );
    gfx.DrawNiceBlockInv( Rect( r.x1 + 3,r.y1 + 3,r.x2 - 3,r.y2 - 3 ) );
    y1 += 2 * font.height();
    for ( int lineNr = 0; lineNr < lineCnt; lineNr++ )
    {
        gfx.PrintXY(
            x1 + (w - strlen( showcreditsmenu[lineNr] ) * font.width()) / 2,
            y1 + lineNr * 2 * font.height(),
            showcreditsmenu[lineNr] );
    }
}

void Game::ProcessKeyBoard()
{
    enterPressed = false;
    upPressed    = false;
    downPressed  = false;
    leftPressed  = false;
    rightPressed = false;
    spacePressed = false;
    lastKbdState.keyStateEnter = currentKbdState.keyStateEnter;
    lastKbdState.keyStateUp    = currentKbdState.keyStateUp;
    lastKbdState.keyStateDown  = currentKbdState.keyStateDown;
    lastKbdState.keyStateLeft  = currentKbdState.keyStateLeft;
    lastKbdState.keyStateRight = currentKbdState.keyStateRight;
    lastKbdState.keyStateSpace = currentKbdState.keyStateSpace;
    if ( kbd.EnterIsPressed() ) currentKbdState.keyStateEnter = keyisdown;
    else                        currentKbdState.keyStateEnter = keyisup;
    if ( kbd.UpIsPressed()    ) currentKbdState.keyStateUp    = keyisdown;
    else                        currentKbdState.keyStateUp    = keyisup;
    if ( kbd.DownIsPressed()  ) currentKbdState.keyStateDown  = keyisdown;
    else                        currentKbdState.keyStateDown  = keyisup;
    if ( kbd.LeftIsPressed()  ) currentKbdState.keyStateLeft  = keyisdown;
    else                        currentKbdState.keyStateLeft  = keyisup;
    if ( kbd.RightIsPressed() ) currentKbdState.keyStateRight = keyisdown;
    else                        currentKbdState.keyStateRight = keyisup;
    if ( kbd.SpaceIsPressed() ) currentKbdState.keyStateSpace = keyisdown;
    else                        currentKbdState.keyStateSpace = keyisup;
    if ( (lastKbdState.keyStateEnter    == keyisup) &&
         (currentKbdState.keyStateEnter == keyisdown) )
         enterPressed = true;
    else enterPressed = false;
    if ( (lastKbdState.keyStateUp       == keyisup) &&
         (currentKbdState.keyStateUp    == keyisdown) )
         upPressed = true;
    else upPressed = false;                 
    if ( (lastKbdState.keyStateDown     == keyisup) &&
         (currentKbdState.keyStateDown  == keyisdown) )
         downPressed = true;
    else downPressed = false;
    if ( (lastKbdState.keyStateLeft     == keyisup) &&
         (currentKbdState.keyStateLeft  == keyisdown) )
         leftPressed = true;
    else leftPressed = false;
    if ( (lastKbdState.keyStateRight    == keyisup) &&
         (currentKbdState.keyStateRight == keyisdown) )
         rightPressed = true;
    else rightPressed = false;
    if ( (lastKbdState.keyStateSpace    == keyisup) &&
         (currentKbdState.keyStateSpace == keyisdown) )
         spacePressed = true;
    else spacePressed = false;
}

void Game::ProcessMouse()
{
    if ( mouse.LeftIsPressed() )
    {
        if ( ! mouseDragging )
        {
            mouseXDragStart = mouse.GetMouseX();
            mouseYDragStart = mouse.GetMouseY();
            mouseDragging = true;
            mouseDragEnded = false;
        }
    } else {
        mouseDragging = false;
        mouseDragEnded = true;        
    }

    leftButtonPressed = false;
    rightButtonPressed = false;
    lastMouseState.leftMouseButton = currentMouseState.leftMouseButton;
    lastMouseState.rightMouseButton = currentMouseState.rightMouseButton;
    if ( mouse.LeftIsPressed() ) 
            currentMouseState.leftMouseButton = buttonisdown;
    else    currentMouseState.leftMouseButton = buttonisup;
    if ( mouse.RightIsPressed() ) 
            currentMouseState.rightMouseButton = buttonisdown;
    else    currentMouseState.rightMouseButton = buttonisup;
    if ( (currentMouseState.leftMouseButton != lastMouseState.leftMouseButton) ||
         (currentMouseState.rightMouseButton != lastMouseState.rightMouseButton) )
    {
        lastMouseState.mx = currentMouseState.mx;
        lastMouseState.my = currentMouseState.my;
        currentMouseState.mx = mouse.GetMouseX();
        currentMouseState.my = mouse.GetMouseY();
        if ( (! mouse.LeftIsPressed()) && 
            (lastMouseState.leftMouseButton == buttonisdown) ) 
            leftButtonPressed = true;
        if ( (! mouse.RightIsPressed()) && 
            (lastMouseState.rightMouseButton == buttonisdown) ) 
            rightButtonPressed = true;
    }
}

inline void Game::ShowHelp ( const char *text[] )
{
    gfx.PrintText( 
        helpWindowRect.x1 + TEXT_SPACER,
        helpWindowRect.y1 + TEXT_SPACER,
        text );
}

void Game::LevelDesignerProcessInput()
{
    if ( mouse.IsInArea( gameWindowRect )  )
    {
        Block& block = levelDesigner.getBlock( levelDesigner.getCurrentBlockNr() );
        Rect r = levelDesigner.getGridCellClientRect( mouse.GetMouseX(),mouse.GetMouseY() );
        if( (r.x1 >= levelDesigner.getGridXOffset()) &&
            (r.x2 <= levelDesigner.getGridMaxX()   ) &&
            (r.y1 >= levelDesigner.getGridYOffset()) &&
            (r.y2 <= levelDesigner.getGridMaxY()   ) )
        {
            if ( kbd.SpaceIsPressed() )  levelDesigner.removeBlockAtPosition( r.x1,r.y1 );
            if ( mouse.LeftIsPressed() && ( ! puMenuActive ) )
                levelDesigner.putBlockAtPosition( r.x1,r.y1 );
        }
        if ( puMenuActive )
        {
        } else { 
            if ( rightButtonPressed ) 
            { 
                puMenuX = mouse.GetMouseX();
                puMenuY = mouse.GetMouseY();
                puNrOfBlock = levelDesigner.getBlockAtPosition( puMenuX,puMenuY );
                if ( puNrOfBlock >= 0 )
                {
                    if ( puMenuX + puMenuWidth >= HOR_SCREEN_RES ) 
                        puMenuX = HOR_SCREEN_RES - 1 - puMenuWidth;
                    if ( puMenuY + puMenuHeight >= VER_SCREEN_RES ) 
                        puMenuY = VER_SCREEN_RES - 1 - puMenuHeight;
                    puMenuRect = Rect( 
                        puMenuX,puMenuY,
                        puMenuX + puMenuWidth - 1,puMenuY + puMenuHeight - 1 );
                    puMenuActive = true;
                }
            }
        }
    } else if ( mouse.IsInArea( backgroundSelectorRect ) )
    {
        if ( mouse.IsInUpperHalf( backgroundSelectorRect ) )
        {
            if ( leftButtonPressed ) levelDesigner.incBackgroundNr();
        }
        else {
            if ( leftButtonPressed ) levelDesigner.decBackgroundNr();
        }

    } else if ( mouse.IsInArea( backgroundNrRect ) )
    {
        if ( leftButtonPressed )
        {
            levelDesigner.switchBackgroundScrollType();
        }
    } else if ( mouse.IsInArea( levelSelectorRect ) )
    {
        if ( mouse.IsInUpperHalf( levelSelectorRect ) )
        {
            if ( leftButtonPressed ) levelDesigner.incLevelNr();
        }
        else {
            if ( leftButtonPressed ) levelDesigner.decLevelNr();
        }
    } else if ( mouse.IsInArea( blockPickerClientRect ) )
    {        
        if ( leftButtonPressed || rightButtonPressed )
            levelDesigner.selectBlockFromLibrary(
                mouse.GetMouseX(),mouse.GetMouseY() );        
        if ( mouseDragging )   
        {
            levelDesigner.shiftBlockLibraryXOrig(
                mouseXDragStart - mouse.GetMouseX() );
            levelDesigner.shiftBlockLibraryYOrig(
                mouseYDragStart - mouse.GetMouseY() );
        }
        mouseXDragStart = mouse.GetMouseX();
        mouseYDragStart = mouse.GetMouseY();
    } else if ( mouse.IsInArea( toBlockEditorSwitchRect ) )
    {
        if( leftButtonPressed ) levelDesigner.switchLevelDesignerMode();
    } else if ( mouse.IsInArea ( gridOnOffSwitchRect ) )
    {
        if( leftButtonPressed ) levelDesigner.gridSwitchVisible();
    } else if( mouse.IsInArea ( gridWidthSelectorRect ) )
    {
        if( mouse.IsInUpperHalf ( gridWidthSelectorRect ) )
        {
            if( ( leftButtonPressed ) ||
                (mouse.RightIsPressed ()) )
                levelDesigner.gridInit (
                    levelDesigner.getGridCellWidth() + 1,
                    levelDesigner.getGridCellHeight(),
                    levelDesigner.getGridSpacer() );
        }
        else {
            if ( ( leftButtonPressed ) ||
                (mouse.RightIsPressed()) )
                levelDesigner.gridInit(
                    levelDesigner.getGridCellWidth() - 1,
                    levelDesigner.getGridCellHeight(),
                    levelDesigner.getGridSpacer() );
        }
    } else if ( mouse.IsInArea ( gridHeightSelectorRect ) )
    {
        if ( mouse.IsInUpperHalf( gridHeightSelectorRect ) )
        {
            if ( ( leftButtonPressed ) ||
                (mouse.RightIsPressed()) )
                levelDesigner.gridInit(
                    levelDesigner.getGridCellWidth(),
                    levelDesigner.getGridCellHeight() + 1,
                    levelDesigner.getGridSpacer() );
        } else {
            if ( ( leftButtonPressed ) ||
                (mouse.RightIsPressed()) )
                levelDesigner.gridInit(
                    levelDesigner.getGridCellWidth(),
                    levelDesigner.getGridCellHeight() - 1,
                    levelDesigner.getGridSpacer() );
        }
    }  else if ( mouse.IsInArea ( gridSpacerSelectorRect ) )
    {
        if ( mouse.IsInUpperHalf( gridSpacerSelectorRect ) )
        {
            if ( leftButtonPressed )
                levelDesigner.gridInit(
                    levelDesigner.getGridCellWidth(),
                    levelDesigner.getGridCellHeight(),
                    levelDesigner.getGridSpacer() + 1 );
        } else {
            if ( leftButtonPressed )
                levelDesigner.gridInit(
                    levelDesigner.getGridCellWidth(),
                    levelDesigner.getGridCellHeight(),
                    levelDesigner.getGridSpacer() - 1 );
        }
    } else if ( mouse.IsInArea( saveAndExitButtonRect ) )
    {
        if ( leftButtonPressed )
        {
            levelDesigner.setLevelNr( levelDesigner.getCurrentLevelNr() );
            levelDesigner.loadInGameLevel( levelDesigner.getCurrentLevelNr() ); // temp debug
            gameState = gamemenu;
        }
    }
}

void Game::BlockDesignerProcessInput()
{
    
    if ( mouse.IsInArea( levelNrRect ) )
    {
        ShowHelp( levelSelectorText );
    } else if ( mouse.IsInArea( levelTitleRect ) )
    {
        ShowHelp( levelTitleText );
    } else if ( mouse.IsInArea( backgroundNrRect ) )
    {
        ShowHelp( backgroundNrText );
        if ( leftButtonPressed )
        {
            levelDesigner.switchBackgroundScrollType();
        }
    } else if ( mouse.IsInArea( skinLibraryRect ) )         
    {
        ShowHelp( skinLibrarySelectorText );
    } else if ( mouse.IsInArea( blockNrRect ) )         
    {
        ShowHelp( blockNrSelectorText );
    } else if ( mouse.IsInArea( colorRRect ) ||
                mouse.IsInArea( colorGRect ) ||
                mouse.IsInArea( colorBRect ) ||
                mouse.IsInArea( colorSampleRect ) )
    {
        ShowHelp( colorRGBText );

    } else if ( mouse.IsInArea( gameWindowRect )  )
    {
        ShowHelp( gameWindowText );
        // Draw the cursor at the mouse cursor position: 
        Rect r = levelDesigner.getGridCellClientRect( mouse.GetMouseX(),mouse.GetMouseY() );
        if( (r.x1 >= levelDesigner.getGridXOffset()) &&
            (r.x2 <= levelDesigner.getGridMaxX()   ) && 
            (r.y1 >= levelDesigner.getGridYOffset()) && 
            (r.y2 <= levelDesigner.getGridMaxY()   ) )
        {
            if ( kbd.SpaceIsPressed() )  levelDesigner.removeBlockAtPosition( r.x1,r.y1 );
            if ( mouse.LeftIsPressed() ) levelDesigner.putBlockAtPosition( r.x1,r.y1 );
        }
        /*
            if rightbutton pressed: cycle through power ups
        */
    } else if ( mouse.IsInArea( backgroundSelectorRect ) )
    {
        ShowHelp( backgroundSelectorText );
        if ( mouse.IsInUpperHalf( backgroundSelectorRect ) )
        {
            if ( leftButtonPressed ) levelDesigner.incBackgroundNr();
        }
        else {
            if ( leftButtonPressed ) levelDesigner.decBackgroundNr();
        }
    } else if ( mouse.IsInArea( levelSelectorRect ) )
    {
        ShowHelp( levelSelectorText );
        if ( mouse.IsInUpperHalf( levelSelectorRect ) )
        {
            if ( leftButtonPressed ) levelDesigner.incLevelNr();
        }
        else {
            if ( leftButtonPressed ) levelDesigner.decLevelNr();
        }

    } else if ( mouse.IsInArea( skinLibrarySelectorRect ) )
    {
        ShowHelp( skinLibrarySelectorText );
        if ( mouse.IsInUpperHalf( skinLibrarySelectorRect ) )
        {
            if ( leftButtonPressed ) levelDesigner.incBlockSkinLibNr();
        }
        else {
            if ( leftButtonPressed ) levelDesigner.decBlockSkinLibNr();
        }
    } else if ( mouse.IsInArea( colorRSelectorRect ) )
    {
        ShowHelp( colorRGBText );
        if( mouse.IsInUpperHalf( colorRSelectorRect ) ) 
        {
            if ( (mouse.LeftIsPressed() && (frameNr & 0x1)) ||
                (mouse.RightIsPressed()) ) 
                levelDesigner.changeRGBColor( rcomponent,1 );
        } else {
            if ( (mouse.LeftIsPressed() && (frameNr & 0x1)) ||
                (mouse.RightIsPressed()) ) 
                levelDesigner.changeRGBColor( rcomponent,-1 );
        }
    } else if ( mouse.IsInArea( colorGSelectorRect ) )
    {
        ShowHelp( colorRGBText );
        if( mouse.IsInUpperHalf( colorGSelectorRect ) ) 
        {
            if ( (mouse.LeftIsPressed() && (frameNr & 0x1)) ||
                (mouse.RightIsPressed()) ) 
                levelDesigner.changeRGBColor( gcomponent,1 );
        } else {
            if ( (mouse.LeftIsPressed() && (frameNr & 0x1)) ||
                (mouse.RightIsPressed()) ) 
                levelDesigner.changeRGBColor( gcomponent,-1 );
        }
    } else if ( mouse.IsInArea( colorBSelectorRect ) )
    {
        ShowHelp( colorRGBText );
        if( mouse.IsInUpperHalf( colorBSelectorRect ) ) 
        {
            if ( (mouse.LeftIsPressed() && (frameNr & 0x1)) ||
                (mouse.RightIsPressed()) ) 
                levelDesigner.changeRGBColor( bcomponent,1 );
        } else {
            if ( (mouse.LeftIsPressed() && (frameNr & 0x1)) ||
                (mouse.RightIsPressed()) ) 
                levelDesigner.changeRGBColor( bcomponent,-1 );
        }
    } else if ( mouse.IsInArea( blockSkinOrColorRect ) )
    {
        ShowHelp( blockSkinOrColorText );
        if ( leftButtonPressed ) levelDesigner.switchBlockColorOrSkinned();
    } else if ( mouse.IsInArea( blockNrSelectorRect ))
    {
        ShowHelp( blockNrSelectorText );
        if ( mouse.IsInUpperHalf( blockNrSelectorRect ))
        {
            if ( leftButtonPressed ) levelDesigner.changeBlockNr( 1 );                            
        }
        else {
            if ( leftButtonPressed ) levelDesigner.changeBlockNr( -1 );
        }
    } else if ( mouse.IsInArea( blockNrFastSelectorRect ) )
    {
        ShowHelp( blockNrSelectorText );
        if ( mouse.IsInUpperHalf( blockNrFastSelectorRect ) )
        {
            if ( leftButtonPressed ) levelDesigner.changeBlockNr( 25 );
        }
        else {
            if ( leftButtonPressed ) levelDesigner.changeBlockNr( -25 );
        }
    } else if ( mouse.IsInArea( hitsToDestructRect ) )
    {
        ShowHelp( hitsToDestructText );
        if ( leftButtonPressed ) levelDesigner.incHitsToDestruct();
    } else if ( mouse.IsInArea( skinPreviewClientRect ) )
    {
        ShowHelp( skinPreviewClientText );
        if ( rightButtonPressed )
            levelDesigner.selectBlockFromSkinLib( 
                mouse.GetMouseX(),mouse.GetMouseY() );
        if ( mouseDragging )   
        {
            levelDesigner.shiftBlockSkinLibXOrig( 
                mouseXDragStart - mouse.GetMouseX() );
            levelDesigner.shiftBlockSkinLibYOrig( 
                mouseYDragStart - mouse.GetMouseY() );
        }
        mouseXDragStart = mouse.GetMouseX();
        mouseYDragStart = mouse.GetMouseY();
    } else if ( mouse.IsInArea( toLevelEditorSwitchRect ) )
    {
        ShowHelp( toLevelEditorSwitchText );
        if ( leftButtonPressed )
        {
            levelDesigner.initBlockLibrary();
            levelDesigner.switchLevelDesignerMode ();
        }
    }
}

void Game::ComposeFrame()
{
#ifdef DEBUG_MODE
    if ( arkanoid.getBall( 0 ).freeze )
        for ( ;; )
        {
            MSG msg;
            ZeroMemory( &msg,sizeof( msg ) );
            if ( PeekMessage( &msg,NULL,0,0,PM_REMOVE ) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            } 
            ProcessKeyBoard();
            if ( spacePressed ) {
                arkanoid.getBall( 0 ).freeze = false;
                break;
            }
        }
#endif
    frameNr++; 
    if ( frameNr > FRAME_RATE - 1 ) 
    {
        frameNr = 0;
        seconds++;
        if ( seconds >= 60 ) seconds = 0;
    }
    ProcessKeyBoard();
    ProcessMouse();

    switch( gameState )
    {
        case gamepaused:
            {
                starVortex.UpdateFrame();
                int o = ( seconds & 0x1 ) ? frameNr * 4: (59 - frameNr) * 4;
                gfx.PrintXY( 220,(VER_SCREEN_RES - neuropolXBMP.height()) / 2,
                                                "Game Paused",o,neuropolXBMP );
                const char *helpStr = "Press Space to continue";
                gfx.PrintXY( 
                    (HOR_SCREEN_RES - strlen( helpStr ) * font.width()) / 2,
                    VER_SCREEN_RES - font.height() - 6,helpStr );
                if ( spacePressed ) gameState = gameplaying;
                break;
            }
        case gamenextlevel:
            {
                arkanoid.setPaddlePosition( mouse.GetMouseX() );
                arkanoid.UpdateGameFrame();
                DrawPlayField();
                if ( countDown > 0 )
                {
                    countDown -= 4;
                    char *s = levelDesigner.getLevelTitle();
                    int x = (HOR_SCREEN_RES - gfx.getStrLen( s,&neuropolXBMP )) / 2;
                    int y = VER_SCREEN_RES / 2;
                    int o;
                    if ( countDown > y )
                        o = ((countDown - y) * 256) / y;
                    else {
                        o = 256 - (countDown * 256) / y;
                        y += o / 2;
                    }
                    gfx.PrintXY( x,y,s,o,neuropolXBMP );
                } else if ( leftButtonPressed )
                {
                    arkanoid.releaseBall();
                    gameState = gameplaying;
                }
                break;
            }
        case gamelost: 
            {                
                starVortex.UpdateFrame();
                int o = (seconds & 0x1) ? frameNr * 4 : (59 - frameNr) * 4;
                gfx.PrintXY( 290,(VER_SCREEN_RES - neuropolXBMP.height()) / 2,
                    "Game Over",o,neuropolXBMP );
                const char *helpStr = "Press Space or click left button to continue";
                gfx.PrintXY(
                    (HOR_SCREEN_RES - strlen( helpStr ) * font.width()) / 2,
                    VER_SCREEN_RES - font.height() - 6,helpStr );
                if ( spacePressed || leftButtonPressed ) gameState = gamemenu;
                break;
            }
        case gamemenu:
            {
                copperBar.updateFrame();
                DoMainMenu();
                break;
            }
        case gamechooselevel:
            {                
                copperBar.updateFrame();
                DoChooseLevelMenu();
                break;
            }                 
        case gameleveldesigner:
            {
                if( levelDesigner.levelDesignerModeActive() )
                {
                    DrawLevelDesigner();
                    LevelDesignerProcessInput();
                } else {
                    DrawBlockDesigner();
                    BlockDesignerProcessInput();
                }
                break;
            }
        case gameplaying:
            {
                arkanoid.setPaddlePosition( mouse.GetMouseX() );
                arkanoid.UpdateGameFrame();
                DrawPlayField();
                if ( arkanoid.NrOfActiveBalls() == 0 )
                {
                    levelDesigner.inGameLevel.playSound( SOUND_LIVE_LOST );
                    if ( livesLeft <= 0 ) gameState = gamelost;
                    else {
                        powerUpLogic.ResetPowerUps();
                        arkanoid.resetPaddle();
                        arkanoid.resetBullets();
                        livesLeft--;
                        arkanoid.CreateBall(
                            arkanoid.getPaddleXCenter(),
                            PADDLE_Y1 - BALL_RADIUS,
                            DEFAULT_HORSPEED,DEFAULT_VERSPEED );
                        arkanoid.stopBall();
                        countDown = COUNTDOWN;
                        gameState = gamenextlevel;
                    }
                }
                if ( levelDesigner.inGameLevel.LevelFinished() )
                {
                    levelDesigner.inGameLevel.playSound( SOUND_NEXT_LEVEL );
                    bgScrollX = 0;
                    bgScrollY = 0;
                    bgScrollHorSpeed = -2;
                    bgScrollVerSpeed = -2;
                    levelDesigner.setLevelNr( levelDesigner.getCurrentLevelNr() + 1 );     // implement protection agains last level 99!
                        levelDesigner.loadInGameLevel( levelDesigner.getCurrentLevelNr() );
                    if ( levelDesigner.inGameLevel.LevelFinished() ) gameState = gamelost; // reached last level
                    else {
                        powerUpLogic.ResetPowerUps();
                        arkanoid.resetPaddle();
                        arkanoid.resetBullets();
                        arkanoid.DeActivateAllBalls();
                        arkanoid.CreateBall(
                            arkanoid.getPaddleXCenter(),
                            PADDLE_Y1 - BALL_RADIUS,
                            DEFAULT_HORSPEED,DEFAULT_VERSPEED );
                        arkanoid.stopBall();
                        countDown = COUNTDOWN;
                        gameState = gamenextlevel;
                    }
                } else { 
                    if ( arkanoid.isGunPad() && rightButtonPressed )
                        arkanoid.fireBullets();
                }                
                if ( spacePressed )
                {
                    gameState = gamepaused;
                    break;
                }
                break;
            }
        case gamecredits:
            {
                starVortex.UpdateFrame();
                DrawCredits();
                if ( spacePressed || leftButtonPressed ) gameState = gamemenu;
                break;
            }
    }   
}