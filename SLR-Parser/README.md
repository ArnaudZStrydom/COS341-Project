# SLR Parser using Bison

## Requirements
```bash
sudo apt install bison flex gcc
```
## To run
```bash
bison -d spl-grammar.y
flex spl-lexer.l
gcc spl.tab.c lex.yy.c -o spl -lfl
./spl
```

## There is also make file
```bash
make
./spl
# enter input
make clean
```