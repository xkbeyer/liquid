#include "Conditional.h"
#include "CompareOperator.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;

namespace liquid
{

Conditional::~Conditional()
{
   delete cmpOp;
   delete thenExpr;
   delete elseExpr;
}

Value* Conditional::codeGen(CodeGenContext& context)
{
   Value* comp = cmpOp->codeGen(context);
   if (comp == nullptr) {
      Node::printError("Code generation for compare operator of the conditional statement failed.");
      context.addError();
      return nullptr;
   }

   Function*   function   = context.currentBlock()->getParent();
   BasicBlock* thenBlock  = BasicBlock::Create(context.getGlobalContext(), "then", function);
   BasicBlock* elseBlock  = BasicBlock::Create(context.getGlobalContext(), "else");
   BasicBlock* mergeBlock = BasicBlock::Create(context.getGlobalContext(), "merge");
   BranchInst::Create(thenBlock, elseBlock, comp, context.currentBlock());

   bool needMergeBlock = false;
   context.setInsertPoint(thenBlock);
   Value* thenValue = thenExpr->codeGen(context);
   if (thenValue == nullptr) {
      Node::printError("Missing else block of the conditional statement.");
      context.addError();
      return nullptr;
   }
   if (context.currentBlock()->getTerminator() == nullptr) {
      BranchInst::Create(mergeBlock, context.currentBlock());
      needMergeBlock = true;
   }

   function->getBasicBlockList().push_back(elseBlock);
   context.setInsertPoint(elseBlock);
   Value* elseValue = nullptr;
   if (elseExpr != nullptr) {
      elseValue = elseExpr->codeGen(context);
   }

   if (context.currentBlock()->getTerminator() == nullptr) {
      BranchInst::Create(mergeBlock, context.currentBlock());
      needMergeBlock = true;
   }

   if (needMergeBlock) {
      function->getBasicBlockList().push_back(mergeBlock);
      context.setInsertPoint(mergeBlock);
   }

   return mergeBlock; // dummy return, for now
}

} // namespace liquid
