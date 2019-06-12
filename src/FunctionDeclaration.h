#ifndef FUNCTION_DECLARATION_H
#define FUNCTION_DECLARATION_H

#include "AstNode.h"
namespace liquid {
class FunctionDeclaration : public Statement
{
public:
    FunctionDeclaration( Identifier* type, Identifier* id, VariableList* arguments, Block* block, YYLTYPE loc )
        : type( type ), id( id ), arguments( arguments ), block( block ), location( loc )
    {}
    FunctionDeclaration( Identifier* id, VariableList* arguments, Block* block, YYLTYPE loc )
       : type(new Identifier("var", loc)), id(id), arguments(arguments), block(block), location(loc)
    {}
    virtual ~FunctionDeclaration();
    llvm::Value*  codeGen(CodeGenContext& context) override;
    NodeType      getType() override { return NodeType::function; }
    std::string   toString() override;
    void          Accept(Visitor& v) override { v.VisitFunctionDeclaration(this); }
    Identifier*   getId() { return id; }
    VariableList* getParameter() { return arguments; }
    Block*        getBody() { return block; }
    Identifier*   getRetType() { return type; }
    YYLTYPE       getlocation() { return location; }

 private:
    friend class ClassDeclaration;
    Identifier*   type{nullptr};
    Identifier*   id{nullptr};
    VariableList* arguments{nullptr};
    Block*        block{nullptr};
    YYLTYPE       location;
};

}
#endif // FUNCTION_DECLARATION_H
