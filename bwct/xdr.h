/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _XDR_H
#define _XDR_H

class File;

#include <bwct/tool.h>

#define bwct_string String

#ifndef HAS_UINT64_T
# define bwct_u64 unsigned long long
#else
# define bwct_u64 uint64_t
#endif

#ifndef HAS_UINTX_T
# define bwct_u32 u_int32_t
# define bwct_u16 u_int16_t
# define bwct_u8 u_int8_t
#else
# define bwct_u32 uint32_t
# define bwct_u16 uint16_t
# define bwct_u8 uint8_t
#endif

#define bool_t bwct_u32
#define enum_t bwct_u32
#define TRUE 1
#define FALSE 0

enum xdr_ops {
	XDR_ENCODE=0,
	XDR_DECODE=1,
	XDR_FREE=2
};

class XDR {
public:
	enum xdr_ops x_op;
	File *file;
};

bool_t xdr_bwct_string(XDR *, bwct_string *);
bool_t xdr_bwct_u64(XDR *, bwct_u64 *);
bool_t xdr_bwct_u32(XDR *, bwct_u32 *);
bool_t xdr_bwct_u16(XDR *, bwct_u16 *);
bool_t xdr_bwct_u8(XDR *, bwct_u8 *);
bool_t xdr_enum(XDR *, enum_t *);

class command;
void xdr_do_command(XDR& xdr, command& cmd);

#endif /* !_XDR_H */
