#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <arpa/inet.h>

#include "abbr.h"
#include "wire.h"
#include "query.h"

static int watchoids(int n, int* oids);

SUB(watch, rssi, noargs)
{
	int oid = 0x1001;
	return watchoids(1, &oid);
}

SUB(watch, hdr, noargs)
{
	int oid = 0x1065;
	return watchoids(1, &oid);
}

/* Request notification for specified oids, and show any packets received.

   This implementation explicitly excludes anything that requires non-empty
   payload. It's ok though, since there's exactly one described oid that
   takes payload with NE and it's not very likely to be watched anyway.
 
   In case NE with payload is to be issued, query() can be used instead. */

CMD(watch, hasargs)
{
	int i;
	int oids[argc];
	char* end;

	for(i = 0; i < argc; i++) {
		oids[i] = strtol(argv[i], &end, 16);
		if(end != argv[i] + 4 || *end)
			errx(0, "4-digit hex number expected, got %s", argv[i]);
	}

	return watchoids(argc, oids);
}

static int watchoids(int n, int* oids)
{
	if(queryall(n, oids, CNS_NE))
		return -1;

	if(watchcns(0, NULL, NULL))
		return -1;

	if(queryall(n, oids, CNS_ND))
		return -1;

	return 0;
}

/* Listen for notifications, and disable any received.
   This is mostly to return modem into some kind of sane state after
   running Sierra Watcher in no-reset-at-exit mode.
 
   With non-empty argv, issues ND for listed oids only. */

static int unwatch_oids(int argc, char** argv)
{
	int oid;
	char* end;
	int i;

	for(i = 0; i < argc; i++) {
		oid = strtol(argv[i], &end, 16);
		if(end != argv[i] + 4 || *end)
			errx(0, "4-digit hex number expected, got %s", argv[i]);
		if(querycns(nE, oid, CNS_ND, 0, NULL) < 0)
			return -1;
	}

	return 0;
}

static int unwatch_listen(void)
{
	extern int signalled;
	extern struct cns* cns;

	signalled = 0;

	while(!signalled) {
		if(recvcns()) {
			if(errno == EINTR)
				break;
			else
				return -1;
		}

		if(cns->op != CNS_NOTIFICATION)
			continue;

		printf("Disabling %04X\n", ntohs(cns->oid));
		if(querycns(nE, ntohs(cns->oid), CNS_ND, 0, NULL) < 0)
			return -1;
	}

	return 0;
}

CMD(unwatch, noargs)
{
	if(argc)
		return unwatch_oids(argc, argv);
	else
		return unwatch_listen();
}
