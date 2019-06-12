#include <typeinfo>
#include <sstream>
#include "CompareOperator.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;

namespace liquid
{

Value* CompOperator::codeGen(CodeGenContext& context)
{
   Value* rhsVal = rhs->codeGen(context);
   Value* lhsVal = lhs->codeGen(context);
   if (rhsVal->getType() != lhsVal->getType()) {
      // since we only support double and int, always cast to double in case of different types.
      auto cinstr = CastInst::getCastOpcode(rhsVal, true, Type::getDoubleTy(context.getGlobalContext()), true);
      rhsVal      = CastInst::Create(cinstr, rhsVal, Type::getDoubleTy(context.getGlobalContext()), "castdb", context.currentBlock());
      cinstr      = CastInst::getCastOpcode(lhsVal, true, Type::getDoubleTy(context.getGlobalContext()), true);
      lhsVal      = CastInst::Create(cinstr, lhsVal, Type::getDoubleTy(context.getGlobalContext()), "castdb", context.currentBlock());
   }

   bool                  isDouble = rhsVal->getType() == Type::getDoubleTy(context.getGlobalContext());
   Instruction::OtherOps oinstr   = isDouble ? Instruction::FCmp : Instruction::ICmp;

   CmpInst::Predicate predicate;
   switch (op) {
      case TCGE:
         predicate = isDouble ? CmpInst::FCMP_OGE : CmpInst::ICMP_SGE;
         break;
      case TCGT:
         predicate = isDouble ? CmpInst::FCMP_OGT : CmpInst::ICMP_SGT;
         break;
      case TCLT:
         predicate = isDouble ? CmpInst::FCMP_OLT : CmpInst::ICMP_SLT;
         break;
      case TCLE:
         predicate = isDouble ? CmpInst::FCMP_OLE : CmpInst::ICMP_SLE;
         break;
      case TCEQ:
         predicate = isDouble ? CmpInst::FCMP_OEQ : CmpInst::ICMP_EQ;
         break;
      case TCNE:
         predicate = isDouble ? CmpInst::FCMP_ONE : CmpInst::ICMP_NE;
         break;
      default:
         Node::printError("Unknown compare operator.");
         context.addError();
         return nullptr;
   }

   return CmpInst::Create(oinstr, predicate, lhsVal, rhsVal, "cmptmp", context.currentBlock());
}

std::string CompOperator::toString()
{
   std::stringstream s;
   s << "compare operation ";
   switch (op) {
      case TCGE:
         s << ">=";
         break;
      case TCGT:
         s << ">";
         break;
      case TCLT:
         s << "<";
         break;
      case TCLE:
         s << "<=";
         break;
      case TCEQ:
         s << "==";
         break;
      case TCNE:
         s << "!=";
         break;
      default:
         s << "unknown";
   }
   return s.str();
}

} // namespace liquid
