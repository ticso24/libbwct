/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _CHANGER
#define _CHANGER

#include <bwct/fdhelper.h>

#ifdef HAVE_SYS_CHIO_H
#ifdef FreeBSD
#define HAVE_CHANGER
class Changer : public Base {
private:
	enum etypes {
		ET_PICK,
		ET_FIRST = ET_PICK,
		ET_SLOT,
		ET_PORTAL,
		ET_DRIVE,
		ET_LAST = ET_DRIVE,
	};
	struct voltag {
		int serial;
		String name;
	};
	struct estatus {
		enum etypes type;
		struct {
			bool full;	// element is full
			bool impexp;	// media deposited by operator
			bool except;	// element in abnormal state
			bool access;	// media accessible by picker
			bool exenab;	// element supports exporting
			bool inenab;	// element supports importing
			bool invert;	// invert bit
			bool svalid;	// source address valid
			bool sid_valid;	// ces_scsi_id is valid
			bool lun_valid;	// ces_scsi_lun is valid
		} flags;
		int sensecode;	// additional sense code for element
		int sensequal;	// additional sense code qualifier
		int source;	// source address of medium
		voltag pvoltag;	// primary volume tag
		voltag avoltag;	// alternate volume tag
		int scsi_id;	// SCSI id of element
		int scsi_lun;	// SCSI lun of element
	};
	Array<estatus> elements;
	File changer;
	String chname;
	int npickers;
	int nslots;
	int nportals;
	int ndrives;
	int getstatus();
public:
	Changer(const String& dev);
	~Changer() {
	}
	void ielem() {
		changer.ioctl(CHIOIELEM);
	}
};

#endif /* FreeBSD */
#endif /* HAVE_SYS_CHIO_H */

#endif /* !_CHANGER */
