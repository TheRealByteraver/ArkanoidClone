#pragma once

#include "globaldefs.h"
#include "D3DGraphics.h"
#include "Sound.h"
#include <d3d9.h>
#include <assert.h>
#include <math.h>

/*
    COLLISION_GRID_MAX_CELLS is the amount of cells the ball can touch in one
    frame. This value is choosen rather arbitrarily, for a speed lower then 20
    The number of 9 will hopefully be enough ;)
*/
#define COLLISION_GRID_MAX_CELLS    9 
#define PERSISTENT_BLOCK            5    // This block can't be destroyed
#define SPECIAL_BLOCK_BOTTOMWALL    0
#define SPECIAL_BLOCK_LEFTWALL      1
#define SPECIAL_BLOCK_RIGHTWALL     2
#define SPECIAL_BLOCK_TOPWALL       3
#define SPECIAL_BLOCK_PADDLE        4
#define SPECIAL_BLOCKS              5    // left, top, bottom, right wall & paddle
#define BACKGROUND_STATIC           0
#define BACKGROUND_SCROLL_UP        1
#define BACKGROUND_SCROLL_DOWN      2
#define BACKGROUND_SCROLL_LEFT      3
#define BACKGROUND_SCROLL_RIGHT     4
#define BACKGROUND_SCROLL_RANDOM    5
#define BACKGROUND_SCROLL_TYPES     6
#define MAX_BLOCKS_PER_LEVEL        500
#define BALL_RADIUS                 5
#define DEFAULT_HORSPEED            3.0f
#define DEFAULT_VERSPEED            -9.0f
#define NR_OF_LEVELS                1
#define EMPTY_SPACE                 0
#define PADDLE_STANDARD_WIDTH       ((HOR_SCREEN_RES /  9) & 0xFFFE) // must be even
#define PADDLE_SMALL_WIDTH          ((HOR_SCREEN_RES / 12) & 0xFFFE) // must be even
#define PADDLE_BIG_WIDTH            ((HOR_SCREEN_RES /  7) & 0xFFFE) // must be even
#define PADDLE_Y1                   (VER_SCREEN_RES - 24)
#define PADDLE_HEIGHT               8              
#define PADDLE_Y2                   (PADDLE_Y1 + PADDLE_HEIGHT)
#define PADDLE_COLOR                0xFFFFFF // 0xA0A0A0
#define PADDLE_GUN_COLOR            0xFF4040
#define MAX_BALLS                   16
#define EPSILON                     0.01f // avoid floating point accidents
#ifndef PI
    #define PI          3.141592f
#endif
// Power up related defines
#define MAX_POWER_UPS               16
#define POWERUP_WIDTH               26
#define POWERUP_HEIGHT              20
#define POWERUP_NOPOWERUP           0
#define POWERUP_DOUBLEBALL          1
#define POWERUP_MULTIBALL           2
#define POWERUP_BALLSTORM           3
#define POWERUP_GOLDENBALL          4
#define POWERUP_GUNS                5
#define POWERUP_BIGPAD              6
#define POWERUP_SMALLPAD            7
#define POWERUP_DOUBLEBALL_COLOR    0xFF00FF
#define POWERUP_MULTIBALL_COLOR     0x00FFFF
#define POWERUP_BALLSTORM_COLOR     0xFFFF00
#define POWERUP_GOLDENBALL_COLOR    0x00FF00
#define POWERUP_GUNS_COLOR          0xFF0000
#define POWERUP_BIGPAD_COLOR        0x0000FF
#define POWERUP_SMALLPAD_COLOR      0x00FF00
#define POWERUP_DOUBLEBALL_CHAR     'D'
#define POWERUP_MULTIBALL_CHAR      'T'
#define POWERUP_BALLSTORM_CHAR      'S'
#define POWERUP_GOLDENBALL_CHAR     'O'
#define POWERUP_GUNS_CHAR           'G'
#define POWERUP_BIGPAD_CHAR         'L'
#define POWERUP_SMALLPAD_CHAR       '-'
// bullet related defines
#define MAX_BULLETS                 20
#define BULLET_SPEED                12
#define BULLET_HEIGHT               16
#define BULLET_COLOR                0xFF20D0
// defines for the sounds:
#define SOUND_PADDLE_HIT            0
#define SOUND_BLOCK_DIES            1
#define SOUND_BLOCK_HIT             2
#define SOUND_FIRE_BULLET           3
#define SOUND_NEXT_LEVEL            4
#define SOUND_LIVE_LOST             5
#define NR_OF_SOUNDS                6
#define SOUND_PADDLE_HIT_FILENAME   "Arkanoid SFX (6).wav"
#define SOUND_BLOCK_DIES_FILENAME   "Arkanoid SFX (7).wav"
#define SOUND_BLOCK_HIT_FILENAME    "Arkanoid SFX (8).wav"
#define SOUND_FIRE_BULLET_FILENAME  "Arkanoid SFX (3).wav"
#define SOUND_NEXT_LEVEL_FILENAME   "Arkanoid SFX (9).wav"
#define SOUND_LIVE_LOST_FILENAME    "Arkanoid SFX (10).wav"


