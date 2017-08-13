/* Show (print) CnS replies contents

   Big fat assumption here: CnS output is context-free.
   That is, it's ok to use the same output format regardless of why
   or when or where did the packet arrive.

   Given sierracns scope, and the packets it supports,
   this assumption works well. */

#include <stdio.h>
#include <arpa/inet.h>
#include <err.h>

#include "abbr.h"
#include "wire.h"
#include "dict.h"

extern int opt_C;
extern int opt_n;

extern struct cns* cns;
extern struct hip* hip;

void dumpcns(int len, char* buf);
void dumphip(int len, char* buf);

#define GR (1 << CNS_GETREPLY)
#define NO (1 << CNS_NOTIFICATION)
#define SR (1 << CNS_SETREPLY)
#define GN (GR | NO)

#undef  OID$
#define OID$(oid, ops, len, func) extern void func(void);
#include "oid.hh"

static struct dict opnames[] = {
	{ CNS_GET,	"GE" },
	{ CNS_SET,	"SE" },
	{ CNS_GETREPLY, "GR" },
	{ CNS_SETREPLY, "SR" },
	{ CNS_NE,	"EN" },
	{ CNS_ND,	"DN" },
	{ CNS_NEREPLY,	"ER" },
	{ CNS_NDREPLY,	"DR" },
	{ CNS_NOTIFICATION, "NO" },
	{ CNS_GETREPLY | CNS_ERRORMASK, "G!" },
	{ CNS_SETREPLY | CNS_ERRORMASK, "S!" },
	{ CNS_NEREPLY  | CNS_ERRORMASK, "E!" },
	{ CNS_NDREPLY  | CNS_ERRORMASK, "D!" },
	{ 0, NULL }
};


struct showcnsobject {
	uint16_t oid;
	uint16_t opm;
	int len;
	void (*call)(void);
	char* arg;
} showcnsobject[] = {
#undef  OID$
#define OID$(oid, ops, len, func) \
	{ 0x##oid, ops, len, func },
#include "oid.hh"
	{ 0x0000, 0,  0, NULL }
};

void showcns(void)
{
	int oid = ntohs(cns->oid);
	int len = ntohs(cns->len);
	struct showcnsobject* p;

	if(cns->op & CNS_ERRORMASK) {
		printf("%04X %.*s\n", oid, len, cns->payload);
		return;
	}

	if(cns->op == CNS_NEREPLY) {
		if(opt_n)
			printf("CNS %04X notifications enabled\n", oid);
		return;
	}
	if(cns->op == CNS_NDREPLY) {
		if(opt_n)
			printf("CNS %04X notifications disabled\n", oid);
		return;
	}

	for(p = showcnsobject; p->call; p++) {
		if(p->oid != oid)
			continue;
		if(!(p->opm & (1 << cns->op)))
			continue;
		if(p->len > 0 && len != p->len)
			continue;
		if(p->len < 0 && len < -p->len)
			continue;

		p->call();
		return;
	}

	if(!opt_C && cns)
		dumpcns(cns->len + sizeof(struct cns), (char*) cns);
	/* with -C, it's been dumped already */
}

void dumphip(int len, char* buf)
{
	char* p;
	struct hip* hip = (struct hip*) buf;

	if(len < sizeof(struct hip)) {
		printf("HIP ...\n");
		return;
	}

	int hiplen = ntohs(hip->len);
	int paylen;
	
	if(len >= sizeof(struct hip) + hiplen)
		paylen = hiplen;
	else
		paylen = len - sizeof(struct hip);

	printf("HIP 0x%02X 0x%02X { ", hip->mid, hip->par);
	for(p = hip->payload; p < hip->payload + paylen; p++)
		printf("%02X ", (int)(*p & 0xFF));
	if(paylen < hiplen)
		printf("... ");
	printf("}\n");
}

void dumpcns(int len, char* buf)
{
	char* p;
	struct cns* cns = (struct cns*) buf;

	if(len < sizeof(struct cns)) {
		printf("CNS ...\n");
		return;
	}

	int cnslen = ntohs(cns->len);
	int paylen = (len >= sizeof(struct cns) + cnslen ? cnslen : len - sizeof(struct cns));

	printf("CNS %04X %s %i:{ ",
		ntohs(cns->oid), dict(opnames, cns->op, 1), ntohs(cns->len));
	for(p = cns->payload; p < cns->payload + paylen; p++)
		printf("%02X ", (int)(*p & 0xFF));
	if(paylen < cnslen)
		printf("... ");
	printf("}\n");

}
