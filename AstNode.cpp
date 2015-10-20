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
    // generate the type for the globale var
    ArrayType* ArrayTy_0 = ArrayType::get(IntegerType::get(context.getGlobalContext(), 8), value.size() +1 );
    // create global var which holds the constant string.
    GlobalVariable* gvar_array__str = new GlobalVariable(*context.getModule(),
                                                         /*Type=*/ArrayTy_0,
                                                         /*isConstant=*/true,
                                                         GlobalValue::PrivateLinkage,
                                                         /*Initializer=*/0, // has initializer, specified below
                                                         ".str");
    gvar_array__str->setAlignment(1);
    // create the contents for the string global.
    Constant* const_array_str =  ConstantDataArray::getString(context.getGlobalContext(), value);
    // Initialize the global with the string
    gvar_array__str->setInitializer(const_array_str);

    // generate access pointer to the string
    std::vector<Constant*> const_ptr_8_indices;
    ConstantInt* const_int = ConstantInt::get(context.getGlobalContext(), APInt(64, StringRef("0"), 10));
    const_ptr_8_indices.push_back(const_int);
    const_ptr_8_indices.push_back(const_int);
#if defined(LLVM37)
    Constant* const_ptr_8 = ConstantExpr::getGetElementPtr(ArrayTy_0, gvar_array__str, const_ptr_8_indices);
#else
    Constant* const_ptr_8 = ConstantExpr::getGetElementPtr(gvar_array__str, const_ptr_8_indices);
#endif
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
            return new LoadInst(alloc, name, false, context.currentBlock());
        }
    } else {
        // get this ptr of struct/class etc...
        // it is a stack variable which is a reference to a class object
        AllocaInst* alloc = context.findVariable(structName);
        if (alloc != nullptr) {
            std::string klassName = context.getType(structName);
            Instruction * ptr = context.getKlassVarAccessInst(klassName, name, alloc);
            return new LoadInst(ptr, name, false, context.currentBlock());
        }
    }
    Node::printError(location, "undeclared variable " + structName + "::" + name );
    context.addError();
    return nullptr;
}

Value* MethodCall::codeGen(CodeGenContext& context)
{
    std::string functionName = id->getName();
    if( !id->getStructName().empty() ) {
        std::string className = context.getType(id->getStructName());
        functionName += "%" + className;
    }
    
    Function *function = context.getModule()->getFunction(functionName.c_str());
    if (function == nullptr) {
        // see if it is a added function to the class like function(classname param,...)
        functionName = id->getName();
        function = context.getModule()->getFunction(functionName.c_str());
        if( function == nullptr ) {
            // May be it is a class function, but called like a function w/o class prefix
            // like: class.function() -> function(class parameter)
            functionName = id->getName() + "%" + getTypeNameOfFirstArg(context);
            function = context.getModule()->getFunction(functionName.c_str());

            if( function == nullptr ) {
                Node::printError(location," no such function '" + id->getName() + "'" );
                context.addError();
                return nullptr;
            }
        }
    }
    
    std::vector<Value*> args;
    if( !id->getStructName().empty() ) {
        // This a class method call, so put the class object onto the stack in order the function has
        // access via a local alloca
        Value* val = context.findVariable(id->getStructName());
        assert( val != nullptr );
        args.push_back(val);
    } else {
        // Check if first parameter is a class object, means variable of a class and a method of this class
        // exists. Then call this method.
        // Since it is possible to call a class.method(arguments) 
        // like method(class, arguments).
        if( arguments->size() && arguments->front()->getType() == NodeType::identifier ) {
            Identifier* ident =(Identifier*) *(arguments->begin());
            // Check if it is a var of class type...
            std::string typeName = context.getType(ident->getName());
            AllocaInst* alloca = context.findVariable(ident->getName()); 
            if( alloca != nullptr ) {
                if( alloca->getType()->getElementType()->isStructTy() ) {
                    args.push_back(alloca);
                    delete ident;
                    arguments->erase(begin(*arguments) );
                }
            }
        }
    }

    // Put all parameter values onto the stack.
    for( auto expr : *arguments ) {
        args.push_back(expr->codeGen(context));
    }
    CallInst *call = CallInst::Create(function, args, "", context.currentBlock());
    return call;
}

