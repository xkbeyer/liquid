#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <cstring>

#include "buildins.h"

/*
 *! some helper/debug function
 */
extern "C"
DECLSPEC int printvalue( int val )
{
    std::cout << "IDEBUG: " << val << "\n";
    return 1;
}

extern "C"
DECLSPEC double printdouble( double val )
{
    std::cout << "DDEBUG: " << val << "\n";
    return 1.;
}

/*! built in display function
 * it works like the C printf function and uses the same format string definition.
 */
extern "C"
DECLSPEC void display( char * str, ... )
{
    va_list argp;
    va_start( argp, str );
    vprintf( str, argp );
    va_end( argp );
}

/*! built in display function with newline
 * it works like the C printf function and uses the same format string definition.
 */
extern "C"
DECLSPEC void displayln( char * str, ... )
{
    char * outstr;
    va_list argp;
    va_start( argp, str );
    outstr = ( char* ) malloc( strlen( str ) + 2 );
    strcpy( outstr, str );
    strcat( outstr, "\n" );
    vprintf( outstr, argp );
    va_end( argp );
    free( outstr );
}
