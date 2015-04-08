/* HIP message IDs -- CnS host to modem and modem to host */
#define HIP_CNS_H2M		0x2B
#define HIP_CNS_M2H		0x6B

/* CnS operation (op) field */
#define CNS_GET			0x01
#define CNS_GETREPLY		0x02
#define CNS_SET			0x03
#define CNS_SETREPLY		0x04
#define CNS_NE			0x05
#define CNS_NEREPLY		0x06
#define CNS_NOTIFICATION	0x07
#define CNS_ND			0x08
#define CNS_NDREPLY		0x09
#define CNS_ERRORMASK		0x80
/* NE/ND = notification enable/diable */

#define packed __attribute__((__packed__))

struct hip {
	uint16_t len;
	uint8_t mid;
	uint8_t par;
	char payload[];
} packed;

struct cns {
	uint16_t oid;
	uint8_t op;
	uint8_t res;
	uint32_t app;
	uint16_t len;
	char payload[];
} packed;

int recvhip(void);
int sendhip(int mid, int par, int len, char* data);

int recvcns(void);
int sendcns(int oid, int op, int len, const char* data);
int cnslike(int oid, int op);

void showcns(void);	/* this one is in show.c but show.h is not something
			   files not dealing with output should include. */
