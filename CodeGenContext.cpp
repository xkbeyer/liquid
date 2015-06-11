
#include <typeinfo>
#include "AstNode.h"
#include "CodeGenContext.h"
#include "parser.hpp"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#if defined(LLVM37) || defined(LLVM36)
#include "llvm/ExecutionEngine/MCJIT.h"
#else
#include "llvm/ExecutionEngine/JIT.h"
#endif
#include <stdarg.h>
#include <stdio.h>
#include <algorithm>

using namespace std;
using namespace llvm;

#if defined(_MSC_VER)
#define DECLSPEC __declspec(dllexport) 
#else
#define DECLSPEC 
#endif


#if defined(_MSC_VER)
#if defined(_M_X64)
#define UseInt64
#else
#define UseInt32
#endif
#endif

#if defined(__GNUC__)
#if defined(__x86_64)
#define UseInt64
#else
#define UseInt32
#endif
#endif


/*
 * some helper/debug function
 */
extern "C"
DECLSPEC int printvalue(int val)
{
    std::cout << "IDEBUG: " << val << "\n";
    return 1;
}

extern "C"
DECLSPEC double printdouble(double val)
{
    std::cout << "DDEBUG: " << val << "\n";
    return 1.;
}

/* ! built in display function
 * it works like the C printf function and uses the same format string definition.
 */
extern "C"
DECLSPEC void display(char * str, ...)
{
    va_list argp;
    va_start(argp,str);
    vprintf(str, argp);
    va_end(argp);
}

/* ! built in display function with newline
 * it works like the C printf function and uses the same format string definition.
 */
extern "C"
DECLSPEC void displayln(char * str, ...)
{
    char * outstr ;
    va_list argp;
    va_start(argp,str);
    outstr = (char*)malloc(strlen(str) + 2);
    strcpy(outstr, str);
    strcat(outstr, "\n");
    vprintf(outstr, argp);
    va_end(argp);
    free(outstr);
}

namespace AST {

CodeGenContext::CodeGenContext()
: self(nullptr)
, mainFunction(nullptr)
, module(nullptr)
, varStruct(nullptr)
{
    llvm::InitializeNativeTarget();
#if defined(LLVM36) || defined(LLVM37)
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
#endif
    module = new llvm::Module("myl", llvmContext);
}

/*! Setup up the built in function:
    * - printvalue
    * - printdouble
    * - sin
    * - displayln
    * - display
    */

#define MAKE_LLVM_EXTERNAL_NAME(a) #a
void CodeGenContext::setupBuiltIns()
{
    auto intType = getGenericIntegerType();

    std::vector<Type *> argTypesOneInt(1, intType);
    FunctionType * ft = FunctionType::get(intType, argTypesOneInt, false);
    Function * f = Function::Create(ft, Function::ExternalLinkage,MAKE_LLVM_EXTERNAL_NAME(printvalue),getModule());
    Function::arg_iterator i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("val");

    std::vector<Type *> argTypesOneDouble(1, Type::getDoubleTy(getGlobalContext()));
    ft = FunctionType::get(Type::getDoubleTy(getGlobalContext()), argTypesOneDouble,false);
    f = Function::Create( ft, Function::ExternalLinkage, MAKE_LLVM_EXTERNAL_NAME(printdouble), getModule() );
    i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("val");

    ft = FunctionType::get(Type::getDoubleTy(getGlobalContext()), argTypesOneDouble,false);
    f = Function::Create( ft, Function::ExternalLinkage, MAKE_LLVM_EXTERNAL_NAME(sin),getModule());
    i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("val");

    std::vector<Type *>argTypesInt8Ptr(1, Type::getInt8PtrTy(getGlobalContext()));
    ft = FunctionType::get(Type::getVoidTy(getGlobalContext()),argTypesInt8Ptr, true);
    f = Function::Create( ft, Function::ExternalLinkage, MAKE_LLVM_EXTERNAL_NAME(display), getModule() );
    i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("format_str");

    ft = FunctionType::get(Type::getVoidTy(getGlobalContext()),argTypesInt8Ptr, true);
    f = Function::Create( ft, Function::ExternalLinkage, MAKE_LLVM_EXTERNAL_NAME(displayln),getModule());
    i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("format_str");
    
}

/*! Compile the AST into a module */
void CodeGenContext::generateCode(Block& root)
{
    std::cout << "Generating code...\n";

    /* Create the top level interpreter function to call as entry */
    vector<Type*> argTypes;
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
    mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", getModule());
    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);
    setupBuiltIns();
    /* Push a new variable/block context */
    newScope(bblock);
    root.codeGen(*this); /* emit byte code for the top level block */
    if( currentBlock()->getTerminator() == nullptr ) {
        ReturnInst::Create(getGlobalContext(),0, currentBlock());
    }
    endScope();

