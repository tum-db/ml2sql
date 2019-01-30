/**
 * @file MyMLVisitor.cpp
 * @author Matthias Bungeroth
 * @brief File contains Implementation of MyMLVisitor
 *
 */

#include "variable/Interval.h"
#include "variable/Accessor.h"
#include "visitor/MyMLVisitor.h"


MyMLVisitor::~MyMLVisitor(){
	this->MLBaseVisitor::~MLBaseVisitor();
	delete cg;
}


///////////////////////////////////////////////////////////////////////
//helper

 

void MyMLVisitor::reportError(MLParser::ExpressionContext *  t , std::string err ){
	errorLog << "Error" << " in line: " << t->getStart()->getLine() << " to " << t->getStop()->getLine() << ": " << err << "!" << endl;
}


/**
 * Resets the Generator.
 * This mehtod resets the Generator to a new codegenerator of the desired target language.
 */
void MyMLVisitor::resetGenerator(){
	switch(targetType){
	case MLParser::POSTGRES:
		cg = new CodeGenerator::PostgresGenerator();
		break;
	case MLParser::PYTHON:
		cg = new CodeGenerator::PythonGenerator();
		break;
	case MLParser::HYPER:
		cg = new CodeGenerator::HyperGenerator();
		break;
    default:
        //This will not happen
        break;
	}
}

/**
 * Creates a vector of strings of the text of tokens.
 * @param vec Vector of tokens
 * @return A vector of strings
 */
vector<string>* MyMLVisitor::tokenToString(vector<antlr4::Token*> &vec){
	vector<string>* ls = new vector<string>();
	for(antlr4::Token * t : vec){
		ls->push_back(t->getText());
	}
	return ls;
}

/**
 * Creates a vector of Variables by visiting all MathexpContexts in the input vector.
 * @param vec Vector of MathexpContext
 * @return A vector of Variables
 */
vector<Variable*>* MyMLVisitor::mathexpToVar(vector<MLParser::MathexpContext*> &vec){
	vector<Variable*>* ls = new vector<Variable*>();
	for(MLParser::MathexpContext * t : vec){
		ls->push_back(visit(t));
	}
	return ls;
}

/**
 * Registers a functions parameter Types.
 * This function registers the function parameter types in order to generate the function definition later on.
 * This function may also change the name of the function if its name in the targen language is different.
 * @param ft The function type of the function consisting of name and number of arguments (which is the unique identifier)
 * @param fname Name of the function
 * @param parmlist List of example parameters to gather the type from
 * @return The name of the actual function to use
 */
string MyMLVisitor::registerFunction(FunctionType ft , string fname , vector<Variable*>* parmlist){


    //Check if function was defined extern if yes change the name
    if(functionToTargetName.find(ft) != functionToTargetName.end()){
        fname = functionToTargetName[ft];
        ft = FunctionType(fname,ft.second);
    }

    //Get the types for defining the function later on
    if(functionToParmTypes.find(ft) == functionToParmTypes.end()){
        vector<TypeDim>* types = new vector<TypeDim>();
        for(Variable* v : *parmlist){
            types->push_back(v->getTypeDim());
        }
        functionToParmTypes[ft] = types;
    }

    return fname;
}

///////////////////////////////////////////////////////////////////////
//file


/**
 * Visit file is the entry point to the syntax tree. It is always the root node of the syntax tree.
 * It will visit all expressions and concatenate them in a string.
 * After evaluating all expressions it will create all function definitions and prepend them to the expressions.
 * To generate the functions, the visitor will create a new codegenerator for each function in order to forget about all
 * already defined variables.
 * @param ctx The root node of the syntax tree
 * @return A string consisting of the target language source code.
 * @returns Type String.
 */
antlrcpp::Any MyMLVisitor::visitFile_compile(MLParser::File_compileContext *ctx) {
	resetGenerator();

	try{
		string exps = visit(ctx->expressionlist);
		//remove tabs in front
		stringstream res;
		res << cg->getInit().rdbuf() << endl << endl ;
		CodeGenerator::Generator* oldCg = cg;
		list<string> functions;
		//Iterating backwords because the first use of a function could be in a function
		for (auto fctx=functionsToDefine.rbegin(); fctx!=functionsToDefine.rend(); ++fctx) {
            Variable *v = defineFunction(*fctx);
            if (v->getType() != Variable::NONE) {
                functions.push_front(v->getContext());
                delete cg;
            }
            delete v;
        }
        for(string &s : functions){
            res << s << endl << endl;
        }
		cg = oldCg;


		res << cg->getStartMain().rdbuf();
		res << cg->declareVarsAndBegin().rdbuf();
		res << exps << endl << endl;
		res << cg->getCleanup().rdbuf() << endl;
		cout << "\x1b[32m";  // Set color to green
		cout << "Compiling completed successfully!";
		cout << "\x1b[0m" << endl; // Restore defaults!
		return res.str();


	}catch( std::string &err){
        cout << "\x1b[31m"; // Set color to red
        cout << "Errors in pre file:" << endl << endl;
        cout << err << endl;
        cout << "\x1b[0m"; // Restore defaults!
		return  std::string("");
	}

}


