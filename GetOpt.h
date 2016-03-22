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
#if !defined(GETOPT_H)
#define GETOPT_H

#include <string>
#include <vector>

class GetOpt
{
public:

   GetOpt( int argc, char* argv[], const std::string optstring, const std::string filename = "" );
   std::string get() { return optionArgument; }
   std::string error() { return errorText; }
   int getIndex() { return index; }
   void reset() { index = 1; };
   char operator()();
   std::vector<std::string> getRemainingArguments();

   class iterator
   {
   public:
      iterator( GetOpt* getopt ) : getopt( getopt ), position( 1 ) {};
      iterator( int pos ) : position( pos ) {};
      iterator& operator++() { ++position; return *this; }  // prefix
      bool operator!=(iterator rhs) { 
         return position != rhs.getopt->argCount; 
      }
      char operator*();
   private:
      int position;
      GetOpt* getopt;
   };
   iterator begin() { return iterator( this ); }
   iterator end() { return iterator( this ); }
   friend class iterator;
private:
   std::string optionArgument; /* Global argument pointer. */
   int index; /* Global argv index. */
   int argCount;
   std::string optionString;
   std::string errorText;
   std::vector<std::string> argStrings;
};

#endif  /* GETOPT_H */