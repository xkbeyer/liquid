#ifndef FUNCTION_DECLARATION_H
#define FUNCTION_DECLARATION_H

#include "AstNode.h"
namespace liquid {
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
    virtual ~FunctionDeclaration();
    virtual llvm::Value* codeGen( CodeGenContext& context );
    NodeType getType() { return NodeType::function; }
    Identifier* getId() { return id; }
    virtual void toString();
};

}
#endif // FUNCTION_DECLARATION_H
