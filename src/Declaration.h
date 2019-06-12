#pragma once
#include "AstNode.h"

namespace liquid
{

/*! Represents a variable declaration. */
class VariableDeclaration : public Statement
{
public:
   VariableDeclaration(Identifier* type, Identifier* id, YYLTYPE loc) : type(type), id(id), assignmentExpr(nullptr), location(loc) {}
   VariableDeclaration(Identifier* type, Identifier* id, Expression* assignmentExpr, YYLTYPE loc)
      : type(type), id(id), assignmentExpr(assignmentExpr), location(loc)
   {
   }
   virtual ~VariableDeclaration()
   {
      delete assignmentExpr;
      delete id;
      delete type;
   }

   llvm::Value*        codeGen(CodeGenContext& context) override;
   NodeType            getType() override { return NodeType::variable; }
   virtual std::string toString() override
   {
      std::stringstream s;
      s << "variable declaration for " << id->getName() << " of type " << (type ? type->getName() : "TBD");
      return s.str();
   }
   void Accept(Visitor& v) override { v.VisitVariablenDeclaration(this); }

   const Identifier& getIdentifierOfVariable() const { return *id; }
   const Identifier& getIdentifierOfVariablenType() const { return *type; }
   std::string       getVariablenTypeName() const { return type->getName(); }
   std::string       getVariablenName() const { return id->getName(); }
   bool              hasAssignmentExpr() { return assignmentExpr != nullptr; }
   Expression*       getAssignment() { return assignmentExpr; }
   YYLTYPE&          getLocation() { return location; }

private:
   Identifier* type{nullptr};
   Identifier* id{nullptr};
   Expression* assignmentExpr{nullptr};
   YYLTYPE     location;
};

class VariableDeclarationDeduce : public Statement
{
public:
   explicit VariableDeclarationDeduce(Identifier* id, Expression* assignmentExpr, YYLTYPE loc) : id(id), assignmentExpr(assignmentExpr), location(loc) {}
   virtual ~VariableDeclarationDeduce()
   { /*Note: id and assignmentExpr are deleted by Assignment()*/
   }

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
   std::string       getVariablenName() const { return id->getName(); }
   bool              hasAssignmentExpr() { return assignmentExpr != nullptr; }
   Expression*       getAssignment() { return assignmentExpr; }
   YYLTYPE&          getLocation() { return location; }

private:
   Identifier* id{nullptr};
   Expression* assignmentExpr{nullptr};
   YYLTYPE     location;
};

} // namespace liquid
