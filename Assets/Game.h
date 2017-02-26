/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	Game.h																				  *
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
#pragma once

/*

    Story:

    - I started to make this game in the second half of May 2016. I wanted
      to make a simple breakout game with realistic bounce effects, so the
      ball would really have to bounce off sharp edges in a realistic way.
      The collision detection and collision correction routines were quite
      difficult to make and still are not 100% perfect but close enough. 
      The smaller the ball, the bigger the error margin. I completely 
      remade the collision detection routines three times and it is one of
      the more challenging things I've done so far in programming. I get
      it now why Chili said in one of the original tutorials that he 
      "didn't like it so much". Understatement of the year lol.
      I got so bored with it that I started to make a level designer. Which
      then became more and more sophisticated. I kept adding features and
      by the time I was done I had to rewrite the rest of the game to 
      accomodate for the complex new levels I made. The effects in the 
      menu and game over screen were also done so I could put my mind to
      something else.
      I am still rather pleased with the result and I'll hope people will 
      experiment a bit with it. You can increase the resolution for example
      in globaldefs.h or change the balls' radius or speed.
      The engine was difficult to make but it is quite versatile: the balls
      can be both big and small, they can move quite fast (20 pixels per 
      frame should be no problem) and quite slow (less than 1 pixel per 
      frame).
      The source code is quite messy unfortunately. This is because I am
      a very, very impatient guy :/ that lives for instant gratification.
      I can recommend everyone to take on a slightly bigger project since
      it confronts you with problems that you'll never encounter in 
      smaller programs. Like writing ugly code is not an option and will 
      come back to bite you.
      Inspiration for this game was taken mostly from the original arkanoid
      and from Beatball.
    - The game is not 100% finished, it has no hiscore for instance. Some
      functionality is a bit crude. But the essential is there.

    What I learned:

    - A program is designed on paper and not behind the computer. Especially
      if you are not that experienced.
    - Starting out with a cessna in mind only to decide halfway that what 
      you really want is boeing 747 will make you built three planes instead
      of one, and having to redo all your work
    - Writing code quick and dirty is not an option really once your project
      starts to be a little bigger
    - making your classes fool proof is a big plus when you need to debug,
      at the very least with assertions
    - collision detection routines are made of pure hatred, as Chili put so 
      aptly

    Features:

    - Level Designer & simple block Designer
    - Scrolling backgrounds
    - shadows
    - powerups
    - Real circle + rectangle collision detection & bounce geometry
    - Engine can handle big or small balls, low or high speeds, over a 100
      balls simultaneously if necessary
    - Sound effects from the original Arkanoid for that arcade feel ;)
    - You can play with just the exe and the font files, if a level data file
      is missing it'll create one.

    What the game misses:

    - moving targets
    - enemies such as in the original arkanoid
    - some power ups I was too lazy to put in (sticky pad)
    - blocks that are in a different shape like a triangle (engine can't handle
      that right know)
    - Proper mouse controls: if the mouse cursor is not on the screen you
      can't control the paddle any more
    - the program uses the frame counter as a timer, making it easier  to play
      on systems that are too slow to run the game ;)

    *** Instructions (Should be mostly pretty self-explanatory) ***

    Level Designer:

    - click a block in the block library to start "drawing" the level with
      this block. You can drag the block library with the left mouse button.
    - Right-click on a block in the playfield to add a power-up to that block. 
      It'll appear once the block gets destroyed in the game.
    - Hold the space bar down to remove blocks on the cursor location.
      The shape of the cursor will change accordingly when doing so.
    - You can change the background in the right bottom corner. Background
      files should have the name structure "background??.bmp" where ?? is a 
      number between 00 and 99 (note the leading zero). 
      To change the direction in which the background scrolls, click the 
      checkerboard pattern repeatedly.
    - To change the level titles just edit the "leveltitles.txt" file. Note 
      that the title can not be longer than the default title.

    Block Designer:

    - Design your own blocks in the block designer! They can have any size but
      must be rectangular in shape and delimited by the color 0x0 (full black).
      Make your own block librairies in whatever drawing program and save them 
      as blockSkinLibrary??.bmp where ?? is between 00 and 99 (note the leading
      zero).
    - The block designer actually has an inline help function so you should be
      just fine there.

    Gameplay:

    - Some blocks can only be destroyed by a golden ball, and do not need to be
      destroyed to finish the level. Other blocks just need several hits before
      they break. 
    - Catch the powerups when they drop from the playfield but 
      don't forget to keep track of your ball(s) :-]
    - If you have the gun powerup, you can shoot bullets with the right mouse
      button. Persistent blocks will not break when hit by a bullet.
     

    Credits:

    - DirectX Framework: the awesome Chili
    - Crappy programming & general compiler abuse: Byteraver
    - Small font "Courier": The Humble Guys Demo maker
    - The Neuropol X by Raymond Larabie () / Typodermic Fonts Inc was 
      downloaded from 1001fonts.com
    - Space backgrounds were downloaded from wallpaperscraft.com
    - Repeating backgrounds were downloaded from https://1-background.com/,
      amongst others
    - Sound effects were downloaded from:
      https://www.sounds-resource.com/nes/arkanoid/sound/3698/
      (They're the original Arkanoid sound effects)
*/


