# Process this file with automake to produce a Makefile.in

DEFS = -DBINDIR=\"$(DESTDIR)$(bindir)\"

bin_PROGRAMS = cvsmount
cvsmount_SOURCES = main.c

install-exec-local:
	ln -fs $(DESTDIR)$(bindir)/cvsmount $(DESTDIR)/sbin/mount.cvsfs 
