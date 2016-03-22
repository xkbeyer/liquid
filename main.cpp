#include "main.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cassert>
#include "config.h"
#include "CodeGenContext.h"
#include "AstNode.h"
#include "GetOpt.h"

extern int yyparse();
extern int yylex_destroy();
extern FILE* yyin;
extern liquid::Block* programBlock;
extern std::stack<std::string> fileNames;
extern std::vector<std::string> libPaths;
extern int parsing_error;

void usage();

int main(int argc, char **argv)
{
    std::string fileName;
    std::cout << "liquid version " << MAJOR_VER << "." << MINOR_VER << "." << REVISION_VER << "\n";
    if( argc == 1 ) {
        fileName = "./test_full.liq";
    }

    libPaths.push_back("./"); // current path

    GetOpt getopt(argc, argv, "hi:");
    for( auto opt : getopt ) {
       switch( opt ) {
          case 'i':
          {
             std::stringstream ss(getopt.get());
             std::string item;
             while( std::getline(ss, item, ';') ) {
                std::replace(std::begin(item), std::end(item), '\\', '/');
                if( item[item.size()] != '/' )
                   item += '/';
                libPaths.push_back(item);
             }
          }
             break;
          case 'h':
          default:
             usage();
             return 1;
       }
    }
    
    auto files = getopt.getRemainingArguments();
    assert(files.size() == 1);
    fileName = files[0]; // Currently only one file is supported.

    yyin = fopen(fileName.c_str(), "r+") ;
    if ( yyin == nullptr )
    {
       std::cout << "File "<< fileName << "not found. Abort" << std::endl;
       return -1;
    }

    fileNames.push(""); // Add the empty file name after last EOF.
    fileNames.push(fileName); // Add the top level file name.
    if( yyparse() || parsing_error) {
       yylex_destroy();
       return 1;
    }

    if( programBlock == nullptr ) {
       std::cout << "Parsing " << fileName << "failed. Abort" << std::endl;
    } else {
       liquid::CodeGenContext context;
       context.printCodeGeneration( *programBlock, std::cout );
       if( context.preProcessing( *programBlock ) ) {
          if(context.generateCode(*programBlock)) {
             context.runCode();
          }
       }
    }

    if( yyin != nullptr )
      fclose(yyin);
    delete programBlock;
    yylex_destroy();
    return 0;
}

void usage()
{
   std::cout << "Usage:\n";
   std::cout << "liq filename -i path1;path2\n";
   std::cout << "\t-i semicolon separated list of import paths where additional liquid files are located.\n";
}
