# Process this file with automake to produce a Makefile.in

bin_PROGRAMS = cvsmount
cvsmount_SOURCES = main.c

install-exec-local:
	ln -s $(DESTDIR)$(bindir)/cvsmount $(DESTDIR)/sbin/mount.cvsfs 
