#pragma once

#include <iostream>
#include <string> 
#include <list>
#include <map>
#include <vector>
#include <regex>
#include <cstddef>

#include "antlr4-runtime/antlr4-runtime.h"
#include "ML/MLParser.h"
#include "ML/MLLexer.h"
#include "ML/MLBaseVisitor.h"
#include "codeGenerator/python/PythonGenerator.h"
#include "codeGenerator/sql/PostgresGenerator.h"
#include "codeGenerator/sql/HyperGenerator.h"
#include "variable/Variable.h"



using namespace std;


class  MyMLVisitor : public MLBaseVisitor { 
public:
	~MyMLVisitor();
	MyMLVisitor(int target, bool csv,bool useModified)
		: MLBaseVisitor() , targetType(target) , csv(csv) , useModified(useModified) {}
	MyMLVisitor() : MLBaseVisitor(){};
	
	
	//file
	antlrcpp::Any visitFile_compile(MLParser::File_compileContext *ctx) override;

	antlrcpp::Any visitFunctions(MLParser::FunctionsContext *ctx) override;
	//explist
	antlrcpp::Any visitExplist(MLParser::ExplistContext *ctx) override;

	//expression
	antlrcpp::Any visitExpression_inject(MLParser::Expression_injectContext *ctx) override;
	antlrcpp::Any visitExpression_setVar(MLParser::Expression_setVarContext *ctx) override;
	antlrcpp::Any visitExpression_createTensor(MLParser::Expression_createTensorContext *ctx) override;
    antlrcpp::Any visitExpression_defineTensor(MLParser::Expression_defineTensorContext *ctx) override;
	antlrcpp::Any visitExpression_sampleTensor(MLParser::Expression_sampleTensorContext *ctx) override;
	antlrcpp::Any visitExpression_saveTensor(MLParser::Expression_saveTensorContext *ctx) override;
	antlrcpp::Any visitExpression_if(MLParser::Expression_ifContext *ctx) override;
	antlrcpp::Any visitExpression_while(MLParser::Expression_whileContext *ctx) override;
	antlrcpp::Any visitExpression_for(MLParser::Expression_forContext *ctx) override;
	antlrcpp::Any visitExpression_for_interval(MLParser::Expression_for_intervalContext *ctx) override;
	antlrcpp::Any visitExpression_functionCall(MLParser::Expression_functionCallContext *ctx) override;
	antlrcpp::Any visitExpression_functionDef(MLParser::Expression_functionDefContext *ctx) override;
	antlrcpp::Any visitExpression_return(MLParser::Expression_returnContext *ctx) override;
	antlrcpp::Any visitExpression_keyword(MLParser::Expression_keywordContext *ctx) override;
	antlrcpp::Any visitExpression_setAccessedTensor(MLParser::Expression_setAccessedTensorContext *ctx) override;
	antlrcpp::Any visitExpression_externFunction(MLParser::Expression_externFunctionContext *ctx) override;
	antlrcpp::Any visitExpression_print(MLParser::Expression_printContext *ctx) override;
	antlrcpp::Any visitExpression_readcsv(MLParser::Expression_readcsvContext *ctx) override ;
    antlrcpp::Any visitExpression_gradientDescent(MLParser::Expression_gradientDescentContext *ctx) override;
	antlrcpp::Any visitExpression_plot(MLParser::Expression_plotContext *ctx) override;

