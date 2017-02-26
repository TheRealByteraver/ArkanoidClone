#pragma once

#include "globaldefs.h"
#include <stdlib.h>
#include <Windows.h>

void getNrStr( char *dest, int source )
{
    assert( source >= 0 );
    assert( source < 1000 );
    _itoa( source,dest,10 );
};