#pragma once
#include "AstNode.h"

namespace liquid
{

/*! Represents an assigment. */
class Assignment : public Statement
{
public:
   explicit Assignment(Identifier* lhs, Expression* rhs, YYLTYPE loc) : lhs(lhs), rhs(rhs), location(loc) {}
   virtual ~Assignment()
   {
      delete lhs;
      delete rhs;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "assignment for " << lhs->getStructName() << "::" << lhs->getName();
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitAssigment(this); }

   Expression* getExpression() { return rhs; }

private:
   Identifier* lhs{nullptr};
   Expression* rhs{nullptr};
   YYLTYPE     location;
};

} // namespace liquid
