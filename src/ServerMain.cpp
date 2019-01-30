#include "ML/MLLexer.h"
#include "ML/MLParser.h"
#include "antlr4-runtime/antlr4-runtime.h"
#include "helper/Helpers.h"
#include "helper/picojson.h"
#include "visitor/MyMLVisitor.h"
#include "visitor/MyPreVisitor.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <string>
// pistache
#include "pistache/endpoint.h"
#include "pistache/http_header.h"
#include "pistache/http_headers.h"

using namespace antlr4;
using namespace Pistache;

class RestHandler : public Http::Handler {
public:
  HTTP_PROTOTYPE(RestHandler)
  void onRequest(const Http::Request &request, Http::ResponseWriter response) {
    response.headers()
        .add<Http::Header::AccessControlAllowOrigin>("*")
        .add<Http::Header::AccessControlAllowMethods>("GET, PUT")
        .add<Http::Header::AccessControlAllowHeaders>("content-type");
    int target;
    bool csv = false, useModifiedCsv = false;
    picojson::value jsonval; // json input object
    std::string inputjson = request.body();
    std::string err = picojson::parse(jsonval, inputjson);
    if (!err.empty()) {
      response.send(Http::Code::Bad_Request, err);
      return;
    } else if (!jsonval.contains("code")) {
      response.send(Http::Code::Bad_Request, "no input source code");
      return;
    }
    std::string inputcode = jsonval.get("code").to_str(),
                targetlang = jsonval.get("lang").to_str();
    for (auto &c : targetlang)
      c = toupper(c);
    if (targetlang == "HYPER") {
      target = MLParser::HYPER;
    } else if (targetlang == "POSTGRES") {
      target = MLParser::POSTGRES;
    } else { // default: python
      target = MLParser::PYTHON;
      targetlang = "PYTHON";
    }
    if (jsonval.contains("csv")) {
      csv = jsonval.get("csv").evaluate_as_boolean();
    }
    if (jsonval.contains("useModifiedCsv")) {
      csv = jsonval.get("useModifiedCsv").evaluate_as_boolean();
    }
    std::string command = " | gcc -D" + targetlang + " -E -x c++ -";
    std::stringstream stream = exec("echo '" + inputcode + "'" + command);
    ANTLRInputStream input(stream);
    PreLexer prelexer(&input);
    CommonTokenStream pretokens(&prelexer);
    PreParser preparser(&pretokens);
    PreParser::FileContext *pretree = preparser.file();
    if (preparser.getNumberOfSyntaxErrors() == 0) {
      MyPreVisitor previsitor;
      std::string res = previsitor.visit(pretree);
      // This part is ugly: we receive the preprocessed string in order to
      // C-preprocess it again. Call C-Preprocessor again since imports may us
      // preprocess statments loaded!
      res = exec("echo '" + res + "'" + command).str();
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
          response.send(Http::Code::Ok, "{\"lang\": \"" + targetlang +
                                            "\",\"result\": \"" + result +
                                            "\"}");
          return;
        }
      }
    }
    response.send(Http::Code::Bad_Request, "");
  }
};

int main(int argc, char **argv) {
  unsigned port = 5000;
  int opt;
  while ((opt = getopt(argc, argv, "p:")) != -1) {
    switch (opt) {
    case 'p':
      port = atoi(optarg);
      break;
    default:
      break;
    }
  }
  Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(port));
  auto opts = Pistache::Http::Endpoint::options().threads(1);
  Http::Endpoint server(addr);
  server.init(opts);
  server.setHandler(Http::make_handler<RestHandler>());
  server.serve();
  server.shutdown();
  return 0;
}
