#include "rose.h" // printf already imported somewhere probably rose.h
#include <list>
#include <vector>
#include <string>

using namespace SageBuilder;
using namespace SageInterface;

class SimpleVarDecTraversal : public AstSimpleProcessing{

    // --------------------------- helper functions ----------------------------------------------
    // all the side effects
    void setbaseType(SgType* type,SgType* newType){
		
		//   Array types-X, Modifier types-/, Pointer type-/, Reference type-/, SgRvalueReferenceType-?
	    //   Complex type ?, SgTypeOfType ?, SgClassType ????, SgTypedefType ????
	
		// bad casting moment should handle certain types more delicately think this is operating on classtypes
		SgType* prevFinalType  = type;
		
		while(type->containsInternalTypes() && !isSgTypedefType(type) ){
			prevFinalType = type;
			type =  (SgType*) ((SgPointerType*) type )->get_base_type();
			
			if(type->containsInternalTypes() && !isSgTypedefType(type)){ // skip copying final node 
				SgTreeCopy copyDeep; // need copy due to changing of functions parameters linked to the types we are changing 
				type = (SgType*) type->copy(copyDeep); // cant preform deep copy on stl class ? or any class ? 
			}
			
			((SgPointerType*) prevFinalType)->set_base_type(type);
		}
		
		((SgPointerType*) prevFinalType)->set_base_type(newType);
    }
    // -------------------------------------------------------------------------------------------
 
public:
    /* can only apply auto if there is rhs to vardec and that is an AssignInitializer and need to
       preserve type info such as point (what about many initilaisations on one line) */
    void visit(SgNode* node){
		
		SgInitializedName* assignNode = isSgInitializedName(node);
		
		// Getting correct type of assignInitializer
		if(assignNode != NULL && assignNode->get_file_info()->isCompilerGenerated() == false
		   && isSgVariableDeclaration(assignNode->get_parent())
		   && isSgClassDefinition(assignNode->get_parent()->get_parent()) == NULL // cant auto non-static fields in classes
		   && isSgAssignInitializer(assignNode->get_initptr())){ 

			//DEBUG
			//printf("%s", isSgClassDefinition(assignNode->get_parent()->get_parent()) ? "true\n" : "false\n");
			
			SgAssignInitializer* initializer =  (SgAssignInitializer* ) assignNode->get_initializer();
			SgType* rhsType = initializer->get_operand()->get_type();
			
			// DEBUG
			printf("Sage class name: %s, str: %s, Compiler Gen: %d, File name: %d\n"
				   ,assignNode->sage_class_name()
				   ,assignNode->unparseToString().c_str()
				   ,assignNode->get_file_info()->isCompilerGenerated()==true
				   ,assignNode->get_file_info()->get_file_id() );

			// DEBUG
			printf("Using auto: %s\n",  assignNode->get_using_auto_keyword() ?  "True" : "False");

			// DEBUG
			printf("Type: %s, TypeClass: %s, RhsTypeName: %s BaseType: %s\n"
				   ,assignNode->get_typeptr()->unparseToString().c_str()
				   ,assignNode->get_typeptr()->class_name().c_str()
				   ,rhsType->class_name().c_str()
				   ,assignNode->get_typeptr()->findBaseType()->unparseToString().c_str());
	    
			SgName name_new = "auto";
		 	SgTreeCopy copyDeep;
			SgType* type = assignNode->get_typeptr();

			// check for an implict cast
			SgCastExp* castExpr = isSgCastExp(initializer->get_operand());

			// need to handle function pointner/ dont try and auto function pointers 

			// check to see if a cast has been created by the compiler meaning that the RHS is an implict cast 
			if( castExpr != NULL  &&  castExpr->isCompilerGenerated()){
				printf("Implict cast: %s\n\n", assignNode->unparseToString().c_str());
				return;
			}
	    
			if(assignNode->get_using_auto_keyword()){
				printf("Using Auto don't alter\n\n");
				return;
			}

			// TESTING IF CHECK MAKES ANY CHANGE
			//if(assignNode->get_file_info()->get_file_id() != 0){
			//	return;
			//}
	    
			if(type->containsInternalTypes() && isSgTypedefType(type) == NULL){ // 
				// only need to copy when we know we have nested types 
				SgType* typeCopy = (SgType*) assignNode->get_typeptr()->copy(copyDeep);
				printf("RECURSIVE TYPES\n\n");		
				
                /* old way 
				   type->reset_base_type(new SgTemplateType(name_new)); */
		
				//set Basetype
				setbaseType(typeCopy, new SgTemplateType(name_new));
				assignNode->set_typeptr( typeCopy->stripType(SgType::STRIP_POINTER_TYPE) );
			}
			else{
				printf("BASE TYPES\n\n");
				assignNode->set_typeptr(new SgTemplateType(name_new));
			}
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
    it = argv_plus.begin() + 2;
    argv_plus.insert(it,"-rose:Cxx11_only");
  
    //debug print args
    printf("Printing args:\n");
	
    for(int i = 0; i < argv_plus.size(); i++){
		printf("\t%s\n", argv_plus[i].c_str());
    }

    // create project from arg list
    SgProject* project = frontend(argv_plus);
    ROSE_ASSERT(project != NULL);

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
