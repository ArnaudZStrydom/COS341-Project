# Compiler and tools
CC = gcc
BISON = bison
FLEX = flex

# Files
TARGET = spl
PARSER = spl-grammar.y
LEXER = spl-lexer.l
PARSER_C = spl-grammar.tab.c
PARSER_H = spl-grammar.tab.h
LEXER_C = lex.yy.c

all: $(TARGET)

$(TARGET): $(PARSER_C) $(LEXER_C)
	$(CC) $(PARSER_C) $(LEXER_C) -o $(TARGET) -lfl

$(PARSER_C) $(PARSER_H): $(PARSER)
	$(BISON) -d $(PARSER)

$(LEXER_C): $(LEXER)
	$(FLEX) $(LEXER)

clean:
	rm -f $(TARGET) $(PARSER_C) $(PARSER_H) $(LEXER_C)
