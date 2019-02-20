#include "codeGenerator/sql/HyperGenerator.h"
#include "codeGenerator/sql/HyperCode.h"

namespace CodeGenerator {

    stringstream HyperGenerator::getInit() {
        stringstream ss;
        ss << create.str() << endl;
        return ss;
    }

    stringstream HyperGenerator::getStartMain() {
        functionsToDrop.emplace_back(ML_MAIN);
        stringstream ss;
        ss << START;
        return ss;
    }

    stringstream HyperGenerator::getCleanup() {
        stringstream ss;
        ss << endl << END << endl << endl << endl;
        for (string const &fname : functionsToDrop) {
            ss << DROPFUNCTION(fname) << endl;
        }
        for (string const &fname : tablesToDrop) {
            ss << DROPTABLE(fname)  << endl;
        }
        return ss;
    }

    stringstream HyperGenerator::declareVarsAndBegin() {
        stringstream ss;
        ss << "\n";
        return ss;
    }

    Variable *HyperGenerator::generateCreateTensorFromDB(string newName, string oldName, vector<string> *list) {
        stringstream ss;
        if(list->empty()){
            ss << LOAD(newName , oldName);
        }else{
            stringstream ls;
            char sep = ' ';
            for (string const &s : *list) {
                ls << sep << oldName << "." << s << "::double precision";
                sep = ',';
            }
            ss << CREATETENSOR(newName, ls.rdbuf(), oldName);
        }
        setVarToType(newName, TypeDim(Variable::FLOAT, 2));
        return new Variable(ss.str(), Variable::VOID);
    }

    Variable *HyperGenerator::generateDefineTensor(string tenName, vector<Accessor *> *accs, Variable *value) {
        stringstream ss;
        ss << FILLARRAY_BEGIN(value->getContext());
        char sep = ' ';
        for (Accessor *acc : *accs) {
            if (!acc->isSingle()) {
                throw ERROR("Can not define Tensors with : operator");
            }
            ss << sep << acc->getFrom()->getContext();
            sep = ',';
        }
        ss << FILLARRAY_END;
        TypeDim td(value->getType(), accs->size());
        Variable *r = new Variable(ss.str(), td);
        return generateSetVar(tenName, r, true);
    }

    Variable *HyperGenerator::generateSaveTensorToDB(string newName, string oldName, vector<string> *cols) {
        stringstream ss;
        if(cols->empty()){
            ss << "DELETE FROM ML_TENSORS WHERE id='" << newName << "'" << EXPEND << endl;
            ss << "INSERT INTO ML_TENSORS VALUES ('" << newName << "'," << oldName << ")" << EXPEND;
        }else{
            generateCreateTable(newName ,cols);
            return new Variable("--TOD save to DB " + oldName, Variable::NONE);
        }
        return new Variable(ss.str(), Variable::VOID);
    }


    Variable *HyperGenerator::generateIfElse(Variable *condition, string ifexps, string elseexps) {
        stringstream ss;
        ss << IF(condition->getContext(), ifexps);
        if (!elseexps.empty()) {
            ss << ELSE(elseexps);
        }
        return new Variable(ss.str(), Variable::VOID);
    }

    Variable *HyperGenerator::generateWhile(Variable *condition, string exps) {
        stringstream ss;
        ss << WHILE(condition->getContext(), exps);
        return new Variable(ss.str(), Variable::VOID);
    }

    Variable *HyperGenerator::generateFor(string name, Variable *from, Variable *to, string exps) {
        stringstream ss;
        ss << FOR(name, from->getContext(), to->getContext(), exps);
        return new Variable(ss.str(), Variable::VOID);
    }


    Variable *HyperGenerator::generateFuncCall(string fname, vector<Variable *> *parmlist, vector<string> *returnList,
                                               vector<TypeDim> *retTypes) {

        stringstream call;
        call << fname << "( ";
        call << generateList(parmlist);
        call << ")";
        if (returnList->empty()) {
            return new Variable(call.str(), Variable::VOID);
        }
        string typeName = "ML_" + to_string(parmlist->size()) + "_" + fname;
        if (typesToDrop.find(typeName) == typesToDrop.end() && returnList->size() > 1) {
            registerType(typeName, retTypes);
        }
        stringstream ss;
        string rname = (*returnList)[0];
        TypeDim rtd = (*retTypes)[0];
        switch (returnList->size()) {
            case 1:
                //Size of return list is one so we can initilize the variable normal
                if (setVarToType(rname, rtd)) {
                    ss << DEFVAR(rname, call.rdbuf(), rtd);
                } else {
                    ss << SETVAR(rname, call.rdbuf());
                }
                return new Variable(ss.str(), Variable::VOID);
            default: // >1
                string resName = typeName + "_result";
                if(setVarToType(resName, typeName)){
                    ss << DEFVARSTR(resName, call.rdbuf()  , typeName ) << endl;
                }else{
                    ss << SETVAR(resName, call.rdbuf()) << endl;
                }

                char col = 'a';
                string sep;
                for (size_t i = 0; i < returnList->size(); i++) {
                    rname = (*returnList)[i];
                    rtd = (*retTypes)[i];
                    if (setVarToType(rname, rtd)) {
                        ss << sep << DEFVAR(rname, resName << "." << col, rtd);
                    } else {
                        ss << sep << SETVAR(rname, resName << "." << col);//https://doxygen.postgresql.org/fmgr_8c.html#a3ef47bae5bdece1f823c804dc5a782c1
                    }
                    sep = '\n';
                    col++;
                }
                return new Variable(ss.str(), Variable::VOID);
        }

    }

