CFLAGS=-std=c99 -pedantic -Wall -Wextra -Wconversion -D_XOPEN_SOURCE=700 -march=native -O3
LDLIBS=-lm -lfl

.PHONY: all
all: nfasm.exe

.PHONY: scanner
scanner: lex.yy.c

nfasm.exe: NeonFox_Assembler.o lex.yy.o scannerhelper.o preprocessor.o

lex.yy.o: lex.yy.c

lex.yy.c:
	flex NeonFox.lex

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.exe:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

# lib%.a: %.o
# 	$(AR) -rs $@ $^

.PHONY: clean
clean:
	$(RM) -- *.exe *.a *.o lex.yy.c