std::string MethodCall::getTypeNameOfFirstArg(CodeGenContext& context)
{
    if( arguments->size() && arguments->front()->getType() == NodeType::identifier ) {
        Identifier* ident =(Identifier*) *(arguments->begin());
        // Check if it is a var of class type...
        return context.getType(ident->getName());
    }
    return "";
}

Value* UnaryOperator::codeGen(CodeGenContext& context)
{
    Instruction::BinaryOps instr;
    switch(op) {
        case TNOT: instr = Instruction::Xor; break;
        default: // TODO user defined operator
           Node::printError("Unknown uni operator.");
           context.addError();
           return nullptr;
    }

    Value* rhsValue = rhs->codeGen(context);
    if(!rhsValue->getType()->isIntegerTy()) {
       Node::printError("Right hand side of uni operator must be an integer type.");
       context.addError();
       return nullptr;
    }
    Value* lhsValue = ConstantInt::get(IntegerType::get(context.getGlobalContext(),context.getGenericIntegerType()->getIntegerBitWidth()), StringRef("-1"),10);
    return BinaryOperator::Create(instr, lhsValue, rhsValue, "unarytmp", context.currentBlock());
}

std::string UnaryOperator::toString() 
{ 
   std::stringstream s; 
   s << "unary operation ";
   switch(op) {
      case TNOT: s << "not"; break;
      default: // TODO user defined operator
         s << "unknown";
   }
   return s.str();
}

