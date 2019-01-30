#pragma once

#include "Variable.h"



/**
 * Holds information to access a tensor.
 * if       isSingle() -> accessor looks like A[i]
 * else if  isFromTo() -> accessor looks like A[i:j]
 * else                -> accessor looks like A[:]
 */
class Accessor {
public:
    Accessor();
    explicit Accessor(Variable* acc);
    Accessor(Variable* from , Variable* to);
    ~Accessor();


    bool isSingle();
    bool isFromTo();

    Variable* getFrom();

    Variable* getTo();
private:
    Variable* from;
    Variable* to;
    bool single; /**< true if accesor looks like A[i] */
    bool fromTo; /**< true if accesor looks like A[i:j] */
};