enum backgroundscrolltype { staticbackground, scrollup, scrolldown,
                            scrollleft, scrollright, randomscroll };
enum powerup            { nopowerup, doubleball, multiball, 
                          ballstorm, goldenball, guns, bigpad, smallpad };
enum balltype           { normalball, destructionball };
enum padtype            { normalpad, shortpad, largepad, gunpad };
enum spindirection      { clockwise, counterclockwise };
enum ballhitlocation    { nohit, left, right ,
                          topleft   , top    , topright  , 
                          bottomleft, bottom , bottomright };

class InGameBlockData
{
public:
    bool    isAlive;
    int     hitsToDestruct;
    int     xOrig;
    int     yOrig;
    int     x2;
    int     y2;
    powerup powerUp;
    Bitmap *skin;
public:
    Rect    getRect()
    {
        return Rect( xOrig,yOrig,x2,y2 );
    }
};

class InGameLevelData
{
public:
    InGameLevelData()
    {
        levelTitle = "";
        backgroundBMP = nullptr;
        backGroundScrolltype = staticbackground;
        //scrollingBackGround = true; // temp debug
        nrOfBlocks = 0;
        blocks = nullptr;
        // init blocks that represent the walls & paddle:
        dummyBmp.createEmptyBMP( 0,0,0x0 );       
        paddle.hitsToDestruct = PERSISTENT_BLOCK;
        paddle.isAlive = true;
        paddle.skin = &dummyBmp;
        paddle.xOrig = (HOR_SCREEN_RES - PADDLE_STANDARD_WIDTH - PADDLE_HEIGHT) / 2;
        paddle.yOrig = PADDLE_Y1;
        paddle.x2 = paddle.xOrig + PADDLE_STANDARD_WIDTH + PADDLE_HEIGHT;
        paddle.y2 = PADDLE_Y2;
        bottomWall.hitsToDestruct = PERSISTENT_BLOCK;
        bottomWall.isAlive = true;
        bottomWall.skin = &dummyBmp;        
        bottomWall.xOrig = -100;
        bottomWall.yOrig = VER_SCREEN_RES;
        bottomWall.x2 = HOR_SCREEN_RES + 100;
        bottomWall.y2 = VER_SCREEN_RES + 100;
        leftWall.hitsToDestruct = PERSISTENT_BLOCK;
        leftWall.isAlive = true;
        leftWall.skin = &dummyBmp;
        leftWall.xOrig = -100;
        leftWall.yOrig = 0;
        leftWall.x2 = -1;
        leftWall.y2 = VER_SCREEN_RES - 1;
        rightWall.hitsToDestruct = PERSISTENT_BLOCK;
        rightWall.isAlive = true;
        rightWall.skin = &dummyBmp;
        rightWall.xOrig = HOR_SCREEN_RES;
        rightWall.yOrig = 0;
        rightWall.x2 = HOR_SCREEN_RES + 100;
        rightWall.y2 = VER_SCREEN_RES - 1;
        topWall.hitsToDestruct = PERSISTENT_BLOCK;
        topWall.isAlive = true;
        topWall.skin = &dummyBmp;
        topWall.xOrig = -100;
        topWall.yOrig = -100;
        topWall.x2 = HOR_SCREEN_RES + 100;
        topWall.y2 = -1;
    }
    ~InGameLevelData()
    {
        if ( blocks != nullptr ) delete[] blocks;
    }
    void                setPaddlePosition( int x1,int x2 )
    {
        assert( blocks != nullptr );
        blocks[SPECIAL_BLOCK_PADDLE].xOrig = x1;
        blocks[SPECIAL_BLOCK_PADDLE].x2    = x2;
    }
    bool                LevelFinished()
    {
        for ( int i = SPECIAL_BLOCKS; i < nrOfBlocks; i++ )
            if( blocks[i].isAlive &&
              ( blocks[i].hitsToDestruct != PERSISTENT_BLOCK ) )
                return false;
        return true;
    }
    void                playSound( int soundFxNr )
    {
        assert( soundFxNr >= 0 );
        assert( soundFxNr < NR_OF_SOUNDS );
        sounds[soundFxNr].Play( -100 );
    }
    char               *levelTitle;
    Bitmap             *backgroundBMP;
    backgroundscrolltype
                        backGroundScrolltype;
    int                 nrOfBlocks;
    InGameBlockData    *blocks;
    InGameBlockData     leftWall;
    InGameBlockData     rightWall;
    InGameBlockData     topWall;
    InGameBlockData     bottomWall;
    InGameBlockData     paddle;
    Sound               sounds[NR_OF_SOUNDS];
private:
    Bitmap              dummyBmp;
};

