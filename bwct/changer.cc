/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <bwct/config.h>
#include <bwct/tool.h>
#include <bwct/changer.h>

#ifdef HAVE_SYS_CHIO_H
#ifdef FreeBSD

Changer::Changer(const String& dev) {
	struct changer_params parms;

	chname = dev;
	changer.open(chname, O_RDWR);
	if (!changer.opened())
		; // TODO throw
	// TODO check values
	changer.ioctl(CHIOGPARAMS, &parms);
	npickers = parms.cp_npickers;
	nslots = parms.cp_nslots;
	nportals = parms.cp_nportals;
	ndrives = parms.cp_ndrives;
}

int
Changer::getstatus() {
	struct changer_element_status_request request;
	struct changer_element_status status;
	int res;
	int i;
	enum etypes j;
	struct estatus nstatus;

	// XXX
	for (j = ET_FIRST; j <= ET_LAST; ((int)j)++) {
		i = 0;
		do {
			switch (j) {
			case ET_PICK:
				request.cesr_element_type = CHET_MT;
			case ET_SLOT:
				request.cesr_element_type = CHET_ST;
			case ET_PORTAL:
				request.cesr_element_type = CHET_IE;
			case ET_DRIVE:
				request.cesr_element_type = CHET_DT;
			}
			request.cesr_element_base = i;
			request.cesr_element_count = 1;
			request.cesr_element_status = &status;
			res = changer.ioctl(CHIOGSTATUS, &request);
			if (res == 0) {
				nstatus.type = j;
				nstatus.sensecode = status.ces_sensecode;
				nstatus.sensequal = status.ces_sensequal;
				nstatus.source = status.ces_source_addr;
				nstatus.pvoltag.serial =
				    status.ces_pvoltag.cv_serial;
				nstatus.pvoltag.name =
				    (char*)status.ces_pvoltag.cv_volid;
				nstatus.avoltag.serial =
				    status.ces_avoltag.cv_serial;
				nstatus.avoltag.name =
				    (char*)status.ces_avoltag.cv_volid;
				nstatus.scsi_id = status.ces_scsi_id;
				nstatus.scsi_lun = status.ces_scsi_lun;
				nstatus.flags.full =
				    (status.ces_flags & CES_STATUS_FULL);
				nstatus.flags.impexp =
				    (status.ces_flags & CES_STATUS_IMPEXP);
				nstatus.flags.except =
				    (status.ces_flags & CES_STATUS_EXCEPT);
				nstatus.flags.access =
				    (status.ces_flags & CES_STATUS_ACCESS);
				nstatus.flags.exenab =
				    (status.ces_flags & CES_STATUS_EXENAB);
				nstatus.flags.inenab =
				    (status.ces_flags & CES_STATUS_INENAB);
				nstatus.flags.invert =
				    (status.ces_flags & CES_INVERT);
				nstatus.flags.svalid =
				    (status.ces_flags & CES_SOURCE_VALID);
				nstatus.flags.sid_valid =
				    (status.ces_flags & CES_SCSIID_VALID);
				nstatus.flags.lun_valid =
				    (status.ces_flags & CES_LUN_VALID);
				elements[status.ces_addr] = nstatus;
			}
			i++;
		} while (res == 0);
	}
	return res;
}

#endif /* FreeBSD */
#endif /* HAVE_SYS_CHIO_H */

