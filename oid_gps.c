/* GPS oids output */

#include <math.h>
#include "oid.h"

/* GPS status, roughly the same as AT!GPSSTATUS */
/* No idea how DLoad works even with AT commands, so that part is skipped */

struct dict PD[] = {
	{ 0x0000, "No error" },
	{ 0x0001, "Internal modem error" },
	{ 0x0002, "Bad service type" },
	{ 0x0003, "Bad session type" },
	{ 0x0004, "Invalid privacy" },
	{ 0x0005, "Invalid data download" },
	{ 0x0006, "Invalid network access" },
	{ 0x0007, "Invalid operation mode" },
	{ 0x0008, "Invalid number of fixes" },
	{ 0x0009, "Invalid server info" },
	{ 0x000A, "Invalid timeout" },
	{ 0x000B, "Invalid QoS parameter" },
	{ 0x000C, "No session active" },
	{ 0x000D, "Session already active" },
	{ 0x000E, "Session busy" },
	{ 0x000F, "Phone is offline" },
	{ 0x0010, "CDMA lock error" },
	{ 0x0011, "GPS lock error" },
	{ 0x0012, "Invalid state" },
	{ 0x0013, "Connection failure" },
	{ 0x0014, "No buffers available" },
	{ 0x0015, "Searcher error" },
	{ 0x0016, "Cannot report now" },
	{ 0x0017, "Resource contention" },
	{ 0x0018, "Mode not supported" },
	{ 0x0019, "Authentication failed" },
	{ 0x001A, "Other error" },
	{ 0x001B, "Fix rate for tracking is too large" },
	{ 0x0000, NULL }
};

struct dict PA[] = {
	{ 0x0000, "No error" },
	{ 0x0001, "Internal error" },
	{ 0x0002, "Invalid client ID" },
	{ 0x0003, "Error in parameter to set" },
	{ 0x0004, "Error in lock type" },
	{ 0x0005, "Phone is in offline state" },
	{ 0x0006, "A command to set parameter is already active" },
	{ 0x0007, "Incorrect application-specific information" },
	{ 0x0008, "Unknown error" },
	{ 0x0000, NULL }
};

struct dict FE[] = {
	{ 0x0000, "Offline" },
	{ 0x0001, "No service" },
	{ 0x0002, "No connection" },
	{ 0x0003, "No data" },
	{ 0x0004, "Session busy" },
	{ 0x0005, "Reserved" },
	{ 0x0006, "GPS disabled" },
	{ 0x0007, "Connection failed" },
	{ 0x0008, "Error state" },
	{ 0x0009, "Client ended" },
	{ 0x000A, "UI ended" },
	{ 0x000B, "Network ended" },
	{ 0x000C, "Timeout" },
	{ 0x000D, "Privacy level" },
	{ 0x000E, "Network access error" },
	{ 0x000F, "Fix error" },
	{ 0x0010, "PDE rejected" },
	{ 0x0011, "Traffic channel exited" },
	{ 0x0012, "E911" },
	{ 0x0013, "Server error" },
	{ 0x0014, "Stale BS information" },
	{ 0x0015, "Resource contention" },
	{ 0x0016, "Authentication parameter failed" },
	{ 0x0017, "Authentication failed - local" },
	{ 0x0018, "Authentication failed - network" },
	{ 0x1000, "VX LCS agent auth fail" },
	{ 0x1001, "Unknown system error" },
	{ 0x1002, "Unsupported service" },
	{ 0x1003, "Subscription violation" },
	{ 0x1004, "Desired fix method failed" },
	{ 0x1005, "Antenna switch" },
	{ 0x1006, "No fix reported due to no Tx confirmation received" },
	{ 0x1007, "Network indicated normal ending of session" },
	{ 0x1008, "No error specified by the network" },
	{ 0x1009, "No resources left on the network" },
	{ 0x100A, "Position server not available" },
	{ 0x100B, "Network reported unsupported protocol version" },
	{ 0x100C, "SS MOLR error—System failure" },
	{ 0x100D, "SS MOLR error—Unexpected data value" },
	{ 0x100E, "SS MOLR error—Data missing" },
	{ 0x100F, "SS MOLR error—Facility not supported" },
	{ 0x1010, "SS MOLR error—SS subscription violation" },
	{ 0x1011, "SS MOLR error—Position method failure" },
	{ 0x1012, "SS MOLR error—Undefined" },
	{ 0x1013, "Control plane’s SMLC timeout, may or may not end PD" },
	{ 0x1014, "Control plane’s MT guard time expires" },
	{ 0x1015, "End waiting for additional assistance" },
	{ 0x0000, NULL }
};