class HitLocationData
{
public:
    int                 blockNr;    // the block nr in the levels' block list
    ballhitlocation     hitLocation;// which side or corner of the block we hit
    float               distance;   // distance travelled from start to collision
    float               Bx;         // the coordinates of the ball at the time
    float               By;         // of the collision
    float               xHit;       // the precise location of the collisions'
    float               yHit;       // location on the block or ball
};

/*

*) power ups should have shadows as well...

    Power up types:
    nopowerup -> restore normal ball or paddle
    letter: N

    doubleball -> gives you one extra ball, in a direction slightly off course
    compared to the main ball
    letter: D

    multiball -> gives you two extra balls, one left and one right of the 
    original or main ball
    letter: T

    ballstorm -> gives you 10 extra balls that go off in a star-like fashion
    from the main ball.
    letter: S

    goldenball -> gives you a ball that doesn't bounce off blocks but runs 
    straight through them, including the persistent blocks
    letter: O
    
    guns -> gives you guns that can shoot non persistent blocks
    letter: G

    large pad -> gives you a larger paddle
    letter: L
*/

class Bullet
{
public:
    bool            isAlive;
    int             x;
    int             y;
};

class Bullets
{
public:
    void            Init()
    {
        for ( int i = 0; i < MAX_BULLETS; i++ ) bullets[i].isAlive = false;
    }
    int             getSlot()
    {
        int i;
        for ( i = 0; i < MAX_BULLETS; i++ )
        {
            if ( bullets[i].isAlive == false ) return i;
        }
        return -1;
    }
    void            CreateBullet( int x )
    {
        assert( x >= 1 );
        assert( x <= HOR_SCREEN_RES - 1 );
        int i = getSlot();
        if ( i < 0 ) return;
        bullets[i].isAlive = true;
        bullets[i].x = x;
        bullets[i].y = PADDLE_Y1 - 2 - BULLET_HEIGHT;
    }
    int             getBulletX( int bullet )
    {
        assert( bullet >= 0 );
        assert( bullet < MAX_BULLETS );
        return bullets[bullet].x;
    }
    int             getBulletY( int bullet )
    {
        assert( bullet >= 0 );
        assert( bullet < MAX_BULLETS );
        return bullets[bullet].y;
    }
    void            setBulletX( int bullet,int x )
    {
        assert( bullet >= 0 );
        assert( bullet < MAX_BULLETS );
        assert( x >= 1 );
        assert( x < HOR_SCREEN_RES - 1 );
        bullets[bullet].x = x;
    }
    void            setBulletY( int bullet,int y )
    {
        assert( bullet >= 0 );
        assert( bullet < MAX_BULLETS );
        assert( y >= 0 );
        assert( y < PADDLE_Y1 - 2 - BULLET_HEIGHT );
        bullets[bullet].y = y;
    }
    bool            isBulletAlive( int bullet )
    {
        assert( bullet >= 0 );
        assert( bullet < MAX_BULLETS );
        return bullets[bullet].isAlive;        
    }
    void            killBullet( int bullet ) { bullets[bullet].isAlive = false; }
private:
    Bullet          bullets[MAX_BULLETS];
};

