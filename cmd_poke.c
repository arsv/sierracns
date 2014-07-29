#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <err.h>

#include "abbr.h"
#include "wire.h"
#include "query.h"

extern struct cns* cns;

CMD(poke, noargs)
{
	int r;

	alarm(1);

	if((r = querycns(nn, 0x0000, CNS_GET, 0, NULL))) {
		if(r > 0)
			showcns();
		else if(errno == EINTR)
			printf("No reply, must be dead\n");
		else
			err(errno, NULL);
		return -1;
	} else {
		printf("Alive and kicking\n");
		return 0;
	}
}
