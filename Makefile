CC = gcc
CFLAGS = -Wall -g

prefix = /usr
bindir = $(prefix)/bin
mandir = $(prefix)/share/man
man1dir = $(mandir)/man1

cmds = cmd.o cmd_gps.o cmd_poke.o cmd_watch.o cmd_query.o
oids = oid.o oid_gps.o oid_time.o oid_net.o
objs = main.o wire.o query.o show.o dict.o $(cmds) $(oids)
	 
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

sierracns: $(objs)
	$(CC) -o $@ $(filter %.o,$^)

main.o: cmd.hh
show.o: oid.hh
cmd.hh: $(patsubst %.o, %.c, $(cmds))
oid.hh: $(patsubst %.o, %.c, $(oids))

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
