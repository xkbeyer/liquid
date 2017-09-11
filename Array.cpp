#include "AstNode.h"
#include "CodeGenContext.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ADT/ArrayRef.h>

using namespace std;
using namespace llvm;

namespace liquid {

llvm::Value* Array::codeGen(CodeGenContext& context) 
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
   auto alloc_str = new AllocaInst(str, 0, "alloc_list",context.currentBlock());
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

llvm::Value* ArrayAccess::codeGen(CodeGenContext& context)
{
   AllocaInst* var = nullptr;
   Type* var_type = nullptr;
   Type* var_struct_type = nullptr;
   if( other != nullptr ) {
      auto tmp = other->codeGen(context);
      var = new AllocaInst(tmp->getType(), 0, "tmp_alloc_list_other", context.currentBlock());
      new StoreInst(tmp, var, context.currentBlock());
      var_type = var->getAllocatedType();
      var_struct_type = var->getAllocatedType()->getContainedType(0);
   } else {
      var = context.findVariable(variable->getName());
      if( var == nullptr ) {
         Node::printError(location, "unknown variable " + variable->getName());
         return nullptr;
      }
      var_type = var->getAllocatedType();
      var_struct_type = var_type->getContainedType(0);
   }
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
      Node::printError(location, variable->getName() + " : index out of range (with index(zero based) = " + std::to_string(index) + " and size = " + std::to_string(var_struct_type->getNumContainedTypes()) + ")");
      context.addError();
      return nullptr;
   }
   std::vector<Value*> ptr_indices;
   ConstantInt* const_int32_0 = ConstantInt::get(context.getModule()->getContext(), APInt(32, 0));
   ConstantInt* const_int32 = ConstantInt::get(context.getModule()->getContext(), APInt(32, index));
   ptr_indices.push_back(const_int32_0);
   ptr_indices.push_back(const_int32);
   auto val = new LoadInst(var, "load_var", context.currentBlock());
   Instruction* ptr = GetElementPtrInst::Create(var_struct_type, val, ptr_indices, "get_struct_element", context.currentBlock());
   auto value = new LoadInst(ptr, "load_ptr_struct", context.currentBlock());
   return value;
}

llvm::Value* ArrayAddElement::codeGen(CodeGenContext& context)
{
   YYLTYPE loc = { 0,0,0,0 };
   Block tmp_code;
   ExpressionList members;
   auto orgVarType = context.getType(ident->getName());
   auto tmpVarName = ident->getName() + "_tmp";
   context.renameVariable(ident->getName(), tmpVarName);
   auto var = context.findVariable(tmpVarName);
   if( var == nullptr ) {
      Node::printError(location, "unknown variable " + ident->getName());
      return nullptr;
   }
   auto var_type = var->getAllocatedType();
   auto var_struct_type = var_type->getContainedType(0);
   auto count = var_struct_type->getNumContainedTypes();
   Identifier tmpIdent(tmpVarName, loc);
   for( decltype(count) i = 0; i < count; ++i ) {
      members.push_back(new ArrayAccess(&tmpIdent, i, loc));
   }
   members.push_back(this->getExpression());
   auto newList = new Array(&members, loc);
   // Restore type name, since the rename has destroyed it and the assign doesn't set it.
   // The type name is only set while declaration.
   context.setVarType(orgVarType, ident->getName()); 
   Assignment assgn(ident, newList, loc);
   auto value = assgn.codeGen(context);
   context.deleteVariable(tmpVarName);
   return value;
}

}
