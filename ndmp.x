/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

/*                             -*- Mode: C -*- 
 * ndmp234.x
 * 
 * Description   : NDMP protocol rpcgen file for versions 2, 3 and 4.
 *
 * I've added some notes at the start of each section to explain some
 * of the changes between the versions.
 *
 * Naming Conventions:
 *
 * Where the object in question is essentially the same between the different
 * versions a single non-version specific object is defined. N.B. in some cases
 * the original .x file representation looked different but was the same in
 * its encoding and these have been considered as exactly the same. Also 
 * pure name changes have not been considered as important. NDMP V4 names
 * are used in this case.
 *
 * Where an object is specific to one or two versions of the protocol the
 * applicable version numbers are appended.
 *
 * Defining a completely different enum in cases where not all values were
 * applicable seemed to be generating work for not very much gain so enums
 * can include values with a mixture of version applicability
 *
 * Specific Points:
 *
 * 1) NDMP V2 ndmp_scsi_device and ndmp_tape_device are replaced directly by
 * strings in V3 and V4 making no difference to the actual over the wire
 * format. This file uses the V4 convention.
 *
 * 2) The V2 config_get_mover_type was renamed as config_get_connection_type
 * at V3 but no change of format was made so a common definition is used.
 *
 * There are probably a lot more things I should mention but life is too
 * short ....    
 */

/***************************************/
/*  COMMON STRUCTURES AND DEFINITIONS  */
/***************************************/

/* 
** CHANGE SUMMARY:
**
** The only major change in the common structures is in the area of
** connection addresses. V3 added Fibre Channel (FC) and Interprocess
** Communication (IPC) but FC was not really ever used and was withdrawn 
** at V4.
**
** V4 introduced TCP address lists to aid in "best route selection" where
** the listener has multiple LAN ports.
**
** V4 also introduced extensions and some more specific vendor area assignments 
*/

const NDMPVER_V2 = 2;
const NDMPVER_V3 = 3;
const NDMPVER_V4 = 4;
const NDMPPORT = 10000;

%#define ndmp_u_quad cbackup_u64
%#define xdr_ndmp_u_quad xdr_cbackup_u64

struct ndmp_pval /* Applicable to V2, V3 and V4 */ 
{ 
    cbackup_string      name; 
    cbackup_string      value; 
}; 

struct ndmp_u_quad /* Applicable to V2, V3 and V4 */
{ 
    cbackup_u32 high; 
    cbackup_u32 low; 
}; 

/* Inter-server connection addresses - see above for change descriptions */
enum ndmp_addr_type
{
    NDMP_ADDR_LOCAL     = 0, 
    NDMP_ADDR_TCP       = 1, 
    NDMP_ADDR_FC_V3     = 2, /* V3 only - never used? omitted from V4 */ 
    NDMP_ADDR_IPC_V34   = 3  /* New at V3 */
};

struct ndmp_tcp_addr_v23 /* called ndmp_mover_tcp_addr at v2 */
{
    cbackup_u32  ip_addr;
    cbackup_u16 port;
};

struct ndmp_tcp_addr_v4  
{ 
    cbackup_u32       ip_addr; 
    cbackup_u16      port; 
    ndmp_pval    addr_env<>; 
}; 

struct ndmp_fc_addr_v3
{
    cbackup_u32  loop_id;
};

struct ndmp_ipc_addr_v34
{
    opaque  comm_data<>;
};

union ndmp_mover_addr_v2 switch (ndmp_addr_type addr_type)
{
    case NDMP_ADDR_LOCAL:
        void;
    case NDMP_ADDR_TCP:
      ndmp_tcp_addr_v23 addr;
};

union ndmp_addr_v3 switch (ndmp_addr_type addr_type)
{
    case NDMP_ADDR_LOCAL:
        void;
    case NDMP_ADDR_TCP:
        ndmp_tcp_addr_v23 tcp_addr;
    case NDMP_ADDR_FC:
        ndmp_fc_addr_v3   fc_addr;
    case NDMP_ADDR_IPC:
        ndmp_ipc_addr_v34 ipc_addr;
    
};

union ndmp_addr_v4 switch (ndmp_addr_type addr_type)  
{ 
    case NDMP_ADDR_LOCAL: 
        void; 
    case NDMP_ADDR_TCP: 
        ndmp_tcp_addr_v4  tcp_addr<>; 
    case NDMP_ADDR_IPC: 
        ndmp_ipc_addr_v34 ipc_addr; 
};  


/* Note: because of extensibility, this is */ 
/* not a complete list of errors. */ 
enum ndmp_error  
{ 
    NDMP_NO_ERR                     =  0,  
    NDMP_NOT_SUPPORTED_ERR            =  1, /* Call is not supported */
    NDMP_DEVICE_BUSY_ERR              =  2, /* The device is in use */
    NDMP_DEVICE_OPENED_ERR            =  3, /* Another tape or scsi device  
                                             *is already open */
    NDMP_NOT_AUTHORIZED_ERR           =  4, /* connection not authorized */
    NDMP_PERMISSION_ERR               =  5, /* permission problem */
    NDMP_DEV_NOT_OPEN_ERR             =  6, /* Tape/SCSI device is not open */
    NDMP_IO_ERR                       =  7, /* Input/Output error */   
    NDMP_TIMEOUT_ERR                  =  8,    
    NDMP_ILLEGAL_ARGS_ERR             =  9, /* illegal arguments in request */    
    NDMP_NO_TAPE_LOADED_ERR           = 10, /* Cannot open because there is 
                                             * no tape loaded */
    NDMP_WRITE_PROTECT_ERR            = 11, /* cannot open tape for write */
    NDMP_EOF_ERR                      = 12, /* EOF encountered on tape */   
    NDMP_EOM_ERR                      = 13, /* EOM encountered on tape */    
    NDMP_FILE_NOT_FOUND_ERR           = 14, /* File not found during restore */  
    NDMP_BAD_FILE_ERR                 = 15, /* File descriptor is invalid */   
    NDMP_NO_DEVICE_ERR                = 16, /* No device at that target */   
    NDMP_NO_BUS_ERR                   = 17, /* Invalid SCSI bus controller */   
    NDMP_XDR_DECODE_ERR               = 18, /* Can't decode the request */   
    NDMP_ILLEGAL_STATE_ERR            = 19, /* Request sent in illegal state*/   
    NDMP_UNDEFINED_ERR                = 20, /* No other error code applies */    
    NDMP_XDR_ENCODE_ERR               = 21, /* Can't encode the reply */   
    NDMP_NO_MEM_ERR                   = 22, /* No memory */   
    NDMP_CONNECT_ERR_V34              = 23, /* Error connecting to NDMP svr */ 
    NDMP_SEQUENCE_NUM_ERR_V4          = 24, /* ndmp_header seq number error */   
    NDMP_READ_IN_PROGRESS_ERR_V4      = 25, /* Reply to NDMP_MOVER_READ */ 
    NDMP_PRECONDITION_ERR_V4          = 26, /* Preparatory set up not done */  
    NDMP_CLASS_NOT_SUPPORTED_ERR_V4   = 27, /* For extensions */ 
    NDMP_VERSION_NOT_SUPPORTED_ERR_V4 = 28, /* For extensions */ 
    NDMP_EXT_DUPL_CLASSES_ERR_V4      = 29, /* For extensions */ 
    NDMP_EXT_DANDN_ILLEGAL_ERR_V4     = 30  /* For extensions */ 
}; 