#include "globaldefs.h"
#include "EffectBar.h"
#include "EffectStarVortex.h"
#include "Arkanoid.h"
#include "D3DGraphics.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Sound.h"
#include "font.h"
#include "EvoConsole.h"
#include "loadbmp.h"
#include "LevelDesigner.h"
#include <stdlib.h>
#include <assert.h>

#define COLOR_BROWN         D3DCOLOR_XRGB( 160,100,0 )
#define COLOR_DARKBROWN     D3DCOLOR_XRGB( 120,75,0 )
#define COLOR_GRAY          D3DCOLOR_XRGB( 180,180,180 )
#define COLOR_DARKGRAY      D3DCOLOR_XRGB( 100,100,100 )
#define COLOR_WHITE         D3DCOLOR_XRGB( 250,250,250 )
#define COLOR_GREEN         D3DCOLOR_XRGB( 100,250,100 )
#define COLOR_YELLOW        D3DCOLOR_XRGB( 250,250,50 )
#define COLOR_RED           D3DCOLOR_XRGB( 250,50,50 )
#define COLOR_BLACK         D3DCOLOR_XRGB( 0,0,0 )
#define COLOR_CYAN          D3DCOLOR_XRGB( 50,250,250 )
#define COLOR_BLUE          D3DCOLOR_XRGB( 50,50,250 )

#define CURSOR_COLOR        COLOR_RED
#define GRID_COLOR          COLOR_RED

#define MENU_START_GAME     0
#define MENU_CHOOSE_LEVEL   1
#define MENU_LEVELDESIGNER  2
#define MENU_CREDITS        3
#define MENU_QUIT           4
#define NR_OF_MENUS         5

#define FRAME_RATE          60
#define COUNTDOWN           (VER_SCREEN_RES)

enum GameStates     {   gamepaused, gamemenu, gameplaying, 
                        gamecredits, gamelost, gamechooselevel,
                        gamenextlevel, gameleveldesigner };
enum KeyStates      { keyisup, keyisdown };
enum ButtonState    { buttonisup, buttonisdown };

class KbdState
{
public:
    KeyStates   keyStateEnter;
    KeyStates   keyStateUp;
    KeyStates   keyStateDown;
    KeyStates   keyStateLeft;
    KeyStates   keyStateRight;
    KeyStates   keyStateSpace;
    KbdState ()
    {
        keyStateEnter = keyisup;
        keyStateUp    = keyisup;
        keyStateDown  = keyisup;
        keyStateLeft  = keyisup;
        keyStateRight = keyisup;
        keyStateSpace = keyisup;
    }
};

class MouseState
{
public:
    int         mx;
    int         my;
    ButtonState leftMouseButton;
    ButtonState rightMouseButton;
    MouseState ()
    {
        leftMouseButton  = buttonisup;
        rightMouseButton = buttonisup;
    }
};

class Game
{
public:
	Game( HWND hWnd,const KeyboardServer& kServer,const MouseServer& mServer );
    ~Game();
	void Go();
private:
	void ComposeFrame();
	/********************************/
	/*  User Functions              */
    void DrawBlockLibrary();
    void DrawPlayField();
    void DrawGrid();
    void DrawScrollLogo();
    void DrawPowerUpMenu();
    void DrawBlockDesigner();
    void DrawLevelDesigner();
    void DoMainMenu();
    void DoChooseLevelMenu();
    void LevelDesignerProcessInput();
    void BlockDesignerProcessInput();
    void DrawCredits();
    void ShowHelp( const char *text[] );
    void ProcessKeyBoard();
    void ProcessMouse();
	/********************************/
private:
	D3DGraphics    gfx;
	KeyboardClient kbd;
	MouseClient    mouse;
	DSound         audio;
	/********************************/
	/*  User Variables              */
#ifdef DEBUG_MODE
    EvoConsole      evoConsole;
    bool            pausegame;
#endif
    Font            font;
    Font            neuropolXBMP;
    Arkanoid        arkanoid;
    PowerUpLogic    powerUpLogic;
    LevelDesigner   levelDesigner;
    CopperBar       copperBar;
    StarVortex      starVortex;
    int             puMenuX;        // for the power up menu
    int             puMenuY;
    int             puMenuWidth;
    int             puMenuHeight;
    int             puNrOfPowerUps;
    int             puNrOfBlock;
    Rect            puMenuRect;
    bool            puMenuActive;
    int             bgScrollX;      // for scrolling background
    int             bgScrollY;
    int             bgScrollHorSpeed;
    int             bgScrollVerSpeed;
    int             mouseXDragStart;  // for mouse drag function
    int             mouseYDragStart;
    bool            mouseDragging;
    bool            mouseDragEnded;
    bool            leftButtonPressed,
                    rightButtonPressed;
    bool            enterPressed,
                    upPressed,
                    downPressed,
                    leftPressed,
                    rightPressed,
                    spacePressed;
    int             frameNr;
    int             seconds;
    int             countDown;
    int             livesLeft;
    int             gameLevel;
    int             gameScore;
    GameStates      gameState;
    KbdState        lastKbdState;
    KbdState        currentKbdState;
    MouseState      lastMouseState;
    MouseState      currentMouseState;
	/********************************/
};