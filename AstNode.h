#ifndef _INC_ASTNODE_H
#define _INC_ASTNODE_H


#include "main.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>


#if defined(_MSC_VER)
#pragma warning( push , 0 )
#endif

#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

#include "Visitor.h"

typedef struct YYLTYPE {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
} YYLTYPE;

namespace liquid {

class CodeGenContext;

using StatementList = std::vector<class Statement*>;
using ExpressionList = std::vector<class Expression*>;
using VariableList = std::vector<class VariableDeclaration*>;

enum class NodeType
{
    expression,
    variable,
    klass,
    function,
    integer,
    decimal,
    string,
    boolean,
    identifier
};

class Node {
public:
    virtual ~Node() {}
    virtual llvm::Value* codeGen(CodeGenContext& context) = 0 ;
    virtual NodeType getType() = 0 ;
    virtual void toString() {std::cout << "node\n" ;}
    virtual void Accept( Visitor& v ) = 0;
    static void printError(YYLTYPE location, std::string msg) {
        std::cerr
        << "line "
        << location.first_line << " column "
        << location.first_column << "-"
        << location.last_column << ":"
        << msg << std::endl;
    }
};

class Expression : public Node {
public:
    virtual ~Expression() {}
    virtual void toString() {std::cout << "Expression\n" ;}
    virtual void Accept( Visitor& v ) { v.VisitExpression( this ); }
};

class Statement : public Expression {
public:
    virtual ~Statement() {}
    NodeType getType() {return NodeType::expression;}
    virtual void toString() {std::cout << "Statement\n" ;}
    virtual void Accept( Visitor& v ) { v.VisitStatement( this ); }
};

class Integer : public Expression {
    long long value;
public:
    Integer(long long value) : value(value) {}
    virtual ~Integer() {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::integer;}
    virtual void toString() {std::cout << "integer: " << value << std::endl;}
    virtual void Accept( Visitor& v ) {v.VisitInteger(this);}
};

class Double : public Expression {
    double value;
public:
    Double(double value) : value(value) {}
    virtual ~Double() {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::decimal;}
    virtual void toString() {std::cout << "double: " << value << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitDouble( this ); }
};

class String : public Expression {
    std::string value;
public:
    String(const std::string& value) : value(value) {}
    virtual ~String() { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::string;}
    virtual void toString() {std::cout << "string: " << value << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitString( this ); }
};

class Boolean : public Expression {
    std::string value;
    int boolVal;
public:
    Boolean(const std::string& value) : value(value)
    {
        boolVal = value == "true" ;
    }
    virtual ~Boolean() {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::boolean;}
    virtual void toString() {std::cout << "boolean: " << value << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitBoolean( this ); }
};

class Identifier : public Expression {
    std::string name;
    std::string structName;
    YYLTYPE location;
public:
    Identifier(const std::string& name, YYLTYPE loc) : name(name), location(loc) {}
    Identifier(const std::string& structName, const std::string& name, YYLTYPE loc) : name(name), structName(structName), location(loc) {}
    Identifier( const Identifier& id ) : name( id.name ), structName( id.structName ), location( id.location ) {}
    virtual ~Identifier() {}
    std::string getName() const {return name;} 
    std::string getStructName() const {return structName;}
    YYLTYPE getLocation() const { return location; }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::identifier;}
    virtual void toString() {std::cout << "identifier reference: " << structName << "::" << name << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitIdentifier( this ); }
};


class UnaryOperator : public Expression {
    int op;
    Expression* rhs;
public:
    UnaryOperator(int op, Expression* rhs) : op(op), rhs(rhs) {}
    virtual ~UnaryOperator()
    {
        delete rhs;
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::expression;}
    virtual void toString() {std::cout << "unary operation " << op << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitUnaryOperator( this ); }
};

