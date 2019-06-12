#pragma once
#include "AstNode.h"

namespace liquid
{

/*! Represents a conditional statement. */
class Conditional : public Statement
{
public:
   explicit Conditional(Expression* op, Expression* thenExpr, Expression* elseExpr = nullptr) : cmpOp((CompOperator*)op), thenExpr(thenExpr), elseExpr(elseExpr)
   {
   }
   virtual ~Conditional();

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override { return "conditional "; }
   void         Accept(Visitor& v) override { v.VisitConditional(this); }

   CompOperator* getCompOperator() { return cmpOp; }
   Expression*   getThen() { return thenExpr; }
   Expression*   getElse() { return elseExpr; }

private:
   CompOperator* cmpOp{nullptr};
   Expression*   thenExpr{nullptr};
   Expression*   elseExpr{nullptr};
};

} // namespace liquid
