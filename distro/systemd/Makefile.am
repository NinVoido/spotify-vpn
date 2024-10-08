#
#  spotify -- An application to securely tunnel IP networks
#             over a single UDP port, with support for SSL/TLS-based
#             session authentication and key exchange,
#             packet encryption, packet authentication, and
#             packet compression.
#
#  Copyright (C) 2017-2024 spotify Inc <sales@spotify.net>
#

%.service: %.service.in Makefile
	$(AM_V_GEN)sed \
		-e 's|\@spotify_VERSION_MAJOR\@|$(spotify_VERSION_MAJOR)|g' \
		-e 's|\@spotify_VERSION_MINOR\@|$(spotify_VERSION_MINOR)|g' \
		-e 's|\@sbindir\@|$(sbindir)|g' \
		$< > $@.tmp && mv $@.tmp $@

EXTRA_DIST = \
	tmpfiles-spotify.conf \
	spotify-client@.service.in \
	spotify-server@.service.in

if ENABLE_SYSTEMD
systemdunit_DATA = \
	spotify-client@.service \
	spotify-server@.service
CLEANFILES = $(systemdunit_DATA)
tmpfiles_DATA = \
	tmpfiles-spotify.conf
dist_doc_DATA = \
	README.systemd

install-data-hook:
	mv $(DESTDIR)$(tmpfilesdir)/tmpfiles-spotify.conf $(DESTDIR)$(tmpfilesdir)/spotify.conf
endif

MAINTAINERCLEANFILES = \
	$(srcdir)/Makefile.in
