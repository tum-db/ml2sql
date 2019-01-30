grammar tf;

file
	: exp=mathexp EOF   #file_compile
	; 




mathexp 
	: left=mathexp (DOT|POW) T                      #mathexp_transpose 
	| left=mathexp POW right=mathexp                #mathexp_pow
	| SUB left=mathexp                              #mathexp_neg
	| left=mathexp op=(MUL|DIV) right=mathexp       #mathexp_mul
	| left=mathexp op=(ADD|SUB) right=mathexp       #mathexp_add
	| OPEN_B expr=mathexp CLOSE_B                   #mathexp_brace
	| OPEN_A  list=mathexplist CLOSE_A              #mathexp_tensor
	| var=VARNAME                                   #mathexp_var
	| num=(INTEGER|FLOAT)                           #mathexp_num
	| fname=VARNAME OPEN_B parms=mathexplist CLOSE_B      #mathexp_fun
	;


mathexplist
	: expressions+=mathexp (KOMMA expressions+=mathexp)*   
	;



//Mathops
ADD : '+' ;
DIV : '/' ;
SUB : '-' ;
MUL : '*' ;
EQUALS : '=' ;
POW : '^';

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

VARNAME     : [a-zA-Z$_][a-zA-Z0-9$_]*;
INTEGER     : [0-9]+;
FLOAT       : ([0-9]+([.][0-9]*)?|[.][0-9]+);

//Whitspaces 

NEWLINE     : ('\r'? '\n' | '\r')  -> skip;
TAB         : ('\t' | '        ' | '    ' )  -> skip;
WHITESPACE  : ' ' -> skip ;
