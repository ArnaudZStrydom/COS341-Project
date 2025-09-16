"halt"      { return HALT; }
"print"     { return PRINT; }
"return"    { return RETURN; }
"while"     { return WHILE; }
"do"        { return DO; }
"until"     { return UNTIL; }
"if"        { return IF; }
"else"      { return ELSE; }

"neg"       { return NEG; }
"not"       { return NOT; }
"eq"        { return EQ; }
">"         { return GT; }
"or"        { return OR; }
"and"       { return AND; }
"plus"      { return PLUS; }
"minus"     { return MINUS; }
"mult"      { return MULT; }
"div"       { return DIV; }

/* Keywords for parser structure */
"glob"      { return GLOB; }
"proc"      { return PROC; }
"func"      { return FUNC; }
"main"      { return MAIN; }
"local"     { return LOCAL; }
"var"       { return VAR_KW; }

/* Numbers */
[0-9]+      { yylval = atoi(yytext); return NUMBER; }

/* Strings */
\"[^\"]*\"  { return STRING; }

/* Variable names (anything else alphabetic) */
[a-zA-Z_][a-zA-Z0-9_]*  { return USERNAME; }

/* Whitespace */
[ \t\n]     ;  

/* Any other character */
.           { return yytext[0]; }
