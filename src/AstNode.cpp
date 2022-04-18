#include <typeinfo>
#include <sstream>
#include "AstNode.h"
#include "CodeGenContext.h"
#include "parser.hpp"


using namespace std;
using namespace llvm;

namespace liquid {

Value* Integer::codeGen(CodeGenContext& context)
{
    return ConstantInt::get(context.getGenericIntegerType(), value, true);
}

Value* Double::codeGen(CodeGenContext& context)
{
    return ConstantFP::get(Type::getDoubleTy(context.getGlobalContext()), value);
}

Value* String::codeGen(CodeGenContext& context)
{
    // generate the type for the global var
    ArrayType* ArrayTy_0 = ArrayType::get(IntegerType::get(context.getGlobalContext(), 8), value.size() +1 );
    // create global var which holds the constant string.
    GlobalVariable* gvar_array__str = new GlobalVariable(*context.getModule(),
                                                         /*Type=*/ArrayTy_0,
                                                         /*isConstant=*/true,
                                                         GlobalValue::PrivateLinkage,
                                                         /*Initializer=*/0, // has initializer, specified below
                                                         ".str");
    gvar_array__str->setAlignment(MaybeAlign(1));
    // create the contents for the string global.
    Constant* const_array_str =  ConstantDataArray::getString(context.getGlobalContext(), value);
    // Initialize the global with the string
    gvar_array__str->setInitializer(const_array_str);

    // generate access pointer to the string
    std::vector<Constant*> const_ptr_8_indices;
    ConstantInt* const_int = ConstantInt::get(context.getGlobalContext(), APInt(64, StringRef("0"), 10));
    const_ptr_8_indices.push_back(const_int);
    const_ptr_8_indices.push_back(const_int);
    Constant* const_ptr_8 = ConstantExpr::getGetElementPtr(ArrayTy_0, gvar_array__str, const_ptr_8_indices);
    return const_ptr_8;
}

Value* Boolean::codeGen(CodeGenContext& context)
{
    return ConstantInt::get(Type::getInt1Ty(context.getGlobalContext()),boolVal);
}

Value* Identifier::codeGen(CodeGenContext& context)
{
    if( structName.empty() ) {
        // A usual stack variable
        AllocaInst* alloc = context.findVariable(name);
        if (alloc != nullptr) {
            return new LoadInst(alloc->getAllocatedType(), alloc, name, false, context.currentBlock());
        }
    } else {
        // get this ptr of struct/class etc...
        // it is a stack variable which is a reference to a class object
        AllocaInst* alloc = context.findVariable(structName);
        if (alloc != nullptr) {
            std::string klassName = context.getType(structName);
            Instruction * ptr = context.getKlassVarAccessInst(klassName, name, alloc);
            return new LoadInst(ptr->getType()->getNonOpaquePointerElementType(), ptr, name, false, context.currentBlock());
        }
    }
    Node::printError(location, "undeclared variable " + structName + "::" + name );
    context.addError();
    return nullptr;
}

Value* Block::codeGen(CodeGenContext& context)
{
    Value *last = nullptr;
    for ( auto s : statements) {
        last = s->codeGen(context);
    }
    return last;
}

Value* ExpressionStatement::codeGen(CodeGenContext& context)
{
    return expression->codeGen(context);
}

}
