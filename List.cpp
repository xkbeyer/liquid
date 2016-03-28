#include "AstNode.h"
#include "CodeGenContext.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ADT/ArrayRef.h>

using namespace std;
using namespace llvm;

namespace liquid {

llvm::Value* List::codeGen(CodeGenContext& context) 
{
   using ValueList = std::vector<Value*>;
   using TypeList = std::vector<Type*>;
   ValueList values;
   TypeList types;
   for( auto e : *exprList ) {
      auto code = e->codeGen(context);
      values.push_back(code);
      types.push_back(code->getType());
   }
   StructType* str = StructType::create(context.getGlobalContext(), makeArrayRef(types), "list");
   auto alloc_str = new AllocaInst(str, "alloc_list",context.currentBlock());
   std::vector<Value*> ptr_indices;
   ConstantInt* const_int32_0 = ConstantInt::get(context.getModule()->getContext(), APInt(32, 0));
   for( int index = 0; index < values.size(); ++index ) {
      ptr_indices.clear();
      ptr_indices.push_back(const_int32_0);
      ConstantInt* const_int32 = ConstantInt::get(context.getModule()->getContext(), APInt(32, index));
      ptr_indices.push_back(const_int32);
      Instruction* ptr = GetElementPtrInst::Create(alloc_str->getType()->getElementType(), alloc_str, ptr_indices, "", context.currentBlock());
      new StoreInst(values[index], ptr, context.currentBlock());
   }
   return alloc_str; 
}

llvm::Value* ListAccess::codeGen(CodeGenContext& context)
{
   auto var = context.findVariable(variable->getName());
   if( var == nullptr ) {
      Node::printError(location, "unknown variable " + variable->getName());
      return nullptr;
   }
   auto var_type = var->getAllocatedType();
   auto var_struct_type = var_type->getContainedType(0);
   if( var_struct_type == nullptr ) {
      Node::printError(location, "Type mismatch: variable " + variable->getName() + " must have type list but has type " + context.getType(variable->getName()));
      context.addError();
      return nullptr;
   }
   if( var_struct_type->getTypeID() != StructType::StructTyID ) {
      Node::printError(location, "Type mismatch: variable " + variable->getName() + " must have type list but has type " + context.getType(variable->getName()));
      context.addError();
      return nullptr;
   }
   if( var_struct_type->getNumContainedTypes() <= index ) {
      Node::printError(location, variable->getName() + ": index out of range (with index = " + std::to_string(index) + " and size = " + std::to_string(var_type->getNumContainedTypes()) + ")");
      context.addError();
      return nullptr;
   }
   std::vector<Value*> ptr_indices;
   ConstantInt* const_int32_0 = ConstantInt::get(context.getModule()->getContext(), APInt(32, 0));
   ConstantInt* const_int32 = ConstantInt::get(context.getModule()->getContext(), APInt(32, index));
   ptr_indices.push_back(const_int32_0);
   ptr_indices.push_back(const_int32);
   auto val = new LoadInst(var, "", context.currentBlock());
   Instruction* ptr = GetElementPtrInst::Create(var_struct_type, val, ptr_indices, "", context.currentBlock());
   auto value = new LoadInst(ptr, "load_ptr_struct", context.currentBlock());
   context.currentBlock()->dump();
   return value;
}

}
