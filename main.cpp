#include "main.h"

#include <iostream>
#include <stdio.h>
#include <cassert>
#include "config.h"
#include "CodeGenContext.h"
#include "AstNode.h"

extern int yyparse();
extern int yylex_destroy();
extern FILE* yyin;
extern liquid::Block* programBlock;
extern std::stack<std::string> fileNames;


int main(int argc, char **argv)
{
    std::string fileName;
    std::cout << "liquid version " << MAJOR_VER << "." << MINOR_VER << "." << REVISION_VER << "\n";
    if( argc == 2 ) {
        fileName = argv[1] ;
    } else {
        fileName = "./test_full.liq";
    }
    yyin = fopen(fileName.c_str(), "r+") ;
    if ( yyin == nullptr )
    {
       std::cout << "File "<< fileName << "not found. Abort" << std::endl;
       return -1;
    }
    fileNames.push(""); // Add the empty file name after last EOF.
    fileNames.push(fileName); // Add the top level file name.
    yyparse();

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


    fclose(yyin);
    delete programBlock;
    yylex_destroy();
    return 0;
}
