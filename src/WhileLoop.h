#pragma once
#include "AstNode.h"

namespace liquid
{

/*! Represents a while loop. */
class WhileLoop : public Statement
{
public:
   explicit WhileLoop(Expression* expr, Block* loopBlock, Block* elseBlock = nullptr) : condition(expr), loopBlock(loopBlock), elseBlock(elseBlock) {}
   virtual ~WhileLoop()
   {
      delete condition;
      delete loopBlock;
      delete elseBlock;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override { return "while loop "; }
   void         Accept(Visitor& v) override { v.VisitWhileLoop(this); }

   Expression* getCondition() { return condition; }
   Block*      getLoopBlock() { return loopBlock; }
   Block*      getElseBlock() { return elseBlock; }

private:
   Expression* condition{nullptr};
   Block*      loopBlock{nullptr};
   Block*      elseBlock{nullptr};
};

} // namespace liquid
