SUBDIRS = de es 

man: gpsdrive.1 garble.1

EXTRA_DIST = gpsdrive.1 garble.1 geo-code.1 geoinfo.1 geo-nearest.1

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(mandir)/man1/
	$(INSTALL_DATA) $(srcdir)/gpsdrive.1 $(DESTDIR)$(mandir)/man1/gpsdrive.1

