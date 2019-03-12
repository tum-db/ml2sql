#include "codeGenerator/sql/PostgresGenerator.h"
#include "codeGenerator/sql/PostgresCode.h"

namespace CodeGenerator {

    stringstream PostgresGenerator::getInit() {
        stringstream ss;
        ss << create.str() << endl;
        return ss;
    }

    stringstream PostgresGenerator::getStartMain() {
        stringstream ss;
        ss << START << endl;
        return ss;
    }

    stringstream PostgresGenerator::getCleanup() {
        stringstream ss;
        ss << END << endl << endl << endl << endl;
        for (string const &fname : functionsToDrop) {
            ss << DROPFUNCTION(fname) << endl;
        }
        for (string const &fname : tablesToDrop) {
            ss << DROPTABLE(fname) << endl;
        }
        for (string const &name : typesToDrop) {
            ss << DROPTYPE(name) << endl;
        }
        return ss;
    }

    stringstream PostgresGenerator::declareVars() {
        stringstream ss;
        ss << DECLARE << endl;
        for (pair<string, TypeDim> const &p : nameToTypePG) {
            ss << ML_TAB << p.first << " " << typeToString(p.second) << EXPEND << endl;
        }
        for (pair<string, string> const &p : nameToTypeNamePG) {
            ss << ML_TAB << p.first << " " << p.second << EXPEND << endl;
        }

        return ss;
    }

    stringstream PostgresGenerator::declareVarsAndBegin() {
        stringstream ss = declareVars();
        ss << BEGIN << endl;

        return ss;
    }

    Variable *PostgresGenerator::generateCreateTensorFromDB(string newName, string oldName, vector<string> *list) {
        stringstream ss;
        if(list->empty()){
            ss << LOAD(newName , oldName);
        }else{
            stringstream ls;
            char sep = ' ';
            for (string const &s : *list) {
                ls << sep << oldName << "." << s;
                sep = ',';
            }
            ss << CREATETENSOR(newName, ls.rdbuf(), oldName);
        }
        setVarToTypePG(newName, TypeDim(Variable::FLOAT, 2));
        return new Variable(ss.str(), Variable::VOID);
    }

    Variable *PostgresGenerator::generateDefineTensor(string tenName, vector<Accessor *> *accs, Variable *value) {
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

    Variable *PostgresGenerator::generateSaveTensorToDB(string newName, string oldName, vector<string> *cols) {
        stringstream ss;
        if(cols->empty()){
            ss << DELETETENSOR_DB(newName) << endl;
            ss << INSERTTENSOR_DB(newName,oldName);
        }else{
            generateCreateTable(newName ,cols);
            return new Variable("--TOD save to DB " + oldName, Variable::NONE);
        }
        return new Variable(ss.str(), Variable::VOID);

    }

    Variable *PostgresGenerator::generateIfElse(Variable *condition, string ifexps, string elseexps) {
        stringstream ss;
        ss << IF(condition->getContext(), ifexps);
        if (!elseexps.empty()) {
            ss << ELSE(elseexps);
        }
        ss << ENDIF;
        return new Variable(ss.str(), Variable::VOID);
    }

    Variable *PostgresGenerator::generateWhile(Variable *condition, string exps) {
        stringstream ss;
        ss << WHILE(condition->getContext(), exps);
        ss << ENDLOOP;
        return new Variable(ss.str(), Variable::VOID);
    }

    Variable *PostgresGenerator::generateFor(string name, Variable *from, Variable *to, string exps) {
        stringstream ss;
        ss << FOR(name, from->getContext(), to->getContext(), exps);
        ss << ENDLOOP;
        return new Variable(ss.str(), Variable::VOID);
    }


    Variable * PostgresGenerator::generateFuncCall(string fname, vector<Variable *> *parmlist, vector<string> *returnList,
                                        vector<TypeDim> *retTypes) {
        string typeName = "ML_" + to_string(parmlist->size()) + "_" + fname;
        if (typesToDrop.find(typeName) == typesToDrop.end() && returnList->size() > 1) {
            registerType(typeName, retTypes);
        }
        for (size_t i = 0; i < returnList->size(); i++) {
            setVarToTypePG((*returnList)[i], (*retTypes)[i]);
        }
        stringstream call;
        call << fname << "( ";
        string sep = "";
        for (Variable *v : *parmlist) {
            call << sep << v->getContext() << " ";
            sep = ",";
        }
        call << ")" << EXPEND;


        switch (returnList->size()) {
            case 0:
                return new Variable(call.str(), Variable::VOID);
            case 1:
                return new Variable((*returnList)[0] + " := " + call.str(), Variable::VOID);
            default: // >1
                stringstream ss;
                string resName = typeName + "_result";
                setVarToTypePG(resName, typeName);
                ss << resName << " := ";
                ss << call.rdbuf() << endl;

                char col = 'a';
                for (string const &s : *returnList) {
                    ss << s << " := " << resName << "." << col << EXPEND << endl;
                    col++;
                }
                return new Variable(ss.str(), Variable::VOID);
        }

    }

    string PostgresGenerator::typeToString(TypeDim td) {
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
                return string("");
        }
        for (size_t i = 0; i < td.second; i++) {
            res += "[]";
        }
        return res;
    }