/* Note: Because of extensibility, this */ 
/* is not a complete list of messages */ 
enum ndmp_message  
{ 
    NDMP_CONNECT_OPEN               = 0x900, 
    NDMP_CONNECT_CLIENT_AUTH        = 0x901, 
    NDMP_CONNECT_CLOSE              = 0x902, 
    NDMP_CONNECT_SERVER_AUTH        = 0x903, 

    NDMP_CONFIG_GET_HOST_INFO       = 0x100, 
    NDMP_CONFIG_GET_BUTYPE_ATTR_V2  = 0x101, /* Replaced after V2 */
    NDMP_CONFIG_GET_CONNECTION_TYPE = 0x102, /* NDMP_CONFIG_GET_MOVER_TYPE on v2 */           
    NDMP_CONFIG_GET_AUTH_ATTR       = 0x103, 
    NDMP_CONFIG_GET_BUTYPE_INFO_V34 = 0x104, /* New in V3 */
    NDMP_CONFIG_GET_FS_INFO_V34     = 0x105, /* New in V3 */ 
    NDMP_CONFIG_GET_TAPE_INFO_V34   = 0x106, /* New in V3 */ 
    NDMP_CONFIG_GET_SCSI_INFO_V34   = 0x107, /* New in V3 */ 
    NDMP_CONFIG_GET_SERVER_INFO_V34 = 0x108, /* New in V3 */ 
    NDMP_CONFIG_SET_EXT_LIST_V4     = 0x109, /* New in V4 */ 
    NDMP_CONFIG_GET_EXT_LIST_V4     = 0x10A, /* New in V4 */ 

    NDMP_SCSI_OPEN                  = 0x200,     
    NDMP_SCSI_CLOSE                 = 0x201, 
    NDMP_SCSI_GET_STATE             = 0x202, 
    NDMP_SCSI_SET_TARGET_V23        = 0x203, /* Removed in V4 */
    NDMP_SCSI_RESET_DEVICE          = 0x204, 
    NDMP_SCSI_RESET_BUS_V23         = 0x205, /* Removed in V4 */
    NDMP_SCSI_EXECUTE_CDB           = 0x206, 

    NDMP_TAPE_OPEN                  = 0x300, 
    NDMP_TAPE_CLOSE                 = 0x301, 
    NDMP_TAPE_GET_STATE             = 0x302, 
    NDMP_TAPE_MTIO                  = 0x303, 
    NDMP_TAPE_WRITE                 = 0x304, 
    NDMP_TAPE_READ                  = 0x305, 
    NDMP_TAPE_SET_RECORD_SIZE_V1    = 0x306, /* V1 only */
    NDMP_TAPE_EXECUTE_CDB           = 0x307, 

    NDMP_DATA_GET_STATE             = 0x400, 
    NDMP_DATA_START_BACKUP          = 0x401, 
    NDMP_DATA_START_RECOVER         = 0x402, 
    NDMP_DATA_ABORT                 = 0x403, 
    NDMP_DATA_GET_ENV               = 0x404, 
    NDMP_DATA_RESVD1_V1             = 0x405, /* V1 only ? DATA_CONTINUE ? */
    NDMP_DATA_RESVD2_V1             = 0x406, /* V1 only ?? */
    NDMP_DATA_STOP                  = 0x407, 
    NDMP_DATA_CONTINUE_V1           = 0x408, /* V1 only -> mover continue? */
    NDMP_DATA_LISTEN_V34            = 0x409, /* New at V3 */
    NDMP_DATA_CONNECT_V34           = 0x40A, /* V3 - split from DATA_START.. */ 
    NDMP_DATA_START_RECOVER_FILEHIST_V4 = 0x40B, /* New at V4 */ 

    NDMP_NOTIFY_PAUSED_V1           = 0x500, /* V1 only */
    NDMP_NOTIFY_DATA_HALTED         = 0x501,     
    NDMP_NOTIFY_CONNECTION_STATUS   = 0x502, /* Was NDMP_NOTIFY_CONNECTED */     
    NDMP_NOTIFY_MOVER_HALTED        = 0x503, 
    NDMP_NOTIFY_MOVER_PAUSED        = 0x504, 
    NDMP_NOTIFY_DATA_READ           = 0x505, 

    NDMP_LOG_LOG_V2                 = 0x600, /*Replaced by LOG_MESSAGE in V3*/
    NDMP_LOG_DEBUG_V2               = 0x601, /*Replaced by LOG_MESSAGE in V3*/
    NDMP_LOG_FILE                   = 0x602, 
    NDMP_LOG_MESSAGE_V34            = 0x603, /* Replaces V2 600 & 601 */ 

    NDMP_FH_ADD_UNIX_PATH_V2        = 0x700, /* Replaced by 703 in V3 */ 
    NDMP_FH_ADD_UNIX_DIR_V2         = 0x701, /* Replaced by 704 in V3 */
    NDMP_FH_ADD_UNIX_NODE_V2        = 0x702, /* Replaced by 705 in V3 */
    NDMP_FH_ADD_FILE_V34            = 0x703, 
    NDMP_FH_ADD_DIR_V34             = 0x704, 
    NDMP_FH_ADD_NODE_V34            = 0x705, 

    NDMP_MOVER_GET_STATE            = 0xA00, 
    NDMP_MOVER_LISTEN               = 0xA01, 
    NDMP_MOVER_CONTINUE             = 0xA02, 
    NDMP_MOVER_ABORT                = 0xA03, 
    NDMP_MOVER_STOP                 = 0xA04, 
    NDMP_MOVER_SET_WINDOW           = 0xA05, 
    NDMP_MOVER_READ                 = 0xA06, 
    NDMP_MOVER_CLOSE                = 0xA07, 
    NDMP_MOVER_SET_RECORD_SIZE      = 0xA08, 
    NDMP_MOVER_CONNECT_V34          = 0xA09, /* new at V3 */ 

    /* The following V2/V3 prototyping ranges obsoleted by V4 extensions */
    NDMP_VENDORS_BASE_V3            = 0xf000,   /* Reserved for the vendor 
                                                 * specific usage
                                                 * from 0xf000 to 0xfeff */
    NDMP_RESERVED_BASE_V23          = 0xff00,   /* Reserved for prototyping 
                                                 * from 0xff00 to 0xffff */

    /* New V4 extensions bases */
    NDMP_EXT_STANDARD_BASE_V4        = 0x10000, /* new at V4 */ 
    NDMP_EXT_PROPRIETARY_BASE_V4     = 0x20000000 /* new at V4 */
};

enum ndmp_header_message_type  
{ 
    NDMP_MESSAGE_REQUEST          = 0, 
    NDMP_MESSAGE_REPLY            = 1 
}; 

const NDMP4_MESSAGE_POST = NDMP_MESSAGE_REQUEST; 

struct ndmp_header  
{ 
    cbackup_u32                    sequence; 
    cbackup_u32                    time_stamp; 
    ndmp_header_message_type  message_type; 
    ndmp_message              message_code; 
    cbackup_u32                    reply_sequence; 
    ndmp_error                error_code; 
};
 

 
/**********************/
/*  CONNECT INTERFACE */
/**********************/

/* CHANGE SUMMARY: No real changes */

