
#include <typeinfo>
#include "AstNode.h"
#include "CodeGenContext.h"
#include "parser.hpp"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include <stdarg.h>
#include <stdio.h>
#include <algorithm>
#include <cmath>

#include "buildins.h"
#include "VisitorSyntaxCheck.h"
#include "VisitorPrettyPrint.h"

using namespace std;
using namespace llvm;

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

namespace liquid
{

CodeGenContext::CodeGenContext(std::ostream& outs) : outs(outs)
{
   llvm::InitializeNativeTarget();
   llvm::InitializeNativeTargetAsmParser();
   llvm::InitializeNativeTargetAsmPrinter();
   module = new llvm::Module("liquid", llvmContext);
}

#define MAKE_LLVM_EXTERNAL_NAME(a) #a
void CodeGenContext::setupBuiltIns()
{
   intType = getGenericIntegerType();
   doubleType = Type::getDoubleTy(getGlobalContext());
   stringType = PointerType::getUnqual(Type::getInt8Ty(getGlobalContext()));
   boolType = Type::getInt1Ty(getGlobalContext());
   voidType = Type::getVoidTy(getGlobalContext());
   varType = StructType::create(getGlobalContext(), "var");
   llvmTypeMap["int"] = intType;
   llvmTypeMap["double"] = doubleType;
   llvmTypeMap["string"] = stringType;
   llvmTypeMap["boolean"] = boolType;
   llvmTypeMap["void"] = voidType;
   llvmTypeMap["var"] = varType;

   std::vector<Type*>     argTypesOneInt(1, intType);
   FunctionType*          ft = FunctionType::get(intType, argTypesOneInt, false);
   Function*              f  = Function::Create(ft, Function::ExternalLinkage, MAKE_LLVM_EXTERNAL_NAME(printvalue), getModule());
   Function::arg_iterator i  = f->arg_begin();
   if (i != f->arg_end())
      i->setName("val");
   builtins.push_back({f, (void*)printvalue});

   std::vector<Type*> argTypesOneDouble(1, Type::getDoubleTy(getGlobalContext()));
   ft = FunctionType::get(Type::getDoubleTy(getGlobalContext()), argTypesOneDouble, false);
   f  = Function::Create(ft, Function::ExternalLinkage, MAKE_LLVM_EXTERNAL_NAME(printdouble), getModule());
   i  = f->arg_begin();
   if (i != f->arg_end())
      i->setName("val");
   builtins.push_back({f, (void*)printdouble});

    ft = FunctionType::get(Type::getDoubleTy(getGlobalContext()), argTypesOneDouble,false);
    f = Function::Create( ft, Function::ExternalLinkage, MAKE_LLVM_EXTERNAL_NAME(sin),getModule());
    i = f->arg_begin();
    if( i != f->arg_end() )
       i->setName("val");
   builtins.push_back({f, (void*)sinus});

   std::vector<Type*> argTypesInt8Ptr(1, stringType);
   ft = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypesInt8Ptr, true);
   f  = Function::Create(ft, Function::ExternalLinkage, MAKE_LLVM_EXTERNAL_NAME(display), getModule());
   i  = f->arg_begin();
   if (i != f->arg_end())
      i->setName("format_str");
   builtins.push_back({f, (void*)display});

   ft = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypesInt8Ptr, true);
   f  = Function::Create(ft, Function::ExternalLinkage, MAKE_LLVM_EXTERNAL_NAME(displayln), getModule());
   i  = f->arg_begin();
   if (i != f->arg_end())
      i->setName("format_str");
   builtins.push_back({f, (void*)displayln});

}

bool CodeGenContext::generateCode(Block& root)
{
   outs << "Generating code...\n";

   /* Create the top level interpreter function to call as entry */
   vector<Type*> argTypes;
   FunctionType* ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
   mainFunction        = Function::Create(ftype, GlobalValue::InternalLinkage, "main", getModule());
   BasicBlock* bblock  = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);
   setupBuiltIns();
   /* Push a new variable/block context */
   newScope(bblock);
   root.codeGen(*this); /* emit byte code for the top level block */
   if (errors) {
      outs << "Compilation error(s). Abort.\n";
      return false;
   }
   if (currentBlock()->getTerminator() == nullptr) {
      ReturnInst::Create(getGlobalContext(), 0, currentBlock());
   }
   endScope();

   outs << "Code is generated.\n";

   outs << "verifying... ";
   llvm::raw_os_ostream rawouts(outs);
   if (verifyModule(*getModule(), &rawouts)) {
      outs << ": Error constructing function!\n";
#if !defined(LLVM_NO_DUMP)
      module->dump();
#endif
      return false;
   }
   outs << "done.\n";

   if (!debug) {
      optimize();
   }