    string HyperGenerator::typeToString(TypeDim td) {
        string res;
        switch (td.first) {
            case Variable::FLOAT:
                res = TFLOAT;
                break;
            case Variable::BOOL :
                res = TBOOL;
                break;
            case Variable::INTEGER:
                res = TINT;
                break;
            default:
                return string("ERROR");
        }
        for (size_t i = 0; i < td.second; i++) {
            res += "[]";
        }
        return res;
    }

    Variable *HyperGenerator::generateDefineFunction(string fname, vector<string> *parmlist, vector<TypeDim> *parmTypes,
                                                     vector<TypeDim> *retTypes, string exps) {
        stringstream ss;
        string function;
        function += fname; // += is more efficent than using +
        function += "(";
        string sep;
        for (size_t i = 0; i < parmlist->size(); i++) {
            function += sep;
            function += (*parmlist)[i];
            function += " ";
            function += typeToString((*parmTypes)[i]);
            function += " ";

            sep = ",";
        }
        function += ")";
        ss << DEFFUNCTION(function) << endl;
        functionsToDrop.push_back(function);
        //Returns
        ss << "returns ";
        switch (retTypes->size()) {
            case 0:
                ss << "void";
                break;
            case 1:
                ss << typeToString((*retTypes)[0]);
                break;
            default: // >1
                ss << "ML_" << to_string(parmlist->size()) << "_" << fname;
                break;
        }
        ss << " AS $$" << endl;

        ss << exps << endl;

        ss << "$$ LANGUAGE 'hyperscript' strict;";
        return new Variable(ss.str(), Variable::VOID, 0);
    }

    Variable *HyperGenerator::generateReturn(vector<Variable *> *returns) {
        stringstream ss;
        if (returns->empty()) {
            ss << "return" << EXPEND;
        } else {
            ss << "return (";
            string sep;
            for (Variable *v : *returns) {
                ss << sep << v->getContext();
                sep = ",";
            }
            ss << ")";
            ss << EXPEND;
        }
        return new Variable(ss.str(), Variable::VOID);
    }


    string HyperGenerator::generateBraceForList(string list) {
        string res = OPENARRAYBRACE;
        res += list;
        res += CLOSEARRAYBRACE;
        return res;
    }

    Variable *HyperGenerator::generateTensorFromList(string list, size_t type, size_t oldDims) {
        stringstream ss;
        ss << CREATEARRAY << OPENARRAYBRACE << list << CLOSEARRAYBRACE;
        if(oldDims == -1){
            oldDims++;
            ss << "::double precision[]";
        }
        return new Variable(ss.str(), type, oldDims + 1);
    }

    Variable *HyperGenerator::generateAccsessTensor(Variable *v, vector<Accessor *> *acc) {
        TypeDim td = v->getTypeDim();
        size_t dim = td.second;
        stringstream res;
        bool slicing = false;
        if (v->getDim() == acc->size()) {
            for (Accessor *ac : *acc) {
                slicing = slicing || !ac->isSingle();
            }
        } else {
            slicing = true;
        }
        if (slicing) {
            res << SLICE << "(";
        } else {
            res << ACCESS << "(";
        }
        res << v->getContext();

        size_t i = 1;
        for (Accessor *ac : *acc) {
            res << ',';
            if (ac->isSingle()) {
                res << "(" <<ac->getFrom()->getContext() << " + 1)::integer";
                if (slicing) {
                    res << ", (" << ac->getFrom()->getContext() << " + 1)::integer";
                }
                if (ac->getFrom()->getDim() != 0) {
                    throw ERROR("You can not access a tensor with a tensor");
                }
                dim--;
            } else if (ac->isFromTo()) {
                res << "(" << ac->getFrom()->getContext() << " + 1" << ")::integer ,(" << ac->getTo()->getContext() << " + 1)::integer";
                if (ac->getFrom()->getDim() != 0 || ac->getTo()->getDim() != 0) {
                    throw ERROR("You can not access a tensor with a tensor");
                }
            } else {
                res << "1, " << LEN << "(" << v->getContext() << "," << i << ")";
            }
            i++;
        }
        for (; i <= td.second; i++) {
            res << ',' << "1, " << LEN << "(" << v->getContext() << "," << i << ")";
        }
        res << ')';
        if (slicing) {
            res << ")"; // This is for array_resetlower
        }
        return new Variable(res.str(), td.first, td.second);
    }


