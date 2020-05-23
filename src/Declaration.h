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
   VariableDeclaration(Identifier* id, YYLTYPE loc)
      : type(new Identifier("var", loc)), id(id), assignmentExpr(nullptr), location(loc)
   {
   }
   VariableDeclaration(Identifier* id, Expression* assignmentExpr, YYLTYPE loc)
      : type(new Identifier("var", loc)), id(id), assignmentExpr(assignmentExpr), location(loc)
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
   virtual const Identifier& getIdentifierOfVariablenType() const { return *type; }
   virtual std::string       getVariablenTypeName() const { return type->getName(); }
   std::string       getVariablenName() const { return id->getName(); }
   bool              hasAssignmentExpr() const { return assignmentExpr != nullptr; }
   Expression*       getAssignment() const { return assignmentExpr; }
   YYLTYPE&          getLocation() { return location; }

protected:
   Identifier* type{nullptr};
   Identifier* id{nullptr};
   Expression* assignmentExpr{nullptr};
   YYLTYPE     location;
};

} // namespace liquid
