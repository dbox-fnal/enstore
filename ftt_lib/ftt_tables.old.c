static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <ftt_private.h>

int ftt_trans_in[] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EIO,
	/*    6	ENXIO	*/	FTT_ENXIO,
	/*    7	EBLKSIZE*/	FTT_EBLKSIZE,
	/*    8	ENOEXEC	*/	FTT_ENOEXEC,
	/*    9	EBADF	*/	FTT_ENOENT,
	/*   10	ECHILD	*/	FTT_ENOTSUPPORTED,
	/*   11	EAGAIN	*/	FTT_ENOTAPE,
	/*   12	ENOMEM	*/	FTT_ENOMEM,
	/*   13	EACCES	*/	FTT_EPERM,
	/*   14	EFAULT	*/	FTT_EFAULT,
	/*   15	ENOTBLK	*/	FTT_ENOTSUPPORTED,
	/*   16	EBUSY	*/	FTT_EBUSY,
	/*   17	EEXIST	*/	FTT_ENOTSUPPORTED,
	/*   18	EXDEV	*/	FTT_ENOTSUPPORTED,
	/*   19	ENODEV	*/	FTT_ENOENT,
	/*   20	ENOTDIR	*/	FTT_ENOENT,
	/*   21	EISDIR	*/	FTT_ENOENT,
	/*   22	EINVAL	*/	FTT_EBLKSIZE,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EBLKSIZE,
	/*   28	ENOSPC	*/	FTT_EBLANK,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32	EPIPE	*/	FTT_ENOTSUPPORTED,
	/*   33	EDOM	*/	FTT_ENOTSUPPORTED,
	/*   34	ERANGE	*/	FTT_ENOTSUPPORTED,
	/*   35	ENOMSG	*/	FTT_ENOTSUPPORTED,
	/*   36	EIDRM	*/	FTT_ENOTSUPPORTED,
	/*   37	ECHRNG	*/	FTT_ENOTSUPPORTED,
};

int ftt_trans_skip[] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EBLANK,
	/*    6	ENXIO	*/	FTT_ENXIO,
	/*    7	EBLKSIZE	*/	FTT_EBLKSIZE,
	/*    8	ENOEXEC	*/	FTT_ENOEXEC,
	/*    9	EBADF	*/	FTT_ENOENT,
	/*   10	ECHILD	*/	FTT_ENOTSUPPORTED,
	/*   11	EAGAIN	*/	FTT_ENOTAPE,
	/*   12	ENOMEM	*/	FTT_ENOMEM,
	/*   13	EACCES	*/	FTT_EPERM,
	/*   14	EFAULT	*/	FTT_EFAULT,
	/*   15	ENOTBLK	*/	FTT_ENOTSUPPORTED,
	/*   16	EBUSY	*/	FTT_EBUSY,
	/*   17	EEXIST	*/	FTT_ENOTSUPPORTED,
	/*   18	EXDEV	*/	FTT_ENOTSUPPORTED,
	/*   19	ENODEV	*/	FTT_ENOENT,
	/*   20	ENOTDIR	*/	FTT_ENOENT,
	/*   21	EISDIR	*/	FTT_ENOENT,
	/*   22	EINVAL	*/	FTT_EBLKSIZE,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EBLANK,
	/*   28	ENOSPC	*/	FTT_EBLANK,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32	EPIPE	*/	FTT_ENOTSUPPORTED,
	/*   33	EDOM	*/	FTT_ENOTSUPPORTED,
	/*   34	ERANGE	*/	FTT_ENOTSUPPORTED,
	/*   35	ENOMSG	*/	FTT_ENOTSUPPORTED,
	/*   36	EIDRM	*/	FTT_ENOTSUPPORTED,
	/*   37	ECHRNG	*/	FTT_ENOTSUPPORTED,
};

