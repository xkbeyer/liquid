#ifndef FUNCTION_DECLARATION_H
#define FUNCTION_DECLARATION_H

#include "AstNode.h"

namespace liquid {

class FunctionDeclaration : public Statement
{
public:
   FunctionDeclaration(const FunctionDeclaration& other);
   FunctionDeclaration(Identifier* type, Identifier* id, VariableList* arguments, Block* block, YYLTYPE loc);
   FunctionDeclaration(Identifier* id, VariableList* arguments, Block* block, YYLTYPE loc);
   virtual ~FunctionDeclaration();

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType getType() override { return NodeType::function; }
   std::string toString() override;
   void Accept(Visitor& v) override { v.VisitFunctionDeclaration(this); }
   Identifier* getId() const { return id; }
   VariableList* getParameter() const { return arguments; }
   Block* getBody() const { return block; }
   Identifier* getRetType() const { return type; }
   bool isTemplated() const { return hasTemplateParameter; }
   YYLTYPE getlocation() { return location; }

private:
   void checkForTemplateParameter();
   std::string buildFunctionName(llvm::Type* retType, std::vector<llvm::Type*> argTypes);
   friend class ClassDeclaration;
   Identifier* type {nullptr};
   Identifier* id {nullptr};
   VariableList* arguments {nullptr};
   Block* block {nullptr};
   bool hasTemplateParameter {false};
   bool isaCopy {false};
   YYLTYPE location;
};

} // namespace liquid
#endif // FUNCTION_DECLARATION_H
