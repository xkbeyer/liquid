#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <cmath>

#include "buildins.h"

extern "C" DECLSPEC int printvalue(int val)
{
   std::cout << "IDEBUG: " << val << "\n";
   return 1;
}

extern "C" DECLSPEC double printdouble(double val)
{
   std::cout << "DDEBUG: " << val << "\n";
   return 1.;
}

extern "C" DECLSPEC void display(char* str, ...)
{
   va_list argp;
   va_start(argp, str);
   vprintf(str, argp);
   va_end(argp);
}

extern "C" DECLSPEC void displayln(char* str, ...)
{
   char*   outstr;
   va_list argp;
   va_start(argp, str);
   outstr = (char*)malloc(strlen(str) + 2);
   strcpy(outstr, str);
   strcat(outstr, "\n");
   vprintf(outstr, argp);
   va_end(argp);
   free(outstr);
}

extern "C" DECLSPEC double sinus(double val)
{
   return std::sin(val);
}