/**
 * Visits a extern function definition in the ml language.
 * @param ctx The function context node
 * @return Returs always zero because we have to return something even if we do not need to (here).
 * @returns Type Int.
 */
antlrcpp::Any MyMLVisitor::visitFunctions(MLParser::FunctionsContext *ctx){
	vector<TypeDim>* retTypes = new vector<TypeDim>;
	//Map Tokentype to variable type!
	for(MLParser::ReturnTypeContext * t: ctx->ret){
		TypeDim td = visit(t);
		retTypes->push_back(td);
	}
	FunctionType ft(ctx->fname->getText() , std::stoi(ctx->parmnum->getText()));
	string targetName = ft.first;

	for(size_t i = 0; i < ctx->targets.size(); i++){
		if(cg->getType() == ctx->targets[i]->getType()){
			targetName = ctx->names[i]->getText();
			break; //No need to complete the loop
		}
	}

	functionToReturnTypes[ft] = retTypes;
	functionToTargetName[ft] = targetName;
	return 0;
}


///////////////////////////////////////////////////////////////////////
//explist


/**
 * Visits a list of expressions and concatenates the the evaluated expressions as a line seperated string.
 * @param ctx The expressionlist context node
 * @return The String of the evaluated string.
 * @returns Type String.
 */
antlrcpp::Any MyMLVisitor::visitExplist(MLParser::ExplistContext *ctx){
	stringstream ss;
	string newLine;
	for(auto exp : ctx->expressions ){
		try{
			Variable* v =  visit(exp);
			if(v->getType() == Variable::INTEGER || v->getType() == Variable::BOOL
			   || v->getType() == Variable::FLOAT || v->getType() == Variable::FLOAT ){
				throw ERROR("Math-expressions can not stand alone");
			}
			string context = v->getContext();

			if(v->getType() != Variable::NONE){
				ss << newLine;
				if(v->getType() != Variable::INJECTED){
					context = std::regex_replace(context , std::regex("\n") , "\n" + ML_TAB);
					ss << ML_TAB;
				}
				ss << context;
				newLine = "\n";
			}

			delete v;
		}catch(std::string err){
			reportError( exp , err);
		}


	}

	string err = errorLog.str();
	if(!err.empty()){
		throw err;
	}

	return ss.str();

}

///////////////////////////////////////////////////////////////////////
//expression

/**
 * Removes quotation marks in the beging and the end of the string.
 * @param s a string consisting of ' in the beginig and the end.
 * @return The sub-stringed string.
 */
string removeQuotationMarks(string s){
	return  s.substr(1 , s.size() -2 );
}