/* NDMP_CONNECT_OPEN */
struct ndmp_connect_open_request  
{ 
    cbackup_u16     protocol_version; 
};  

struct ndmp_connect_open_reply  
{ 
    ndmp_error  error; 
}; 

/* NDMP_CONNECT_CLIENT_AUTH */
enum ndmp_auth_type  
{ 
    NDMP_AUTH_NONE  = 0, 
    NDMP_AUTH_TEXT  = 1, 
    NDMP_AUTH_MD5   = 2 
};  

struct ndmp_auth_text  
{ 
    cbackup_string auth_id; 
    cbackup_string auth_password; 
};  

struct ndmp_auth_md5  
{ 
    cbackup_string   auth_id; 
    opaque   auth_digest[16]; 
};  

union ndmp_auth_data switch (enum ndmp_auth_type auth_type)  
{ 
    case NDMP_AUTH_NONE: 
        void; 
    case NDMP_AUTH_TEXT: 
        struct ndmp_auth_text   auth_text; 
    case NDMP_AUTH_MD5: 
        struct ndmp_auth_md5    auth_md5; 
};  

struct ndmp_connect_client_auth_request  
{ 
    ndmp_auth_data   auth_data; 
};  

struct ndmp_connect_client_auth_reply  
{ 
    ndmp_error       error; 
};  

/* NDMP_CONNECT_CLOSE */
/* no request arguments */
/* no reply message */

/* NDMP_CONNECT_SERVER_AUTH */
union ndmp_auth_attr switch (enum ndmp_auth_type auth_type)
{
    case NDMP_AUTH_NONE:
        void;
    case NDMP_AUTH_TEXT:
        void;
    case NDMP_AUTH_MD5:
        opaque  challenge[64];
};

struct ndmp_connect_server_auth_request  
{ 
    ndmp_auth_attr   client_attr; 
};  

struct ndmp_connect_server_auth_reply  
{ 
    ndmp_error            error; 
    ndmp_auth_data        server_result; 
};  

/********************/
/* CONFIG INTERFACE */
/********************/

/*
** CHANGE SUMMARY:
**
** Fairly major changes at V3 - mostly new messages - see specific messages
** for details
*/

/* NDMP_CONFIG_GET_HOST_INFO */
/* no request arguments */

struct ndmp_config_get_host_info_reply_v2 /* V2 only */
{
    ndmp_error          error;
    cbackup_string              hostname; /* host name */
    cbackup_string              os_type;  /* The operating system type (i.e.
                                     * SOLARIS) */
    cbackup_string              os_vers;  /* The version number of the OS (i.e.
                                     * 2.5) */
    cbackup_string              hostid;
    ndmp_auth_type      auth_type<>; /* Moved to separate request at V3 */
};

struct ndmp_config_get_host_info_reply_v34  /* V3/V4 only */  
{ 
    ndmp_error  error; 
    cbackup_string      hostname; 
    cbackup_string      os_type; /* Operating system type (e.g. SOLARIS) */ 
    cbackup_string      os_vers; /* Operating system version */ 
    cbackup_string      hostid; /* Ethernet MAC address in displayable hex */ 
}; 

/* NDMP_CONFIG_GET_BUTYPE_ATTR_V2 - V2 only replaced by GET_BUTYPE_INFO */
const NDMP_NO_BACKUP_FILELIST_V2    = 0x0001;
const NDMP_NO_BACKUP_FHINFO_V2      = 0x0002;
const NDMP_NO_RECOVER_FILELIST_V2   = 0x0004;
const NDMP_NO_RECOVER_FHINFO_V2     = 0x0008;
const NDMP_NO_RECOVER_SSID_V2       = 0x0010;
const NDMP_NO_RECOVER_INC_ONLY_V2   = 0x0020;

struct ndmp_config_get_butype_attr_request_v2
{
    cbackup_string  name;     /* backup type name */
};

struct ndmp_config_get_butype_attr_reply_v2
{
    ndmp_error  error;
    cbackup_u32      attrs;
};

/* NDMP_CONFIG_GET_CONNECTION_TYPE
   N.B. at V2 this was called mover_type rather than connection_type */
/* no request arguments */

struct ndmp_config_get_connection_type_reply
{
    ndmp_error      error;
    ndmp_addr_type  addr_types<>;
};


/* NDMP_CONFIG_GET_AUTH_ATTR */
/* ndmp_auth_attr defined above */ 
struct ndmp_config_get_auth_attr_request  
{ 
    ndmp_auth_type      auth_type; 
};  

struct ndmp_config_get_auth_attr_reply  
{ 
    ndmp_error          error; 
    ndmp_auth_attr      server_attr; 
}; 

/* NDMP_CONFIG_GET_BUTYPE_INFO */
/* no request arguments */

/* backup type attributes */
const NDMP_BUTYPE_BACKUP_FILE_HISTORY_V3   = 0x0001; /* V3 only */
const NDMP_BUTYPE_BACKUP_FILELIST_V34      = 0x0002; 
const NDMP_BUTYPE_RECOVER_FILELIST_V34     = 0x0004; 
const NDMP_BUTYPE_BACKUP_DIRECT_V34        = 0x0008; 
const NDMP_BUTYPE_RECOVER_DIRECT_V34       = 0x0010; 
const NDMP_BUTYPE_BACKUP_INCREMENTAL_V34   = 0x0020; 
const NDMP_BUTYPE_RECOVER_INCREMENTAL_V34  = 0x0040; 
const NDMP_BUTYPE_BACKUP_UTF8_V34          = 0x0080; 
const NDMP_BUTYPE_RECOVER_UTF8_V34         = 0x0100; 
const NDMP_BUTYPE_BACKUP_FH_FILE_V4        = 0x0200; /* V4 only */ 
const NDMP_BUTYPE_BACKUP_FH_DIR_V4         = 0x0400; /* V4 only */ 
const NDMP_BUTYPE_RECOVER_FILEHIST_V4      = 0x0800; /* V4 only */ 
const NDMP_BUTYPE_RECOVER_FH_FILE_V4       = 0x1000; /* V4 only */ 
const NDMP_BUTYPE_RECOVER_FH_DIR_V4        = 0x2000; /* V4 only */ 

struct ndmp_butype_info_v34  
{ 
    cbackup_string      butype_name; 
    ndmp_pval   default_env<>; 
    cbackup_u32      attrs; 
};  

struct ndmp_config_get_butype_attr_reply_v34  
{ 
    ndmp_error            error; 
    ndmp_butype_info_v34  butype_info<>; 
}; 

/* NDMP_CONFIG_GET_FS_INFO_V34 */
/* no request arguments */

/* unsupported/invalid bit - at V3 was ..._INVALID */
const NDMP_FS_INFO_TOTAL_SIZE_UNS_V34    = 0x00000001; 
const NDMP_FS_INFO_USED_SIZE_UNS_V34     = 0x00000002; 
const NDMP_FS_INFO_AVAIL_SIZE_UNS_V34    = 0x00000004; 
const NDMP_FS_INFO_TOTAL_INODES_UNS_V34  = 0x00000008; 
const NDMP_FS_INFO_USED_INODES_UNS_V34   = 0x00000010;  

