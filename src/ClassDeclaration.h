#ifndef ClassDeclaration_h__
#define ClassDeclaration_h__

#include <string>
#include <sstream>

#include "AstNode.h"

namespace liquid 
{

class ClassDeclaration : public Statement
{
    Identifier* id;
    Block* block;
public:
    ClassDeclaration(Identifier* id, Block* block) : id(id), block(block) {}
    virtual ~ClassDeclaration() { delete id; delete block; }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::klass;}
    virtual std::string toString() { std::stringstream s; s << "Class: " << id->getName(); return s.str(); }
    virtual Block* getBlock() { return block; }
    virtual Identifier* getIdentifier() { return id; }
    virtual void Accept(Visitor& v) { v.VisitClassDeclaration(this); }
private:
    void removeVarDeclStatements();
    void constructStructFields(std::vector<llvm::Type*>& StructTy_fields, CodeGenContext& context);
    void addVarsToClassAttributes(CodeGenContext& context);
};


}
#endif // ClassDeclaration_h__