    //return type
	antlrcpp::Any visitReturnType(MLParser::ReturnTypeContext *ctx) override;
	//interval
	antlrcpp::Any visitInterval(MLParser::IntervalContext *ctx) override;
	//Mathexp
	antlrcpp::Any visitMathexp_transpose(MLParser::Mathexp_transposeContext *ctx) override;
	antlrcpp::Any visitMathexp_mul(MLParser::Mathexp_mulContext *ctx) override;
	antlrcpp::Any visitMathexp_add(MLParser::Mathexp_addContext *ctx) override;
	antlrcpp::Any visitMathexp_neg(MLParser::Mathexp_negContext *ctx) override;
	antlrcpp::Any visitMathexp_pow(MLParser::Mathexp_powContext *ctx) override; 

	
	antlrcpp::Any visitMathexp_brace(MLParser::Mathexp_braceContext *ctx) override;
	antlrcpp::Any visitMathexp_tensor(MLParser::Mathexp_tensorContext *ctx) override;
	antlrcpp::Any visitMathexp_accsesstensor(MLParser::Mathexp_accsesstensorContext *ctx) override;
	antlrcpp::Any visitMathexp_functionCall(MLParser::Mathexp_functionCallContext *ctx) override;
	antlrcpp::Any visitMathexp_var(MLParser::Mathexp_varContext *ctx) override;
	antlrcpp::Any visitMathexp_num(MLParser::Mathexp_numContext *ctx) override;
	
	antlrcpp::Any visitMathexp_boolop(MLParser::Mathexp_boolopContext *ctx) override;
	antlrcpp::Any visitMathexp_bool(MLParser::Mathexp_boolContext *ctx) override;
	antlrcpp::Any visitMathexp_boolNot(MLParser::Mathexp_boolNotContext *ctx) override;
	antlrcpp::Any visitMathexp_compare(MLParser::Mathexp_compareContext *ctx) override;
	antlrcpp::Any visitMathexp_append(MLParser::Mathexp_appendContext *ctx) override;
	antlrcpp::Any visitMathexp_string(MLParser::Mathexp_stringContext *ctx) override; 

	//varlist
	antlrcpp::Any visitVarlist(MLParser::VarlistContext *ctx) override;
	//mathexplist
	antlrcpp::Any visitMathexplist(MLParser::MathexplistContext *ctx) override;
	//accsesslist
	antlrcpp::Any visitAccsesslist(MLParser::AccsesslistContext *ctx) override;
	//accsessor
	antlrcpp::Any visitAccessor_mathexp(MLParser::Accessor_mathexpContext *ctx) override;
  	antlrcpp::Any visitAccessor_fromto(MLParser::Accessor_fromtoContext *ctx) override;
 	antlrcpp::Any visitAccessor_all(MLParser::Accessor_allContext *ctx) override;
    //nameshape
    antlrcpp::Any visitNameshape(MLParser::NameshapeContext *ctx) override;

	//distribution
	antlrcpp::Any visitExpression_setDist(MLParser::Expression_setDistContext *ctx) override;
  	antlrcpp::Any visitExpression_sampleDist(MLParser::Expression_sampleDistContext *ctx) override;


	//getter
	int getType();

private:
	typedef std::pair<std::string, int> FunctionType;
    typedef std::pair<std::string, std::string> NameShape;
	
	//helper
	void reportError(MLParser::ExpressionContext *  t, std::string err);
	void resetGenerator();
	void readFunctionsTXT();
	Variable* defineFunction(MLParser::Expression_functionDefContext* ctx);
	vector<string>* tokenToString(vector<antlr4::Token*> &vec);
	vector<Variable*>* mathexpToVar(vector<MLParser::MathexpContext*> &vec);
	string registerFunction(FunctionType ft  , string fname , vector<Variable*>* parmlist);

	

	//Variables
	CodeGenerator::Generator* cg; /**< The codegenerator for the desired target language */
	map<FunctionType,vector<TypeDim>*> functionToParmTypes; /**< Maps FunctionType to the parameter types of the function */
	map<FunctionType,vector<TypeDim>*> functionToReturnTypes; /**< Maps FunctionType to the return types of the function */
	map<FunctionType,string> functionToTargetName; /**< Maps FunctionType to the actual function name in the target language*/
	map<string,pair<string,string>> distributionMap; /**< Maps a distribution name to its sample and probability function name */
	list<MLParser::Expression_functionDefContext*> functionsToDefine; /**< Set of all function that must be defiend in the end */
	int targetType; /**< type of the target language*/
	bool csv; /**< should the modified csv be created, false if it already exists*/
	bool useModified; /**< should the modified csv be used*/
	stringstream errorLog;

	
};