/**
 * Injects a string that is already written in the target language. It cuts of "inject""" in the bgeinnnig and """" in the end.
 * @param ctx the inject context node
 * @return A Variable consisting of the injected code of type Variable::INJECTED
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_inject(MLParser::Expression_injectContext *ctx){
	string inj = ctx->getText();
	return new Variable(inj.substr(8,inj.length() -10) ,  Variable::INJECTED);
}


/**
 * Creates a Tensor by calling the Codegenerator.
 * @param ctx the create tensor context node
 * @return A Variable consisting of the created tensor.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_createTensor(MLParser::Expression_createTensorContext *ctx) {
	string newName = ctx->newName->getText();
	string oldName = ctx->oldName->getText();
	vector<string>* columns = visit(ctx->columns);
	Variable* res = cg->generateCreateTensorFromDB(newName , oldName ,columns);

	//Cleanup
	delete columns;
	return res;
}

/**
 * Defines a Tensor by calling the code generator.
 * @param ctx the define tensor context node
 * @return A Variable consisting of the created tensor.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_defineTensor(MLParser::Expression_defineTensorContext *ctx){
    string tenName = ctx->name->getText();
	vector<Accessor*>* acc = visit(ctx->list);
    Variable * v = visit(ctx->value);
    Variable* res = cg->generateDefineTensor(tenName , acc , v);

    //Cleanup
    delete acc;
    delete  v;
    return res;
}

/**
 * Creates a Tensor by calling the Codegenerator and fills it with random values sampled from the desired distribution.
 * The Tensor will be created by calling generateDefineTensor. After that every value in the tensor will be reset by
 * iterating over it and calling the sample method of the distribution.
 * @param ctx the sample tensor context node
 * @return A Variable consisting of the created tensor.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_sampleTensor(MLParser::Expression_sampleTensorContext *ctx){

    //Define tensor
    string tenName = ctx->name->getText();
    vector<Accessor*>* acc = visit(ctx->list);
    Variable * v = new Variable("0.0" , Variable::FLOAT , 0);
    Variable* defineTensor = cg->generateDefineTensor(tenName , acc , v);


	//Access Tensor for iterating over it
	string forVarName = "ML_DIST_";
	char dim = 'a';
	vector<Accessor*>* setNewTensor = new vector<Accessor*>();
	for(size_t i = 0; i < acc->size(); i++){
		string accesor = forVarName + dim;
		setNewTensor->push_back(new Accessor(new Variable(accesor , Variable::INTEGER)));
        dim++;
	}

	//Call the sample function
	string dname = ctx->dname->getText();
	if(distributionMap.find(dname) == distributionMap.end()){
		throw ERROR("Distribution " +  dname + " not defined");
	}

	string fname = distributionMap[dname].first; // Get the name of the sample function

	vector<Variable*>* parmlist = mathexpToVar(ctx->parms);
	FunctionType ft(fname  , parmlist->size());
	fname = registerFunction(ft , fname , parmlist);
	string fcall = fname + "(";
	fcall += cg->generateList(parmlist);
	fcall += ')';

	Variable* accesedTen = cg->generateSetAccesedTensor(tenName , setNewTensor , new Variable(fcall, Variable::FLOAT ,acc->size() ));

    //Build for loop to actually iterate over the tensor
    string forLoop = accesedTen->getContext();
	dim = 'a';
	//Build for loops around set expression.
	for(size_t i = 0; i < setNewTensor->size(); i++){

		Variable* tmp = cg->generateFor(forVarName + dim , new Variable("0" , Variable::INTEGER) , (*acc)[i]->getFrom() , forLoop);
		forLoop = tmp->getContext();
        forLoop = std::regex_replace(forLoop , std::regex("\n") , "\n"+ ML_TAB); // Add indenteing! This is a must have for Python!
        dim++;
		delete tmp;
	}

    //Build result string

    string res = defineTensor->getContext();
    res += '\n';
	res += forLoop;

    //Cleanup
    delete acc;
    delete v;
    delete defineTensor;
    delete setNewTensor;
    delete accesedTen;
    delete parmlist;
    return new Variable(res , Variable::VOID);
}


/**
 * Calls the Codegenerator to generate code that reads in a CSV file.
 * @param ctx the read csv context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_readcsv(MLParser::Expression_readcsvContext *ctx){
    if(!ctx->filename) throw ERROR("filename is not specified");
    if(!ctx->outname) throw ERROR("name is not specified");
    if(!ctx->columns) throw ERROR("columns is not specified");
	string csvname = ctx->outname->getText();
	string filename = removeQuotationMarks(ctx->filename->getText());
	vector<string>* cols= visit(ctx->columns);
    string del = ",";
    Variable* replaceEmpty = nullptr;
    bool delteEmpty = true;
    if(ctx->delimiter){
        del = ctx->delimiter->getText();
    }
    if(ctx->repEmptyBy){
        delteEmpty = false;
        replaceEmpty = visit(ctx->repEmptyBy);
    }
	vector<pair<string,string>>* replaceList = new vector<pair<string,string>>();
    for(size_t i = 0; i < ctx->rep.size(); i++){
        replaceList->push_back(make_pair(removeQuotationMarks(ctx->rep[i]->getText()),
										 removeQuotationMarks(ctx->by[i]->getText())));
    }

    Variable* res = cg->generateReadCsv( csvname , filename , cols , del , delteEmpty , replaceEmpty , replaceList , csv , useModified);

    //Cleanup
    delete cols;
    delete replaceList;
    delete replaceEmpty;
    return res;
}

/**
 * Calls the Codegenerator to generate code to set a variable.
 * @param ctx the set var context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_setVar(MLParser::Expression_setVarContext *ctx) {
	string newName = ctx->left->getText();
	Variable* r = visit(ctx->right);
	Variable* res = cg->generateSetVar(newName  , r , true);

	//Cleanup
	delete r;
	return res;
}


/**
 * Calls the Codegenerator to generate code to save a tensor.
 * @param ctx the set var context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_saveTensor(MLParser::Expression_saveTensorContext *ctx){
	string tenName = ctx->tenName->getText();
	string dbName = ctx->dbName->getText();
	vector<string>* columns = visit(ctx->columns);
	Variable* res;
	if(ctx->dbName->getType() == MLParser::VARNAME){
		//save to daML_TABase
		res =  cg->generateSaveTensorToDB(tenName , dbName ,columns);
	}else{
		string del = "','";
		dbName =removeQuotationMarks(dbName);
		if(ctx->del){
			del = ctx->del->getText();
		}
		//save to csv file!
		res =  cg->generateSaveTensorToCSV(tenName , dbName,columns, del);
	}

	//Cleanup
	delete columns;
	return res;
}



/**
 * Calls the Codegenerator to generate a if statement.
 * @param ctx the if context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_if(MLParser::Expression_ifContext *ctx) {
	Variable* condition = visit(ctx->condition);
	string ifexps= visit(ctx->ifExpressionList);
	string elseexps;
	//There exists an else expressions
	if(ctx->children.size() > 7){
	   string s = visit(ctx->elseExpressionList); 
	   elseexps = s;
	}
	Variable* res = cg->generateIfElse(condition , ifexps , elseexps);

	//Cleanup
	delete condition;
	return res;
}

/**
 * Calls the Codegenerator to generate a while statement.
 * @param ctx the while context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_while(MLParser::Expression_whileContext *ctx) {
	Variable* condition = visit(ctx->condition);
	string ifexps = visit(ctx->expressionList);
	Variable* res = cg->generateWhile(condition , ifexps);

	// Cleanup
	delete condition;
	return res;
}

/**
 * Calls the Codegenerator to generate a for statement.
 * @param ctx the for context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_for(MLParser::Expression_forContext *ctx) {
	string name = ctx->name->getText();
	Variable* from = visit(ctx->from);
	Variable* to = visit(ctx->to);
	cg->setVarToType(name , TypeDim(Variable::INTEGER,0) );
	string ifexps = visit(ctx->expressionList);
	cg->removeVar(name);
	Variable* res = cg->generateFor(name ,from , to , ifexps);

	//Cleanup
	delete from;
	delete to;
	return res;
}


/**
 * Calls the Codegenerator to generate a for statement iterating over an interval.
 * @param ctx the if context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_for_interval(MLParser::Expression_for_intervalContext *ctx) {
    string varname = ctx->name->getText();
    Interval* interval = visit(ctx->values);
	string res;


	if(interval->isListInterval()){
		//Interval has the from [1,2,3,4,5]
		ListInterval* li = static_cast<ListInterval*>(interval);
        string arrayName = "ML_FOR_ARRAY_" + varname;
        string indexName = "ML_FOR_INDEX_" + varname;

        //Generate the array to iterate over
		Variable* array = cg->generateSetVar( arrayName , li->getList(), true);

		//The accessor we need to access the tensor
		vector<Accessor*>* accsessors = new vector<Accessor*>();
		accsessors->push_back(new Accessor(new Variable(indexName , Variable::INTEGER)));
		Variable* access = cg->generateAccsessTensor(cg->generateVariable(arrayName), accsessors );
		// varname = array[accsessors]
		Variable* var = cg->generateSetVar( varname , access , true );

		string forexps = ML_TAB;
		forexps += var->getContext();
		forexps += '\n';;
		string exps = visit(ctx->expressionList);
		forexps += exps;
		//Create the for loop
		Variable * forloop = cg->generateFor( indexName ,new Variable("1" , Variable::INTEGER) , new Variable( "array_length(  " + arrayName  +",1)" , Variable::INTEGER) , forexps);

		res = array->getContext();
		res += '\n';
		res += forloop->getContext();

		//Cleanup

		delete array;
		delete accsessors;
		delete access;
		delete var;
		delete forloop;
	}else{
		//Interval has the from [1;100]:4
		NumInterval* ni = static_cast<NumInterval*>(interval);
		Variable* fvar;
		Variable* setVar;

		Variable * from = ni->getFrom();
		//make sure variable has correct type if step or from is float make from to type float!
		if(ni->getStep()->getType() == Variable::FLOAT){
            from = new Variable(from->getContext(), Variable::FLOAT , from->getDim());
		}else{
			from = new Variable(from->getContext()  , from->getTypeDim());
		}

        //add functioncall if steps are scaled by function
		if(!ni->getFname().empty()){
			string fvarname = varname; 
			varname = "ML_FOR_" + varname; // Another variable is needed (to be the actual run variable)

			setVar = cg->generateSetVar(varname , from, true); // ML_FOR_<varname> = from
			vector<Variable*>* parms = new vector<Variable*>();
			vector<string>* rets = new vector<string>();
			vector<TypeDim>* tds = new vector<TypeDim>();

			parms->push_back(cg->generateVariable(varname));
			rets->push_back(fvarname);
			tds->push_back(TypeDim(Variable::FLOAT , 0));

			fvar = cg->generateFuncCall(ni->getFname(),parms,rets,tds); // fvarname = function(ML_FOR_<varname>)  Call the scaling function

			//Cleanup
			delete parms;
			delete rets;
			delete tds;
		}else{
			setVar = cg->generateSetVar(varname , from, true); // varname = from // initzilaistation before loop
			fvar = new Variable("" , Variable::VOID);
		}
        Variable* add = cg->generateAdd(cg->generateVariable(varname) ,ni->getStep()  ); // varname + step
        Variable* updateVar = cg->generateSetVar(varname , add , true); // varname = varname + step
		
		string exps = ML_TAB + fvar->getContext();
		string resExps = visit(ctx->expressionList);
		exps += '\n';
		exps += resExps;   // add normal expressions from expression list!
		exps += "\n" + ML_TAB;
        exps += updateVar->getContext(); // the var updating in the end

        // Create While condtion
		string condition = varname;
        condition += " <= ";
        condition += ni->getTo()->getContext();

        //Generate While loop
		Variable* wh = cg->generateWhile(new Variable(condition, Variable::BOOL) , exps);
        res = setVar->getContext();
		res += '\n';
        res +=  wh->getContext();

        //Cleanup
        delete fvar;
        delete setVar;
        delete from;
        delete add;
        delete updateVar;
		delete wh;
	}
	delete interval;
    return new Variable(res, Variable::VOID);
}


/**
 * Calls the Codegenerator to generate a function call and registres the function to memorize the parameter types.
 * @see registerFunction
 * @param ctx the function call context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_functionCall(MLParser::Expression_functionCallContext *ctx) {
	string fname = ctx->funcName->getText();
	if(distributionMap.find(fname) != distributionMap.end() ){
	    fname = distributionMap[fname].second;
	}
	//Parameters to pass?
	vector<Variable*>* parmlist = mathexpToVar(ctx->parms);
	FunctionType ft(fname  , parmlist->size());
	fname = registerFunction(ft , fname , parmlist);
	//Save return somewhere?
	//Due to grammar structurue return list size must be 0 or >1 (case =1 is handled in matexp)
	vector<string>* returnList = tokenToString(ctx->res);
	vector<TypeDim>* returnTypes = functionToReturnTypes[ft]; // get return types
	Variable* res = cg->generateFuncCall(fname , parmlist , returnList, returnTypes);

	//Cleanup
	delete parmlist;
	delete returnList;
	//Do NOT delete returnTypes
	return res;
}

/**
 * Remembers a function definition for later definition of the function
 * @see functionsToDefine
 * @param ctx the function definition context node
 * @return A empty Variable of type Variable::NONE.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_functionDef(MLParser::Expression_functionDefContext *ctx) {
	//Add to map to define function later!
	functionsToDefine.push_back(ctx);

	vector<TypeDim>* retTypes = new vector<TypeDim>;
	//Map Tokentype to variable type!
	for(MLParser::ReturnTypeContext * t: ctx->ret){
		TypeDim td = visit(t);
		retTypes->push_back(td);
	}
	//Save return types if function is called!
	string fname = ctx->funcName->getText();
	FunctionType ft(fname  , ctx->parms.size());
	functionToReturnTypes[ft] = retTypes;
	//No cleanup needed!
	return new Variable("" , Variable::NONE);
}


/**
 * Calls the code generator to actually generate the code of the function definition
 * @param ctx the function definition context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
Variable* MyMLVisitor::defineFunction(MLParser::Expression_functionDefContext* ctx){

	string fname = ctx->funcName->getText();
	vector<string>* parmlist = tokenToString(ctx->parms);
	FunctionType ft(fname , parmlist->size());
	if(functionToParmTypes.find(ft) == functionToParmTypes.end()){
		// Function was not used so we can not define it because we donth know the parmtypes
		return new Variable("", Variable::NONE);
	}
	vector<TypeDim>* parmtypes  = functionToParmTypes[ft]; // get the type of a function
	resetGenerator(); // reset generator to allow shadowing add set a new context
	for(size_t i = 0; i < parmlist->size(); i++){
		string s = (*parmlist)[i];
		TypeDim type = (*parmtypes)[i];
		cg->setVarToType(s ,type );
	}
	string exps = visit(ctx->expressionList);
	Variable* res = cg->generateDefineFunction(fname , parmlist , parmtypes , functionToReturnTypes[ft] , exps);

	//Cleanup
	delete parmlist;
	delete parmtypes;
	return res;
}


/**
 * Calls the code generator to generate a return statement
 * @param ctx the return context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_return(MLParser::Expression_returnContext *ctx){
	vector<Variable*>* returnList = mathexpToVar(ctx->ret);
	return cg->generateReturn(returnList);
}

/**
 * Calls the code generator to generate either break or continue.
 * @param ctx the keyword context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_keyword(MLParser::Expression_keywordContext *ctx){
	return new Variable(ctx->word->getText(), Variable::VOID);
}


/**
 * Calls the code generator to generate code that sets an accesed tensor.
 * Example: A[0] = 10
 * @param ctx the keyword context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_setAccessedTensor(MLParser::Expression_setAccessedTensorContext *ctx){
	string tenName = ctx->left->getText();
	vector<Accessor*>* acc = visit(ctx->list);
	Variable * toSet = visit(ctx->rigth);
	Variable* res = cg->generateSetAccesedTensor(tenName , acc , toSet);

	//Cleanup
	delete acc;
	delete toSet;
	return res;
}


/**
 * Calls visit to visit the extern function
 * @param ctx the extern function context node
 * @return An empyt Variable of typer Variable::NONE
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_externFunction(MLParser::Expression_externFunctionContext *ctx){
	visit(ctx->fun);
	return new Variable("" , Variable::NONE);
}


/**
 * Calls the code generator to generate code for printing.
 * @param ctx the print context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_print(MLParser::Expression_printContext *ctx){
	vector<Variable*>* list = visit(ctx->toPrint);
	return cg->generatePrint(list);
}


///////////////////////////////////////////////////////////////////////
//interval

/**
 * Visits an interval context node.
 * @see Interval, NumInterval, ListInterval
 * @param ctx The Interval context node
 * @return A interval consisting of the infromation proviede in the context node
 * @returns Type NumInterval or ListInterval
 */