class PowerUp
{
public:
    PowerUp() { isAlive = false; }
    bool            isAlive;
    powerup         type;
    int             x;
    int             y;
    int             color;
    char            symbol;
    int             speed;
};

class PowerUpLogic
{
public:
    PowerUp         powerUps[MAX_POWER_UPS];
    void            createNew( int x,int y,powerup powerUpType )
    {
        int newSlot = getFreeSlot();
        if ( newSlot < 0 ) return;
        PowerUp& powerUp = powerUps[newSlot];
        powerUp.isAlive = true;
        powerUp.type = powerUpType;
        if( x + POWERUP_WIDTH >= HOR_SCREEN_RES )
                powerUp.x = HOR_SCREEN_RES - POWERUP_WIDTH - 1;
        else    powerUp.x = x;
        if ( x < 0 ) powerUp.x = 0;
        powerUp.y = y;
        powerUp.speed = 2 + (rand() % 40) / 10;
        switch ( powerUp.type )
        {
            case doubleball:
            {
                powerUp.color = POWERUP_DOUBLEBALL_COLOR;
                powerUp.symbol = POWERUP_DOUBLEBALL_CHAR;
                break;
            }
            case multiball:
            {
                powerUp.color = POWERUP_MULTIBALL_COLOR;
                powerUp.symbol = POWERUP_MULTIBALL_CHAR;
                break;
            }
            case ballstorm:
            {
                powerUp.color = POWERUP_BALLSTORM_COLOR;
                powerUp.symbol = POWERUP_BALLSTORM_CHAR;
                break;
            }
            case goldenball:
            {
                powerUp.color = POWERUP_GOLDENBALL_COLOR;
                powerUp.symbol = POWERUP_GOLDENBALL_CHAR;
                break;
            }
            case guns:
            {
                powerUp.color = POWERUP_GUNS_COLOR;
                powerUp.symbol = POWERUP_GUNS_CHAR;
                break;
            }
            case bigpad:
            {
                powerUp.color = POWERUP_BIGPAD_COLOR;
                powerUp.symbol = POWERUP_BIGPAD_CHAR;
                break;
            }
            case smallpad:
            {
                powerUp.color = POWERUP_SMALLPAD_COLOR;
                powerUp.symbol = POWERUP_SMALLPAD_CHAR;
                break;
            }
            default: powerUp.isAlive = false;
        }
    }
    void            updatePowerUps()
    {
        for ( int i = 0; i < MAX_POWER_UPS; i++ )
            if ( powerUps[i].isAlive )
            {
                powerUps[i].y += powerUps[i].speed;
                if ( powerUps[i].y + POWERUP_HEIGHT >= VER_SCREEN_RES ) 
                    powerUps[i].isAlive = false;
            }
    }
    void            ResetPowerUps()
    {
        for ( int i = 0; i < MAX_POWER_UPS; i++ ) powerUps[i].isAlive = false;
    }
private:
    int             getFreeSlot()
    {
        int i;
        for ( i = 0; i < MAX_POWER_UPS; i++ )
            if ( !powerUps[i].isAlive ) break;
        if ( i < MAX_POWER_UPS ) return i;
        else return -1;
    }
};

