APXS=/opt/apache2/bin/apxs

target = mod_zdcp.la

all: $(target)

$(target): mod_zdcp.c mod_zdcp.h
	$(APXS) -L/usr/lib -lpcre -L../lib -I../src/lib -lzdcp -c $<

install: all
	$(APXS) -a -i -n zdcp $(target)

clean:
	-rm -rf .libs
	-rm *.slo *.la *.lo *.o

regress: regress.c
	gcc -I../src/lib -L../lib -O2 -Wall -o regress regress.c -lzdcp

