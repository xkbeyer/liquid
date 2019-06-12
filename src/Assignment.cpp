#include "Assignment.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;

namespace liquid
{

Value* Assignment::codeGen(CodeGenContext& context)
{
   Value* value = rhs->codeGen(context);
   if (value == nullptr) {
      Node::printError(location, " Assignment expression results in nothing");
      context.addError();
      return nullptr;
   }

   AllocaInst* var = nullptr;
   if (lhs->getStructName().empty()) {
      var = context.findVariable(lhs->getName());
      if (var == nullptr) {
         /* In this case the type deductions takes place. This is an assignment with the var keyword. */
         Type* ty                         = value->getType();
         var                              = new AllocaInst(ty, 0, lhs->getName().c_str(), context.currentBlock());
         context.locals()[lhs->getName()] = var;
      }
   } else {
      AllocaInst* varStruct = context.findVariable(lhs->getStructName());
      if (varStruct == nullptr) {
         // Check if the assignment is coming from a class member initialization.
         // In that case the context varStruct is set, which points to the member variable.
         if (context.varStruct == nullptr) {
            Node::printError(location, "undeclared variable '" + lhs->getName() + "'");
            context.addError();
            return nullptr;
         }
         varStruct = dyn_cast<AllocaInst>(context.varStruct);
         if (varStruct == nullptr) {
            Node::printError(location, "undeclared class of variable '" + lhs->getStructName() + "." + lhs->getName() + "'");
            context.addError();
            return nullptr;
         }
         std::string  klassName = lhs->getStructName();
         Instruction* ptr       = context.getKlassVarAccessInst(klassName, lhs->getName(), varStruct);
         return new StoreInst(value, ptr, false, context.currentBlock());
      }
      std::string  klassName = context.getType(lhs->getStructName());
      Instruction* ptr       = context.getKlassVarAccessInst(klassName, lhs->getName(), varStruct);
      return new StoreInst(value, ptr, false, context.currentBlock());
   }
   Type* varType = var->getType()->getElementType();

   if (value->getType()->getTypeID() == varType->getTypeID()) {
      // same type but different bit size.
      if (value->getType()->getScalarSizeInBits() > varType->getScalarSizeInBits()) {
         value = CastInst::CreateTruncOrBitCast(value, varType, "cast", context.currentBlock());
      } else if (value->getType()->getScalarSizeInBits() < varType->getScalarSizeInBits()) {
         value = CastInst::CreateZExtOrBitCast(value, varType, "cast", context.currentBlock());
      }
   } else if (value->getType() != varType) {
      std::stringstream msg;
      msg << " Assignment of incompatible types " << varType->getTypeID() << "(" << varType->getScalarSizeInBits() << ") = " << value->getType()->getTypeID()
          << "(" << value->getType()->getScalarSizeInBits() << "). Is a cast missing? ";
      Node::printError(location, msg.str());
      context.addError();
      return nullptr;
   }

   return new StoreInst(value, var, false, context.currentBlock());
}

} // namespace liquid
