CFLAGS=-std=c99 -pedantic -Wall -Wextra -Wconversion -D_XOPEN_SOURCE=700 -march=native -O3
LDLIBS=-lm -lfl

.PHONY: all

all: nfasm.exe

nfasm.exe: NeonFox_Assembler.o lex.yy.o scannerhelper.o preprocessor.o

%.exe:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

lib%.a: %.o
	$(AR) -rs $@ $^

.PHONY: clean
clean:
	$(RM) -- *.exe *.a *.o
