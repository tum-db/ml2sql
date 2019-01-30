#pragma once

#include "SqlGenerator.h"
#include <set>



namespace CodeGenerator {

    /**
	 *  Generates Hyper-Script code
	 */
    class HyperGenerator : public SqlGenerator {
    public:
        ~HyperGenerator() {}

        HyperGenerator() {
            type = MLParser::HYPER;
        }

    #include "SqlOverrideFunctions.h"


    //Hyper has diffrent syntax compared to Python and Postgres
    virtual Variable* generateSetAccesedTensor(string name , vector<Accessor*>* ac , Variable* toSet );


    };

};