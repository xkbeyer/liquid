#include "Return.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;

namespace liquid
{

Value* Return::codeGen(CodeGenContext& context)
{
   if (retExpr) {
      Value* ret = retExpr->codeGen(context);
      if (ret == nullptr)
         return nullptr;
      return ReturnInst::Create(context.getGlobalContext(), ret, context.currentBlock());
   } else {
      return ReturnInst::Create(context.getGlobalContext(), 0, context.currentBlock());
   }
}

}
