#pragma once

#include "AstNode.h"

namespace liquid
{
/*! Represents a binary operators  + - * / */
class BinaryOp : public Expression
{
public:
   BinaryOp(Expression* lhs, int op, Expression* rhs, YYLTYPE loc) : op(op), lhs(lhs), rhs(rhs), location(loc) {}
   virtual ~BinaryOp()
   {
      delete lhs;
      delete rhs;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override;
   void         Accept(Visitor& v) override { v.VisitBinaryOp(this); }

   Expression* getLHS() { return lhs; }
   Expression* getRHS() { return rhs; }
   int         getOperator() const { return op; }

private:
   llvm::Value* codeGenAddList(llvm::Value* rhsValue, llvm::Value* lhsValue, CodeGenContext& context);

   int         op{0};
   Expression* lhs{nullptr};
   Expression* rhs{nullptr};
   YYLTYPE     location;
};

}
