#ifndef ClassDeclaration_h__
#define ClassDeclaration_h__
#include "AstNode.h"

namespace AST 
{

class ClassDeclaration : public Statement
{
    Identifier* id;
    Block* block;
public:
    ClassDeclaration(Identifier* id, Block* block)
    : id(id), block(block) {}
    virtual ~ClassDeclaration()
    {
        delete id;
        delete block;
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType::Enum getType() {return NodeType::klass;}
    virtual void toString() {
        std::cout << "  Creating Class: " << id->getName() << std::endl;
        std::for_each(std::begin(block->statements), std::end(block->statements),
            [] (Statement* s) {
                s->toString();
            });
    }
private:
    void removeVarDeclStatements();
    void constructStructFields(std::vector<llvm::Type*>& StructTy_fields, CodeGenContext& context);
    void addVarsToClassAttributes(CodeGenContext& context);
};


}
#endif // ClassDeclaration_h__



