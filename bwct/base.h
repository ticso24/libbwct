/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _BASE
#define _BASE

#include <bwct/config.h>

#include <sys/param.h>

#include <sys/types.h>
#include <sys/time.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/wait.h>

#ifdef HAVE_MMAP
# include <sys/mman.h>
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/rand.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <inttypes.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include <bwct/bsd.h>

#include <typeinfo>
#include <exception>
#include <new>

#ifndef MAXSOCKADDR
# ifdef SOCK_MAXADDRLEN
# define MAXSOCKADDR SOCK_MAXADDRLEN
# endif
#endif

#ifndef MAXSOCKADDR
# ifdef _SS_MAXSIZE
# define MAXSOCKADDR _SS_MAXSIZE
# endif
#endif

#ifndef MAXSOCKADDR
# define MAXSOCKADDR 128
#endif

#ifndef HAVE_IOV_MAX
# define IOV_MAX 16
#endif

#ifndef LOG_PERROR
# define LOG_PERROR 0
#endif

#ifndef HAVE_SOCKLEN_T
typedef uint32_t socklen_t
#endif

#ifndef SOCK_MAXADDRLEN
# define SOCK_MAXADDRLEN 255
#endif

/*
 * Posix.1g requires that an #include of <poll.h> DefinE INFTIM, but many
 * systems still DefinE it in <sys/stropts.h>.  We don't want to include
 * all the streams stuff if it's not needed, so we just DefinE INFTIM here.
 * This is the standard value, but there's no guarantee it is -1.
 */
#ifndef INFTIM
# define INFTIM (-1)
#endif

#endif /* !_BASE */
