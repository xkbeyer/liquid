#ifndef VisitorSyntaxCheck_h__
#define VisitorSyntaxCheck_h__
#include <iostream>
#include <vector>
#include <unordered_set>

#include "Visitor.h"
struct YYLTYPE;

namespace liquid {

class VisitorSyntaxCheck : public Visitor
{
   int syntaxErrors;
   std::vector<YYLTYPE> ReturnStatementLocations;
   std::unordered_set<std::string> TypeNames{ "int","double","string","boolean","var" };
public:
   VisitorSyntaxCheck();
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
   void VisitFunctionDeclaration( FunctionDeclaration* fndecl );
   void VisitExpressionStatement(ExpressionStatement* expr);
   void VisitAssigment(Assignment* expr);
   void VisitMethodCall(MethodCall* expr);
   void VisitVariablenDeclaration(VariableDeclaration* expr);
   void VisitVariablenDeclarationDeduce(VariableDeclarationDeduce* expr);
   void VisitConditional(Conditional* expr);
   void VisitWhileLoop(WhileLoop* expr);
   void VisitClassDeclaration(ClassDeclaration* expr);
   void VisitList(List* expr);

   bool hasErrors() { return syntaxErrors != 0 ; }
};

}
#endif // VisitorSyntaxCheck_h__


