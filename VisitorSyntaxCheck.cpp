#include "VisitorSyntaxCheck.h"
#include "AstNode.h"
#include "FunctionDeclaration.h"
#include "ClassDeclaration.h"

namespace liquid {

void VisitorSyntaxCheck::VisitExpression( Expression* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}  

void VisitorSyntaxCheck::VisitStatement( Statement* stmt )
{
   std::cout << "Visiting " << stmt->toString() << std::endl;

}

void VisitorSyntaxCheck::VisitReturnStatement( Return* retstmt )
{
   std::cout << "Visiting " << retstmt->toString() << std::endl;
   ReturnStatementLocations.push_back( retstmt->getLocation() );
   ++returnStatements;
}

void VisitorSyntaxCheck::VisitFunctionDeclaration( FunctionDeclaration* fndecl )
{
   std::cout << "Visiting " << fndecl->toString() << std::endl;
   returnStatements = 0;
   ReturnStatementLocations.clear();

   auto body = fndecl->getBody();
   for( auto stmt : body->statements ) {
      stmt->Accept( *this );
   }

   if( returnStatements > 1 ) {
      Node::printError( fndecl->getlocation(), " Too many return statement in function '" + fndecl->getId()->getName() + "()' for return type deduction.\nThe possible statements are:");
      for( auto loc : ReturnStatementLocations ) {
         Node::printError( loc, " return ..." );
      }
      syntaxErrors++;
   }
}

void VisitorSyntaxCheck::VisitCompareStatement( Conditional* cmp )
{
   std::cout << "Visiting " << cmp->toString() << std::endl;
   if( cmp->getThen() ) {
      cmp->getThen()->Accept( *this );
   }
   if( cmp->getElse() ) {
      cmp->getElse()->Accept( *this );
   }
}

void VisitorSyntaxCheck::VisitInteger( Integer* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitDouble( Double* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitString( String* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitBoolean( Boolean* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitIdentifier( Identifier* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitUnaryOperator( UnaryOperator* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitBinaryOp( BinaryOp* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitCompOperator( CompOperator* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitBlock( Block* expr )
{
   std::cout << "Visiting " << expr->toString() << std::endl; 
   for(auto stmt : expr->statements) {
      stmt->Accept(*this);
   }

}

void VisitorSyntaxCheck::VisitExpressionStatement( ExpressionStatement* expr )
{
   std::cout << "Visiting " << expr->toString() << std::endl;
}

void VisitorSyntaxCheck::VisitAssigment( Assignment* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitMethodCall( MethodCall* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitVariablenDeclaration( VariableDeclaration* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitVariablenDeclarationDeduce( VariableDeclarationDeduce* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitConditional( Conditional* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitWhileLoop( WhileLoop* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

void VisitorSyntaxCheck::VisitClassDeclaration( ClassDeclaration* expr )
{
   std::cout << "Visiting "<< expr->toString() << std::endl; 
}

}
