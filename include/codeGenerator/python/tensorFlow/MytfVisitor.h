
// Generated from tf.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime/antlr4-runtime.h"
#include "tf/tfParser.h"
#include "tf/tfLexer.h"
#include "tf/tfBaseVisitor.h"

/**
 * This namespace is used for generating tensorflow gradient descent code.
 */
namespace tf {

    class MytfVisitor : public tfBaseVisitor {
    public:

        antlrcpp::Any visitFile_compile(tfParser::File_compileContext *ctx) override;

        antlrcpp::Any visitMathexp_tensor(tfParser::Mathexp_tensorContext *ctx) override;

        antlrcpp::Any visitMathexp_neg(tfParser::Mathexp_negContext *ctx) override;

        antlrcpp::Any visitMathexp_brace(tfParser::Mathexp_braceContext *ctx) override;

        antlrcpp::Any visitMathexp_add(tfParser::Mathexp_addContext *ctx) override;

        antlrcpp::Any visitMathexp_mul(tfParser::Mathexp_mulContext *ctx) override;

        antlrcpp::Any visitMathexp_transpose(tfParser::Mathexp_transposeContext *ctx) override;

        antlrcpp::Any visitMathexp_pow(tfParser::Mathexp_powContext *ctx) override;

        antlrcpp::Any visitMathexp_num(tfParser::Mathexp_numContext *ctx) override;

        antlrcpp::Any visitMathexp_var(tfParser::Mathexp_varContext *ctx) override;

        antlrcpp::Any visitMathexplist(tfParser::MathexplistContext *ctx) override;

        antlrcpp::Any visitMathexp_fun(tfParser::Mathexp_funContext *ctx) override;


    };

};