int ftt_trans_out[] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EROFS,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EIO,
	/*    6	ENXIO	*/	FTT_ENXIO,
	/*    7	EBLKSIZE	*/	FTT_EBLKSIZE,
	/*    8	ENOEXEC	*/	FTT_ENOEXEC,
	/*    9	EBADF	*/	FTT_EROFS,
	/*   10	ECHILD	*/	FTT_ENOTSUPPORTED,
	/*   11	EAGAIN	*/	FTT_ENOTAPE,
	/*   12	ENOMEM	*/	FTT_ENOMEM,
	/*   13	EACCES	*/	FTT_EPERM,
	/*   14	EFAULT	*/	FTT_EFAULT,
	/*   15	ENOTBLK	*/	FTT_ENOTSUPPORTED,
	/*   16	EBUSY	*/	FTT_EBUSY,
	/*   17	EEXIST	*/	FTT_ENOTSUPPORTED,
	/*   18	EXDEV	*/	FTT_ENOTSUPPORTED,
	/*   19	ENODEV	*/	FTT_ENOENT,
	/*   20	ENOTDIR	*/	FTT_ENOENT,
	/*   21	EISDIR	*/	FTT_ENOENT,
	/*   22	EINVAL	*/	FTT_EBLKSIZE,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EBLKSIZE,
	/*   28	ENOSPC	*/	FTT_EBLKSIZE,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32	EPIPE	*/	FTT_ENOTSUPPORTED,
	/*   33	EDOM	*/	FTT_ENOTSUPPORTED,
	/*   34	ERANGE	*/	FTT_ENOTSUPPORTED,
	/*   35	ENOMSG	*/	FTT_ENOTSUPPORTED,
	/*   36	EIDRM	*/	FTT_ENOTSUPPORTED,
	/*   37	ECHRNG	*/	FTT_ENOTSUPPORTED,
};

int *ftt_trans_table[] = {
    /* none...but just to be safe0 */ ftt_trans_in,
    /* FTT_OPN_READ		 1 */ ftt_trans_in,
    /* FTT_OPN_WRITE		 2 */ ftt_trans_out,
    /* FTT_OPN_WRITEFM		 3 */ ftt_trans_out,
    /* FTT_OPN_SKIPREC		 4 */ ftt_trans_skip,
    /* FTT_OPN_SKIPFM		 5 */ ftt_trans_skip,
    /* FTT_OPN_REWIND		 6 */ ftt_trans_skip,
    /* FTT_OPN_UNLOAD		 7 */ ftt_trans_skip,
    /* FTT_OPN_RETENSION	 8 */ ftt_trans_skip,
    /* FTT_OPN_ERASE		 9 */ ftt_trans_out,
    /* FTT_OPN_STATUS		10 */ ftt_trans_in,
    /* FTT_OPN_GET_STATUS	11 */ ftt_trans_in,
    /* FTT_OPN_ASYNC 		12 */ ftt_trans_in,
    /* FTT_OPN_PASSTHRU         13 */ ftt_trans_out,
    /* FTT_OPN_CHALL            14 */ ftt_trans_out,
    /* FTT_OPN_OPEN             15 */ ftt_trans_out,
};

