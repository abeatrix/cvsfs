# Process this file with automake to produce a Makefile.in

EXTRA_DIST = makeinit cvsfsd.Debian cvsfsd.RedHat cvsfsd.SuSE cvsfsd.default

noinst_SCRIPTS = makeinit cvsfsd.Debian cvsfsd.RedHat cvsfsd.SuSE cvsfsd.default

install-exec-local:
	./makeinit $(sbindir)/cvsfsd $(DESTDIR)

uninstall-local:
	rm -f /etc/init.d/cvsfsd