antlrcpp::Any MyMLVisitor::visitInterval(MLParser::IntervalContext *ctx){
	if(ctx->children.size() == 1){
		Variable* s = visit(ctx->ten);
		return static_cast<Interval*>(new ListInterval(s));
	}
	string fname;
	Variable* step = nullptr;
	Variable* from;
	Variable* to;
	switch(ctx->children.size()){
		//No break needed!
		case 9: // Function name
			fname = ctx->fname->getText();
		case 7: //Step length
			step = visit(ctx->num);
	    default: /*case 5:*/ //Interval
			from = visit(ctx->from);
			to = visit(ctx->to);
	}
	if(!step){
		step = new Variable("1" , Variable::INTEGER);
	}
    return static_cast<Interval*>(new NumInterval(from , to , step , fname));
}


/**
 * Visits a gradient descent context node and calls the codegenerator to generate the code for gradient descent.
 * @param ctx The gradient descent context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_gradientDescent(MLParser::Expression_gradientDescentContext *ctx){
    if(!ctx->fun) throw ERROR("function is not specified");
    if(!ctx->data) throw ERROR("data is not specified");
    if(!ctx->opt.size()) throw ERROR("optimize is not specified");

    string fun = removeQuotationMarks(ctx->fun->getText());
	vector<string>* data = visit(ctx->data);
	vector<NameShape>* opt = new vector<NameShape>();
	for(auto nsc : ctx->opt){
		NameShape ns = visit(nsc);
		opt->emplace_back(ns);
	}
	Variable* stepSize = nullptr;
	Variable* maxSteps = nullptr;
	Variable* batchSize = nullptr;
    Variable* threshold = nullptr;
	if(ctx->stepSize){
		stepSize = visit(ctx->stepSize);
	}
	if(ctx->maxSteps){
		maxSteps = visit(ctx->maxSteps);
	}
	if(ctx->batchSize){
		batchSize = visit(ctx->batchSize);
	}
	if(ctx->threshold){
        threshold = visit(ctx->threshold);
	}
    Variable* res = cg->generateGradientDescent(fun , data ,opt  , stepSize , maxSteps , batchSize ,threshold);

	//Cleanup
	delete  data;
    //This is ok sice deleating null pointers has no effect!
	delete stepSize;
    delete maxSteps;
    delete batchSize;
    delete threshold;

    return res;
}


/**
 * Visits an plot context node and calls the codegenerator to generate the code to plot.
 * @param ctx The plot context node
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_plot(MLParser::Expression_plotContext *ctx){

    if(!ctx->filename) throw ERROR("filename is not specified");
    if(!ctx->x) throw ERROR("xData is not specified");
    if(!ctx->y) throw ERROR("yData is not specified");
	//Checks if values are present. If not the it will set the default values.
    string xLable = ctx->xLable ? ctx->xLable->getText():"'x'";
    string yLable = ctx->yLable ? ctx->yLable->getText():"'y'";
    string type = ctx->type ? ctx->type->getText():"'b-'";
    string fileName = ctx->filename->getText();
    Variable* xData = visit(ctx->x);
    Variable* yData = visit(ctx->y);
    Variable* res = cg->generatePlot(fileName , xData , yData  , xLable , yLable , type);

    //cleanup
    delete xData;
    delete yData;

    return res;
}

///////////////////////////////////////////////////////////////////////
//accsesslist

/**
 * Visits an access list context and creates a vector of accessors out of it.
 * @see Accessor
 * @param ctx The access list context node
 * @return A vector contating of Accessor objects
 * @returns Type vector<Accessor*>*
 */
