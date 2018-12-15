#include <vector>
#include "rose.h"

class SimpleForLoopTraversal : public AstSimpleProcessing{
	
private:
	/*
	 * If all 3 part of for loop match the intialisation condition and 
	 * iteration return true 
	 */
	bool isIteratorLoop(SgForStatement *forNode){
		return true;
	}
	
	bool safeForTransform(SgForStatement *forNode){
		return true;
	}
	
public:

	void visit(SgNode *node){
		
		SgForStatement *loopNode  = isSgForStatement(node);
			
		if(loopNode != NULL){
			if( isIteratorLoop(loopNode) && safeForTransform(loopNode)){
				
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
    //traversal.traverseInputFiles(project, preorder);
    
    return backend(project);

}
