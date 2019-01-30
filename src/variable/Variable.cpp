

#include <variable/Variable.h>

#include "variable/Variable.h"


Variable::Variable(std::string context, size_t type, size_t dim) : context(context) ,td(type,dim) {}
Variable::Variable(std::string context , size_t type ) : context(context) ,td(type,0){}
Variable::Variable(std::string context , TypeDim td ) : context(context) ,td(td) {}
void Variable::setContext(std::string context){
	this->context = context;
}
TypeDim Variable::getTypeDim(){
	return td;
}
size_t Variable::getType(){
	return td.first;
}
size_t Variable::getDim(){
	return td.second;
}
std::string Variable::getContext(){
	return context;
}

