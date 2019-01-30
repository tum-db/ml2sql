#pragma once

#include <iostream>
#include <string> 
#include <set>
#include <regex>

#include "antlr4-runtime/antlr4-runtime.h"
#include "Pre/PreParser.h"
#include "Pre/PreLexer.h"
#include "Pre/PreBaseVisitor.h"




using namespace std;


class  MyPreVisitor : public PreBaseVisitor {
public:
	MyPreVisitor() : PreBaseVisitor() {}
	
	antlrcpp::Any visitFile(PreParser::FileContext *ctx) override;
  	antlrcpp::Any visitExp_cross(PreParser::Exp_crossContext *ctx) override;
	antlrcpp::Any visitExp_laodLib(PreParser::Exp_laodLibContext *ctx) override;
	antlrcpp::Any visitExp_line(PreParser::Exp_lineContext *ctx) override;
	
};




