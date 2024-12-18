#pragma once

#include "AstNode.h"

namespace liquid
{
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

   YYLTYPE         getLocation() const { return location; }
   ExpressionList* getExpressions() const { return exprList; }

private:
   ExpressionList* exprList {nullptr};
   YYLTYPE         location {};
};

/*! Represents an array element access */
class ArrayAccess : public Expression
{
public:
   ArrayAccess(Identifier* id, long long index, YYLTYPE loc) : variable(id), index(index), location(loc) {}
   ArrayAccess(Expression* id, long long index, YYLTYPE loc) : index(index), location(loc), other(id) {}
   virtual ~ArrayAccess() = default;

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::list; }
   std::string  toString() override { return "array-element-access"; }
   void         Accept(Visitor& v) override { v.VisitArrayAccess(this); }

   YYLTYPE getLocation() const { return location; }

private:
   Identifier* variable{nullptr};
   long long   index{0LL};
   YYLTYPE     location{};
   Expression* other{nullptr};
   friend class VisitorSyntaxCheck;
   friend class VisitorPrettyPrint;
};

/*! Represents adding an element to the array. */
class ArrayAddElement : public Statement
{
public:
   ArrayAddElement(Identifier* ident, Expression* expr, YYLTYPE loc) : expr(expr), ident(ident), location(loc) {}
   virtual ~ArrayAddElement() = default;

   llvm::Value* codeGen(CodeGenContext& context) override;
   NodeType     getType() override { return NodeType::list; }
   std::string  toString() override { return "list add element"; }
   void         Accept(Visitor& v) override { v.VisitArrayAddElement(this); }

   YYLTYPE     getLocation() const { return location; }
   Expression* getExpression() const { return expr; }

private:
   Expression* expr{nullptr};
   Identifier* ident{nullptr};
   YYLTYPE     location;
   friend class VisitorSyntaxCheck;
   friend class VisitorPrettyPrint;
};

} // namespace liquid
