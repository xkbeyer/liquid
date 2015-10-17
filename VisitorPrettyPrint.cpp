#include "VisitorPrettyPrint.h"
#include "AstNode.h"
#include "FunctionDeclaration.h"
#include "ClassDeclaration.h"

namespace liquid {

static inline std::string indent_spaces(int indent)
{
   return std::string(indent * 2, ' ');
}

void VisitorPrettyPrint::VisitExpression( Expression* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}  

void VisitorPrettyPrint::VisitStatement( Statement* stmt )
{
   std::cout << indent_spaces(indent) << "Create " << stmt->toString() << std::endl;

}

void VisitorPrettyPrint::VisitReturnStatement( Return* retstmt )
{
   std::cout << indent_spaces(indent) << "Create " << retstmt->toString() << std::endl;
}

void VisitorPrettyPrint::VisitFunctionDeclaration( FunctionDeclaration* fndecl )
{
   std::cout << indent_spaces(indent) << "Create " << fndecl->toString() << std::endl;
   ++indent;
   auto body = fndecl->getBody();
   for( auto stmt : body->statements ) {
      stmt->Accept( *this );
   }
   --indent;
}

void VisitorPrettyPrint::VisitConditional( Conditional* cmp )
{
   std::cout << indent_spaces(indent) << "Create " << cmp->toString() << std::endl;
   if( cmp->getThen() ) {
      cmp->getThen()->Accept( *this );
   }
   if( cmp->getElse() ) {
      cmp->getElse()->Accept( *this );
   }
}

void VisitorPrettyPrint::VisitInteger( Integer* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}

void VisitorPrettyPrint::VisitDouble( Double* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}

void VisitorPrettyPrint::VisitString( String* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}

void VisitorPrettyPrint::VisitBoolean( Boolean* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}

void VisitorPrettyPrint::VisitIdentifier( Identifier* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}

void VisitorPrettyPrint::VisitUnaryOperator( UnaryOperator* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
   ++indent;
   expr->getRHS()->Accept(*this);
   --indent;
}

void VisitorPrettyPrint::VisitBinaryOp( BinaryOp* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
   ++indent;
   expr->getLHS()->Accept(*this);
   expr->getRHS()->Accept(*this);
   --indent;
}

void VisitorPrettyPrint::VisitCompOperator( CompOperator* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}

void VisitorPrettyPrint::VisitBlock( Block* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
   ++indent;
   for(auto stmt : expr->statements) {
      stmt->Accept( *this );
   }
   --indent;
   std::cout << indent_spaces(indent) << "End " << expr->toString() << std::endl;
}

void VisitorPrettyPrint::VisitExpressionStatement( ExpressionStatement* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
   ++indent;
   expr->getExpression()->Accept(*this);
   --indent;
}

void VisitorPrettyPrint::VisitAssigment( Assignment* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}

void VisitorPrettyPrint::VisitMethodCall( MethodCall* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
   ++indent;
   auto args = expr->getArguments();
   if(args->size()) {
      std::cout << indent_spaces(indent) << "Arguments are:\n";
      for(auto arg : *args) {
         arg->Accept(*this);
      }
   }
   --indent;
}

void VisitorPrettyPrint::VisitVariablenDeclaration( VariableDeclaration* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
   if(expr->hasAssignmentExpr()) {
      ++indent;
      expr->getAssignment()->Accept(*this);
      --indent;
   }
}

void VisitorPrettyPrint::VisitVariablenDeclarationDeduce( VariableDeclarationDeduce* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << " with type deduction" << std::endl;
   if(expr->hasAssignmentExpr()) {
      ++indent;
      expr->getAssignment()->Accept(*this);
      --indent;
   }
}

void VisitorPrettyPrint::VisitWhileLoop( WhileLoop* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}

void VisitorPrettyPrint::VisitClassDeclaration( ClassDeclaration* expr )
{
   std::cout << indent_spaces(indent) << "Create " << expr->toString() << std::endl;
}

}
