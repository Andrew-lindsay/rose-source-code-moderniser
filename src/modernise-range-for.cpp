
#include "rose.h"
#include <vector>
#include <string>
#include <utility>

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
	
	void replaceExpressionCheck(SgExpression* currentExp, SgExpression* newExp){
		
		if(SgExpression* parentParent = isSgExpression(currentExp->get_parent())){;
			//if(parentParent == NULL){ printf("Not expression\n");return;}
				
			printf("parent node: %s \n", parentParent->unparseToString().c_str());
			parentParent->replace_expression(currentExp, newExp);
   
			printf("Finished repalcement\n");
					
		}// expression Statement may not be the only possible statement that is parent of pointerDeref
		else if(SgExprStatement* exptStmt = isSgExprStatement(currentExp->get_parent())){
		    printf("Not expression\n");
			exptStmt->set_expression(newExp);
			printf("Finished repalcement\n");
		}else{
			printf("Conditions not meet no transformation intacted\n");
		}
	}
	
	/*
	 * Search for uses of the iterator variable to remove the dereferences or arrow reference 
	 * from the iterator, iterator in ranged loop takes on values of the container
	 */
	void visit(SgNode* node){ // we know that the only uses of variable are deref and arrowexp

		// Target VarRefs of the iterator 
		if(isSgVarRefExp(node) && ((SgVarRefExp*)node)->get_symbol()->get_name() == iterator){
	   
			SgVarRefExp* iterRef = (SgVarRefExp*) node;
			// parent of instance of iterator variable 
			SgNode* parentNode = ((SgVarRefExp*)node)->get_parent();

			// ==== POINTER DEFERENCE ====
			if(SgPointerDerefExp* ptrDeRef = isSgPointerDerefExp(parentNode)){
				// remove the pointerDeref node and replace with just the varRefExp node (don't delete the node)
				printf("Replacing expression\n");

				// if the parent of the dereference is not an expression does it have to be a ExprStatement ?(No)
				// can be in an if statementhtop
				
				if(SgExpression* parentParent = isSgExpression(ptrDeRef->get_parent())){;
					//if(parentParent == NULL){ printf("Not expression\n");return;}
				
					printf("parent node of deReference: %s \n", parentParent->unparseToString().c_str());
					parentParent->replace_expression(ptrDeRef, iterRef);
   
					printf("Finished repalcement\n");
					
				}// expression Statement may not be the only possible statement that is parent of pointerDeref
				else if(SgExprStatement* exptStmt = isSgExprStatement(ptrDeRef->get_parent())){
					exptStmt->set_expression(iterRef);
				}

			} // ==== ARROW DEREFERENCE ==== 
			else if(SgArrowExp* arrowExp = isSgArrowExp(parentNode)){ // shows up in odd places like overloaded derefence
				// build dotExp type
				// replace arrow with dotExp
				
				SgDotExp* dotExp = buildDotExp(iterRef, arrowExp->get_rhs_operand());

				SgExpression* parentParent = isSgExpression(arrowExp->get_parent());
				if(parentParent == NULL){ printf("Not expression\n");return;}
				printf("parent node of ArrowExp: %s \n", parentParent->unparseToString().c_str());
				
				parentParent->replace_expression(arrowExp, dotExp );   
				printf("Finished repalcement\n");
					
			} // ==== OVERLOADED OPERATORS ====
			else if(SgDotExp* dotExp = isSgDotExp(parentNode)){ 
				
				// ensure parent is an DotExp
				// checking that operator is either -> or * both require the same action
				// just arrow requires changing arrow to dot as well
				// need to find the root functionExp that is the overloaded function being used :)

				SgMemberFunctionRefExp*
					funcMember = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
				
				// operator* or operator->
				if(funcMember != NULL && 
				   (funcMember->get_symbol()->get_name() == "operator*"
					|| funcMember->get_symbol()->get_name() == "operator->") ){

					SgNode* nodeCheck = dotExp->get_parent();

					// get function call related with call to deference
					while( !isSgFunctionCallExp(nodeCheck) ){
						nodeCheck = nodeCheck->get_parent();
					}

					SgFunctionCallExp* funcExp =  isSgFunctionCallExp(nodeCheck);
					if(funcExp == NULL){ printf("failed to find Function call\n");return;}

					// find arrow 
					if(funcMember->get_symbol()->get_name() == "operator->"){
						SgNode* nodeCheckArrow = funcExp->get_parent();

						/* in some cases between the overloaded call and the
						 * arrowExp node there is other things such as cast nodes
						 */
						while(!isSgArrowExp(nodeCheckArrow)){
							nodeCheckArrow = nodeCheckArrow->get_parent();
						}

						SgArrowExp* arrowBaseExp = isSgArrowExp(nodeCheckArrow);
						if(arrowBaseExp == NULL){ printf("Expected ArrowExp not found\n");return;}

						// build new dot to replace arrow
						SgDotExp* dotExpNew = buildDotExp(iterRef, arrowBaseExp->get_rhs_operand());
						
						
						replaceExpressionCheck(arrowBaseExp, dotExpNew); // replace arrowExp
						
					}else{ // if not arrow just dot
						replaceExpressionCheck(funcExp, iterRef); // replace funCall
					}
				}
			}
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

	/*
	 * only to be used after traversing the loop body
	 */
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
		
		//printf("Found use of container or iterator: %s\n" , varRef->get_symbol()->get_name().getString().c_str());

		if(varRef != NULL && varRef->get_symbol()->get_name() == iterator){
			// check that only uses are dereference normal or overloaded
			
			SgNode* parentNode = varRef->get_parent(); // check for compiler gen ? 
			
			// pointer Dereference (simple)
			if(SgPointerDerefExp* ptrDeref = isSgPointerDerefExp(parentNode) ){
				// do nothing this is okay  
			}// arrowExp 
			else if(SgArrowExp* arrowExp = isSgArrowExp(parentNode)) {
				// allow arrow to pass
			}// dotexp (overloaded function are of this form)
			else if(SgDotExp* dotExp = isSgDotExp(parentNode)){
				// right hand side has to be a member function
				SgMemberFunctionRefExp* rhsOpCall = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
				// if not method call then 
				if(rhsOpCall != NULL && ( rhsOpCall->get_symbol()->get_name() == "operator*"
										   || rhsOpCall->get_symbol()->get_name() == "operator->") ){
					// do nothing still valid transform
				}else{// if not method call on other side of dot or method call not * or -> not valid
					validToTransform = false;
					printf("Invalid Iterator use: line %d, Dot rhs class: %s\n"
						   ,dotExp->get_rhs_operand()->get_file_info()->get_line()
						   ,dotExp->get_rhs_operand()->class_name().c_str());
				}
					
			}else{
				// if non of the above then
				printf("Invalid Iterator use: line %d, parent class: %s\n", parentNode->get_file_info()->get_line(),
					parentNode->class_name().c_str());
				validToTransform = false;
			}
				
		}// dealing with a container reference
		// TODO: change to fail if container used at all in loop body
		else if(varRef != NULL && varRef->get_symbol()->get_name() == container){
			// if any method call to container, unsafe to transform
			// what about field access ?
			SgNode* parentNode = varRef->get_parent();
			// any function call on container is dangerous 
			if(SgDotExp* dotExp = isSgDotExp(parentNode)){
				// right hand side has to be a member 
				SgMemberFunctionRefExp* rhsOpCall = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
				if(rhsOpCall != NULL){validToTransform = false;}
				printf("Method used on container: line %d\n",rhsOpCall->get_file_info()->get_line());
			}
		}
	}
};


