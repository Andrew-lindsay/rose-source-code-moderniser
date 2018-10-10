#include "rose.h"
#include <list>
#include <vector>
#include <string>
// printf already imported somewhere probably rose.h


// class SimpleVarDecTraversal : public AstSimpleProcessing{
  
//  public:
//   void visit(SgNode* node){
//     SgStatement* stmt = isSgStatement(node);
//     if( stmt != NULL && stmt->get_file_info()->isCompilerGenerated() == false){
//       printf("Sage class name: %s, str: %s, Compiler Gen: %d, File name: %d\n"
// 	     ,stmt->sage_class_name(), stmt->unparseToString().c_str()
// 	     ,stmt->get_file_info()->isCompilerGenerated()==true, stmt->get_file_info()->get_file_id() );
      
//       MiddleLevelRewrite::insert(node,"// hello world", MidLevelCollectionTypedefs::AfterCurrentPosition);
//     }
//   }
// };

using namespace SageBuilder;
using namespace SageInterface;

class SimpleVarDecTraversal : public AstSimpleProcessing{
  
 public:
  void visit(SgNode* node){
    if(node->get_file_info()->isCompilerGenerated() == false && isSgRangeBasedForStatement(node)){
      printf("Sage class name: %s, str: %s, Compiler Gen: %d, File name: %d\n"
	     ,node->sage_class_name(), node->unparseToString().c_str()
	     ,node->get_file_info()->isCompilerGenerated()==true, node->get_file_info()->get_file_id() );      
    }
  }
};


// main Ast pass function
int main(int argc, char *argv[]){
  
  ROSE_INITIALIZE;

  // added additional args to stop rose backend from actually compiling the code
  std::vector<std::string> argv_plus(argv, argv + argc );
  auto it = argv_plus.begin();
  argv_plus.insert(++it,"-rose:skipfinalCompileStep");
  it = argv_plus.begin() +2;
  argv_plus.insert(it,"-rose:Cxx11_only");
  
  //debug print args
  for(int i = 0; i < argv_plus.size(); i++){
    printf("%s\n",argv_plus[i].c_str());
  }

  // create project from arg list
  SgProject* project = frontend(argv_plus);
  ROSE_ASSERT(project !=NULL);

  // debug to check for files that it searches for can check id
  for(auto & files: project->get_fileList() ){
    printf("%s\n",files->get_file_info()->get_filename());
  }

  // traverse the ast tree
  SimpleVarDecTraversal traversal;
  traversal.traverseInputFiles(project, preorder);
  
  // get file id
  printf("First FILE ID: %d\n", project[0][0]->get_file_info()->get_file_id());
  
  //AstTests::runAllTests(project);
  // querySubTree( SgNode*, VariantT,
  //AstQueryNamespace::QueryDepth = AstQueryNamespace::AllNodes)
  
  //Rose_STL_Container<SgNode*> queryResult;
  //queryResult = NodeQuery::querySubTree(project , V_SgGlobal);
  // queryResult = NodeQuery::querySubTree(project, V_SgVariableDeclaration);
  // printf("\nNumber of Nodes: %d\n", queryResult.size());
  // for(auto & var_dec_node : queryResult  ){
  //   // check that var dec is not compiler generated or from an include statement like #include <string>
  //   if(var_dec_node->get_file_info()->isCompilerGenerated() == false && var_dec_node->get_file_info()->get_file_id() == 0){
  //     // do something with Nodes
  //    }
  // }
  
  return backend(project);
}