antlrcpp::Any MyMLVisitor::visitAccsesslist(MLParser::AccsesslistContext *ctx) {
	vector<Accessor*>* res = new vector<Accessor*>();
	for(MLParser::AccessorContext * ac : ctx->expressions){
		Accessor* acret = visit(ac);
		res->push_back(acret);
	}
	return res;
}


///////////////////////////////////////////////////////////////////////

/**
 * Visits an return type node of a function definition and creates a TypeDim type out of it.
 * @see TypeDim
 * @param ctx The return type context node
 * @return A TypeDim object holding the corresponding type and dimension.
 * @returns Type TypeDim
 */
antlrcpp::Any MyMLVisitor::visitReturnType(MLParser::ReturnTypeContext *ctx){
	int type;
	string sdim = ctx->type->getText();
	switch(ctx->type->getType()){
		case MLParser::INTT: sdim = sdim.substr(3); break;
		case MLParser::BOOLT: sdim = sdim.substr(4); break;
		case MLParser::FLOATT: sdim = sdim.substr(5); break;
		default: sdim = "0";
	}
	int dim;
	if(sdim.length() == 0){
		dim = 0;
	}else{
		dim = std::stoi(sdim);
	}
	switch(ctx->type->getType()){
		case MLParser::INTT: type=Variable::INTEGER; break;
		case MLParser::BOOLT: type=Variable::BOOL; break;
		case MLParser::FLOATT: type=Variable::FLOAT; break;
		default: type = Variable::NONE;
	}
	return TypeDim(type,dim);
}

