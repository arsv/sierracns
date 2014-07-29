#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <arpa/inet.h>

#include "wire.h"
#include "query.h"

extern struct cns* cns;
extern int signalled;
extern int opt_a;
extern int opt_e;

/* Run one query, wait for result. Returns -1 on hard errors,
   and 1 upon receiving GETERROR reply. */
int querycns(int mode, int oid, int op, int len, char* payload)
{
	if(sendcns(oid, op, len, payload))
		return -1;
	
	op += 1; 	/* this assumes N*REPLY = N + 1, a rather accidental feature of CnS op field */
	while(1) {
		if(recvcns())
			return -1;

		if(ntohs(cns->oid) != oid)
			continue;
		if((cns->op & ~CNS_ERRORMASK) != op)
			continue;

		break;
	}

	if((cns->op & CNS_ERRORMASK)) {
		if(mode & SHOW_E) showcns();
	} else {
		if(mode & SHOW_R) showcns();
	}

	return ((cns->op & CNS_ERRORMASK) ? 1 : 0);
}

/* Two common short-hands, simple GET query dumping any reply,
   and SET query showing errors but silently accepting results. */
int getcns(int oid)
{
	return querycns(RE, oid, CNS_GET, 0, NULL);
}

int setcns(int oid, int len, char* payload)
{
	return querycns(nE, oid, CNS_SET, len, payload);
}

/* For NE and ND queries */
int queryall(int n, int* oids, int op)
{
	int i;

	for(i = 0; i < n; i++)
		if(querycns(nE, oids[i], op, 0, NULL))
			return -1;

	return 0;
}

/* Keep reading and show()ing packets until interrupted by a signal.
   Typical sequence is queryall(..., CNS_NE), watchcns, queryall(...., CNS_ND)
   but since modems do not reset active notifications on falling DTR it is totally
   possible to issue NE request, detach, and then come back to listen for notifications. */
int watchcns(int m, int* term, int* which)
{
	int i;

	signalled = 0;
	while(!signalled) {
		if(recvcns()) {
			if(errno == EINTR)
				break;
			else
				return -1;
		} else {
			showcns();
		} for(i = 0; i < m; i++) {
			if(ntohs(cns->oid) == term[i]) {
				if(which)
					*which = ntohs(cns->oid);
				return 0;
			}
		}
	}

	return 0;
}
