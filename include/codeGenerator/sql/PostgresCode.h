#pragma once

#define INITFILE "postgresql/PostgresGenerator_Init.sql"
#define CLEANUPFILE "postgresql/PostgresGenerator_Cleanup.sql"

#define END "END$$;"
#define START "DO $$"

#define DECLARE "declare"
#define BEGIN "begin"
#define EXPEND ";"

#define TFLOAT "DOUBLE PRECISION"
#define TINT "DOUBLE PRECISION"
#define TBOOL  "boolean"


#define LOAD(n , o) "select tensor into " << n << " from ML_TENSORS where id='" << o << "'"<< EXPEND
#define CREATETENSOR(n , l , o) "select ARRAY(select ARRAY["<< l << "]  from " << o <<  ")  into " << n << EXPEND
#define OPENARRAYBRACE "["
#define CLOSEARRAYBRACE "]"
#define CREATEARRAY "array"

#define DELETETENSOR_DB(newName) "DELETE FROM ML_TENSORS WHERE id='" << newName << "'" << EXPEND
#define INSERTTENSOR_DB(newName,oldName) "INSERT INTO ML_TENSORS VALUES ('" << newName << "'," << oldName << ")" << EXPEND

#define DROPTABLE(n) "DROP TABLE " << n << EXPEND
#define DROPFUNCTION(n) "DROP FUNCTION " << n << EXPEND
#define DROPTYPE(n) "DROP TYPE " << n << EXPEND

#define SETVAR(name , r ) name << " := " << r << EXPEND
//#define MUL(l , r) "matrix_multiplication("<< l << ","<< r << ")"
//#define ADD(l, r) "matrix_addition("<< l << ","<< r << ")"
#define SCALAR(l , r) "tensor_scalar(" << l << ',' << r << ')'
#define MATRIXPOW(l,r) "matrix_power(" << l << ", "<<  r << "::integer )";
#define POW(l,r)  l << "^("<<  r << ")";


#define TRANSPOSE(l) "matrix_transpose(" << l << ")"

#define IF(c , exps) "IF " << c << " THEN" << endl << exps
#define ELSE(exps) endl <<  "ELSE "<< endl << exps
#define ENDIF endl << "END IF" << EXPEND

#define WHILE(c , exps) "WHILE " << c << " LOOP" << endl << exps
#define FOR( n, f , t , exps) "FOR " << n << " IN " << f << "..("<< t << "-1) LOOP" << endl << exps
#define ENDLOOP endl << "END LOOP" << EXPEND

#define DEFFUNCTION(n) "CREATE OR REPLACE FUNCTION " << n

#define FILLARRAY_BEGIN(v) "array_fill( " << v << "::double precision, ARRAY["
#define FILLARRAY_END "])"
#define PRINT "RAISE NOTICE"
#define PLOT(fileName , xData , yData , xLable, yLable, type ) "plot_data(" << fileName << ", " << xData << ", " \
            << yData << ", " << xLable << ", " <<  yLable  << ", " << type <<")"


