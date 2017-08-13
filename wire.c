#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <arpa/inet.h>

#include "wire.h"

#define RXBUFLEN 500
#define TXBUFLEN 500
#define PACKETLEN 500

static int rxbuflen = RXBUFLEN;	/* total size of the buffers */
static int txbuflen = TXBUFLEN;
static int tdbuflen = PACKETLEN;
static int rdbuflen = PACKETLEN;

/* This is twice the number of buffers actually needed, but having it all
   available makes opt_H and opt_O handling much easier. */

static char rxbuf[RXBUFLEN];	/* [rt]xbuf are raw wire bytes, */
static char txbuf[TXBUFLEN];	/*   with 7E delimiters and escaping */
static char rdbuf[PACKETLEN];	/* [rt]dbuf are for decoded data. */
static char tdbuf[PACKETLEN];

static int rxlen = 0;		/* used space */
static int rxptr = 0;		/* end of a complete packet, if any */
static int txlen = 0;		/* length of stuff to send */
static int rdlen = 0;
static int tdlen = 0;

/* External interface. Due to the scope of the tool, there's always
   a single connection and a single packet being processed. */

int hipfd = -1;			/* opened modem tty */
struct hip* hip = NULL;		/* current HIP packet being sent/recvd */
struct cns* cns = NULL;		/* same, CnS packet */

/* It's very convenient to tap data flows here in wire.c instead of trying
   to fetch the data from some other point. It is also makes sense to keep
   dump*() in show.c where all generic output routines are.
   To avoid exporting [rt][dx]buf, dump*() routines get them as arguments. */
extern int opt_H;
extern int opt_C;
extern int opt_O;
extern void dumphip(int len, char* buf);
extern void dumpcns(int len, char* buf);

/* Data to send is in tdbuf. Encode it for transmission, filling txbuf. */
static int encpacket(void)
{
	if(tdbuflen < tdlen + 2)
		return -1;

	char *p, *q = txbuf;
	char* txend = txbuf + txbuflen;

	*(q++) = 0x7E;
	for(p = tdbuf; p < tdbuf + tdlen && q < txend - 1; p++)
		if(*p == 0x7E || *p == 0x7D) {
			*(q++) = 0x7D;
			*(q++) = (*p ^ 0x20);
		} else {
			*(q++) = *p;
		}
	if(q >= txend)
		return -1;
	*(q++) = 0x7E;

	txlen = q - txbuf;

	return 0;
}

/* There's a HIP packet in rxbuf, from 0 to rxptr.
   Decode it into rdbuf, setting rdlen to the decoded length. */

static int decpacket(void)
{
	char *p, *q;
	char* rxend = rxbuf + rxptr;
	char* rdend = rdbuf + rdbuflen;

	/* rxptr points at the terminating 7E,
	   so it's p = rxbuf + 1 but p < rxend */
	for(p = rxbuf + 1, q = rdbuf; p < rxend && q < rdend; p++)
		if(*p == 0x7D)
			*(q++) = *(++p) ^ 0x20;
		else	
			*(q++) = *p;
	if(p < rxend)
		return -1;

	rdlen = q - rdbuf;

	return 0;
}

/* Unencoded data to send is in txbuf, its length is txlen.
   Encode the packet and send it. */

int encodesend(void)
{
	if(opt_H && opt_O)
		dumphip(tdlen, tdbuf);

	/* This will alter hiplen */
	if(encpacket())
		return -1;

	int sent = write(hipfd, txbuf, txlen);

	if(sent < 0)
		err(6, "send failed");
	if(sent != txlen)
		err(6, "send: only %i bytes out of %i", sent, txlen);

	return 0;
}

/* Skip garbage (that's anything before 7E) at the start of rxbuf */

static void pullrx(int d)
{
	memmove(rxbuf, rxbuf + d, rxlen - d);
	rxlen -= d;
}

/* Receive some bytes, and do basic HIP packet checks.
   Return packet length when there's a full HIP packet at the start of rxbuf,
   or 0 if there' none. */

static int packetend(void)
{
	char* p;
	int ltt;

again:	if(!(p = memchr(rxbuf, 0x7E, rxlen)))
		return 0;

	/* skip leading garbage */
	if((ltt = p - rxbuf) > 0)
		pullrx(ltt);

	/* do we have terminal byte? */
	if(!(p = memchr(rxbuf + 1, 0x7E, rxlen - 1)))
		return 0;

	/* is there anything resembling a packet? */
	if(p < rxbuf + sizeof(struct hip)) {
		pullrx(p - rxbuf);
		goto again;
	}

	return (p - rxbuf);
}

int sendhip(int mid, int par, int len, char* data)
{
	hip = (struct hip*) tdbuf;

	if(len > tdbuflen - sizeof(struct hip))
		return -1;

	hip->mid = mid;
	hip->par = par;
	hip->len = htons(len);

	memcpy(hip->payload, data, len);

	tdlen = sizeof(struct hip) + len;

	return encodesend();
}

int sendcns(int oid, int op, int len, const char* data)
{
	hip = (struct hip*) tdbuf;
	cns = (struct cns*) hip->payload;

	hip->mid = HIP_CNS_H2M;
	hip->par = 0x00;
	hip->len = htons(sizeof(struct cns) + len);

	if(len > tdbuflen - sizeof(struct cns) - sizeof(struct hip))
		return -1;

	cns->oid = htons(oid);
	cns->op = op;
	cns->app = 0x00000000;
	cns->len = htons(len);
	if(len && data)
		memcpy(cns->payload, data, len);

	tdlen = sizeof(struct hip) + sizeof(struct cns) + len;

	if(opt_C && opt_O)
		dumpcns(ntohs(hip->len), hip->payload);

	return encodesend();
}

/* Get next HIP packet. Generally means reading from the TTY,
   but may also return immediately if there's one more in rxbuf. */

int recvhip(void)
{
	int rd;

	/* Remove old (already decoded) packet if there's one */
	hip = NULL;
	if(rxptr) {
		pullrx(rxptr);
		rxptr = 0;
	}

	/* Blocking complete read for now */
	while(!(rxptr = packetend()))
		if((rd = read(hipfd, rxbuf + rxlen, rxbuflen - rxlen)) <= 0)
			return -1;
		else
			rxlen += rd;

	if(decpacket())
		return -1;
	if(opt_H)
		dumphip(rdlen, rdbuf);

	hip = (struct hip*) rdbuf;

	int explen = ntohs(hip->len);
	int actlen = rdlen - sizeof(struct hip);
	if(explen > actlen) {
		warnx("HIP packet expected %i got %i bytes", explen, actlen);
		hip->len = htons(actlen);
	}

	return 0;
}

/* Receive a HIP, and set *cns if it happens to be a CnS packet.
   Not every HIP packet has CnS payload, though it does not look
   like the device ever sends non-CnS HIPs on its own. */

int recvcns(void)
{
	cns = NULL;

	int r = recvhip();

	if(r < 0)
		return -1;
	if(r > 0)
		return 1;

	if(hip->mid != HIP_CNS_M2H)
		return 1;

	cns = (struct cns*) hip->payload;

	int explen = ntohs(cns->len);
	int actlen = ntohs(hip->len);
	/*  ^ assumed to be reasonable, see recvhip() */

	if(explen > actlen) {
		warnx("CNS packet expected %i got %i bytes", explen, actlen);
		cns->len = htons(actlen);
	}

	if(opt_C)
		dumpcns(ntohs(hip->len), hip->payload);

	return 0;
}
