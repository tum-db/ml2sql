
// Generated from tf.g4 by ANTLR 4.7.1




#include "codeGenerator/python/tensorFlow/MytfVisitor.h"
#include "codeGenerator/python/tensorFlow/Graph.h"

using namespace tf;

antlrcpp::Any MytfVisitor::visitFile_compile(tfParser::File_compileContext *ctx){
  return visit(ctx->exp);
}

antlrcpp::Any MytfVisitor::visitMathexp_tensor(tfParser::Mathexp_tensorContext *){
  throw std::string("Not const tenosrs in function string not supported");
  return nullptr;
}

antlrcpp::Any MytfVisitor::visitMathexp_neg(tfParser::Mathexp_negContext *ctx){
  Node* l = visit(ctx->left);
  return static_cast<Node*>(new Neg(l));
}

antlrcpp::Any MytfVisitor::visitMathexp_brace(tfParser::Mathexp_braceContext *ctx){
  Node* l = visit(ctx->expr);
  return static_cast<Node*>(new Brace(l));
}

antlrcpp::Any MytfVisitor::visitMathexp_add(tfParser::Mathexp_addContext *ctx){
  if(ctx->op->getType() == tfParser::ADD){
      return static_cast<Node*>(new Add(visit(ctx->left) , visit(ctx->right)));
  }else{
      return static_cast<Node*>(new Sub(visit(ctx->left) , visit(ctx->right)));
  }

}

antlrcpp::Any MytfVisitor::visitMathexp_mul(tfParser::Mathexp_mulContext *ctx){
  if(ctx->op->getType() == tfParser::MUL){
    return static_cast<Node*>(new Mul(visit(ctx->left) , visit(ctx->right)));
  }else{
    return static_cast<Node*>(new Div(visit(ctx->left) , visit(ctx->right)));
  }
}

antlrcpp::Any MytfVisitor::visitMathexp_transpose(tfParser::Mathexp_transposeContext *ctx){
  Node* l = visit(ctx->left);
  return static_cast<Node*>(new Transpose(l));
}

antlrcpp::Any MytfVisitor::visitMathexp_pow(tfParser::Mathexp_powContext *ctx){
  return static_cast<Node*>(new Pow(visit(ctx->left) , visit(ctx->right)));
}

antlrcpp::Any MytfVisitor::visitMathexp_num(tfParser::Mathexp_numContext *ctx){
  return static_cast<Node*>(new ConstNumber(std::stod(ctx->num->getText())));
}

antlrcpp::Any MytfVisitor::visitMathexp_var(tfParser::Mathexp_varContext *ctx){
  return static_cast<Node*>(new Variable(ctx->var->getText()));
}

antlrcpp::Any MytfVisitor::visitMathexp_fun(tfParser::Mathexp_funContext *ctx){
  std::vector<Node*>* parms = visit(ctx->parms);
  std::string fname = ctx->fname->getText();
  return static_cast<Node*>(new Function( fname, parms));
}

antlrcpp::Any MytfVisitor::visitMathexplist(tfParser::MathexplistContext *ctx){
    std::vector<Node*>* ls = new std::vector<Node*>();
    for(tfParser::MathexpContext * t : ctx->expressions){
        ls->push_back(visit(t));
    }
    return ls;
}



