#ifndef ClassDeclaration_h__
#define ClassDeclaration_h__


#include <string>
#include <sstream>

#include "AstNode.h"

namespace liquid 
{

class ClassDeclaration : public Statement
{
public:
    explicit ClassDeclaration(Identifier* id, Block* block) : id(id), block(block) {}
    virtual ~ClassDeclaration() { delete id; delete block; }
    llvm::Value* codeGen(CodeGenContext& context) override;
    NodeType     getType() override { return NodeType::klass; }
    std::string  toString() override
    {
       std::stringstream s;
       s << "Class: " << id->getName();
       return s.str();
    }
    void        Accept(Visitor& v) override { v.VisitClassDeclaration(this); }
    Block*      getBlock() { return block; }
    Identifier* getIdentifier() { return id; }
private:
    void removeVarDeclStatements();
    void constructStructFields(std::vector<llvm::Type*>& StructTy_fields, CodeGenContext& context);
    void addVarsToClassAttributes(CodeGenContext& context, std::vector<llvm::Type*>& StructTy_fields);

    Identifier* id{nullptr};
    Block*      block{nullptr};
};


}
#endif // ClassDeclaration_h__



