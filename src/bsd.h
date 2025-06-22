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

#ifndef HAVE_DAEMON
int daemon(int nochdir, int noclose);
#endif

#endif /* !_REPLACEMENTS */

