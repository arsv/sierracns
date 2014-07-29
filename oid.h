/*
   This is a "header" file for all out*.c, keeping repeating stuff
   out of those files so that they would start with a single
   	# include "out.h"
*/

#include <stdio.h>
#include <arpa/inet.h>

#include "abbr.h"
#include "wire.h"
#include "dict.h"

extern struct cns* cns;

/* Byte, short and (32bit) int at specified offsets in cns payload. */
#define B(off) (*((uint8_t*)(cns->payload+off)))
#define S(off) ntohs(*((uint16_t*)(cns->payload+off)))
#define I(off) ntohl(*((uint32_t*)(cns->payload+off)))
/* Packet length */
#define L ntohs(cns->len)
/* String at offset */
#define P(off) (cns->payload + off)
/* Dict value */
#define DS(off, d) dict(d, S(off), 2)
#define DB(off, d) dict(d, B(off), 1)
