#ifndef _INC_ASTNODE_H
#define _INC_ASTNODE_H

#include "main.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <stack>

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
   virtual std::string toString() { return "node\n"; }

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
   explicit Boolean(const std::string& value) : value(value), boolVal(value == "true") {}
   virtual ~Boolean() = default;
   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::boolean; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "boolean: " << value;
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitBoolean(this); }

private:
   std::string value;
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


/*! Represents a unary operator. */
class UnaryOperator : public Expression
{
public:
   explicit UnaryOperator(int op, Expression* rhs) : op(op), rhs(rhs) {}
   virtual ~UnaryOperator() { delete rhs; }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override;
   void         Accept(Visitor& v) override { v.VisitUnaryOperator(this); }

   Expression*  getRHS() { return rhs; }

private:
   int         op{0};
   Expression* rhs;
};

/*! Represents a binary operators  + - * / */
class BinaryOp : public Expression 
{
public:
   BinaryOp(Expression* lhs, int op, Expression* rhs, YYLTYPE loc) : op(op), lhs(lhs), rhs(rhs), location(loc) {}
   virtual ~BinaryOp()
   {
      delete lhs;
      delete rhs;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override;
   void         Accept(Visitor& v) override { v.VisitBinaryOp(this); }

   Expression* getLHS() { return lhs; }
   Expression* getRHS() { return rhs; }
   int         getOperator() const { return op; }

private:
   llvm::Value* codeGenAddList(llvm::Value* rhsValue, llvm::Value* lhsValue, CodeGenContext& context);

   int          op{0};
   Expression*  lhs{nullptr};
   Expression*  rhs{nullptr};
   YYLTYPE      location;
};

/*! Represents a compare operator  == != > < >= <= */
class CompOperator : public Expression
{
public:
   explicit CompOperator(Expression* lhs, int op, Expression* rhs) : op(op), lhs(lhs), rhs(rhs) {}
   virtual ~CompOperator()
   {
      delete lhs;
      delete rhs;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override;
   void         Accept(Visitor& v) override { v.VisitCompOperator(this); }

   int          getOperator() const { return op; }
   Expression* getLHS() { return lhs; }
   Expression* getRHS() { return rhs; }

private:
   int          op{0};
   Expression*  lhs{nullptr};
   Expression*  rhs{nullptr};
};

/*! Represents an array */
class Array : public Expression
{
public:
   Array(YYLTYPE loc) : location(loc) {}
   Array(ExpressionList* exprs, YYLTYPE loc) : exprList(exprs), location(loc) {}
   virtual ~Array() = default;

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::list; }
   std::string  toString() override { return "array"; }
   void         Accept(Visitor& v) override { v.VisitArray(this); }

   YYLTYPE getLocation() const { return location; }
   ExpressionList* getExpressions() const { return exprList; }

private:
   ExpressionList* exprList = nullptr;
   YYLTYPE         location;
};

/*! Represents an array element access */
class ArrayAccess : public Expression
{
public:
   ArrayAccess(Identifier* id, long long index, YYLTYPE loc) : variable(id), index(index), location(loc) {}
   ArrayAccess(Expression* id, long long index, YYLTYPE loc) : other(id), index(index), location(loc) {}
   virtual ~ArrayAccess() = default;

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::list; }
   std::string  toString() override { return "array-element-access"; }
   void         Accept(Visitor& v) override { v.VisitArrayAccess(this); }

   YYLTYPE getLocation() const { return location; }

private:
   Identifier* variable{nullptr};
   long long   index{0};
   YYLTYPE     location;
   Expression* other{nullptr};
   friend class VisitorSyntaxCheck;
   friend class VisitorPrettyPrint;
};

/*! Represents a range */
class Range : public Expression
{
public:
   explicit Range(Expression* begin, Expression* end, YYLTYPE loc) : begin(begin), end(end), location(loc) {}
   virtual ~Range() = default;
   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::range; }
   std::string  toString() override { return "range"; }
   void         Accept(Visitor& v) override { v.VisitRange(this); }

   YYLTYPE getLocation() const { return location; }

private:
   Expression* begin{nullptr};
   Expression* end{nullptr};
   YYLTYPE     location;
   friend class VisitorSyntaxCheck;
   friend class VisitorPrettyPrint;
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
   std::string  toString() override
   {
      std::stringstream s;
      s << "block ";
      return s.str();
   }
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

/*! Represents adding an element to the array. */
class ArrayAddElement : public Statement
{
public:
   ArrayAddElement(Identifier* ident, Expression* expr, YYLTYPE loc) : ident(ident), expr(expr), location(loc) {}
   virtual ~ArrayAddElement() = default;

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::list; }
   std::string  toString() override { return "list add element"; }
   void         Accept(Visitor& v) override { v.VisitArrayAddElement(this); }

   YYLTYPE getLocation() const { return location; }
   Expression* getExpression() const { return expr; }

private:
   Expression* expr{nullptr};
   Identifier* ident{nullptr};
   YYLTYPE     location;
   friend class VisitorSyntaxCheck;
   friend class VisitorPrettyPrint;
};

/*! Represents an assigment. */
class Assignment : public Statement
{
public:
   explicit Assignment(Identifier* lhs, Expression* rhs, YYLTYPE loc) : lhs(lhs), rhs(rhs), location(loc) {}
   virtual ~Assignment() { delete lhs; delete rhs; }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "assignment for " << lhs->getStructName() << "::" << lhs->getName();
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitAssigment(this); }

   Expression* getExpression() { return rhs; }

