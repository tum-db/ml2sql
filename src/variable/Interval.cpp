//
// Created by matthias on 12.09.18.
//



#include "variable/Interval.h"


Interval::Interval(bool isList) : isList(isList){}
bool Interval::isListInterval(){
    return isList;
}



ListInterval::ListInterval(Variable* list) : Interval(true)  , list(list){}

ListInterval::~ListInterval(){
    delete list;
}
Variable* ListInterval::getList(){
    return list;
}



NumInterval::NumInterval(Variable* from , Variable* to , Variable* step , std::string& fname )
    : Interval(false) , from(from) , to(to) , step(step) , fname(fname)  {}

NumInterval::~NumInterval(){
    delete from;
    delete to;
    delete step;
}

Variable* NumInterval::getFrom(){
    return from;
}
Variable* NumInterval::getTo(){
    return to;
}
Variable* NumInterval::getStep(){
    return step;
}
std::string NumInterval::getFname(){
    return fname;
}



