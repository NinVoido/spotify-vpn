#
#  spotify (TM) Down Root Plugin -- spotify Plugin
#
#  Copyright (C) 2012      Alon Bar-Lev <alon.barlev@gmail.com>
#

MAINTAINERCLEANFILES = \
	$(srcdir)/Makefile.in

AM_CFLAGS = \
	-I$(top_srcdir)/include \
	$(OPTIONAL_CRYPTO_CFLAGS)

if ENABLE_PLUGIN_DOWN_ROOT
plugin_LTLIBRARIES = spotify-plugin-down-root.la
dist_doc_DATA = README.down-root
endif

spotify_plugin_down_root_la_SOURCES = \
	down-root.c \
	down-root.exports
spotify_plugin_down_root_la_LDFLAGS = $(AM_LDFLAGS) \
	-export-symbols "$(srcdir)/down-root.exports" \
	-module -shared -avoid-version -no-undefined
