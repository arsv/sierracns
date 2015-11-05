#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

/* With no -d specified, the tool needs to guess which tty
   to use. Sierra's are generic nondescript usb ttys,
   but they are always handled by sierra.ko, so we look through

       /sys/bus/usb/drivers/sierra/(device)/ttyUSB(n)

   and pick the second tty (presumed to be CnS port) of the first
   device found. Sierra devices always seem to register their ports
   in sequence, so it's a good guess.

   Now we can't rely on n being 1 due to possible presence of other
   usb ttys, or on /dev/ttyUSB(n) existing even if it is listed
   in sysfs since udev could have renamed it.
   So instead, we take n to be device minor number and use
   /dev/char/188:n instead. */

#define USB_TTY_MAJ 188

char* sierrabase = "/sys/bus/usb/drivers/sierra";

int findmidtty(char* sysdevdir)
{
	DIR* dd;
	struct dirent* de;
	int di;
	int ret = -1;

	if(!(dd = opendir(sysdevdir)))
		return ret;

	while((de = readdir(dd))) {
		if(sscanf(de->d_name, "ttyUSB%i", &di) != 1)
			continue;
		if(ret < 0 || di < ret)
			ret = di;
	};

	return ret < 0 ? ret : ret + 1;
}

int findsierra(int outlen, char* outbuf)
{
	DIR* dd;
	struct dirent* de;
	int di;
	int ret = -1;

	int ttylen = 500;
	char ttybuf[ttylen];

	if(!(dd = opendir(sierrabase)))
		return ret;

	while((de = readdir(dd))) {
		if(de->d_name[0] < '0' && de->d_name[1] > '9')
			continue;

		snprintf(ttybuf, ttylen, "%s/%s", sierrabase, de->d_name);

		if((di = findmidtty(ttybuf)) < 0)
			continue;
		
		snprintf(outbuf, outlen, "/dev/char/%i:%i", USB_TTY_MAJ, di);
		ret = 0; break;
	}

	closedir(dd);
	return ret;
}
