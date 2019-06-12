#pragma once
#include "AstNode.h"

namespace liquid
{

/*! Represents a compare operator  == != > < >= <= */
class CompOperator : public Expression
{
public:
   explicit CompOperator(Expression* lhs, int op, Expression* rhs) : op(op), lhs(lhs), rhs(rhs) {}
   virtual ~CompOperator()
   {
      delete lhs;
      delete rhs;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override;
   void         Accept(Visitor& v) override { v.VisitCompOperator(this); }

   int         getOperator() const { return op; }
   Expression* getLHS() { return lhs; }
   Expression* getRHS() { return rhs; }

private:
   int         op{0};
   Expression* lhs{nullptr};
   Expression* rhs{nullptr};
};

} // namespace liquid
