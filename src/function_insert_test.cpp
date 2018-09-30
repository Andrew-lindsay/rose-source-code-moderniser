#include "rose.h"

using namespace SageBuilder;
using namespace SageInterface;

class SimpleInstrumentation : public SgSimpleProcessing{
  
public:
  void visit(SgNode* astNode);
};


void SimpleInstrumentation::visit(SgNode* astNode){
  
  SgGlobal* globalscope = isSgGlobal(astNode);
  
  if( globalscope != NULL){
    
    // create parameter lis with parameter (function arg list)
    SgName name = "var_name";
    //SgReferenceType* ref_type = buildReferenceType(buildIntType());
    SgInitializedName* var1_init_name = buildInitializedName(name, buildIntType() );
    
    // create object add args later through function (append args)
    SgFunctionParameterList* parameterList = buildFunctionParameterList();
    appendArg(parameterList, var1_init_name);

    // Create function declaration with function body
    SgName function_name = "My_function";
    SgFunctionDeclaration* func
      = buildDefiningFunctionDeclaration(function_name, buildVoidType(), parameterList, globalscope);
    SgBasicBlock* func_body = func->get_definition()->get_body();

    // create statement for functin body
    SgVarRefExp* var_ref = buildVarRefExp(name, func_body); // has scope in function body
    SgPlusPlusOp* plus_plus_exp = buildPlusPlusOp(var_ref);
    SgExprStatement* new_statement = buildExprStatement(plus_plus_exp);

    // insert statement in function body
    prependStatement(new_statement, func_body);
    prependStatement(func, globalscope);
  }
}

int main(int argc, char *argv[]){
  ROSE_INITIALIZE;
  
  SgProject* project = frontend(argc, argv);
  ROSE_ASSERT(project != NULL);
  
  SimpleInstrumentation treeTraversal;
  treeTraversal.traverseInputFiles(project, preorder);

  AstTests::runAllTests(project);

  return backend(project);
 
}
