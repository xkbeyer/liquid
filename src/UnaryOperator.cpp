#include "UnaryOperator.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;

namespace liquid
{

Value* UnaryOperator::codeGen(CodeGenContext& context)
{
   Instruction::BinaryOps instr;
   switch (op) {
      case TNOT:
         instr = Instruction::Xor;
         break;
      default: // TODO user defined operator
         Node::printError("Unknown uni operator.");
         context.addError();
         return nullptr;
   }

   Value* rhsValue = rhs->codeGen(context);
   if (!rhsValue->getType()->isIntegerTy()) {
      Node::printError("Right hand side of uni operator must be an integer type.");
      context.addError();
      return nullptr;
   }
   Value* lhsValue = ConstantInt::get(IntegerType::get(context.getGlobalContext(), context.getGenericIntegerType()->getIntegerBitWidth()), StringRef("-1"), 10);
   return BinaryOperator::Create(instr, lhsValue, rhsValue, "unarytmp", context.currentBlock());
}

std::string UnaryOperator::toString()
{
   std::stringstream s;
   s << "unary operation ";
   switch (op) {
      case TNOT:
         s << "not";
         break;
      default: // TODO user defined operator
         s << "unknown";
   }
   return s.str();
}

} // namespace liquid
