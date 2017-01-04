#!/bin/bash
top=.

all:python/pyzdcp.so perl/plzdcp.so java/libzdcpSwigInterface.so

python/pyzdcp.so:
	swig -python -o python/zdcp.c -outdir python/ -I$(top)/../src/lib $(top)/zdcp.i
	gcc -fPIC -shared -Wall -Wl,-soname=pyzdcp.so -o$@ python/zdcp.c -lzdcp $(shell python-config --cflags) -L$(top)/../lib -I$(top)/../src/lib

perl/plzdcp.so:
	swig -perl5 -o perl/zdcp.c -outdir perl/ -I$(top)/../src/lib $(top)/zdcp.i
	gcc -fPIC -shared -Wall -Wl,-soname=plzdcp.so -o$@ perl/zdcp.c -lzdcp -L$(top)/../lib -I$(top)/../src/lib $(shell perl -MConfig -e 'print "-I$$Config{archlib}/CORE $$Config{ccflags}"')

java/libzdcpjava.so:
	swig -java -o java/zdcpswig_wrap.c -outdir java/ -I$(top)/../src/lib $(top)/zdcp.i 
	gcc -fPIC -shared -Wall -Wl,-soname=libzdcpjava.so -o$@ java/zdcpswig_wrap.c -lzdcp -L$(top)/../lib -I$(top)/../src/lib
#	swig -java -o java/zdcpswig_wrap.c -outdir java/ -I$(top)/../src/lib -package com.alibaba.zdcp $(top)/zdcp.i 
clean:
	rm -rf python/pyzdcp.so perl/plzdcp.so java/libzdcpjava.so
