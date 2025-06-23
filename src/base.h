/*
 * Copyright (c) 2001,02,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/fizonbase.h $
 * $Date: 2025-05-14 21:05:33 +0200 (Wed, 14 May 2025) $
 * $Author: ticso $
 * $Rev: 49218 $
 */

#ifndef _BASE
#define _BASE

#include "config.h"

#include <sys/param.h>

#include <sys/types.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/sctp.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

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
#include <math.h>
#include <regex.h>
#include <stdint.h>

#include <string.h>
#include <strings.h>
#include "bsd.h"

#include <typeinfo>
#include <exception>
#include <new>
#include <iostream>
#include <memory>

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

#ifndef IOV_MAX
# define IOV_MAX 16
#endif

#ifndef LOG_PERROR
# define LOG_PERROR 0
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

// workarround for systems with broken alsways fast clocks
#ifndef CLOCK_REALTIME_FAST
#define CLOCK_REALTIME_FAST CLOCK_REALTIME
#endif

#endif /* !_BASE */