ftt_dev_entry devtable[] = {
    {"AIX", "EXB-8505", FTT_FLAG_HOLD_SIGNALS, FTT_OP_UNLOAD|FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table,{
    /*   string                  den mod hwd pas fxd rewind            sfx */
    /*   ======                  === === === === === ======            === */
        { ".1",                   0,  0,0x15, 0,  0,                 0, 1,1},
        { ".1",                   0,  0,0x00, 0,  0,                 0, 1,1},
        { ".1",                  -2,  0,   0, 1,  0, FTT_RWOC|       0, 1,0},
        { ".0",                   0,  0,0x15, 0,  0, FTT_RWOC|       0, 1,1},
        { ".1",                   0,  0,0x15, 0,  0,                 0, 1,0},
        { ".2",                   0,  0,0x15, 0,  0, FTT_RWOC|FTT_RTOO, 1,1},
        { ".3",                   0,  0,0x15, 0,  0,        0|FTT_RTOO, 1,0},
        { ".0",                   0,  1,0x8c, 0,  0, FTT_RWOC|       0, 1,0},
        { ".1",                   0,  1,0x8c, 0,  0,                 0, 1,0},
        { ".2",                   0,  1,0x8c, 0,  0, FTT_RWOC|FTT_RTOO, 1,0},
        { ".3",                   0,  1,0x8c, 0,  0,        0|FTT_RTOO, 1,0},
        { ".4",                   1,  0,0x14, 0,  0, FTT_RWOC|       0, 1,1},
        { ".5",                   1,  0,0x14, 0,  0,                 0, 1,1},
        { ".6",                   1,  0,0x14, 0,  0, FTT_RWOC|FTT_RTOO, 1,1},
        { ".7",                   1,  0,0x14, 0,  0,        0|FTT_RTOO, 1,1},
        { ".4",                   1,  1,0x90, 0,  0, FTT_RWOC|       0, 1,0},
        { ".5",                   1,  1,0x90, 0,  0,                 0, 1,0},
        { ".6",                   1,  1,0x90, 0,  0, FTT_RWOC|FTT_RTOO, 1,0},
        { ".7",                   1,  1,0x90, 0,  0,        0|FTT_RTOO, 1,0},
        { ".0",                   0,  0,0x15, 0,  1, FTT_RWOC|       0, 1,0},
        { ".1",                   0,  0,0x15, 0,  1,                 0, 1,0},
        { ".2",                   0,  0,0x15, 0,  1, FTT_RWOC|FTT_RTOO, 1,0},
        { ".3",                   0,  0,0x15, 0,  1,        0|FTT_RTOO, 1,0},
        { ".0",                   0,  1,0x8c, 0,  1, FTT_RWOC|       0, 1,0},
        { ".1",                   0,  1,0x8c, 0,  1,                 0, 1,0},
        { ".2",                   0,  1,0x8c, 0,  1, FTT_RWOC|FTT_RTOO, 1,0},
        { ".3",                   0,  1,0x8c, 0,  1,        0|FTT_RTOO, 1,0},
        { ".4",                   1,  0,0x14, 0,  1, FTT_RWOC|       0, 1,0},
        { ".5",                   1,  0,0x14, 0,  1,                 0, 1,0},
        { ".6",                   1,  0,0x14, 0,  1, FTT_RWOC|FTT_RTOO, 1,0},
        { ".7",                   1,  0,0x14, 0,  1,        0|FTT_RTOO, 1,0},
        { ".4",                   1,  1,0x90, 0,  1, FTT_RWOC|       0, 1,0},
        { ".5",                   1,  1,0x90, 0,  1,                 0, 1,0},
        { ".6",                   1,  1,0x90, 0,  1, FTT_RWOC|FTT_RTOO, 1,0},
        { ".7",                   1,  1,0x90, 0,  1,        0|FTT_RTOO, 1,0},
        { 0, },
    }},
    {"AIX", "EXB-8500", FTT_FLAG_HOLD_SIGNALS, FTT_OP_UNLOAD|FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table,{
    /*   string                  den mod hwd pas fxd rewind            sf,1st */
    /*   ======                  === === === === === ======            === */
        { ".1",                   0,  0,0x15, 0,  0,                 0, 1,1},
        { ".1",                   0,  0,0x00, 0,  0,                 0, 1,1},
        { ".1",                  -2,  0,   0, 1,  0, FTT_RWOC|       0, 1,0},
        { ".0",                   0,  0,0x15, 0,  0, FTT_RWOC|       0, 1,1},
        { ".1",                   0,  0,0x15, 0,  0,                 0, 1,0},
        { ".2",                   0,  0,0x15, 0,  0, FTT_RWOC|FTT_RTOO, 1,1},
        { ".3",                   0,  0,0x15, 0,  0,        0|FTT_RTOO, 1,1},
        { ".4",                   1,  0,0x14, 0,  0, FTT_RWOC|       0, 1,1},
        { ".5",                   1,  0,0x14, 0,  0,                 0, 1,1},
        { ".6",                   1,  0,0x14, 0,  0, FTT_RWOC|FTT_RTOO, 1,1},
        { ".7",                   1,  0,0x14, 0,  0,        0|FTT_RTOO, 1,1},
        { ".0",                   0,  0,0x15, 0,  1, FTT_RWOC|       0, 1,0},
        { ".1",                   0,  0,0x15, 0,  1,                 0, 1,0},
        { ".2",                   0,  0,0x15, 0,  1, FTT_RWOC|FTT_RTOO, 1,0},
        { ".3",                   0,  0,0x15, 0,  1,        0|FTT_RTOO, 1,0},
        { ".4",                   1,  0,0x14, 0,  1, FTT_RWOC|       0, 1,0},
        { ".5",                   1,  0,0x14, 0,  1,                 0, 1,0},
        { ".6",                   1,  0,0x14, 0,  1, FTT_RWOC|FTT_RTOO, 1,0},
        { ".7",                   1,  0,0x14, 0,  1,        0|FTT_RTOO, 1,0},
        { 0, },
    }},
    {"AIX", "EXB-8200", FTT_FLAG_HOLD_SIGNALS, FTT_OP_UNLOAD|FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table,{
    /*   string                  den mod hwd pas fxd rewind            sf,1st */
    /*   ======                  === === === === === ======            === */
        { ".1",                   0,  0,   0, 0,  0,                 0, 1,1},
        { ".1",                  -2,  0,   0, 1,  0, FTT_RWOC|       0, 1,0},
        { ".0",                   0,  0,   0, 0,  0, FTT_RWOC|       0, 1,1},
        { ".1",                   0,  0,   0, 0,  0,                 0, 1,0},
        { ".2",                   0,  0,   0, 0,  0, FTT_RWOC|FTT_RTOO, 1,1},
        { ".3",                   0,  0,   0, 0,  0,        0|FTT_RTOO, 1,1},
        { ".0",                   0,  0,   0, 0,  1, FTT_RWOC|       0, 1,0},
        { ".1",                   0,  0,   0, 0,  1,                 0, 1,0},
        { ".2",                   0,  0,   0, 0,  1, FTT_RWOC|FTT_RTOO, 1,0},
        { ".3",                   0,  0,   0, 0,  1,        0|FTT_RTOO, 1,0},
        { 0, },
    }},
    {"IRIX+5", "DLT", FTT_FLAG_ASYNC_REWIND, FTT_OP_GET_STATUS, ftt_trans_table,{
    /*   string                  den mod hwd pas fxd rewind            sf,1st */
    /*   ======                  === === === === === ======            === */
	{ "nrv",                  4,  0,0x1A, 0,  0,                 0, 1,1},
	{ "/dev/scsi/sc%dd%dl0", -2,  0,   0, 1,  0,                 0, 0,1},
	{ "",                     4,  0,0x1A, 0,  1,          FTT_RWOC, 1,1},
	{ "nr",                   4,  0,0x1A, 0,  1,                 0, 1,1},
	{ "v",                    4,  0,0x1A, 0,  0,          FTT_RWOC, 1,1},
	{ "nrv",                  4,  0,0x1A, 0,  0,                 0, 1,1},
        { 0,},
    }},
    {"IRIX+5", "EXB-85", FTT_FLAG_ASYNC_REWIND, FTT_OP_GET_STATUS, ftt_trans_table,{
    /*   string                  den mod hwd pas fxd rewind            sf,1st */
    /*   ======                  === === === === === ======            === */
	{ "nrv.8500",             1,  0,0x15, 0,  0,                 0, 1,1},
	{ "nrv.8500",             1,  0,0x00, 0,  0,                 0, 1,1},
	{ "/dev/scsi/sc%dd%dl0", -2,  0,   0, 1,  0,                 0, 0,1},
	{ "",                     1,  0,0x15, 0,  1,          FTT_RWOC, 1,1},
	{ "nr",                   1,  0,0x15, 0,  0,                 0, 1,1},
	{ "v",                    1,  0,0x15, 0,  0,          FTT_RWOC, 1,1},
	{ "nrv",                  1,  0,0x15, 0,  0,                 0, 1,1},
	{ ".8200",                0,  0,0x14, 0,  1,          FTT_RWOC, 1,1},
	{ "nr.8200",              0,  0,0x14, 0,  1,                 0, 1,1},
	{ "v.8200",               0,  0,0x14, 0,  0,          FTT_RWOC, 1,1},
	{ "nrv.8200",             0,  0,0x14, 0,  0,                 0, 1,1},
	{ ".8500",                1,  0,0x15, 0,  1,          FTT_RWOC, 1,1},
	{ "nr.8500",              1,  0,0x15, 0,  1,                 0, 1,1},
	{ "nrv.8500",             1,  0,0x15, 0,  0,                 0, 1,0},
	{ "v.8500",               1,  0,0x15, 0,  0,          FTT_RWOC, 1,1},
        { 0,},
    }},
    {"IRIX+5", "EXB-82", FTT_FLAG_ASYNC_REWIND, FTT_OP_GET_STATUS, ftt_trans_table,{
    /*   string                  den mod hwd pas fxd rewind            sf,1st */
    /*   ======                  === === === === === ======            === */
	{ "nrv",                  0,  0,   0, 0,  0,                 0, 1,1},
	{ "/dev/scsi/sc%dd%dl0", -2,  0,   0, 1,  0,                 0, 0,1},
	{ "",                     0,  0,   0, 0,  1,          FTT_RWOC, 1,1},
	{ "nr",                   0,  0,   0, 0,  1,                 0, 1,1},
	{ "v",                    0,  0,   0, 0,  0,          FTT_RWOC, 1,1},
	{ "nrv",                  0,  0,   0, 0,  0,                 0, 1,0},
        { 0,},
    }},
    {"IRIX+4", "EXB-85", FTT_FLAG_ASYNC_REWIND, FTT_OP_GET_STATUS, ftt_trans_table,{
    /*   string                  den mod hwd pas fxd rewind            sf,1st */
    /*   ======                  === === === === === ======            === */
	{ "nrnsv.8500",           1,  0,0x15, 0,  0,                 0, 1,1},
	{ "/dev/scsi/sc%dd%dl0", -2,  0,   0, 1,  1,                 0, 0,1},
	{ "",                     1,  0,0x15, 0,  1, FTT_BTSW|FTT_RWOC, 1,1},
	{ "ns",                   1,  0,0x15, 0,  1,          FTT_RWOC, 1,1},
	{ "nrns",                 1,  0,0x15, 0,  0,                 0, 1,1},
	{ "v",                    1,  0,0x15, 0,  0, FTT_BTSW|FTT_RWOC, 1,1},
	{ "nsv",                  1,  0,0x15, 0,  0,          FTT_RWOC, 1,1},
	{ "nrnsv",                1,  0,0x15, 0,  0,                 0, 1,1},
	{ ".8200",                0,  0,0x14, 0,  1, FTT_BTSW|FTT_RWOC, 1,1},
	{ "ns.8200",              0,  0,0x14, 0,  1,          FTT_RWOC, 1,1},
	{ "nrns.8200",            0,  0,0x14, 0,  1,                 0, 1,1},
	{ "v.8200",               0,  0,0x14, 0,  0, FTT_BTSW|FTT_RWOC, 1,1},
	{ "nsv.8200",             0,  0,0x14, 0,  0,          FTT_RWOC, 1,1},
	{ "nrnsv.8200",           0,  0,0x14, 0,  0,                 0, 1,1},
	{ ".8500",                1,  0,0x15, 0,  1, FTT_BTSW|FTT_RWOC, 1,1},
	{ "ns.8500",              1,  0,0x15, 0,  1,          FTT_RWOC, 1,1},
	{ "nrns.8500",            1,  0,0x15, 0,  1,                 0, 1,1},
	{ "nrnsv.8500",           1,  0,0x15, 0,  0,                 0, 1,0},
	{ "v.8500",               1,  0,0x15, 0,  0, FTT_BTSW|FTT_RWOC, 1,1},
	{ "nsv.8500",             1,  0,0x15, 0,  0,          FTT_RWOC, 1,1},
        { 0,},
    }},
    {"IRIX+4", "EXB-82", FTT_FLAG_ASYNC_REWIND, FTT_OP_GET_STATUS, ftt_trans_table,{
    /*   string                  den mod hwd pas fxd rewind            sf,1st */
    /*   ======                  === === === === === ======            === */
	{ "nrnsv",                0,  0,   0, 0,  0,                 0, 1,1},
	{ "/dev/scsi/sc%dd%dl0", -2,  0,   0, 1,  1,                 0, 0,1},
	{ "",                     0,  0,   0, 0,  1, FTT_BTSW|FTT_RWOC, 1,1},
	{ "ns",                   0,  0,   0, 0,  1,          FTT_RWOC, 1,1},
	{ "nrns",                 0,  0,   0, 0,  0,                 0, 1,1},
	{ "v",                    0,  0,   0, 0,  0, FTT_BTSW|FTT_RWOC, 1,1},
	{ "nsv",                  0,  0,   0, 0,  0,          FTT_RWOC, 1,1},
	{ "nrnsv",                0,  0,   0, 0,  0,                 0, 1,0},
        { 0,},
    }},
    {0, },
};


ftt_stat_entry ftt_stat_op_tab[] = {
    {"EXB-8200", 
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS|FTT_DO_EXBRS},

    {"EXB-8500", 
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS|FTT_DO_EXBRS|
	FTT_DO_SN|FTT_DO_RP},

    {"EXB-8505", 
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS|FTT_DO_EXBRS|
	FTT_DO_05RS|FTT_DO_SN|FTT_DO_LSRW|FTT_DO_RP},

    {"DLT",      
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS|FTT_DO_DLTRS|
	FTT_DO_SN|FTT_DO_LSRW|FTT_DO_LSC|FTT_DO_RP},

    {0,0}
};
