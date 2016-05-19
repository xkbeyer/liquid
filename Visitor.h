#ifndef Visitor_h__
#define Visitor_h__

namespace liquid {
   class Expression;
   class Statement;
   class Return;
   class FunctionDeclaration;
   class Conditional;
   class Integer;
   class Double;
   class String;
   class Boolean;
   class Identifier;
   class UnaryOperator;
   class BinaryOp;
   class CompOperator;
   class Block;
   class ExpressionStatement;
   class Assignment;
   class MethodCall;
   class VariableDeclaration;
   class VariableDeclarationDeduce;
   class Conditional;
   class WhileLoop;
   class ClassDeclaration;
   class List;
   class ListAccess;
   class ListAddElement;
   class Range;

class Visitor
{
public:
   virtual void VisitExpression( Expression* expr ) = 0;
   virtual void VisitInteger( Integer* expr ) = 0;
   virtual void VisitDouble( Double* expr ) = 0;
   virtual void VisitString( String* expr ) = 0;
   virtual void VisitBoolean( Boolean* expr ) = 0;
   virtual void VisitIdentifier( Identifier* expr ) = 0;
   virtual void VisitUnaryOperator( UnaryOperator* expr ) = 0;
   virtual void VisitBinaryOp( BinaryOp* expr ) = 0;
   virtual void VisitCompOperator( CompOperator* expr ) = 0;
   virtual void VisitBlock( Block* expr ) = 0;
   virtual void VisitStatement( Statement* stmt ) = 0;
   virtual void VisitReturnStatement( Return* retstmt ) = 0;
   virtual void VisitFunctionDeclaration( FunctionDeclaration* fndecl ) = 0;
   virtual void VisitExpressionStatement( ExpressionStatement* expr ) = 0;
   virtual void VisitAssigment( Assignment* expr ) = 0;
   virtual void VisitMethodCall( MethodCall* expr ) = 0;
   virtual void VisitVariablenDeclaration( VariableDeclaration* expr ) = 0;
   virtual void VisitVariablenDeclarationDeduce( VariableDeclarationDeduce* expr ) = 0;
   virtual void VisitConditional( Conditional* expr ) = 0;
   virtual void VisitWhileLoop( WhileLoop* expr ) = 0;
   virtual void VisitClassDeclaration( ClassDeclaration* expr ) = 0 ;
   virtual void VisitList(List* expr) = 0;
   virtual void VisitListAccess(ListAccess* expr) = 0;
   virtual void VisitListAddElement(ListAddElement* expr) = 0;
   virtual void VisitRange(Range* expr) = 0;
};

}
#endif // Visitor_h__