class Paddle
{
public:
    int             xCenter;
    int             yCenter;
    int             xMin;
    int             xMax;
    int             width;
    int             x1;
    int             x2;
    int             y1;
    int             y2;
    padtype         paddleType;
    void            Init()
    {
        paddleType = normalpad;
        xCenter = HOR_SCREEN_RES / 2;
        setPaddleWidth( PADDLE_STANDARD_WIDTH );
        y1 = PADDLE_Y1;
        y2 = PADDLE_Y2;
        yCenter = y1 + PADDLE_HEIGHT / 2;
        setPosition( xCenter );
    }
    void            setPaddleWidth( int w )
    {
        width = w + PADDLE_HEIGHT; // paddle height for the round corners
        xMin = width / 2;
        xMax = HOR_SCREEN_RES - 1 - width / 2;
        setPosition( xCenter );
    }
    void            setPosition( int x )
    {
        xCenter = x;
        if ( xCenter < xMin ) xCenter = xMin;
        if ( xCenter > xMax ) xCenter = xMax;
        x1 = xCenter - width / 2 + PADDLE_HEIGHT / 2;
        x2 = x1 + width - PADDLE_HEIGHT;
    }
};

class Ball
{
private:
#ifdef DEBUG_MODE
public:
    bool            freeze;         
    D3DGraphics     *gfx;           
#endif
    float           m1;             // y = m1 * x + b1 is the equation of the
    float           b1;             // line of travel of the ball
    float           speed;
    float           bTop;           // the "b" in y = mx + b trajectory of ball(x,y - radius)
    float           bBottom;        // the "b" in y = mx + b trajectory of ball(x,y + radius)
    float           bLeft;          // the "b" in y = mx + b trajectory of ball(x - radius,y)
    float           bRight;         // the "b" in y = mx + b trajectory of ball(x + radius,y)
    int             lastBlockHit;
    Rect            collisionArea;
    int             blockHitList[MAX_BLOCKS_PER_LEVEL];
    HitLocationData hitLocationData[COLLISION_GRID_MAX_CELLS];
    InGameLevelData *inGameLevelData;
    PowerUpLogic    *powerUpLogic;
public:
    int             color;
    bool            isActive;
    bool            isDestructionBall;
    float           ballX;          // ball position
    float           ballY;
    float           radius;         // ball radius
    float           horSpeed;       // positive == to the right
    float           verSpeed;       // positive == downwards
public:
    Ball();
    void            Init( InGameLevelData *inGameLevelData_,
                          PowerUpLogic *powerUpLogic_ )
    {
        inGameLevelData = inGameLevelData_;
        powerUpLogic = powerUpLogic_;
    }
#ifdef DEBUG_MODE
    void            Init( D3DGraphics *gfx_,InGameLevelData *inGameLevelData_,
                          PowerUpLogic *powerUpLogic_ )
    {
        gfx = gfx_;
        inGameLevelData = inGameLevelData_;
        powerUpLogic = powerUpLogic_;
    }
    #endif
    void            UpdatePosition();
    void            stop() { speed = 0.0f; }
    void            release() 
    {
        setSpeed( horSpeed,verSpeed,ballX,ballY );
        //speed = sqrt( verSpeed  * verSpeed + horSpeed * horSpeed );
    }
    float           getSpeed() { return speed; }
    void            setSpeed( float horSpeed_,float verSpeed_,float ballX_,float ballY_ );
private:
    void            sortCollisions( HitLocationData *hitLocationData,const int size );
    Rect            getCollisionArea( int xStart,int yStart,int xEnd,int yEnd );
    int             populateBlockHitList( const Rect& collisionArea );
    void            setTrajectories( const float ballX_,const float ballY_ );
    ballhitlocation getCollisionLocation( float ballX_,float ballY_,const Rect& blockRect );
    bool            isHitConfirmed( float ballX_,float ballY_,
                            HitLocationData &hitLData,const Rect& blockRect );
    void            correctCollision( float ballX_,float ballY_,
                            HitLocationData &hitLData,const Rect& blockRect );
    void            processCollision( HitLocationData &hitLData );
};

