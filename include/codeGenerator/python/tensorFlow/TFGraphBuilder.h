//
// Created by matthias on 27.09.18.
//

#pragma once

#include "Graph.h"
#include <string>


namespace tf {
    class TFGraphBuilder {
    public:
        TFGraphBuilder(std::string &s);

        ~TFGraphBuilder() { if (err) delete graph; }

        std::string toTFGraph();

        bool hasError();

    private:
        bool err;
        tf::Node *graph;
    };
};