#if !defined(LLVM_NO_DUMP) // Only the debug build of LLVM has a dump() method.
   /* Print the byte code in a human-readable format
    *     to see if our program compiled properly
    */
   if (verbose)
      module->dump();
#endif
   return true;
}

GenericValue CodeGenContext::runCode()
{
   outs << "Running code...\n";
   std::string      err;
   ExecutionEngine* ee = EngineBuilder(std::unique_ptr<Module>(module)).setErrorStr(&err).setEngineKind(EngineKind::JIT).create();
   assert(ee);
   for (auto info : builtins) {
      ee->addGlobalMapping(info.f, info.addr);
   }

   ee->finalizeObject();
   vector<GenericValue> noargs;
   GenericValue         v = ee->runFunction(mainFunction, noargs);
   outs << "Code was run.\n";
   delete ee;
   return v;
}

void CodeGenContext::printCodeGeneration(class Block& root, std::ostream& outstream)
{
   VisitorPrettyPrint visitor(outstream);
   root.Accept(visitor);
}

void CodeGenContext::optimize()
{
   outs << "Optimize code...\n";
   LoopAnalysisManager LAM;
   FunctionAnalysisManager FAM;
   CGSCCAnalysisManager CGAM;
   ModuleAnalysisManager MAM;
   PassBuilder PB;
   PB.registerModuleAnalyses(MAM);
   PB.registerCGSCCAnalyses(CGAM);
   PB.registerFunctionAnalyses(FAM);
   PB.registerLoopAnalyses(LAM);
   PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
   ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O2);
   // Optimize the IR!
   MPM.run(*getModule(), MAM);
}

void CodeGenContext::newScope(BasicBlock* bb, ScopeType scopeType)
{
   currentScopeType = scopeType;
   if (bb == nullptr) {
      bb = llvm::BasicBlock::Create(getGlobalContext(), "scope");
   }
   codeBlocks.push_front(new CodeGenBlock(bb));
}

void CodeGenContext::endScope()
{
   CodeGenBlock* top = codeBlocks.front();
   codeBlocks.pop_front();
   delete top;
   currentScopeType = ScopeType::CodeBlock;
}

AllocaInst* CodeGenContext::findVariable(std::string varName)
{
   if (currentScopeType == ScopeType::FunctionDeclaration) {
      // Only look in current scope, since outer scope isn't valid while in function declaration.
      auto& names = locals();
      if (names.find(varName) != names.end()) {
         return names[varName];
      }
      return nullptr;
   }

   // Travers from inner to outer scope (block) to find the variable.
   for (auto& cb : codeBlocks) {
      auto& names = cb->getValueNames();
      if (names.find(varName) != names.end()) {
         return names[varName];
      }
   }

   // If we are in a class then check the calls variables, too.
   if (self) {
      ValueNames& vnames = self->getValueNames();
      if (vnames.find(varName) != vnames.end()) {
         return vnames[varName];
      }
   }
   return nullptr;
}

void CodeGenContext::deleteVariable(std::string varName)
{
   ValueNames& names   = locals();
   auto&       typeMap = codeBlocks.front()->getTypeMap();
   if (names.find(varName) != names.end()) {
      names.erase(varName);
      if (typeMap.count(varName)) {
         typeMap.erase(varName);
      }
   }
}

void CodeGenContext::renameVariable(std::string oldVarName, std::string newVarName)
{
   ValueNames& names = locals();
   if (names.find(oldVarName) != names.end()) {
      auto value = names[oldVarName];
      names.erase(oldVarName);
      names[newVarName] = value;
      auto& typeMap     = codeBlocks.front()->getTypeMap();
      if (typeMap.count(oldVarName)) {
         auto val = typeMap[oldVarName];
         typeMap.erase(oldVarName);
         typeMap[newVarName] = val;
      }
   }
}

void CodeGenContext::newKlass(std::string name)
{
   self                       = codeBlocks.front();
   klassName                  = name;
   classAttributes[klassName] = KlassValueNames();
}