class Arkanoid
{
public:
    Arkanoid() { nrOfActiveBalls = 0; }
    void            Init( InGameLevelData *inGameLevelData_,
                          PowerUpLogic *powerUpLogic_ )
    {
        inGameLevelData = inGameLevelData_;
        powerUpLogic = powerUpLogic_;
        for ( int i = 0; i < MAX_BALLS; i++ )
            balls[i].Init( inGameLevelData_,powerUpLogic_ );
        paddle.Init();
    }
#ifdef DEBUG_MODE
    void            Init( D3DGraphics *gfx_,InGameLevelData *inGameLevelData_,
                          PowerUpLogic *powerUpLogic_ )
    {
        inGameLevelData = inGameLevelData_;
        powerUpLogic = powerUpLogic_;
        for ( int i = 0; i < MAX_BALLS; i++ )
            balls[i].Init( gfx_,inGameLevelData_,powerUpLogic_ );
        paddle.Init();
    }
#endif
    void            UpdateGameFrame()
    {
        // Update ball positions:
        nrOfActiveBalls = 0;
        for ( int i = 0; i < MAX_BALLS; i++ )
            if ( balls[i].isActive )
            {
                balls[i].UpdatePosition();
                nrOfActiveBalls++;
            }
        // Update bullet positions:
        // (bullet width = 3 pixels)
        for ( int i = 0; i < MAX_BULLETS; i++ )
        {
            if ( bullets.isBulletAlive( i ) == false ) continue;
            int x = bullets.getBulletX( i );
            int y = bullets.getBulletY( i );
            y -= BULLET_SPEED;
            if ( y < 0 )
            {
                bullets.killBullet( i );
                continue;
            }
            bullets.setBulletY( i,y );
            for ( int b = SPECIAL_BLOCKS; b < inGameLevelData->nrOfBlocks; b++ )
            {
                if( inGameLevelData->blocks[b].isAlive == false ) continue;
                if( inGameLevelData->blocks[b].xOrig > x + 1 ) continue;
                if( inGameLevelData->blocks[b].x2    < x - 1 ) continue;
                if( inGameLevelData->blocks[b].y2    < y     ) continue;
                int& htd = inGameLevelData->blocks[b].hitsToDestruct;
                if ( htd != PERSISTENT_BLOCK ) htd--;
                bullets.killBullet( i );
                if ( htd <= 0 ) 
                {
                    powerup powerUp = inGameLevelData->blocks[b].powerUp;
                    if ( powerUp != nopowerup )
                    {
                        powerUpLogic->createNew( x,y,powerUp );
                    }
                    inGameLevelData->blocks[b].isAlive = false;
                    //inGameLevelData->playSound( SOUND_BLOCK_DIES ); // too much ;)
                }
                b = inGameLevelData->nrOfBlocks;
            }
        }
        /*
        take care of powerups:
        */
        powerUpLogic->updatePowerUps();
        int iBall = getActiveBall();
        if ( iBall < 0 ) return;
        Ball& origBall = balls[iBall];
        for ( int i = 0; i < MAX_POWER_UPS; i++ )
        {
            PowerUp& powerUp = powerUpLogic->powerUps[i];
            if ( !powerUp.isAlive ) continue;
            if ( powerUp.y + POWERUP_HEIGHT < PADDLE_Y1 ) continue;
            int paddleX1 = inGameLevelData->blocks[SPECIAL_BLOCK_PADDLE].xOrig;
            int paddleX2 = inGameLevelData->blocks[SPECIAL_BLOCK_PADDLE].x2;
            if ( powerUp.x + POWERUP_WIDTH < paddleX1 ) continue;
            if ( powerUp.x > paddleX2 ) continue;
            powerUp.isAlive = false;
            switch ( powerUp.type )
            {
                case doubleball:
                {
                    float speed = origBall.getSpeed();
                    float alfa = asin( origBall.verSpeed / speed ) + PI / 12.0f;
                    CreateBall(
                        (int)origBall.ballX,(int)origBall.ballY,
                        speed * cos( alfa ),speed * sin( alfa ) );
                    break;
                }
                case multiball:
                {
                    float speed = origBall.getSpeed();
                    float alfa = asin( origBall.verSpeed / speed ) + PI / 12.0f;
                    CreateBall(
                        (int)origBall.ballX,(int)origBall.ballY,
                        speed * cos( alfa ),speed * sin( alfa ) );
                    alfa -= PI / 6.0f;
                    CreateBall(
                        (int)origBall.ballX,(int)origBall.ballY,
                        speed * cos( alfa ),speed * sin( alfa ) );
                    break;
                }
                case ballstorm:
                {
                    const int nrofballs = 11;
                    float speed = origBall.getSpeed();
                    float alfa = asin( origBall.verSpeed / speed );
                    for ( int i = 0; i < nrofballs; i++ )
                    {
                        alfa += ( 2.0f * PI) / (nrofballs + 1);
                        CreateBall(
                            (int)origBall.ballX,(int)origBall.ballY,
                            speed * cos( alfa ),speed * sin( alfa ) );
                    }
                    break;
                }
                case goldenball:
                {
                    origBall.isDestructionBall = true;
                    origBall.color = 0xDFCF00;
                    break;
                }
                case guns:
                {
                    paddle.paddleType = gunpad;
                    paddle.setPaddleWidth( PADDLE_STANDARD_WIDTH );
                    break;
                }
                case bigpad:
                {
                    paddle.paddleType = largepad;
                    paddle.setPaddleWidth( PADDLE_BIG_WIDTH );
                    break;
                }
                case smallpad:
                {
                    paddle.paddleType = shortpad;
                    paddle.setPaddleWidth( PADDLE_SMALL_WIDTH );
                    break;
                }
                default: powerUp.isAlive = false;
            }
        }
    }
    // for paddle related stuff:
    bool            isGunPad() { return paddle.paddleType == gunpad; }
    void            setPaddlePosition( int x ) 
    { 
        paddle.setPosition( x ); 
        inGameLevelData->setPaddlePosition( paddle.x1,paddle.x2 );
    }
    Rect            getPaddleCoords() 
    { return Rect( paddle.x1,paddle.y1,paddle.x2,paddle.y2 ); }
    int             getPaddleXCenter() { return paddle.xCenter; }
    int             getPaddleYCenter() { return paddle.yCenter; }
    void            resetPaddle() { paddle.Init(); }
    // For ball related stuff:
    void            DeActivateAllBalls()
    {
        for ( int i = 0; i < MAX_BALLS; i++ ) balls[i].isActive = false;

    }
    int             NrOfActiveBalls() { return nrOfActiveBalls; }
    void            CreateBall( int x,int y,float horSpeed,float verSpeed )
    {
        int i = 0;
        for ( ; i < MAX_BALLS; i++ )
            if ( ! balls[i].isActive ) break;
        if ( i >= MAX_BALLS ) return;
        balls[i].isActive = true;
        balls[i].color = 0xFFFFFF;
        balls[i].isDestructionBall = false;
        balls[i].ballX = (float)x;
        balls[i].ballY = (float)y;
        balls[i].setSpeed( horSpeed,verSpeed,balls[i].ballX,balls[i].ballY );
    }
    int             getActiveBall()
    {
        int i = 0;
        for ( ; i < MAX_BALLS; i++ )
            if ( balls[i].isActive ) break;
        if ( i < MAX_BALLS ) return i;
        else return -1;
    }
    void            stopBall()
    {
        int i = getActiveBall();
        balls[i].stop();
    }
    void            releaseBall()
    {
        int i = getActiveBall();
        balls[i].release();
    }
    Ball&           getBall( int iBall )
    {
        assert( iBall >= 0 );
        assert( iBall < MAX_BALLS );
        return balls[iBall];
    }
    // For bullet related stuff:
    void            getBulletXY( int bullet,int& x,int& y )
    {
        x = bullets.getBulletX( bullet );
        y = bullets.getBulletY( bullet );
    }
    bool            isBulletAlive( int bullet )
    {
        return bullets.isBulletAlive( bullet );
    }
    void            resetBullets() { bullets.Init(); }
    void            fireBullets()
    {
        bullets.CreateBullet( paddle.x1 + 3 );
        bullets.CreateBullet( paddle.x2 - 3 );
        inGameLevelData->playSound( SOUND_FIRE_BULLET );
    }
private:
    Paddle          paddle;
    int             nrOfActiveBalls;
    InGameLevelData *inGameLevelData;
    PowerUpLogic    *powerUpLogic;
    Ball            balls[MAX_BALLS];
    Bullets         bullets;
};

