

#pragma once

#include "codeGenerator/Generator.h"
#include <set>
#include <cstddef>


namespace CodeGenerator {

	/**
	 *  Implemets methods that are the same in postgres and hyper
	 */
	class SqlGenerator : public Generator {
	public:


		Variable *generateRandom() override;

		Variable* generateNum(string num , int type) override;

		Variable *generateAppend(Variable *l, Variable *r) override;

		Variable* generateSaveTensorToCSV(string newName, string filename , vector<string>* l, string del) override;
		Variable *generateReadCsv(string newName, string filename, vector<string> *cols, string del, bool deleteEmpty,
								  Variable *replaceEmpts, vector<pair<string, string>> *replaceList, bool createCsv , bool useModified) override;

		virtual string typeToString(TypeDim type) = 0;

	protected:
		stringstream create;
		set<string> typesToDrop; /**< Set of types that have to be removed in the end*/
		set<string> tablesToDrop; /**< Set of tables that have to be removed in the end*/
		static list<string> functionsToDrop; /**< Set of functions that have to be removed in the end*/
		string generateCreateTable(string tableName ,vector<string> *cols );
		void registerType(string tableName, vector<TypeDim> *types);


	};

};