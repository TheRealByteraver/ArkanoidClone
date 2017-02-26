#pragma once

#include "arkanoid.h"

Ball::Ball()
{
    isActive = false;
    lastBlockHit = -1;
    isDestructionBall = false;
    radius = BALL_RADIUS;
#ifdef DEBUG_MODE
    freeze = false;
#endif
}
void Ball::UpdatePosition()
{
    assert( inGameLevelData != nullptr );
    if ( speed <= 0.0f )
    {
        int w =   inGameLevelData->blocks[SPECIAL_BLOCK_PADDLE].x2 
                - inGameLevelData->blocks[SPECIAL_BLOCK_PADDLE].xOrig + 1;
        ballX = (float)
                (inGameLevelData->blocks[SPECIAL_BLOCK_PADDLE].xOrig + w / 2);
        return;
    }
    float speedLeft = speed;
    float xStartPos = ballX;
    float yStartPos = ballY;
    float newBallX;
    float newBallY;

    for ( ; speedLeft > 0.0f; )
    {
        float restSpeed = speedLeft / speed;
        newBallX = xStartPos + restSpeed * horSpeed;
        newBallY = yStartPos + restSpeed * verSpeed;
        collisionArea = getCollisionArea(
            (int)xStartPos,(int)yStartPos,(int)newBallX,(int)newBallY );

#ifdef DEBUG_MODE
        //gfx->DrawBox( collisionArea,0xFFFF ); // box in which we check for blocks
        //gfx->DrawMathLine( (int)ballX,(int)ballY,m1,100,0xFF00 );
        //gfx->DrawCircle( (int)newBallX,(int)newBallY,(int)radius,0xFF40FF );
#endif 

        int NrOfCandidates = populateBlockHitList( collisionArea );

        // check if any of the candidates is in the trajectory
        // of the ball:
        memset( hitLocationData,0,sizeof( HitLocationData ) * COLLISION_GRID_MAX_CELLS );
        for ( int i = 0; i < COLLISION_GRID_MAX_CELLS; i++ )
            hitLocationData[i].distance = 1.0e8f;
        int iCollision = 0;
        for ( int iCandidate = 0; iCandidate < NrOfCandidates; iCandidate++ )
        {
            // initialize hitLocationData
            HitLocationData &hitLData = hitLocationData[iCollision];
            hitLData.blockNr = blockHitList[iCandidate];

            // blockRect is the coords set of the block we are going to test
            Rect blockRect =
                inGameLevelData->blocks[hitLData.blockNr].getRect();

            // check which side of the block we hit:
            hitLData.hitLocation =
                getCollisionLocation( xStartPos,yStartPos,blockRect );

            // if we can't validate the hit, continue to next candidate:
            bool hitConfirmed = isHitConfirmed(
                xStartPos,yStartPos,hitLData,blockRect );
#ifdef DEBUG_MODE
            gfx->DrawBox(
                blockRect.x1 - 1,
                blockRect.y1 - 1,
                blockRect.x2 + 1,
                blockRect.y2 + 1,
                0xFF0000 + (iCandidate + 1) * 0x30 ); // collision candidates
            char buf[128];
            buf[0] = '\0';
            int dbx = 20;
            int dby = VER_SCREEN_RES - 170 + iCandidate * 18;
            if ( speedLeft < speed ) dby += 48;
            _itoa( hitLData.blockNr,buf,10 );
            strcat( buf," block nr hit: " );
            switch ( hitLData.hitLocation )
            {
                case topleft    : { strcat( buf,"topleft" );        break; }
                case topright   : { strcat( buf,"topright" );       break; }
                case bottomleft : { strcat( buf,"bottomleft" );     break; }
                case bottomright: { strcat( buf,"bottomright" );    break; }
                case top        : { strcat( buf,"top" );            break; }
                case bottom     : { strcat( buf,"bottom" );         break; }
                case left       : { strcat( buf,"left" );           break; }
                case right      : { strcat( buf,"right" );          break; }
                default         : { strcat( buf,"no hit" );         break; }
            }
            if ( hitConfirmed ) strcat( buf,", confirmed" );
            else                strcat( buf,", debunked" );
            strcat( buf,", xHit = " );
            char buf2[16];
            buf2[0] = '\0';
            _itoa( (int)hitLData.xHit,buf2,10 );
            strcat( buf,buf2 );
            strcat( buf,", yHit = " );
            _itoa( (int)hitLData.yHit,buf2,10 );
            strcat( buf,buf2 );
            //gfx->PrintXY( dbx,dby,buf );
#endif                
            if ( hitConfirmed == false ) {
                hitLData.distance = 1.0e8f;   
                hitLData.hitLocation = nohit;
                continue;
            }
            // correct the collision:  
            correctCollision( xStartPos,yStartPos,hitLData,blockRect );
#ifdef DEBUG_MODE
            /*
            gfx->DrawCircle(
                (int)hitLData.xHit,
                (int)hitLData.yHit,
                (int)5,0xFFFFFF );
            */
#endif                
            iCollision++; // next collision candidate
        }
        if ( NrOfCandidates == 0 ) break;
        // Now sort the collisions: closest block first
        sortCollisions( hitLocationData,COLLISION_GRID_MAX_CELLS );
        // Now recalculate the ball's direction (speed) / angle of travel
        HitLocationData &hitLData = hitLocationData[0];
        if ( (hitLData.hitLocation != nohit) &&
            (hitLData.distance <= speedLeft) )
        {
#ifndef GOD_MODE
            if ( hitLData.blockNr == SPECIAL_BLOCK_BOTTOMWALL ) this->isActive = false; 
#endif
            lastBlockHit = hitLData.blockNr;
            if ( isDestructionBall == false || hitLData.blockNr == SPECIAL_BLOCK_PADDLE )
            {
                processCollision( hitLData );
                xStartPos = hitLData.Bx;
                yStartPos = hitLData.By;
                if ( hitLData.blockNr == SPECIAL_BLOCK_PADDLE )
                    inGameLevelData->playSound( SOUND_PADDLE_HIT );
            }
            speedLeft -= hitLData.distance;
            int& htd = inGameLevelData->blocks[hitLData.blockNr].hitsToDestruct;
            if ( isDestructionBall == false )
            {
                if ( htd != PERSISTENT_BLOCK ) htd--;
            } else { 
                if ( hitLData.blockNr >= SPECIAL_BLOCKS ) htd = 0;
            }
            if ( htd <= 0 ) {
                powerup powerUp = inGameLevelData->blocks[hitLData.blockNr].powerUp;
                if ( powerUp != nopowerup )
                {
                    powerUpLogic->createNew(
                        (int)hitLData.Bx,
                        (int)hitLData.By,
                        powerUp );
                }
                inGameLevelData->blocks[hitLData.blockNr].isAlive = false;
                inGameLevelData->playSound( SOUND_BLOCK_DIES );
            } else if ( hitLData.blockNr >= SPECIAL_BLOCKS )
                inGameLevelData->playSound( SOUND_BLOCK_HIT );
            /*
            // for debugging purposes
            if(
            hitLData.hitLocation == bottomright ||
            hitLData.hitLocation == bottomleft ||
            hitLData.hitLocation == topright ||
            hitLData.hitLocation == topleft
            ) freeze = true; // debug !!!!!!!!!!!!!!!!!!
            */
        } else {
            break;
        }
    }
    ballX = newBallX;
    ballY = newBallY;
    // safety net:
    if ( ballX < radius )
    {
        if ( horSpeed < 0 )
        {
            horSpeed = -horSpeed;
            m1 = verSpeed / horSpeed;
            b1 = ballY - m1 * ballX;
            lastBlockHit = SPECIAL_BLOCK_LEFTWALL;
        }
    } else if ( ballX > HOR_SCREEN_RES - 1 - radius )
    {
        if ( horSpeed > 0 )
        {
            horSpeed = -horSpeed;
            m1 = verSpeed / horSpeed;
            b1 = ballY - m1 * ballX;
            lastBlockHit = SPECIAL_BLOCK_RIGHTWALL;
        }
    }
    if ( ballY < radius )
    {
        if ( verSpeed < 0 )
        {
            verSpeed = -verSpeed;
            m1 = verSpeed / horSpeed;
            b1 = ballY - m1 * ballX;
            lastBlockHit = SPECIAL_BLOCK_TOPWALL;
        }
    } else if ( ballY > VER_SCREEN_RES - 1 - radius )
    {
#ifndef GOD_MODE
        this->isActive = false; 
#else
        if ( verSpeed > 0 ) 
        {
            verSpeed = -verSpeed;
            m1 = verSpeed / horSpeed;
            b1 = ballY - m1 * ballX;
            lastBlockHit = SPECIAL_BLOCK_BOTTOMWALL;
        }
#endif
    }
}
void Ball::setSpeed( float horSpeed_,float verSpeed_,float ballX_,float ballY_ )
{
    if ( horSpeed_ >= 0.0f )
    {
        if ( horSpeed_ < EPSILON ) horSpeed = EPSILON;
        else                       horSpeed = horSpeed_;
    } else {
        if ( horSpeed_ > -EPSILON ) horSpeed = -EPSILON;
        else                         horSpeed = horSpeed_;
    }
    if ( verSpeed_ >= 0.0f )
    {
        if ( verSpeed_ < EPSILON ) verSpeed = EPSILON;
        else                       verSpeed = verSpeed_;
    } else {
        if ( verSpeed_ > -EPSILON ) verSpeed = -EPSILON;
        else                         verSpeed = verSpeed_;
    }
    m1 = verSpeed / horSpeed;
    b1 = ballY_ - m1 * ballX_;
    speed = sqrt( verSpeed  * verSpeed + horSpeed * horSpeed );
}
void Ball::sortCollisions( HitLocationData *hitLocationData,const int size )
{
    for ( int i = 0; i < size; i++ )
        for ( int j = size - 1; j > 0; j-- )
            if ( hitLocationData[j].distance < hitLocationData[j - 1].distance )
            {
                HitLocationData t = hitLocationData[j - 1];
                hitLocationData[j - 1] = hitLocationData[j];
                hitLocationData[j] = t;
            }
}
Rect Ball::getCollisionArea( int xStart,int yStart,int xEnd,int yEnd )
{
    return Rect(
        ((xStart < xEnd) ? xStart : xEnd) - (int)radius - 1,
        ((yStart < yEnd) ? yStart : yEnd) - (int)radius - 1,
        ((xStart > xEnd) ? xStart : xEnd) + (int)radius + 1,
        ((yStart > yEnd) ? yStart : yEnd) + (int)radius + 1
    );
}
// check which blocks are candidates for a collision:
int Ball::populateBlockHitList( const Rect& collisionArea )
{
    memset( blockHitList,-1,sizeof( int ) * MAX_BLOCKS_PER_LEVEL );
    int CollisionCnt = 0;
    for ( int i = 0; i < inGameLevelData->nrOfBlocks; i++ )
    {
        if ( inGameLevelData->blocks[i].isAlive == false ) continue;
        if ( inGameLevelData->blocks[i].xOrig > collisionArea.x2 ) continue;
        if ( inGameLevelData->blocks[i].yOrig > collisionArea.y2 ) continue;
        if ( inGameLevelData->blocks[i].x2    < collisionArea.x1 ) continue;
        if ( inGameLevelData->blocks[i].y2    < collisionArea.y1 ) continue;        
        assert( CollisionCnt < COLLISION_GRID_MAX_CELLS );
        blockHitList[CollisionCnt] = i;
        CollisionCnt++;
    }
    return CollisionCnt;
}
void Ball::setTrajectories( const float ballX_,const float ballY_ )
{
    /*
    // recenter the system at ballX,ballY
    bTop    =  - radius;
    bBottom = radius;
    bLeft   = m1 * radius;   // - m1 * ( - radius );
    bRight  = - bLeft;       // - m1 * radius;
    */
    bTop = ballY_ - radius - m1 * ballX_;
    bBottom = ballY_ + radius - m1 * ballX_;
    bLeft = ballY_ - m1 * (ballX_ - radius);
    bRight = ballY_ - m1 * (ballX_ + radius);
}
ballhitlocation Ball::getCollisionLocation( float ballX_,float ballY_,const Rect& blockRect )
{
    ballhitlocation ballHitLocation;
    /*
    // recenter the system at ballX,ballY
    bTop    =  - radius;
    bBottom = radius;
    bLeft   = m1 * radius;   // - m1 * ( - radius );
    bRight  = - bLeft;       // - m1 * radius;
    Rect blockRect(
        blockRect_.x1 - (int)ballX_,
        blockRect_.y1 - (int)ballY_,
        blockRect_.x2 - (int)ballX_,
        blockRect_.y2 - (int)ballY_ );
    */
    /*
        The ball is centered at BallX, BallY with a radius "radius"
        y = m1 * x + bTop
        ... is the line of travel of point (BallX, BallY - radius)

        y = m1 * x + bBottom
        ... is the line of travel of point (BallX, BallY + radius)

        y = m1 * x + bLeft
        ... is the line of travel of point (BallX - radius, BallY)

        y = m1 * x + bRight
        ... is the line of travel of point (BallX + radius, BallY)
    */
    setTrajectories( ballX_,ballY_ );
    if ( horSpeed > 0.0 )
    {
        if ( verSpeed > 0.0 )
        { // approach from the top left
            float yBottomL = m1 * blockRect.x1 + bBottom;
            float yBottomR = m1 * blockRect.x2 + bBottom;
            float yRight = m1 * blockRect.x1 + bRight;
            if ( yBottomR < blockRect.y1 )      ballHitLocation = topright;
            else if ( yBottomL < blockRect.y1 ) ballHitLocation = top;
            else if ( yRight   < blockRect.y1 ) ballHitLocation = topleft;
            else if ( yRight   < blockRect.y2 ) ballHitLocation = left;
            else                                ballHitLocation = bottomleft;
        } else { // approach from the bottom left
            float yTopL = m1 * blockRect.x1 + bTop;
            float yTopR = m1 * blockRect.x2 + bTop;
            float yRight = m1 * blockRect.x1 + bRight;
            if ( yRight < blockRect.y1 )        ballHitLocation = topleft;
            else if ( yRight   < blockRect.y2 ) ballHitLocation = left;
            else if ( yTopL    < blockRect.y2 ) ballHitLocation = bottomleft;
            else if ( yTopR    < blockRect.y2 ) ballHitLocation = bottom;
            else                                ballHitLocation = bottomright;
        }
    } else {
        if ( verSpeed > 0.0 )
        { // approach from the top right
            float yBottomL = m1 * blockRect.x1 + bBottom;
            float yBottomR = m1 * blockRect.x2 + bBottom;
            float yLeft = m1 * blockRect.x2 + bLeft;
            if ( yBottomL < blockRect.y1 )      ballHitLocation = topleft;
            else if ( yBottomR < blockRect.y1 ) ballHitLocation = top;
            else if ( yLeft    < blockRect.y1 ) ballHitLocation = topright;
            else if ( yLeft    < blockRect.y2 ) ballHitLocation = right;
            else                                ballHitLocation = bottomright;
        } else { // approach from the bottom right
            float yTopL = m1 * blockRect.x1 + bTop;
            float yTopR = m1 * blockRect.x2 + bTop;
            float yLeft = m1 * blockRect.x2 + bLeft;
            if ( yLeft < blockRect.y1 )        ballHitLocation = topright;
            else if ( yLeft   < blockRect.y2 ) ballHitLocation = right;
            else if ( yTopR   < blockRect.y2 ) ballHitLocation = bottomright;
            else if ( yTopL   < blockRect.y2 ) ballHitLocation = bottom;
            else                               ballHitLocation = bottomleft;
        }
    }
    //setTrajectories( ballX_,ballY_ );
    return ballHitLocation;
}
bool Ball::isHitConfirmed( float ballX_,float ballY_,
    HitLocationData &hitLData,const Rect& blockRect )
{
    if ( hitLData.blockNr == lastBlockHit ) return false;
    switch ( hitLData.hitLocation )
    {
        case topleft:
        {
            hitLData.xHit = (float)blockRect.x1;
            hitLData.yHit = (float)blockRect.y1;
            break;
        }
        case topright:
        {
            hitLData.xHit = (float)blockRect.x2;
            hitLData.yHit = (float)blockRect.y1;
            break;
        }
        case bottomleft:
        {
            hitLData.xHit = (float)blockRect.x1;
            hitLData.yHit = (float)blockRect.y2;
            break;
        }
        case bottomright:
        {
            hitLData.xHit = (float)blockRect.x2;
            hitLData.yHit = (float)blockRect.y2;
            break;
        }
        case top:
        {
            if ( ballY_ + radius <= blockRect.y1 )
            {
                hitLData.yHit = (float)blockRect.y1;
                hitLData.xHit = (hitLData.yHit - bBottom) / m1;
                hitLData.Bx = hitLData.xHit;
                hitLData.By = hitLData.yHit - radius;
                return true;
            } else return false;
            break;
        }
        case bottom:
        {
            if ( ballY_ - radius >= blockRect.y2 )
            {
                hitLData.yHit = (float)blockRect.y2;
                hitLData.xHit = (hitLData.yHit - bTop) / m1;
                hitLData.Bx = hitLData.xHit;
                hitLData.By = hitLData.yHit + radius;
                return true;
            } else return false;
            break;
        }
        case left:
        {
            if ( ballX_ + radius <= blockRect.x1 )
            {
                hitLData.xHit = (float)blockRect.x1;
                hitLData.yHit = m1 * hitLData.xHit + bRight;
                hitLData.Bx = hitLData.xHit - radius;
                hitLData.By = hitLData.yHit;
                return true;
            } else return false;
            break;
        }
        case right:
        {
            if ( ballX_ - radius >= blockRect.x2 )
            {
                hitLData.xHit = (float)blockRect.x2;
                hitLData.yHit = m1 * hitLData.xHit + bLeft;
                hitLData.Bx = hitLData.xHit + radius;
                hitLData.By = hitLData.yHit;
                return true;
            } else return false;
            break;
        }
        /*
        case top:
        {
            hitLData.yHit = (float)blockRect.y1;
            hitLData.xHit = (hitLData.yHit - bBottom) / m1;
            hitLData.Bx = hitLData.xHit;
            hitLData.By = hitLData.yHit - radius;
            break;
        }
        case bottom:
        {
            hitLData.yHit = (float)blockRect.y2;
            hitLData.xHit = (hitLData.yHit - bTop) / m1;
            hitLData.Bx = hitLData.xHit;
            hitLData.By = hitLData.yHit + radius;
            break;
        }
        case left:
        {
            hitLData.xHit = (float)blockRect.x1;
            hitLData.yHit = m1 * hitLData.xHit + bRight;
            hitLData.Bx = hitLData.xHit - radius;
            hitLData.By = hitLData.yHit;
            break;
        }
        case right:
        {
            hitLData.xHit = (float)blockRect.x2;
            hitLData.yHit = m1 * hitLData.xHit + bLeft;
            hitLData.Bx = hitLData.xHit + radius;
            hitLData.By = hitLData.yHit;
            break;
        }
        */
    }
    // Check if the point we hit is actually inside the collision box:
    if ( ! collisionArea.contains( (int)hitLData.xHit,(int)hitLData.yHit ) ) return false;
    /*
    Now check if the corner of the block will actually touch
    the ball when it passes, e.g. if the block is on the balls'
    trajectory.
    y = m2 * x + b2
    ... is the equation of the line perpendicular to the
    trajectory of the ball, going through the corner of the
    block.
    H(Hx,Hy) is the center of the ball at that time or position
    */
    float m2 = -1 / m1;
    float b2 = hitLData.yHit - m2 * hitLData.xHit;
    float Hx = (b2 - b1) / (m1 - m2);
    /*
    check if we already passed this block or not: if the
    horSpeed and the distance between the ball at the start &
    the hit location have a different sign, it means the block
    lies behind the ball, so there is point in further checks
    */

    // debug!! needs finetuning:
    //if ( (Hx - ballX_) * horSpeed < 0.0f ) return false;

    float Hy = m1 * Hx + b1;
    //float Hy = m2 * Hx + b2;
    float HExD = Hx - hitLData.xHit;
    float HEyD = Hy - hitLData.yHit;
    float distHToCorner = sqrt( HExD * HExD + HEyD * HEyD );

#ifdef DEBUG_MODE
    //gfx->DrawMathLine( (int)Hx,(int)Hy,m2,100,0xFF00 );
    //gfx->DrawCircle( (int)Hx,(int)Hy,radius,0xFF40FF );
#endif        
    if ( distHToCorner <= radius ) return true;
    else return false;

    //return false; // testing
}

