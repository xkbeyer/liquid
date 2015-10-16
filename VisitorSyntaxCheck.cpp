#include "VisitorSyntaxCheck.h"
#include "AstNode.h"
#include "FunctionDeclaration.h"
#include "ClassDeclaration.h"

namespace liquid {

void VisitorSyntaxCheck::VisitExpression( Expression* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}  

void VisitorSyntaxCheck::VisitStatement( Statement* stmt )
{
   std::cout << "Visiting "; stmt->toString(); 

}

void VisitorSyntaxCheck::VisitReturnStatement( Return* retstmt )
{
   std::cout << "Visiting "; retstmt->toString(); 
   ++returnStatements;
}

void VisitorSyntaxCheck::VisitFunctionDeclaration( FunctionDeclaration* fndecl )
{
   std::cout << "Visiting "; fndecl->toString(); 
   returnStatements = 0;
   auto body = fndecl->getBody();
   for( auto stmt : body->statements ) {
      stmt->Accept( *this );
   }

   if( returnStatements > 1 ) {
      Node::printError( fndecl->getlocation(), " Too many return statement in function '" + fndecl->getId()->getName() + "()' for return type deduction.");
      syntaxErrors++;
   }
}

void VisitorSyntaxCheck::VisitCompareStatement( Conditional* cmp )
{
   std::cout << "Visiting "; cmp->toString(); 
   if( cmp->getThen() ) {
      cmp->getThen()->Accept( *this );
   }
   if( cmp->getElse() ) {
      cmp->getElse()->Accept( *this );
   }
}

void VisitorSyntaxCheck::VisitInteger( Integer* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitDouble( Double* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitString( String* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitBoolean( Boolean* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitIdentifier( Identifier* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitUnaryOperator( UnaryOperator* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitBinaryOp( BinaryOp* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitCompOperator( CompOperator* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitBlock( Block* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitExpressionStatement( ExpressionStatement* expr )
{
   std::cout << "Visiting expression statement"; expr->toString(); std::cout << "\n" ;
}

void VisitorSyntaxCheck::VisitAssigment( Assignment* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitMethodCall( MethodCall* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitVariablenDeclaration( VariableDeclaration* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitVariablenDeclarationDeduce( VariableDeclarationDeduce* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitConditional( Conditional* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitWhileLoop( WhileLoop* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

void VisitorSyntaxCheck::VisitClassDeclaration( ClassDeclaration* expr )
{
   std::cout << "Visiting "; expr->toString(); 
}

}
