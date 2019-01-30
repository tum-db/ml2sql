//
// Created by matthias on 12.09.18.
//

#pragma once

#include "Variable.h"

/**
 * Needed to generate for loops over the Interval
 */
class Interval {
public:
    explicit Interval(bool isList);
    bool isListInterval();
private:
    bool isList; /**< true if the interval of type ListInterval */
};

/**
 * Consists of a tensor.
 */
class ListInterval : public Interval{
public:
    explicit ListInterval(Variable* list);
    ~ListInterval();
    Variable* getList();
private:
    Variable* list;  /**< The Variable that saves the tensor */
};


/**
 * Is a numerical interval and going from <from> to <to> with stepsize <step>.
 * The stepsize <step> may be scaled by the function <fname>.
 */
class NumInterval: public Interval{
public:

    NumInterval(Variable* from , Variable* to , Variable* step , std::string& fname );
    ~NumInterval();

    Variable* getFrom();
    Variable* getTo();
    Variable* getStep();
    std::string getFname();
private:
    Variable* from;
    Variable* to;
    Variable* step;
    std::string fname;
};

