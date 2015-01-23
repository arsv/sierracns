#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

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
		
		snprintf(outbuf, outlen, "/dev/char/%i:%i", 188, di);
		ret = 0; break;
	}

	closedir(dd);
	return ret;
}
