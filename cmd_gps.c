/* Commands for GPS OIDs */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <err.h>

#include "abbr.h"
#include "wire.h"
#include "query.h"

extern struct cns* cns;
#define S(off) (*(uint16_t*)(cns->payload + off))

SUB(gps, clear, hasargs)
{
	char data[9];
	data[0] = 0;
	data[1] = 1;

	int i, j;
	memset(data + 2, argc ? 0x00 : 0x01, 7);

	static struct {
		int off;
		char* key;
	} keys[] = {
		{ 2, "ephemeris" },
		{ 3, "almanac" },
		{ 4, "position" },
		{ 5, "sldb" },
		{ 6, "ssdb" },
		{ 7, "timeref" },
		{ 8, "throttling" }
	};
	for(i = 0; i < argc; i++)
		for(j = 0; j < sizeof(keys)/sizeof(*keys); j++)
			if(!strcmp(argv[i], keys[i].key))
				data[keys[i].off] = 0x01;

	return setcns(0x0F20, 9, data);
}

SUB(gps, satellites, noargs)
{
	return getcns(0x0F1F);
}

SUB(gps, fix, hasargs)
{
	struct {
		uint16_t ver;
		uint16_t fixtype;
		uint16_t timelim;
		uint32_t accuracy;
	} packed req = { htons(0x0001), htons(0x0001), htons(0x00FF), htonl(0xFFFFFFF0) };
	int watch = 0;
	
	int c;
	while((c = getopt(argc, argv, "+a:t:m:")) > 0)
		switch(c) {
			case 'a': req.accuracy = htons(atoi(optarg)); break;
			case 't': req.accuracy = htons(atoi(optarg)); break;
			case 'm': req.fixtype = htonl(atoi(optarg)); break;
			case '?': return -1;
		}
	char* arg;
	for(; optind < argc; optind++) {
		arg = argv[optind];
		if(!strcmp(arg, "-"))
			watch = 1;
		else
			errx(5, "unknown keyword %s", arg);
	}

	if(!watch)
		return setcns(0x0F02, sizeof(req), (char*)&req);

	int oids[] = { 0x0F0B, 0x0F0C, 0x0F0E };
	int term[] = { 0x0F0B, 0x0F0C };
	int n = sizeof(oids)/sizeof(*oids);
	int m = sizeof(term)/sizeof(*term);
	int last = 0x0000;

	/* watchoids() can't be used here because notifications must be enabled
	   before requesting 0x0F02 */
	if(queryall(n, oids, CNS_NE))
		return -1;
	if(setcns(0x0F02, sizeof(req), (char*)&req))
		return -1;
	if(S(2))
		goto nd; /* fix request failed, skip the watch part */
	if(watchcns(m, term, &last))
		return -1;
nd:	if(queryall(n, oids, CNS_ND))
		return -1;

	if(last == 0x0F0B) /* success */
		if(getcns(0x0F0A))
			return -1;

	return 0;
}

SUB(gps, status, noargs)
{
	return getcns(0x0F03);
}

SUB(gps, end, noargs)
{
	char data[] = { 0x00, 0x01, 0x00, 0x00 };
	return setcns(0x0F05, 4, data);
}

SUB(gps, on, noargs)
{
	char data[] = { 0x00, 0x01, 0x00, 0x00 };
	return setcns(0x0F14, 4, data);
}

SUB(gps, off, noargs)
{
	char data[] = { 0x00, 0x01, 0x00, 0x01 };
	return setcns(0x0F14, 4, data);
}

SUB(gps, ip, noargs)
{
	return !getcns(0x0F13)
	    && !getcns(0x0F16);
}

CMD(gps, hasargs)
{
	if(argc) errx(5, "unknown subcommad %s", *argv);
	return getcns(0x0F0A);
}
