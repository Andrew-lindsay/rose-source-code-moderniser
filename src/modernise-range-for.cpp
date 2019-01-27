#include <vector>
#include "rose.h"
#include <string>

using std::string;
using namespace SageBuilder;
using namespace SageInterface;

class IteratorUseTransform : public AstSimpleProcessing{
	
    SgName iterator;
	
public:
	IteratorUseTransform(const SgName& iter){
		iterator = iter;
	}

private:
	void visit(SgNode* node){

		// need to take into account the overloaded operators again
		// pointerDerefExp not enough
		if(SgPointerDerefExp* refExp = isSgPointerDerefExp(node)){
			
		}
		else if(SgArrowExp* arrowExp = isSgArrowExp(node)){
			
		}
		
	}
	
};


class IteratorUseTraversal : public AstSimpleProcessing{

    SgName container;
	SgName iterator;
	bool validToTransform = true;
private:
	
	/*
	 * returns name of the function called if object getting its method called matches containerName 
	 */
    SgName* getMethodCalled(SgFunctionCallExp* func, const SgName& containerName){

		//printf("class of func: %s\n", func->get_function()->class_name().c_str());
		SgDotExp* dotExp =  isSgDotExp(func->get_function());
		if(dotExp == NULL){ return NULL;}
		
		SgVarRefExp* lhs = isSgVarRefExp(dotExp->get_lhs_operand());
		SgMemberFunctionRefExp* rhs = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
	   
		//printf("class of dotVar: %s\n", dotExp->get_lhs_operand()->class_name().c_str());
		//printf("class of dotMemFunc: %s\n", dotExp->get_rhs_operand()->class_name().c_str());
		
   		if(lhs == NULL || rhs == NULL ){return NULL;}
		
		printf("Method call - Condition MethodName: %s ,containerName %s\n"
			   , rhs->get_symbol()->get_name().getString().c_str()
			   , lhs->get_symbol()->get_name().getString().c_str());
		
		if(lhs->get_symbol()->get_name() == containerName){
			return new SgName(rhs->get_symbol()->get_name());
		}
		
		return NULL;
	}

public:
	
	IteratorUseTraversal(const SgName& con, const SgName& iter){
		container = con;
		iterator = iter;
	}

	bool getValidToTransform(){
		return validToTransform;
	}
	
