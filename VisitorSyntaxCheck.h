#ifndef VisitorSyntaxCheck_h__
#define VisitorSyntaxCheck_h__
#include <iostream>
#include "Visitor.h"
namespace liquid {

class VisitorSyntaxCheck : public Visitor
{
   int returnStatements;
   int syntaxErrors;
public:
   VisitorSyntaxCheck() : returnStatements(0), syntaxErrors(0) {}
   virtual ~VisitorSyntaxCheck() {}
   void VisitExpression(Expression* expr);
   void VisitInteger( Integer* expr );
   void VisitDouble( Double* expr );
   void VisitString( String* expr );
   void VisitBoolean( Boolean* expr );
   void VisitIdentifier( Identifier* expr );
   void VisitUnaryOperator( UnaryOperator* expr );
   void VisitBinaryOp( BinaryOp* expr );
   void VisitCompOperator( CompOperator* expr );
   void VisitBlock( Block* expr );
   void VisitStatement( Statement* stmt );
   void VisitReturnStatement( Return* retstmt );
   void VisitCompareStatement( Conditional* cmp ) ;
   void VisitFunctionDeclaration( FunctionDeclaration* fndecl );
   void VisitExpressionStatement(ExpressionStatement* expr);
   void VisitAssigment(Assignment* expr);
   void VisitMethodCall(MethodCall* expr);
   void VisitVariablenDeclaration(VariableDeclaration* expr);
   void VisitVariablenDeclarationDeduce(VariableDeclarationDeduce* expr);
   void VisitConditional(Conditional* expr);
   void VisitWhileLoop(WhileLoop* expr);
   void VisitClassDeclaration(ClassDeclaration* expr);

   bool hasErrors() { return syntaxErrors != 0 ; }
};

}
#endif // VisitorSyntaxCheck_h__


