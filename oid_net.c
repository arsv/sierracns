#include "oid.h"

/* 4xxx series OIDs, which stands either for "CDMA specific",
   or for "Generic network info" which in turn happens
   to be CDMA or UMTS specific. */

OID(4001, GN, 2)
{
	printf("SID: %i\n", S(0));
}

OID(4002, GN, 2)
{
	printf("NID: %i\n", S(0));
}

OID(4005, GN, 8)
{
	printf("FER: %i/%i\n", I(0), I(4));
}

OID(4004, GN, 2)
{
	printf("Temp: %iC\n", S(0));
}

OID(4008, GN, 2)
{
	static struct dict D[] = {
		{ 0x0000, "Cellular band" },
		{ 0x0001, "PCS band" },
		{ 0x0000, NULL }
	};
	printf("Tech Bandclass: %s\n", DS(0, D));
}

OID(400A, GN, 2)
{
	printf("Packet zone ID: %i\n", S(0));
}

/* Ref. standard 3GPP2 C.S0005-A v6.0, see CdmaCellLocation() in Android. */
/* Lat/Lon values are in signed quarter-seconds. */

struct netcoord {
	char sign;
	uint8_t cent;
	uint8_t sec;
	uint8_t min;
	uint8_t deg;
	double ddeg;
};

static void ncunpack(struct netcoord* p, int v, char* signs)
{
	p->ddeg = v / (4.0*60*60);

	p->sign = (v < 0 ? signs[0] : signs[1]);
	v = v < 0 ? -v : v;

	p->cent = v % 4; 	v /= 4;
	p->sec = v % 60;	v /= 60;
	p->min = v % 60;	v /= 60;
	p->deg = v;
	p->cent *= 25;
}

OID(4003, GN, 8)
{
	struct netcoord lat;
	struct netcoord lon;

	ncunpack(&lat, I(4), "SN");
	ncunpack(&lon, I(0), "WE");

	printf("Network location: %02i°%02i'%02i\"%c %02i°%02i'%02i\"%c (%f, %f)\n",
			lat.deg, lat.min, lat.sec, lat.sign,
			lon.deg, lon.min, lon.sec, lon.sign,
			lat.ddeg, lon.ddeg);
}
