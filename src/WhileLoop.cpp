#include "WhileLoop.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;

namespace liquid
{

Value* WhileLoop::codeGen(CodeGenContext& context)
{
   Function*   function       = context.currentBlock()->getParent();
   BasicBlock* firstCondBlock = BasicBlock::Create(context.getGlobalContext(), "firstcond", function);
   BasicBlock* condBB         = BasicBlock::Create(context.getGlobalContext(), "cond");
   BasicBlock* loopBB         = BasicBlock::Create(context.getGlobalContext(), "loop");
   BasicBlock* elseBB         = BasicBlock::Create(context.getGlobalContext(), "else");
   BasicBlock* mergeBB        = BasicBlock::Create(context.getGlobalContext(), "merge");
   BranchInst::Create(firstCondBlock, context.currentBlock());
   context.newScope(firstCondBlock);
   Value* firstCondValue = this->condition->codeGen(context);
   if (firstCondValue == nullptr) {
      Node::printError("Missing condition in while loop.");
      context.addError();
      return nullptr;
   }
   BranchInst::Create(loopBB, elseBB, firstCondValue, context.currentBlock());

   function->getBasicBlockList().push_back(condBB);
   context.endScope();
   context.newScope(condBB);
   Value* condValue = this->condition->codeGen(context);
   if (condValue == nullptr) {
      Node::printError("Code gen for condition expression in while loop failed.");
      context.addError();
      return nullptr;
   }
   BranchInst::Create(loopBB, mergeBB, condValue, context.currentBlock());

   function->getBasicBlockList().push_back(loopBB);
   context.endScope();
   context.newScope(loopBB);
   Value* loopValue = this->loopBlock->codeGen(context);
   if (loopValue == nullptr) {
      Node::printError("Code gen for loop value in while loop failed.");
      context.addError();
      return nullptr;
   }
   BranchInst::Create(condBB, context.currentBlock());

   function->getBasicBlockList().push_back(elseBB);
   context.endScope();
   context.newScope(elseBB);
   if (this->elseBlock != nullptr) {
      Value* elseValue = this->elseBlock->codeGen(context);
      if (elseValue == nullptr) {
         Node::printError("Code gen for else block in while loop failed.");
         context.addError();
         return nullptr;
      }
   }
   BranchInst::Create(mergeBB, context.currentBlock());
   function->getBasicBlockList().push_back(mergeBB);
   context.endScope();
   context.setInsertPoint(mergeBB);
   return mergeBB;
}

} // namespace liquid