    std::cout << "Code is generated.\n";

    std::cout << "verifying... ";
    if( verifyModule(*getModule()) ) {
       std::cout << ": Error constructing function!\n";
       return ;
    }
    std::cout << "done.\n";

#if !defined(_DEBUG)
    optimize();
#else
    /* Print the byte code in a human-readable format
     *     to see if our program compiled properly
     */
    module->dump();
#endif
}

/*! Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
    std::cout << "Running code...\n";
    std::string err;
#if defined(LLVM37) || defined(LLVM36)
    ExecutionEngine *ee = EngineBuilder( std::unique_ptr<Module>( module ) ).setErrorStr( &err ).setEngineKind( EngineKind::JIT ).create();
#else
    ExecutionEngine *ee = EngineBuilder(module).setErrorStr(&err).setEngineKind(EngineKind::JIT).create();
#endif
    assert(ee);
#if defined(LLVM37) || defined(LLVM36)
    ee->finalizeObject();
#endif
    vector<GenericValue> noargs;
    GenericValue v = ee->runFunction(mainFunction, noargs);
    std::cout << "Code was run.\n";
    delete ee;
    return v;
}

void CodeGenContext::printCodeGeneration(class Block& root)
{
    root.toString();
}

/*! Runs the optimizer over all function */
void CodeGenContext::optimize()
{
#if defined(LLVM37)
#else
    FunctionPassManager fpm(getModule());
    fpm.add(createBasicAliasAnalysisPass());
    fpm.add(createPromoteMemoryToRegisterPass());
    fpm.add(createCFGSimplificationPass());
    fpm.add(createInstructionCombiningPass());
    fpm.add(createGVNPass());
    fpm.add(createReassociatePass());
    fpm.doInitialization();
    for ( auto& fn : getModule()->getFunctionList() )
    {
        fpm.run( fn );
    }
    fpm.run(*mainFunction);
#endif
}

void CodeGenContext::newScope(BasicBlock* bb, enum ScopeType scopeType)
{
    currentScopeType = scopeType;
    if( bb == nullptr) {
        bb = llvm::BasicBlock::Create(getGlobalContext(),"scope");
    }
    codeBlocks.push_front(new CodeGenBlock(bb));
}

void CodeGenContext::endScope() {
    CodeGenBlock* top = codeBlocks.front();
    codeBlocks.pop_front();
    delete top;
    currentScopeType = CodeBlockScope;
}

/*! Searches a variable name in all known locals of the current code block.
 * If not found it looks in the current class block, too.
 * \param[in] varName variable name
 * \return The alloca instruction 
 */
AllocaInst* CodeGenContext::findVariable(std::string varName)
{
    ValueNames& names =  locals();
    if(names.find(varName) != names.end() ) {
        return names[varName];
    }
    if( self ) {
        ValueNames& names = self->getValueNames();
        if(names.find(varName) != names.end() ) {
            return names[varName];
        }
    }
    return nullptr;
}

/*! Creates a new class block scope. */
void CodeGenContext::newKlass(std::string  name)
{
    self = codeBlocks.front();
    klassName = name;
    classAttributes[klassName] = KlassValueNames();
}

/*! Closes a class block scope */
void CodeGenContext::endKlass()
{
    self = nullptr;
    klassName = "";
}

