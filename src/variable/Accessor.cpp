#include "variable/Accessor.h"


Accessor::Accessor() : from(nullptr) , to(nullptr) , single(false) , fromTo(false) {}
Accessor::Accessor(Variable* acc) : from(acc) , to(nullptr) , single(true) , fromTo(false) {}
Accessor::Accessor(Variable* from , Variable* to) : from(from) , to(to) , single(false) , fromTo(true) {}
Accessor::~Accessor(){
    delete from;
    if(fromTo){
        delete to;
    }
}

bool Accessor::isSingle(){
    return single;
}
bool Accessor::isFromTo(){
    return fromTo;
}

Variable* Accessor::getFrom(){
    return from;
}

Variable* Accessor::getTo(){
    return to;
}
