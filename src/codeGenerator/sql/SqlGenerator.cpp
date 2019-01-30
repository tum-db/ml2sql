#include "codeGenerator/sql/SqlGenerator.h"



namespace CodeGenerator {

    list<string> SqlGenerator::functionsToDrop;

    /**
     * Creates a new type in sql. This may bes used the supoort muliple returns of a function.
     * @param tname The typename of the new type.
     */
    void SqlGenerator::registerType(string tname, vector<TypeDim> *types) {
        create << "CREATE TYPE " << tname << " AS (";
        char col = 'a';
        string sep;
        for (TypeDim const &i : *types) {
            create << sep << col << " " << typeToString(i);
            col++;
            sep = ",";
        }
        create << ");" << endl;
        typesToDrop.insert(tname);
    }

    Variable* SqlGenerator::generateRandom() {
        return new Variable("random()", Variable::FLOAT, 0);
    }


    Variable* SqlGenerator::generateNum(string num , int type){
        if(type == Variable::INTEGER){
            return new Variable(num , type );
        }else{
            return new Variable(num + "::double precision", type );
        }

    }


    Variable* SqlGenerator::generateAppend(Variable *l, Variable *r) {
        stringstream res;
        if (l->getDim() == r->getDim() && r->getDim() > 0) {
            res << "array_cat(";
        } else if (l->getDim() == r->getDim() + 1 && l->getDim() > 0) {
            res << "array_append(";
        } else if (l->getDim() + 1 == r->getDim() && r->getDim() > 0) {
            res << "array_prepend(";
        } else {
            throw ERROR("Appending of those elements is not possible");
        }
        res << l->getContext() << "," << r->getContext() << ")";
        return new Variable(res.str(), l->getTypeDim());
    }

    string SqlGenerator::generateCreateTable(string tableName ,vector<string> *cols ){
        create << "create table " << tableName << '(';
        string sep;
        string names;
        for (const string &s : *cols) {
            create << sep << endl;
            create << ML_TAB << s << " DOUBLE PRECISION";
            names += sep;
            names += s;
            sep = ",";
        }
        create << endl << ");" << endl;
        return names;
    }

    Variable* SqlGenerator::generateSaveTensorToCSV(string oldName, string filename, vector<string> *l, string del) {

        //Save tensor to DB to use COPY TO  statement
        string tablename = "ML_"+oldName;
        tablesToDrop.insert(tablename);
        Variable* createTable = generateSaveTensorToDB(oldName , tablename , l);
        stringstream ss;
        ss << createTable->getContext();
        ss << "COPY " << tablename <<" TO " << filename << "DELIMITER " << del << " CSV HEADER;";
        return new Variable(ss.str(), Variable::VOID);
    }



    Variable* SqlGenerator::generateReadCsv(string tableName, string filename, vector<string> *cols, string del, bool deleteEmpty,
                                  Variable *replaceEmptys, vector<pair<string, string>> *replaceList, bool createCsv, bool useModified ) {


        tablesToDrop.insert(tableName);

        //Create table to store csv date in
        string names = generateCreateTable(tableName ,cols );
        create << endl;
        //Copy csv into tabel

        //This is only to make it run on scyper 16
        if(getType() == MLParser::POSTGRES){
            create <<"\\";
        }

        create << "COPY " << tableName << '(' << names << ") ";
        create << "FROM '" << filename;
        if(useModified){
            create << ".ml.csv";
        }
        create << "'";
        create << "WITH DELIMITER " << del << " CSV HEADER;" << endl;

        if (deleteEmpty) {
            //delte all rows with any NULL entry
            create << "delete from " << tableName << " where (";
            string sep;
            for (const string &s : *cols) {
                create << sep << '(' << s << " is null) ";
                sep = "or ";
            }
            create << ");" << endl;
        } else {
            //replace all NULL entries with value;
            string repEmpString = replaceEmptys->getContext();
            for (const string &s : *cols) {
                create << "UPDATE " << tableName << " SET " << s << " = " << repEmpString << " WHERE " << s
                       << "  IS NULL;" << endl;
            }
        }
        create << endl;

        //Build and Execute command line expression to modify csv file!
        if(createCsv){
            createvsv(filename , names , del , replaceList);
        }


        return new Variable("", Variable::VOID);
    }
};