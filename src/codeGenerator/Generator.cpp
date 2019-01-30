#include "codeGenerator/Generator.h"
#include "helper/Helpers.h"

namespace CodeGenerator{


    /**
     * Concats the context of all Variables of the list  (comma seperated) and returns it.
     * @param call the stream to pipe the list in
     * @param list th list of Variables
     */
    string Generator::generateList( vector<Variable*>* list){
        string sep;
        string res;
        for(Variable* v : *list){
            res += sep;
            res += v->getContext();
            res += " ";
            sep = ",";
        }
        return res;
    }


    /**
     * Sets a variable name to a specific type
     * @param name The name of the variable
     * @param td The type and dimension information to set the variable to
     * @return false if var already exists/ true if not
     */
    bool Generator::setVarToType(string name , TypeDim td){
        if( nameToType.find(name) != nameToType.end()){
            return false;
        }else{
            nameToType.insert({name , td });
            return true;
        }
    }
    /**
     * Sets a variable name to a specific cunstomly defined type
     * @param name The name of the variable
     * @param td The type to set the variable to
     * @return false if var already exists/ true if not
     */
    bool Generator::setVarToType(string name  ,string type){
        if(nameToTypeName.find(name) != nameToTypeName.end()){
             return false;
        }else{
            nameToTypeName.insert({name , type});
            return true;
        }
    }

    /**
     * Removes a Variable of the map
     * @parm name The name of the variable to remove
     * @return if the variable we wanted to remove did exist
     */
    bool Generator::removeVar(string name){
        return (nameToTypeName.erase(name) + nameToType.erase(name)) > 0;
    }


    /**
     * Returns the type of the Generator.
     * @return Either MLParser::PYTHON, MLParser::POSTGRES, or MLParser::HYPER
     */
    size_t Generator::getType(){
        return type;
    }



    /**
     * This is a shortcut mehtod that creates a simple math operation Variabel
     * @param l The left side of the operator
     * @@param l  The right side of the operator
     * @return The newly created Variable
     */
    Variable* Generator::mathHelper(Variable* l , Variable* r , string s){
        if((l->getDim() != r->getDim() && s != "*")
            || ( s == "*" && l->getDim() != r->getDim() && l->getDim() != 0 && r->getDim() != 0) ){
            throw ERROR("Dimensions of * product does not match!");
        }
        stringstream ss;
        ss << l->getContext() << " " << s << " " << r->getContext();
        int type = l->getType();
        if( r->getType() == Variable::FLOAT ){
            type = r->getType();
        }
        int dim = max(l->getDim() , r->getDim());
        return new Variable(ss.str() , TypeDim(type , dim));
    }


    Variable* Generator::generateSub(Variable* l , Variable* r){
        return mathHelper(l,r,"-");
    }
    Variable* Generator::generateAdd(Variable* l , Variable* r){
        return mathHelper(l,r,"+");
    }

    Variable* Generator::generateAnd(Variable *l, Variable *r) {
        return mathHelper(l,r,"and");
    }

    Variable* Generator::generateOr(Variable *l, Variable *r) {
        return mathHelper(l,r,"or");
    }

    Variable* Generator::generateDiv(Variable* l , Variable* r){
        stringstream res;
        int type;
        if(l->getDim() == 0 && r->getDim() == 0){
            res << l->getContext() << "/" << r->getContext();
        }else if (r->getDim() > 0){
            throw ERROR("Can not divide by tensor");
        }else{// if (l->getDim()>0 && r->getDim() == 0)
            res << l->getContext() << "* (1/" << r->getContext() << ")";
        }

        type = l->getType() == Variable::FLOAT || r->getType() == Variable::FLOAT ? Variable::FLOAT :Variable::INTEGER ;
        return new Variable(res.str() , type , 0);
    }

    Variable* Generator::generateNeg(Variable* r){
        return new Variable('-' + r->getContext() , r->getTypeDim());
    }


    // Will be overriden by hyper because hyper has array access not implemented jet
    Variable* Generator::generateSetAccesedTensor(string tenName , vector<Accessor*>* acc , Variable* toSet ){
        //Variable* accesdTen = generateAccsessTensor(generateVariable(tenName) , acc);

        //I dont wank to copy the hole methode again to each code generator yust beacuse the offset
        string offset;
        if(getType() != MLParser::PYTHON){
            offset = " + 1";
        }


        bool slicing = false;
        for (Accessor *ac : *acc) {
            slicing = slicing || !ac->isSingle();
        }

        stringstream res;
        res << tenName;
        for (Accessor *ac : *acc) {
            res << '[';
            if (ac->isSingle()) {
                res << ac->getFrom()->getContext() << offset;
                if (slicing) {
                    res << ':' << ac->getFrom()->getContext() << offset;
                }
                if (ac->getFrom()->getDim() != 0) {
                    throw ERROR("You can not access a tensor with a tensor");
                }
            } else if (ac->isFromTo()) {
                res << ac->getFrom()->getContext() << " + 1" << ":" << ac->getTo()->getContext() << offset;
                if (ac->getFrom()->getDim() != 0 || ac->getTo()->getDim() != 0) {
                    throw ERROR("You can not access a tensor with a tensor");
                }
            } else {
                res << ':';
            }
            res << ']';
        }



        if(toSet){
            Variable* resv = generateSetVar(res.str() , toSet , false);
            return resv;
        }else{
            return new Variable(res.str(), Variable::NONE); // We do not know the type!
        }
    }

    /**
     * Converts a variable name into a Variable object.
     * @param name
     * @return The newly created Variable object
     * @throws string if Variable has not be defined
     */
    Variable* Generator::generateVariable(string name) {
        if(nameToType.find(name) == nameToType.end()){
            throw ERROR("Variable " + name + " not defined");
        }
        TypeDim p = nameToType[name];
        return new Variable(name, p);
    }


    /**
     * Creats a new csv file that consits only consits of coulms defined in names
     * @param filename The filname of the original csv file
     * @param names The string consisting of comma spereated collums that will be selected in the output file
     * @param del The delimeter of the csv file
     * @param replaceList Each first parameter of the list will be repoalced by its second parameter in the csv file
     */
    void Generator::createvsv(string filename , string names , string del , vector<pair<string, string>> *replaceList) {
        stringstream commandLine;
        commandLine << "cat " << filename;
        //Replace Values with awk
        if (!replaceList->empty()) {
            commandLine << " | awk '";
            for (pair<string, string> p : *replaceList) {
                commandLine << "{gsub(/" << p.first << "/,\"" << p.second << "\");";
            }
            commandLine << "}1' ";
        }

        //Cut of the correct columns

        commandLine << " | csvcut -d " << del << " -c " << names << "  >" << filename << ".ml.csv" << endl;
        //Kind of Hacky but works!
        string commandLineString = commandLine.str();
        exec(commandLineString);
    }

};



