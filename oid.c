/* Generic OIDs output */

#include "oid.h"

static struct dict models[] = {
	{ 0x00, "AirCard 710" },
	{ 0x01, "AirCard 720" },
	{ 0x02, "AirCard 750" },
	{ 0x03, "SB750" },
	{ 0x04, "SB750A" },
	{ 0x05, "AirCard 755" },
	{ 0x06, "AirCard 775" },
	{ 0x07, "AirCard 77" },
	{ 0x08, "PC7000" },
	{ 0x09, "PC7200" },
	{ 0x0A, "AirCard 850" },
	{ 0x0B, "AirCard 860" },
	{ 0x0C, "MC8755" },
	{ 0x0D, "MC8765" },
	{ 0x0E, "MC5720" },
	{ 0x0F, "EM5625" },
	{ 0x10, "AirCard 595" },
	{ 0x11, "MC5725/MC5725V" },
	{ 0x12, "AirCard 597E" },
	{ 0x13, "AirCard 875" },
	{ 0x14, "MC8775" },
	{ 0x15, "MC8775V" },
	{ 0x16, "EM5725" },
	{ 0x17, "AirCard 597" },
	{ 0x18, "AirCard 880" },
	{ 0x19, "AirCard 881" },
	{ 0x1A, "MC8780" },
	{ 0x1B, "MC8781" },
	{ 0x1C, "MC8780V" },
	{ 0x1D, "MC8781V" },
	{ 0x1E, "EC880" },
	{ 0x1F, "EC881" },
	{ 0x20, "EM8780" },
	{ 0x21, "EM8781" },
	{ 0x22, "M81A" },
	{ 0x23, "M81B" },
	{ 0x24, "AirCard 595U" },
	{ 0x25, "MC5727" },
	{ 0x26, "Compass 597" },
	{ 0x27, "MC8785" },
	{ 0x28, "MC8785V" },
	{ 0x29, "AirCard 885E" },
	{ 0x2A, "Compass 885" },
	{ 0x2B, "MC8790" },
	{ 0x2C, "MC8790V" },
	{ 0x2D, "AirCard 501" },
	{ 0x2E, "Compass 888" },
	{ 0x2F, "Compass 889" },
	{ 0x30, "K888" },
	{ 0x31, "K889" },
	{ 0x32, "MC8791V" },
	{ 0x33, "MC8792V" },
	{ 0x34, "C22" },
	{ 0x35, "C23" },
	{ 0x36, "USB 598" },
	{ 0x37, "T11" },
	{ 0x38, "unassigned" },
	{ 0x39, "AirCard 402" },
	{ 0x3A, "MC5727V" },
	{ 0x3B, "MC5728" },
	{ 0x3C, "MC5728V" },
	{ 0x3D, "K22" },
	{ 0x3E, "K23" },
	{ 0x3F, "USB 305" },
	{ 0x40, "C25" },
	{ 0x41, "USB 303" },
	{ 0x42, "USB 304" },
	{ 0x43, "USB 306" },
	{ 0x44, "MC8777V" },
	{ 0x45, "MC8700" },
	{ 0x46, "AirCard 502" },
	{ 0x47, "USB 301" },
	{ 0x48, "USB 307" },
	{ 0x49, "USB 308" },
	{ 0x4A, "USB 309" },
	{ 0x4B, "USB 311" },
	{ 0x4C, "USB 312" },
	{ 0xFE, "(use HIP 3F.001 instead)" },
	{ 0x00, NULL }
};

static void string(char* tag)
{
	printf("%s: %.*s\n", tag, L, P(0));
}

OID(0000, GR, 0)
{
	printf("CNS heartbeat acknowledge\n");
}

OID(0001, GN, 0)
{
	string("Firmware version");
}

OID(0002, GN, 0)
{
	string("Firmware build date");
}

OID(0003, GN, 0)
{
	string("Hardware version");
}

