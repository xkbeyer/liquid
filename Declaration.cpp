#include <typeinfo>
#include "AstNode.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;

namespace AST {
    
Value* VariableDeclaration::codeGen(CodeGenContext& context)
{
    Value* val = nullptr;
    if( context.findVariable(id->getName()) ) {
        Node::printError(location, " variable '" + id->getName()  + "' already exist\n");
        return nullptr;
    }

    Type* ty = context.typeOf(*type);
    if( ty->isStructTy() && context.getScopeType() != FunctionDeclarationScope ) {
        // It is really a declaration of a class type which we put always onto the heap.
        AllocaInst* alloc = new AllocaInst(ty, id->getName().c_str(), context.currentBlock());
        context.locals()[id->getName()] = alloc;
        val = alloc;
        context.varStruct = val; // Indicates that a variable of a class is declared
    }
    else
    {
        if( ty->isStructTy() ) {
            // It is a declaration of a class type in a function declaration as a formal parameter.
            // Therefor a pointer reference is needed.
            ty = PointerType::get(ty,0);
        }
        AllocaInst* alloc = new AllocaInst(ty, id->getName().c_str(), context.currentBlock());
        context.locals()[id->getName()] = alloc;
        val = alloc;
    }
    context.setVarType(type->getName(), id->getName());
    
    if (assignmentExpr != nullptr) {
        Assignment assn(id, assignmentExpr, location);
        assn.codeGen(context);
        // they are already deleted by assn.
        id = nullptr;
        assignmentExpr = nullptr;
    }
    else if ( context.varStruct )
    {
        // The variable gets nothing assigned so 
        // auto assign defaults (member assignments) on classes ctor call.
        auto stmts = context.getKlassInitCode(type->getName());
        for ( auto assign : stmts )
        {
            assign->codeGen( context );
        }

        // Generate function call to the ctor, if exists.
        Function *fn = context.getModule()->getFunction( "__init__%" + type->getName() );
        if ( fn != nullptr )
        {
            std::vector<Value*> args;
            args.push_back( context.varStruct );
            CallInst *call = CallInst::Create( fn, args, "", context.currentBlock() );
        }
        context.varStruct = nullptr;
    }
    assert(val != nullptr);
    return val;
}


Value* VariableDeclarationDeduce::codeGen( CodeGenContext& context )
{
   if( context.findVariable( id->getName() ) ) {
      Node::printError( location, " variable '" + id->getName() + "' already exist\n" );
      return nullptr;
   }

   if( !hasAssignmentExpr() ) {
       Node::printError( location, " var statements for '" + id->getName() + "' w/o assignment\n" );
       return nullptr;
   }

   Assignment assn( id, getAssignment(), location );
   auto val = assn.codeGen( context );
   assert( val != nullptr );

   context.setVarType( "var", id->getName() );

   return val;
}


Value* FunctionDeclaration::codeGen(CodeGenContext& context)
{
    vector<Type*> argTypes;
    if( !context.getKlassName().empty() ) {
        // add the self pointer of the class as first argument..
        Type* self_ty = context.typeOf(context.getKlassName());
        argTypes.push_back(PointerType::get(self_ty, 0));
    }

    for( auto varDecl : *arguments) {
        Type* ty = context.typeOf(varDecl->getIdentifierOfVariablenType());
        if( ty->isStructTy() ) {
            ty = PointerType::get(ty, 0);
        }
        argTypes.push_back(ty);
    }

    // TODO check return type if it is a struct type !!! May be it should be a ptr to the struct!
    FunctionType *ftype = FunctionType::get(context.typeOf(*type), argTypes, false);
    std::string functionName = id->getName();
    if( !context.getKlassName().empty() ) {
        functionName += "%" + context.getKlassName() ;
    }
    Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, functionName.c_str(), context.getModule());
    BasicBlock *bblock = BasicBlock::Create(context.getGlobalContext(), "entry", function, 0);
    context.newScope(bblock, FunctionDeclarationScope);

