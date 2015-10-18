#include <typeinfo>
#include "CodeGenContext.h"
#include "parser.hpp"
#include "llvm/Transforms/Utils/Cloning.h"
#include "FunctionDeclaration.h"

using namespace std;
using namespace llvm;

namespace liquid {


FunctionDeclaration::~FunctionDeclaration()
{
    for( auto i : *arguments ) {
        delete i;
    }
    delete type;
    delete id;
    delete arguments;
    delete block;
}

Value* FunctionDeclaration::codeGen( CodeGenContext& context )
{
    vector<Type*> argTypes;
    if( !context.getKlassName().empty() ) {
        // add the self pointer of the class as first argument..
        Type* self_ty = context.typeOf( context.getKlassName() );
        argTypes.push_back( PointerType::get( self_ty, 0 ) );
    }

    for( auto varDecl : *arguments ) {
        Type* ty = context.typeOf( varDecl->getIdentifierOfVariablenType() );
        if( ty->isStructTy() ) {
            ty = PointerType::get( ty, 0 );
        }
        argTypes.push_back( ty );
    }

    // TODO check return type if it is a struct type !!! May be it should be a ptr to the struct!
    FunctionType *ftype = FunctionType::get( context.typeOf( *type ), argTypes, false );
    std::string functionName = id->getName();
    if( type->getName() == "var" ) {
        functionName += "_del";
    }
    if( !context.getKlassName().empty() ) {
        functionName += "%" + context.getKlassName();
    }
    Function *function = Function::Create( ftype, GlobalValue::InternalLinkage, functionName.c_str(), context.getModule() );
    BasicBlock *bblock = BasicBlock::Create( context.getGlobalContext(), "entry", function, 0 );
    context.newScope( bblock, ScopeType::FunctionDeclaration );

    // Put the parameter onto the stack in order to be accessed in the function.
    Function::arg_iterator actualArgs = function->arg_begin();
    // First check if a elf ptr has to be put as first argument.
    if( !context.getKlassName().empty() ) {
        actualArgs->setName( "this" );
        Value* ptr_this = actualArgs++;
        Type* self_ty = context.typeOf( context.getKlassName() );
        Type* self_ptr_ty = PointerType::get( self_ty, 0 );
        AllocaInst* alloca = new AllocaInst( self_ptr_ty, "self_addr", context.currentBlock() );
        new StoreInst( ptr_this, alloca, context.currentBlock() );
        context.locals()["self"] = alloca;
    }
    // Now the remaining arguments
    for( auto varDecl : *arguments ) {
        AllocaInst* alloca = llvm::dyn_cast< AllocaInst >(varDecl->codeGen( context ));
        std::string valName = varDecl->getVariablenName();
        // TODO a struct is coming as struct alloca, but needed to be a pointer to a struct alloca.
        if( alloca ) {
            if( alloca->getAllocatedType()->isPointerTy() ) {
                valName += "_addr";
            }
            actualArgs->setName( valName );
            new StoreInst( actualArgs, alloca, context.currentBlock() );
        }
        ++actualArgs;
    }

    // Generate the function body.
    auto blockValue = block->codeGen( context );
    auto retTy = blockValue->getType();

    // If the function doesn't have a return type and doesn't have a return statement, make a ret void.
    // Obsolete default is var.
    if( type->getName() == "void" ) {
        if( context.currentBlock()->getTerminator() == nullptr ) {
            ReturnInst::Create( context.getGlobalContext(), 0, context.currentBlock() );
        }
    }

    // If the now return instruction is generated so far ...
    if( context.currentBlock()->getTerminator() == nullptr ) {
        if( type->getName() == "var" && !retTy->isVoidTy() ) {
            // Generate one according to the value of the function body.
            ReturnInst::Create( context.getGlobalContext(), blockValue, context.currentBlock() );
        } else {
            // Or a ret void.
            ReturnInst::Create( context.getGlobalContext(), 0, context.currentBlock() );
        }
    }


    if( type->getName() == "var" ) {
        if( retTy->isLabelTy() || retTy->isMetadataTy() ) {
            context.endScope();
            Node::printError( location, " Function w/ var return type and multiple return statements are not supported: " + id->getName() + "(...)" );
            return nullptr;
        }
        // Now create the a new function (the real one) since we know the return type now.
        FunctionType *ftypeNew = FunctionType::get( blockValue->getType(), argTypes, false );
        std::string functionNameNew = id->getName();
        if( !context.getKlassName().empty() ) {
            functionNameNew += "%" + context.getKlassName();
        }

        Function *functionNew = Function::Create( ftypeNew, GlobalValue::InternalLinkage, functionNameNew.c_str(), context.getModule() );

        // Create a value map for all arguments to be mapped to the new function.
        ValueToValueMapTy VMap;
        Function::arg_iterator DestI = functionNew->arg_begin();

        for( Function::const_arg_iterator J = function->arg_begin(); J != function->arg_end(); ++J ) {
            DestI->setName( J->getName() ); // Copy name of argument to the argument of the new function.
            VMap[J] = DestI++; // Map the value 
        }

        // Clone the function to the new (real) function w/ the correct return type.
        SmallVector<ReturnInst*, 8> Returns;  // Ignore returns cloned.
        CloneFunctionInto( functionNew, function, VMap, false, Returns );

        // Remove the old one.
        function->eraseFromParent();

        function = functionNew;
    }

    context.endScope();
    return function;
}

std::string FunctionDeclaration::toString()
{
   std::stringstream s;
   s << "function declaration: " << id->getName();
   return s.str();
}

}
