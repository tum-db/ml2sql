/**
 * @file Main.cpp
 * @author Matthias Bungeroth
 * @brief File contains main methode.
 *
 */
#include "ML/MLLexer.h"
#include "ML/MLParser.h"
#include "antlr4-runtime/antlr4-runtime.h"
#include "helper/Helpers.h"
#include "visitor/MyMLVisitor.h"
#include "visitor/MyPreVisitor.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <iostream>
#include <list>
#include <string>

using namespace antlr4;

string typeToFileEnding(int type) {
  switch (type) {
  case MLParser::PYTHON:
    return ".py";
  case MLParser::HYPER:
  case MLParser::POSTGRES:
    return ".sql";
  default:
    return "";
  }
}

int main(int argc, const char *argv[]) {

  if (argc <= 2) {
    std::cout << "No input/output file!" << std::endl;
    return 0;
  }
  int target;
  string def;
  bool csv = false;
  bool useModifiedCsv = false;
  if (argc <= 3) {
    std::cout << "No target language specified!" << std::endl;
    return 0;
  } else {
    string x = argv[3];
    if (x == "python") {
      target = MLParser::PYTHON;
      def = "PYTHON";
    } else if (x == "hyper") {
      target = MLParser::HYPER;
      def = "HYPER";
    } else if (x == "postgres") {
      target = MLParser::POSTGRES;
      def = "POSTGRES";
    } else {
      std::cout << "Unvalid target language specified!" << std::endl;
      return 0;
    }
    if (argc >= 5) {
      string a = argv[4];
      string b;
      if (argc >= 6) {
        b = argv[5];
      }
      if (a == "buildcsv" || b == "buildcsv") {
        csv = true;
      }
      if (a == "usemod" || b == "usemod") {
        useModifiedCsv = true;
      }
    }
  }
  std::string command = "gcc -D";
  command += def; // Define Target language in ml code
  command += " -E - <";
  std::stringstream stream = exec(command + argv[1]);
  ANTLRInputStream input(stream);
  PreLexer prelexer(&input);
  CommonTokenStream pretokens(&prelexer);
  PreParser preparser(&pretokens);
  PreParser::FileContext *pretree = preparser.file();
  if (preparser.getNumberOfSyntaxErrors() == 0) {
    MyPreVisitor previsitor;
    std::string res = previsitor.visit(pretree);
    // This part is ugly:
    // we receive the preprocessed string writing it to a file in order to
    // C-preprocess it again.
    //-----------------------------------------------------------------------------------------------------
    std::ofstream preOutStream(std::string(argv[2]) + "_pre.ml");
    preOutStream << res;
    preOutStream.close();
    // Call C-Preprocessor again since imports may us preprocess statments
    // loaded!
    res = exec(command + argv[2] + "_pre.ml").str();
    std::ofstream preOutStream2(std::string(argv[2]) + "_pre.ml");
    preOutStream2 << res;
    preOutStream2.close();
    //-----------------------------------------------------------------------------------------------------
    stream << res; // Fill the stream with the preprocessed input
    ANTLRInputStream input(stream);
    MLLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    MLParser parser(&tokens);
    MLParser::FileContext *tree = parser.file();

    if (parser.getNumberOfSyntaxErrors() == 0) {
      MyMLVisitor visitor(target, csv, useModifiedCsv);
      std::string result = visitor.visit(tree);
      if (!result.empty()) {
        std::ofstream os(argv[2] + typeToFileEnding(visitor.getType()));
        os << result;
        os.close();
      }
    }
  }
  return 0;
}