struct ndmp_fs_info_v34  
{ 
    cbackup_u32            unsupported; /* At V3 was called "invalid" */ 
    cbackup_string            fs_type; 
    cbackup_string            fs_logical_device; 
    cbackup_string            fs_physical_device; 
    ndmp_u_quad       total_size; 
    ndmp_u_quad       used_size; 
    ndmp_u_quad       avail_size; 
    ndmp_u_quad       total_inodes; 
    ndmp_u_quad       used_inodes; 
    ndmp_pval         fs_env<>; 
    cbackup_string            fs_status; 
};  

struct ndmp_config_get_fs_info_reply_v34  
{ 
    ndmp_error        error; 
    ndmp_fs_info_v34  fs_info<>; 
}; 


/* NDMP_CONFIG_GET_TAPE_INFO_V34 */
/* no request arguments */
/* tape attributes */
const NDMP_TAPE_ATTR_REWIND_V34 = 0x00000001; 
const NDMP_TAPE_ATTR_UNLOAD_V34 = 0x00000002; 
const NDMP_TAPE_ATTR_RAW_V4    = 0x00000004;  /* New at V4 */ 

struct ndmp_device_capability_v34  
{ 
    cbackup_string                  device; 
    cbackup_u32                  attr; 
    ndmp_pval               capability<>; 
};  

struct ndmp_device_info_v34  
{ 
    cbackup_string                      model; 
    ndmp_device_capability_v34  caplist<>; 
}; 

struct ndmp_config_get_tape_info_reply_v34  
{ 
    ndmp_error              error; 
    ndmp_device_info_v34    tape_info<>; 
};  

/* NDMP_CONFIG_GET_SCSI_INFO_V34 */
/* no request arguments */
/* jukebox attributes */
struct ndmp_config_get_scsi_info_reply_v34
{
    ndmp_error              error;
    ndmp_device_info_v34    scsi_info<>;
};

/* NDMP_CONFIG_GET_SERVER_INFO_V34 */
/* no request arguments */
struct ndmp_config_get_server_info_reply_v34  
{ 
    ndmp_error        error; 
    cbackup_string            vendor_name; 
    cbackup_string            product_name; 
    cbackup_string            revision_number; 
    ndmp_auth_type    auth_type<>; 
}; 


/**************************************/
/* EXTENSIONS CONFIG INTERFACE V4 ONLY*/
/**************************************/

struct ndmp_class_list_v4  
{ 
    cbackup_u16      ext_class_id; 
    cbackup_u16      ext_version<>; 
};  

struct ndmp_class_version_v4 
{ 
    cbackup_u16      ext_class_id; 
    cbackup_u16      ext_version; 
}; 

/* NDMP_CONFIG_GET_EXT_LIST_V4 */
/* no request arguments */
struct ndmp_config_get_ext_list_reply_v4 
{ 
    ndmp_error         error; 
    ndmp_class_list_v4 class_list<>; 
}; 

/* NDMP_CONFIG_SET_EXT_LIST_V4 */
struct ndmp_config_set_ext_list_request_v4 
{ 
    ndmp_class_version_v4    ndmp_selected_ext<>; 
};  
                   
struct ndmp_config_set_ext_list_reply_v4 
{ 
    ndmp_error      error; 
}; 


/******************/
/* SCSI INTERFACE */
/******************/

/*
** CHANGE SUMMARY:
**
** Use of NDMP_SCSI_SET_TARGET and NDMP_SCSI_RESET_BUS is removed
** in V4. The former is removed because the server is now expected to
** offer a list of specific devices rather than a generic SCSI bus access
** device - for security reasons. The reset bus is also removed because 
** it is considered dangerous.
** Along similar lines, NDMP_SCSI_RESET_DEVICE should only be supported
** by the server if the reset is restricted specifically to the device
** referenced and not some larger device containing it. 
*/ 

/* NDMP_SCSI_OPEN */
struct ndmp_scsi_open_request 
{ 
    cbackup_string      device; 
}; 

struct ndmp_scsi_open_reply 
{ 
    ndmp_error      error; 
}; 

/* NDMP_SCSI_CLOSE */
/* no request arguments */
struct ndmp_scsi_close_reply 
{ 
    ndmp_error      error; 
}; 

/* NDMP_SCSI_GET_STATE */
/* no request arguments */
struct ndmp_scsi_get_state_reply 
{ 
    ndmp_error       error; 
    short            target_controller; 
    short            target_id; 
    short            target_lun; 
}; 

/* NDMP_SCSI_SET_TARGET_V23 - removed at V4 */
struct ndmp_scsi_set_target_request_v23
{
    cbackup_string      device;
    cbackup_u16     target_controller;
    cbackup_u16     target_id;
    cbackup_u16     target_lun;
};

struct ndmp_scsi_set_target_reply_v23
{
    ndmp_error  error;
};

/* NDMP_SCSI_RESET_DEVICE */
/* no request arguments */
struct ndmp_scsi_reset_device_reply 
{ 
    ndmp_error      error; 
}; 


/* NDMP_SCSI_RESET_BUS_V23 - removed at V4 */
/* no request arguments */
struct ndmp_scsi_reset_bus_reply_v23
{
    ndmp_error  error;
};

/* NDMP_SCSI_EXECUTE_CDB */
const NDMP_SCSI_DATA_IN  = 0x00000001;  /* Expect data from SCSI device */ 
const NDMP_SCSI_DATA_OUT = 0x00000002;  /* Transfer data to SCSI device */

struct ndmp_execute_cdb_request 
{ 
    cbackup_u32            flags; 
    cbackup_u32            timeout; 
    cbackup_u32            datain_len; 
    opaque            cdb<>; 
    opaque            dataout<>; 
}; 

struct ndmp_execute_cdb_reply 
{ 
    ndmp_error        error; 
    cbackup_u8            status; 
    cbackup_u32            dataout_len; 
    opaque            datain<>; 
    opaque            ext_sense<>; 
}; 

/******************/
/* TAPE INTERFACE */
/******************/

/*
** CHANGE SUMMARY:
**
** Only minor details change in the XDR definition through V2 to V4.
**
** However, the semantics of the messages are more explicitly stated
** in the V4 spec and there may be minor differences in usage between
** V2/V3 and V4 in some products.
*/

/* NDMP_TAPE_OPEN */
enum ndmp_tape_open_mode 
{ 
    NDMP_TAPE_READ_MODE   = 0, 
    NDMP_TAPE_RDWR_MODE   = 1, 
    NDMP_TAPE_RAW_MODE_V4 = 2 /* New at V4 */ 
}; 

struct ndmp_tape_open_request  
{ 
    cbackup_string                   device; 
    ndmp_tape_open_mode      mode; 
}; 

struct ndmp_tape_open_reply  
{ 
    ndmp_error      error; 
}; 

/* NDMP_TAPE_CLOSE */
/* no request arguments */
struct ndmp_tape_close_reply 
{ 
    ndmp_error      error; 
}; 

/* NDMP_TAPE_GET_STATE */
/* no request arguments */

/* At V2 the following had _TAPE_ rather than _TAPE_STATE_ */
const NDMP_TAPE_STATE_NOREWIND         = 0x0008; /* no rewind on close */  
const NDMP_TAPE_STATE_WR_PROT          = 0x0010; /* write-protected */ 
const NDMP_TAPE_STATE_ERROR            = 0x0020; /* media error */ 
const NDMP_TAPE_STATE_UNLOAD           = 0x0040; /* tape unloaded on close */

