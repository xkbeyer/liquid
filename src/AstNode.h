#ifndef INC_ASTNODE_H
#define INC_ASTNODE_H

#include "main.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

#include "Visitor.h"

struct YYLTYPE
{
   int         first_line{0};
   int         first_column{0};
   int         last_line{0};
   int         last_column{0};
   std::string file_name;
};

namespace liquid {

class CodeGenContext;

using StatementList = std::vector<class Statement*>;
using ExpressionList = std::vector<class Expression*>;
using VariableList = std::vector<class VariableDeclaration*>;

/*! Type of the AST node */
enum class NodeType
{
    expression,
    variable,
    klass,
    function,
    integer,
    decimal,
    string,
    boolean,
    identifier,
    list,
    range
};

/*! Base class of all nodes */
class Node 
{
public:
   virtual ~Node() = default;

   /*! Code generation for this node
    * \param[in] context  The context of the code gen run.
    * \return Generated code as LLVM value. 
    */
   virtual llvm::Value* codeGen(CodeGenContext& context) = 0;

   /*! Returns the type of the node. */
   virtual NodeType getType() = 0;

   /*! Returns the textual representation. */
   virtual std::string toString() { return "node"; }

   /*! Accept a visitor. */
   virtual void Accept(Visitor& v) = 0;

   /*! Prints the found parsing error.
    * \param[in] location file, line, col information.
    * \param[in] msg      The message to print.
    */
   static void printError(YYLTYPE location, std::string msg)
   {
      std::cerr
         << location.file_name
         << ": line "
         << location.first_line << " column "
         << location.first_column << "-"
         << location.last_column << ":"
         << msg << std::endl;
   }

   /*! Prints an error message where no source location is available.
    * \param[in] msg      The message to print.
    */
   static void printError(std::string msg)
   {
      std::cerr << msg << std::endl;
   }
};

/*! Represents an expression. */
class Expression : public Node 
{
public:
   virtual ~Expression() = default;
   std::string toString() override { return "Expression"; }
   void Accept(Visitor& v) override { v.VisitExpression(this); }
};

/*! Represents a statement. */
class Statement : public Expression
{
public:
   virtual ~Statement() = default;
   NodeType    getType() override { return NodeType::expression; }
   std::string toString() override { return "Statement"; }
   void        Accept(Visitor& v) override { v.VisitStatement(this); }
};

/*! Represents an integer. */
class Integer : public Expression
{
public:
   explicit Integer(long long value) : value(value) {}
   virtual ~Integer() = default;
   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::integer; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "integer: " << value;
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitInteger(this); }

private:
   long long value{0};
};

/*! Represents a double. */
class Double : public Expression
{
public:
   explicit Double(double value) : value(value) {}
   virtual ~Double() = default;
   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::decimal; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "double: " << value;
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitDouble(this); }

private:
   double value{0.};
};

/*! Represents a string. */
class String : public Expression
{
public:
   explicit String(const std::string& value) : value(value) {}
   virtual ~String() = default;
   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::string; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "string: '" << value << "'";
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitString(this); }

private:
   std::string value;
};

/*! Represents a boolean. */
class Boolean : public Expression
{
public:
   explicit Boolean(int const value) : boolVal(value) {}
   virtual ~Boolean() = default;
   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::boolean; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "boolean: " << (boolVal == 1 ? "true" : "false");
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitBoolean(this); }

private:
   int         boolVal{0};
};

/*! Represents an identifier. */
class Identifier : public Expression
{
public:
   Identifier(const std::string& name, YYLTYPE loc) : name(name), location(loc) {}
   Identifier(const std::string& structName, const std::string& name, YYLTYPE loc) : name(name), structName(structName), location(loc) {}
   Identifier(const Identifier& id) : name(id.name), structName(id.structName), location(id.location) {}
   virtual ~Identifier() = default;

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::identifier; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "identifier reference: " << structName << "::" << name;
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitIdentifier(this); }

   std::string getName() const { return name; }
   std::string getStructName() const { return structName; }
   YYLTYPE     getLocation() const { return location; }

private:
   std::string name;
   std::string structName;
   YYLTYPE     location;
};

/*! Represents a block */
class Block : public Expression
{
public:
   StatementList statements;
   
   Block() = default;
   virtual ~Block()
   {
      for(auto i : statements) {
         delete i;
      }
      statements.clear();
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override { return "block "; }
   void Accept(Visitor& v) override { v.VisitBlock(this); }
};

/*! Represents a expression statement. */
class ExpressionStatement : public Statement
{
public:
   explicit ExpressionStatement(Expression* expression) : expression(expression) {}
   virtual ~ExpressionStatement() { delete expression; }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   Expression*  getExpression() { return expression; }
   std::string  toString() override { return "expression statement "; }
   void         Accept(Visitor& v) override { v.VisitExpressionStatement(this); }

private:
   Expression*  expression{nullptr};
};

}

#endif
