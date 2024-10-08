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

include $(top_srcdir)/ltrc.inc

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/include -I$(top_srcdir)/src/spotify -I$(top_srcdir)/src/compat

EXTRA_DIST = \
	CMakeLists.txt

if WIN32
sbin_PROGRAMS = spotifyserv
spotifyserv_CFLAGS = \
	-municode -D_UNICODE \
	-UNTDDI_VERSION -U_WIN32_WINNT \
	-D_WIN32_WINNT=_WIN32_WINNT_VISTA
spotifyserv_LDADD = -ladvapi32 -luserenv -liphlpapi -lfwpuclnt -lrpcrt4 -lshlwapi -lnetapi32 -lws2_32 -lntdll
endif

spotifyserv_SOURCES = \
        common.c \
	interactive.c \
	service.c service.h \
	validate.c validate.h \
	$(top_srcdir)/src/spotify/wfp_block.c $(top_srcdir)/src/spotify/wfp_block.h \
	spotifyserv_resources.rc \
	$(top_srcdir)/src/spotify/ring_buffer.h
