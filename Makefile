CC = gcc
CFLAGS = -Wall -g

prefix = /usr
bindir = $(prefix)/bin
mandir = $(prefix)/share/man
man1dir = $(mandir)/man1

cmds = cmd.c cmd_gps.c cmd_poke.c cmd_watch.c cmd_query.c
oids = oid.c oid_gps.c oid_time.c oid_net.c
srcs = main.c wire.c query.c show.c dict.c $(cmds) $(oids)
	 
.c.o:
	$(CC) $(CFLAGS) -c $<

sierracns: $(srcs:.c=.o)
	$(CC) -o $@ $(filter %.o,$^)

main.o: cmd.hh
show.o: oid.hh
cmd.hh: $(cmds)
oid.hh: $(oids)

# see comments in abbr.h on this
cmd.hh oid.hh:
	awk -f abbr.awk $^ > $@

install:
	mkdir -p $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(man1dir)
	install -sm 0755 sierracns  $(DESTDIR)$(bindir)/sierracns
	install -m 0644 sierracns.1 $(DESTDIR)$(man1dir)/sierracns.1

clean:
	rm -f *.o *.hh