/* The following were named ..._INVALID rather that ..._UNS at V3 */   
const NDMP_TAPE_STATE_FILE_NUM_UNS_V34     = 0x00000001; 
const NDMP_TAPE_STATE_SOFT_ERRORS_UNS_V34  = 0x00000002; 
const NDMP_TAPE_STATE_BLOCK_SIZE_UNS_V34   = 0x00000004; 
const NDMP_TAPE_STATE_BLOCKNO_UNS_V34      = 0x00000008; 
const NDMP_TAPE_STATE_TOTAL_SPACE_UNS_V34  = 0x00000010; 
const NDMP_TAPE_STATE_SPACE_REMAIN_UNS_V34 = 0x00000020; 
const NDMP_TAPE_STATE_PARTITION_UNS_V3     = 0x00000040; 

struct ndmp_tape_get_state_reply_v2
{
    ndmp_error  error;
    cbackup_u32      flags;
    cbackup_u32      file_num;
    cbackup_u32      soft_errors;
    cbackup_u32      block_size;
    cbackup_u32      blockno;
    ndmp_u_quad total_space;
    ndmp_u_quad space_remain;
};

struct ndmp_tape_get_state_reply_v3
{
    cbackup_u32      invalid; /* Use the ..._UNS bit settings */
    ndmp_error  error;
    cbackup_u32      flags;
    cbackup_u32      file_num;
    cbackup_u32      soft_errors;
    cbackup_u32      block_size;
    cbackup_u32      blockno;
    ndmp_u_quad total_space;
    ndmp_u_quad space_remain;
    cbackup_u32      partition;  /* V3 only */
};

struct ndmp_tape_get_state_reply_v4 
{ 
    cbackup_u32       unsupported; /* renamed "invalid" field */ 
    ndmp_error   error; 
    cbackup_u32       flags; 
    cbackup_u32       file_num; 
    cbackup_u32       soft_errors; 
    cbackup_u32       block_size; 
    cbackup_u32       blockno; 
    ndmp_u_quad  total_space; 
    ndmp_u_quad  space_remain; 
}; 

/* NDMP_TAPE_SET_RECORD_SIZE_V1 - V1 only - not defined here */

/* NDMP_TAPE_MTIO */

enum ndmp_tape_mtio_op 
{ 
    NDMP_MTIO_FSF     = 0,   /* Forward space files */ 
    NDMP_MTIO_BSF     = 1,   /* Backward space files */ 
    NDMP_MTIO_FSR     = 2,   /* Forward space records */ 
    NDMP_MTIO_BSR     = 3,   /* Backward space records */ 
    NDMP_MTIO_REW     = 4,   /* Rewind */ 
    NDMP_MTIO_EOF     = 5,   /* Write filemark(s) */ 
    NDMP_MTIO_OFF     = 6,   /* Unload tape */ 
    NDMP_MTIO_TUR_V4  = 7    /* Test Unit Ready : New at V4 */ 
}; 

struct ndmp_tape_mtio_request 
{ 
    ndmp_tape_mtio_op   tape_op; 
    cbackup_u32              count; 
}; 

struct ndmp_tape_mtio_reply 
{ 
    ndmp_error          error; 
    cbackup_u32              resid_count; 
}; 

/* NDMP_TAPE_WRITE */
struct ndmp_tape_write_request 
{ 
    opaque              data_out<>; 
}; 

struct ndmp_tape_write_reply 
{ 
    ndmp_error          error; 
    cbackup_u32              count; 
}; 


/* NDMP_TAPE_READ */
struct ndmp_tape_read_request 
{ 
    cbackup_u32              count; 
}; 

struct ndmp_tape_read_reply 
{ 
    ndmp_error          error; 
    opaque              data_in<>; 
}; 

typedef ndmp_execute_cdb_request ndmp_tape_execute_cdb_request; 
typedef ndmp_execute_cdb_reply ndmp_tape_execute_cdb_reply; 


/********************************/
/* DATA INTERFACE               */
/********************************/

/*
** CHANGE SUMMARY:
**
** At V3 the data/mover connection is separated from the start backup and
** start recovery requests and becomes bi-directional. Hence new
** ndmp_data_connect_request/ndmp_data_listen_request and new data states.
** (N.B. this allows replication using standard NDMP protocols - ndmpcopy.)
**
** Note that the recovery name structure was used in different ways by 
** different vendors at V3 and use may differ between V3 and V4. 
*/

/* NDMP_DATA_GET_STATE */
/* no request arguments */

enum ndmp_data_operation  
{  
    NDMP_DATA_OP_NOACTION              = 0,  
    NDMP_DATA_OP_BACKUP                = 1,  
    NDMP_DATA_OP_RECOVER               = 2,  
    NDMP_DATA_OP_RECOVER_FILEHIST_V4   = 3  /* New at V4 */  
}; 

enum ndmp_data_state  
{ 
    NDMP_DATA_STATE_IDLE          = 0, 
    NDMP_DATA_STATE_ACTIVE        = 1, 
    NDMP_DATA_STATE_HALTED        = 2, 
    NDMP_DATA_STATE_LISTEN_V34    = 3, /* New at V3 */ 
    NDMP_DATA_STATE_CONNECTED_V34 = 4  /* New at V3 */ 
};  

enum ndmp_data_halt_reason  
{ 
    NDMP_DATA_HALT_NA             = 0, 
    NDMP_DATA_HALT_SUCCESSFUL     = 1, 
    NDMP_DATA_HALT_ABORTED        = 2, 
    NDMP_DATA_HALT_INTERNAL_ERROR = 3, 
    NDMP_DATA_HALT_CONNECT_ERROR  = 4 
}; 

struct ndmp_data_get_state_reply_v2
{
    ndmp_error                  error;
    ndmp_data_operation         operation;
    ndmp_data_state             state;
    ndmp_data_halt_reason       halt_reason;
    ndmp_u_quad                 bytes_processed;
    ndmp_u_quad                 est_bytes_remain;
    cbackup_u32                      est_time_remain;
    ndmp_mover_addr_v2          mover;
    ndmp_u_quad                 read_offset;
    ndmp_u_quad                 read_length;
};

/* Following for "unsupported" field - was called "invalid" at V3 */
const NDMP_DATA_STATE_EST_BYTES_REMAIN_UNS_V34 = 0x00000001; 
const NDMP_DATA_STATE_EST_TIME_REMAIN_UNS_V34  = 0x00000002; 

struct ndmp_data_get_state_reply_v3
{
    cbackup_u32                  invalid;
    ndmp_error              error;
    ndmp_data_operation     operation;
    ndmp_data_state         state;
    ndmp_data_halt_reason   halt_reason;
    ndmp_u_quad             bytes_processed;
    ndmp_u_quad             est_bytes_remain;
    cbackup_u32                  est_time_remain;
    ndmp_addr_v3            data_connection_addr;
    ndmp_u_quad             read_offset;
    ndmp_u_quad             read_length;
};

struct ndmp_data_get_state_reply_v4  
{ 
    cbackup_u32                    unsupported; 
    ndmp_error                error; 
    ndmp_data_operation       operation; 
    ndmp_data_state           state; 
    ndmp_data_halt_reason     halt_reason; 
    ndmp_u_quad               bytes_processed; 
    ndmp_u_quad               est_bytes_remain; 
    cbackup_u32                    est_time_remain; 
    ndmp_addr_v4              data_connection_addr; 
    ndmp_u_quad               read_offset; 
    ndmp_u_quad               read_length; 
}; 