class IndexUseTraversal : public AstSimpleProcessing{

	SgName container;
	SgName index;
	bool validTransform = false;

	std::pair<SgName,SgName> getMethodCall(SgNode* funcExp){
		// default initialises to ""
		std::pair<SgName,SgName> methodCall;
		printf("%s\n", funcExp->class_name().c_str());
		
		SgFunctionCallExp* rhsExp = isSgFunctionCallExp(funcExp);
		if(rhsExp == NULL){ return methodCall;}
			
		SgDotExp* dotExp =  isSgDotExp(rhsExp->get_function());
		if(dotExp == NULL){ return methodCall;}
		
		SgVarRefExp* lhsFuncContainer = isSgVarRefExp(dotExp->get_lhs_operand());			
		SgMemberFunctionRefExp* rhsFuncMethod = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
		
		if(lhsFuncContainer == NULL || rhsFuncMethod == NULL ){return methodCall;}
		// checking if function call does use the size call
		return std::make_pair(lhsFuncContainer->get_symbol()->get_name(),rhsFuncMethod->get_symbol()->get_name());
	}
	
public:
	IndexUseTraversal(const SgName & con, const SgName& indx){
		this->container = con;
		this->index = indx;
	}

	// will be removing the index with the element of the array  
	// should be able to be used by both array transforms
	void visit(SgNode* node){

		// check for use of index out side of array access 
		
		// search for container or index they are linked 
		
		// [] operation

		// .at() operation
		
		// overloaded [], .operator[]

		// CHECK 2: only calls to the container are at() and [] 
		
		// CHECK 1: container.call(index);
		//     check index is inside an appropriate operator [] at or operator[] 
		//     then check that the function call taking place is an 

		// SgPntrArrRefExp
		SgVarRefExp* varRef = isSgVarRefExp(node);
		
	    if(varRef != NULL && varRef->get_symbol()->get_name() == index){

			// remove casting
			SgNode* parentOfVar = varRef->get_parent();
			if(SgCastExp* castExp = isSgCastExp(parentOfVar)){
				parentOfVar = castExp->get_parent();
			}

			// is and overloaded operator at or []
			if(SgExprListExp* funcArgs =  isSgExprListExp(parentOfVar)){
					
				std::pair<SgName,SgName> funCall = getMethodCall(funcArgs->get_parent());

				// if not the container specified in the loop header error if it is only
				// methods allowed to be called are [] or at
				if(funCall.first != container
				   ||  !(funCall.second == "operator[]" || funCall.second == "at")){
				    validTransform = false;
				}
				
			}// is a non overloaded array [] 
			else if(SgPntrArrRefExp* arrAccess = isSgPntrArrRefExp(parentOfVar)){
				
				SgVarRefExp* conRef = isSgVarRefExp(arrAccess->get_lhs_operand());
				
			    if(conRef == NULL || conRef->get_symbol()->get_name() != container){
					validTransform = false;
				}
			
			}
			else{
				validTransform = false;
			}
			
		} // container check, only calls to the container are at() and []
		else if(varRef != NULL && varRef->get_symbol()->get_name() == container){

			SgDotExp* dotExp = isSgDotExp(varRef->get_parent());
			if(dotExp == NULL){return;}

		    SgMemberFunctionRefExp* memberF
				= isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
				
			if(memberF == NULL
			   || !( memberF->get_symbol()->get_name() == "at"
					 || memberF->get_symbol()->get_name() == "operator[]")){
				   validTransform = false;
			}
		}
	}