/*! Adds a class member variable name and its index to the list of the
 * class attributes map. Since the access to the corresponding llvm structure
 * goes via the index into this structure.
 * \param[in] name Variable name of the current class.
 * \param[in] index into the llvm structure of this class.
 * 
 */
void CodeGenContext::klassAddVariableAccess(std::string name, int index)
{
    classAttributes[klassName][name] = index;
}

/*! Returns the load/getelementptr instruction to access a class member.
 * \param[in] klass The class name
 * \param[in] name Class member name
 * \param[in] this_ptr The alloca instruction to be used for the getelementptr instruction.
 * \return Instruction pointer.
 * \remark Is used to access the elements via the alloca where the ptr to the ref of the class object is stored.
 *         Means that the ref is stored in a local (stack) variable.
 */
Instruction * CodeGenContext::getKlassVarAccessInst(std::string klass, std::string name, AllocaInst* this_ptr)
{
    assert( classAttributes.find(klass) != classAttributes.end());
    int index = classAttributes[klass][name];
    std::vector<Value*> ptr_indices;
    ConstantInt* const_int32_0 = ConstantInt::get(getModule()->getContext(), APInt(32, 0));
    ConstantInt* const_int32 = ConstantInt::get(getModule()->getContext(), APInt(32, index));
    ptr_indices.push_back(const_int32_0);
    ptr_indices.push_back(const_int32);
    if( this_ptr->getType()->getElementType()->isPointerTy() ) {
        // since the alloc is a ptr to ptr 
        Value * val = new LoadInst(this_ptr, "", false, currentBlock());
#if defined(LLVM37)
        return GetElementPtrInst::Create( val->getType()->getPointerElementType(), val, ptr_indices, "", currentBlock() );
#else
        return GetElementPtrInst::Create(val, ptr_indices, "", currentBlock());
#endif
    }
#if defined(LLVM37)
    Instruction* ptr = GetElementPtrInst::Create( this_ptr->getType()->getElementType(), this_ptr, ptr_indices, "", currentBlock() );
#else
    Instruction* ptr = GetElementPtrInst::Create(this_ptr, ptr_indices, "", currentBlock());
#endif
    return ptr;
}

/*! Get the type of a variable name.
 * \param[in] varName the name of the variable to be looked up.
 * \return The name of the type.
 */
std::string CodeGenContext::getType(std::string varName)
{
    if( varName == "self" ) {
        return klassName;
    }
    if( codeBlocks.front()->getTypeMap().find(varName) != codeBlocks.front()->getTypeMap().end() ){
        return codeBlocks.front()->getTypeMap()[varName];
    }
    return std::string("");
}

/* Returns an LLVM type based on the identifier */
Type* CodeGenContext::typeOf(const Identifier& type)
{
    return typeOf(type.getName());
}

Type* CodeGenContext::typeOf(const std::string name)
{
    if (name.compare("int") == 0) {
        return getGenericIntegerType();
    } else if (name.compare("double") == 0) {
        return Type::getDoubleTy(getGlobalContext());
    } else if (name.compare("string") == 0) {
        return Type::getInt8PtrTy(getGlobalContext());
    } else if (name.compare("boolean") == 0) {
        return Type::getInt1Ty(getGlobalContext());
    } else if (name.compare("void") == 0) {
        return Type::getVoidTy(getGlobalContext());
    }
    llvm::Type* ty = getModule()->getTypeByName("class." + name);
    if( ty != nullptr ) {
        return ty;
    }
    return Type::getVoidTy(getGlobalContext());
    
}

void CodeGenContext::addKlassInitCode( std::string name, Assignment* assign )
{
    classInitCode[name].insert( assign );
}

KlassInitCodeAssign& CodeGenContext::getKlassInitCode( std::string name )
{
    return classInitCode[name];
}

llvm::Type* CodeGenContext::getGenericIntegerType()
{
#if defined(UseInt64) && defined(LLVM37)
    return Type::getInt64Ty( getGlobalContext() );
#else
    return Type::getInt32Ty( getGlobalContext() );
#endif
}



}
