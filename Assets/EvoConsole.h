
#pragma once


//#include <stdio.h>
#include <ostream>
//#include <iomanip>
#include <sstream>
#include "font.h"



class EvoConsole {
public:
    EvoConsole ();
    ~EvoConsole ();
    void    initialize ( int nrColumns,int nrRows );
    void    Print ( const char *str );
    void    operator<<= (std::ostream &stream);
    void    ScrollUp ();
public:
    int     columns;
    int     rows;
    char  **data;
private:
    int     xCursor;
    int     yCursor;
private:
};