/* NDMP_DATA_START_BACKUP */
/* At V3 connection establishment was separated from start backup */

struct ndmp_data_start_backup_request_v2
{
    ndmp_mover_addr_v2  mover;      /* mover to receive data */
    cbackup_string              bu_type;  /* backup method to use */
    ndmp_pval           env<>;      /* Parameters that may modify backup */
};

struct ndmp_data_start_backup_request_v34  
{  
    cbackup_string          butype_name;  
    ndmp_pval       env<>;  
}; 

struct ndmp_data_start_backup_reply  
{ 
    ndmp_error     error; 
}; 

/* NDMP_DATA_START_RECOVER */
/* At V3 connection establishment was separated from start backup */

struct ndmp_name_v2
{
    cbackup_string      name;
    cbackup_string      dest;
    cbackup_u16     ssid;
    ndmp_u_quad fh_info;
};

struct ndmp_data_start_recover_request_v2
{
    ndmp_mover_addr_v2 mover;
    ndmp_pval          env<>;
    ndmp_name_v2       nlist<>;
    cbackup_string             bu_type;

};

/*
** NOTE: Although the ndmp_name structure is essentially the same in
** V3 and V4 it was used in different ways at V3 by many vendors
*/
struct ndmp_name_v34  
{ 
    cbackup_string       original_path; 
    cbackup_string       destination_dir; 
    cbackup_string       name;    /* was new_name at V3 */ 
    cbackup_string       other_name; 
    ndmp_u_quad  node; 
    ndmp_u_quad  fh_info; 
};  

struct ndmp_data_start_recover_request_v34  
{  
    ndmp_pval       env<>;  
    ndmp_name_v34   nlist<>;  
    cbackup_string          butype_name;  
};  

struct ndmp_data_start_recover_reply  
{ 
    ndmp_error      error; 
}; 

/* NDMP_DATA_ABORT */
/* no request arguments */
struct ndmp_data_abort_reply  
{ 
    ndmp_error error; 
}; 

/* NDMP_DATA_STOP */
/* no request arguments */
struct ndmp_data_stop_reply  
{ 
    ndmp_error error; 
}; 

/* NDMP_DATA_GET_ENV */
/* no request arguments */
struct ndmp_data_get_env_reply  
{ 
    ndmp_error  error; 
    ndmp_pval   env<>; 
}; 


/* NDMP_DATA_LISTEN - new at v3 */
struct ndmp_data_listen_request_v34  
{ 
    ndmp_addr_type   addr_type; 
};  

struct ndmp_data_listen_reply_v3  
{ 
    ndmp_error   error; 
    ndmp_addr_v3 connect_addr; 
}; 

struct ndmp_data_listen_reply_v4  
{ 
    ndmp_error   error; 
    ndmp_addr_v4 connect_addr; 
}; 

/* NDMP_DATA_CONNECT - new at v3 */
struct ndmp_data_connect_request_v3  
{ 
    ndmp_addr_v3   addr; 
};  

struct ndmp_data_connect_request_v4  
{ 
    ndmp_addr_v4   addr; 
};  

struct ndmp_data_connect_reply_v34  
{ 
    ndmp_error   error; 
}; 

/********************************/
/* MOVER INTERFACE              */
/********************************/

/*
** CHANGE SUMMARY:
**
** At V3 the data/mover connection is separated from the start backup and
** start recovery requests and becomes bi-directional. Hence new
** ndmp_mover_connect_request.
**
** Note at V4 the states in which ndmp_mover_set_window and
** ndmp_mover_set_record_size are sent are changed - see V3/V4 specs. 
*/

/* NDMP_MOVER_GET_STATE */
/* no request arguments */

enum ndmp_mover_state 
{ 
    NDMP_MOVER_STATE_IDLE    = 0, 
    NDMP_MOVER_STATE_LISTEN  = 1, 
    NDMP_MOVER_STATE_ACTIVE  = 2, 
    NDMP_MOVER_STATE_PAUSED  = 3, 
    NDMP_MOVER_STATE_HALTED  = 4 
}; 

enum ndmp_mover_pause_reason 
{ 
    NDMP_MOVER_PAUSE_NA              = 0, 
    NDMP_MOVER_PAUSE_EOM             = 1, 
    NDMP_MOVER_PAUSE_EOF             = 2, 
    NDMP_MOVER_PAUSE_SEEK            = 3, 
    NDMP_MOVER_PAUSE_MEDIA_ERROR_V23 = 4, /* At V4 media error causes HALT */ 
    NDMP_MOVER_PAUSED_EOW_V34        = 5  /* For backup only - new at V3 */
}; 

enum ndmp_mover_halt_reason 
{ 
    NDMP_MOVER_HALT_NA             = 0, 
    NDMP_MOVER_HALT_CONNECT_CLOSED = 1, 
    NDMP_MOVER_HALT_ABORTED        = 2, 
    NDMP_MOVER_HALT_INTERNAL_ERROR = 3, 
    NDMP_MOVER_HALT_CONNECT_ERROR  = 4, 
    NDMP_MOVER_HALT_MEDIA_ERROR_V4 = 5  /* Was pause reason pre-V4 */ 
}; 

/* mover address */
enum ndmp_mover_mode  
{ 
    NDMP_MOVER_MODE_READ   = 0, /* read from data connection; write to tape */  
    NDMP_MOVER_MODE_WRITE  = 1, /* write to data connection; read from tape */ 
    NDMP_MOVER_MODE_NOACTION_V4 = 2  
};  

struct ndmp_mover_get_state_reply_v2
{
    ndmp_error              error;
    ndmp_mover_state        state;
    ndmp_mover_pause_reason pause_reason;
    ndmp_mover_halt_reason  halt_reason;
    cbackup_u32                  record_size;
    cbackup_u32                  record_num;
    ndmp_u_quad             data_written;
    ndmp_u_quad             seek_position;
    ndmp_u_quad             bytes_left_to_read;
    ndmp_u_quad             window_offset;
    ndmp_u_quad             window_length;
};

struct ndmp_mover_get_state_reply_v3
{
    ndmp_error              error;
    ndmp_mover_state        state;
    ndmp_mover_pause_reason pause_reason;
    ndmp_mover_halt_reason  halt_reason;
    cbackup_u32                  record_size;
    cbackup_u32                  record_num;
    ndmp_u_quad             data_written;
    ndmp_u_quad             seek_position;
    ndmp_u_quad             bytes_left_to_read;
    ndmp_u_quad             window_offset;
    ndmp_u_quad             window_length;
    ndmp_addr_v3            data_connection_addr; /* New at V3 */
};

struct ndmp_mover_get_state_reply_v4  
{  
    ndmp_error               error;  
    ndmp_mover_mode          mode; /* New at V4 */  
    ndmp_mover_state         state;  
    ndmp_mover_pause_reason  pause_reason;  
    ndmp_mover_halt_reason   halt_reason;  
    cbackup_u32                   record_size;  
    cbackup_u32                   record_num;  
    ndmp_u_quad              bytes_moved;  
    ndmp_u_quad              seek_position;  
    ndmp_u_quad              bytes_left_to_read;  
    ndmp_u_quad              window_offset;  
    ndmp_u_quad              window_length;  
    ndmp_addr_v4             data_connection_addr;  
};  

