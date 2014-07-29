#include "oid.h"

static struct dict weekdays[] = {
	{ 0x0000, "Mon" },
	{ 0x0001, "Tue" },
	{ 0x0002, "Wed" },
	{ 0x0003, "Thu" },
	{ 0x0004, "Fri" },
	{ 0x0005, "Sat" },
	{ 0x0006, "Sun" },
	{ 0x0000, NULL }
};

static struct dict dst[] = {
	{ 0x00, "" },
	{ 0x01, "DST" },
	{ 0x02, "DST2h" },
	{ 0x00, NULL }
};

/* UMTS network time */
OID(001B, GN, 19)
{
	printf("Network time: %04i-%02i-%02i %s %02i:%02i:%02i UTC%+02i.%02i %s",
			S(2), S(4), S(6),
			DS(8, weekdays),
			S(10), S(12), S(14),
			S(16)/4, S(16)%4*25, DS(18, dst));
}

/* CDMA network time? The time is in *local* time zone. */
OID(1032, GN, 14)
{
	printf("Network time: %04i-%02i-%02i %s %02i:%02i:%02i\n",
			S(0), S(2), S(4),
			DS(6, weekdays),
			S(8), S(10), S(12));
}