    Variable *HyperGenerator::generatePow(Variable *l, Variable *r) {
        if(r->getDim() != 0){
            throw ERROR("The dimension of the rigth side of pow is not zero");
        }
        stringstream ss;
        switch (l->getDim()) {
            case 0:
                ss << POW(l->getContext(), r->getContext());
                break;
            case 2:
                ss << MATRIXPOW(l->getContext(), r->getContext());
                break;
            default:
                throw ERROR("Power can only be used on matrices and numbers");
        }
        TypeDim td = l->getTypeDim();
        return new Variable(ss.str(), td);
    }

    Variable *HyperGenerator::generateMul(Variable *l, Variable *r) {
        return mathHelper(l, r, "*");
    }

    Variable *HyperGenerator::generateTranspose(Variable *l) {
        stringstream ss;
        ss << TRANSPOSE(l->getContext());
        return new Variable(ss.str(), l->getTypeDim());
    }

    Variable *HyperGenerator::generateBoolean(bool b) {
        return new Variable(b ? "True" : "False", Variable::BOOL, 0);
    }

    Variable *HyperGenerator::generateXor(Variable *l, Variable *r) {
        stringstream ss;
        ss << "(" << l->getContext() << " and not " << r->getContext() << ") or " << "(not " << l->getContext()
           << " and " << r->getContext() << ")";
        int type = Variable::BOOL;
        int dim = 0;
        return new Variable(ss.str(), TypeDim(type, dim));
    }

    string HyperGenerator::generateComparisonEquals() {
        return string("=");
    }

    Variable *HyperGenerator::generatePrint(vector<Variable *> *list) {
        stringstream ss;
        ss << PRINT << "(";
        char sep = ' ';
        for (Variable *v : *list) {
            ss << sep << v->getContext();
            sep = ',';
        }
        ss << ')' << EXPEND;
        return new Variable(ss.str(), Variable::VOID);
    }

    Variable *HyperGenerator::generateSetVar(string name, Variable *r, bool remember) {
        stringstream ss;
        if (remember && setVarToType(name, r->getTypeDim())) {
            //We have to to this sice hyper is broken. var double[] = ... for example is not possible!
            if (r->getDim() > 0) {
                ss << DEFVAR(name, r->getContext(), r->getTypeDim());
            } else {
                ss << DEFVARWITHTYPE(name, r->getContext(), r->getTypeDim());
            }
        } else {
            ss << SETVAR(name, r->getContext());
        }
        return new Variable(ss.str(),Variable::VOID);
    }

    Variable *HyperGenerator::generatePlot(string, Variable *, Variable *, string, string, string) {
        return new Variable("-- Plotting not supported in Hyper", Variable::VOID);
    }


    Variable *HyperGenerator::generateGradientDescent(string function, vector<string> *data, vector<pair<string,string>>* opt
            , Variable *stepSize,Variable *maxSteps, Variable *batchsize, Variable *threshold) {

        string maxStepss = maxSteps ? maxSteps->getContext(): GD_MAX_STPES_DEFAULT;
        string stepSizes = stepSize ? stepSize->getContext(): GD_LEARNING_RATE_DEFAULT;
        string thresholds = threshold ? threshold->getContext(): GD_THRESHOLD_DEFAULT;
        string batchsizes = batchsize ? threshold->getContext() : GD_BATCH_SIZE_DEFAULT;

        string sep = "",optstring="",placestring="";
        stringstream ss;
        ss << "select ";
        for(pair<string,string> &p : *opt){
            ss << sep << p.first;
            optstring += sep;
            optstring += p.first;
            sep = ",";
        }
        sep = "";
        for(string &d : *data){
            placestring += sep;
            placestring += d;
            sep = ",";
        }

        ss << " from array_gd('" << function << "','" << optstring << "','" 
            << placestring << '\'' << ',' << stepSizes << "::double precision, " 
            << thresholds << "::double precision, " << maxStepss << ',' << batchsizes
            << ',' << optstring << ',' << placestring << ')' << EXPEND;


        return new Variable(ss.str(), Variable::VOID);
    }


    Variable* HyperGenerator::generateSetAccesedTensor(string tenName , vector<Accessor*>* acc , Variable* toSet ){
        //Variable* accesdTen = generateAccsessTensor(generateVariable(tenName) , acc);

        //I dont wank to copy the hole methode again to each code generator yust beacuse the offset
        stringstream res;
        res << tenName <<  "  = array_set(" << tenName << ", " << toSet->getContext();
        string sep = ", ";
        for (Accessor *ac : *acc) {
            if (ac->isSingle()) {
                res << sep << ac->getFrom()->getContext() << " + 1";
                if (ac->getFrom()->getDim() != 0) {
                    throw ERROR("You can not access a tensor with a tensor");
                }
            } else {
                throw ERROR("Setting a sliced Tensor in Hyper is not supported");
            }
        }
        res << ')' << EXPEND;
        return new Variable(res.str() ,Variable::VOID);

    }

};