void CodeGenContext::endKlass()
{
   self      = nullptr;
   klassName = "";
}

void CodeGenContext::klassAddVariableAccess(std::string name, int index, llvm::Type* type) { classAttributes[klassName][name] = {index, type}; }

Instruction* CodeGenContext::getKlassVarAccessInst(std::string klass, std::string name, AllocaInst* this_ptr)
{
   assert(classAttributes.find(klass) != classAttributes.end());
   int                 index = std::get<0>(classAttributes[klass][name]);
   std::vector<Value*> ptr_indices;
   ConstantInt*        const_int32_0 = ConstantInt::get(getModule()->getContext(), APInt(32, 0));
   ConstantInt*        const_int32   = ConstantInt::get(getModule()->getContext(), APInt(32, index));
   ptr_indices.push_back(const_int32_0);
   ptr_indices.push_back(const_int32);
   auto structTy = classTypeMap[klass];
   auto valType = std::get<1>(classAttributes[klass][name]);
   if (this_ptr->getAllocatedType()->isPointerTy()) {
      // since the alloc is a ptr to ptr
      auto klassPtr = new LoadInst(this_ptr->getType(), this_ptr, "load.this", currentBlock());
      auto instr = GetElementPtrInst::Create(structTy, klassPtr, ptr_indices, "", currentBlock());
      return instr;
   }
   Instruction* ptr = GetElementPtrInst::Create(structTy, this_ptr, ptr_indices, "", currentBlock());
   return ptr;
}

std::string CodeGenContext::getType(std::string varName)
{
   if (varName == "self") {
      return klassName;
   }
   for (auto& cb : codeBlocks) {
      auto iter = cb->getTypeMap().find(varName);
      if (iter != std::end(cb->getTypeMap())) {
         return cb->getTypeMap()[varName];
      }
   }
   return std::string("");
}

Type* CodeGenContext::typeOf(const Identifier& type) { return typeOf(type.getName()); }

Type* CodeGenContext::typeOf(const std::string& name)
{
   if( llvmTypeMap.count(name) != 0 ) {
      return llvmTypeMap[name];
   }

   llvm::Type* ty = StructType::getTypeByName(getModule()->getContext(), "class." + name);
   if (ty != nullptr) {
      return ty;
   }
   return voidType;
}

std::string CodeGenContext::typeNameOf(llvm::Type* type)
{
   switch( type->getTypeID() ) {
      case llvm::Type::TypeID::DoubleTyID:
         return "double";
      case llvm::Type::TypeID::IntegerTyID:
         if( type == llvm::Type::getInt1Ty(getGlobalContext()) )
            return "boolean";
         return "int";
      case llvm::Type::TypeID::VoidTyID:
         return "void";
      default:
         return "void";
   }
}

void CodeGenContext::addKlassInitCode(std::string name, Assignment* assign) { classInitCode[name].insert(assign); }

KlassInitCodeAssign& CodeGenContext::getKlassInitCode(std::string name) { return classInitCode[name]; }

std::string CodeGenContext::findClassNameByType(llvm::Type* ty)
{
   auto found = std::find_if(std::begin(classTypeMap), std::end(classTypeMap), [&](auto kv) { return kv.second == ty; });
   if (found != std::end(classTypeMap)) {
      return found->first;
   }
   return "";
}

llvm::Type* CodeGenContext::getGenericIntegerType()
{
#if defined(UseInt64)
   return Type::getInt64Ty(getGlobalContext());
#else
   return Type::getInt32Ty(getGlobalContext());
#endif
}

bool CodeGenContext::preProcessing(Block& root)
{
   VisitorSyntaxCheck visitor;
   root.Accept(visitor);
   return !visitor.hasErrors();
}

FunctionDeclaration* CodeGenContext::getTemplateFunction(const std::string& name)
{
   if( templatedFunctionDeclarations.count(name) > 0 ) {
      return templatedFunctionDeclarations[name];
   }
   return nullptr;
}
   llvm::Type* CodeGenContext::getType(Identifier const& ident)
   {
      if( ident.getStructName() == "self" ) {
         if(classAttributes[klassName].count(ident.getName()) != 0u) {
            return classAttributes[klassName][ident.getName()].second;
         }
      }
      return voidType;
   }

   llvm::Type* CodeGenContext::getKlassMemberType(std::string const& klassName, std::string const& memberName)
   {
      return classAttributes[klassName][memberName].second;
   }

}
