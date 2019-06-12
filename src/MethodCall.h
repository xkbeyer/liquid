#pragma once
#include "AstNode.h"

namespace liquid
{

/*! Represents a method call. */
class MethodCall : public Statement
{
public:
   explicit MethodCall(Identifier* id, ExpressionList* arguments, YYLTYPE loc) : id(id), arguments(arguments), location(loc) {}
   virtual ~MethodCall()
   {
      for (auto i : *arguments) {
         delete i;
      }
      arguments->clear();
      delete arguments;
      delete id;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "method call: " << id->getStructName() << "." << id->getName();
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitMethodCall(this); }

   ExpressionList* getArguments() { return arguments; }

private:
   std::string getTypeNameOfFirstArg(CodeGenContext& context);

   Identifier*     id{nullptr};
   ExpressionList* arguments{nullptr};
   YYLTYPE         location;
};

} // namespace liquid
