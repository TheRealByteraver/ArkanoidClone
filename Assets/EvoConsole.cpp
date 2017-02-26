
#include <assert.h>
#include <string.h>
#include "EvoConsole.h"


EvoConsole::EvoConsole ()
{
    memset(this, 0, sizeof(EvoConsole));
}

EvoConsole::~EvoConsole ()
{
    if ( data )
    {
        for ( int r = 0; r < rows; r++ )
        {
            if ( data[r] ) delete data[r];
        } 
        delete data;
    }
}

void EvoConsole::initialize  ( int nrColumns,int nrRows  )
{
    columns = nrColumns;
    rows = nrRows;
    data = new char *[nrRows];
    for ( int r = 0; r < rows; r++ )
    {
        data[r] = new char [columns + 1];
        memset( data[r],0,sizeof(char) * (columns + 1) );
    }
}

void EvoConsole::Print ( const char *str )
{
    while ( (xCursor + (int)(strlen( str ))) >= columns )
    {
        char *d = data[yCursor] + xCursor;
        for (int i = xCursor; i < columns; i++) 
        {
            *d++ = *str++;
        }
        xCursor = 0;
        yCursor++;
        if ( yCursor >= rows ) 
        {
            ScrollUp ();
            yCursor--;
        }
    } 
    char *d = data[yCursor] + xCursor;
    int s = strlen( str );
    xCursor += s;
    for (int i = 0; i < s; i++) 
    {
        *d++ = *str++;        
    }     
}

void EvoConsole::ScrollUp ()
{
    char *tmp = data[0];
    memset ( data[0],0,sizeof(char) * (columns) );
    for ( int i = 0; i < (rows - 1); i++ )
    {
        data[i] = data[i + 1];
    }
    data[rows - 1] = tmp;
    xCursor = 0;
}
/*
void EvoConsole::operator<<= (std::ostream &stream)
{
    std::stringstream s;
    s << stream;
    Print(s.str().c_str());
}
*/
