/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <bwct/base.h>
#include <bwct/bsd.h>
#include <bwct/tool.h>
#include <bwct/tape.h>

int
Tape::rewind() {
	struct mtop mo;

	cassert(opened());
	syslog(LOG_DEBUG, "%s MTREW", tinfo().c_str());
	mo.mt_op = MTREW;
	mo.mt_count = 0;
	if (ioctl(MTIOCTOP, &mo) == -1) {
		syslog(LOG_CRIT, "%s error %s", tinfo().c_str(),
		    strerror(errno));
		return 1;
	}
	return 0;
};

int
Tape::bsf(int count) {
	struct mtop mo;

	cassert(opened());
	syslog(LOG_DEBUG, "%s MTBSF %d", tinfo().c_str(), count);
	mo.mt_op = MTBSF;
	mo.mt_count = count;
	if (ioctl(MTIOCTOP, &mo) == -1) {
		syslog(LOG_CRIT, "%s error %s", tinfo().c_str(),
		    strerror(errno));
		return 1;
	}
	return 0;
};

int
Tape::fsf(int count) {
	struct mtop mo;

	cassert(opened());
	syslog(LOG_DEBUG, "%s: MTFSF %d", tinfo().c_str(), count);
	mo.mt_op = MTFSF;
	mo.mt_count = count;
	if (ioctl(MTIOCTOP, &mo) == -1) {
		syslog(LOG_CRIT, "%s error %s", tinfo().c_str(),
		    strerror(errno));
		return 1;
	}
	return 0;
};

int
Tape::weof() {
	struct mtop mo;

	cassert(opened());
	syslog(LOG_DEBUG, "%s: MTWEOF", tinfo().c_str());
	mo.mt_op = MTWEOF;
	mo.mt_count = 1;
	if (ioctl(MTIOCTOP, &mo) == -1) {
		syslog(LOG_CRIT, "%s error %s", tinfo().c_str(),
		    strerror(errno));
		return 1;
	}
	return 0;
}


