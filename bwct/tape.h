/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _TAPE
#define _TAPE

#include <bwct/fdhelper.h>

/*
 * XXX: It may be required that read's and write's are page aligned!
 * => freebsd-scsi Message-ID: <20030602131225.F71034@beppo>
 */

class Tape : public File {
public:
	void open(const String& device) {
		File::open(device, O_RDWR);
	}
	int rewind();
	int fsf(int count);
	int bsf(int count);
	int weof();
};

#endif /* !_TAPE */
