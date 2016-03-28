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

}
