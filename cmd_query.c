#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <arpa/inet.h>

#include "wire.h"
#include "query.h"

extern struct cns* cns;

static int queryoid(char* oidspec);
static void parsepayload(char* spec, int* len, char* payload);
static int parsehexbyte(char* dst, char* src);

int cmd_query(int argc, char** argv)
{
	int i;

	for(i = 0; i < argc; i++)
		if(!strcmp(argv[i], "-")) {
			if(watchcns(0, NULL, NULL))
				return -1;
		} else if(queryoid(argv[i])) {
			return -1;
		}

	return 0;
}

/* Oid spec is something like this:
   	000A		-> { 000A, GE, 0, [ ] }
	000A:00010002	-> { 000A, GE, 4, [ 00 01 00 02 ] }
	000A=0012	-> { 000A, SE, 2, [ 00 12 ] }
	000A+		-> { 000A, NE, 0, [ ] }
 */
static int queryoid(char* oidspec)
{
	int oid;
	int op;
	char* end;

	oid = strtol(oidspec, &end, 16);
	if(end != oidspec + 4)
		errx(0, "4-digit hex number expected, got %s", oidspec);

	switch(oidspec[4]) {
		case ':': op = CNS_GET; break;
		case '=': op = CNS_SET; break;
		case '+': op = CNS_NE; break;
		case '-': op = CNS_ND; break;
		case '\0': op = CNS_GET; break;
		default:
			   errx(0, "bad op specification: %s", oidspec);
	}

	int speclen = oidspec[4] ? strlen(oidspec + 5) : 0;
	int len = speclen / 2;	 /* may be a bit too much, but who cares */
	char payload[len];

	if(speclen)
		parsepayload(oidspec + 5, &len, payload);

	return querycns(RE, oid, op, len, payload);
}

/* Parse payload specification from spec, putting the result to payload
   and adjusting *len if necessary.
   	spec is something like "23DF-01-0000"
 	*len is the space available initially in payload */
static void parsepayload(char* spec, int* len, char* payload)
{
	char* p = spec;
	char* q = payload;
	char* e = payload + *len;
	
	while(*p && q < e)
		if(strchr("-,", *p))
			p += 1;
		else if(parsehexbyte(q++, p))
			errx(0, "bad oid payload: %s", spec);
		else /* parsehexbyte fails if there's less that two characters at p */
			p += 2;

	*len = q - payload;
}

/* Take somethings like "7D" from src and put 0x7D to *dst */
static int parsehexbyte(char* dst, char* src)
{
	int off;
	*dst = 0x00;
	for(off = 4; off >= 0; off -= 4, src++)
		if(*src >= '0' && *src <= '9')
			*dst |= (*src - '0' + 0x00) << off;
		else if(*src >= 'a' && *src <= 'f')
			*dst |= (*src - 'a' + 0x0a) << off;
		else if(*src >= 'A' && *src <= 'F')
			*dst |= (*src - 'A' + 0x0A) << off;
		else
			return -1;
	return 0;
}
