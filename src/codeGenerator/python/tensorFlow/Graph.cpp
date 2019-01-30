#include "codeGenerator/python/tensorFlow/Graph.h"

namespace tf {

    std::string ConstNumber::toString() {
        return std::to_string(l);
    }

    std::string Add::toString() {
        return "tf.add(" + l->toString() + ", " + r->toString() + ")";
    }

    std::string Sub::toString() {
        return "tf.subtract(" + l->toString() + ", " + r->toString() + ")";
    }

    std::string Mul::toString() {
        return "tf.matmul(" + l->toString() + ", " + r->toString() + ")";
    }

    std::string Pow::toString() {
        return "tf.pow(" + l->toString() + ", " + r->toString() + ")";
    }

    std::string Div::toString() {
        return "tf.div(" + l->toString() + ", " + r->toString() + ")";
    }

    std::string Variable::toString() {
        return " ML_tf_" + l ;
    }

    std::string Neg::toString() {
        return "tf.negative(" + l->toString() + ")";
    }

    std::string Transpose::toString() {
        return "tf.transpose(" + l->toString() + ")";
    }

    std::string Brace::toString() {
        return l->toString();
    }


    std::string Function::toString() {
        std::string res = "tf." + name + "(";
        std::string sep;
        for (Node *n : *parms) {
            res += sep;
            res += n->toString();
            sep = ",";
        }
        res += ')';
        return res;
    }

}