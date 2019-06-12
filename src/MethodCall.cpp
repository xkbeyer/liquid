#include "MethodCall.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;

namespace liquid
{

Value* MethodCall::codeGen(CodeGenContext& context)
{
   std::string functionName = id->getName();
   if (!id->getStructName().empty()) {
      std::string className = context.getType(id->getStructName());
      functionName += "%" + className;
   }

   Function* function = context.getModule()->getFunction(functionName.c_str());
   if (function == nullptr) {
      // see if it is a added function to the class like function(classname param,...)
      functionName = id->getName();
      function     = context.getModule()->getFunction(functionName.c_str());
      if (function == nullptr) {
         // May be it is a class function, but called like a function w/o class prefix
         // like: class.function() -> function(class parameter)
         functionName = id->getName() + "%" + getTypeNameOfFirstArg(context);
         function     = context.getModule()->getFunction(functionName.c_str());

         if (function == nullptr) {
            Node::printError(location, " no such function '" + id->getName() + "'");
            context.addError();
            return nullptr;
         }
      }
   }

   std::vector<Value*> args;
   if (!id->getStructName().empty()) {
      // This a class method call, so put the class object onto the stack in order the function has
      // access via a local alloca
      Value* val = context.findVariable(id->getStructName());
      assert(val != nullptr);
      args.push_back(val);
   } else {
      // Check if first parameter is a class object, means variable of a class and a method of this class
      // exists. Then call this method.
      // Since it is possible to call a class.method(arguments)
      // like method(class, arguments).
      if (arguments->size() && arguments->front()->getType() == NodeType::identifier) {
         Identifier* ident = (Identifier*)*(arguments->begin());
         // Check if it is a var of class type...
         std::string typeName = context.getType(ident->getName());
         AllocaInst* alloca   = context.findVariable(ident->getName());
         if (alloca != nullptr) {
            if (alloca->getType()->getElementType()->isStructTy()) {
               args.push_back(alloca);
               delete ident;
               arguments->erase(begin(*arguments));
            }
         }
      }
   }

   // Put all parameter values onto the stack.
   for (auto expr : *arguments) {
      args.push_back(expr->codeGen(context));
   }
   CallInst* call = CallInst::Create(function, args, "", context.currentBlock());
   return call;
}

std::string MethodCall::getTypeNameOfFirstArg(CodeGenContext& context)
{
   if (arguments->size() && arguments->front()->getType() == NodeType::identifier) {
      Identifier* ident = (Identifier*)*(arguments->begin());
      // Check if it is a var of class type...
      return context.getType(ident->getName());
   }
   return "";
}

} // namespace liquid
