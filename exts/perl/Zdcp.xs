#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <../../src/lib/zdcp.h>

#include "const-c.inc"

MODULE = Zdcp		PACKAGE = Zdcp		

INCLUDE: const-xs.inc

void* create(cfg)
	const char *cfg;
	CODE:
		if (*cfg == '\0')
			cfg = NULL;
		RETVAL = (void *)zdcp_create(cfg);
	OUTPUT:
		RETVAL

void* destroy(handle)
	void *handle;
	CODE:
		zdcp_destroy(handle);

int classify(handle, document, class)
	void *handle;
	const char *document;
	SV* class;	
	CODE:
		const char *pc;
		SV*			sv;
		RETVAL  = zdcp_classify(handle, document, &pc);
		if (!SvROK(class)) 
			croak("class is not a reference");
		if (pc) {	
			sv = SvRV(class);
			sv_setpv(sv, pc);
		} else {
			sv = SvRV(class);
			sv_setpv(sv, "");
		}
		
	OUTPUT:
		RETVAL

const char* error()
	CODE:
		RETVAL = zdcp_error();
	OUTPUT:
		RETVAL
