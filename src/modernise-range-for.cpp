#include <vector>
#include "rose.h"
#include <string>

using std::string;

/* USAGE:
 *   InitialisedNameTraversal traveral;
 *   traveral.traverse(initFor, preorder);
 */
class InitialisedNameTraversal : public AstSimpleProcessing{

	SgInitializedName* retNode = NULL;

public:
	
	SgInitializedName* getName(){
		return retNode;
	} 
	
    void visit(SgNode* node){
		
		SgInitializedName* nameNode = isSgInitializedName(node);
		
		if(nameNode != NULL){
			retNode = nameNode;
		}
		
	}
};


class InitialConditionTraversal  : public AstSimpleProcessing{

	bool isValid = false;
	SgName begin = "begin"; // hopefully can compare SgNames like this!
	SgName containerName; // if no contianer then empty SgName on completion
	
public:
	SgName getContainersName(){
		return containerName;
	}
	
	bool hasCalledBegin(){
		return isValid;
	}
	
    void visit(SgNode* node){	
		SgMemberFunctionRefExp* memberFunc = isSgMemberFunctionRefExp(node);
		SgVarRefExp* containerRef = isSgVarRefExp(node);
		
		// has begin method been called (needs to be more general handle pointers to containers)
		// problem with method call after method call e.g vec.somthing().begin()
		if(isSgMemberFunctionRefExp(node) && ((SgMemberFunctionRefExp*)node)->get_symbol()->get_name() == begin){
			isValid = true;
		}
		// if our initialiser varDec is of size 1 (one statement) it should
		// call the containers "begin" method to initialise the iterator. 
		else if(isSgVarRefExp(node)){ // edit if set dont change ? 
			containerName = ((SgVarRefExp*) node)->get_symbol()->get_name();
		}
	}
};


/*
 * Detects the 3 standard uses of for loops and transforms them to ranged for loops 
 *   1. statically allocated 
 *   2. iterators for conatiners
 *   3. array like containers using operators [] and at()
 *   NOTE: keep implementation simple
 */
class SimpleForLoopTraversal : public AstSimpleProcessing{
	
private:

    bool hasValidInitializer(SgNode* node, SgName& containerName, SgName& iteratorName){
		SgName begin = "begin";
		
		// 1. get variable name(or symbol) and check container name and that it calls begin
		SgVariableDeclaration *initVarDec = isSgVariableDeclaration(node->get_traversalSuccessorContainer()[0]); 
		if(initVarDec == NULL){return false;}
		
		// iterators name 
	    iteratorName =  initVarDec->get_variables().at(0)->get_name();
		SgInitializedName* initialisedName = initVarDec->get_variables().at(0);
	   
		
		SgAssignInitializer* assign = isSgAssignInitializer(initialisedName->get_initializer());
		if(assign == NULL ){ return false;}

		SgFunctionCallExp* func = isSgFunctionCallExp(assign->get_operand()); // ensures we dont get any nesting
		if(func == NULL){ return false;}
		
		SgDotExp* dotExp =  isSgDotExp(func->get_function());
		if(dotExp == NULL){ return false;}
		
		SgVarRefExp* lhs = isSgVarRefExp(dotExp->get_lhs_operand());
		SgMemberFunctionRefExp* rhs = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
   		if(lhs == NULL || rhs == NULL ){return false;}
		if(rhs->get_symbol()->get_name() != begin){return false;}

		
		containerName = lhs->get_symbol()->get_name();
		
		return true;
	}
	
	/*
	 * If all 3 part of for loop match the intialisation condition and return true 
	 * for( vector<something>::iterator iter = vec.begin; iter != vec.end(); vec++) has to iterate throught the entire container 
	 * for ( type elem : container)
	 */
	bool isIteratorLoop(SgForStatement *forNode){		
		SgForInitStatement* initFor = forNode->get_for_init_stmt();
		SgStatement* testFor = forNode->get_test();
		SgExpression* incrFor = forNode->get_increment();
		
		// DEBUG 
		for(SgNode* iter : initFor->get_traversalSuccessorContainer()){
			printf("Node Name: %s\n", iter->unparseToString().c_str());
		}

		// Ensure only single vardeclaration om init statement
		if(initFor->get_traversalSuccessorContainer().size() != 1){ return false;}
		
		SgName containerName;
		SgName iteratorName;
		if(!hasValidInitializer(initFor, containerName, iteratorName)){
			return false;
		}
		// DEBUG
		printf("Container Name: %s\n", containerName.getString().c_str());
		printf("Name of Var %s\n", iteratorName.getString().c_str());

		
		// if assigned
		
		// check (lhs || rhs) is the same variable and the other side is same container calling end
		

		// needs to be the same container being iterated (adding one may not work for all containers) 
		
		// should only be Variable declaration or assignment
		// get initialiser name 
		
		return true;
	}

	/*
	 * The use of the data structure in the loop means it is safe to transform
	 */
	bool safeForTransform(SgForStatement *forNode){
		return true;
	}

	inline bool isCompilerGenerated(SgNode *node){return node->get_file_info()->isCompilerGenerated();}
	
public:

	void visit(SgNode *node){
		
		SgForStatement *loopNode  = isSgForStatement(node); // ignores ranged for loops 
			
		if(loopNode != NULL && !isCompilerGenerated(loopNode)){
			
			if( isIteratorLoop(loopNode) && safeForTransform(loopNode)){
				
				printf("FOR LOOP FOUND: %s, name: %s, compiler Gen: %s\n"
					   , loopNode->class_name().c_str()
					   , loopNode->unparseToString().c_str()
					   , isCompilerGenerated(loopNode) ? "true" : "false");
				
			}
			else if(false){
				
			}
			else if(false){

			}
		}
		
	}
};


int main(int argc, char* argv[]){

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
    //SimpleVarDecTraversal traversal;
	SimpleForLoopTraversal traversal;
	traversal.traverseInputFiles(project, preorder);
	
    return backend(project);
}
