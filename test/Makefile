all: regress

regress: regress.c
	gcc -I../src/lib -L../lib -O2 -Wall -o regress regress.c -lzdcp
