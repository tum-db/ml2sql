grammar Pre;

file: exps+=expression*  EOF
	;

expression:
	'crossvalidate' WS?
		'{'
            WS?  
			( 
				( (NEWLINE WS?)+ 'minfun' WS? ':'WS? wname=VARNAME WS? '=' WS? minfun=fun WS?    )
				| ((NEWLINE WS?)+ 'lossfun' WS? ':'WS? lossfun=fun WS?    )
				| ((NEWLINE WS?)+ 'kernel'  WS?  ':' WS? designfun=VARNAME WS? ':'WS?  designfunData=VARNAME ':'WS?  designMaxIndex+=~NEWLINE*?   WS? )
				| ((NEWLINE WS?)+ 'data' WS?  ':' WS?  foldNames+=VARNAME ( WS? ',' WS? foldNames+=VARNAME)* WS?   )
				| ((NEWLINE WS?)+ 'folds'WS?  ':' WS? folds+=~NEWLINE*?  WS? )
				| ((NEWLINE WS?)+ 'n'    WS?  ':' WS? n+=~NEWLINE*? WS? )
				| ((NEWLINE WS?)+ 'test' WS?  '{' WS?  NEWLINE* WS?  (testNames+=VARNAME WS? '=' WS?  testIntervals+=interval WS?  NEWLINE* WS? )+ '}' WS?     )
			)+	(NEWLINE WS?)+																		
        '}'		  	#exp_cross
    | 'import' WS? libname=VARNAME #exp_laodLib
	| line			#exp_line
	| NEWLINE		#exp_line
	;

fun
    : fname=VARNAME WS? '(' (WS? parms+=VARNAME WS? (','WS? parms+=VARNAME)*)? WS?')' 
	;
interval:
	('['|']'|','|';'|'.'|':'|INTEGER|FLOAT|VARNAME|WS)+
	;

line 
	: ~NEWLINE+ NEWLINE
	;

VARNAME     : [a-zA-Z$_][a-zA-Z0-9$_]*;
INTEGER     : [0-9]+;
FLOAT       : ([0-9]+([.][0-9]*)?|[.][0-9]+);
STRING      : '"'  (~['"])* '"';


//Whitspaces and Comments

NEWLINE     : ('\r'? '\n' | '\r')+ ;
WS  : ('\t' | '        ' | '    '|' ' )+;



COMMENT
	:   '/*' .*? '*/' -> skip
	;

LINE_COMMENT
	:   '//' ~[\r\n]* -> skip
	;
LINE_COMMENT2
	:   '#' ~[\r\n]* -> skip
	;

ANY  : ~[\r\n] ;