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
#include <bwct/bsd.h>
#include <bwct/tool.h>
#include <bwct/xdr.h>
#include <bwct/fdhelper.h>

bool_t
xdr_cbackup_string(XDR *xdrs, cbackup_string *objp) {
	bool_t res;
	ssize_t sz;
	uint32_t len;

	switch (xdrs->x_op) {
	case XDR_FREE:
		return TRUE;
	case XDR_ENCODE:
		len = objp->length() + 1;
		res = xdr_cbackup_u32(xdrs, &len);
		if (res == FALSE)
			return FALSE;
		sz = xdrs->file->write(objp->c_str(), len);
		if (sz == (ssize_t)len)
			return TRUE;
		break;
	case XDR_DECODE:
		res = xdr_cbackup_u32(xdrs, &len);
		if (res == FALSE || len > 4096)
			return FALSE;
		Matrix<char> str(len);
		sz = xdrs->file->read(str.get(), len);
		if (sz == (ssize_t)len && str[len - 1] == '\0') {
			*objp = str.get();
			return TRUE;
		}
		break;
	}
	return FALSE;
}

bool_t
xdr_cbackup_u64(XDR *xdrs, cbackup_u64 *objp) {
	ssize_t sz;
	cbackup_u32 data1, data2;

	switch (xdrs->x_op) {
	case XDR_FREE:
		return TRUE;
	case XDR_ENCODE:
		data2 = htonl((*objp) & 0xffffffff);
		data1 = htonl((*objp) >> 32);
		sz = xdrs->file->write(&data1, sizeof(data1));
		if (sz != sizeof(data1))
			return FALSE;
		sz = xdrs->file->write(&data2, sizeof(data2));
		if (sz == sizeof(data2))
			return TRUE;
		break;
	case XDR_DECODE:
		sz = xdrs->file->read(&data1, sizeof(data1));
		if (sz != sizeof(data1))
			return FALSE;
		sz = xdrs->file->read(&data2, sizeof(data2));
		if (sz != sizeof(data2))
			return FALSE;
		*objp = ((cbackup_u64)ntohl(data1)) << 32 | ntohl(data2);
		return TRUE;
	}
	return FALSE;
}

bool_t
xdr_cbackup_u32(XDR *xdrs, cbackup_u32 *objp) {
	ssize_t sz;
	cbackup_u32 data;

	switch (xdrs->x_op) {
	case XDR_FREE:
		return TRUE;
	case XDR_ENCODE:
		data = htonl(*objp);
		sz = xdrs->file->write(&data, sizeof(data));
		if (sz == sizeof(data))
			return TRUE;
		break;
	case XDR_DECODE:
		sz = xdrs->file->read(&data, sizeof(data));
		if (sz == sizeof(data)) {
			*objp = ntohl(data);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

bool_t
xdr_cbackup_u16(XDR *xdrs, cbackup_u16 *objp) {
	ssize_t sz;
	cbackup_u16 data;

	switch (xdrs->x_op) {
	case XDR_FREE:
		return TRUE;
	case XDR_ENCODE:
		data = htons(*objp);
		sz = xdrs->file->write(&data, sizeof(data));
		if (sz == sizeof(data))
			return TRUE;
		break;
	case XDR_DECODE:
		sz = xdrs->file->read(&data, sizeof(data));
		if (sz == sizeof(data)) {
			*objp = ntohs(data);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

bool_t
xdr_cbackup_u8(XDR *xdrs, cbackup_u8 *objp) {
	ssize_t sz;

	switch (xdrs->x_op) {
	case XDR_FREE:
		return TRUE;
	case XDR_ENCODE:
		sz = xdrs->file->write(objp, sizeof(*objp));
		if (sz == sizeof(*objp))
			return TRUE;
		break;
	case XDR_DECODE:
		sz = xdrs->file->read(objp, sizeof(*objp));
		if (sz == sizeof(*objp))
			return TRUE;
		break;
	}
	return FALSE;
}

bool_t
xdr_enum(XDR *xdrs, enum_t *objp) {
	return xdr_cbackup_u32(xdrs, objp);
}

void
xdr_do_command(XDR& xdr, command& cmd) {
	if (!xdr_command(&xdr, &cmd))
		throw Error("xdr failed");
}

