#include "main.h"

#include <iostream>
#include <stdio.h>
#include <cassert>
#include "CodeGenContext.h"
#include "AstNode.h"

extern int yyparse();
extern int yylex_destroy();
extern FILE* yyin;
extern liquid::Block* programBlock;


int main(int argc, char **argv)
{
    std::string fileName;
    
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
    
    yyparse();

    if( programBlock == nullptr ) {
       std::cout << "Parsing " << fileName << "failed. Abort" << std::endl;
    } else {
       liquid::CodeGenContext context;
       context.printCodeGeneration( *programBlock );
       context.visitSomething( *programBlock );
       context.generateCode( *programBlock );
       context.runCode();
    }


    fclose(yyin);
    delete programBlock;
    yylex_destroy();
    return 0;
}
