.SUFFIXES:

sintact: main.o Semantico.tab.o TS.o SemanticoGest.o
	gcc -o Semantico main.o Semantico.tab.o lex.yy.o TS.o SemanticoGest.o

Semantico.tab.o: Semantico.tab.c lex.yy.c TS.h SemanticoGest.h
	gcc -c Semantico.tab.c
	gcc -c lex.yy.c

TS.o: TS.c TS.h
	gcc -c TS.c

SemanticoGest.o: SemanticoGest.c SemanticoGest.h TS.h
	gcc -c SemanticoGest.c

main.o: main.c
	gcc -c main.c

Semantico.tab.c: Semantico.y lex.yy.c
	bison -v -d Semantico.y

lex.yy.c: Lexico.lex
	flex -l Lexico.lex

clean:
	rm -f Semantico main.o Semantico.tab.o Semantico.tab.c lex.yy.c lex.yy.o Semantico.tab.h TS.o SemanticoGest.o Semantico.output
	rm -f salida/dec_fun.h salida/main.c salida/programa

all:
	make clean
	make sintact

test: sintact
	bash tests/run.sh
