#pragma once

#include "AstNode.h"

namespace liquid
{
/*! Represents a unary operator. */
class UnaryOperator : public Expression
{
public:
   explicit UnaryOperator(int op, Expression* rhs) : op(op), rhs(rhs) {}
   virtual ~UnaryOperator() { delete rhs; }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override;
   void         Accept(Visitor& v) override { v.VisitUnaryOperator(this); }

   Expression* getRHS() { return rhs; }

private:
   int         op{0};
   Expression* rhs;
};

}