// Binary operators are +,-,*,/
class BinaryOp : public Expression {
    int op;
    Expression* lhs;
    Expression* rhs;
    YYLTYPE location;
public:
    BinaryOp(Expression* lhs, int op, Expression* rhs, YYLTYPE loc) : op(op), lhs(lhs), rhs(rhs), location(loc) {}
    virtual ~BinaryOp()
    {
        delete lhs;
        delete rhs;
    }
    int getOperator() const {return op;}
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::expression;}
    virtual void toString() {std::cout << "binary operation " << op << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitBinaryOp( this ); }
};

class CompOperator : public Expression {
    int op;
    Expression* lhs;
    Expression* rhs;
public:
    CompOperator(Expression* lhs, int op, Expression* rhs) : op(op), lhs(lhs), rhs(rhs) {}
    virtual ~CompOperator()
    {
        delete lhs;
        delete rhs;
    }
    int getOperator() const {return op;}
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::expression;}
    virtual void toString() {std::cout << "compare operation " << op << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitCompOperator( this ); }
};

class Block : public Expression {
public:
   StatementList statements;
   
   Block() {}
    virtual ~Block()
    {
        for(auto i : statements) {
            delete i;
        }
        statements.clear();
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::expression;}
    virtual void toString() 
    {
        std::cout << "block " << std::endl;
        for( auto stmt : statements) {
           stmt->toString();
        }
        std::cout << "End block " << std::endl;
    }
    void Accept( Visitor& v )
    {
       for( auto stmt : statements ) {
          stmt->Accept( v );
       }
    }
};

class ExpressionStatement : public Statement
{
    Expression* expression;
public:
    ExpressionStatement(Expression* expression) : expression(expression) {}
    virtual ~ExpressionStatement() { delete expression; }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::expression;}
    virtual void toString() {/*std::cout << "expression statement " << std::endl;*/}
    virtual void Accept( Visitor& v ) { v.VisitExpressionStatement( this ); }
};

class Assignment : public Statement
{
    Identifier* lhs;
    Expression* rhs;
    YYLTYPE location;
public:
    Assignment( Identifier* lhs, Expression* rhs, YYLTYPE loc ) : lhs( lhs ), rhs( rhs ), location( loc ) {}
    virtual ~Assignment() { delete lhs; delete rhs; }
    virtual llvm::Value* codeGen( CodeGenContext& context );
    NodeType getType() { return NodeType::expression; }
    virtual void toString() { std::cout << "assignment for " << lhs->getStructName() << "::" << lhs->getName() << std::endl; }
    virtual void Accept( Visitor& v ) { v.VisitAssigment( this ); }
};


class MethodCall : public Statement
{
    Identifier* id;
    ExpressionList* arguments;
    YYLTYPE location;
public:
    MethodCall( Identifier* id, ExpressionList* arguments, YYLTYPE loc ) : id( id ), arguments( arguments ), location( loc ) {}
    virtual ~MethodCall()
    {
        for( ExpressionList::iterator i = arguments->begin(); i != arguments->end(); ++i ) {
            delete *i;
        }
        arguments->clear();
        delete arguments;
        delete id;
    }
    virtual llvm::Value* codeGen( CodeGenContext& context );
    NodeType getType() { return NodeType::expression; }
    virtual void toString() { std::cout << "method call: " << id->getStructName() << "." << id->getName() << std::endl; }
    virtual void Accept( Visitor& v ) { v.VisitMethodCall( this ); }
private:
    std::string getTypeNameOfFirstArg( CodeGenContext& context );
};


