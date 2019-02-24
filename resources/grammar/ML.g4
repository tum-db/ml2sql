grammar ML;

file
	: expressionlist=explist EOF   #file_compile
	;

explist
	:(NEWLINE+ expressions+=expression)* NEWLINE*
	;



expression
	: INJECT                                                                                          	#expression_inject
	| CREATE TENSOR newName=VARNAME FROM oldName=VARNAME
			OPEN_B columns=varlist CLOSE_B      								        #expression_createTensor
	| name=VARNAME list=accsesslist ALL value=mathexp                                                   #expression_defineTensor
	| name=VARNAME list=accsesslist SAMPLE dname=VARNAME
	        OPEN_B ( parms+=mathexp (KOMMA parms+=mathexp)*)? CLOSE_B                                   #expression_sampleTensor
	| SAVE TENSOR tenName=VARNAME TO dbName=(VARNAME|STRING)
			(ALL del=STRING)? OPEN_B columns=varlist CLOSE_B       							            #expression_saveTensor
	| left=VARNAME list=accsesslist  EQUALS rigth=mathexp												#expression_setAccessedTensor
	| IF OPEN_B condition=mathexp CLOSE_B OPEN_C ifExpressionList=explist
		CLOSE_C (ELSE OPEN_C elseExpressionList=explist CLOSE_C)?                                     	#expression_if
	| WHILE OPEN_B condition=mathexp CLOSE_B OPEN_C expressionList=explist CLOSE_C                    	#expression_while
	| FOR name=VARNAME FROM from=mathexp TO to=mathexp OPEN_C expressionList=explist CLOSE_C          	#expression_for
	| FOR name=VARNAME IN values=interval OPEN_C expressionList=explist CLOSE_C                         #expression_for_interval
	| word=(CONTINUE | BREAK)                                                                         	#expression_keyword
	| (res+=VARNAME (KOMMA res+=VARNAME)+  EQUALS)? 													//The plus ensures that the res.size == 1 gets handeld in setvar with mathexp
		funcName=VARNAME OPEN_B ( parms+=mathexp (KOMMA parms+=mathexp)*)? CLOSE_B                      #expression_functionCall
	| ret+=returnType* FUCNTION funcName=VARNAME
		OPEN_B parms+=VARNAME? (KOMMA parms+=VARNAME)*  CLOSE_B OPEN_C  expressionList=explist  CLOSE_C	#expression_functionDef
	| fun=functions																						#expression_externFunction
	| RETURN  ret+=mathexp (KOMMA ret+=mathexp)* NEWLINE*                                             	#expression_return
	| left=VARNAME EQUALS right=mathexp                                                               	#expression_setVar
	| var=VARNAME EQUALS DISTRIBTION OPEN_B sample=VARNAME KOMMA pr=VARNAME CLOSE_B					  	#expression_setDist
	| var=VARNAME SAMPLE dname=VARNAME OPEN_B ( parms+=mathexp (KOMMA parms+=mathexp)*)? CLOSE_B	  	#expression_sampleDist
	| PRINT toPrint=mathexplist																			#expression_print
	| op=(READCSV|WRITECSV)
		OPEN_C
			(
				(  (NEWLINE )+ 'name:'  outname=VARNAME     )
				| ((NEWLINE )+ 'file:'   filename=STRING     )
				| ((NEWLINE )+ 'columns:'   columns=varlist     )
				| ((NEWLINE )+ 'delimiter:'   delimiter=STRING     )
				| ((NEWLINE )+ 'replace:'   OPEN_C   NEWLINE*   (rep+=STRING ALL by+=STRING   NEWLINE*  )+ CLOSE_C )
				| ((NEWLINE )+ 'delete empty entries')
				| ((NEWLINE )+ 'replace empty entries:'  repEmptyBy=mathexp)
			)+	(NEWLINE )+
        CLOSE_C																							#expression_readcsv
    | GRADIENTDESCENT
        OPEN_C
        (
            (  (NEWLINE )+ 'function:'   fun=STRING)
            | ((NEWLINE )+ 'data:'   data=varlist )
            | ((NEWLINE )+ 'optimize:'   (opt+=nameshape (KOMMA opt+=nameshape)*)? )
            | ((NEWLINE )+ 'learningrate:'   stepSize=mathexp)
            | ((NEWLINE )+ 'maxsteps:'   maxSteps=mathexp)
            | ((NEWLINE )+ 'batchsize:'   batchSize=mathexp)
            | ((NEWLINE )+ 'threshold:'   threshold=mathexp)
        )+ (NEWLINE)+
        CLOSE_C                                                                                         #expression_gradientDescent
    | PLOT
        OPEN_C
        (
            (  (NEWLINE )+ 'xData:'   x=mathexp)
            | ((NEWLINE )+ 'yData:'   y=mathexp)
            | ((NEWLINE )+ 'xLable:'   xLable=STRING)
            | ((NEWLINE )+ 'yLable:'   yLable=STRING)
            | ((NEWLINE )+ 'type:'   type=STRING)
            | ((NEWLINE )+ 'filename:'   filename=STRING)
        )+ (NEWLINE)+
        CLOSE_C                                                                                         #expression_plot
	;