///////////////////////////////////////////////////////////////////////
//Mathexp

/**
 * Calls the codegenerator to transpose a matrix.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_transpose(MLParser::Mathexp_transposeContext *ctx){
	Variable* l = visit(ctx->left);
	Variable* res = cg->generateTranspose(l);

	//Cleanup
	delete l;
	return res;
}

/**
 * Calls the codegenerator to multiply two math expressions.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_mul(MLParser::Mathexp_mulContext *ctx) {
	Variable* l = visit(ctx->left);
	Variable* r = visit(ctx->right);
	Variable* res;
	if(ctx->op->getType() == MLParser::MUL){
		res =  cg->generateMul(l, r);
	}else{  //if(ctx->op->getType() == MLParser::DIV)
		res =  cg->generateDiv(l, r);
	}

	//Cleanup
	delete l;
	delete r;
	return res;
}

/**
 * Calls the codegenerator to add two math expressions.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_add(MLParser::Mathexp_addContext *ctx) {
	Variable* l = visit(ctx->left);
	Variable* r = visit(ctx->right);
	Variable* res;
	if(ctx->op->getType() == MLParser::ADD){
		res = cg->generateAdd(l, r);
	}else{  //if(ctx->op->getType() == MLParser::DIV)
		res = cg->generateSub(l, r);
	}

	//Cleanup
	delete l;
	delete r;
	return res;
}

/**
 * Calls the codegenerator to nagete a math expressions.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_neg(MLParser::Mathexp_negContext *ctx) {
	Variable* r = visit(ctx->right); 
	return cg->generateNeg( r);
}

/**
 * Calls the codegenerator to take a math expression to the power of a math expression
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_pow(MLParser::Mathexp_powContext *ctx){
	Variable* l = visit(ctx->left);
	Variable* r = visit(ctx->right);
	return cg->generatePow(l , r);
}

/**
 * Generates braces around a math expression.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_brace(MLParser::Mathexp_braceContext *ctx){
	Variable* res = visit(ctx->expr) ;
	res->setContext("(" + res->getContext() + ")");
	return res;
}

/**
 * Calls the codegenerator to get a Variable object of a variable.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_var(MLParser::Mathexp_varContext *ctx) {
	string name = ctx->var->getText();
	Variable* res = cg->generateVariable(name);
	return res;
}

/**
 * Calls the codegenerator to generate a tensor from a list.
 * Example: [[1,2],[3,4]]
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_tensor(MLParser::Mathexp_tensorContext *ctx){
	vector<Variable*>* ls = visit(ctx->list);
	stringstream ss;
	char sep = ' ';
	int dim = -1;
	int type = Variable::INTEGER;
	for(Variable * t : *ls){
		ss << sep << t->getContext();
		int ndim = t->getDim();
		if(dim != -1 && dim != ndim){
            throw ERROR("Dimension of explicitly defined tensor does not match!");
		}
		dim = ndim;
		if(t->getType() == Variable::FLOAT){
			type = t->getType();
		}
		sep = ',';
	}
	Variable* res = cg->generateTensorFromList(ss.str() , type , dim );

	//Cleanup
	delete ls;
	return res;
}

/**
 * Calls the codegenerator to access a tensor.
 * Example: A[i,j]
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_accsesstensor(MLParser::Mathexp_accsesstensorContext *ctx){
	vector<Accessor*>* s = visit(ctx->acc);
	Variable* v = visit(ctx->ten);
	Variable* res = cg->generateAccsessTensor(v, s);

	//Cleanup
	delete s;
	delete v;
	return res;
}

/**
 * Generates a function call.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_functionCall(MLParser::Mathexp_functionCallContext *ctx){
	vector<Variable*>* parmlist = mathexpToVar(ctx->parms);
	string fname = ctx->funcName->getText();
	FunctionType ft(fname , ctx->parms.size());
	if(functionToReturnTypes.find(ft) == functionToReturnTypes.end()){
		throw ERROR("Function " + fname + " is not defined");
	}
	vector<TypeDim>* returnTypes = functionToReturnTypes[ft];
	fname = registerFunction(ft , fname , parmlist);
	if(returnTypes->size() != 1){
		throw ERROR("Can not use functions with more than one return as mathexpression");
	}
	stringstream ss;
	ss << fname << "(";
	ss << cg->generateList(parmlist);
	ss << ")";

	//Cleanup
	delete parmlist;
    //Do NOT delete returnTypes
	return new Variable( ss.str(), (*returnTypes)[0]);
}

/**
 * Generates a const number.
 * @return A Variable consisting of the generated code.
 *
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_num(MLParser::Mathexp_numContext *ctx) {
	int type =  ctx->num->getType()== MLParser::INTEGER ? Variable::INTEGER : Variable::FLOAT;
	return cg->generateNum(ctx->getText() , type);
}


/**
 * Calls the codegenerator to generate "and", "or", and "xor".
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_boolop(MLParser::Mathexp_boolopContext *ctx) {
	Variable* l = visit(ctx->left);
	Variable* r = visit(ctx->right);
	Variable* res;
	if(l->getType() != Variable::BOOL || r->getType() != Variable::BOOL){
		throw ERROR("Boolean operations can be only used on booleans");
	}
	switch(ctx->op->getType()){
		case MLParser::AND:
			res = cg->generateAnd(l , r); break;
		case MLParser::OR:
			res = cg->generateOr(l , r); break;
		case MLParser::XOR:
			res = cg->generateXor(l , r); break;
        default:
			throw ERROR("You messed up the grammar!");
            break;
	}

	//Cleanup
	delete l;
	delete r;
	return res;
	
}

/**
 * Generates a const boolean.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_bool(MLParser::Mathexp_boolContext *ctx) {
	return cg->generateBoolean(ctx->op->getType() == MLParser::TRUE);
}


/**
 * Generates a negation of a boolean.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_boolNot(MLParser::Mathexp_boolNotContext *ctx){
	Variable* v = visit(ctx->left);
	v->setContext("not " + v->getContext());
	return v;
}

/**
 * Calls codegenerator to append to a tensor.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_append(MLParser::Mathexp_appendContext *ctx){
	Variable* l = visit(ctx->left);
	Variable* r = visit(ctx->right);
	Variable* res = cg->generateAppend(l,r);

	//Cleanup
	delete l;
	delete r;
	return res;
}


/**
 * Generates comparsions like "==" , "<" and "<=".
 * Could be moved to code geneartor.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_compare(MLParser::Mathexp_compareContext *ctx) {
	Variable* l = visit(ctx->left);
	Variable* r = visit(ctx->right);
	stringstream ss;
	string op;
	if(ctx->op->getType() == MLParser::EQUALSEQUALS){
		op = cg->generateComparisonEquals();
	}else{
		op = ctx->op->getText();
	}
	ss << l->getContext() << " " << op << " "<< r->getContext();
	delete l;
	delete r;
	return new Variable(ss.str() , Variable::BOOL);
}

/**
 * Generates a Variable of type string
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitMathexp_string(MLParser::Mathexp_stringContext *ctx){
	return new Variable(ctx->getText() , Variable::STRING);
}

///////////////////////////////////////////////////////////////////////
//distribution

/**
 * Adds sample and probability function to the distributionMap
 * @see distributionMap
 * @return An empty Variable of type Variable::NONE.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_setDist(MLParser::Expression_setDistContext *ctx){
	distributionMap.insert(make_pair(ctx->var->getText() , make_pair(ctx->sample->getText() , ctx->pr->getText())));
	return new Variable("", Variable::NONE);
}

/**
 * Generates code to samples a number from the given distribution.
 * @return A Variable consisting of the generated code.
 * @returns Type Variable
 */