class VariableDeclaration : public Statement
{
    Identifier* type;
    Identifier* id;
    Expression *assignmentExpr;
    YYLTYPE location;
public:
    VariableDeclaration(Identifier* type, Identifier* id, YYLTYPE loc) : type(type), id(id), assignmentExpr(nullptr), location(loc) {}
    VariableDeclaration(Identifier* type, Identifier* id, Expression *assignmentExpr, YYLTYPE loc) : type(type), id(id), assignmentExpr(assignmentExpr), location(loc) {}
    VariableDeclaration( int ident, Identifier* id, Expression *assignmentExpr, YYLTYPE loc ) : type( nullptr ), id( id ), assignmentExpr( assignmentExpr ), location( loc ) {}
    virtual ~VariableDeclaration()
    {
        delete assignmentExpr;
        delete id;
        delete type;
    }
    const Identifier& getIdentifierOfVariable() const { return *id; }
    const Identifier& getIdentifierOfVariablenType() const { return *type; }
    std::string getVariablenTypeName() const {return type->getName();}
    std::string getVariablenName() const {return id->getName();}
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::variable;}
    bool hasAssignmentExpr() {return assignmentExpr != nullptr;}
    Expression* getAssignment() { return assignmentExpr; }
    YYLTYPE& getLocation() {return location;} 
    virtual void toString() {std::cout << "variable declaration for " << id->getName() << " of type " << (type ? type->getName() : "TBD") << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitVariablenDeclaration( this ); }
};

class VariableDeclarationDeduce : public Statement
{
    Identifier* id;
    Expression *assignmentExpr;
    YYLTYPE location;
public:
    VariableDeclarationDeduce( Identifier* id, Expression *assignmentExpr, YYLTYPE loc ) : id( id ), assignmentExpr( assignmentExpr ), location( loc ) {}
    virtual ~VariableDeclarationDeduce() {/*Note: id and assignmentExpr are deleted by Assignment()*/ }
    const Identifier& getIdentifierOfVariable() const { return *id; }
    std::string getVariablenName() const { return id->getName(); }
    virtual llvm::Value* codeGen( CodeGenContext& context );
    NodeType getType() { return NodeType::variable; }
    bool hasAssignmentExpr() { return assignmentExpr != nullptr; }
    Expression* getAssignment() { return assignmentExpr; }
    YYLTYPE& getLocation() { return location; }
    virtual void toString() { std::cout << "variable declaration for " << id->getName() << " of unknown type " << std::endl; }
    virtual void Accept( Visitor& v ) { v.VisitVariablenDeclarationDeduce( this ); }
};

class Conditional : public Statement
{
    CompOperator* cmpOp;
    Expression *thenExpr;
    Expression *elseExpr;
public:
    Conditional(Expression* op, Expression *thenExpr,Expression *elseExpr = nullptr)
    : cmpOp((CompOperator*)op) , thenExpr(thenExpr), elseExpr(elseExpr) { }
    virtual ~Conditional()
    {
        delete cmpOp; delete thenExpr; delete elseExpr;
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::expression;}
    virtual void toString() {std::cout << "conditional " << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitCompareStatement( this ); }

    virtual Expression* getThen() { return thenExpr; }
    virtual Expression* getElse() { return elseExpr; }
};

class WhileLoop : public Statement
{
    Expression* condition;
    Block *loopBlock;
    Block *elseBlock;
public:
    WhileLoop(Expression* expr, Block *loopBlock,Block *elseBlock = nullptr)
    : condition(expr) , loopBlock(loopBlock), elseBlock(elseBlock) {}
    virtual ~WhileLoop()
    {
        delete condition; delete loopBlock; delete elseBlock;
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::expression;}
    virtual void toString() {std::cout << "while loop " << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitWhileLoop( this ); }
};


class Return : public Statement
{
    Expression* retExpr;
    YYLTYPE location;
public:
    Return( YYLTYPE loc, Expression* expr = nullptr ) : retExpr( expr ), location( loc ) {}
    virtual ~Return() {delete retExpr;}
    virtual llvm::Value* codeGen(CodeGenContext& context);
    NodeType getType() {return NodeType::expression;}
    virtual void toString() {std::cout << "return statement " << std::endl;}
    virtual void Accept( Visitor& v ) { v.VisitReturnStatement( this ); }
    YYLTYPE& getLocation() { return location; }
};

}

#endif
