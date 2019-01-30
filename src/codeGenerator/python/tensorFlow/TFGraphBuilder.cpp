//
// Created by matthias on 27.09.18.
//
#include "codeGenerator/python/tensorFlow/TFGraphBuilder.h"

#include "antlr4-runtime/antlr4-runtime.h"
#include "tf/tfLexer.h"
#include "tf/tfParser.h"
#include "codeGenerator/python/tensorFlow/MytfVisitor.h"


using namespace antlr4;


/**
 * Parsers the input string into a Graph and sets the error variable.
 * @param s the string representatino of the function to parse
 */
tf::TFGraphBuilder::TFGraphBuilder(std::string& s){
    ANTLRInputStream input(s);
    tfLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tfParser parser(&tokens);
    tfParser::FileContext* tree = parser.file();

    if(parser.getNumberOfSyntaxErrors() == 0){
        MytfVisitor visitor;
        graph = visitor.visit(tree);
        err = false;
    }else{
        err = true;
    }
}
/**
 * Returns if the graph parsing had any error.
 */
bool tf::TFGraphBuilder::hasError(){
    return err;
}

/**
 * Builds the string representation of the graph and returs it.
 */
std::string tf::TFGraphBuilder::toTFGraph(){
    return graph->toString();
}