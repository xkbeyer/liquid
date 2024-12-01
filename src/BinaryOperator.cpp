#include "BinaryOperator.h"
#include "CodeGenContext.h"
#include "Array.h"
#include "parser.hpp"

using namespace llvm;

namespace liquid
{

Value* BinaryOp::codeGen(CodeGenContext& context)
{
   Value* rhsValue = rhs->codeGen(context);
   Value* lhsValue = lhs->codeGen(context);
   if ((rhsValue == nullptr) || (lhsValue == nullptr)) {
      return nullptr;
   }
   auto Ty = rhsValue->getType();
   if (Ty->isPointerTy() && Ty->getPointerTo()->isStructTy()) {
      // A class or list object is added.
      return codeGenAddList(rhsValue, lhsValue, context);
   }

   if (rhsValue->getType() != lhsValue->getType()) {
     // since we only support double and int, always cast to double in case of different types.
     auto doubleTy = Type::getDoubleTy(context.getGlobalContext());
     auto cinstr   = CastInst::getCastOpcode(rhsValue, true, doubleTy, true);
     rhsValue      = CastInst::Create(cinstr, rhsValue, doubleTy, "castdb", context.currentBlock());
     cinstr        = CastInst::getCastOpcode(lhsValue, true, doubleTy, true);
     lhsValue      = CastInst::Create(cinstr, lhsValue, doubleTy, "castdb", context.currentBlock());
   }

   bool isDoubleTy = rhsValue->getType()->isFloatingPointTy();
   if (isDoubleTy && (op == TAND || op == TOR)) {
      Node::printError(location, "Binary operation (AND,OR) on floating point value is not supported. Is a cast missing?");
      context.addError();
      return nullptr;
   }

   Instruction::BinaryOps instr;
   switch (op) {
      case TPLUS:
         isDoubleTy ? instr = Instruction::FAdd : instr = Instruction::Add;
         break;
      case TMINUS:
         isDoubleTy ? instr = Instruction::FSub : instr = Instruction::Sub;
         break;
      case TMUL:
         isDoubleTy ? instr = Instruction::FMul : instr = Instruction::Mul;
         break;
      case TDIV:
         isDoubleTy ? instr = Instruction::FDiv : instr = Instruction::SDiv;
         break;
      case TAND:
         instr = Instruction::And;
         break;
      case TOR:
         instr = Instruction::Or;
         break;
      default:
         Node::printError(location, "Unknown binary operator.");
         context.addError();
         return nullptr;
   }
   return BinaryOperator::Create(instr, lhsValue, rhsValue, "mathtmp", context.currentBlock());
}

std::string BinaryOp::toString()
{
   std::stringstream s;
   s << "binary operation ";
   switch (op) {
      case TPLUS:
         s << "+";
         break;
      case TMINUS:
         s << "-";
         break;
      case TMUL:
         s << "*";
         break;
      case TDIV:
         s << "/";
         break;
      case TAND:
         s << "and";
         break;
      case TOR:
         s << "or";
         break;
      default:
         s << "unknown";
   }
   return s.str();
}

llvm::Value* BinaryOp::codeGenAddList(llvm::Value* rhsValue, llvm::Value* lhsValue, CodeGenContext& context)
{
   auto rhsTy = rhsValue->getType()->getNonOpaquePointerElementType();
   auto lhsTy = lhsValue->getType()->getNonOpaquePointerElementType();
   if (!lhsTy->isStructTy()) {
      Node::printError(location, "First operand is not of a list type.");
      return nullptr;
   }
   if (!rhsTy->isStructTy()) {
      Node::printError(location, "Second operand is not of a list type.");
      return nullptr;
   }

   if (getLHS()->getType() != NodeType::identifier) {
      Node::printError(location, "First operand must be an identifier.");
      return nullptr;
   }
   if (getRHS()->getType() != NodeType::identifier) {
      Node::printError(location, "Second operand must be an identifier.");
      return nullptr;
   }
   if (op != TPLUS) {
      Node::printError(location, "Only operator addition is currently supported.");
      return nullptr;
   }

   // Construct a new list with the contents of the both.
   auto           rhsCount = rhsTy->getNumContainedTypes();
   auto           lhsCount = lhsTy->getNumContainedTypes();
   ExpressionList exprList;
   for (unsigned int i = 0; i < lhsCount; ++i) {
      auto         id     = (Identifier*)this->getLHS();
      ArrayAccess* access = new ArrayAccess(id, i, id->getLocation());
      exprList.push_back(access);
   }
   for (unsigned int i = 0; i < rhsCount; ++i) {
      auto         id     = (Identifier*)this->getRHS();
      ArrayAccess* access = new ArrayAccess(id, i, id->getLocation());
      exprList.push_back(access);
   }
   auto list    = new Array(&exprList, location);
   auto newList = list->codeGen(context);
   return newList;
}

} // namespace liquid