	bool getValidToTransform(){
		return validTransform;
	}
};



/*
 * Detects the 3 standard uses of for loops and transforms them to ranged for loops 
 *   1. statically allocated 
 *   2. iterators for conatiners
 *   3. array like containers using operators [] and at()
 *   NOTE: keep implementation simple
 */
class ForLoopTraversal : public AstSimpleProcessing{
	
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
		if( SgFunctionCallExp* funcOverLoad = isSgFunctionCallExp(expr->get_expression())){
	
			SgFunctionRefExp* neqFunc = isSgFunctionRefExp(funcOverLoad->get_function());
		
			if( !( neqFunc != NULL && neqFunc->get_symbol()->get_name() == "operator!=")){return false;}
			
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
			   && isMethodCall(func, containerName, methodNameEnd)){
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
		}// is a non overloaded !=
		else if (SgNotEqualOp* notEq = isSgNotEqualOp(expr->get_expression())){
		    
			SgVarRefExp* iterVar;
			SgFunctionCallExp* func;
			printf("NON-overloaded not equals\n");
			// checking both orderings of the not equals operator
			if(isSgVarRefExp(notEq->get_lhs_operand())){
				iterVar = isSgVarRefExp(notEq->get_lhs_operand());
				func = isSgFunctionCallExp(notEq->get_rhs_operand());
			}else{
				func = isSgFunctionCallExp(notEq->get_lhs_operand());
				iterVar = isSgVarRefExp(notEq->get_rhs_operand());
			}
						
			if( iterVar != NULL && func != NULL
				&& iterVar->get_symbol()->get_name() == iterName
				&& isMethodCall(func, containerName, methodNameEnd)){	
				return true;
			}
			
		}// if not overloaded != or standard != then return false
		else{
			return false;
		}
	}

	// iterators have overloaded operators functions !=, ++. ==, etc
	
	/*
	 * Function checks that an overloaded ++ operator is utilised on the same variable passed as argument 
	 * iterators using prefix ++ and posfix ++ are both handled
	 */
	bool hasVaildIncrement( SgExpression* incrFor, const SgName& iteratorName){

		// is a function call using overloaded ++ operator
	    if(SgFunctionCallExp* funCall = isSgFunctionCallExp(incrFor)){
			// check method call is on iterator and is the ++ member operation 
			if(isMethodCall(funCall, iteratorName, "operator++")){
				printf("Valid Increment\n");
				return true;
			}
		}
		else if(SgPlusPlusOp* iterPP = isSgPlusPlusOp(incrFor)) {
		    SgVarRefExp* iterVar = isSgVarRefExp(iterPP->get_operand());
			if(iterVar != NULL &&  iterVar->get_symbol()->get_name() == iteratorName){
				printf("Valid Increment\n");
				return true;
			}
		}
		else{
			printf("InValid Increment\n");
			return false;
		}
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
		
		printf("\tChecking Initializer: Iterator Container\n");
		if(!hasValidInitializer(initFor, containerName, iteratorName)){return false;}
		
		// DEBUG
		printf("Container Name: %s\n", containerName.getString().c_str());
		printf("Name of Var %s\n", iteratorName.getString().c_str());
		
		printf("\tChecking comparitor: Iterator Container\n");
		// init variable already declared before for loop maybe a problem 
		// check (lhs || rhs) is the same variable and the other side is same container calling end
		if(!hasValidComparitor(testFor, containerName, iteratorName)){return false;}
		
		printf("\tChecking Increment: Iterator Container\n");
		if(!hasVaildIncrement( incrFor, iteratorName)){return false;}
		
		// needs to be the same container being iterated (adding one may not work for all containers) 
		return true;
	}

	/*
	 * Used to assess the safety of a transformation
	 *   - checks for uses of the iterator in defined in the initializer
	 *   - methods are called on the iterator then 
	 * 
	 * For loop bodies can contain basic blocks or singe statements this does not 
	 * matter as we are traversing the entire body 
	 */
	bool safeIteratorForTransform(SgForStatement *forNode, const SgName& conName, const SgName& iterName){
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
		
		// SgInitializedName* iteratorNode =  hasValidInitializer(loopNode->get_for_init_stmt() , conName, iterName);
		// SgType* iteratorType = iteratorNode->get_type();
		
		//SgVariableDeclaration* varDecl = isSgVariableDeclaration(iteratorNode->get_declaration());
		
		// if classType or TypeDefType try and extract type else default to auto
		// ======================= ======================= =======================
		// get definition of class which is should be a Templated class
		// if(isSgClassType(iteratorType->dereference()) && isSgTemplateInstantiationDecl(((SgClassType*) iteratorType->dereference())->get_declaration())){
		//     printf("Is Templated Class\n");}else{printf("Is Not Templated Class\n");
		// }
		// DEBUG
		// if(varDecl != NULL){printf("Variable Declaration specialisation: %s\n", (varDecl->isSpecialization() ?  "true" : "false"));}
		// ======================= ======================= =======================   

		// BUILDING REPLACEMENT FOR LOOP:
		pushScopeStack(loopNode); // very important to set scope
		
		// DEBUG
		printf("conName: %s\n",conName.getString().c_str());
		
		// Initialiser declaration: here is were the type would be extracted from the container is added but just use auto instead
		SgVariableDeclaration* initializer_var
			= buildVariableDeclaration(iterName, buildTemplateType("auto")); // need to add reference operator &
		
		// Rangedeclaration
		SgVariableDeclaration* range_var =
			buildVariableDeclaration("_range", buildTemplateType("auto"), buildAssignInitializer(buildVarRefExp(conName)));
		
		/* do not require many of the components of the rangeBasedForStatement to be actually
		 * added just variable storing elements and the range (container being used)*/
		SgRangeBasedForStatement* rangeFor =
			buildRangeBasedForStatement_nfi(initializer_var, range_var, NULL, NULL, NULL, NULL, buildBasicBlock());
		
		return rangeFor;
	}
