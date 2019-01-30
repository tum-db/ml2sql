#pragma once
#include <string>
#include <sstream> 
#include <utility>  
#define ML_TAB std::string("    ")

/**
 * Describs the type and dimension of a Variable
 * first will be the type
 * second will be the dimension
 */
typedef std::pair<size_t, size_t> TypeDim;



/**
 * A Variable holds generated code and the corresponding type of the code.
 */
class Variable{
public:
	/**
	 * Types a Variable can be.
	 * td.first will always be on of those values!
	 */
	enum{
		INTEGER, FLOAT , BOOL , STRING ,VOID  , NONE , INJECTED, DISTRIBUTION
	};
	Variable(std::string context , size_t type , size_t dim );
	Variable(std::string context , size_t type );
	Variable(std::string context , TypeDim td );
	void setContext(std::string context);
	TypeDim getTypeDim();
    size_t getType();
    size_t getDim();
	std::string getContext();
protected:
	std::string context; /**< The generated code the Variable holds */
	TypeDim td; /**< The information about the Type and Dimension of the Variable */


};