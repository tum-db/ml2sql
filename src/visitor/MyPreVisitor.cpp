/**
 * @file MyPreVisitor.cpp
 * @author Matthias Bungeroth
 * @brief File contains Implementation of MyPreVisitor
 *
 */

#include "visitor/MyPreVisitor.h"
#include "mllib/Ml_Lib.h"



using namespace std;

/**
 * Visit file is the entry point to the syntax tree. It is always the root node of the syntax tree.
 * @param ctx The root node of the syntax tree
 * @return A string consisting of the preprocessed code.
 * @returns Type String.
 */
antlrcpp::Any MyPreVisitor::visitFile(PreParser::FileContext *ctx) {
	stringstream res;
	//First include the basics.ml file!
    int index = libNameToIndex["basics"];
    res << string( (char * ) ml_libs[index] , ml_libs_len[index] ) << endl;

	for(auto ct : ctx->exps){
		string s = visit(ct);
		res << s;
	}
	//cout << res.rdbuf();
	return res.str();
}

/**
 * Generates a function call while folding al varaibles in the set <folds>, renaming vars in set <test>,
 * and replacing every occurrence of <replace> by <by>
 * @param fun The functino context node
 * @param folds holds all the data that will be folded
 * @param test holds all variables that will be tested
 * @param ntext text that describes the maximum number of datapoints
 * @param replace Text replaced by <by>
 * @param by Tetxt that replaces <replace>
 * @return A stringstring holding the generated function call.
 */
stringstream buildFunctionCall(PreParser::FunContext* fun , set<string> folds , set<string> test , string ntext , string replace , string by){
	stringstream ss;
	ss << fun->fname->getText() << '(';
	char sep = ' ';
	for(auto parm : fun->parms){
		ss << sep;
		sep = ',';
		string p = parm->getText();
		if(folds.find(p) != folds.end()){
			//make variable folded!
			if(p == replace){
			    p = by;
			}
			ss << p << "[0:ML_l-1]::" << p << "[ML_r+1:" << ntext << "-1]";
		}else if(test.find(p) != test.end()){
			ss << "ML_try_" << p;
		}else{
			ss << p;
		}
	}
	ss << ')';
	return ss;
}


/**
 * Concatenates the text (only one char in this case) of all tokens in the input vector.
 * @param list Vector of tokens
 * @return A string consisting of the concatenated text of all tokens.
 */
string getStringFromTokenList(vector<antlr4::Token *> &list){
	string s;
	for(auto t : list){
		s += t->getText();
	}
	return s;
}


/**
 * Generates code for cross validation in the ml language.
 * @param ctx Cross-Validation contex node
 * @return A string consisting of the generated code.
 */
antlrcpp::Any MyPreVisitor::visitExp_cross(PreParser::Exp_crossContext *ctx) {

	if(!ctx->minfun) throw "minfun is not specified";
	if(!ctx->lossfun) throw "lossfun is not specified";
	if(!ctx->wname) throw "wname is not specified";


	string ntext = getStringFromTokenList(ctx->n);
	string foldstext = getStringFromTokenList(ctx->folds);
	string weights = ctx->wname->getText();

	stringstream init;
	init << "ML_minLoss = 10000000.0 //MAX_VALUE" << endl;
	init << weights << "= [[]]" << endl;
	init << "ML_try_" << weights << "= [[]]" << endl;
    string designData;
    string designMatrix;
	if(!ctx->designMaxIndex.empty()){
		string dname = ctx->designfun->getText();
        string designMax = getStringFromTokenList(ctx->designMaxIndex);
        designData = ctx->designfunData->getText();
        designMatrix = "ML_Design_" + designData;
		init << designMatrix << "[" << ntext << ", " << designMax <<  " +1 ]:0.0 // shape(n x maxindex+1)" << endl;
		init << "for ML_i from 0 to " << ntext << "{" << endl;;
		init << '\t' << "for ML_j from 0 to " << designMax << "+1 {" << endl;;
		init << '\t' << '\t' << designMatrix << "[ML_i,ML_j] = " << dname << '(' << designData << "[ML_i] , ML_j )" << endl;
		init << '\t' << '}' << endl;
		init << '}' << endl;;
	}

    set<string> folds;
    set<string> test;

    for(auto t : ctx->foldNames){
        folds.insert(t->getText());
    }

    string tabs;
    for(auto t : ctx->testNames){
        tabs += '\t';
        test.insert(t->getText());
    }
    stringstream folding;


	folding
	    << tabs << "ML_loss = 0.0" << endl
		<< tabs << "ML_size = (" << ntext << ") / (" << foldstext << ")" << endl
		<< tabs << "for ML_i from 1 to "<< foldstext << "-1{" << endl
		<< tabs << "\tML_l = ML_i * ML_size" << endl
        << tabs << "\tML_r = min(ML_l + ML_size - 1,"<< ntext << ")" << endl;



	test.insert(weights);// Not really a test variable but can be handled like that!

	stringstream minfun = buildFunctionCall(ctx->minfun , folds , test , ntext , designData , designMatrix);
	stringstream lossfun = buildFunctionCall(ctx->lossfun , folds , test , ntext, designData , designMatrix);


	
	folding << tabs << "\tML_try_"<< weights <<" = "<< minfun.rdbuf() << endl;
	folding << tabs << "\tML_loss = ML_loss +"<< lossfun.rdbuf() << endl;
	folding << tabs << "}" << endl ;
	folding << tabs << "if(ML_minLoss > ML_loss){" << endl;
	folding	<< tabs << "\tML_minLoss = ML_loss" << endl;
	folding << tabs << "\t"<< weights << " = ML_try_"<< weights << endl;
			


	stringstream outerforloops;
	string outertabs;
	for(size_t i = 0; i < ctx->testNames.size(); i++){
		string var = ctx->testNames[i]->getText();
		string interval = ctx->testIntervals[i]->getText();

		init << var << " = 0.0"  << endl;
		folding << tabs << "\t" << var << " = "<< "ML_try_" << var << endl;
		outerforloops << outertabs <<"for " << "ML_try_" << var << " in " << interval  << "{" << endl;
		outertabs += '\t';
	}
	folding << tabs << "}" << endl ;
	for(size_t i = 0; i < ctx->testNames.size(); i++){
		tabs.erase(tabs.begin());
		folding << tabs << "}" << endl;
	}
	
	init <<outerforloops.rdbuf() << folding.rdbuf();
	return init.str();
}


antlrcpp::Any MyPreVisitor::visitExp_laodLib(PreParser::Exp_laodLibContext *ctx){
	string lname = ctx->libname->getText();
	if(libNameToIndex.find(lname) == libNameToIndex.end()){
		throw string("Could not find import");
	}else{
		int index = libNameToIndex[lname];
		return "\n" + string( (char * ) ml_libs[index] , ml_libs_len[index] ) + "\n";
	}

}


/**
 * Returns the text of the hole line because there is nothing to preprocess here.
 */
antlrcpp::Any MyPreVisitor::visitExp_line(PreParser::Exp_lineContext *ctx){
	return ctx->getText();
}