    Variable *
    PostgresGenerator::generateDefineFunction(string fname, vector<string> *parmlist, vector<TypeDim> *parmTypes,
                                              vector<TypeDim> *retTypes, string exps) {
        stringstream ss;
        string function;
        function += fname; // += is more efficent than using +
        function += "(";
        string sep = "";
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
        //declare begin
        ss << declareVars().rdbuf();
        if (retTypes->size() > 1) {
            ss << ML_TAB << "ML_result_return" << " " << "ML_" << parmlist->size() << "_" << fname << EXPEND;
        }
        ss << BEGIN << endl;

        ss << exps << endl;

        ss << "END;" << endl;
        ss << "$$ LANGUAGE plpgsql;";
        return new Variable(ss.str(), Variable::VOID);
    }

    Variable *PostgresGenerator::generateReturn(vector<Variable *> *returns) {
        stringstream ss;
        switch (returns->size()) {
            case 0:
                ss << "return" << EXPEND;
                break;
            case 1:
                ss << "return " << (*returns)[0]->getContext() << EXPEND;
                break;
            default: // >1
                ss << "select ";
                string sep = "";
                for (Variable *v : *returns) {
                    ss << sep << v->getContext();
                    sep = ",";
                }

                ss << " into ML_result_return" << EXPEND << endl;
                ss << "return ML_result_return" << EXPEND;
        }
        return new Variable(ss.str(), Variable::VOID, 0);
    }



    string PostgresGenerator::generateBraceForList(string list) {
        string res = OPENARRAYBRACE;
        res += list;
        res += CLOSEARRAYBRACE;
        return res;
    }

    Variable *PostgresGenerator::generateTensorFromList(string list, size_t type, size_t oldDims) {
        stringstream ss;
        ss << CREATEARRAY << OPENARRAYBRACE << list << CLOSEARRAYBRACE;
        if(oldDims == -1){
            oldDims++;
            ss << "::float[]";
        }
        return new Variable(ss.str(), type, oldDims + 1);
    }

    Variable *PostgresGenerator::generateAccsessTensor(Variable *v, vector<Accessor *> *acc) {
        TypeDim td = v->getTypeDim();
        stringstream res;
        res << v->getContext();
        int dim = td.second;
        bool slicing = false;
        if (v->getDim() == acc->size()) {
            for (Accessor *ac : *acc) {
                slicing = slicing || !ac->isSingle();
            }
        } else {
            slicing = true;
        }


        for (Accessor *ac : *acc) {
            res << '[';
            if (ac->isSingle()) {
                res << ac->getFrom()->getContext() << " + 1";
                if (slicing) {
                    res << ':' << ac->getFrom()->getContext() << " + 1";
                }
                if (ac->getFrom()->getDim() != 0) {
                    throw ERROR("You can not access a tensor with a tensor");
                }
                dim--;
            } else if (ac->isFromTo()) {
                res << ac->getFrom()->getContext() << " + 1" << ":" << ac->getTo()->getContext() << " + 1";
                if (ac->getFrom()->getDim() != 0 || ac->getTo()->getDim() != 0) {
                    throw ERROR("You can not access a tensor with a tensor");
                }
            } else {
                res << ':';
            }
            res << ']';
        }
        for (size_t i = acc->size(); i < td.second; i++) {
            res << '[' << ':' << ']';
        }

        if (dim != 0) {
            //Sice SQL allows no dim reduction!
            dim = td.second;
        }
        return new Variable(res.str(), td.first, dim);
    }

