#include <stdio.h>
#include "dict.h"

char* dict(struct dict* d, int val, int width)
{
	struct dict* p;

	for(p = d; p->expl; p++)
		if(p->val == val)
			return p->expl;

	static char buf[10];
	snprintf(buf, 10, "0x%0*X", 2*width, val);
	return buf;
}