// ========================== Array like containers ========================

	bool safeIndexForTransform(SgForStatement *forNode, const SgName& conName, const SgName& iterName){
		printf("\tChecking Safe to Transform\n");
		IndexUseTraversal indxUseTraversal = IndexUseTraversal(conName, iterName); // constructor
	    indxUseTraversal.traverse(forNode->get_loop_body(), preorder);
		return indxUseTraversal.getValidToTransform();
	}
	
	/*
	 * Returns method call name of an object 
	 */
	static std::pair<SgName,SgName> getMethodCall(SgNode* funcExp){
		// default initialises to ""
		std::pair<SgName,SgName> methodCall;
		printf("%s\n", funcExp->class_name().c_str());
		
		SgFunctionCallExp* rhsExp = isSgFunctionCallExp(funcExp);
		if(rhsExp == NULL){ return methodCall;}
			
		SgDotExp* dotExp =  isSgDotExp(rhsExp->get_function());
		if(dotExp == NULL){ return methodCall;}
		
		SgVarRefExp* lhsFuncContainer = isSgVarRefExp(dotExp->get_lhs_operand());			
		SgMemberFunctionRefExp* rhsFuncMethod = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
		
		if(lhsFuncContainer == NULL || rhsFuncMethod == NULL ){return methodCall;}
		// checking if function call does use the size call
		return std::make_pair(lhsFuncContainer->get_symbol()->get_name(),rhsFuncMethod->get_symbol()->get_name());
	}
	
	bool hasVaildSizeComparitor(SgStatement* testFor, SgName& container, const SgName& index){
		// not expecting overloaded operators and size call should always
		// return some integer value
		
		// can only have dotExp call not arrow, is the container allowed to be a pointer ? 
		
		// validate that index it left handside or right handside
		SgExprStatement* exprCompare = isSgExprStatement(testFor);
		if(exprCompare == NULL){return false;}
		
        // > case
		if(SgLessThanOp* lessOp = isSgLessThanOp(exprCompare->get_expression())){
			
			printf("CLASS LHS: %s \n", lessOp->get_lhs_operand()->class_name().c_str());
			printf("CLASS RHS: %s \n", lessOp->get_rhs_operand()->class_name().c_str());

			// remove casting
			SgNode* indexMaybeCast = lessOp->get_lhs_operand();
			if(SgCastExp* castExp = isSgCastExp(lessOp->get_lhs_operand())){
				indexMaybeCast = castExp->get_operand();
			}
	 
			SgVarRefExp* lhsIndex = isSgVarRefExp(indexMaybeCast);
			
			if(lhsIndex == NULL || lhsIndex->get_symbol()->get_name() != index){return false;}
			// check index matches 

			SgNode* funcExp = lessOp->get_rhs_operand();
			if(SgCastExp* castExp = isSgCastExp(lessOp->get_rhs_operand())){
				funcExp = castExp->get_operand();
			}
			
			std::pair<SgName,SgName> methodCall = getMethodCall(funcExp);
			
			printf("METHODCALL: %s, %s\n", methodCall.first.getString().c_str(),
				   methodCall.second.getString().c_str());
				
			if(methodCall.second == "size"){
				container = methodCall.first;
				printf("Success Less Than\n");
				return true;
			}else{
				printf("Fail Less Than\n");
				return false;
			}
			
		}// < case
		else if(SgGreaterThanOp* greaterOp = isSgGreaterThanOp((exprCompare->get_expression()))){

			printf("CLASS LHS: %s \n", greaterOp->get_lhs_operand()->class_name().c_str());
			printf("CLASS RHS: %s \n", greaterOp->get_rhs_operand()->class_name().c_str());
			
			SgNode* indexMaybeCast = greaterOp->get_rhs_operand();
			if(SgCastExp* castExp = isSgCastExp(greaterOp->get_rhs_operand())){
				indexMaybeCast = castExp->get_operand();
			}
			
			SgVarRefExp* rhsIndex = isSgVarRefExp(indexMaybeCast);
			if(rhsIndex == NULL || rhsIndex->get_symbol()->get_name() != index){return false;}
			// check index matches 

			SgNode* funcExp = greaterOp->get_lhs_operand();
			if(SgCastExp* castExp = isSgCastExp(greaterOp->get_lhs_operand())){
				funcExp = castExp->get_operand();
			}
			
			std::pair<SgName,SgName> methodCall = getMethodCall(funcExp);

			printf("METHODCALL: %s, %s\n", methodCall.first.getString().c_str(),
				   methodCall.second.getString().c_str());
			
			if(methodCall.second == "size"){
				container = methodCall.first;
				printf("Success Greater Than\n");
				return true;
			}else{
				printf("Fail Greater Than\n");
				return false;
			}
			
		}// != case (TRUE MAYBE)
		else if(SgNotEqualOp* notEqualOp = isSgNotEqualOp(exprCompare->get_expression())){
			// === remove casting === 
			SgNode* leftNode = notEqualOp->get_lhs_operand();
			SgNode* rightNode = notEqualOp->get_rhs_operand();
			
			if(SgCastExp* castExp = isSgCastExp(leftNode)){
			    leftNode = castExp->get_operand();
			}

			if(SgCastExp* castExp = isSgCastExp(rightNode)){
			    rightNode = castExp->get_operand();
			}

			SgVarRefExp* indexRef;
			SgNode* funcCall; 

			printf("CLASS LHS: %s \n", notEqualOp->get_lhs_operand()->class_name().c_str());
			printf("CLASS RHS: %s \n", notEqualOp->get_rhs_operand()->class_name().c_str());
			
			// check index in lhs
			if(isSgVarRefExp(leftNode)){
				indexRef = isSgVarRefExp(leftNode);
				funcCall = rightNode;
			}// check index in rhs
			else{
				indexRef = isSgVarRefExp(rightNode);
				funcCall = leftNode;
			}

			// indexRef matches index in initialiser
			if(indexRef == NULL || indexRef->get_symbol()->get_name() != index){return false;}
			
			// remove casting if there
			if(SgCastExp* castExp = isSgCastExp(funcCall)){
				funcCall = castExp->get_operand();
			}
			
			std::pair<SgName,SgName> methodCall = getMethodCall(funcCall);
			
			printf("METHODCALL: %s, %s\n", methodCall.first.getString().c_str(),
				   methodCall.second.getString().c_str());
				
			if(methodCall.second == "size"){
				container = methodCall.first;
				printf("Success NotEqual Than\n");
				return true;
			}else{
				printf("Fail NotEqual Than\n");
				return false;
			}
		}
		
		return false;
	}
	
	bool hasValidIndexInitialiser(SgForInitStatement* initFor, SgName& index){
		// get first varDec
		SgVariableDeclaration* varDecInit = isSgVariableDeclaration(initFor->get_init_stmt().at(0));
		if(varDecInit == NULL and varDecInit->get_variables().size() >= 1){return false;}

		SgInitializedName*  initName = varDecInit->get_variables().at(0);
		index = initName->get_name();// set index name

		SgAssignInitializer* assignInit =  isSgAssignInitializer(initName->get_initializer());
		if(assignInit == NULL){return false;}

		// handle casting of values expressions 
		SgNode* valNode = assignInit->get_operand();
		if(SgCastExp* castExp = isSgCastExp(assignInit->get_operand())){
			valNode = castExp->get_operand();
		}

		SgValueExp* val = isSgValueExp(valNode);
		
		if( !isSgDoubleVal(val) && !isSgFloatVal(val) && !isSgLongDoubleVal(val) ){
			printf("checking val\n");
			if(getIntegerConstantValue(val) == 0){
				printf("VALID: Initialiser\n");
				return true;
			}else{return false;}
		}
		else{
			return false;
		}
	}
	
	bool isArraySizeLoop(SgForStatement* forLoopNode, SgName& container, SgName& index ){

		SgForInitStatement* initFor = forLoopNode->get_for_init_stmt();
		SgStatement* testFor = forLoopNode->get_test();
		SgExpression* incrFor = forLoopNode->get_increment();
		
		// Ensure only single vardeclaration in init statement
		if(initFor->get_traversalSuccessorContainer().size() != 1){ return false;}
		
		printf("\tChecking Initializer: Array container\n"); // sets index
		if(!hasValidIndexInitialiser(initFor, index)){return false;};
		
		// DEBUG
		printf("Name of Index: %s\n", index.getString().c_str());
		
		printf("\tChecking comparitor: Array container\n");
		// sets container name 
		if(!hasVaildSizeComparitor(testFor, container, index) ){return false;}
		printf("Container Name: %s\n", container.getString().c_str());

		printf("\tChecking Increment: Array container\n");
		if(!hasVaildIncrement( incrFor, index)){return false;}
		
		// needs to be the same container being iterated (adding one may not work for all containers) 
		
		// should only be Variable declaration or assignment
		// get initialiser name 
		
		return true;
		
	}
	
// =========================================================================
public:

	void visit(SgNode *node){
		
		SgForStatement *loopNode  = isSgForStatement(node); // ignores ranged for loops
		
		SgName containerName;
		SgName iteratorName;
		
		if(loopNode != NULL && !isCompilerGenerated(loopNode)){
			printf("=== FOR loop Start ===\n");

			if( isIteratorLoop(loopNode, containerName, iteratorName)
				&& safeIteratorForTransform(loopNode, containerName, iteratorName)){
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
			else if(isArraySizeLoop(loopNode, containerName, iteratorName)
					&& safeIndexForTransform(loopNode, containerName, iteratorName) ){
				
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
	ForLoopTraversal traversal;
	traversal.traverseInputFiles(project, preorder);
	//AstTests::runAllTests(project) ;
    return backend(project);
}
