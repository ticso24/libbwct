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
