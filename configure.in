#
# Copyright (c) 2001,02 Bernd Walter Computer Technology
# All rights reserved.
#
# $URL$
# $Date$
# $Author$
# $Rev$
#

dnl Process this file with autoconf to produce a configure script.

AC_INIT([libbwct],[0.7],[bernd@bwct.de])
AC_CONFIG_SRCDIR(bwct/bwct.h)
AM_INIT_AUTOMAKE(libbwct, 0.7)
AM_CONFIG_HEADER(bwct/config.h)

AC_AIX

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

CB_CFLAGS="-Wall -DDEBUG"
# _XOPEN_SOURCE hurts on Solaris...
# CB_CFLAGS="$CB_CFLAGS -D_XOPEN_SOURCE=1"	# POLLRDNORM/POLLWRNORM under Linux
CB_CFLAGS="$CB_CFLAGS -D_XOPEN_SOURCE_EXTENDED=1" # enable mostly useable sockets on HPUX
CB_CFLAGS="$CB_CFLAGS -D_BSD_SOURCE=1"		# reenable gethostname and others on Linux
CB_CFLAGS="$CB_CFLAGS -D_LARGEFILE64_SOURCE=1"	# access to files greater than 2Gb in Solaris
CB_CFLAGS="$CB_CFLAGS -D_USE_LARGEFILE64=1"	# access to files greater than 2Gb in Linux
CB_CFLAGS="$CB_CFLAGS -D_LARGE_FILE_API=1"	# access to files greater than 2Gb in AIX

CXXFLAGS="$CXXFLAGS $CB_CFLAGS"
CFLAGS="$CFLAGS $CB_CFLAGS"

dnl test using a C-compiler as there are some strange effects with some
dnl generic tests using a c++ compiler
AC_LANG([C])

dnl Checks for libraries.
dnl Replace `main' with a function in -le:
dnl AC_CHECK_LIB(e, main)
dnl Replace `main' with a function in -links:
dnl AC_CHECK_LIB(inks, main)
AC_CHECK_LIB(inet6, main)
AC_CHECK_LIB(socket, main)
AC_CHECK_LIB(nsl, main)
AC_CHECK_LIB(z, main)
AC_CHECK_LIB(bz2, main)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h strings.h sys/ioctl.h sys/time.h syslog.h)
AC_CHECK_HEADERS(unistd.h fts.h err.h netdb.h sys/mtio.h)
AC_CHECK_HEADERS(sys/gentape.h sys/device.h sys/tprintf.h sys/buf.h sgtty.h)
AC_CHECK_HEADERS(sys/io/trioctl.h sys/tape.h sys/chio.h)
AC_CHECK_HEADERS(zlib.h bzlib.h)
AC_CHECK_FUNCS(BZ2_bzCompressInit bzCompressInit)

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_C_BIGENDIAN
AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_HEADER_TIME

dnl Checks for library functions.
dnl AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_FORK([])
AC_CHECK_FUNCS(gethostname gettimeofday select socket strerror)
AC_CHECK_FUNCS(daemon)
AC_CHECK_FUNCS(bzero memset, break)
AC_CHECK_FUNCS(waitpid getnameinfo gai_strerror kqueue sigset)
AC_CHECK_FUNCS(xdr_u_int64_t)
AC_CHECK_FUNCS(open64)
AC_CHECK_FUNCS(readdir64)

AC_MSG_CHECKING(wether the system has IOV_MAX)
AC_TRY_COMPILE([ #include <limits.h> ],
	[ int foo = IOV_MAX],
	[ AC_DEFINE(HAVE_IOV_MAX, 1, [define wether IOV_MAX is defined])
	AC_MSG_RESULT(yes) ], AC_MSG_RESULT(no))

AC_MSG_CHECKING(wether the system has socklen_t)
AC_TRY_COMPILE([
	#include <sys/types.h>
	#include <sys/socket.h>
	],
	[ socklen_t foo; ],
	[ AC_DEFINE(HAVE_SOCKLEN_T, 1, [define wether socklen_t is defined])
	AC_MSG_RESULT(yes) ], AC_MSG_RESULT(no))

AC_MSG_CHECKING(wether the system has uintx types)
AC_TRY_COMPILE([
	#include <sys/types.h>
	#include <inttypes.h>
	],
	[ uint32_t l32],
	[ AC_DEFINE(HAS_UINTX_T, 1, [define wether uintx_t types are available])
	AC_MSG_RESULT(yes) ], AC_MSG_RESULT(no))

AC_MSG_CHECKING(wether the system has uint64_t)
AC_TRY_COMPILE([
	#include <sys/types.h>
	#include <inttypes.h>
	],
	[ uint64_t l64],
	[ AC_DEFINE(HAS_UINT64_T, 1, [define wether uint64_t is available])
	AC_MSG_RESULT(yes) ], AC_MSG_RESULT(no))

ACX_PTHREAD([
	AC_DEFINE(HAVE_PTHREAD, 1, [pthreads OK])
	LIBS="$PTHREAD_LIBS $LIBS"
	CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
	], [
	echo "using forked threads"
	]) 

AC_ARG_WITH(openssl-prefix,
[  --with-openssl-prefix=PREFIX
                          Prefix where openssl is installed (optional)]
, openssl_config_prefix="$withval", openssl_config_prefix="")

for dir in $openssl_config_prefix /usr/local/ssl /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr /opt/openssl; do
    ssldir="$dir"
    if test -f "$dir/include/openssl/ssl.h"; then
        found_ssl="yes";
        CXXFLAGS="$CXXFLAGS -I$ssldir/include";
	AC_DEFINE(HAVE_SSL, 1, [define wether we have SSL])
        break;
    fi
done
if test x_$found_ssl != x_yes; then
    AC_MSG_ERROR(Cannot find ssl libraries)
else
    printf "OpenSSL found in $ssldir\n";
    LIBS="$LIBS -lssl -lcrypto";
    LDFLAGS="$LDFLAGS -L$ssldir/lib";
    HAVE_SSL=yes
fi
AC_SUBST(HAVE_SSL)

AC_ARG_WITH(mird-prefix,
[  --with-mird-prefix=PREFIX
                          Prefix where mird is installed (optional)]
, mird_config_prefix="$withval", mird_config_prefix="")

for dir in $mird_config_prefix /usr/local /usr/lib /usr /usr /opt/mird; do
    mirddir="$dir"
    if test -f "$dir/include/mird.h"; then
        found_mird="yes";
        CXXFLAGS="$CXXFLAGS -I$mirddir/include";
	AC_DEFINE(HAVE_MIRD, 1, [define wether we have MIRD])
        break;
    fi
done
if test x_$found_mird != x_yes; then
    AC_MSG_ERROR(Cannot find mird libraries)
else
    printf "mird found in $mirddir\n";
    LIBS="$LIBS -lmird";
    LDFLAGS="$LDFLAGS -L$mirddir/lib";
    HAVE_MIRD=yes
fi
AC_SUBST(HAVE_MIRD)

AC_LANG([C++])

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