antlrcpp::Any MyMLVisitor::visitExpression_sampleDist(MLParser::Expression_sampleDistContext *ctx){
	string fname = distributionMap[ctx->dname->getText()].first;

	vector<Variable*>* parmlist = mathexpToVar(ctx->parms);
	//parmlist->push_back(cg->generateRandom());
	vector<string>* returnList = new vector<string>(1);
	vector<TypeDim>* returnTypes = new vector<TypeDim>(1);
	(*returnList)[0] = ctx->var->getText();
	(*returnTypes)[0] = TypeDim(Variable::FLOAT , 0);
    FunctionType ft(fname  , parmlist->size());
    fname = registerFunction(ft , fname , parmlist);
	Variable* res = cg->generateFuncCall(fname , parmlist , returnList , returnTypes);

	//Cleanup
	delete parmlist;
	delete returnList;
	delete returnTypes;
	return res;
}


///////////////////////////////////////////////////////////////////////
//accsessor

/**
 * Visits the accesor context node in order to generate an Accessor.
 * @return An Accessor by the information in the Accesor context node.
 * @returns Type Accessor
 */
antlrcpp::Any MyMLVisitor::visitAccessor_mathexp(MLParser::Accessor_mathexpContext *ctx) {
	Variable* v = visit(ctx->ac);
	return new Accessor(v);
}
/**
 * Visits the accesor from to context node in order to generate an Accessor.
 * @return An Accessor by the information in the Accesor from to context node.
 * @returns Type Accessor
 */