/* MOVER_LISTEN */
struct ndmp_mover_listen_request  
{ 
    ndmp_mover_mode         mode; 
    ndmp_addr_type          addr_type; 
}; 

struct ndmp_mover_listen_reply_v2
{
    ndmp_error          error;
    ndmp_mover_addr_v2  mover;
};

struct ndmp_mover_listen_reply_v3
{
    ndmp_error      error;
    ndmp_addr_v3    data_connection_addr;
};

struct ndmp_mover_listen_reply_v4  
{ 
    ndmp_error      error; 
    ndmp_addr_v4    connect_addr; 
}; 

/* MOVER_CONTINUE */
/* no request arguments */
struct ndmp_mover_continue_reply  
{ 
    ndmp_error            error; 
}; 

/* NDMP_MOVER_ABORT */
/* no request arguments */
struct ndmp_mover_abort_reply  
{ 
    ndmp_error            error; 
}; 

/* MOVER_STOP */
/* no request arguments */
struct ndmp_mover_stop_reply  
{ 
    ndmp_error            error; 
}; 

/* NDMP_MOVER_SET_WINDOW */
struct ndmp_mover_set_window_request  
{ 
    ndmp_u_quad            offset; 
    ndmp_u_quad            length; 
}; 

struct ndmp_mover_set_window_reply  
{ 
    ndmp_error             error; 
}; 

/* NDMP_MOVER_READ */
struct ndmp_mover_read_request  
{ 
    ndmp_u_quad            offset; 
    ndmp_u_quad            length; 
}; 

struct ndmp_mover_read_reply  
{ 
    ndmp_error            error; 
}; 

/* NDMP_MOVER_CLOSE */
/* no request arguments */
struct ndmp_mover_close_reply  
{ 
    ndmp_error            error; 
}; 

/* NDMP_MOVER_SET_RECORD_SIZE */
struct ndmp_mover_set_record_size_request  
{ 
    cbackup_u32         len;             
}; 

struct ndmp_mover_set_record_size_reply  
{ 
    ndmp_error     error; 
}; 

/* NDMP_MOVER_CONNECT - new at V3 */
struct ndmp_mover_connect_request_v3  
{ 
    ndmp_mover_mode       mode; 
    ndmp_addr_v3          addr; 
}; 

struct ndmp_mover_connect_request_v4  
{ 
    ndmp_mover_mode       mode; 
    ndmp_addr_v4          addr; 
}; 

struct ndmp_mover_connect_reply_v34  
{ 
    ndmp_error            error; 
}; 


/********************************/
/* NOTIFY INTERFACE             */
/********************************/

/*
** CHANGE SUMMARY:
**
** At V2/V3 these messages were called requests, at V4 they are renamed posts
** to distinguish them from other requests. Notify messages are sent from
** server to client and have no associated reply messages.
**
** At V4 the halted notifications have their text reason messages removed in
** favour of sending separate NDMP_LOG_MESSAGE posts.
**
** Note mover media error is a HALT condition in V4 but was a PAUSE condition
** in V2/V3. 
*/

/* NDMP_NOTIFY_DATA_HALTED */
struct ndmp_notify_data_halted_request_v23
{
    ndmp_data_halt_reason   reason;
    cbackup_string                  text_reason; /* Omitted in V4 */
};

struct ndmp_notify_data_halted_post_v4 
{ 
    ndmp_data_halt_reason   reason; 
}; 

/* NDMP_NOTIFY_CONNECTION_STATUS / NDMP_NOTIFY_CONNECTED */
enum ndmp_connection_status_reason /* Was called ndmp_connect_reason pre-v4 */ 
{ 
    NDMP_CONNECTED  = 0, 
    NDMP_SHUTDOWN   = 1, 
    NDMP_REFUSED    = 2  
}; 

struct ndmp_notify_connection_status_post /* ndmp_notify_connected_request */
{ 
    ndmp_connection_status_reason       reason; 
    cbackup_u16                             protocol_version; 
    cbackup_string                              text_reason; 
}; 

/* NDMP_NOTIFY_MOVER_HALTED */
struct ndmp_notify_mover_halted_request_v23
{
    ndmp_mover_halt_reason  reason;
    cbackup_string                  text_reason; /* Omitted in V4 */
};

struct ndmp_notify_mover_halted_post_v4 
{ 
    ndmp_mover_halt_reason      reason; 
}; 

/* NDMP_NOTIFY_MOVER_PAUSED */
struct ndmp_notify_mover_paused_post /* ndmp_notify_mover_paused_request */ 
{ 
    ndmp_mover_pause_reason reason; 
    ndmp_u_quad             seek_position; 
}; 

/* NDMP_NOTIFY_DATA_READ */
struct ndmp_notify_data_read_post /* ndmp_notify_data_read_request */ 
{ 
    ndmp_u_quad  offset; 
    ndmp_u_quad  length; 
}; 

/********************************/
/* LOG INTERFACE                */
/********************************/

/*
** CHANGE SUMMARY:
**
** NDMP_LOG_LOG and NDMP_LOG_DEBUG are V2 messages replaced by the 
** NDMP_LOG_MESSAGE interface at V3/V4.
**
** In V4 NDMP_LOG_MESSAGE can specify the sequence number of an
** associated reply or post message to which it refers. This was 
** introduced in order to retain context information when the
** text reason messages were removed from notify halted and were
** replaced by LOG_MESSAGE.
**
** At V2/V3 these messages were called requests, at V4 they are renamed posts
** to distinguish them from other requests. Log messages are sent from
** server to client and have no associated reply messages.
*/

/* NDMP_LOG_LOG_V2 */
struct ndmp_log_log_request_v2
{
    cbackup_string  entry;
};

/* NDMP_LOG_DEBUG_V2 */
enum ndmp_debug_level_v2
{
    NDMP_DBG_USER_INFO_V2,
    NDMP_DBG_USER_SUMMARY_V2,
    NDMP_DBG_USER_DETAIL_V2,
    NDMP_DBG_DIAG_INFO_V2,
    NDMP_DBG_DIAG_SUMMARY_V2,
    NDMP_DBG_DIAG_DETAIL_V2,
    NDMP_DBG_PROG_INFO_V2,
    NDMP_DBG_PROG_SUMMARY_V2,
    NDMP_DBG_PROG_DETAIL_V2
};

struct ndmp_log_debug_request_v2
{
    ndmp_debug_level_v2 level;
    cbackup_string              message;
};

/* NDMP_LOG_MESSAGE (At V3 replaced LOG_LOG/LOG_DEBUG) */

enum ndmp_log_type_v34 
{ 
    NDMP_LOG_NORMAL_V34  = 0, 
    NDMP_LOG_DEBUG_V34   = 1, 
    NDMP_LOG_ERROR_V34   = 2, 
    NDMP_LOG_WARNING_V34 = 3 
}; 

struct ndmp_log_message_request_v3
{
    ndmp_log_type_v34 log_type;
    cbackup_u32            message_id;
    cbackup_string            entry;
};

enum ndmp_has_associated_message_v4 
{ 
    NDMP_NO_ASSOCIATED_MESSAGE_V4  = 0, 
    NDMP_HAS_ASSOCIATED_MESSAGE_V4 = 1 
}; 

