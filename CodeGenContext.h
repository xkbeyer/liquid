#pragma once

#include <stack>
#include <map>
#include <list>
#include <set>

#include "config.h"

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/Support/ManagedStatic.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include "AstNode.h"

namespace liquid
{
///< Used to keep track the context of a code block.
enum class ScopeType {
   Class,
   FunctionDeclaration,
   CodeBlock,
};

///< Maps a variable name to its alloca instruction.
using ValueNames = std::map<std::string, llvm::AllocaInst*>;
///< Maps a variable name of a class definition to its position in the llvm structure type.
using KlassValueNames = std::map<std::string, int>;
///< Maps a class name to its attributes (member variables).
using KlassAttributes = std::map<std::string, KlassValueNames>;
///< Maps a variable name to its type name.
using VariableTypeMap = std::map<std::string, std::string>;
///< A set of assignments to hold the init code of class members
using KlassInitCodeAssign = std::set<Assignment*>;
///< Maps the init code to the class name.
using KlassInitCode = std::map<std::string, KlassInitCodeAssign>;

///< context of a scoped code block of expressions.
class CodeGenBlock
{
   llvm::BasicBlock* bblock;
   ValueNames        locals;
   VariableTypeMap   types;

 public:
   CodeGenBlock(llvm::BasicBlock* bb) { bblock = bb; }
   ~CodeGenBlock() {}
   void              setCodeBlock(llvm::BasicBlock* bb) { bblock = bb; }
   llvm::BasicBlock* currentBlock() { return bblock; }
   ValueNames&       getValueNames() { return locals; }
   VariableTypeMap&  getTypeMap() { return types; }
};

///! The context of the current compiling process.
class CodeGenContext
{
   std::list<CodeGenBlock*> codeBlocks;             ///< List of all code blocks
   CodeGenBlock*            self = nullptr;         ///< The current code block.
   std::string              klassName;              ///< The current class definition block
   llvm::Function*          mainFunction = nullptr; ///< main function
   llvm::Module*            module       = nullptr; ///< llvm module ...
   llvm::LLVMContext        llvmContext;            ///< and context
   KlassAttributes          classAttributes;        ///< List of attributes for the current class being processed
   KlassInitCode            classInitCode;
   int                      errors           = 0; ///< Count of errors while code gen.
   ScopeType                currentScopeType = ScopeType::CodeBlock;
   std::ostream&            outs;
   void                     setCurrentBlock(llvm::BasicBlock* block) { codeBlocks.front()->setCodeBlock(block); }
   struct buildin_info_t {
      llvm::Function* f = nullptr;
      void*           addr = nullptr;
   };
   std::vector<buildin_info_t> builtins;
   void                         setupBuiltIns();

 public:
   llvm::Value* varStruct = nullptr; ///< Hold the alloc of the structure variable (class object). TODO move it to a better place.
   bool         verbose   = false;
   bool         debug     = false;

   CodeGenContext(std::ostream& outs);
   ~CodeGenContext() { llvm::llvm_shutdown(); }

   llvm::Module*      getModule() { return module; }
   llvm::LLVMContext& getGlobalContext() { return llvmContext; }
   void               newScope(llvm::BasicBlock* bb = nullptr, ScopeType scopeType = ScopeType::CodeBlock);
   void               endScope();
   ScopeType          getScopeType() { return currentScopeType; }
   void               setInsertPoint(llvm::BasicBlock* bblock) { setCurrentBlock(bblock); }
   llvm::BasicBlock*  getInsertPoint() { return currentBlock(); }
   bool               generateCode(class Block& root);
   llvm::GenericValue runCode();
   void               printCodeGeneration(class Block& root, std::ostream& outs);
   ValueNames&        locals() { return codeBlocks.front()->getValueNames(); }
   void               setVarType(std::string varType, std::string varName) { codeBlocks.front()->getTypeMap()[varName] = varType; }
   std::string        getType(std::string varName);
   llvm::AllocaInst*  findVariable(std::string varName);
   void               deleteVariable(std::string varName);
   void               renameVariable(std::string oldVarName, std::string newVarName);

   llvm::BasicBlock*    currentBlock() { return codeBlocks.front()->currentBlock(); }
   void                 optimize();
   void                 newKlass(std::string name);
   void                 endKlass();
   void                 klassAddVariableAccess(std::string name, int index);
   llvm::Instruction*   getKlassVarAccessInst(std::string klass, std::string name, llvm::AllocaInst* this_ptr);
   std::string          getKlassName() { return klassName; }
   llvm::Type*          typeOf(const class Identifier& type);
   llvm::Type*          typeOf(const std::string name);
   void                 addKlassInitCode(std::string name, Assignment* assign);
   KlassInitCodeAssign& getKlassInitCode(std::string name);
   llvm::Type*          getGenericIntegerType();
   bool                 preProcessing(class Block& root);
   void                 addError() { ++errors; }
};
}
