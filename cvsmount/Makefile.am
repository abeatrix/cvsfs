# Process this file with automake to produce a Makefile.in

DEFS = -DBINDIR=\"$(bindir)\"

bin_PROGRAMS = cvsmount
cvsmount_SOURCES = main.c

install-exec-local:
	install -d $(DESTDIR)/sbin
	ln -fs $(bindir)/cvsmount $(DESTDIR)/sbin/mount.cvsfs 

uninstall-local:
	rm -f /sbin/mount.cvsfs
