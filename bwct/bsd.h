/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _REPLACEMENTS
#define _REPLACEMENTS

#if !HAVE_BZERO && HAVE_MEMSET
#define bzero(buf, bytes) \
	((void) memset(buf, 0, bytes))
#endif

#ifndef HAVE_DAEMON
int daemon(int nochdir, int noclose);
#endif

#endif /* !_REPLACEMENTS */