mathexp
	: left=mathexp (DOT|POW) T                      #mathexp_transpose
	| ten=mathexp acc=accsesslist 					#mathexp_accsesstensor
	| left=mathexp POW right=mathexp                #mathexp_pow
	| SUB right=mathexp                             #mathexp_neg
	| left=mathexp op=(MUL|DIV) right=mathexp       #mathexp_mul
	| left=mathexp op=(ADD|SUB) right=mathexp       #mathexp_add
	| OPEN_B expr=mathexp CLOSE_B                   #mathexp_brace
	| OPEN_A (NEWLINE)? list=mathexplist (NEWLINE)? CLOSE_A              #mathexp_tensor
	| funcName=VARNAME 	OPEN_B ( parms+=mathexp
		(KOMMA parms+=mathexp)*)? CLOSE_B			#mathexp_functionCall
	| var=VARNAME                                   #mathexp_var
	| num=(INTEGER|FLOAT)                           #mathexp_num
	| op=(TRUE|FALSE)                               #mathexp_bool
	| left=mathexp
		op=(GREATHER|GREATHEREQUALS|LESS|LESSEQUALS|EQUALSEQUALS|UNEQUALS)
			right=mathexp                           #mathexp_compare
	| left=mathexp op=(AND|OR|XOR) right=mathexp    #mathexp_boolop
	| NOT left=mathexp                              #mathexp_boolNot
	| left=mathexp APPEND right=mathexp				#mathexp_append
	| str=STRING									#mathexp_string
	;


functions
    :  NEWLINE* ret+=returnType* EXTERN fname=VARNAME OPEN_B parmnum=INTEGER CLOSE_B
        (OPEN_C  // ispresent is only to check if the seond part is present!
			(NEWLINE* targets+=(POSTGRES|PYTHON|HYPER) EQUALS names+=externFunName)+
			NEWLINE*
		CLOSE_C)?
    ;

returnType
	: type=(INTT|FLOATT|BOOLT)
	;

externFunName
	: (VARNAME DOT?)+
	;


mathexplist
	: (expressions+=mathexp (KOMMA (NEWLINE)? expressions+=mathexp)*)?
	;


accsesslist
	: OPEN_A expressions+=accessor (KOMMA expressions+=accessor)* CLOSE_A
	//| OPEN_A expressions+=accessor CLOSE_A  (OPEN_A expressions+=accessor CLOSE_A )*
	;

accessor
	: ac=mathexp                    #accessor_mathexp
	| from=mathexp ALL to=mathexp   #accessor_fromto
	| ALL                           #accessor_all
	;


interval
    :   ten=mathexp
    |   OPEN_A from=mathexp SEP to=mathexp CLOSE_A (ALL num=mathexp (ALL fname=VARNAME)? )?
    ;


varlist
	: (names+=VARNAME (KOMMA names+=VARNAME)*)?
	;


nameshape
	: name=VARNAME (shape=accsesslist)?
	;







//Boolean stuff
GREATHER        : '>';
LESS            : '<';
LESSEQUALS      : '<=';
GREATHEREQUALS  : '>=';
UNEQUALS        : '!=';
EQUALSEQUALS    : '==';

TRUE    : 'true';
FALSE   : 'false';
AND     : ('and'|'&&'|'&');
OR      : ('or'|'||'|'|');
XOR     : ('xor');
NOT     : ('not'|'!');


//Mathops
ADD : '+' ;
DIV : '/' ;
SUB : '-' ;
MUL : '*' ;
EQUALS : '=' ;
POW : '^';
SAMPLE : '~';
APPEND : '::';


//Target
POSTGRES: 'postgres';
PYTHON  : 'python';
HYPER	: 'hyper';

//Keywords

READCSV : 'readcsv';
WRITECSV : 'writecsv';
PLOT : 'plot';
GRADIENTDESCENT : 'gradientdescent';

INJECT : 'inject""' .*? '""'  ;
CREATE  : 'create' ;
SAVE    : 'save';
PRINT    : 'print';
TENSOR  : 'tensor';
FROM    : ('from'|'<-');
TO      : ('to'|'->');
FUCNTION : 'function';
EXTERN : 'extern';
RETURN  : 'return';
CONTINUE : 'continue';
BREAK   : 'break';

IF      : 'if';
ELSE    : 'else';
WHILE   : 'while';
FOR     : 'for';
IN      : 'in';


//TYPES
INTT : 'int'[0-9]*;
FLOATT : 'float'[0-9]*;
BOOLT : 'bool'[0-9]*;

DISTRIBTION : 'distribution';

//internfunctions:

//Braces
OPEN_A  : '[';
CLOSE_A : ']';
OPEN_B  : '(';
CLOSE_B : ')';
OPEN_C  : '{';
CLOSE_C : '}';

KOMMA   : ',';
DOT     : '.';
ALL     : ':';
SEP     : ';';
T       : 'T';
//
VARNAME     : [a-zA-Z$_][a-zA-Z0-9$_]*;
INTEGER     : [0-9]+;
FLOAT       : ([0-9]+([.][0-9]*)?|[.][0-9]+);
STRING      : '\''  (~['"])* '\'';


//Whitspaces and Comments

NEWLINE     : ('\r'? '\n' | '\r')+ ;
TAB         : ('\t' | '        ' | '    ' )  -> skip;
WHITESPACE  : ' ' -> skip ;


COMMENT
	:   '/*' .*? '*/' -> skip
	;

LINE_COMMENT
	:   '//' ~[\r\n]* -> skip
	;
LINE_COMMENT2
	:   '#' ~[\r\n]* -> skip
	;