void Ball::correctCollision( float ballX_,float ballY_,
    HitLocationData &hitLData,
    const Rect& blockRect )
{
    /*
    In case we have a collision with a corner we need to know
    the coordinates of the center of the ball when it hits the
    corner of the block. This is called the collision
    correction if I'm not mistaken.
    */
    switch ( hitLData.hitLocation )
    {
        case topleft:
        case topright:
        case bottomleft:
        case bottomright:
        {
            /*
            y = m2 * x + b2
            ... is the equation of the line perpendicular to the
            line of travel of the ball, and going through the
            corner of the collisionbox (point B) that the ball hit

            The point C is the point of intersection of both lines
            The point B is the corner of the collision box the ball
            overshot
            The point A is the corrected origin of the ball
            AC is the distance between A and C
            */
            float m2 = -1 / m1;
            float b2 = hitLData.yHit - m2 * hitLData.xHit;
            float Cx = (b2 - b1) / (m1 - m2);
            float Cy = m1 * Cx + b1;
            float CBx = (Cx - hitLData.xHit);
            float CBy = (Cy - hitLData.yHit);
            float BCSq = CBx * CBx + CBy * CBy;
            float radiusSq = (float)((radius) * (radius));
            float AC;
            /*
            // should be reinstated?
            if ( radiusSq > BCSq ) AC = sqrt( radiusSq - BCSq );
            else                   AC = EPSILON;
            */
            AC = sqrt( radiusSq - BCSq );

            float alfa = atan( m1 );
            float Ax = Cx;
            float Ay = Cy;
            if ( horSpeed > 0.0f )
            {
                hitLData.Bx = Ax - AC * cos( alfa );
                hitLData.By = Ay - AC * sin( alfa );
            } else {
                hitLData.Bx = Ax + AC * cos( alfa );
                hitLData.By = Ay + AC * sin( alfa );
            }
#ifdef DEBUG_MODE
            /*
            gfx->DrawMathLine( (int)hitLData.xHit,(int)hitLData.yHit,m2,100,0xFFFF00 );
            gfx->DrawMathLine( (int)ballX_,(int)ballY_,m1,100,0xFFFF00 );
            gfx->DrawCircle(
            (int)hitLData.Bx,
            (int)hitLData.By,
            (int)radius,
            D3DCOLOR_XRGB( 50,255,255 ) );
            */
#endif
            break;
        }
    }
    // Now we calculate the actual distance travelled to this hit

    //if( hitLData.hitLocation != nohit )
    //{
    float dX = hitLData.Bx - ballX_;
    float dY = hitLData.By - ballY_;
    hitLData.distance = sqrt( dX * dX + dY * dY );
    if ( hitLData.distance < EPSILON ) hitLData.distance = EPSILON; // avoid eternal loops
    //}

#ifdef DEBUG_MODE
    /*
    gfx->DrawCircle(
    (int)hitLData.Bx,
    (int)hitLData.By,
    (int)hitLData.distance,
    D3DCOLOR_XRGB( 50,255,255 ) );
    */
#endif
}
void Ball::processCollision( HitLocationData &hitLData )
{
    switch ( hitLData.hitLocation )
    {
        case topleft:
        case topright:
        case bottomright:
        case bottomleft:
        {
            //if ( isDestructionBall ) break;
            float Bx = hitLData.xHit;
            float By = hitLData.yHit;
            float Ax = hitLData.Bx;
            float Ay = hitLData.By;
#ifdef DEBUG_MODE
            // draw line along travelling axis
            //gfx->DrawMathLine( (int)Bx,(int)By,m1,100,
            //    D3DCOLOR_XRGB( 180,100,20 ) );
            /*
            // draw line perpendicular to travelling axis
            gfx.DrawLine( (int)Cx,(int)Cy,(int)Bx,(int)By,
            D3DCOLOR_XRGB( 100,250,100 ) );
            */
            // draw the corrected ball as a circle
            //gfx->DrawCircle( (int)Bx,(int)By,(int)radius,
            //    D3DCOLOR_XRGB( 250,50,50 ) );
            /*
            // draw the line from the radius of the circle to the
            // point where it hits the block
            gfx.DrawLine( (int)Bx,(int)By,(int)Bx,(int)By,
            D3DCOLOR_XRGB( 100,250,100 ) );
            */
#endif
            /*
            Now calculate the new direction.
            y = m3 * x + b3
            ... is the equation of the line going from A (the
            center of the ball after collision correction) to B
            (the corner of the block the ball just hit). This
            line segment AB is also the radius of the ball.
            y = m4 * x + b4
            ... is the equation of the line of the new direction
            of travel i.e. the direction of travel of the ball
            after the bounce.
            Theta is the angle between the line of travel of the
            ball and the X axis.
            Gamma1 is the angle between the line AB, that goes
            from the corrected center of the ball to the corner
            the ball just hit, and the X axis, multiplied by
            two. Multiplied by two because we always need twice
            this angle in the formulas.
            Tau is the angle between the new line of travel of
            the ball and the X axis.
            */
            float m3 = (By - Ay) / (Bx - Ax);
            float b3 = By - m3 * Bx;
            // find the angle of the new line of travel
            float DAy = b1 - Ay;
            if ( DAy < 0.0 ) DAy = -DAy;
            float theta = asin( DAy / (sqrt( DAy * DAy + Ax * Ax )) );
            float AEy = Ay - b3;
            if ( AEy < 0.0 ) AEy = -AEy;
            float gamma1 = asin( AEy / (sqrt( AEy * AEy + Ax * Ax )) ) * 2.0f;
            float tau;
            if ( horSpeed > 0.0 )
            {
                if ( verSpeed > 0.0 )
                {
                    if ( hitLData.hitLocation == topleft )
                            tau = -theta + gamma1;
                    else    tau = -theta - gamma1;
                } else {
                    if ( hitLData.hitLocation == bottomleft )
                            tau = theta - gamma1;
                    else    tau = theta + gamma1;
                }
            } else {
                if ( verSpeed > 0.0 )
                {
                    if ( hitLData.hitLocation == topright )
                            tau = theta - gamma1;
                    else    tau = theta + gamma1;
                } else {
                    if ( hitLData.hitLocation == bottomright )
                            tau = -theta + gamma1;
                    else    tau = -theta - gamma1;
                }
            }
            float m4 = tan( tau );
            float b4 = By - m4 * Bx;
#ifdef DEBUG_MODE
            /*
            Draw the line that is orthogonal to AB. This is the
            imaginable line the ball bounces back against
            */
            /*
            gfx->DrawMathLine( (int)Bx,(int)By,-1.0f / m3,100,
            D3DCOLOR_XRGB( 100,250,100 ) );
            gfx->DrawMathLine( (int)Bx,(int)By,m3,100,
            D3DCOLOR_XRGB( 100,250,100 ) );
            // Draw the new line of travel
            gfx->DrawMathLine( (int)Bx,(int)By,m4,100,
            D3DCOLOR_XRGB( 250,20,250 ) );
            */
#endif
            /*
            Now that we have the equation of the new line of
            travel (y = m4 * x + b4) of the ball, we need to
            find out which way we travel on this line from
            the point B (the edge of the block). Key here is
            that we can't cross the (imaginary) line against
            which the ball bounces back. So we calculate
            both new coordinates and check whether they are
            on the right side of this line. This should be
            the same side as the point A (the point A is the
            corrected origin of the ball).
            */
            float alfa = atan( m4 ); // simplify?
            float S1y = speed * sin( alfa );    // Speed or speedleft??? DEBUG??
            float S2y = By - S1y;
            S1y += By;
            float S1x = (S1y - b4) / m4;
            float S2x = (S2y - b4) / m4;
            /*
            y = m6 * x + b6
            ... is the line going through B and perpendicular
            to AB. This is the line the ball bounces on.
            Now we check which point (S1 or S2) is on the same
            side of the bounce-line as point A:
            */
            float m6 = -1.0f / m3;
            float b6 = By - m6 * Bx;
            float PS1y = m6 * S1x + b6;
            float PS2y = m6 * S2x + b6;
            float PAy = m6 * Ax + b6;
            float Sx;
            float Sy;
            if ( Ay < PAy )
            {
                if ( S1y < PS1y )
                {
                    Sx = S1x;
                    Sy = S1y;
                } else {
                    Sx = S2x;
                    Sy = S2y;
                }
            } else {
                if ( S1y > PS1y )
                {
                    Sx = S1x;
                    Sy = S1y;
                } else {
                    Sx = S2x;
                    Sy = S2y;
                }
            }
#ifdef DEBUG_MODE
            //gfx->DrawCircle( (int)Sx,(int)Sy,2,D3DCOLOR_XRGB( 50,250,100 ) );
#endif
            setSpeed( Sx - Bx,Sy - By,hitLData.Bx,hitLData.By );
            break;
        }
        case top:
        case bottom:
        {
            if ( hitLData.blockNr != SPECIAL_BLOCK_PADDLE || 
                 hitLData.hitLocation == bottom ) // only usefull if invincible
            {
                setSpeed( horSpeed,-verSpeed,hitLData.Bx,hitLData.By );
            } else { // Ball hit the top side of the paddle!
                /*
                    D is the center of the Circle that the paddle is a part off
                    The paddle is the top part of the circle.
                    y = m2 * x + b2 is the line from D to the point of impact.
                    OA is the distance between the point A and the point where
                    the line y = m1 * x + b1 hits the y axis (at x == 0).
                    PA is the distance between the point A and the point where
                    the line y = m2 * x + b2 hits the y axis (at x == 0).
                */
#ifdef DEBUG_MODE
                gfx->DrawMathLine( (int)hitLData.xHit,(int)hitLData.yHit,m1,100,0xFFFF00 ); 
#endif
                // center of paddle-ball is below paddle
                const float Dx = (float)(inGameLevelData->blocks[hitLData.blockNr].xOrig + PADDLE_STANDARD_WIDTH / 2);
                const float Dy = (float)(inGameLevelData->blocks[hitLData.blockNr].yOrig + 150 ); 
                float Ax = hitLData.xHit;
                float Ay = hitLData.yHit;
                float m2 = (Dy - Ay) / (Dx - Ax);    
                float b2 = Ay - m2 * Ax;
                float PAyd = Ay - b2;
                if ( PAyd < 0 ) PAyd = -PAyd;
                float PA = sqrt( Ax * Ax + PAyd * PAyd );
                float gamma1 = asin( PAyd / PA ) * 2;
                float AOyd = Ay - b1;
                if ( AOyd < 0 ) AOyd = -AOyd; 
                float OA = sqrt( Ax * Ax + AOyd * AOyd );
                float theta = asin( AOyd / OA );
                float tau;
                if ( horSpeed > 0.0f )
                {
                    if ( Ax < Dx )  tau = PI + theta - gamma1;
                    else            tau = PI + theta + gamma1;
                } else {
                    if ( Ax < Dx )  tau = - theta - gamma1;
                    else            tau = - theta + gamma1;
                }
                horSpeed = speed * cos( tau );
                verSpeed = - speed * sin( tau );
                if ( verSpeed > 0 ) verSpeed = -verSpeed; // safeguard
                if ( verSpeed > -0.2f ) verSpeed = -0.3f; // safeguard against boredom ;)

                setSpeed( horSpeed,verSpeed,hitLData.Bx,hitLData.By );
#ifdef DEBUG_MODE
                float m3 = -1.0f / m2; // The ball bounces of this line
                gfx->DrawMathLine( (int)hitLData.xHit,(int)hitLData.yHit,m2,100,0xFFFF );
                gfx->DrawMathLine( (int)hitLData.xHit,(int)hitLData.yHit,m3,100,0xFFFF );
                gfx->DrawMathLine( (int)hitLData.xHit,(int)hitLData.yHit,m1,100,0xFF00 );
                //freeze = true;// debug!!!
#endif
            }
            break;
        }
        case left:
        case right:
        {
            setSpeed( -horSpeed,verSpeed,hitLData.Bx,hitLData.By );
            break;
        }
    }
}