OID(0004, GN, 0)
{
	string("Boot version");
}

OID(1005, GN, 6)
{
	string("Hardware build date");
}

OID(1016, GN, 15)
{
	string("FSN");
}

OID(1067, GN, 15)
{
	string("IMEI");
}

OID(0010, SR, 2)
{
	printf("Reset v%i\n", S(0));
}

OID(000A, GN, -2)
{
	printf("Model: %s\n", DS(2, models));
}

OiD(1000, GN, 4, cdma)
{
	printf("ESN: 0x%08X\n", I(0));
};

OiD(1000, GN, 27, umts)
{
	return;
}

OID(1002, GN, 10)
{
	printf("MIN: %.*s\n", L, P(0));
}

/* 2 bytes, probably v1: immediate dBm value */
OiD(1001, GN, 2, cmda)
{
	printf("RSSI -%idBm\n", S(0));
};

/* 4 bytes, probably v2: positive offset from baseline + BER */
/* WARNING: some sources imply the baseline may be
   either -110dBm or -125dBm depending on radio */
OiD(1001, GN, 4, umts)
{
	printf("RSSI -%idBm  BER ~ %.1f\n", 110 + S(0), 0.1 + 0.2*S(2));
}

/* Either v1 or CDMA-specific */
OID(1065, GN, 0)
{
	printf("RSSI -%idBm Ec/io %.1fdBm  HDR -%idBm Ec/io %.1fdBm\n",
			S(0), -0.5*S(2), S(4), -0.5*S(6));
}

OiD(1006, GN, 2, cdma)
{
	static struct dict D[] = {
		{ 0x00, "No service" },
		{ 0x02, "CDMA" },
		{ 0x03, "GPS" },
		{ 0x00, NULL }
	};
	printf("Service indication: %s\n", DS(0, D));
}

OiD(1006, GN, -5, umts)
{
	static struct dict D[] = {
		{ 0x00, "None" },
		{ 0x01, "GPRS" },
		{ 0x02, "EDGE" },
		{ 0x03, "UMTS" },
		{ 0x04, "HSDPA" },
		{ 0x05, "HSUPA" },
		{ 0x06, "HSPA (both HSDPA and HSUPA)" },
		{ 0x07, "HSPA+" },
		{ 0x08, "DC-HSPA+" },
		{ 0x00, NULL }
	};
	printf("Service indication: %s", B(2) ? "" : "no service");
	if(B(2)) {
		printf("%s", DB(3, D));
		if(B(4)) printf("+GPRS");
		if(B(5)) printf(", packet session active");
	}
	printf("\n");
}


OiD(1004, GN, 4, umts)
{
	printf("IMSI: MCC %i MNC %i", S(0), S(2));
	if(5 + B(4) <= L)
		printf(" \"%.*s\"", B(4), P(5));
	printf("\n");
}

OiD(1004, GN, 2, cdma)
{
	printf("Channel: %i\n", S(0));
}

OID(1069, GN, 0)
{
	static struct dict D[] = {
		{ 0x0000, "No service" },
		{ 0x0002, "1xEV-DO Rev.0" },
		{ 0x0004, "1xEV-DO Rev.A" },
		{ 0x0000, NULL }
	};
	printf("HDR Service status: %s\n", DS(0, D));
}

OID(106A, GN, 2)
{
	printf("HDR hybrid mode: %s\n", S(0) ? "on" : "off");
}

OID(1009, GN, 2)
{
	printf("Activation status: %s\n", S(0) ? "activated" : "not activated");
}

OID(1007, GN, 2)
{
	printf("%s\n", S(0) ? "Roaming" : "Not roaming");
}

OID(1075, GN, 4)
{
	/* no idea what S(2) means */
	printf("Radio status: %s\n", S(0) ? "on" : "off");
}

/* Should this be in oid_net.c? What does PREV mean btw?.. */
OID(101C, GN, 2)
{
	printf("PREV: %i\n", S(0));
}
