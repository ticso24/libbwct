#!/bin/sh -x
#
# Copyright (c) 2001 Bernd Walter Computer Technology
# All rights reserved.
#
# $URL$
# $Date$
# $Author$
# $Rev$
#

aclocal && \
autoheader && \
automake --foreign --add-missing --copy && \
autoconf