antlrcpp::Any MyMLVisitor::visitAccessor_fromto(MLParser::Accessor_fromtoContext *ctx) {
	Variable* from = visit(ctx->from);
	Variable* to = visit(ctx->to);
	return new Accessor(from , to);
}
/**
 * Visits the accesor all context node in order to generate an Accessor.
 * @return An empty Accessor.
 * @returns Type Accessor
 */
antlrcpp::Any MyMLVisitor::visitAccessor_all(MLParser::Accessor_allContext *) {
	return new Accessor();
}

///////////////////////////////////////////////////////////////////////
//nameshape
antlrcpp::Any MyMLVisitor::visitNameshape(MLParser::NameshapeContext *ctx){
	if(ctx->shape){
		return NameShape(ctx->name->getText() , ctx->shape->getText());
	}else{
		return NameShape(ctx->name->getText() , "");
	}
}

///////////////////////////////////////////////////////////////////////
//varlist


/**
 * Visits the varlist context node.
 * @return An vector of strings the varlists consists of.
 * @returns Type vector<string>*
 */
antlrcpp::Any MyMLVisitor::visitVarlist(MLParser::VarlistContext *ctx) {
	return tokenToString(ctx->names);
}


/**
 * Visits the matexp list context node.
 * @return An vector of Variable objects the matheexp list consists of.
 * @returns Type vector<Variable*>*
 */
antlrcpp::Any MyMLVisitor::visitMathexplist(MLParser::MathexplistContext *ctx){
	return mathexpToVar(ctx->expressions);
}

///////////////////////////////////////////////////////////////////////
//getter

/**
 * Getter for targetType
 * @return The type of the target language
 */
int MyMLVisitor::getType(){
	return targetType;
}

