#pragma once

#include<map>
#include<list>
#include<string>
#include<vector>
#include "variable/Interval.h"
#include "variable/Accessor.h"
#include "variable/Variable.h"
#include "ML/MLParser.h"

#define ERROR(s) std::string(s)


#define GD_MAX_STPES_DEFAULT "1000"
#define GD_LEARNING_RATE_DEFAULT "0.002"
#define GD_BATCH_SIZE_DEFAULT "1"
#define GD_THRESHOLD_DEFAULT "0.001"

using namespace std;


/**
 * This namespace contains all code generators.
 */
namespace CodeGenerator{


	/**
	 *  Abstract class Generator
	 */
	class Generator {
		public:

		virtual ~Generator() = default;

		virtual Variable* generateCreateTensorFromDB(string newName,string oldName, vector<string>* l) = 0;
		virtual Variable* generateDefineTensor(string tenName, vector<Accessor*>* accs ,Variable* value) = 0;

		virtual Variable* generateSaveTensorToDB(string newName,string oldName, vector<string>* l) = 0;
		virtual Variable* generateSaveTensorToCSV(string newName, string filename , vector<string>* l, string del) = 0;

		virtual Variable* generateReadCsv(string newName , string filename , vector<string>* cols , string del , bool deleteEmpty
				, Variable* replaceEmptys , vector<pair<string,string>>* replaceList, bool createCsv, bool useModified ) = 0;

		virtual Variable* generateIfElse(Variable* condition , string ifexps , string elseexps) = 0;
		virtual Variable* generateWhile(Variable* condition , string exps) = 0;
		virtual Variable* generateFor(string name,Variable* from , Variable* to , string exps) = 0;

				string generateList( vector<Variable*>* list);
		virtual Variable* generateFuncCall(string fname ,vector<Variable*>* parmlist ,vector<string>* returnList , vector<TypeDim>* retTypes) = 0;
		virtual Variable* generateDefineFunction(string fname ,vector<string>*  parmlist, vector<TypeDim>* parmTypes , vector<TypeDim>* retTypes ,string exps) = 0;
		virtual Variable* generateReturn(vector<Variable*>* returns) = 0;

		 		Variable* generateVariable(string name);
		virtual string    generateBraceForList(string list) = 0;
		virtual Variable* generateTensorFromList(string list, size_t type, size_t oldDims) = 0;
		virtual Variable* generateAppend(Variable* l , Variable* r) = 0;
		virtual Variable* generateAccsessTensor(Variable* v,  vector<Accessor*>* ac) = 0;
		virtual Variable* generateSetAccesedTensor(string name , vector<Accessor*>* ac , Variable* toSet );

		virtual Variable* generateNum(string num , int type) = 0;
		virtual Variable* generateMul(Variable* l , Variable* r) = 0;
				Variable* generateDiv(Variable* l , Variable* r);
				Variable* generateSub(Variable* l , Variable* r);
				Variable* generateAdd(Variable* l , Variable* r);
		virtual Variable* generatePow(Variable* l , Variable* r) = 0;
				Variable* generateNeg(Variable* r);
		virtual Variable* generateTranspose(Variable* l) = 0;

		virtual Variable* generateBoolean(bool b) = 0;
		virtual Variable* generateAnd (Variable* l , Variable* r);
		virtual Variable* generateOr (Variable* l , Variable* r);
		virtual Variable* generateXor (Variable* l , Variable* r) = 0;


		virtual Variable* generatePrint(vector<Variable*>* list) = 0;

		virtual Variable* generateRandom() = 0;

		virtual string generateComparisonEquals() = 0;

		virtual Variable* generatePlot(string fileName ,Variable* xData ,Variable* yData  ,string xLable ,string yLable ,string type) = 0;
		virtual Variable* generateGradientDescent(string function , vector<string>* data , vector<pair<string,string>>* opt
		        , Variable* stepSize, Variable* steps, Variable* batchsize, Variable *threshold) = 0;

		virtual Variable* generateSetVar(string name  , Variable*  r , bool remember) = 0;


		virtual stringstream declareVarsAndBegin() = 0;
		virtual stringstream getInit() = 0;
		virtual stringstream getStartMain() = 0;
		virtual stringstream getCleanup() = 0;

				Variable* mathHelper(Variable* l , Variable* r , string s);
				bool setVarToType(string name , TypeDim td);
				bool setVarToType(string name  ,string type);
				bool removeVar(string name);


		size_t getType();


	protected:
		map<string,TypeDim> nameToType; /**< Maps variable names to TypeDim objects*/
		map<string,string> nameToTypeName; /**< Maps variable names to the name of custom types*/
		size_t type; /**< The type of the codegenerator*/

        void createvsv(string filename , string names , string del , vector<pair<string, string>> *replaceList);

	};

};