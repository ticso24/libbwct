%/*
% * Copyright (c) 2001,02 Bernd Walter Computer Technology
% * All rights reserved.
% *
% * $URL$
% * $Date$
% * $Author$
% * $Rev$
% */
%

/*
 * media components
 */

%#include <bwct/config.h>
%#include <bwct/xdr.h>

enum devicetypes {
	DT_NONE,
	DT_TAPE,
	DT_FILE,
	DT_RAW
};

struct media_label {
	cbackup_string cbckcheck;
	cbackup_u64 id;
	cbackup_u64 date;
	cbackup_string name;
};

struct media_record {
	cbackup_u64 recno;
	cbackup_u64 streamid;
	cbackup_u64 chunkno;
};

/*
 * fs components
 */

/*
 * db components
 */

/*
 * command components
 */

enum commands {
	/* basic commands */
	C_QUIT = 0x0,
	/* server commands */
	C_SSTORE = 0x100,
	C_SMOUNT = 0x101,
	C_SUMOUNT = 0x102,
	C_SWRITE = 0x103,
	C_SLABEL = 0x104,
	C_STORE = 0x105,
	/* master server commands */
	C_RESTORE = 0x200,
	C_BACKUP = 0x201,
	C_LISTBACKUPS = 0x202,
	C_MOUNT = 0x203,
	C_UMOUNT = 0x204,
	C_WRITE = 0x205,
	C_LABEL = 0x206,
	C_SETCLIENT = 0x207,
	C_STREAMDB = 0x208,
	/* client commands */
	C_LOAD = 0x300,
	C_LISTDIR = 0x301,
	C_SAVE = 0x302,
	/* global returns */
	C_RET_OK = 0x400,
	C_RET_FAILED = 0x401,
	/* special returns */
	C_RET_LABEL = 0x500,
	C_RET_STRING = 0x501,
	C_RET_BACKUP = 0x502,
	C_RET_STAT = 0x503,
	C_RET_SAVESTAT = 0x504,
	/* lists */
	C_LIST = 0x600,
	C_LIST_END = 0x601
};

struct sc_mediaaction {
	enum devicetypes devtype;
	cbackup_string dev;
	cbackup_u64 mid;
	cbackup_string medianame;
	cbackup_string MServer;
};

struct sc_restore {
	cbackup_string Client;
	cbackup_u64 sid;
	cbackup_string targetdir;
};

struct sc_backup {
	cbackup_string Client;
	cbackup_string Server;
	cbackup_u32 level;
	cbackup_string partition;
};

struct sc_setclient {
	cbackup_string hostname;
	cbackup_u64 sserver;
};

struct sc_streamdb {
	cbackup_u64 media_id;
	cbackup_u64 mediapos;
	cbackup_u64 stream_id;
	cbackup_u64 chunkno;
};

struct sc_save {
	cbackup_string server;
	cbackup_u64 sid;
};

struct sc_load {
	cbackup_string server;
	cbackup_u64 sid;
};

struct sc_listdir {
	cbackup_string partition;
};

struct sc_ret_failed {
	cbackup_string reason;
};

struct sc_ret_backup {
	cbackup_string host;
	cbackup_u64 level;
	cbackup_u64 time;
	/* TODO: extend */
};

struct sc_ret_savestat {
	cbackup_string name;
	cbackup_u64 length;
	cbackup_u64 streampos;
	cbackup_u64 inode;
	cbackup_u64 mtime;
	cbackup_u64 mtimeusec;
	cbackup_u64 ctime;
	cbackup_u64 ctimeusec;
	cbackup_u64 size;
	cbackup_u64 mode;
	cbackup_u64 uid;
	cbackup_u64 gid;
	cbackup_u64 rdev;
	cbackup_u64 flags;
};

struct sc_ret_stat {
	cbackup_string name;
	cbackup_u64 inode;
	cbackup_u64 mtime;
	cbackup_u64 mtimeusec;
	cbackup_u64 ctime;
	cbackup_u64 ctimeusec;
	cbackup_u64 size;
	cbackup_u64 mode;
	cbackup_u64 uid;
	cbackup_u64 gid;
	cbackup_u64 rdev;
	cbackup_u64 flags;
};

struct sc_store {
	cbackup_u64 sid;
	cbackup_u64 cno;
};

struct sc_sstore {
	cbackup_string Client;
	cbackup_u64 sid;
};

union command switch (enum commands cmd) {
	/* basic commands */
	case C_QUIT:
		void;
	case C_SMOUNT:
	case C_SUMOUNT:
	case C_SWRITE:
	case C_SLABEL:
	case C_MOUNT:
	case C_UMOUNT:
	case C_WRITE:
	case C_LABEL:
		sc_mediaaction c_mediaaction;
	case C_SSTORE:
		sc_sstore c_sstore;
	case C_STORE:
		sc_store c_store;
	/* master server commands */
	case C_RESTORE:
		sc_restore c_restore;
	case C_BACKUP:
		sc_backup c_backup;
	case C_LISTBACKUPS:
		cbackup_string Client;
	case C_SETCLIENT:
		sc_setclient c_setclient;
	case C_STREAMDB:
		sc_streamdb c_streamdb;
	/* client commands */
	case C_LOAD:
		sc_load c_load;
	case C_LISTDIR:
		sc_listdir c_listdir;
	case C_SAVE:
		sc_save c_save;
	/* global returns */
	case C_RET_OK:
		void;
	case C_RET_FAILED:
		sc_ret_failed c_ret_failed;
	/* special returns */
	case C_RET_LABEL:
		media_label c_ret_label;
	case C_RET_STRING:
		cbackup_string c_ret_string;
	case C_RET_BACKUP:
		sc_ret_backup c_ret_backup;
	case C_RET_STAT:
		sc_ret_stat c_ret_stat;
	case C_RET_SAVESTAT:
		sc_ret_savestat c_ret_savestat;
	case C_LIST:
		cbackup_string c_list;
	case C_LIST_END:
		void;
};

