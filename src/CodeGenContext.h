#pragma once

#include "config.h"

#include <vector>
#include <map>
#include <list>
#include <set>

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
public:
   CodeGenBlock(llvm::BasicBlock* bb) { bblock = bb; }
   ~CodeGenBlock() {}
   void              setCodeBlock(llvm::BasicBlock* bb) { bblock = bb; }
   llvm::BasicBlock* currentBlock() { return bblock; }
   ValueNames&       getValueNames() { return locals; }
   VariableTypeMap&  getTypeMap() { return types; }

private:
   llvm::BasicBlock* bblock{nullptr};
   ValueNames        locals;
   VariableTypeMap   types;
};

///! The context of the current compiling process.
class CodeGenContext
{
public:
   llvm::Value* varStruct{nullptr}; ///< Hold the alloc of the structure variable (class object). TODO move it to a better place.
   bool verbose {false};            ///< Verbose output
   bool debug {false};              ///< Dump the generated LLVM byte code.

   CodeGenContext(std::ostream & outs);
   ~CodeGenContext() { llvm::llvm_shutdown(); }

   llvm::Module*      getModule() const { return module; }

   llvm::LLVMContext& getGlobalContext() { return llvmContext; }

   /*! Enters a new scope (block)
    * \param[in] bb         The basic block containing of the new scope. If nullptr then a new block is created.
    * \param[in] scopeType  Type of scope @see ScopeType
    */
   void               newScope(llvm::BasicBlock* bb = nullptr, ScopeType scopeType = ScopeType::CodeBlock);

   /*! Leaves current scope, the previous one will be active now.*/
   void endScope();

   /*! Return the current scope type. */
   ScopeType getScopeType() { return currentScopeType; }

   /*! Set the LLVM block where to put the next instructions. */
   void setInsertPoint(llvm::BasicBlock* bblock) { setCurrentBlock(bblock); }

   /*! Get the current insert block. */
   llvm::BasicBlock* getInsertPoint() { return currentBlock(); }

   /*! Compile the AST into a module */
   bool generateCode(class Block & root);

   /*! Executes the AST by running the main function */
   llvm::GenericValue runCode();

   /*! Prints how the code will be generated */
   void printCodeGeneration(class Block & root, std::ostream & outs);

   /*! Get the local variable names (of current scope). */
   ValueNames& locals() { return codeBlocks.front()->getValueNames(); }

   /*! Set type of a variable in current scope.
    * \param[in] varTypeName Variable type name.
    * \param[in] varName Variable name.
    */
   void setVarType(std::string varTypeName, std::string varName) { codeBlocks.front()->getTypeMap()[varName] = varTypeName; }

   /*! Get the type of a variable name.
    * \param[in] varName the name of the variable to be looked up.
    * \return The name of the type.
    */
   std::string getType(std::string varName);

   /*! Searches a variable name in all known locals of the current code block.
    * If not found it looks in the current class block, too.
    * \param[in] varName variable name
    * \return The alloca instruction
    */
   llvm::AllocaInst* findVariable(std::string varName);

   /*! Deletes a variable name in all known locals of the current code block.
    *
    * \param[in] varName variable name
    */
   void deleteVariable(std::string varName);

   /*! Renames a variable in all known locals of the current code block.
    *
    * \param[in] oldVarName The variable o rename
    * \param[in] newVarName The new name fo the variable
    */
   void renameVariable(std::string oldVarName, std::string newVarName);

   /*! Returns the current code block. */
   llvm::BasicBlock* currentBlock() { return codeBlocks.front()->currentBlock(); }

   /*! Runs the optimizer over all function */
   void optimize();

   /*! Creates a new class block scope. */
   void newKlass(std::string name);

   /*! Closes a class block scope */
   void endKlass();

   /*! Adds a class member variable name and its index to the list of the
    * class attributes map. Since the access to the corresponding llvm structure
    * goes via the index into this structure.
    * \param[in] name Variable name of the current class.
    * \param[in] index into the llvm structure of this class.
    *
    */
   void klassAddVariableAccess(std::string name, int index);

