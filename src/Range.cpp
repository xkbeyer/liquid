#include "AstNode.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;

namespace liquid {

llvm::Value* Range::codeGen(CodeGenContext& context) 
{
   // var l = [lhs]
   // var n = lhs
   // while n < end 
   //    n += 1
   //    l << n
   // return l
   
   YYLTYPE loc = { 0,0,0,0 };
   Block tmp_code;
   auto rc = context.findVariable("tmp_l");
   if( rc ) {
      context.deleteVariable("tmp_l");
   }
   rc = context.findVariable("tmp_n");
   if( rc ) {
      context.deleteVariable("tmp_n");
   }
   // var l = [lhs]
   ExpressionList exprs;
   exprs.push_back(begin);
   auto l = new Array( &exprs, loc);
   auto vardecl = new VariableDeclarationDeduce(new Identifier("tmp_l", loc), l, loc);
   tmp_code.statements.push_back(vardecl);
   // var n = lhs
   auto vardecl_n = new VariableDeclarationDeduce(new Identifier("tmp_n", loc), this->begin, loc);
   tmp_code.statements.push_back(vardecl_n);
   // while loop
   auto while_block = new Block();
   auto assgn = new Assignment(new Identifier("tmp_n", loc), new BinaryOp(new Identifier("tmp_n", loc), TPLUS, new Integer(1), loc), loc);
   auto l_plus_n = new ArrayAddElement(new Identifier("tmp_l", loc), new Identifier("tmp_n", loc), loc);
   while_block->statements.push_back(assgn);
   while_block->statements.push_back(l_plus_n);
   auto cond = new CompOperator(new Identifier("tmp_n", loc), TCLE, this->end);
   auto wl = new WhileLoop(cond, while_block);
   tmp_code.statements.push_back(wl);
   tmp_code.codeGen(context);
   #if !defined(LLVM_NO_DUMP)
   context.getModule()->dump();
   #endif
   rc = context.findVariable("tmp_l");
   return rc;
}


}
