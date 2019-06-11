#pragma once

#if defined(_MSC_VER)
#define DECLSPEC __declspec(dllexport) 
#else
#define DECLSPEC 
#endif

/*
 *! some helper/debug function
 */


/*! Prints an integer value.
 * \param[in] val Integer value to be printed.
 * \return Always one FIXME
 */
extern "C" DECLSPEC int printvalue( int val );

/*! Prints a double value.
 * \param[in] val Double value to be printed.
 * \return Always one FIXME
 */
extern "C" DECLSPEC double printdouble(double val);

/*! Built in display function
 * it works like the C printf function and uses the same format string definition.
 * \param[in] str  The format string.
 */
extern "C" DECLSPEC void display(char* str, ...);

/*! Prints formated string like printf but with a cr/lf
 * \param[in] str  The format string.
 */
extern "C" DECLSPEC void displayln(char* str, ...);

/*! Calculates a sinus.
 */
extern "C" DECLSPEC double sinus(double val);