    Variable *PostgresGenerator::generatePow(Variable *l, Variable *r) {
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

    Variable *PostgresGenerator::generateTranspose(Variable *l) {
        stringstream ss;
        ss << TRANSPOSE(l->getContext());
        return new Variable(ss.str(), l->getTypeDim());
    }

    Variable *PostgresGenerator::generateMul(Variable *l, Variable *r) {
        if ((l->getDim() != 0 && r->getDim() != 0) || (l->getDim() == 0 && r->getDim() == 0)) {
            return mathHelper(l, r, "*");
        } else {
            TypeDim td;
            int type2;
            //Swap them the rigth way
            stringstream ss;
            if (r->getDim() > 0) {
                ss << SCALAR(r->getContext(), l->getContext());
                td = r->getTypeDim();
                type2 = l->getType();
            } else if (l->getDim() > 0) {
                ss << SCALAR(l->getContext(), r->getContext());
                td = l->getTypeDim();
                type2 = r->getType();
            }
            //If one of both is float make it float!
            if (type2 == Variable::FLOAT) {
                td = TypeDim(Variable::FLOAT, td.second);
            }
            return new Variable(ss.str(), td);
        }
    }

    Variable *PostgresGenerator::generateBoolean(bool b) {
        return new Variable(b ? "True" : "False", Variable::BOOL);
    }

    Variable *PostgresGenerator::generateXor(Variable *l, Variable *r) {
        return mathHelper(l, r, "^");
    }

    string PostgresGenerator::generateComparisonEquals() {
        return string("=");
    }


    Variable *PostgresGenerator::generatePrint(vector<Variable *> *list) {
        stringstream ss;
        ss << PRINT;
        char sep = ' ';
        for (Variable *v : *list) {
            ss << sep << v->getContext();
            sep = ',';
        }
        ss << EXPEND;
        return new Variable(ss.str(), Variable::VOID);
    }

    Variable *PostgresGenerator::generateSetVar(string name, Variable *r, bool remember) {
        if (remember) {
            setVarToTypePG(name, r->getTypeDim());
        }
        stringstream ss;
        ss << SETVAR(name, r->getContext());
        return new Variable(ss.str(), Variable::VOID);
    }



    bool PostgresGenerator::setVarToTypePG(string name, TypeDim td) {
        if (setVarToType(name, td)) {
            nameToTypePG[name] = td;
            return true;
        } else {
            return false;
        }
    }

    bool PostgresGenerator::setVarToTypePG(string name, string type) {
        if (setVarToType(name, type)) {
            nameToTypeNamePG[name] = type;
            return true;
        } else {
            return false;
        }
    }

    Variable * PostgresGenerator::generatePlot(string fileName, Variable *xData, Variable *yData, string xLable, string yLable,
                                    string type) {
        stringstream ss;
        ss << PLOT(fileName, xData->getContext(), yData->getContext(), xLable, yLable, type);
        return new Variable(ss.str(), Variable::VOID);

    }

    Variable *PostgresGenerator::generateGradientDescent(string function, vector<string> *data, vector<pair<string,string>>* opt
            , Variable *stepSize, Variable *maxSteps, Variable *batchsize, Variable *threshold) {


        stringstream ss;

        string batchsizes;
        string maxStepss;
        string stepSizes;
        string thresholds;
        //Configure Parameters
        if (batchsize) {
            batchsizes = "1"; // Postgres only supports 1//batchsize->getContext();
        } else {
            batchsizes = "1"; // Stochastick GD is default
        }
        if (maxSteps) {
            maxStepss = maxSteps->getContext();
        } else {
            maxStepss = "100000";
        }
        if (stepSize) {
            stepSizes = stepSize->getContext();
        } else {
            stepSizes = "0.001";
        }
        if (threshold) {
            thresholds = threshold->getContext();
        } else {
            thresholds = "0.02";
        }

        string optstring;
        string placestring;

        char param = '0';
        string sep = "";
        ss << "select ";
        for(pair<string,string> &p : *opt){
            ss << sep << 'w' << param << " INTO " << p.first;
            optstring += sep;
            optstring += p.first;

            sep = ",";
            param++;
        }
        sep = "";
        for(string &d : *data){
            placestring += sep;
            placestring += d;
            sep = ",";
        }

        ss << " from gradientdescent('" << function << "','" << optstring << "','" << placestring << '\''
                << ',' << stepSizes << ',' << thresholds << ',' << maxStepss << ',' << batchsizes
                << ',' << optstring << ',' << placestring << ')' << EXPEND;


        return new Variable(ss.str(), Variable::VOID);
    }

};
