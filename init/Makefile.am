# Process this file with automake to produce a Makefile.in

EXTRA_DIST = makeinit cvsfsd.Debian cvsfsd.RedHat cvsfsd.SuSE

noinst_SCRIPTS = makeinit cvsfsd.Debian cvsfsd.RedHat cvsfsd.SuSE

install-exec-local:
	./makeinit $(sbindir)/cvsfsd $(DESTDIR)

uninstall-local:
	rm -f /etc/init.d/cvsfsd
