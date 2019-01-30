#pragma once

#define ML_MAIN "ML_main()"


#define END "$$ LANGUAGE 'hyperscript' strict; \n\nselect ML_main();\n"
#define START "CREATE OR REPLACE FUNCTION "<< ML_MAIN << " AS $$"

#define BEGIN "begin"
#define EXPEND ";"

#define TFLOAT "double precision"
#define TINT "double precision"//"integer"
#define TBOOL  "boolean"



#define LOAD(n , o) "select tensor as " << n << " from ML_TENSORS where id='" << o << "'"<< EXPEND
#define CREATETENSOR(n , l , o) "select ARRAY(select ARRAY["<< l << "]  from " << oldName <<  ")  as " << n << EXPEND
#define OPENARRAYBRACE "["
#define CLOSEARRAYBRACE "]"
#define CREATEARRAY "array"

#define DROPTABLE(n) "DROP TABLE " << n << EXPEND
#define DROPFUNCTION(n) "DROP FUNCTION " << n << EXPEND
#define DROPTYPE(n) "DROP TYPE " << n << EXPEND


#define MATRIXPOW(l,r) "power(" << l << ", "<<  r << "::integer )";
#define POW(l,r)  l << "^("<<  r << ")";


#define SETVAR(name , r ) name << " = " << r << EXPEND
#define DEFVAR(name , r , td) "var " << name << " " /*<< typeToString(td)*/  << " = " << r << EXPEND
#define DEFVARWITHTYPE(name , r , td) "var " << name << " " << typeToString(td)  << " = " << r << EXPEND
#define DEFVARSTR(name , r , type ) "var " << name << " " << type  << " = " << r << EXPEND

#define TRANSPOSE(l) "array_transpose(" << l << ")"


#define IF(c , exps) "if( " << c << " ){" << endl << exps << endl <<"}"
#define ELSE(exps) endl <<  "else{ "<< endl << exps << endl <<"}"


#define WHILE(c , exps) "while( " << c << " ){" << endl << exps << endl << "}"
#define FOR( n, f , t , exps) "select index as " << n << " from sequence(" << f << ", "<< t << "- 1 ) {" << endl << exps << endl << "}"

#define DEFFUNCTION(n) "CREATE OR REPLACE FUNCTION " << n


#define PRINT "debug_print"
#define FILLARRAY_BEGIN(v) "array_fill( " << v << "::double precision , "
#define FILLARRAY_END ")"


#define SLICE "array_resetlower(array_slice"
#define ACCESS "array_access"
#define LEN "array_length"