static struct dict PU[] = {
	{ 0x0000, "0.5m" },
	{ 0x0001, "0.75m" },
	{ 0x0002, "1m" },
	{ 0x0003, "1.5m" },
	{ 0x0004, "2m" },
	{ 0x0005, "3m" },
	{ 0x0006, "4m" },
	{ 0x0007, "6m" },
	{ 0x0008, "8m" },
	{ 0x0009, "12m" },
	{ 0x000A, "16m" },
	{ 0x000B, "24m" },
	{ 0x000C, "32m" },
	{ 0x000D, "48m" },
	{ 0x000E, "64m" },
	{ 0x000F, "96m" },
	{ 0x0010, "128m" },
	{ 0x0011, "192m" },
	{ 0x0012, "256m" },
	{ 0x0000, NULL }
};

OID(0F03, GN, 82)
{
	static struct dict D[] = {
		{ 0x0000, "NONE" },
		{ 0x0001, "ACTIVE" },
		{ 0x0002, "SUCCESS" },
		{ 0x0003, "FAIL" },
		{ 0x0000, NULL }
	};

	/* Error code is always reported but only matters when status code is FAIL */
	printf("GPS %.15s Last fix status:    [%s] %s\n",
			P( 2), DS(18, D),
			S(18) == 0x03 ? DS(20, PD) : "");
	printf("GPS %.15s Fix session status: [%s] %s\n",
			P(42), DS(58, D),
			S(58) == 0x03 ? DS(60, PD) : "");
}

OID(0F02, SR, 4)
{
	printf("GPS fix: %s\n", S(2) ? DS(2, PD) : "started");
}

OID(0F04, SR, 4)
{
	printf("GPS tracking: %s\n", S(2) ? DS(2, PD) : "started");
}

OID(0F05, SR, 4)
{
	printf("GPS end session: %s\n", DS(2, PD));
}

OID(0F0B, GN, 0)
{
	printf("GPS fix: completed\n");
}

OID(0F0C, GN, 4)
{
	printf("GPS fix: %s\n", DS(2, FE));
}

OID(0F14, SR, 4)
{
	printf("GPS lock: %s\n", S(2) ? "error" : "ok");
}

OID(0F0D, GN, 0)
{
	printf("GPS fix: started\n");
}

OID(0F0E, GN, 0)
{
	printf("GPS fix: failed\n");
}

OID(0F20, SR, 0)
{
	printf("GPS clear ok\n");
}

OID(0F1F, GN, -4)
{
	int num, i, off;

	if(S(0) != 0x0001)
		return;

	printf("GPS visible satellites: ");

	if(!(num = B(2))) {
		printf("none\n");
		return;
	} else
		printf("\n");

	for(i = 0; i < num && ((off = 3 + 5*(i+1)) <= L); i++)
		printf("SAT%02i elevation %i° azimuth %i° SNR %idB\n",
				B(off + 0), B(off + 1), S(off + 2), B(off + 4));
}

OID(0F13, GN, 6)
{
	printf("GPS IP: %i.%i.%i.%i\n", B(2), B(3), B(4), B(5));
}

OID(0F16, GN, 6)
{
	printf("GPS port: %i\n", I(2));
}

OID(0F0A, GN, -36)
{
	double lat = I(2)*360.0/(2<<25);
	double lon = I(6)*360.0/(2<<26);

	if(S(0) != 0x0001 && S(0) != 0x0002) {
		printf("GPS location data: unsupported version 0x%04X\n", S(0));
		return;
	}

	printf("GPS time: %i\n", I(10));
	printf("GPS location: %.6f %.6f %s\n", lat, lon, S(20) ? "3D" : "2D");
	printf("GPS uncertainity: angle %.3f A %s P %s\n", 5.625*S(14), DS(16, PU), DS(18, PU));
	if(S(22))
		printf("GPS height: %im", S(24) - 500);
	if(S(28))
		printf("GPS h-velocity: %.2fm/s heading %.3f°\n", 0.25*S(32), S(30)*360.0/(2<<10));
	if(S(28) && S(20))
		printf("GPS v-velocity: %.1fm/s\n", 0.5*S(34));
}
