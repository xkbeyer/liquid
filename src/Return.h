#pragma once
#include "AstNode.h"

namespace liquid
{

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
