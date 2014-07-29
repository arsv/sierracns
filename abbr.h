/* Commands and oid_NNN functions must be listed at least twice
   outside of the place they are defined in: first, as prototypes,
   and then as an entry in relevant dict-list.

   Both also carry some information that's irrelevant near
   declaration but must be mentioned in dict-list (min length etc).

   This all gets really messy really fast. To keep things
   nice and clean, some preprocessor tricks come in handy.

   All relevant information goes to the place of declaration,
   gets grep'ed into resp. *.hh file, and used later in main.c
   and show.c to make dict-list. */

#define CMD(cmd, arg)       CMD$(#cmd, NULL, arg, cmd_##cmd)
#define SUB(cmd, sub, arg)  CMD$(#cmd, #sub, arg, cmd_##cmd##_##sub)

#define CMD$(cmd, sub, arg, func) \
	int func(int argc, char** argv)


/* ops: GR = get reply, GN = GR or notification, SR = set reply */
/* len is exact payload length if positive, minimum length if negative */
#define OID(oid, ops, len)     OID$(oid, ops, len, oid_##oid)
#define OiD(oid, ops, len, v)  OID$(oid, ops, len, oid_##oid##v)

#define OID$(oid, ops, len, func) \
	void func(void)

#define noargs 0
#define hasargs 1
