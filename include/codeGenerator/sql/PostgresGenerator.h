#pragma once

#include "SqlGenerator.h"

#include <set>


namespace CodeGenerator {


	/**
	 *  Generates plpgsql code
	 */
	class PostgresGenerator : public SqlGenerator {
	public:
		~PostgresGenerator() {}

		PostgresGenerator() {
			type = MLParser::POSTGRES;
		}

		#include "SqlOverrideFunctions.h"


	private:
		//Folowing are need because we do not want to declare the fuction parameters!
		map<string, TypeDim> nameToTypePG; /**< Maps variable names to TypeDim objects needed because functions parameters
 											* muss be it this map but should not be declared in the beginning*/
		map<string, string> nameToTypeNamePG; /**< Maps variable names to custom type names objects needed because
 												* functions parameters muss be it this map but should not be declared in the beginning*/


		bool setVarToTypePG(string name, TypeDim td);

		bool setVarToTypePG(string name, string type);

		stringstream declareVars();
	};

};