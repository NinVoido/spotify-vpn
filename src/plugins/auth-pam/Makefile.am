#
#  spotify (TM) PAM Auth Plugin -- spotify Plugin
#
#  Copyright (C) 2012      Alon Bar-Lev <alon.barlev@gmail.com>
#

MAINTAINERCLEANFILES = \
	$(srcdir)/Makefile.in

AM_CFLAGS = \
	-I$(top_srcdir)/include \
	$(PLUGIN_AUTH_PAM_CFLAGS) \
	$(OPTIONAL_CRYPTO_CFLAGS)

if ENABLE_PLUGIN_AUTH_PAM
plugin_LTLIBRARIES = spotify-plugin-auth-pam.la
dist_doc_DATA = README.auth-pam
endif

spotify_plugin_auth_pam_la_SOURCES = \
	utils.c \
	auth-pam.c \
	pamdl.c  pamdl.h \
	auth-pam.exports
spotify_plugin_auth_pam_la_LIBADD = \
	$(PLUGIN_AUTH_PAM_LIBS)
spotify_plugin_auth_pam_la_LDFLAGS = $(AM_LDFLAGS) \
	-export-symbols "$(srcdir)/auth-pam.exports" \
	-module -shared -avoid-version -no-undefined