    // Put the parameter onto the stack in order to be accessed in the function.
    Function::arg_iterator actualArgs = function->arg_begin();
    if( !context.getKlassName().empty() ) {
        actualArgs->setName("this");
        Value* ptr_this = actualArgs++;
        Type* self_ty = context.typeOf(context.getKlassName());
        Type* self_ptr_ty = PointerType::get(self_ty, 0);
        AllocaInst* alloca = new AllocaInst(self_ptr_ty, "self_addr", context.currentBlock() );
        new StoreInst(ptr_this, alloca, context.currentBlock());
        context.locals()["self"] = alloca;
    }

    for ( auto varDecl : *arguments )
    {
        AllocaInst* alloca = llvm::dyn_cast<AllocaInst>(varDecl->codeGen(context));
        std::string valName = varDecl->getVariablenName();
        // TODO a struct is coming as struct alloca, but needed to be a pointer to a struct alloca.
        if( alloca ) {
            if( alloca->getAllocatedType()->isPointerTy() ) {
                valName += "_addr";
            }
            actualArgs->setName(valName);
            new StoreInst(actualArgs, alloca, context.currentBlock());
        }
        ++actualArgs;
    }
    
    block->codeGen(context);

    // If the function doesn't have a return type and doesn't have a return statement, make a ret void.
    if( type->getName() == "void" ) {
        if( context.currentBlock()->getTerminator() == nullptr ) {
            ReturnInst::Create(context.getGlobalContext(),0, context.currentBlock());
        }
    }
    context.endScope();
    return function;
}

Value* ClassDeclaration::codeGen(CodeGenContext& context)
{
    std::vector<Type*>StructTy_fields;
    context.newKlass(id->getName());
    constructStructFields(StructTy_fields, context);
    StructType::create(context.getGlobalContext(), StructTy_fields, std::string("class.") +  id->getName(), /*isPacked=*/false);
    addVarsToClassAttributes(context);
    removeVarDeclStatements();
    Value* retval = block->codeGen(context);
    context.endKlass();
    return retval;
}

void ClassDeclaration::constructStructFields(std::vector<llvm::Type* >& StructTy_fields, CodeGenContext& context)
{
    // Get all variables and put them in the struct vector.
    for( auto statement : block->statements) {
        if ( statement->getType() == NodeType::variable )
        {
            // Type Definitions
            VariableDeclaration* vardecl = dynamic_cast< VariableDeclaration* >( statement );
            StructTy_fields.push_back( context.typeOf( vardecl->getIdentifierOfVariablenType() ) );
        }
    }
}

void ClassDeclaration::addVarsToClassAttributes(CodeGenContext& context)
{
    int index = 0;
    for ( auto statement : block->statements )
    {
        if ( statement->getType() == NodeType::variable )
        {
            std::string klassName = this->id->getName();
            // Type Definitions
            VariableDeclaration* vardecl = dynamic_cast< VariableDeclaration* >( statement );
            std::string varName = vardecl->getVariablenName();
            context.klassAddVariableAccess( varName, index++ );
            if ( vardecl->hasAssignmentExpr() )
            {
                // call the assignments when class is instantiated.
                // Copy it to a place, so that it can be execute at the init time of the class.
                auto assignmentExpr = vardecl->getAssignment();
                auto ident = vardecl->getIdentifierOfVariable();
                Identifier* id = new Identifier( klassName, ident.getName(), ident.getLocation() );

                auto assn = new Assignment( id, assignmentExpr, vardecl->getLocation() );
                context.addKlassInitCode( klassName, assn );
            }
        }
    }
    
}

void ClassDeclaration::removeVarDeclStatements()
{
    auto new_end = std::remove_if( begin(block->statements), end(block->statements),
        [](Statement* statement) -> bool {
            return statement->getType() == NodeType::variable ? true : false ;
        }
    );
    block->statements.erase(new_end, end(block->statements));
}

}