private:
   Identifier* lhs{nullptr};
   Expression* rhs{nullptr};
   YYLTYPE     location;
};


/*! Represents a method call. */
class MethodCall : public Statement
{
public:
   explicit MethodCall(Identifier* id, ExpressionList* arguments, YYLTYPE loc) : id(id), arguments(arguments), location(loc) {}
   virtual ~MethodCall()
   {
      for(auto i : *arguments) {
         delete i;
      }
      arguments->clear();
      delete arguments;
      delete id;
   }

   llvm::Value*    codeGen(CodeGenContext& context) override;
   NodeType        getType() override { return NodeType::expression; }
   std::string     toString() override
   {
      std::stringstream s;
      s << "method call: " << id->getStructName() << "." << id->getName();
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitMethodCall(this); }

   ExpressionList* getArguments() { return arguments; }

private:
   std::string getTypeNameOfFirstArg(CodeGenContext& context);

   Identifier*     id{nullptr};
   ExpressionList* arguments{nullptr};
   YYLTYPE         location;
};


/*! Represents a variable declaration. */
class VariableDeclaration : public Statement
{
public:
   VariableDeclaration(Identifier* type, Identifier* id, YYLTYPE loc) : type(type), id(id), assignmentExpr(nullptr), location(loc) {}
   VariableDeclaration(Identifier* type, Identifier* id, Expression *assignmentExpr, YYLTYPE loc) : type(type), id(id), assignmentExpr(assignmentExpr), location(loc) {}
   virtual ~VariableDeclaration()
   {
      delete assignmentExpr;
      delete id;
      delete type;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::variable; }
   virtual std::string toString() override
   {
      std::stringstream s;
      s << "variable declaration for " << id->getName() << " of type " << (type ? type->getName() : "TBD");
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitVariablenDeclaration(this); }

   const Identifier& getIdentifierOfVariable() const { return *id; }
   const Identifier& getIdentifierOfVariablenType() const { return *type; }
   std::string getVariablenTypeName() const { return type->getName(); }
   std::string getVariablenName() const { return id->getName(); }
   bool hasAssignmentExpr() { return assignmentExpr != nullptr; }
   Expression* getAssignment() { return assignmentExpr; }
   YYLTYPE& getLocation() { return location; }

private:
   Identifier* type{nullptr};
   Identifier* id{nullptr};
   Expression* assignmentExpr{nullptr};
   YYLTYPE     location;
};

class VariableDeclarationDeduce : public Statement
{
public:
   explicit VariableDeclarationDeduce(Identifier* id, Expression *assignmentExpr, YYLTYPE loc) : id(id), assignmentExpr(assignmentExpr), location(loc) {}
   virtual ~VariableDeclarationDeduce() {/*Note: id and assignmentExpr are deleted by Assignment()*/ }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::variable; }
   std::string  toString() override
   {
      std::stringstream s;
      s << "variable declaration for " << id->getName() << " of unknown type ";
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitVariablenDeclarationDeduce(this); }

   const Identifier& getIdentifierOfVariable() const { return *id; }
   std::string getVariablenName() const { return id->getName(); }
   bool hasAssignmentExpr() { return assignmentExpr != nullptr; }
   Expression* getAssignment() { return assignmentExpr; }
   YYLTYPE& getLocation() { return location; }

private:
   Identifier* id{nullptr};
   Expression* assignmentExpr{nullptr};
   YYLTYPE     location;
};

/*! Represents a conditional statement. */
class Conditional : public Statement
{
public:
   explicit Conditional(Expression* op, Expression *thenExpr, Expression *elseExpr = nullptr) : cmpOp((CompOperator*) op), thenExpr(thenExpr), elseExpr(elseExpr) {}
   virtual ~Conditional()
   {
      delete cmpOp; delete thenExpr; delete elseExpr;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override { return "conditional "; }
   void         Accept(Visitor& v) override { v.VisitConditional(this); }

   CompOperator* getCompOperator() { return cmpOp; }
   Expression*   getThen() { return thenExpr; }
   Expression*   getElse() { return elseExpr; }

private:
   CompOperator* cmpOp{nullptr};
   Expression*   thenExpr{nullptr};
   Expression*   elseExpr{nullptr};
};

/*! Represents a while loop. */
class WhileLoop : public Statement
{
public:
   explicit WhileLoop(Expression* expr, Block *loopBlock,Block *elseBlock = nullptr)
   : condition(expr) , loopBlock(loopBlock), elseBlock(elseBlock) {}
   virtual ~WhileLoop()
   {
      delete condition; delete loopBlock; delete elseBlock;
   }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override { return "while loop "; }
   void         Accept(Visitor& v) override { v.VisitWhileLoop(this); }

   Expression* getCondition() { return condition; }
   Block*      getLoopBlock() { return loopBlock; }
   Block*      getElseBlock() { return elseBlock; }

private:
   Expression* condition{nullptr};
   Block*      loopBlock{nullptr};
   Block*      elseBlock{nullptr};
};

/*! Represents a return statement. */
class Return : public Statement
{
public:
   Return(YYLTYPE loc, Expression* expr = nullptr) : retExpr(expr), location(loc) {}
   virtual ~Return() { delete retExpr; }

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::expression; }
   std::string  toString() override { return "return statement "; }
   void         Accept(Visitor& v) override { v.VisitReturnStatement(this); }

   Expression* getRetExpression() { return retExpr; }
   YYLTYPE&    getLocation() { return location; }

private:
   Expression* retExpr{nullptr};
   YYLTYPE     location;
};

}

#endif
