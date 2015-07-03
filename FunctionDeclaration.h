#ifndef FUNCTION_DECLARATION_H
#define FUNCTION_DECLARATION_H

#include "AstNode.h"
namespace AST {
class FunctionDeclaration : public Statement
{
    friend class ClassDeclaration;
    Identifier* type;
    Identifier* id;
    VariableList* arguments;
    Block* block;
    YYLTYPE location;
public:
    FunctionDeclaration( Identifier* type, Identifier* id, VariableList* arguments, Block* block, YYLTYPE loc )
        : type( type ), id( id ), arguments( arguments ), block( block ), location( loc )
    {}
    FunctionDeclaration( Identifier* id, VariableList* arguments, Block* block, YYLTYPE loc )
        : type( new Identifier( "var", location ) ), id( id ), arguments( arguments ), block( block ), location( loc )
    {}
    virtual ~FunctionDeclaration()
    {
        for( VariableList::iterator i = arguments->begin(); i != arguments->end(); ++i ) {
            delete *i;
        }
        delete type;
        delete id;
        delete arguments;
        delete block;
    }
    virtual llvm::Value* codeGen( CodeGenContext& context );
    NodeType getType() { return NodeType::function; }
    Identifier* getId() { return id; }
    virtual void toString()
    {
        std::cout << "  Creating function: " << id->getName() << std::endl;
        std::cout << "  Parameters : " << std::endl;
        std::for_each( std::begin( *arguments ), std::end( *arguments ),
                       [] ( VariableDeclaration* decl ) {
            std::cout << "      " << decl->getVariablenTypeName() << ", " << decl->getVariablenName() << std::endl;
            decl->toString();
        }
        );
    }
};

}
#endif // FUNCTION_DECLARATION_H