Value* BinaryOp::codeGen(CodeGenContext& context)
{

    Value* rhsValue = rhs->codeGen( context );
    Value* lhsValue = lhs->codeGen( context );
    if( rhsValue->getType() != lhsValue->getType() ) {
       // since we only support double and int, always cast to double in case of different types.
       auto doubleTy = Type::getDoubleTy( context.getGlobalContext() );
       auto cinstr = CastInst::getCastOpcode( rhsValue, true, doubleTy, true );
       rhsValue = CastInst::Create( cinstr, rhsValue, doubleTy, "castdb", context.currentBlock() );
       cinstr = CastInst::getCastOpcode( lhsValue, true, doubleTy, true );
       lhsValue = CastInst::Create( cinstr, lhsValue, doubleTy, "castdb", context.currentBlock() );
    }

    bool isDoubleTy = rhsValue->getType()->isFloatingPointTy();
    if( isDoubleTy && (op == TAND || op == TOR) ) {
       Node::printError( location, "Binary operation (AND,OR) on floating point value is not supported. Is a cast missing?" );
       context.addError();
       return nullptr;
    }

    Instruction::BinaryOps instr;
    switch( op ) {
       case TPLUS: isDoubleTy ? instr = Instruction::FAdd : instr = Instruction::Add; break;
       case TMINUS:isDoubleTy ? instr = Instruction::FSub : instr = Instruction::Sub; break;
       case TMUL:  isDoubleTy ? instr = Instruction::FMul : instr = Instruction::Mul; break;
       case TDIV:  isDoubleTy ? instr = Instruction::FDiv : instr = Instruction::SDiv; break;
       case TAND:  instr = Instruction::And; break;
       case TOR:   instr = Instruction::Or; break;
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
   s << "binary operation " ;  
   switch(op) {
      case TPLUS: s << "+"; break;
      case TMINUS:s << "-"; break;
      case TMUL:  s << "*"; break;
      case TDIV:  s << "/"; break;
      case TAND:  s << "and"; break;
      case TOR:   s << "or"; break;
      default: s << "unknown";
   }
   return s.str();
}

Value* CompOperator::codeGen(CodeGenContext& context)
{
    Value * rhsVal = rhs->codeGen(context);
    Value * lhsVal = lhs->codeGen(context);
    if( rhsVal->getType() != lhsVal->getType() ) {
        // since we only support double and int, always cast to double in case of different types.
        auto cinstr = CastInst::getCastOpcode(rhsVal,true, Type::getDoubleTy(context.getGlobalContext()), true);
        rhsVal = CastInst::Create(cinstr, rhsVal , Type::getDoubleTy(context.getGlobalContext()), "castdb" , context.currentBlock());
        cinstr = CastInst::getCastOpcode(lhsVal,true, Type::getDoubleTy(context.getGlobalContext()), true);
        lhsVal = CastInst::Create(cinstr,lhsVal, Type::getDoubleTy(context.getGlobalContext()), "castdb" , context.currentBlock());
    }

    bool isDouble = rhsVal->getType() == Type::getDoubleTy( context.getGlobalContext() );
    Instruction::OtherOps oinstr = isDouble ? Instruction::FCmp : Instruction::ICmp;

    unsigned short predicate;
    switch ( op )
    {
        case TCGE: predicate = isDouble ? CmpInst::FCMP_OGE : CmpInst::ICMP_SGE;break;
        case TCGT: predicate = isDouble ? CmpInst::FCMP_OGT : CmpInst::ICMP_SGT;break;
        case TCLT: predicate = isDouble ? CmpInst::FCMP_OLT : CmpInst::ICMP_SLT;break;
        case TCLE: predicate = isDouble ? CmpInst::FCMP_OLE : CmpInst::ICMP_SLE;break;
        case TCEQ: predicate = isDouble ? CmpInst::FCMP_OEQ : CmpInst::ICMP_EQ ;break;
        case TCNE: predicate = isDouble ? CmpInst::FCMP_ONE : CmpInst::ICMP_NE; break;
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
   s << "compare operation " ; 
   switch(op) {
      case TCGE: s << ">="; break;
      case TCGT: s << ">" ; break;
      case TCLT: s << "<" ; break;
      case TCLE: s << "<="; break;
      case TCEQ: s << "=="; break;
      case TCNE: s << "!="; break;
      default: s << "unknown";
   }
   return s.str(); }

Value* Assignment::codeGen(CodeGenContext& context)
{
    Value* value = rhs->codeGen(context);
    if( value == nullptr ) {
        Node::printError(location," Assignment expression results in nothing");
        context.addError();
        return nullptr;
    }

    AllocaInst * var = nullptr;
    if( lhs->getStructName().empty() ) {
        var = context.findVariable(lhs->getName()) ;
        if( var == nullptr ) {
            /* In this case the type deductions takes place. This is an assignment with the var keyword. */
            Type* ty = value->getType();
            var = new AllocaInst( ty, lhs->getName().c_str(), context.currentBlock() );
            context.locals()[lhs->getName()] = var;
        }
    } else {
        AllocaInst* varStruct = context.findVariable(lhs->getStructName()) ;
        if( varStruct == nullptr ) {
            // Check if the assignment is coming from a class member initialization.
            // In that case the context varStruct is set, which points to the member variable.
            if ( context.varStruct == nullptr)
            {
                Node::printError( location, "undeclared variable '" + lhs->getName() + "'" );
                context.addError();
                return nullptr;
            }
            varStruct = dyn_cast<AllocaInst>(context.varStruct);
            if ( varStruct == nullptr )
            {
                Node::printError( location, "undeclared class of variable '" + lhs->getStructName() + "." + lhs->getName() + "'" );
                context.addError();
                return nullptr;
            }
            std::string klassName = lhs->getStructName() ;
            Instruction* ptr = context.getKlassVarAccessInst( klassName, lhs->getName(), varStruct );
            return new StoreInst( value, ptr, false, context.currentBlock() );

        }
        std::string klassName = context.getType(lhs->getStructName());
        Instruction* ptr = context.getKlassVarAccessInst(klassName, lhs->getName(), varStruct);
        return new StoreInst(value, ptr, false, context.currentBlock());
    }
    Type* varType = var->getType()->getElementType();

    if(value->getType()->getTypeID() == varType->getTypeID()) {
        // same type but different bit size.
        if( value->getType()->getScalarSizeInBits() != varType->getScalarSizeInBits() ) {
            value = CastInst::CreateTruncOrBitCast(value, varType, "cast", context.currentBlock());
        }
    } else if ( value->getType() != varType ) {
        std::stringstream msg ;
        msg << " Assignment of incompatible types "
                  << varType->getTypeID() << "(" << varType->getScalarSizeInBits() << ") = "
                  << value->getType()->getTypeID()
                  << "(" << value->getType()->getScalarSizeInBits() << "). Is a cast missing? ";
        Node::printError(location, msg.str());
        context.addError();
        return nullptr;
    }

    return new StoreInst(value, var, false, context.currentBlock());
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

Value* Conditional::codeGen(CodeGenContext& context)
{
    Value* comp = cmpOp->codeGen(context);
    if(comp == nullptr) {
       Node::printError("Code generation for compare operator of the conditional statement failed.");
       context.addError();
       return nullptr;
    }

    Function* function = context.currentBlock()->getParent();
    BasicBlock* thenBlock = BasicBlock::Create(context.getGlobalContext(), "then",function);
    BasicBlock* elseBlock = BasicBlock::Create(context.getGlobalContext(), "else");
    BasicBlock* mergeBlock = BasicBlock::Create(context.getGlobalContext(), "merge");
    BranchInst::Create(thenBlock,elseBlock,comp,context.currentBlock());

    bool needMergeBlock = false;
    context.setInsertPoint(thenBlock);
    Value* thenValue = thenExpr->codeGen(context);
    if(thenValue == nullptr) {
       Node::printError("Missing else block of the conditional statement.");
       context.addError();
       return nullptr;
    }
    if( context.currentBlock()->getTerminator() == nullptr) {
        BranchInst::Create(mergeBlock,context.currentBlock());
        needMergeBlock = true;
    }

    function->getBasicBlockList().push_back(elseBlock);
    context.setInsertPoint(elseBlock);
    Value* elseValue = nullptr;
    if( elseExpr != nullptr ) {
        elseValue = elseExpr->codeGen(context);
    }

    if( context.currentBlock()->getTerminator() == nullptr) {
        BranchInst::Create(mergeBlock,context.currentBlock());
        needMergeBlock = true;
    }

    if( needMergeBlock ) {
        function->getBasicBlockList().push_back(mergeBlock);
        context.setInsertPoint(mergeBlock);
    }

    return mergeBlock; // dummy return, for now
}

Value* WhileLoop::codeGen(CodeGenContext& context)
{
    Function* function = context.currentBlock()->getParent();
    BasicBlock* firstCondBlock = BasicBlock::Create(context.getGlobalContext(), "firstcond",function);
    BasicBlock* condBlock = BasicBlock::Create(context.getGlobalContext(), "cond");
    BasicBlock* loopBlock = BasicBlock::Create(context.getGlobalContext(), "loop");
    BasicBlock* elseBlock = BasicBlock::Create(context.getGlobalContext(), "else");
    BasicBlock* mergeBlock = BasicBlock::Create(context.getGlobalContext(), "merge");
    BranchInst::Create(firstCondBlock,context.currentBlock());

    context.setInsertPoint(firstCondBlock);
    Value* firstCondValue = this->condition->codeGen(context);
    if(firstCondValue == nullptr) {
       Node::printError("Missing condition in while loop.");
       context.addError();
       return nullptr;
    }
    BranchInst::Create(loopBlock,elseBlock,firstCondValue,context.currentBlock());

    function->getBasicBlockList().push_back(condBlock);
    context.setInsertPoint(condBlock);
    Value* condValue = this->condition->codeGen(context);
    if(condValue == nullptr) {
       Node::printError("Code gen for condition expression in while loop failed.");
       context.addError();
       return nullptr;
    }
    BranchInst::Create(loopBlock,mergeBlock,condValue,context.currentBlock());

    function->getBasicBlockList().push_back(loopBlock);
    context.setInsertPoint(loopBlock);
    Value* loopValue = this->loopBlock->codeGen(context);
    if(loopValue == nullptr) {
       Node::printError("Code gen for loop value in while loop failed.");
       context.addError();
       return nullptr;
    }
    BranchInst::Create(condBlock,context.currentBlock());

    function->getBasicBlockList().push_back(elseBlock);
    context.setInsertPoint(elseBlock);
    if( this->elseBlock != nullptr ) {
        Value* elseValue = this->elseBlock->codeGen(context);
        if(elseValue == nullptr) {
           Node::printError("Code gen for else block in while loop failed.");
           context.addError();
           return nullptr;
        }
    }
    BranchInst::Create(mergeBlock,context.currentBlock());
    function->getBasicBlockList().push_back(mergeBlock);
    context.setInsertPoint(mergeBlock);

    return mergeBlock;
}

Value* Return::codeGen(CodeGenContext& context)
{
    if( retExpr ) {
        Value* ret = retExpr->codeGen(context);
        if( ret == nullptr ) return nullptr;
        return ReturnInst::Create(context.getGlobalContext(), ret, context.currentBlock());
    } else {
        return ReturnInst::Create(context.getGlobalContext(), 0, context.currentBlock());
    }
}

}
