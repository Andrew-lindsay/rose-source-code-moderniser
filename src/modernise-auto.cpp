#include "rose.h"
#include <list>
#include <vector>
#include <string>
// printf already imported somewhere probably rose.h

using namespace SageBuilder;
using namespace SageInterface;

class SimpleVarDecTraversal : public AstSimpleProcessing{

  // helper functions
  void setbaseType(SgType* type,SgType* newType){

    // haver to cast to the correct type to get base
    // type with internal types
    /* 
       Array types - X
       Modifier types - / 
       Pointer type - / 
       Reference type - /
       SgRvalueReferenceType - ?
       Complex type ? 
       SgTypeOfType
     */    
    SgType* prevFinalType  = type;
    
    while(type->containsInternalTypes()){
      prevFinalType = type;
      type = ((SgPointerType*) type )->get_base_type();
    }

    ((SgPointerType*) prevFinalType)->set_base_type(newType);
    
  }

 public:
  /* can only apply auto if there is rhs to vardec and that is an AssignInitializer and need to
     preserve type info such as point (what about many initilaisations on one line) */
  void visit(SgNode* node){
    SgInitializedName* assignNode = isSgInitializedName(node);
    
    if(assignNode != NULL && assignNode->get_file_info()->isCompilerGenerated() == false
       && isSgVariableDeclaration(assignNode->get_parent()) &&  isSgAssignInitializer(assignNode->get_initptr()) ){

      // WARNING NOW CASTING
      SgAssignInitializer* initializer =  (SgAssignInitializer* ) assignNode->get_initializer();
      
      printf("Sage class name: %s, str: %s, Compiler Gen: %d, File name: %d\n"
	     ,assignNode->sage_class_name()
	     , assignNode->unparseToString().c_str()
	     ,assignNode->get_file_info()->isCompilerGenerated()==true
	     ,assignNode->get_file_info()->get_file_id() );
      
      printf("Using auto: %s\n",  assignNode->get_using_auto_keyword() ?  "True" : "False");
      
      printf("Type: %s, TypeName: %s, BaseType: %s\n",
	     assignNode->get_typeptr()->unparseToString().c_str()
	     ,assignNode->get_typeptr()->class_name().c_str()
	     ,assignNode->get_typeptr()->findBaseType()->unparseToString().c_str());

      // HANDLE IMPLICT CASTING
      // get_operand_i ()
      // http://rosecompiler.org/ROSE_HTML_Reference/classSgAssignInitializer.html
      
      SgName name_new = "auto";
      SgType* type = assignNode->get_typeptr();
      
      // constVolitail modifier
      // set base type to nothing
      if(type->containsInternalTypes()){

	//old way 
	//type->reset_base_type(new SgTemplateType(name_new));
	
	//set Basetype
	setbaseType(type, new SgTemplateType(name_new));
	
	assignNode->set_typeptr(type->stripType(SgType::STRIP_POINTER_TYPE));
	//printf("NoInternal types\n");
      }else{
	assignNode->set_type(new SgTemplateType(name_new));
	//printf("NoInternal types\n");
      }
      
      // add auto keyword to type  
      //assignNode->set_using_auto_keyword(true);
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
  printf("Printing args:\n");
  for(int i = 0; i < argv_plus.size(); i++){
    printf("\t%s\n",argv_plus[i].c_str());
  }

  // create project from arg list
  SgProject* project = frontend(argv_plus);
  ROSE_ASSERT(project !=NULL);

  // get file id
  printf("First FILE ID: %d\n", project[0][0]->get_file_info()->get_file_id());
  
  // debug to check for files that it searches for can check id
  printf("File Names:\n");
  for(auto & files: project->get_fileList() ){
    printf("\t%s\n",files->get_file_info()->get_filename());
  }

  printf("Traversing AST tree:\n");
  // traverse the ast tree
  SimpleVarDecTraversal traversal;
  traversal.traverseInputFiles(project, preorder);
    
  return backend(project);
}
