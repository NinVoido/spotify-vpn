#
#  spotify -- An application to securely tunnel IP networks
#             over a single UDP port, with support for SSL/TLS-based
#             session authentication and key exchange,
#             packet encryption, packet authentication, and
#             packet compression.
#
#  Copyright (C) 2002-2024 spotify Inc <sales@spotify.net>
#  Copyright (C) 2006-2012 Alon Bar-Lev <alon.barlev@gmail.com>
#

SUBDIRS = doxygen

#
# List of man and HTML pages we build when rst2man/rst2html is available
#
# NOTE: Remember to add source .rst files to $(dist_noinst_DATA) below
#       This could be automated with GNU Make, but we need BSD Make support
#
build_man_pages = spotify.8 spotify-examples.5
build_html_pages = spotify.8.html spotify-examples.5.html

dist_doc_DATA = \
	management-notes.txt gui-notes.txt

spotify_sections = \
	man-sections/advanced-options.rst \
	man-sections/cipher-negotiation.rst \
	man-sections/client-options.rst \
	man-sections/connection-profiles.rst \
	man-sections/encryption-options.rst \
	man-sections/generic-options.rst \
	man-sections/inline-files.rst \
	man-sections/link-options.rst \
	man-sections/log-options.rst \
	man-sections/management-options.rst \
	man-sections/network-config.rst \
	man-sections/pkcs11-options.rst \
	man-sections/plugin-options.rst \
	man-sections/protocol-options.rst \
	man-sections/proxy-options.rst \
	man-sections/renegotiation.rst \
	man-sections/signals.rst \
	man-sections/script-options.rst \
	man-sections/server-options.rst \
	man-sections/tls-options.rst \
	man-sections/unsupported-options.rst \
	man-sections/virtual-routing-and-forwarding.rst \
	man-sections/vpn-network-options.rst \
	man-sections/windows-options.rst

spotify_examples_sections = \
	man-sections/example-fingerprint.rst \
	man-sections/examples.rst

dist_noinst_DATA = \
	android.txt \
	interactive-service-notes.rst \
	keying-material-exporter.txt \
	spotify.8.rst \
	spotify-examples.5.rst \
	README.man \
	README.plugins \
	tls-crypt-v2.txt \
	$(spotify_sections) \
	$(spotify_examples_sections) \
	CMakeLists.txt

EXTRA_DIST = tests

# dependencies
spotify.8 spotify.8.html: $(spotify_sections)
spotify-examples.5 spotify-examples.5.html: $(spotify_examples_sections)

######  GENERIC  RULES  ##########

SUFFIXES = .8.rst .8 .8.html .5.rst .5 .5.html

RST_FLAGS = --strict

MAINTAINERCLEANFILES = \
	$(srcdir)/Makefile.in

.8.rst.8 .5.rst.5 :
if HAVE_PYDOCUTILS
	$(RST2MAN) $(RST_FLAGS) $< > $@
else
	@echo "Missing python-docutils - skipping man page generation ($@)"
endif

.8.rst.8.html .5.rst.5.html :
if HAVE_PYDOCUTILS
	$(RST2HTML) $(RST_FLAGS) $< > $@
else
	@echo "Missing python-docutils - skipping html page generation ($@)"
endif


if HAVE_PYDOCUTILS
dist_noinst_DATA += $(build_man_pages)
dist_html_DATA = $(build_html_pages)

# Failsafe - do not delete these files unless we can recreate them
CLEANFILES = $(build_man_pages) $(build_html_pages)

endif

if WIN32
else
dist_man_MANS = $(build_man_pages)
endif

dist-hook : $(build_man_pages) $(build_html_pages)