	void visit(SgNode* node){
		// check for uses of iterator
		// check for methods applied to iterator
		// check that it is only used to be defereneced
		// arrow methods are allowed as they are just deference followed by method call
		// in the end we replace all dereferences with the elem for the range operator
		// check for uses of the container in the loop body, if used do not transform
				
		SgVarRefExp* varRef = isSgVarRefExp(node);
		
		if(varRef != NULL /*&& (varRef->get_symbol()->get_name() == container || varRef->get_symbol()->get_name() == iterator)*/){
			//printf("Found use of container or iterator: %s\n" , varRef->get_symbol()->get_name().getString().c_str());
			// check no method calls to container take place 
			// only operations on the iterator is dereferences
			
		}
		else if(SgFunctionCallExp* func = isSgFunctionCallExp(node)){
			// detects any Dot method call to the container (container wont be a pointer so no need for arrow)
			// other types of function calls (need to know format of call)
		   
			SgName* containerMethod = getMethodCalled(func, container);
			if(containerMethod != NULL){ // if container has a method call, transform not safe
				printf("Container Method call: %s.%s\n", container.getString().c_str(),
					   containerMethod->getString().c_str());
				
				validToTransform = false;
			}
			delete containerMethod;

			// what about reference then dereference not really an iterator anymore in type ?
			
			// only operation on the iterator is a deference (*iter)
			SgName* opName = getMethodCalled(func, iterator);
			if(opName != NULL && ( *opName) != SgName("operator*") && (*opName) != SgName("operator->") ){
				printf("iterOp not dereference: %s\n", opName->getString().c_str());
				validToTransform = false;
			}
			delete opName;
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
	
    SgInitializedName* hasValidInitializer(SgNode* node, SgName& containerName, SgName& iteratorName){
		SgName begin = "begin";
		
		// 1. get variable name(or symbol) and check container name and that it calls begin
		SgVariableDeclaration *initVarDec = isSgVariableDeclaration(node->get_traversalSuccessorContainer()[0]); 
		if(initVarDec == NULL){return NULL;}
		
		// iterators name 
	    iteratorName =  initVarDec->get_variables().at(0)->get_name();
		SgInitializedName* initialisedName = initVarDec->get_variables().at(0);
	   
		SgAssignInitializer* assign = isSgAssignInitializer(initialisedName->get_initializer());
		if(assign == NULL ){ return NULL;}
		
		SgFunctionCallExp* func = isSgFunctionCallExp(assign->get_operand()); // ensures we dont get any nesting
		if(func == NULL){ return NULL;}

		SgDotExp* dotExp =  isSgDotExp(func->get_function());
		if(dotExp == NULL){ return NULL;}
		
		SgVarRefExp* lhs = isSgVarRefExp(dotExp->get_lhs_operand());
		SgMemberFunctionRefExp* rhs = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
   		if(lhs == NULL || rhs == NULL ){return NULL;}
		if(rhs->get_symbol()->get_name() != begin){return NULL;}
		
		containerName = lhs->get_symbol()->get_name();
		
		return initialisedName;;
	}
	
	bool isMethodCall(SgFunctionCallExp* func, const SgName& containerName,const SgName& methodName){

		//printf("class of func: %s\n", func->get_function()->class_name().c_str());
		SgDotExp* dotExp =  isSgDotExp(func->get_function());
		if(dotExp == NULL){ return false;}
		
		SgVarRefExp* lhs = isSgVarRefExp(dotExp->get_lhs_operand());
		SgMemberFunctionRefExp* rhs = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
		
		//printf("class of dotVar: %s\n", dotExp->get_lhs_operand()->class_name().c_str());
		//printf("class of dotMemFunc: %s\n", dotExp->get_rhs_operand()->class_name().c_str());
		
   		if(lhs == NULL || rhs == NULL ){return false;}
		
		printf("Method call - Condition MethodName: %s ,containerName %s\n"
			   , rhs->get_symbol()->get_name().getString().c_str()
			   , lhs->get_symbol()->get_name().getString().c_str());
		
		if(rhs->get_symbol()->get_name() != methodName || lhs->get_symbol()->get_name() != containerName){
			return false;
		}

		return true;
	}
	
	bool hasValidComparitor(SgStatement* testFor, const SgName& containerName, const SgName& iterName){
		SgName methodNameEnd = "end";

		//DEBUG
		//printf("class of test: %s\n",testFor->class_name().c_str());
		SgExprStatement* expr = isSgExprStatement(testFor);
		if(expr == NULL){return false;}
		
		//DEBUG
		//printf("class of NEQ : %s\n",expr->get_expression()->class_name().c_str());
		
		// dealing with an over loaded operator due to iterators
		SgFunctionCallExp* funcOverLoad = isSgFunctionCallExp(expr->get_expression());
		if(funcOverLoad == NULL){return false;}
		
		SgFunctionRefExp* neqFunc = isSgFunctionRefExp(funcOverLoad->get_function());
		if( !(neqFunc != NULL && neqFunc->get_symbol()->get_name() == "operator!=")){return false;}
		
		if(funcOverLoad->get_args()->get_expressions().size() != 2){ return false;}
		
		SgVarRefExp* var = isSgVarRefExp(funcOverLoad->get_args()->get_expressions().at(0) );
		SgFunctionCallExp* func = isSgFunctionCallExp(funcOverLoad->get_args()->get_expressions().at(1));
		
		// SgNotEqualOp* neq = isSgNotEqualOp(expr->get_expression()) ;
		// if(neq == NULL){return false;}
		
		// DEBUG
		//printf("class of NEQ FUNC: %s\n", funcOverLoad->get_args()->get_expressions().at(1)->class_name().c_str());	// *somthing* != function
		//printf("class of NEQ varRef: %s\n",funcOverLoad->get_args()->get_expressions().at(0)->class_name().c_str()); // a != *something*
		
		if(var != NULL && func != NULL
		   &&  var->get_symbol()->get_name() == iterName
		   && isMethodCall(func, containerName, methodNameEnd ) ){
			printf("Condition iteratorName: %s\n",var->get_symbol()->get_name().getString().c_str());
			return true;
		}
		
		// check for the args to =! being the oppsite way round
		SgVarRefExp* varR = isSgVarRefExp(funcOverLoad->get_args()->get_expressions().at(1) );
		SgFunctionCallExp* funcL = isSgFunctionCallExp(funcOverLoad->get_args()->get_expressions().at(0));
													  
		if( varR != NULL && funcL != NULL
			&& varR->get_symbol()->get_name() == iterName
			&& isMethodCall(funcL, containerName, methodNameEnd)){
			//DEBUG
			printf("Condition iteratorName: %s\n",varR->get_symbol()->get_name().getString().c_str());
			return true;
		}
		
	    return false;
	}

	// iterators have overloaded operators functions !=, ++. ==, etc
	
	/*
	 * Function checks that an overloaded ++ operator is utilised on the same variable passed as argument 
	 * iterators using prefix ++ and posfix ++ are both handled
	 */
	bool hasVaildIncrement( SgExpression* incrFor, const SgName& iteratorName){

		// is a function call using overloaded ++ operator
	    SgFunctionCallExp* funCall = isSgFunctionCallExp(incrFor);
		if(funCall == NULL){return false;}
		
		// check method call is on iterator and is the ++ member operation 
		if(!isMethodCall(funCall, iteratorName, "operator++")){
			return false;
		}

		printf("Valid Increment\n");
		return true;
	}
	
	/*
	 * If all 3 part of for loop match the intialisation condition and return true 
	 * for( vector<something>::iterator iter = vec.begin; iter != vec.end(); vec++) has to iterate throught the entire container 
	 * for ( type elem : container)
	 */
	bool isIteratorLoop(SgForStatement *forNode, SgName& containerName, SgName& iteratorName){		
		SgForInitStatement* initFor = forNode->get_for_init_stmt();
		SgStatement* testFor = forNode->get_test();
		SgExpression* incrFor = forNode->get_increment();
		
		// DEBUG 
		for(SgNode* iter : initFor->get_traversalSuccessorContainer()){
			printf("Node Name: %s\n", iter->unparseToString().c_str());
		}

		// Ensure only single vardeclaration om init statement
		if(initFor->get_traversalSuccessorContainer().size() != 1){ return false;}
		
		printf("\tChecking Initializer\n");
		if(!hasValidInitializer(initFor, containerName, iteratorName)){return false;}
		
		// DEBUG
		printf("Container Name: %s\n", containerName.getString().c_str());
		printf("Name of Var %s\n", iteratorName.getString().c_str());
		
		printf("\tChecking comparitor\n");
		// init variable already declared before for loop maybe a problem 
		// check (lhs || rhs) is the same variable and the other side is same container calling end
		if(!hasValidComparitor(testFor, containerName, iteratorName)){return false;}
		
		printf("\tChecking Increment\n");
		if(!hasVaildIncrement( incrFor, iteratorName)){return false;}
		
		// needs to be the same container being iterated (adding one may not work for all containers) 
		
		// should only be Variable declaration or assignment
		// get initialiser name 
		
		return true;
	}

	/*
	 * Used to access the safety of a transformation
	 *   - checks for uses of the iterator in defined in the initializer
	 *   - methods are called on the iterator then 
	 * 
	 * For loop bodies can contain basic blocks or singe statements this does not 
	 * matter as we are traversing the entire body 
	 */
	bool safeForTransform(SgForStatement *forNode, const SgName& conName, const SgName& iterName){

		printf("\tChecking Safe to Transform\n");
		IteratorUseTraversal iterUseTraversal = IteratorUseTraversal(conName, iterName); // constructor
		iterUseTraversal.traverse(forNode->get_loop_body(), preorder);
		
		return iterUseTraversal.getValidToTransform();
	}

	inline bool isCompilerGenerated(SgNode *node){return node->get_file_info()->isCompilerGenerated();}

	SgRangeBasedForStatement* constructRangedBasedForLoop(SgName& conName, SgName& iterName, SgForStatement *loopNode){
		// normal array will be of array type, every STL class vector, map, etc will probably be of class type (don't know what TypedefType is or used for like typedef struct thing)		
		// get iterator type (assumption being made that the iterator type will have some way of gettin the underlying type)
		// can just use auto instead to get type,
		SgInitializedName* iteratorNode =  hasValidInitializer(loopNode->get_for_init_stmt() , conName, iterName);
		SgType* iteratorType = iteratorNode->get_type();
		
		// DEBUG
		// printf("Type of Iterator: %s, class name: %s\n", iteratorType->unparseToString().c_str(), iteratorType->class_name().c_str());
		// printf("Has Internal Types %s, base type: %s\n", (iteratorType->containsInternalTypes() ? "true" : "false"), iteratorType->dereference()->class_name().c_str() );
		// printf("Iter deref Class: %s, TEXT: %s \n",iteratorType->dereference()->unparseToString().c_str(), iteratorType->dereference()->class_name().c_str());
		// printf("Iter deref^2 Class: %s, TEXT: %s \n",iteratorType->dereference()->dereference()->unparseToString().c_str()
		// 	   , iteratorType->dereference()->dereference()->class_name().c_str());

		SgVariableDeclaration* varDecl = isSgVariableDeclaration(iteratorNode->get_declaration());
		
		// if classType or TypeDefType try and extract type else default to auto
		// ======================= ======================= =======================
		// get definition of class which is should be a Templated class
		if(isSgClassType(iteratorType->dereference()) && isSgTemplateInstantiationDecl(((SgClassType*) iteratorType->dereference())->get_declaration())){
		    printf("Is Templated Class\n");}else{printf("Is Not Templated Class\n");
		}
		// DEBUG
		if(varDecl != NULL){printf("Variable Declaration specialisation: %s\n", (varDecl->isSpecialization() ?  "true" : "false"));}
		// ======================= ======================= =======================
   

		// BUILDING REPLACEMENT FOR LOOP:
		pushScopeStack(loopNode); // very important to set scope
		
		// DEBUG
		printf("conName: %s\n",conName.getString().c_str());
		// Initialiser declaration: here is were the type would be extracted from the container is added but just use auto instead
		SgVariableDeclaration* initializer_var
			= buildVariableDeclaration(iterName, buildTemplateType("auto"));
		
		// Range declaration
		SgVariableDeclaration* range_var =
			buildVariableDeclaration("_range", buildTemplateType("auto"), buildAssignInitializer(buildVarRefExp(conName)));
		
		/* do not require many of the components of the rangeBasedForStatement to be actually
		 * added just variable storing elements and the range (container being used)*/
		SgRangeBasedForStatement* rangeFor =
			buildRangeBasedForStatement_nfi(initializer_var, range_var, NULL, NULL, NULL, NULL, buildBasicBlock());
		
		return rangeFor;
	}
	
public:

	void visit(SgNode *node){
		
		SgForStatement *loopNode  = isSgForStatement(node); // ignores ranged for loops
		
		SgName containerName;
		SgName iteratorName;
		
		if(loopNode != NULL && !isCompilerGenerated(loopNode)){
			printf("=== FOR loop Start ===\n");

			if( isIteratorLoop(loopNode, containerName, iteratorName) && safeForTransform(loopNode, containerName, iteratorName)){
				// DEBUG
				printf("isSafeToTransform: true\n");
				printf("FOR LOOP FOUND: %s, name: %s, compiler Gen: %s\n"
					   , loopNode->class_name().c_str()
					   , loopNode->unparseToString().c_str()
					   , isCompilerGenerated(loopNode) ? "true" : "false");

				// save old loop body
				SgStatement* loopBody = loopNode->get_loop_body();
				
				// build new ranged for loop
				SgRangeBasedForStatement* rangeFor = constructRangedBasedForLoop(containerName, iteratorName, loopNode);
			    
				// transform loop body to work with new ranged header
				
				IteratorUseTransform iteratorTransform(iteratorName);
				iteratorTransform.traverse(loopBody, preorder);
				
				// change loop body
				rangeFor->set_loop_body(loopBody);
				
				// replace old "for" with new "rangeFor"
				replaceStatement(loopNode, rangeFor); // actually inserting happens here
				popScopeStack(); // return scope to whatever it was remove
				
			}
			else if(false){
				
			}
			else if(false){

			}
			
			printf("======================\n");
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
	//AstTests::runAllTests(project) ;
    return backend(project);
}
