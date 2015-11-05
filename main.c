#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <alloca.h>
#include <sys/file.h>
#include <err.h>

#include "abbr.h"

/* Overall program structure:
   	wire.c			underlying wire protocols, packet send/recv
	query.c			basic commands operating on raw OIDs
	show.c			entry point for formatted CnS output
	main.c			entry point for commands
	oid*.c			display rountines for individual OIDs
	cmd*.c			commands (gps, sms and such)

   Grouping routines by topic only (gps.c, sms.c etc) sounds like a good idea
   at first, but it turns out there's literaly nothing common between show*
   and commands operating on the same oids. */

int signalled = -1;

int opt_H = 0;	/* dump raw HIP packets */
int opt_C = 0;	/* dump raw CnS packets */
int opt_O = 0;	/* dump outgoing packets */
int opt_c = 0;	/* dump unrecognized CnS packets */
int opt_n = 0;	/* show NE/ND packets */
int opt_a = 0;	/* show stray CnS packets */
int opt_e = 0; 	/* show errors */

extern int hipfd;
static int lookslikeoid(char* str);

extern int cmd_(void);
extern int cmd_query(int argc, char** argv);
static void sigsetup(void);
extern int findsierra(int len, char* buf);

#undef  CMD$
#define CMD$(cmd, sub, arg, func) extern int func(int argc, char** argv);
#include "cmd.hh"

struct cmd {
	char* pri;
	char* sub;
	int arg;
	int (*func)(int argc, char** argv);
} cmds[] = {
#undef  CMD$
#define CMD$(cmd, sub, arg, func) { cmd, sub, arg, func },
#include "cmd.hh"
	{ NULL, NULL }
};

/* OID commands start with 4 hex digits, possibly followed by query
   type char and payload. Anything else is a regular command to be
   checked against cmds[]. The source of confusion here are numbers
   like "aaaa" (oid) and "aaa" (command, should be "0aaa" instead). */
static int lookslikeoid(char* str)
{
	int i;
	char c;

	for(i = 0; (c = str[i]) && (i < 4); i++)
		if((c >= '0' && c <= '9')
		|| (c >= 'a' && c <= 'f')
		|| (c >= 'A' && c <= 'F'))
			continue;
		else
			return 0;

	if(i < 4)
		return 0;

	switch(c) {
		case ':':
		case '=':
		case '+':
		case '-':
		case '\0':
			return 1;
		default:
			return 0;
	}
}

/* Some additional care is needed here to match subcommands and produce
   readable error messages. The entries in cmds[] are ordered so that
   raw command comes after any of its subcommands, allowing a one-pass
   scan over cmds[]. */
int trycmds(int argc, char** argv)
{
	struct cmd* cmd;
	int primatch = 0;

	for(cmd = cmds; cmd->pri; cmd++) {
		if(strcmp(cmd->pri, *argv))
			continue;
		else
			primatch = 1;
		if(!cmd->sub)
			break;
		if(argc < 2)
			continue;
		if(strcmp(cmd->sub, *(argv+1)))
			continue;
		break;
	} if(!cmd->pri) {
		if(primatch && argc > 1)
			errx(5, "unknown %s subcommand %s", *argv, *(argv+1));
		else if(primatch)
			errx(5, "command %s needs subcommand", *argv);
		else
			errx(5, "unknown command %s", *argv);
	}

	return cmd->func(argc - 1, argv + 1);
}

/* The port must be switched to raw mode; sierra.ko makes it a regular
   serial port with terminal-style line discipline, completely unsuitable
   for a protocol operating in 0x7E-delimited packets instead of lines. */
int modemsetup(int fd)
{
        struct termios tio = {
                .c_iflag = IGNBRK | IGNPAR,
                .c_oflag = 0,
                .c_cflag = CLOCAL,
                .c_lflag = 0
        };

        memset(tio.c_cc, 0, sizeof(tio.c_cc));
        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 0;

        if(tcsetattr(fd, TCSANOW, &tio))
                err(4, "tcsetattr");

        return 0;
}

static void sighandler(int sig)
{
	switch(sig) {
		case SIGINT:
			if(signalled)
				_exit(0);
			else
				signalled = 1;
			break;
	}
}

static void sigsetup(void)
{
	struct sigaction sa = {
		.sa_handler = sighandler,
		.sa_mask = { { 0 } },
		.sa_flags = 0,	/* do NOT auto-resetart read() calls */
		.sa_restorer = NULL
	};

	if(sigaction(SIGINT, &sa, NULL))
		err(1, "sigaction");
	if(sigaction(SIGALRM, &sa, NULL))
		err(1, "sigaction");
}

int main(int argc, char** argv)
{
	char* modem = NULL;
	int mdlen = 100;
	int c;
	int r;

	while((c = getopt(argc, argv, "+cCHOnad:")) > 0)
		switch(c) {
			case 'a': opt_a = 1; break;
			case 'c': opt_c = 1; break;
			case 'C': opt_C = 1; break;
			case 'H': opt_H = 1; break;
			case 'O': opt_O = 1; break;
			case 'n': opt_n = 1; break;
			case 'd': modem = optarg; break;
			case '?': return -1;
		}

	if(!modem && findsierra(mdlen, modem = alloca(mdlen)))
		errx(2, "can't find suitable device");

	if((hipfd = open(modem, O_RDWR)) < 0)
		err(3, "can't open %s", modem);

	if(flock(hipfd, LOCK_EX | LOCK_NB))
		err(3, "can't lock %s", modem);

	modemsetup(hipfd);
	sigsetup();

	argv += optind;
	argc -= optind;
	optind = 0;

	if(argc <= 0)
		errx(5, "command required");

	if(lookslikeoid(*argv))
		r = cmd_query(argc, argv);
	else
		r = trycmds(argc, argv);

	return r;
}