struct ndmp_log_message_post_v4 
{ 
    ndmp_log_type_v34              log_type; 
    cbackup_u32                         message_id; 
    cbackup_string                         entry; 
    ndmp_has_associated_message_v4 associated_message_valid; 
    cbackup_u32                         associated_message_sequence; 
}; 

/* NDMP_LOG_FILE */
struct ndmp_log_file_request_v2
{
    cbackup_string      name;
    cbackup_u16     ssid;   /* Never used? - removed at V3 */
    ndmp_error  error;
};

struct ndmp_log_file_request_v3
{
    cbackup_string      name;
    ndmp_error  error;
};

enum ndmp_recovery_status_v4 
{ 
    NDMP_RECOVERY_SUCCESSFUL_V4              = 0,    
    NDMP_RECOVERY_FAILED_PERMISSION_V4       = 1, 
    NDMP_RECOVERY_FAILED_NOT_FOUND_V4        = 2, 
    NDMP_RECOVERY_FAILED_NO_DIRECTORY_V4     = 3, 
    NDMP_RECOVERY_FAILED_OUT_OF_MEMORY_V4    = 4, 
    NDMP_RECOVERY_FAILED_IO_ERROR_V4         = 5, 
    NDMP_RECOVERY_FAILED_UNDEFINED_ERROR_V4  = 6, 
    NDMP_RECOVERY_FAILED_FILE_PATH_EXISTS_V4 = 7 
}; 

struct ndmp_log_file_post_v4 
{ 
    cbackup_string                   name; 
    ndmp_recovery_status_v4  recovery_status; 
}; 

/********************************/
/* File History INTERFACE	    */
/********************************/

/*
** CHANGE SUMMARY:
**
** At V2 these messages assumed a UNIX-like FS. The messages were replaced at
** V3 with the intention of being able to report NT-like or other types of
** file details. However, I'm not sure this was completely successful,
** particularly in the area of "owner/group". Probably need to look at this
** again at V5.
**
** At V2/V3 these messages were called requests, at V4 they are renamed posts
** to distinguish them from other requests. File History messages are sent from
** server to client and have no associated reply messages.
**
** Note the acceptable sequences of dir/node file history are defined in the
** V4 spec. In pre-V4 products the rules are not well defined.
*/

/* Common file type enumeration */
enum ndmp_file_type /* was ndmp_unix_file_type at V2 */
{ 
    NDMP_FILE_DIR          = 0, 
    NDMP_FILE_FIFO         = 1, 
    NDMP_FILE_CSPEC        = 2, 
    NDMP_FILE_BSPEC        = 3, 
    NDMP_FILE_REG          = 4, 
    NDMP_FILE_SLINK        = 5, 
    NDMP_FILE_SOCK         = 6, 
    NDMP_FILE_REGISTRY_V34 = 7, 
    NDMP_FILE_OTHER_V34    = 8 
}; 

/* NDMP Version 2 messages */

/* NDMP_FH_ADD_UNIX_V2 */
typedef cbackup_string ndmp_unix_path;

struct ndmp_unix_file_stat_v2
{
	ndmp_file_type	ftype;
	cbackup_u32			mtime;
	cbackup_u32			atime;
	cbackup_u32			ctime;
	cbackup_u32			uid;
	cbackup_u32			gid;
	cbackup_u32			mode;
	ndmp_u_quad		size;
	ndmp_u_quad		fh_info;
};

struct ndmp_fh_unix_path_v2
{
	ndmp_unix_path		    name;
	ndmp_unix_file_stat_v2	fstat;
};

struct ndmp_fh_add_unix_path_request_v2
{
	ndmp_fh_unix_path_v2	paths<>;
};

/* NDMP_FH_ADD_UNIX_DIR */
struct ndmp_fh_unix_dir_v2
{
	ndmp_unix_path	name;
	cbackup_u32			node;
	cbackup_u32			parent;
};

struct ndmp_fh_add_unix_dir_request_v2
{
	ndmp_fh_unix_dir_v2	dirs<>;
};

struct ndmp_fh_unix_node_v2
{
	ndmp_unix_file_stat_v2	fstat;
	cbackup_u32				    node;
};

struct ndmp_fh_add_unix_node_request_v2
{
	ndmp_fh_unix_node_v2	nodes<>;
};

/* NDMP_FH_ADD_FILE */
enum ndmp_fs_type_v34 
{ 
    NDMP_FS_UNIX   = 0, 
    NDMP_FS_NT     = 1, 
    NDMP_FS_OTHER  = 2 
}; 

typedef cbackup_string ndmp_path; 

struct ndmp_nt_path_v34 
{ 
    ndmp_path      nt_path; 
    ndmp_path      dos_path; 
}; 

union ndmp_file_name_v34 switch (ndmp_fs_type_v34 fs_type) 
{ 
    case NDMP_FS_UNIX: 
        ndmp_path          unix_name; 
    case NDMP_FS_NT: 
        ndmp_nt_path_v34   nt_name; 
    default: 
        ndmp_path          other_name; 
}; 

/* These were .._INVALID rather than .._UNS at V3 */
const NDMP_FILE_STAT_ATIME_UNS_V34 = 0x00000001; 
const NDMP_FILE_STAT_CTIME_UNS_V34 = 0x00000002; 
const NDMP_FILE_STAT_GROUP_UNS_V34 = 0x00000004; 

struct ndmp_file_stat_v34 
{ 
    cbackup_u32            unsupported; /* Was called "invalid" at V3 */  
    ndmp_fs_type_v34  fs_type; 
    ndmp_file_type    ftype; 
    cbackup_u32            mtime; 
    cbackup_u32            atime; 
    cbackup_u32            ctime; 
    cbackup_u32            owner; /* uid for UNIX, ? NA for NT */ 
    cbackup_u32            group; /* gid for UNIX, NA for NT */ 
    cbackup_u32            fattr; /* mode for UNIX, fattr for NT */
    ndmp_u_quad       size; 
    cbackup_u32            links; 
}; 


/*
** one file could have both UNIX and NT name and attributes
** - explanation for the arrays of name/stat - however, I don't
** know if anyone uses this feature
*/
struct ndmp_file_v34 
{ 
    ndmp_file_name_v34  name<>; 
    ndmp_file_stat_v34  stat<>; 
    ndmp_u_quad         node; 
    ndmp_u_quad         fh_info; 
}; 

struct ndmp_fh_add_file_post_v34 /* ndmp_fh_add_file_request */ 
{ 
    ndmp_file_v34       files<>; 
}; 


/* NDMP_FH_ADD_DIR */
struct ndmp_dir_v34 
{ 
    ndmp_file_name_v34    name<>; 
    ndmp_u_quad           node; 
    ndmp_u_quad           parent; 
}; 

struct ndmp_fh_add_dir_post_v34 /* ndmp_fh_add_dir_request */ 
{ 
    ndmp_dir_v34 dirs<>; 
}; 

/* NDMP_FH_ADD_NODE */
struct ndmp_node_v34 
{ 
    ndmp_file_stat_v34  stats<>; 
    ndmp_u_quad         node; 
    ndmp_u_quad         fh_info; 
}; 

struct ndmp_fh_add_node_post_v34 /* ndmp_fh_add_node_request */ 
{ 
    ndmp_node_v34       nodes<>; 
}; 
