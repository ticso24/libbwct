#
# Copyright (c) 2001,02,03 Bernd Walter Computer Technology
# All rights reserved.
#
# $URL$
# $Date$
# $Author$
# $Rev$
#

includedir =		$(DESTDIR)$(prefix)/include/bwct
bin_SCRIPTS =		libbwct-config
EXTRA_DIST =		INSTALL \
			libbwct.m4
lib_LIBRARIES =		libbwct.a
include_HEADERS =	bwct/base.h \
			bwct/bwct.h \
			bwct/config.h \
			bwct/thread.h \
			bwct/tool.h \
			bwct/mstring.h \
			bwct/fdhelper.h \
			bwct/bsd.h \
			bwct/network.h \
			bwct/json.h \
			bwct/aarray.h \
			bwct/sarray.h \
			bwct/array.h \
			bwct/ssl.h \
			bwct/compress.h
libbwct_a_SOURCES =	bwct/thread.cc \
			bwct/tool.cc \
			bwct/mstring.cc \
			bwct/fdhelper.cc \
			bwct/bsd.cc \
			bwct/network.cc \
			bwct/json.cc \
			bwct/aarray.cc \
			bwct/sarray.cc \
			bwct/array.cc \
			bwct/ssl.cc \
			bwct/compress.cc
m4datadir =		$(datadir)/aclocal
m4data_DATA =		libbwct.m4

all-local:
	@echo "*** Everything completed ***"

