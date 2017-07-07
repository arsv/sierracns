/* Commands for generic OIDs */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <err.h>

#include "abbr.h"
#include "wire.h"
#include "query.h"

CMD(info, noargs)
{
	getcns(0x000A);	/* model id */
	getcns(0x1000);	/* ESN */
	getcns(0x0001);	/* FW version */
	getcns(0x0002);	/* FW build date */
	getcns(0x0003);	/* HW version */
	getcns(0x0004);	/* Boot version */
	return 0;
}

CMD(reset, noargs)
{
	char ver[2] = { 0x00, 0x01 };
	return setcns(0x0010, 2, ver);
}

CMD(rssi, noargs)
{
	return getcns(0x1001);
}

CMD(hdr, noargs)
{
	return !getcns(0x1065)
	    && !getcns(0x1069);
}

CMD(time, hasargs)
{
	int r;

	/* Try UMTS first */
	if((r = querycns(Rn, 0x001B, CNS_GET, 0, NULL)) <= 0)
		return r;
	/* CDMA */
	if((r = querycns(Rn, 0x1032, CNS_GET, 0, NULL)) <= 0)
		return r;

	/* There should be gps time somewhere too but I have no idea
	   what the right OID is. */
	return -1;
}

SUB(radio, on, noargs)
{
	char data[4] = { 0x00, 0x01, 0x00, 0x00 };
	return setcns(0x1075, 4, data);
}

SUB(radio, off, noargs)
{
	char data[4] = { 0x00, 0x00, 0x00, 0x00 };
	return setcns(0x1075, 4, data);
}

CMD(radio, noargs)
{
	return getcns(0x1075);
}
