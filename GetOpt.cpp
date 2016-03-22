/* GetOpt - getopt as a c++ class.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Klaus Beyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>  // EOF const
#include <iostream>
#include <fstream>
#include <algorithm>

#include "GetOpt.h"

static std::string removeLeftWhiteSpaces( std::string text )
{
   std::string newtext;
   int pos = text.find_first_not_of( " \t\n" );
   if( pos != std::string::npos ) {
      newtext = text.substr( pos, text.length() - pos );
   } else {
      newtext = text;
   }
   return newtext;
}

GetOpt::GetOpt( int argc, char* argv[], const std::string optstring, const std::string filename )
   : index( 1 )
   , optionString( optstring )
{

   // Put cmd line options into front, they win over the file options.
   for(int i = 0; i < argc; ++i) {
      argStrings.push_back(argv[i]);
   }


   if( !filename.empty() ) {
      std::ifstream file;
      file.open( filename );
      std::string line;
      while( getline( file, line ) ) {
         if( line[0] == '-' ) {
            // Remove spaces between option and its argument.
            if( line[2] == ' ' ) {
               auto arg = line.substr( 2 );
               line = line.substr( 0, 2 ) + removeLeftWhiteSpaces( arg );
            }
         }
         auto found = std::count_if(std::begin(argStrings), std::end(argStrings), [line](auto s) { return line == s; });
         if(found == 0) {
            argStrings.push_back(line);
         }
      }
   }

   argCount = argStrings.size();
}

char GetOpt::operator()()
{
   optionArgument.clear();
   errorText.clear();

   // Is first character of option string a ':'
   if( optionString[0] == ':' ) {
      errorText = argStrings[0] + std::string( ": missing option argument in " ) + optionString + "\n";
      optionString.erase( 0, 1 );
      return ':';
   }

   // Is end of argument list? 
   if( index >= argCount ) {
      return EOF;
   }

   // Is a non option argument reached?  
   if( argStrings[index][0] != '-' ) {
      if( argStrings[index][1] == '\0' )
         return EOF;
      if( index == argCount - 1 ) {
         return EOF;
      }
      std::rotate(argStrings.begin()+index, argStrings.begin()+index+1, argStrings.end());
      --argCount;
      return this->operator()();
   }

   // Is end of argument list reached? 
   if( argStrings[index][0] == '-' && argStrings[index][1] == '-' ) {
      index++;
      return EOF;
   }

   auto scan = argStrings[index];
   index++;

   // Skip '-'
   // Is current character in the option string 
   char c = scan[1];
   auto place = optionString.find_first_of( c );
   if( place == std::string::npos || c == ':' ) {
      errorText = argStrings[0] + std::string( ": unknown option -" ) + c + "\n";
      return '?';
   }

   // Check if an additional argument is needed.
   place++;
   if( optionString[place] == ':' ) {
      place++;
      bool argIsOptional = optionString[place] == ':';
      // Check if no space is between option and its argument.
      if( scan[2] != '\0' ) {
         optionArgument = scan.substr( 2 );
      } else if( index < argCount ) {
         if( argStrings[index][0] != '-' ) {
            optionArgument = argStrings[index];
            index++;
         } else if( !argIsOptional ) {
            errorText = argStrings[0] + std::string( ": option requires argument -" ) + c + "\n";
            return ':';
         }
      } else if( !argIsOptional ) {
         errorText = argStrings[0] + std::string( ": option requires argument -" ) + c + "\n";
         return ':';
      }
   }
   return c;
}

std::vector<std::string> GetOpt::getRemainingArguments()
{
   std::vector<std::string> args(argStrings.begin() + index, argStrings.end());
   return args;
}


char GetOpt::iterator::operator*()
{
   auto ret = getopt->operator()();
   if( ret == EOF ) {
      position = getopt->argCount - 1; // Set iterator to the end
   } else {
      position = getopt->index - 1; // In case index has advanced more than one position.
   }

   return ret;
}
