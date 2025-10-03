# COS341-Project
COS341 Project (compiler)
```
Build parser
arnaudzs@ArnaudsPC:~/COS341/COS341-Project$ bison -d spl.y
Build lexer and parser and entrance point
arnaudzs@ArnaudsPC:~/COS341/COS341-Project$ g++ main.cpp spl.tab.cpp spl_lexer.cpp lexer_bridge.cpp -o spl_compiler
run testfile
arnaudzs@ArnaudsPC:~/COS341/COS341-Project$ ./spl_compiler tests/valid/valid_program.txt
```