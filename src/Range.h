#pragma once
#include "AstNode.h"

namespace liquid
{

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

} // namespace liquid