   /*! Returns the load/getelementptr instruction to access a class member.
    * \param[in] klass The class name
    * \param[in] name Class member name
    * \param[in] this_ptr The alloca instruction to be used for the getelementptr instruction.
    * \return Instruction pointer.
    * \remark Is used to access the elements via the alloca where the ptr to the ref of the class object is stored.
    *         Means that the ref is stored in a local (stack) variable.
    */
   llvm::Instruction* getKlassVarAccessInst(std::string klass, std::string name, llvm::AllocaInst * this_ptr);

   /*! Returns the currently processed class definition. */
   std::string getKlassName() { return klassName; }

   /*! Returns an LLVM type based on the identifier */
   llvm::Type* typeOf(const class Identifier& type);

   /*! Returns an LLVM type based on the name */
   llvm::Type* typeOf(const std::string& name);

   /*! Returns type name based on LLVM Type */
   std::string typeNameOf(llvm::Type* type);

   /*! Store the init code of the class to be used while creation. */
   void addKlassInitCode(std::string name, Assignment * assign);

   /*! Returns the class init code */
   KlassInitCodeAssign& getKlassInitCode(std::string name);

   /*! Check if a name is a class */
   bool isClass(const std::string& name) { return classAttributes.find(name) != std::end(classAttributes); }

   /*! Associate a LLVM type with a class name. */
   void addClassType(const std::string& name, llvm::Type* ty) { classTypeMap.emplace(name, ty); }

   /*! Look up a class name of a given LLVM type. */
   std::string findClassNameByType(llvm::Type* ty);

   /*! Returns the LLVM integer type used by liquid. */
   llvm::Type* getGenericIntegerType();

   /*! Preprocess the AST generated by the parser.
    * \param[in] root  The root block of the AST.
    * \return true on success.
    */
   bool preProcessing(class Block & root);

   /*! Increments the error counter. */
   void addError() { ++errors; }

   /*! Add a function which has var arguments and has to be generated later at call time.
    * \param[in] name Function name.
    * \param[in] funcDecl Function declaration node.
    */
   void addTemplateFunction(const std::string& name, FunctionDeclaration* funcDecl) { templatedFunctionDeclarations[name] = funcDecl; }

   /*! Returns the function declaration of the 'template' function.
    * \param[in] name Function name.
    * \note Does return nullptr if name is not found.
    */
   FunctionDeclaration* getTemplateFunction(const std::string& name);

   /*! Start/End of generating a template function (args with type var).
    * \param[in] setFlag true: begin of generation. false: end of generation.
    */
   void setGenerateTemplatedFunction(bool setFlag) { generateTemplatedFunction = setFlag; }

   /*! Returns true if a template function is to be generated otherwise false. */
   bool codeGenTheTemplatedFunction() const { return generateTemplatedFunction; }

 private:
   void setCurrentBlock(llvm::BasicBlock * block) { codeBlocks.front()->setCodeBlock(block); }

   /*! Setup up the built in functions and types:
    * - printvalue
    * - printdouble
    * - sin
    * - displayln
    * - display
    */
   void setupBuiltIns();

   std::list<CodeGenBlock*> codeBlocks;             ///< List of all code blocks
   CodeGenBlock*            self{nullptr};          ///< The current code block.
   std::string              klassName;              ///< The current class definition block
   llvm::Function*          mainFunction{nullptr};  ///< main function
   llvm::Module*            module{nullptr};        ///< llvm module ...
   llvm::LLVMContext        llvmContext;            ///< and context
   KlassAttributes          classAttributes;        ///< List of attributes a class
   KlassInitCode            classInitCode;          ///< The init code (statements) for each class
   std::map<std::string, llvm::Type*> classTypeMap; ///< Maps a class name to its LLVM struct type
   int                      errors{0};              ///< Count of errors while code gen.
   ScopeType                currentScopeType{ScopeType::CodeBlock};
   std::ostream&            outs;
   struct buildin_info_t {
      llvm::Function* f{nullptr};
      void*           addr{nullptr};
   };
   std::vector<buildin_info_t> builtins;
   llvm::Type* intType {nullptr};
   llvm::Type* doubleType {nullptr};
   llvm::Type* stringType {nullptr};
   llvm::Type* boolType {nullptr};
   llvm::Type* voidType {nullptr};
   llvm::Type* varType {nullptr};
   std::map<std::string, llvm::Type*> llvmTypeMap;
   std::map<std::string, FunctionDeclaration*> templatedFunctionDeclarations;
   bool generateTemplatedFunction {false};
};

}
