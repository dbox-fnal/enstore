home/vsergeev/ftt/ftt_lib/ftt_async.c                                                               0100660 0022366 0022366 00000010442 06747415462 020503  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h> 
#include <ftt_private.h>
#include <string.h>
#include <sys/param.h>

#ifndef WIN32
#include <signal.h>
#include <unistd.h>
#endif

/*
** this process starts an asynchronous process to do ftt operations
** and sets up the pipe for ftt_report()ing status.
** If successful it returns the result of fork().
**
** this is sort of like popen(), except we don't exec anything else
** in the child.
*/
int
ftt_fork(ftt_descriptor d) {
    int fds[2];
    int res=0;
    int i;

    ENTERING("ftt_fork");			
    CKNULL("ftt_descriptor", d);

    ftt_close_dev(d);
    res = pipe(fds);
    DEBUG3(stderr, "pipe returns %d and %d\n", fds[0], fds[1]);
    if (0 == res) {
	switch (res = fork()) {

	case 0:    /* child, fork again so no SIGCLD, zombies, etc. */

#if 0
	  /*
          ** with this in here we can't keep a pipe open to
          ** write paritition info to kids...
          */

	  /* close all files except fds[1] */
	  for( i=3; i<NOFILE; i++ ) {
	    if ( i != fds[1] ) close(i);
	  }
#else
          close(fds[0]);
#endif
	  
	    if(fork() == 0){
		   /* grandchild, send our pid up the pipe */
	      
	        d->async_pf_parent = fdopen(fds[1],"w");
		fprintf(d->async_pf_parent,"%d\n", (int)getpid());
		fflush(d->async_pf_parent);
	    } else {
		exit(0);
	    }
	    break;

	default:     /* parent */
	    close(fds[1]);
	    waitpid(res,0,0);
	    d->async_pf_child = fdopen(fds[0],"r");
	    res = fscanf(d->async_pf_child, "%d", &d->async_pid);
	    if (res == 0) {
		DEBUG3(stderr, "retrying read of pid from pipe\n");
	        res = fscanf(d->async_pf_child, "\n%d", &d->async_pid);
	    }
	    DEBUG3(stderr,"got pid %d\n", d->async_pid);
	    break;

	case -1:
	    res = ftt_translate_error(d, FTT_OPN_ASYNC, "ftt_fork", res, "a fork() system call to\n\
 	create a process to perform asynchronous actions", 1);
	    break;
	}
    } else {
	res = ftt_translate_error(d, FTT_OPN_ASYNC, "ftt_fork", res, "a pipe() system call to\n\
	create a channel to return asynchronous results", 1);
    }
    return res;
}

int
ftt_check(ftt_descriptor d) {
    
    /*
    ** can't use CKOK 'cause it fails when theres an unwaited for task!
    */
    ENTERING("ftt_check");
    CKNULL("ftt_descriptor", d);

    DEBUG3(stderr,"looking for pid %d\n", d->async_pid);
    if (d->async_pid != 0 && 0 == kill(d->async_pid, 0)) {
	ftt_eprintf("ftt_check: background process still running\n");
	ftt_errno = FTT_EBUSY;
	return -1;
    } else {
       return 0;
    }
}

int
ftt_wait(ftt_descriptor d) {
    int len;

    /*
    ** can't use CKOK 'cause it fails when theres an unwaited for task!
    */
    ENTERING("ftt_wait");
    CKNULL("ftt_descriptor", d);

    DEBUG3(stderr,"async_pid is %d", d->async_pid );
    DEBUG3(stderr,"async_pf is %lx\n", (long)d->async_pf_child );
    ftt_eprintf("ftt_wait: unable to rendezvous with background process %d, ftt_errno FTT_ENXIO",
		d->async_pid);
    if (0 != d->async_pid ) {
	fscanf(d->async_pf_child, "\n%d\n", &ftt_errno);
	DEBUG3(stderr,"scanf of child pipe yeilds errno %d\n", ftt_errno);
	len = fread(ftt_eprint_buf, 1, FTT_EPRINT_BUF_SIZE - 1, d->async_pf_child);
	DEBUG3(stderr,"fread of child pipe returns %d\n", len);
	if ( len > 0 ) {
	    ftt_eprint_buf[len] = 0;
	}
	fclose(d->async_pf_child);
	d->async_pf_child = 0;
	d->async_pid = 0;
	if (ftt_errno != 0) {
	    return -1;
	} else {
	    return 0;
	}
    } else {
       ftt_eprintf("ftt_wait: there is no background process, ftt_errno FTT_ENXIO");
       ftt_errno = FTT_ENXIO;
       return -1;
    }
}

int
ftt_report(ftt_descriptor d) {
    int e; char *p;

    /*
    ** Can't use CKOK or ENTERING macro, 'cause it clears the errors we 
    ** want to report!
    */
    char *_name = "ftt_report";			
    DEBUG1(stderr,"Entering ftt_report");
    CKNULL("ftt_descriptor", d);

    if (d->async_pf_parent) {
	p = ftt_get_error(&e);
	p = strdup(p); /* don't lose messages! */
	ftt_close_dev(d);
	DEBUG3(stderr,"Writing ftt_errno %d  message %s to pipe\n", e, p);
	fprintf(d->async_pf_parent, "%d\n%s", e, p);
	fflush(d->async_pf_parent);
	exit(0);
    } else {
	ftt_eprintf("ftt_report: there is no connection to a parent process, ftt_errno FTT_ENXIO");
	ftt_errno = FTT_ENXIO;
	return -1;
    }
    return 0;
}
                                                                                                                                                                                                                              home/vsergeev/ftt/ftt_lib/ftt_checktable.c                                                          0100660 0022366 0022366 00000012540 06443574413 021447  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";

#include <stdio.h>
#include <ftt_private.h>
#include <string.h>

void ftt_check_table();

static int table_debug;
int
main() {
   
    table_debug = 0;
    ftt_check_table(stdout);
	return 0;
}

void ftt_check_table(FILE *pf) {
    ftt_descriptor d1, d2;
    ftt_dev_entry *dt;
    ftt_devinfo *pd;
    int i,j,k,l;
    int first_seen;
    extern int devtable_size;

    fprintf( pf, "Stage 0: whole table reachable check\n");

    for(ftt_first_supported(&i); (d1 = ftt_next_supported(&i)) ; ) {
	if (table_debug) fprintf(pf, 
	     "DEBUG: os '%s' basename '%s' prod_id '%s' controller '%s':\n",
		      d1->os, d1->basename, d1->prod_id, d1->controller);
    }
    if (i < (int)(devtable_size / sizeof(ftt_dev_entry) - 1)) {
	fprintf(pf, "Premature end of table scan, slot %d of %d/%d\n", 
		i, devtable_size, sizeof(ftt_dev_entry));
    }

    fprintf( pf, "Stage 1: 'first' flag checks\n");

    for(ftt_first_supported(&i); (d1 = ftt_next_supported(&i)) ; ) {
	if (table_debug) fprintf(pf, 
	     "DEBUG: os '%s' basename '%s' prod_id '%s' controller '%s':\n",
		      d1->os, d1->basename, d1->prod_id, d1->controller);
	pd = d1->devinfo;
	for( k = 0; 0 != pd[k].device_name; k++ ) {
	    first_seen = 0;
	    for( l = 0; l < k ; l++ ) {
		if (0 == strcmp(pd[k].device_name, pd[l].device_name) 
				&& 1 == pd[l].first ) {
		    if (first_seen) {
			/* already saw a first flag for this name */
			fprintf(pf, 
			      "entry for os '%s' basename '%s' prod_id '%s' controller '%s':\n\
			       extra 'first' flag in slot %d, name '%s'\n",
			       d1->os, d1->basename, d1->prod_id,
			       d1->controller, l, pd[l].device_name);
			/* change it for now, so we don't keep warning about it */
			pd[l].first = 0;
		    }
		    first_seen = 1;
		}
	    }
	    if (first_seen == 0 && ! d1->devinfo[k].first ) {
		fprintf(pf, 
		     "entry for os '%s' basename '%s' prod_id '%s' controller '%s':\n\
		      missing 'first' flag in slot %d, name '%s'\n",
		      d1->os, d1->basename, d1->prod_id,
		      d1->controller, k, pd[k].device_name);
		/* change it for now, so we don't keep warning about it */
		pd[k].first = 1;
	    }
	}
    }

    fprintf( pf, "Stage 2: obscured entry checks\n");

    for(ftt_first_supported(&i); (d1 = ftt_next_supported(&i) ); ) {
	if (table_debug) fprintf(pf, 
	     "DEBUG: os '%s' basename '%s' prod_id '%s' controller '%s':\n",
		      d1->os, d1->basename, d1->prod_id, d1->controller);
        for(ftt_first_supported(&j); (d2 = ftt_next_supported(&j)) && j < i; ) {
	    if (0 == strncmp(d1->os,d2->os,strlen(d1->os)) &&
	      0 == strncmp(d1->prod_id,d2->prod_id,strlen(d2->prod_id)) &&
	      0 == strcmp(d1->basename,d2->basename)) {
		fprintf(pf, 
		     "entry for os '%s' basename '%s' prod_id '%s' controller '%s':\n\
obscures entry for os '%s' basename '%s' prod_id '%s' controller '%s':\n",
		      d1->os, d1->basename, d1->prod_id, d1->controller,
		      d2->os, d2->basename, d2->prod_id, d1->controller);
	    }
	}
    }


    fprintf( pf, "Stage 3: format string, etc. checks\n");

    for(dt = devtable; dt->os != 0; dt++ ) {
	int slashes, prcnt, prcnts, prcntd;
	char *pc;

	if (table_debug) fprintf(pf, 
	     "DEBUG: os '%s' baseconv_out '%s' drivid '%s' controller '%s':\n",
		      dt->os, dt->baseconv_out, dt->drivid, dt->controller);

	if (dt->errortrans == 0 ) {
	     fprintf(pf, 
	         "missing errortrans: os '%s' baseconv_out '%s' drivid '%s' controller '%s':\n",
		      dt->os, dt->baseconv_out, dt->drivid, dt->controller);
	}
	if (dt->densitytrans == 0 ) {
	     fprintf(pf, 
	         "missing densitytrans: os '%s' baseconv_out '%s' drivid '%s' controller '%s':\n",
		      dt->os, dt->baseconv_out, dt->drivid, dt->controller);
	}

	pd = dt->devs;
	for( k = 0; 0 != pd[k].device_name; k++ ) {
	    if (table_debug) fprintf(pf, "devslot %d, name %s\n", k, pd[k].device_name);
	    slashes = 0;
	    prcnt = 0;
	    prcnts = 0;
	    prcntd = 0;
	    for (pc = pd[k].device_name; 0 != pc && 0 != *pc ; pc++ ) {
		if ('/' == pc[0] ) {
		    slashes++;
		}
		if ('%' == pc[0] ) {
		    prcnt++;
		    if ('%' == pc[1]) { /* "%%" doesnt count */
			 prcnt--;
		    }
		    if ('s' == pc[1]) {
			 prcnts++;
		    }
		    if ('d' == pc[1]) {
			 prcntd++;
		    }
		}
	    }
	    if (slashes > 1) {
		fprintf(pf, 
		     "entry for os '%s' baseconv_out '%s' drivid '%s' controller '%s':\n\
		      too many '/' characters in format slot %d, name '%s'\n",
		      dt->os, dt->baseconv_out, dt->drivid,
		      dt->controller, k, pd[k].device_name);
	    }
	    if (prcnt > 2) {
		fprintf(pf, 
		     "entry for os '%s' baseconv_out '%s' drivid '%s' controller '%s':\n\
		      too many '%%' characters in format slot %d, name '%s'\n",
		      dt->os, dt->baseconv_out, dt->drivid,
		      dt->controller, k, pd[k].device_name);
	    }
	    if (prcnts > 1) {
		fprintf(pf, 
		     "entry for os '%s' baseconv_out '%s' drivid '%s' controller '%s':\n\
		      too many '%%s' specs in format slot %d, name '%s'\n",
		      dt->os, dt->baseconv_out, dt->drivid,
		      dt->controller, k, pd[k].device_name);
	    }
	    if (prcntd > 2) {
		fprintf(pf, 
		     "entry for os '%s' baseconv_out '%s' drivid '%s' controller '%s':\n\
		      too many '%%d' specs in format slot %d, name '%s'\n",
		      dt->os, dt->baseconv_out, dt->drivid,
		      dt->controller, k, pd[k].device_name);
	    }
	}
    }
}
                                                                                                                                                                home/vsergeev/ftt/ftt_lib/ftt_debug.c                                                               0100640 0022366 0022366 00000002225 10037773175 020445  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";

#include <stdio.h>
#include <ctype.h>

int
ftt_dump(FILE *pf, unsigned char *pc, int n, int do_offsets, int do_chars) {
    int i, j;

    if( 0 == pc ){
        fprintf(stderr,"0");
        return 0;
    }
    for (i = 0; i < n-16; i += 16) {

        if (do_offsets) {
            fprintf(pf, "%04x: ", i);
        }

        for (j = 0; j < 16; j++) {
            fprintf(pf,"%02x", pc[i+j] & 0xff );
        }

        if (do_chars) {
            putc('\t',pf);
            for (j = 0; j < 16; j++) {
                putc( isprint(pc[i+j]) ? pc[i+j] : '.' , pf);
            }
        }
        putc('\n', pf);
    }
    if (do_offsets) {
	fprintf(pf, "%04x: ", i);
    }
    for ( j = 0; j < n - i ; j++ ) {
        fprintf(pf,"%02x", pc[i+j] & 0xff );
    }
    if (do_chars) {
	for ( ; j < 16; j++ ) {
	    fprintf(pf,"  ");
	}
	putc('\t',pf);
	for ( j = 0; j < n - i ; j++ ) {
	    putc( isprint(pc[i+j]) ? pc[i+j] : '.' ,pf );
	}
    }
    putc('\n',pf);
    fflush(pf);
    return 0;
}

int
ftt_debug_dump(unsigned char *pc, int n) {
    return ftt_dump(stderr,pc,n,1,1);
}

                                                                                                                                                                                                                                                                                                                                                                           home/vsergeev/ftt/ftt_lib/ftt_do_scsi.c                                                             0100660 0022366 0022366 00000044261 07144077446 021015  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";

#include <stdio.h>
#include <string.h>
#include <ftt_private.h>
#include <ctype.h>

#ifdef WIN32
#include <io.h>
#include <process.h>
#include <windows.h>

#define geteuid() -1
#define bzero ZeroMemory


#else
#include <unistd.h>
#endif

int ftt_close_scsi_dev(ftt_descriptor d) ;
int ftt_close_io_dev(ftt_descriptor d);
int ftt_get_stat_ops(char *name) ;
int ftt_describe_error();

void 
ftt_set_transfer_length( unsigned char *cdb, int n ) {
	cdb[2]= n >> 16 & 0xff;
	cdb[3]= n >> 8 & 0xff;
	cdb[4]= n & 0xff;
}

int
ftt_do_scsi_command(ftt_descriptor d,char *pcOp,unsigned char *pcCmd, 
	int nCmd, unsigned char *pcRdWr, int nRdWr, int delay, int iswrite) {
    int res;

    ENTERING("ftt_do_scsi_command");
    CKNULL("ftt_descriptor", d);
    CKNULL("Operation Name", pcOp);
    CKNULL("SCSI CDB", pcCmd);

    res = ftt_open_scsi_dev(d);  if (res < 0) return res;
    if ( !iswrite && nRdWr ) {
	memset(pcRdWr,0,nRdWr);
    }
    res = ftt_scsi_command(d->scsi_descriptor,pcOp, pcCmd, nCmd, pcRdWr, nRdWr, delay, iswrite);
    return res;
}

int
ftt_open_scsi_dev(ftt_descriptor d) {
    char *devname;

    /* can't have regular device and passthru open at same time */
    /* UNLESS the device we have default is also passthru... */

    if (!d->devinfo[d->which_is_default].passthru) {
	ftt_close_io_dev(d);

	if (d->scsi_descriptor < 0) {
	    devname = ftt_get_scsi_devname(d);
	    d->scsi_descriptor = ftt_scsi_open(devname);
	    if (d->scsi_descriptor < 0) {
		return ftt_translate_error(d,FTT_OPN_OPEN,"a SCSI open",
				    d->scsi_descriptor,"ftt_scsi_open",1);
	    }
	}
    } else {
       ftt_open_dev(d);
       d->scsi_descriptor = d->file_descriptor;
    }
    return d->scsi_descriptor;
}

int
ftt_close_scsi_dev(ftt_descriptor d) {
    int res;
    extern int errno;

    DEBUG3(stderr,"Entering close_scsi_dev\n");
    /* check if we're using the regular device */
    if(d->scsi_descriptor == d->file_descriptor) {
	d->scsi_descriptor = -1;
    }
    if(d->scsi_descriptor >= 0 ) {
	DEBUG1(stderr,"Actually closing scsi device\n");
        res = ftt_scsi_close(d->scsi_descriptor);
	DEBUG2(stderr,"close returned %d, errno %d\n", res, errno);
	d->scsi_descriptor = -1;
	return res;
    }
    return 0;
}

unsigned char ftt_sensebuf[18];

int
ftt_scsi_check(scsi_handle n,char *pcOp, int stat, int len) {
    int res;
    static int recursive = 0;
    static char *errmsg =
	"ftt_scsi_command: %s command returned  a %d, \n\
request sense data: \n\
%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n";

    static unsigned char acReqSense[]={ 0x03, 0x00, 0x00, 0x00, 
				     sizeof(ftt_sensebuf), 0x00 };

    DEBUG2(stderr, "ftt_scsi_check called with status %d len %d\n", stat, len);

    if (0 != n) {
	switch(stat) {
	default:
	    ftt_errno = FTT_ENXIO;
	    ftt_eprintf("While attempting SCSI passthrough, we encountered an \n\
unrecoverable system error");
	    break;
	case 0x00:
	    ftt_errno = FTT_SUCCESS;
	    break;
	case 0x04:
	    ftt_errno = FTT_EBUSY;
	    ftt_eprintf("While attempting SCSI passthrough, we encountered a \n\
device which was not ready");
	    break;
	case 0x02:
            if (!recursive) {
	        recursive = 1; /* keep from recursing if sense fails */
	        res = ftt_scsi_command(n,"sense",acReqSense, sizeof(acReqSense),
	  		               ftt_sensebuf, sizeof(ftt_sensebuf),5,0);
		DEBUG3(stderr,"request sense returns res %d:\n", res);
		DEBUG3(stderr, errmsg, pcOp, stat,
			ftt_sensebuf[0], ftt_sensebuf[1],
			ftt_sensebuf[2], ftt_sensebuf[3],
			ftt_sensebuf[4], ftt_sensebuf[5],
			ftt_sensebuf[6], ftt_sensebuf[7],
			ftt_sensebuf[8], ftt_sensebuf[9],
			ftt_sensebuf[10], ftt_sensebuf[12],
			ftt_sensebuf[13], ftt_sensebuf[14],
			ftt_sensebuf[15]);
		recursive = 0;
	    } else {
		return 0;
	    }
	    ftt_eprintf(errmsg, pcOp, stat,
		    ftt_sensebuf[0], ftt_sensebuf[1],
		    ftt_sensebuf[2], ftt_sensebuf[3],
		    ftt_sensebuf[4], ftt_sensebuf[5],
		    ftt_sensebuf[6], ftt_sensebuf[7],
		    ftt_sensebuf[8], ftt_sensebuf[9],
		    ftt_sensebuf[10], ftt_sensebuf[12],
		    ftt_sensebuf[13], ftt_sensebuf[14],
		    ftt_sensebuf[15]);
	    switch(ftt_sensebuf[2]& 0xf) {
	    default:
	    case 0x0:
		    if ( (ftt_sensebuf[2]&0x20) && (ftt_sensebuf[0]&0x80) ) {
			/* we have a valid, incorrect length indication */
			len -=  (ftt_sensebuf[3] << 24) + 
				(ftt_sensebuf[4] << 16) + 
				(ftt_sensebuf[5] <<  8) +
				ftt_sensebuf[6];
		        ftt_errno =  FTT_SUCCESS;
			/* XXX -- does this work in block mode? */
		    } else if ((ftt_sensebuf[2]&0x80) && (ftt_sensebuf[0]&0x80)){
			/* we read a filemark */
			len = 0;
		        ftt_errno =  FTT_SUCCESS;
		    } else if ((ftt_sensebuf[2]&0x40) && (ftt_sensebuf[0]&0x80)){
			/* we hit end of tape */
		        ftt_errno =  FTT_ENOSPC;
		    } else {
		        ftt_errno =  FTT_SUCCESS;
		    }
		    break;
	    case 0x1:
		    ftt_errno = FTT_EIO;
		    break;
	    case 0x2:
		    ftt_errno = FTT_ENOTAPE;
		    break;
	    case 0x3:
	    case 0x4:
		    ftt_errno = FTT_EIO;
		    break;
	    case 0x5:
	    case 0x6:
		    ftt_errno = FTT_ENOTSUPPORTED;
		    break;
	    case 0x7:
		    ftt_errno = FTT_EROFS;
		    break;
	    case 0x8:
		    ftt_errno = FTT_EBLANK;
		    break;
	    }
	}
    } 
    if (ftt_errno == FTT_SUCCESS) {
	return len;
    } else {
        return -stat;
    }
}

char *
ftt_get_scsi_devname(ftt_descriptor d){
    int j;

    ENTERING("ftt_get_scsi_devname");
    PCKNULL("ftt_descriptor", d);

    for( j = 0; d->devinfo[j].device_name != 0 ; j++ ){
	if( d->devinfo[j].passthru ){
	    DEBUG3(stderr, "Found slot %d, name %s\n", 
				       j,       d->devinfo[j].device_name);
	    return  d->devinfo[j].device_name;
	}
    }
    return 0;
}

/* 
** force us to use scsi pass-through ops to do everything
*/
int
ftt_all_scsi(ftt_descriptor d) {
    ENTERING("ftt_all_scsi");
    PCKNULL("ftt_descriptor", d);

    if ((d->flags & FTT_FLAG_SUID_SCSI) && geteuid() != 0) {
	ftt_eprintf("ftt_all_scsi: Must be root on this platform to do scsi pass through!");
	ftt_errno = FTT_EPERM;
	return -1;
    }


    d->scsi_ops = 0xffffffff;
    return 0;
}

#include "ftt_dbd.h"

static double pad;
int
ftt_scsi_set_compression(ftt_descriptor d, int compression) {

    /* getting evil alignment errors on IRIX6.5 */
    static unsigned char 
	mod_sen10[8] = { 0x1a, DBD, 0x10, 0x00, BD_SIZE+16, 0x00},
	mod_sel10[8] = { 0x15, 0x10, 0x00, 0x00, BD_SIZE+16, 0x00},
	mod_sen0f[8] = { 0x1a, DBD, 0x0f, 0x00, BD_SIZE+16, 0x00},
	mod_sel0f[8] = { 0x15, 0x10, 0x00, 0x00, BD_SIZE+16, 0x00},
	buf [32],
        opbuf[512];
    int res = 0;

    ENTERING("ftt_set_compression");
    CKNULL("ftt_descriptor", d);
    sprintf( opbuf , "2%s", d->prod_id);

    if ((d->flags&FTT_FLAG_SUID_SCSI) == 0 || 0 == geteuid()) {
	if (ftt_get_stat_ops(opbuf) & FTT_DO_MS_Px0f) {
	    DEBUG2(stderr, "Using SCSI Mode sense 0x0f page to set compression\n");
	    res = ftt_open_scsi_dev(d);        
	    if(res < 0) return res;
	    res = ftt_do_scsi_command(d, "Mode sense", mod_sen0f, 6, buf, BD_SIZE+16, 5, 0);
	    if(res < 0) return res;
	    buf[0] = 0;
	    buf[1] = 0;
	    /* enable outgoing compression */
	    buf[BD_SIZE + 2] &= ~(1 << 7);
	    buf[BD_SIZE + 2] |= (compression << 7);

	    res = ftt_do_scsi_command(d, "Mode Select", mod_sel0f, 6, buf, BD_SIZE+16, 120, 1);
	    if(res < 0) return res;
	    res = ftt_close_scsi_dev(d);
	    if(res < 0) return res;
	}
	if (ftt_get_stat_ops(opbuf) & FTT_DO_MS_Px10) {
	    DEBUG2(stderr, "Using SCSI Mode sense 0x10 page to set compression\n");
	    res = ftt_open_scsi_dev(d);        
	    if(res < 0) return res;
	    res = ftt_do_scsi_command(d, "Mode sense", mod_sen10, 6, buf, BD_SIZE+16, 5, 0);
	    if(res < 0) return res;
	    buf[0] = 0;
	    /* we shouldn't be changing density here but it shouldn't hurt */
	    /* yes it will! the setuid program doesn't know which density */
	    /* the parent process set... */
	    /* buf[BD_SIZE] = d->devinfo[d->which_is_default].hwdens; */
 	    buf[BD_SIZE + 14] = compression;
	    res = ftt_do_scsi_command(d, "Mode Select", mod_sel10, 6, buf, BD_SIZE+16, 120, 1);
	    if(res < 0) return res;
	    res = ftt_close_scsi_dev(d);
	    if(res < 0) return res;
	}
    } else {
        ftt_close_dev(d);
        ftt_close_scsi_dev(d);
	switch(ftt_fork(d)){

	static char s1[10];

	case -1:
		return -1;

	case 0:  /* child */
		fflush(stdout);	/* make async_pf stdout */
		close(1);
		dup2(fileno(d->async_pf_parent),1);
		sprintf(s1, "%d", compression);
		if (ftt_debug) {
		 execlp("ftt_suid", "ftt_suid", "-x", "-C", s1, d->basename, 0);
		} else {
		 execlp("ftt_suid", "ftt_suid", "-C", s1, d->basename, 0);
		}
		ftt_eprintf("ftt_set_compression: exec of ftt_suid failed");
		ftt_errno=FTT_ENOEXEC;
		ftt_report(d);

	default: /* parent */
		res = ftt_wait(d);
	}
    }
    return res;
}
extern ftt_itoa(long n);

int
ftt_scsi_locate( ftt_descriptor d, int blockno) {

    int res = 0;

    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {
	ftt_close_dev(d);
	switch(ftt_fork(d)){
	case -1:
		return -1;

	case 0:  /* child */
		fflush(stdout);	/* make async_pf stdout */
		fflush(d->async_pf_parent);
		close(1);
		dup2(fileno(d->async_pf_parent),1);
		if (ftt_debug) {
		    execlp("ftt_suid", "ftt_suid", "-x", "-l", ftt_itoa(blockno), d->basename, 0);
		} else {
		    execlp("ftt_suid", "ftt_suid", "-l", ftt_itoa(blockno), d->basename, 0);
		}

	default: /* parent */
		res = ftt_wait(d);
	}
    } else {
	static unsigned char 
	    locate_cmd[10] = {0x2b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	 
	locate_cmd[3] = (blockno >> 24) & 0xff;
	locate_cmd[4] = (blockno >> 16) & 0xff;
	locate_cmd[5] = (blockno >> 8)  & 0xff; 
	locate_cmd[6] = blockno & 0xff;
	res = ftt_do_scsi_command(d,"Locate",locate_cmd,10,NULL,0,300,0);
	res = ftt_describe_error(d,0,"a SCSI pass-through call", res, res,"Locate", 0);
        if (res < 0 && blockno == 0 && (ftt_errno == FTT_EBLANK || ftt_errno == FTT_ENOSPC)) {
             res = 0;
             ftt_errno = 0;
             ftt_eprintf("Ok");
        }

    }
    return res;
}

/*
 definitions and functions to do a scsi inquire and print the formatted results
*/
typedef struct
{
  unsigned char inqd[8];
/*
 bit fields in the inquire buffer  occupy the first 8 bytes, inq is a ptr to inqure buf
*/
#define BITFLD(offset,hibit,lobit) ((*(((char*)inq)+offset)) & ((2 << (hibit+1)) - 1)) >> lobit
#define pqt BITFLD(0,7,5)
#define pdt BITFLD(0,4,0)
#define rmb BITFLD(1,7,7)
#define dtq BITFLD(1,6,0)
#define iso BITFLD(2,7,6)
#define ecma BITFLD(2,5,3)
#define ansi BITFLD(2,2,0)
#define aenc BITFLD(3,7,7)
#define trmiop BITFLD(3,6,6)
#define res0 BITFLD(3,5,4)
#define respfmt BITFLD(3,4,0)
#define ailen BITFLD(4,7,0)
#define reladr BITFLD(7,7,7)
#define wide32 BITFLD(7,6,6)
#define wide16 BITFLD(7,5,5)
#define synch BITFLD(7,4,4)
#define link BITFLD( 7,3,3)
#define cmdq BITFLD( 7,1,1)
#define softre BITFLD(7,0,0)

  char  vid[8];        /* vendor ID */
  char  pid[16];       /* product ID */
  char  prl[4];        /* product revision level*/
  char  vendsp[20];      /* vendor specific; typically firmware */
  char  res4[40];        /* reserved for scsi 3, etc. */
  char  vendsp2[159];    /* more vend spec (fill to 255 bytes) */
} inqdata;

#define hex(x) "0123456789ABCDEF" [ (x) & 0xF ]

/* print an array in hex format, only looks OK if nperline a multiple of 4, 
 * but that's OK.  value of space must be 0 <= space <= 3;
 */
void
hprint(unsigned char *s, int n, int nperline, int space, int ascii)
{
        int   i, x, startl;

        for(startl=i=0;i<n;i++)  {
                x = s[i];
                printf("%c%c", hex(x>>4), hex(x));
                if(space)
                        printf("%.*s", ((i%4)==3)+space, "    ");
                if ( i%nperline == (nperline - 1) ) {
                        if(ascii == 1) {
                                putchar('\t');
                                while(startl < i) {
                                        if(isprint(s[startl]))
                                                putchar(s[startl]);
                                        else
                                                putchar('.');
                                        startl++;
                                }
                        }
                        putchar('\n');
                        if(ascii>1 && i<(n-1))  /* hack hack */
                                printf("%.*s", ascii-1, "        ");
                }
        }
        if(space && (i%nperline))
                putchar('\n');
}


/* aenc, trmiop, reladr, wbus*, synch, linkq, softre are only valid if
 * if respfmt has the value 2 (or possibly larger values for future
 * versions of the SCSI standard). */

static char pdt_types[][16] = {
   "Disk", "Tape", "Printer", "Processor", "WORM", "CD-ROM",
   "Scanner", "Optical", "Jukebox", "Comm", "Unknown"
};
#define NPDT (sizeof pdt_types / sizeof pdt_types[0])

void
printinq(inqdata *inq)
{ 
   unsigned char special;
   int neednl = 1;
   printf("%-10s", pdt_types[(pdt < NPDT) ? pdt : NPDT-1]);
      printf("%12.8s", inq->vid);
      printf("%.16s", inq->pid);
      printf("%.4s", inq->prl);
   printf("\n");
      printf("ANSI vers %d, ISO ver: %d, ECMA ver: %d; ",
              ansi, iso, ecma);
      special = *(inq->vid-1 );
      if(respfmt >= 2 || special) {
         if(respfmt < 2)
                 printf("\nResponse format type %d, but has "
                   "SCSI-2 capability bits set\n", respfmt );

         printf("supports: ");
         if(aenc)
                 printf(" AENC");
         if(trmiop)
                 printf(" termiop");
         if(reladr)
                 printf(" reladdr");
         if(wide32)
                 printf(" 32bit");
         if(wide16)
                 printf(" 16bit");
         if(synch)
                 printf(" synch");
         if(synch)
                 printf(" linkedcmds");
         if(cmdq)
                 printf(" cmdqueing");
         if(softre)
                 printf(" softreset");
      }
      if(respfmt < 2) {
         if(special)
                 printf(".  ");
         printf("inquiry format is %s",
                 respfmt ? "SCSI 1" : "CCS");
      }
      printf("\nvendor specific data:\n");
      hprint(inq->vendsp, 20,  16, 1, 1);
      neednl = 0;
      printf("reserved (for SCSI 3) data:\n");
      hprint(inq->res4, 40, 16, 1, 1) ;
      printf("more vendor data\n");
      hprint(inq->vendsp2, 159, 16, 1, 1);
      if(neednl)
         putchar('\n');
}

int ftt_inquire(ftt_descriptor d) {

    static unsigned char 
	inquiry[6] = { 0x12, 0x00, 0x00, 0x00, 255, 0x00};
    inqdata inqbuf ;
    int res;

    ENTERING("ftt_get_inquire");
    CKNULL("ftt_descriptor", d);
    DEBUG2(stderr, "Entering ftt_inquire\n");
    DEBUG3(stderr, "Using SCSI inquire \n");
    res = ftt_open_scsi_dev(d);   
    if(res < 0) return res;
    res = ftt_do_scsi_command(d, "inquire", inquiry, 6, (char *)&inqbuf, sizeof(inqdata), 5, 0);
    if(res < 0) return res;

    printinq(&inqbuf); 

    return res;
}

/*
  Use mode sense 0x3f to get all modesense pages and print them
*/
int ftt_modesense(ftt_descriptor d) {

    static unsigned char 
	mod_sen3f[6] = { 0x1a, 0x00, 0x3f, 0x00, 255, 0x00},
	msbuf [255], *mptr;
    int res;
    int dlen;

    ENTERING("ftt_modesense");
    CKNULL("ftt_descriptor", d);
    DEBUG2(stderr, "Entering ftt_modesense\n");
    DEBUG3(stderr, "Using SCSI Mode sense 0x3f page to get all mode sense\n");
    res = ftt_open_scsi_dev(d);        
    if(res < 0) return res;
    res = ftt_do_scsi_command(d, "Mode sense", mod_sen3f, 6, msbuf, 255, 5, 0);
    if(res < 0) return res;

    dlen = msbuf[0];
    if(dlen < 4)
                return 1;
    mptr = msbuf;

    printf("Header:\n length %#x, med type %#x, dev spcfc %#x, blk desc len %#x\n", 
           msbuf[0], msbuf[1], msbuf[2], msbuf[3]);
    mptr += 4;
    dlen -= 4;
    if(msbuf[3])
       printf("Block Descriptors:\n ");
    while(msbuf[3] && dlen >= 8) {
       hprint(mptr, 8, 8, 1, 0);
       msbuf[3] -= 8;
       dlen -= 8;
       mptr += 8;
    }
    while(dlen >= 3) {
       int len;
       printf("Page %#x, length %#x:\n ", 0x3f & *mptr, mptr[1]);
       len = dlen > (mptr[1]+2) ? mptr[1] : dlen - 2;
       hprint(&mptr[2], mptr[1], 20, 1, 0);
       mptr += len + 2;
       dlen -= len + 2;
    }

    return res;
}
/*
 use log sense 0x0 to get a list of log sense pages, then get each page in turn
 and print it
*/
int ftt_logsense(ftt_descriptor d) {

    static unsigned char 
	logsense0h[10]={0x4d, 0x00, 0x40, 0x00, 0x00,0x00,0x00, 0x10, 0x00, 0x00},
        lslist[255],
	lsbuf [0x1000], *lptr;
    int res;
    int dlen;
    int pagelen, param_code, param_length, param_flags;
    unsigned long param_val;
    unsigned char *pageptr, *param_ptr;

    ENTERING("ftt_get_logsense");
    CKNULL("ftt_descriptor", d);
    DEBUG2(stderr, "Entering ftt_get_logsense\n");
    DEBUG3(stderr, "Using SCSI log sense 0x0 page to get get list of pages\n");
    res = ftt_open_scsi_dev(d);        
    if(res < 0) return res;
    res = ftt_do_scsi_command(d, "log sense", logsense0h, 10, lslist, 255, 5, 0);
    if(res < 0) return res;
    dlen = (lslist[2] << 8) + lslist[3];
    for(lptr=&lslist[4]; dlen-- > 0; lptr++) {
       if (*lptr == 0) 
          continue;
       memset(lsbuf, 0, 8);
       logsense0h[2]= 0x40 | *lptr;		/* cum values for page */
       printf ("Retrieving LOG SENSE PAGE %x \n",*lptr);
       res = ftt_do_scsi_command(d, "log sense", 
                    logsense0h, 10, lsbuf, 0x1000, 5, 0);
       if(res < 0) return res;
       printf ("CODE FLAG LENGTH   VAL BASE 10     VAL HEX - got page %x\n", lsbuf[0]);
       pagelen = (lsbuf[2]<<8) + lsbuf[3];
       pageptr = lsbuf + 4;
       while (pageptr < (lsbuf + pagelen)) {
          param_code   = (*pageptr << 8) + *(pageptr+1);
          param_length = *(pageptr+3);
          param_flags  = *(pageptr+2);
          for (param_ptr = pageptr+4, param_val=0; 
                   param_ptr < pageptr+4+param_length; param_ptr++)
              param_val = (param_val*256) + *param_ptr;
          printf("%4x %4x %4x %16d ", param_code, param_flags, param_length, param_val);
          for (param_ptr = pageptr+4; param_ptr < pageptr+4+param_length; param_ptr++)
              printf("%3x", *param_ptr);
          printf("\n");
          pageptr = pageptr + param_length + 4;
       }
    }
    return res;
}

                                                                                                                                                                                                                                                                                                                                               home/vsergeev/ftt/ftt_lib/ftt_error.c                                                               0100660 0022366 0022366 00000037127 07200414760 020511  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdarg.h>
#include <stdio.h>
#include <ftt_private.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <winioctl.h>

int ftt_describe_error_WIN();
#endif

int ftt_describe_error();
int ftt_verify_blank(ftt_descriptor d);

/*
** ftt_eprintf call...
** we'll make it more robust later...
*/
void
ftt_eprintf(char *format, ...)
{
  va_list args;

  va_start(args, format);
  vsprintf(ftt_eprint_buf, format, args);
  va_end(args);
}

char *
ftt_get_error(int *pn){
   if (pn != 0) {
	*pn = ftt_errno;
   }
   return ftt_eprint_buf;
}

char *ftt_ascii_error[] = {
/* FTT_SUCCESS		 0 */ "FTT_SUCCESS",
/* FTT_EPARTIALSTAT	 1 */ "FTT_EPARTIALSTAT",
/* FTT_EUNRECOVERED	 2 */ "FTT_EUNRECOVERED",
/* FTT_ENOTAPE		 3 */ "FTT_ENOTAPE",
/* FTT_ENOTSUPPORTED 	 4 */ "FTT_ENOTSUPPORTED",
/* FTT_EPERM		 5 */ "FTT_EPERM",
/* FTT_EFAULT		 6 */ "FTT_EFAULT",
/* FTT_ENOSPC		 7 */ "FTT_ENOSPC",
/* FTT_ENOENT		 8 */ "FTT_ENOENT",
/* FTT_EIO		 9 */ "FTT_EIO",
/* FTT_EBLKSIZE		10 */ "FTT_EBLKSIZE",
/* FTT_ENOEXEC		11 */ "FTT_ENOEXEC",
/* FTT_EBLANK           12 */ "FTT_EBLANK", 
/* FTT_EBUSY		13 */ "FTT_EBUSY",
/* FTT_ENODEV		14 */ "FTT_ENODEV",
/* FTT_ENXIO		15 */ "FTT_ENXIO",
/* FTT_ENFILE		16 */ "FTT_ENFILE",
/* FTT_EROFS		17 */ "FTT_EROFS",
/* FTT_EPIPE		18 */ "FTT_EPIPE",
/* FTT_ERANGE		19 */ "FTT_ERANGE",
/* FTT_ENOMEM		20 */ "FTT_ENOMEM",
/* FTT_ENOTTAPE		21 */ "FTT_ENOTTAPE",
/* FTT_E2SMALL		22 */ "FTT_E2SMALL",
/* FTT_ERWFS		23 */ "FTT_ERWFS",
/* FTT_EWRONGVOL	24 */ "FTT_EWRONGVOL",
/* FTT_EWRONGVOLTYP	25 */ "FTT_EWRONGVOLTYP",
/* FTT_ELEADER		26 */ "FTT_ELEADER",
/* FTT_EFILEMARK	27 */ "FTT_EFILEMARK",
/* FTT_ELOST		28 */ "FTT_ELOST",
/* FTT_ENOTBOT  	29 */ "FTT_ENOTBOT",
/* FTT_EPARTIALWRITE  	30 */ "FTT_EPARTIALWRITE",
/* FTT_MAX_ERROR	31 */ "FTT_MAX_ERROR",
0 
};

static char *messages[] = {
	/* FTT_SUCCESS		 0 */
    "that no error has occurred",
	/* FTT_EPARTIALSTAT	 1 */ 
    "that not all of the statistics which should have been avaliable\n\
	on this drive and system were able to be obtained.",
	/* FTT_EUNRECOVERED	 2 */ 
    "\tWe are now not at an unknown tape position and are \n\
	unable to proceed without possibly damaging the tape.  This \n\
	message will repeat until a rewind of the tape is performed.\n",
	/* FTT_ENOTAPE		 3 */ 
    "that no tape is currently in the tape drive.",
	/* FTT_ENOTSUPPORTED 	 4 */ 
    "that the device/tape drive combination is not supported.",
	/* FTT_EPERM		 5 */ 
    "that you do not have permissions to access the system device.",
	/* FTT_EFAULT		 6 */ 
    "that you have provided an invalid buffer address.",
	/* FTT_ENOSPC		 7 */ 
    "that the data you requested will not fit in the provided buffer.",
	/* FTT_ENOENT		 8 */ 
    "that the system device for the mode and density you requested\n\
	does not exist on this system.",
	/* FTT_EIO		 9 */
    "that an unrecoverable error occurred due to bad or damaged tape \n\
	or bad or dirty tape heads.",
	/* FTT_EBLKSIZE		10 */ 
    "that the block size you tried to write is not appropriate for \n\
	this device and mode.",
	/* FTT_ENOEXEC		11 */ 
    "that the setuid executable needed to perform this operation on \n\
	this system is not avaliable.",
	/* FTT_EBLANK		12 */
    "that we encountered blank tape or end of tape.",
	/* FTT_EBUSY		13 */
    "that some other process is using the drive at this time.",
	/* FTT_ENODEV		14 */
    "that while the device for this mode and density appears in /dev,\n\
	the device driver is not configured for it.",
	/* FTT_ENXIO		15 */
    "that you have tried to go past the end of the tape.",
	/* FTT_ENFILE		16 */ 
    "that you have tried to open more files simultaneously than is \n\
	possible on this system.",
	/* FTT_EROFS		17 */ 
    "that the tape is write protected.",
	/* FTT_EPIPE		18 */
    "that the process which was invoked to perform this task on our \n\
	behalf died unexpectedly.",
	/* FTT_ERANGE		19 */
    "The buffer you provided for tape data was smaller than the data \n\
     block on the tape.",
	/* FTT_ENOMEM		20 */
    "that we were unable to allocate memory needed to perform the task.",
	/* FTT_ENOTTAPE		21 */
    "that the device specified was not a tape.",
	/* FTT_E2SMALL		22 */
    "that the block size issued is smaller than this device can handle.",
	/* FTT_ERWFS		23 */
    "that the tape was supposed to be write protected and is writable.",
	/* FTT_EWRONGVOL	24 */
    "The wrong tape volume was given to the volume verification",
	/* FTT_EWRONGVOLTYP	25 */
    "The wrong type of tape volume was given to the volume verification",
	/* FTT_ELEADER		26 */
    "Beginning of tape was encountered before completing the operation",
	/* FTT_EFILEMARK	27 */
    "A Filemark was encountered before completing the operation",
	/* FTT_ELOST	28 */
    "We do not yet know our current tape position.",
	/* FTT_ENOTBOT  	29 */ 
    "FTT_ENOTBOT",
	/* FTT_MAX_ERROR	30 */ 
    "FTT_MAX_ERROR",
    0
};

extern int errno;

int
ftt_translate_error(ftt_descriptor d, int opn, char *op, int res, char *what, int recoverable) {
    int terrno;		/* trimmed errno to fit in table size */
    static ftt_stat sbuf;
    char *p;
    int keep_errno;   /* errno when we started */
    int guess_errno;  /* best guess so far */
    int keep_res;     /* res when we started */

    /* save initial state */
    keep_res = res;
    keep_errno = errno;

    DEBUG3(stderr,"Entering ftt_translate_error -- opn == %d, op = %s, res=%d, what=%s recoverable=%d\n",
	opn,op, res, what, recoverable);

    if( 0 == d ) {
	ftt_eprintf("%s called with NULL ftt_descriptor\n", op);
	ftt_errno = FTT_EFAULT;
	return -1;
    }

    /* linux gives this when out of buffers. For some reason we bail here? */
    if (keep_errno == 75) {	
	terrno = ENOMEM;
        ftt_errno = d->errortrans[opn][terrno];
	errno = keep_errno;
        return ftt_describe_error(d, opn, op, keep_res, res, what, recoverable);
    }

    /* 
    ** otherwise we pick an errno to translate to make sure its not 
    ** past the table end
    */
    if (keep_errno >= MAX_TRANS_ERRNO) {
        terrno = MAX_TRANS_ERRNO - 1;
    } else {
	terrno = keep_errno;
    } 

    /*
    ** our initial guess for ftt_errno comes from the table
    */
    guess_errno = d->errortrans[opn][terrno];

    /*
    ** Now we may need to verify that we have a real eof versus a fake one
    ** that should have been a read-past-eot error, if we do that here
    */
    if (0 == res && FTT_OPN_READ == opn && 0 !=(d->flags&FTT_FLAG_VERIFY_EOFS)) {
	DEBUG2(stderr, "translate_error: Verifying an eof...\n");
	ftt_get_stats(d, &sbuf);

	if (0 != (p = ftt_extract_stats(&sbuf,FTT_SENSE_KEY)) && 8 == atoi(p)) {
	    DEBUG3(stderr, "Saw blank check sense key\n");
	    res = -1;
	    guess_errno = FTT_EBLANK;
	} else {
	    DEBUG3(stderr, "Sense key was %s\n", p);
	    if (0 != (p = ftt_extract_stats(&sbuf,FTT_BLOC_LOC))) {
		DEBUG3(stderr, "Current loc %s, last loc %d\n", p, d->last_pos);
		if ((d->last_pos > 0 && atoi(p) == d->last_pos) || atoi(p) == 0) {
		    guess_errno = FTT_EBLANK;
		    res = -1;
		}
		d->last_pos = atoi(p);
	    } else if (0 != (p = ftt_extract_stats(&sbuf,FTT_REMAIN_TAPE))) {
		DEBUG3(stderr, "Current remain %s, last remain %d\n", p, d->last_pos);
		if (d->last_pos > 0 && atoi(p) == d->last_pos) {
		    guess_errno = FTT_EBLANK;
		    res = -1;
		}
		d->last_pos = atoi(p);
	    }

	}
    }


    /*
    ** now if we have an EIO error and we're file skipping or rewinding,
    ** it could be end of tape, or no tape, so figure that out
    */
#   define CHECKS (FTT_OP_SKIPFM|FTT_OP_RSKIPFM|FTT_OP_SKIPREC|FTT_OP_RSKIPREC\
			|FTT_OP_READ|FTT_OP_REWIND)

    if ( -1 == res && ((1<<opn)&CHECKS) && (guess_errno == FTT_EIO || guess_errno == FTT_ELEADER)) {
	int statres;

	 DEBUG3(stderr, "Checking for blank tape on other error\n");
	 statres = ftt_status(d, 0);

         if (! (statres & FTT_ONLINE)) {
	     guess_errno = FTT_ENOTAPE;
	     res = -1;
         } else if (statres & (FTT_AEOT|FTT_ABOT)) {
             if (statres & FTT_AEOT) {
		 guess_errno = FTT_EBLANK;
		 res = -1;
             }
	     if ( statres & FTT_ABOT ) {
		 guess_errno = FTT_ELEADER;
		 res = -1;
             }
	 } else {

	    if (FTT_EBLANK == guess_errno && opn == FTT_OPN_READ &&
			d->current_file == 0 && d->current_block == 0 &&
			(d->scsi_ops & FTT_OP_READ) == 0 && d->scsi_ops != 0 ) {

	        DEBUG2(stderr, "translate_error: checking for empty tape error...\n");
		res = ftt_verify_blank(d);
		if ( 0 <= res && ftt_errno == FTT_SUCCESS) {
		     guess_errno = FTT_EIO;
		     res = -1;
		}
	    }
        }
    }

    /*
    ** blank check on writes
    */
    if (FTT_EBLANK == guess_errno && opn == FTT_OPN_WRITE || opn == FTT_OPN_WRITEFM ) {

	/* people don't take  "Blank" seriously on writes... */

	guess_errno = FTT_EIO;
    }

    /* 
    ** in case any calls we made messed with errno, we put it back 
    ** and we take our best guess to be ftt_errno.
    */
    errno = keep_errno;
    ftt_errno = guess_errno;
    return ftt_describe_error(d, opn, op, keep_res, res, what, recoverable);
}

int
ftt_describe_error(ftt_descriptor d, int opn, char *op, int keep_res, int res, char *what, int recoverable) {

    if (0 <= res) {
	ftt_errno = FTT_SUCCESS;
	return res;
    }
    ftt_eprintf( "\
%s: doing %s on %s returned %d,\n\
	errno %d, => result %d, ftt error %s(%d), meaning \n\
	%s\n%s",

	what, op,  (d->which_is_open >= 0 ? 
				d->devinfo[d->which_is_open].device_name :
				d->basename),
	keep_res, errno, res, ftt_ascii_error[ftt_errno], ftt_errno,
	messages[ftt_errno], recoverable ? "": messages[FTT_EUNRECOVERED] );

    DEBUG2(stderr, "ftt_translate_error -- message is:\n%s", ftt_eprint_buf);

    if (!recoverable) {
	d->unrecovered_error = opn < FTT_OPN_WRITEFM ? 1 : 2;
	d->current_valid = 0;
    }

    return res;
}

#ifdef WIN32

static int
ftt_WIN_error (DWORD res) {
	
	switch (res) {
		case NO_ERROR						: return FTT_SUCCESS;
		case ERROR_BEGINNING_OF_MEDIA		: return FTT_ELEADER;
		case ERROR_BUS_RESET				: return FTT_EUNRECOVERED;
		case ERROR_END_OF_MEDIA				: return FTT_ENOSPC;
		case ERROR_FILEMARK_DETECTED		: return FTT_EFILEMARK;
		case ERROR_SETMARK_DETECTED			: return FTT_ENOTSUPPORTED;
		case ERROR_NO_DATA_DETECTED			: return FTT_EBLANK;
		case ERROR_PARTITION_FAILURE		: return FTT_ENOTSUPPORTED;
		case ERROR_INVALID_BLOCK_LENGTH		: return FTT_EBLKSIZE;
		case ERROR_DEVICE_NOT_PARTITIONED	: return FTT_ENOTSUPPORTED;
		case ERROR_MEDIA_CHANGED			: return FTT_EUNRECOVERED;
		case ERROR_NO_MEDIA_IN_DRIVE		: return FTT_ENOTAPE;
		case ERROR_NOT_SUPPORTED			: return FTT_ENOTSUPPORTED;
		case ERROR_UNABLE_TO_LOCK_MEDIA		: return FTT_ENOTSUPPORTED;
		case ERROR_UNABLE_TO_UNLOAD_MEDIA	: return FTT_ENOTSUPPORTED;
		case ERROR_WRITE_PROTECT			: return FTT_EROFS;
	
		case ERROR_INVALID_FUNCTION			: return FTT_ENOENT;
		case ERROR_FILE_NOT_FOUND			: return FTT_ENOENT;
		case ERROR_PATH_NOT_FOUND			: return FTT_ENOENT;
		case ERROR_TOO_MANY_OPEN_FILES		: return FTT_ENFILE;
		case ERROR_ACCESS_DENIED			: return FTT_EPERM;
		case ERROR_INVALID_HANDLE			: return FTT_ENOENT;
		case ERROR_NOT_ENOUGH_MEMORY		: return FTT_ENOMEM;
		case ERROR_INVALID_BLOCK			: return FTT_ENOTSUPPORTED;
		case ERROR_BAD_ENVIRONMENT			: return FTT_EFAULT;
		case ERROR_BAD_FORMAT				: return FTT_EFAULT;
		case ERROR_INVALID_ACCESS			: return FTT_ENOENT;
		case ERROR_INVALID_DATA				: return FTT_ENOENT;
		case ERROR_OUTOFMEMORY				: return FTT_ENOMEM;
		case ERROR_INVALID_DRIVE			: return FTT_ENOENT;
		case ERROR_CURRENT_DIRECTORY		: return FTT_EPERM;
		case ERROR_NOT_SAME_DEVICE			: return FTT_ENOTSUPPORTED;
		case ERROR_NO_MORE_FILES			: return FTT_ENOENT;
		case ERROR_BAD_UNIT					: return FTT_ENOTSUPPORTED;
		case ERROR_NOT_READY				: return FTT_EIO; /* ?????????*/
		case ERROR_BAD_COMMAND				: return FTT_ENOENT;
		case ERROR_BAD_LENGTH				: return FTT_ENOENT;
		case ERROR_WRITE_FAULT				: return FTT_EIO;
		case ERROR_READ_FAULT				: return FTT_EIO;
		case ERROR_GEN_FAILURE				: return FTT_EFAULT;
		case ERROR_HANDLE_EOF				: return FTT_EIO;
		case ERROR_OPEN_FAILED				: return FTT_EFAULT;
		case ERROR_BUSY_DRIVE				: return FTT_EBUSY;
		case ERROR_BUSY						: return FTT_EBUSY;
		case ERROR_IO_PENDING				: return FTT_EBUSY;

		case ERROR_LOCK_VIOLATION			: return FTT_ENOTSUPPORTED;
		case ERROR_BAD_NETPATH				: return FTT_ENOTSUPPORTED;
		case ERROR_FILE_EXISTS				: return FTT_ENOTSUPPORTED;
		case ERROR_CANNOT_MAKE				: return FTT_ENOTSUPPORTED;
		case ERROR_FAIL_I24					: return FTT_ENOTSUPPORTED;
		case ERROR_INVALID_PARAMETER		: return FTT_EBLKSIZE; /* this is what you get */
		
		case ERROR_DRIVE_LOCKED				: return FTT_EPERM;
		case ERROR_BROKEN_PIPE				: return FTT_ENOTSUPPORTED;
		case ERROR_IO_DEVICE				: return FTT_EIO;

		case ERROR_EOM_OVERFLOW				: return FTT_EBLANK;
		case ERROR_POSSIBLE_DEADLOCK		: return FTT_EBUSY;

		case ERROR_SHARING_VIOLATION        : return FTT_EBUSY; /* the device is opened */
		default								: return FTT_ENOTSUPPORTED;
  }
}

 int
	ftt_translate_error_WIN(ftt_descriptor d, int opn, char *op, DWORD fres, char *what, int recoverable) {
    
    static ftt_stat sbuf;
    char *p;
    int save1, save2 ;

    DEBUG3(stderr,"Entering ftt_translate_error_WIN -- opn == %d, op = %s, res=%d, what=%s recoverable=%d\n",
	opn,op, fres, what, recoverable);

    if( 0 == d ) {
		ftt_eprintf("%s called with NULL ftt_descriptor\n", op);
		ftt_errno = FTT_EFAULT;
		return -1;
    }
	/* - instead of table - is easier this way - */

    ftt_errno = ftt_WIN_error(fres);


#   define CHECKS (FTT_OP_SKIPFM|FTT_OP_RSKIPFM|FTT_OP_SKIPREC|FTT_OP_RSKIPREC\
			|FTT_OP_READ|FTT_OP_REWIND)

    if ((NO_ERROR == fres && FTT_OPN_READ == opn && 0 !=(d->flags&FTT_FLAG_VERIFY_EOFS))
    		      || (NO_ERROR != fres && ((1<<opn)&CHECKS) )) {
		/* 
		** save errno and ftt_errno so we can restore them 
		** after getting status 
		*/
		save1 = ftt_errno;
		save2 = errno;
		
		ftt_get_stats(d, &sbuf);
		errno = save2;
		
		if (0 != (p = ftt_extract_stats(&sbuf,FTT_SENSE_KEY))) {
			if (8 == atoi(p)){
				fres = (DWORD) -1;
				save1 = ftt_errno = FTT_EBLANK;
			} else {
				ftt_errno = save1;
			}
		} else {
			ftt_errno = save1;
		}
		if (0 != (p = ftt_extract_stats(&sbuf,FTT_BLOC_LOC))) {
			DEBUG3(stderr, "Current loc %s, last loc %d\n", p, d->last_pos);
			if (d->last_pos > 0 && atoi(p) == d->last_pos) {
				ftt_errno = FTT_EBLANK;
				fres = (DWORD)-1;
			} else {
				ftt_errno = save1;
			}
			d->last_pos = atoi(p);
		} else if (0 != (p = ftt_extract_stats(&sbuf,FTT_REMAIN_TAPE))) {
			DEBUG3(stderr, "Current remain %s, last remain %d\n", p, d->last_pos);
			if (d->last_pos > 0 && atoi(p) == d->last_pos) {
				ftt_errno = FTT_EBLANK;
				fres = (DWORD)-1;
			} else {
				ftt_errno = save1;
			}
			d->last_pos = atoi(p);
		} else {
			ftt_errno = save1;
		}
    }
	
    return ftt_describe_error_WIN(d, opn, op, fres, what, recoverable);
}

int
ftt_describe_error_WIN(ftt_descriptor d, int opn, char *op, DWORD res, char *what, int recoverable) 
{
	if (NO_ERROR == res) {
		ftt_errno = FTT_SUCCESS;
		return 0;
    }
    ftt_eprintf( "\
%s: doing %s on %s returned %d,\n\
	errno %d, => ftt error %s(%d), meaning \n\
	%s\n%s",

	what, op,  (d->which_is_open >= 0 ? 
				d->devinfo[d->which_is_open].device_name :
				d->basename),
	res, errno, ftt_ascii_error[ftt_errno], ftt_errno,
	messages[ftt_errno], recoverable ? "": messages[FTT_EUNRECOVERED] );

    DEBUG2(stderr, "ftt_translate_error_WIN -- message is:\n%s", ftt_eprint_buf);

    if (!recoverable) {
		d->unrecovered_error = opn < FTT_OPN_WRITEFM ? 1 : 2;
		d->current_valid = 0;
    }

    return -1;
}
#endif                    /* WIN-NT trunslate error functions */

#ifdef TESTTABLES
main(){
	int i;
	for( i = 0; ftt_ascii_error[i] != 0 ; i++ ) {
		printf("%d -> %s -> %s\n", i, ftt_ascii_error[i], messages[i]);
	}
}
#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                         home/vsergeev/ftt/ftt_lib/ftt_find.c                                                                0100660 0022366 0022366 00000013667 07023542344 020306  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <ftt_private.h>

#ifdef WIN32         /* this is Windows */

#include <process.h>
#include <windows.h>
#define geteuid() -1 
#define popen _popen
#define pclose _pclose


char * ftt_get_os() {
	char ver[20],rel[20];
	char *os = "WINNT";
    OSVERSIONINFO buf;
	buf.dwOSVersionInfoSize =sizeof(OSVERSIONINFO);
    GetVersionEx(&buf);
	if (buf.dwPlatformId != VER_PLATFORM_WIN32_NT ) os = "WIN32";
	sprintf(rel,"%d",buf.dwMajorVersion);
	sprintf(ver,"%d",buf.dwMinorVersion);
    return ftt_make_os_name( "WINNT", rel,ver);
}

#else                /* this is UNIX */

#include <unistd.h>
#include <sys/utsname.h>

char *
ftt_get_os() {
    struct utsname buf;

    uname(&buf);
    return ftt_make_os_name( buf.sysname, buf.release, buf.version);
}
#endif

char *
ftt_make_os_name(char *sys, char *release , char *version) {
    static char sysname[512];

    sprintf(sysname,"%s+%s.%s", sys, release, version);
    return sysname;
}

int
ftt_findslot (char *basename, char *os, char *drivid,  
			void *p1, void *p2, void *p3) {
    int i;
    char *lastpart;
    int res;

    DEBUG4(stderr,"Entering: ftt_findslot %s %s %s\n", basename, os, drivid );

    /* tables now only deal with the last directory and file 
    ** component of the pathname 
    */ 

    lastpart = ftt_find_last_part(basename);

    DEBUG3(stderr,"looking at '%s' part of name\n", lastpart);

    for( i = 0; devtable[i].os !=0 ; i++ ) {
	if (ftt_matches(os, devtable[i].os) && 
		ftt_matches(drivid, devtable[i].drivid)) {
	   DEBUG4(stderr,"trying format \"%s\" against %s\n", 
		devtable[i].baseconv_in, lastpart);


           res = sscanf(lastpart,devtable[i].baseconv_in,p1,p2,p3);

	   if (devtable[i].nconv == res ) {
		     DEBUG3(stderr, "format Matches (\"%s\" against %s)!\n",devtable[i].baseconv_in, lastpart);
		     return i;
	   }
	   DEBUG3(stderr, "format missed... got %d, not %d\n",
				res, devtable[i].nconv);
	}
    }
    return -1;
}

extern char *
ftt_strip_to_basename(const char *basename,char *os) {
    static char buf[512];
    static char buf2[512];
    static union { int n; char s[512];} s1, s2, s3;
 
    int i;
    char *lastpart;

    DEBUG4(stderr, "Entering: ftt_strip_to_basename\n");
    memset(buf,0, 512);
    memset(buf2,0, 512);
    memset(s1.s,0, 512);

    strncpy(buf, basename, 512);

#ifdef WIN32
    strlwr( buf);
#endif 

#ifdef DO_SKIP_SYMLINKS
    {
      int maxlinks=512;
	  
      while( 0 <  readlink(buf, buf2, 512) && maxlinks-- >0 ) {
	if( buf2[0] == '/' ) {
	  /* absolute pathname, replace the whole buffer */
	  strncpy(buf,buf2,512);
	} else {
	  /* relative pathname, replace after last /, if any */
	  if ( 0 == (p = strrchr(buf,'/'))) {
	    p = buf;
	  } else {
	    p++;
	  }
	  strncpy(p, buf2, 512 - (p - buf));
	}
      }
    }
#endif

    i = ftt_findslot(buf, os, "", &s1, &s2, &s3);
    if (i < 0) {
	return 0;
    }
    /* tables now only deal with the last directory and file component of 
    ** the pathname 
    */
    lastpart = ftt_find_last_part(buf);

    /*
    ** first item in the format can be either a string or a digit;
    ** check for strings
    */
    if ( devtable[i].baseconv_out[1] == 's') {
	sprintf(lastpart, devtable[i].baseconv_out, s1.s, s2.n, s3.n);
    } else {
	sprintf(lastpart, devtable[i].baseconv_out, s1.n, s2.n, s3.n);
    }
    return strdup(buf);
}

/*
** search for last 2 slashes in pathname,
** and return the pointer to the character after the next to last one.
** if there isn't one, return the pointer to the original string
*/
char *
ftt_find_last_part( char *p ) {
    char *s, *s1 = 0, *s2 = 0;
	char s_find = '/';

	/* -------------------- for Windows NT ------------------------------- */
#ifdef WIN32
	s_find = '\\';
#endif

    s = p;
    while( s && *s ) {
	if( *s == s_find ) {
	    s2 = s1;
	    s1 = s;
	}
	s++;
    }
    if( s2 ) {
	return s2+1;
    } else {
	return p;
    }
}


/*
** get_driveid guesses the drive id the best it can from the available
** system configuration command(s).
** Returns a SCSI device id, or a prefix thereof
*/
extern char *
ftt_get_driveid(char *basename,char *os) {
    static char cmdbuf[512];
    static char output[512];
    static union { int n; char s[512];} s1, s2, s3;
    FILE *pf;
    char *res = 0;
    int i;

    DEBUG4(stderr, "Entering: ftt_get_driveid\n");
    i = ftt_findslot(basename, os, "",  &s1, &s2, &s3);
    if (i < 0) {
	return 0;
    }
    if ( 0 != geteuid() && (devtable[i].flags & FTT_FLAG_SUID_DRIVEID) ) {

	DEBUG3( stderr, "Running ftt_suid...\n" );
	sprintf(cmdbuf, "ftt_suid -i %s", basename );
	pf = popen(cmdbuf, "r");
        if (pf != 0) {
	    res = fgets(output,512,pf);
	    pclose(pf);
	} else {
	    res = 0;
	}
    } else {
	if (FTT_GUESS_ID) {
	    if ( devtable[i].drividcmd[1] == 's') {
		sprintf(cmdbuf, devtable[i].drividcmd, s1.s, s2.n, s3.n);
	    } else {
		sprintf(cmdbuf, devtable[i].drividcmd, s1.n, s2.n, s3.n);
	    }
	    DEBUG4(stderr,"Running \"%s\" to get drivid (lenght %d < 512 ) \n", cmdbuf,strlen(cmdbuf));
	    pf = popen(cmdbuf, "r");
	    if (pf) {
		res = fgets(output, 512,pf);
		pclose(pf);
	    }
        } else {
	    /* Actually look it up... */
            ftt_descriptor tmp;
            ftt_stat_buf b;
            char *pc;

 	    tmp = ftt_open_logical(basename,  ftt_get_os(), "XXXXXX", 1);
            b = ftt_alloc_stat();
            ftt_get_stats(tmp, b);
            if (ftt_debug > 3) {
		printf("stats at open are:\n");
		ftt_dump_stats(b,stdout);
            }
            pc = ftt_extract_stats(b,FTT_PRODUCT_ID);
	    if (pc) {
		res = strcpy(output,  pc);
		strcat(output, "\n");
	    } else {
                strcpy( output , "\n");
            }
            ftt_free_stat(b);
            ftt_close(tmp);
        }
    }
    if (res != 0) {
	output[strlen(output)-1] = 0; /* stomp the newline */
	res = strdup(output);
    }
    DEBUG3(stderr, "returning %s\n", res);
    return res;
}


                                                                         home/vsergeev/ftt/ftt_lib/ftt_global.c                                                              0100660 0022366 0022366 00000000313 06252127456 020614  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <ftt_private.h>

int ftt_errno;
int ftt_debug;
char ftt_eprint_buf[FTT_EPRINT_BUF_SIZE];
                                                                                                                                                                                                                                                                                                                     home/vsergeev/ftt/ftt_lib/ftt_higher.c                                                              0100640 0022366 0022366 00000051641 10037773175 020633  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftt_private.h"

#ifdef WIN32
#include <io.h>
#include <process.h>
#define geteuid() -1
#endif

extern int errno;

void ftt_to_upper( char *p ) ;

int
ftt_verify_vol_label(ftt_descriptor d, int type, char *vollabel, 
			int timeout, int rdonly) {
    char *buf;
    char label_buf[512];
    int res=0,status=0,retval=0;
    char *pname;
    int len;
    int blocksize;

    ENTERING("ftt_verify_vol_label");
    CKNULL("ftt_descriptor", d);
    CKNULL("volume label", vollabel);

    if (type >= FTT_MAX_HEADER || type < 0) {
	ftt_errno = FTT_ENOTSUPPORTED;
	ftt_eprintf("ftt_verify_vol_label: unsupported type number %d", type);
	return -1;
    }

    status = ftt_status(d,timeout);	if (res < 0) return res;

    if (0 == (status & FTT_ONLINE)) {
	ftt_errno = FTT_ENOTAPE;
	ftt_eprintf("ftt_verify_vol_label: the drive is empty");
	return -1;
    }

    if (0 != (status & FTT_BUSY)) {
	ftt_errno = FTT_EBUSY;
	ftt_eprintf("ftt_verify_vol_label: the drive is busy");
	return -1;
    }


    res = ftt_rewind(d);  			if (res < 0) return res;

    if (type != FTT_DONTCHECK_HEADER) {

	blocksize = ftt_get_max_blocksize(d);
	buf = malloc(blocksize);
	if (buf == 0) {
	    extern int errno;
	    ftt_errno = FTT_ENOMEM;
	    ftt_eprintf("ftt_verify_vol_label: Unable to allocate block to read header, errno %d",
			errno);
	    return -1;
	}
	if (type == FTT_ANSI_HEADER) {
	    ftt_to_upper(vollabel);
	}
	memset(buf,0,blocksize);
	res = ftt_read(d,buf,blocksize); 	/* errors to guess_label */
	if ( (res = ftt_guess_label(buf,res,&pname, &len) ) < 0) {
		free(buf);
		return res;
	}
	if (type != res || (len != 0 && 
		(0 != strncmp(vollabel,pname,len) || len != (int)strlen(vollabel)))){
	  if (len > 512) len = 511;
	  strncpy(label_buf,pname,len);
	  label_buf[len] = 0;
	  if (type == res) {
	    ftt_eprintf("ftt_verify_vol_label: expected vol '%s', but got '%s'.",
			vollabel, label_buf);
	    ftt_errno = FTT_EWRONGVOL;
	    retval = -1;
	  } else {
	    ftt_eprintf("ftt_verify_vol_label: expected %s header, but got %s", 
			ftt_label_type_names[type], ftt_label_type_names[res]);
	    ftt_errno = FTT_EWRONGVOLTYP;
	    retval = -1;
	  }
	}
        free(buf);
    }
    if ( retval == 0 ) { /* Check protection only if everything else if OK */
      if (0 != (status & FTT_PROT) && rdonly == FTT_RDWR) {
	ftt_eprintf("ftt_verify_vol_label: unexpected write protection");
	ftt_errno = FTT_EROFS;
	retval =  -1;
      }
      else if (0 == (status & FTT_PROT) && rdonly == FTT_RDONLY) {
	ftt_eprintf("ftt_verify_vol_label: missing expected write protection");
	ftt_errno = FTT_ERWFS;
	retval =  -1;
      }
    }
    return retval;
}

int
ftt_write_vol_label(ftt_descriptor d, int type, char *vollabel) {
    int res;
    static long int filler; /* try to force buf to be word aligned for IRIX */
    static char buf[10240]; /* biggest blocksize of any label we support */
    int blocksize = 10240;

    CKOK(d,"ftt_write_vol_label",1,2);
    CKNULL("ftt_descriptor", d);
    CKNULL("volume label", vollabel);

    res = ftt_rewind(d);			if (res <  0) return res;
    res = ftt_format_label(buf,blocksize,vollabel, strlen(vollabel), type);
						if (res <  0) return res;
    /* next highest blocksize */
    if (d->default_blocksize != 0) {
	res = res + d->default_blocksize - 1 ;
	res = res - (res % d->default_blocksize);
    }
    res = ftt_write(d,buf,res);			if (res <  0) return res;
    ftt_close_dev(d);
    res = ftt_skip_fm(d,1);
    return res;
}

char *ftt_ascii_rewindflags[] = {
	"rewind",
	"retension",
	"swab",
	"read only",
	0
};

int
ftt_describe_dev(ftt_descriptor d, char *dev, FILE *pf) {
    int i;
    int j;
    int found;
    char *starter;
    char *dname;

    ENTERING("ftt_describe_dev");
    CKNULL("ftt_descriptor", d);
    CKNULL("device name", dev);
    CKNULL("stdio file handle", pf);

    found = 0;
    starter = "\t";
    for (i = 0; d->devinfo[i].device_name !=0; i++) {
	dname = d->densitytrans[d->devinfo[i].density+1];
	if (dname == 0) {
	    dname = "unknown";
	}
	if (0 == strcmp(d->devinfo[i].device_name, dev)) {
	    if (d->devinfo[i].passthru) {
	        fprintf(pf, "%s SCSI pass-thru ", starter);
	    } else {
	        fprintf(pf, "%s %s mode(%d), %s, (0x%x), %s",
		    starter,
		    dname,
		    d->devinfo[i].density, 
		    d->devinfo[i].mode? "compressed":"uncompressed",
		    d->devinfo[i].hwdens,
		    d->devinfo[i].fixed? "fixed":"variable");
		for (j = 0; ftt_ascii_rewindflags[j] != 0; j++) {
		    if (d->devinfo[i].rewind & (1<<j)) {
			fprintf(pf, ", %s", ftt_ascii_rewindflags[j]);
		    }
		}
	    }
	    starter = " and\n\t";
	    found = 1;
	}
    }
    if (found == 0) {
	ftt_eprintf("ftt_describe_dev: device name not associated with ftt descriptor");
	ftt_errno = FTT_ENOENT;
	return -1;
    }
    fprintf(pf, "\n");
    return 0;
}

#define LAST   1
#define TOTALS 0
ftt_stat_buf *
ftt_init_stats(ftt_descriptor d){
	ftt_stat_buf *res;
	int ires;

	ENTERING("ftt_init_stats");
	PCKNULL("ftt_descriptor",d);

	res = ftt_alloc_stats();
	ires = ftt_get_stats(d,res[LAST]);
	if (ires < 0) {
	    ftt_free_stats(res);
	}
	return res;
}

ftt_statdb_buf *
ftt_init_statdb(ftt_descriptor d) {
    ftt_statdb_buf *res;
    int ires;

    ENTERING("ftt_init_statdb");
    PCKNULL("ftt_descriptor",d);

    res = ftt_alloc_statdbs();
    ires = ftt_get_statdb (d, res[0]);
    if (ires <0) {
       ftt_free_statdbs (res);
       ftt_errno = FTT_ENOMEM;
       return 0;
    }
    return res;
}
    


ftt_stat_buf *
ftt_alloc_stats() {
    ftt_stat_buf *res;

    res = malloc(sizeof(ftt_stat_buf)*2);
    if (0 == res) {
	ftt_eprintf("ftt_init_stats unable to allocate memory errno %d", errno);
	ftt_errno = FTT_ENOMEM;
	return 0;
    }
    res[LAST] = ftt_alloc_stat();
    res[TOTALS] = ftt_alloc_stat();
    return res;
}

ftt_statdb_buf *
ftt_alloc_statdbs() {
    ftt_statdb_buf *res;
    int i;

    res = malloc(sizeof(ftt_statdb_buf)*FTT_MAX_NUMDB);
    if (0 == res) {
	ftt_eprintf("ftt_init_statdbs unable to allocate memory errno %d", errno);       
	ftt_errno = FTT_ENOMEM;
        return 0;
    }
    for (i = 0; i < FTT_MAX_NUMDB; i++) {
        res[i] = ftt_alloc_statdb();
    }
    return res;
}


void
ftt_free_stats( ftt_stat_buf *res ) {
    ftt_free_stat(res[LAST]);
    ftt_free_stat(res[TOTALS]);
    free(res);
}

void
ftt_free_statdbs(ftt_statdb_buf *res) {
    int i;

    for (i = 0; i < FTT_MAX_NUMDB; i++) {
        ftt_free_statdb(res[i]);
    }
    free (res);
}

int
ftt_update_stats(ftt_descriptor d, ftt_stat_buf *bp){
	ftt_stat_buf delta, new_cur, tmp;
	int res;

        ENTERING("ftt_update_stats"); 
	CKNULL("ftt_descriptor", d);
	CKNULL("ftt_stat_buf pair pointer", bp);
	CKNULL("first ftt_stat_buf", bp[0]);
	CKNULL("second ftt_stat_buf", bp[1]);

	delta = ftt_alloc_stat(); 		if(delta == 0) return -1;
	new_cur = ftt_alloc_stat(); 		if(new_cur == 0) return -1;
	res = ftt_get_stats(d,new_cur); 	if(res < 0) return res;
	ftt_sub_stats(new_cur,bp[LAST],delta);
	if (ftt_debug > 2) {
		fprintf(stderr,"Old statistics");
		ftt_dump_stats(bp[LAST], stderr);
		fprintf(stderr,"New statistics");
		ftt_dump_stats(new_cur, stderr);
		fprintf(stderr,"delta statistics");
		ftt_dump_stats(delta, stderr);
		fprintf(stderr,"Old totals");
		ftt_dump_stats(bp[TOTALS], stderr);
	}
	ftt_add_stats(delta,bp[TOTALS],bp[TOTALS]);
	if (ftt_debug > 2){
		fprintf(stderr,"New totals");
		ftt_dump_stats(bp[TOTALS], stderr);
	}
	tmp = bp[LAST];
	bp[LAST] = new_cur;
	ftt_free_stat(tmp);
	ftt_free_stat(delta);
	return 0;
}

int
ftt_update_statdb(ftt_descriptor d, ftt_statdb_buf *bp) {
        int i, j, res;
    

        ENTERING("ftt_update_statdbs");
	CKNULL("ftt_descriptor", d);
	CKNULL("ftt_statdb_buf pointer", bp);
	CKNULL("ftt_statdb_buf pointer", bp[i]);

       for (i = (FTT_MAX_NUMDB-1); i > 0; i--) {
           for (j = 0; j < FTT_MAX_STATDB; j++) {
               bp[i]->value[j] = bp[i-1]->value[j];
           }
       }
       res = ftt_get_statdb (d,bp[0]);

}        


char *ftt_stat_names[] = {
 /* FTT_VENDOR_ID	 0 */ "FTT_VENDOR_ID",
 /* FTT_PRODUCT_ID	 1 */ "FTT_PRODUCT_ID",
 /* FTT_FIRMWARE	 2 */ "FTT_FIRMWARE",
 /* FTT_SERIAL_NUM	 3 */ "FTT_SERIAL_NUM",
 /* FTT_CLEANING_BIT	 4 */ "FTT_CLEANING_BIT",
 /* FTT_READ_COUNT	 5 */ "FTT_READ_COUNT",
 /* FTT_WRITE_COUNT	 6 */ "FTT_WRITE_COUNT",
 /* FTT_READ_ERRORS	 7 */ "FTT_READ_ERRORS",
 /* FTT_WRITE_ERRORS	 8 */ "FTT_WRITE_ERR'S",
 /* FTT_READ_COMP	 9 */ "FTT_READ_COMP",
 /* FTT_FILE_NUMBER	10 */ "FTT_FILE_NUMBER",
 /* FTT_BLOCK_NUMBER	11 */ "FTT_BLOCK_NUMBER",
 /* FTT_BOT		12 */ "FTT_BOT",
 /* FTT_READY		13 */ "FTT_READY",
 /* FTT_WRITE_PROT	14 */ "FTT_WRITE_PROT",
 /* FTT_FMK		15 */ "FTT_FMK",
 /* FTT_EOM		16 */ "FTT_EOM",
 /* FTT_PEOT		17 */ "FTT_PEOT",
 /* FTT_MEDIA_TYPE	18 */ "FTT_MEDIA_TYPE",
 /* FTT_BLOCK_SIZE	19 */ "FTT_BLOCK_SIZE",
 /* FTT_BLOCK_TOTAL	20 */ "FTT_BLOCK_TOTAL",
 /* FTT_TRANS_DENSITY	21 */ "FTT_TRANS_DENSITY",
 /* FTT_TRANS_COMPRESS	22 */ "FTT_TRANS_COMPRESS",
 /* FTT_REMAIN_TAPE	23 */ "FTT_REMAIN_TAPE",
 /* FTT_USER_READ	24 */ "FTT_USER_READ",
 /* FTT_USER_WRITE	25 */ "FTT_USER_WRITE",
 /* FTT_CONTROLLER	26 */ "FTT_CONTROLLER",
 /* FTT_DENSITY		27 */ "FTT_DENSITY",
 /* FTT_ILI		28 */ "FTT_ILI",
 /* FTT_SCSI_ASC	29 */ "FTT_SCSI_ASC",
 /* FTT_SCSI_ASCQ	30 */ "FTT_SCSI_ASCQ",
 /* FTT_PF		31 */ "FTT_PF",
 /* FTT_CLEANED_BIT     32 */ "FTT_CLEANED_BIT",
 /* FTT_WRITE_COMP	33 */ "FTT_WRITE_COMP",
 /* FTT_TRACK_RETRY	34 */ "FTT_TRACK_RETRY",
 /* FTT_UNDERRUN	35 */ "FTT_UNDERRUN",
 /* FTT_MOTION_HOURS	36 */ "FTT_MOTION_H-RS",
 /* FTT_POWER_HOURS	37 */ "FTT_POWER_HOURS",
 /* FTT_TUR_STATUS	38 */ "FTT_TUR_STATUS",
 /* FTT_BLOC_LOC	39 */ "FTT_BLOC_LOC",
 /* FTT_COUNT_ORIGIN	40 */ "FTT_COUNT_ORIGIN",
 /* FTT_N_READS		41 */ "FTT_N_READS",
 /* FTT_N_WRITES	42 */ "FTT_N_WRITES",
 /* FTT_TNP		43 */ "FTT_TNP",
 /* FTT_SENSE_KEY	44 */ "FTT_SENSE_KEY",
 /* FTT_TRANS_SENSE_KEY	45 */ "FTT_TRANS_SENSE_KEY",
 /* FTT_RETRIES		46 */ "FTT_F_RETRIES",
 /* FTT_RESETS		48 */ "FTT_RESETS",
 /* FTT_HARD_ERRORS	49 */ "FTT_HARD_ERRORS",
 /* FTT_UNC_WRITE	50 */ "FTT_UNC_WRITE",
 /* FTT_UNC_READ	51 */ "FTT_UNC_READ",
 /* FTT_CMP_WRITE	52 */ "FTT_CMP_WRITE",
 /* FTT_CMP_READ	53 */ "FTT_CMP_READ",
 /* FTT_ERROR_CODE	54 */ "FTT_ERROR_CODE",
 /* FTT_CUR_PART	55 */ "FTT_CUR_PART",
 /* FTT_MOUNT_PART	56 */ "FTT_MOUNT_PART",
 /* FTT_MAX_STAT	57 */ "FTT_MAX_STAT",
 0
};

int
ftt_dump_stats(ftt_stat_buf b, FILE *pf) {
	int i;

	ENTERING("ftt_dump_stats");
	CKNULL("statitics buffer pointer", b);
	CKNULL("stdio file handle", pf);

	for( i = 0 ; i < FTT_MAX_STAT; i++ ) {
		if(b->value[i] != 0) { 
			fprintf(pf, "%s is %s\n", 
				ftt_stat_names[i], b->value[i]);
		}
	}
	fprintf(pf, "- is -\n");
	return 0;
}

int
ftt_dump_statdb(ftt_statdb_buf b, FILE *pf) {
	int i, k = 0;

	ENTERING("ftt_dump_statdb");
	CKNULL("statitics buffer pointer", b);
	CKNULL("stdio file handle", pf);

        for (i = 0; ftt_stat_names[i] != 0; i++) {
            if (ftt_numeric_tab[i]) {
               fprintf (pf, "%s\t%s\n",ftt_stat_names[i], b->value[k]);
               k++;
            }
        }
        fprintf (pf,"- is -\n");
	return 0;
}


int
ftt_dump_statdbs(ftt_statdb_buf *b, FILE *pf) {
        int i, j, k;
    
 	ENTERING("ftt_dump_statdbs"); 
	CKNULL("statitics buffer pointer", b);
	CKNULL("stdio file handle", pf);

        for (k = 0; k <= 3; k++) {
            fprintf (pf, "%s\t %s\n",ftt_stat_names[k], b[0]->value[k]);
        }

        for (i = 4; ftt_stat_names[i] != 0; i++) {
            if (ftt_numeric_tab[i]) {
               fprintf (pf, "%s\t",ftt_stat_names[i]);
               for (j = 0; j < FTT_MAX_NUMDB; j++) {
                   fprintf (pf, "%12s\t",b[j]->value[k]);
               }
               k++;
            fprintf (pf, "\n");
            }
        }
        fprintf (pf,"- is -\n");
        return 0;
}

int 
ftt_dump_rsdata(ftt_descriptor d, FILE *pf) {
        int i;
        unsigned char buf[248];
        int lng;
        int res;

        ENTERING("ftt_dump_srdata");
	CKNULL("ftt_descriptor", d);
	CKNULL("stdio file handle", pf);

        memset(buf,0,sizeof(buf));

        static unsigned char cdb_req_sense[] = {0x03, 0x00, 0x00, 0x00, 18, 0x00};
        res = ftt_do_scsi_command(d, "Req Sense:", cdb_req_sense, 6, buf, 18, 10, 0);
        if (res < 0) {
        return -1;
        }


        fprintf(pf, "Request Sense Data\n");
        fprintf(pf, "|_____|_____|_____|_____|_____|_____|_____|_____|\n");
        fprintf(pf, "|  %x  |                   %02x                    |\n",buf[0]&0x80>>7, buf[0]&0x7f);
        fprintf(pf, "|                         %02x                    |\n",buf[1]);
        fprintf(pf, "|  %x  |  %x  |  %x  |  %x  |              %x        |\n",buf[2]&0x80>>7, buf[2]&0x40>>6, buf[2]&0x20>>5,
								               buf[2]&0x10>>4, buf[2]&0xf);
        for (i = 3; i <= 14; i++) {
            fprintf(pf, "|                         %02x                    |\n",buf[i]);
        }
        fprintf(pf, "|  %x  |  %x  |     %x     |  %x  |        %x        |\n",buf[15]&0x80>>7, buf[15]&0x40>>6, buf[15]&0x30>>4,
									       buf[15]&0x8,     buf[15]&0x7);
        for (i = 16; i <= 18; i++) {
           fprintf(pf, "|                         %02x                    |\n",buf[i]); 
        }
           lng = buf[7] + 7;
        if ((d->prod_id[1] == 'E') || (d->prod_id[2] == 'm')) {
           for (i = 19; i <= 21; i++) {
               fprintf(pf, "|  %x  |  %x  |  %x  |  %x  |  %x  |  %x  |  %x  |  %x  |\n",buf[i]&0x80>>7, buf[i]&0x40>>6, buf[i]&0x20>>5,
											 buf[i]&0x10>>4, buf[i]&0x8, buf[i]&0x4, 
											 buf[i]&0x2, buf[i]&0x1);
           }
           for (i = 22; i <= lng; i++) {
               fprintf(pf, "|                         %02x                    |\n",buf[i]);
           }
        }   
        if (d->prod_id[0] == 'U') {
           fprintf(pf, "|                         %02x                    |\n",buf[19]);
           fprintf(pf, "|                         %02x                    |\n",buf[20]);
           fprintf(pf, "|                       |  %x  |     |     |     |\n",buf[21]&0x8);
           for (i = 22; i <= lng; i++) {
               fprintf(pf, "|                         %02x                    |\n",buf[i]); 
           }
        }
        if (d->prod_id[1] == '9') {
           for (i = 19; i <= 23; i++) {
               fprintf(pf, "|                         %02x                    |\n",buf[i]);
           }
           fprintf(pf, "|          %x            |  %x  |  %x  |  %x  |  %x  |\n",buf[24]&0xf>>4, buf[24]&0x8, buf[24]&0x4,
										 buf[24]&0x2, buf[24]&0x1);
           fprintf(pf, "|  %x  |    %x      |  %x  |  %x  |  %x  |  %x  |  %x  |\n",buf[25]&0x80>>7, buf[25]&0x60>>5, buf[25]&0x10>>4, 
										buf[25]&0x8, buf[25]&0x4, buf[25]&0x2, buf[25]&0x1);
        }
        if (d->prod_id[0] == 'D') {
           for (i = 19; i <= lng; i++) {
               fprintf(pf, "|                         %02x                    |\n",buf[i]);
           }
        }

          

        fprintf (pf, "|_______________________________________________|\n");
        
        return 0;
}

int
ftt_undump_stats(ftt_stat_buf b, FILE *pf) {
	int i;
	static char name[512], value[512];

	ENTERING("ftt_undump_stats");
	CKNULL("statitics buffer pointer", b);
	CKNULL("stdio file handle", pf);

	/* note that this only works 'cause we know what
	** order the entries were printed in by dump_stats.
	** therefore the next item on the input has to be
	** one of the upcoming entries in the table.
	** so we go through all the stats, and if the
	** line we have read in is that stat we set it
	** and get the next line.
	*/
	fscanf(pf, "%s is %[^\n]\n", name, value);
	for( i = 0 ; i < FTT_MAX_STAT; i++ ) {
	    if (0 != b->value[i]) {
		free(b->value[i]);
		b->value[i] = 0;
	    }
	    if (0 == strcmp(name,ftt_stat_names[i])) {
		b->value[i] = strdup(value);
		fscanf(pf, "%s is %[^\n]\n", name, value);
	    }
	}
	return 0;
}

static char namebuf[512];

void
ftt_first_supported(int *pi) {
	*pi = 0;
	return;
}

ftt_descriptor
ftt_next_supported(int *pi) {
	ftt_descriptor res;
	if(devtable[*pi].os == 0) {
		return 0;
	}
	/* handle %s case... */
	if (0 == strncmp(devtable[*pi].baseconv_out,"%s", 2)) {
	    sprintf(namebuf, devtable[*pi].baseconv_out, "xxx" , 0);
	} else {
	    sprintf(namebuf, devtable[*pi].baseconv_out, 0, 0);
	}
	res = ftt_open_logical(namebuf,devtable[*pi].os,devtable[*pi].drivid,0);
	(*pi)++;
	return res;
}
int
ftt_list_supported(FILE *pf) {
    ftt_descriptor d;
    char *last_os, *last_prod_id, *last_controller;
    int i, dens; 
    int flags;

    last_os = strdup("-");
    last_prod_id = strdup("-"); 
    last_controller = strdup("-"); 
    for(ftt_first_supported(&i); (d = ftt_next_supported(&i) ); ) {
	for( dens = 20; dens > -1; dens-- ) {
	    flags = 0;

	    if (0 != ftt_avail_mode(d, dens, 0, 0)) {
		flags |= 1;
	    }
	    if (0 != ftt_avail_mode(d, dens, 0, 1)) {
		flags |= 2;
	    }
	    if (0 != ftt_avail_mode(d, dens, 1, 0)) {
		flags |= 4;
	    }
	    if (0 != ftt_avail_mode(d, dens, 1, 1)) {
		flags |= 8;
	    }

	    if (flags == 0) {
		continue;
	    }

	    /* now print a line based on the flags */

	    /* only print OS if different */
	    if (0 != strcmp(last_os, d->os)) {
		fprintf(pf, "\n");
		fprintf(pf, "OS\tCNTRLR\tDEVICE\t\tCOMP\tBLOCK\tMODE\n");
		fprintf(pf, "--\t------\t------\t\t----\t-----\t----\n");
		fprintf(pf, "%s\t", d->os);
	    } else {
		fprintf(pf, "\t");
	    }

	    /* only print controller if different */
	    if (0 != d->controller && ( 0 != strcmp(last_controller, d->controller) || 0 != strcmp(last_os,d->os) )) {
	        fprintf(pf,"%s\t", d->controller);
	    } else {
		fprintf(pf, "\t");
	    }

	    /* only print prod_id if different */
	    if (0 != d->prod_id && ( 0 != strcmp(last_prod_id, d->prod_id) 
				     || 0 != strcmp(last_controller,d->controller) 
				     || 0 != strcmp(last_os,d->os) ) ) {
		if( strlen(d->prod_id) > 7 ) {
		    fprintf(pf, "%s\t", d->prod_id);
		} else if (strlen(d->prod_id) > 0 ) {
		    fprintf(pf, "%s\t\t", d->prod_id);
		} else {
		    fprintf(pf, "(unknown)\t");
		}
		free(last_os);
		free(last_prod_id);
		free(last_controller);
		last_os = strdup(d->os);
		last_prod_id = strdup(d->prod_id);
		last_controller = strdup(d->controller);
	    } else {
		fprintf(pf, "\t\t");
	    }


	    if ( (flags & 12) && (flags & 3) ) { /* compression (8 | 4) and not compression (1|2) */
		fprintf(pf, "y/n\t");
	    } else if ( (flags & 12)) { /* compression (8 | 4) */
		fprintf(pf, "y\t");
	    } else {
		fprintf(pf, "n\t");
	    }

	    if ( (flags & 10) && (flags & 5) ) { /* fixed block and variable */
		fprintf(pf,"f/v\t");
	    } else if ( flags & 10 ) {
		fprintf(pf,"f\t");
	    } else {
		fprintf(pf,"v\t");
	    }
	    fprintf(pf, "%s mode\n", ftt_density_to_name(d, dens));

	}
	ftt_close(d);
    }
    return 0;
}

int
ftt_retry( ftt_descriptor d, int  n, int (*op)(ftt_descriptor, char *, int),
		char *buf, int len) {
    int curfile, curblock;
    int res;

    ENTERING("ftt_retry");
    CKNULL("ftt_descriptor", d);
    CKNULL("operation", op);

    res = ftt_get_position(d, &curfile, &curblock); 	if (res<0) return res;

    res = (*op)(d, buf, len);

    /* eblank is the end of data error, so don't retry it */

    while( res < 0 && ftt_errno != FTT_EBLANK && n-- > 0 ) {
	d->nretries++;
	/* recover position -- skip back over filemark and forward again */
 	res = ftt_skip_fm(d, -1);   			if (res<0) return res;
 	res = ftt_skip_fm(d, 1);    			if (res<0) return res;
	res = ftt_skip_rec(d, curblock); 		if (res<0) return res;

        res = (*op)(d, buf, len);
    }
    if (res < 0) {
	d->nfailretries++;
    }
    return res;
}

/*
** allow us to forge on ahead. -- dangerous!
*/
int 
ftt_clear_unrecovered(ftt_descriptor d) {
	ENTERING("ftt_clear_unrecovered");
	CKNULL("ftt_descriptor", d);

	d->unrecovered_error = 0;
	return 0;
}

/*
** allow us to forgo filemarks, etc. -- dangerous!
*/
int 
ftt_clear_write_state(ftt_descriptor d) {
	ENTERING("ftt_clear_unrecovered");
	CKNULL("ftt_descriptor", d);

	d->last_operation = 0;
	return 0;
}

int
ftt_verify_blank(ftt_descriptor d) {
    int max;
    char *buffer;
    int res;
    ENTERING("ftt_verify_blank");
    CKNULL("ftt_descriptor", d);

    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {
        ftt_close_dev(d);
        switch(ftt_fork(d)){
        case -1:
                return -1;

        case 0:  /* child */
                fflush(stdout); /* make async_pf stdout */
                fflush(d->async_pf_parent);
                close(1);
                dup2(fileno(d->async_pf_parent),1);
		if (ftt_debug) {
		    execlp("ftt_suid", "ftt_suid", "-x", "-v", d->basename,
0);
		} else {
		    execlp("ftt_suid", "ftt_suid", "-v", d->basename, 0);
		}

        default: /* parent */
                return ftt_wait(d);
        }
    }

    max = ftt_get_max_blocksize(d);

    buffer = malloc(max);
    if (0 == buffer) {
        fprintf(stderr, "unable to allocate buffer for copy, errno %d", errno);
        return 0;
    }
 
    d->scsi_ops |= FTT_OP_READ;
    ftt_rewind(d);
    res = ftt_read(d,buffer,max);
    d->scsi_ops &= ~FTT_OP_READ;
    
    return res;
}
                                                                                               home/vsergeev/ftt/ftt_lib/ftt_info.c                                                                0100660 0022366 0022366 00000017524 06443574423 020325  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ftt_private.h>

#ifndef WIN32
#include <unistd.h>
#endif

extern int errno;
 
int 
ftt_get_data_direction(ftt_descriptor d) {
    ENTERING("ftt_get_data_direction");
    PCKNULL("ftt_descriptor", d);

    return d->data_direction;
}

char *
ftt_get_basename(ftt_descriptor d) {
    
    ENTERING("ftt_get_basename");
    PCKNULL("ftt_descriptor", d);

    return d->basename;
}

char *
ftt_density_to_name(ftt_descriptor d, int density){
    char *res;

    ENTERING("ftt_density_to_name");
    if (density + 1 < MAX_TRANS_DENSITY ) {
	res = d->densitytrans[density + 1];
    } else {
	res = 0;
    }
    if ( 0 == res ) {
	res = "unknown";
    }
    return res;
}

int
ftt_name_to_density(ftt_descriptor d, const char *name){
    int res;

    ENTERING("ftt_name_to_density");
    CKNULL("density name", name);

    for (res = 0; d->densitytrans[res] != 0; res++) {
	if( ftt_matches(name, d->densitytrans[res])) {
	    return res - 1;
	}
    }
    ftt_errno = FTT_ENODEV;
    ftt_eprintf(
       "ftt_name_to_density: name %s is not appropriate for device %s\n",
       name,
       d->basename );
    return -1;
}

int
ftt_get_max_blocksize(ftt_descriptor d) {
    int result;

    ENTERING("ftt_get_max_blocksize");
    CKNULL("ftt_descriptor", d);

    result = d->devinfo[d->which_is_default].max_blocksize;

    /* round down to nearest fixed blocksize */
    if (d->default_blocksize != 0) {
        result = result - (result % d->default_blocksize);
    }

    return result;
}

char **
ftt_list_all(ftt_descriptor d) {
    static char *table[MAXDEVSLOTS];
    int i,j;

    ENTERING("ftt_list_all");
    PCKNULL("ftt_descriptor", d);

    for( i = 0,j = 0; j <= MAXDEVSLOTS  && d->devinfo[i].device_name != 0; i++ ){
	if (d->devinfo[i].first) {
	    table[j++] = d->devinfo[i].device_name;
	}
    }
    table[j++] = 0;
    return table;
}

int
ftt_chall(ftt_descriptor d, int uid, int gid, int mode) {
    static struct stat sbuf;
    char **pp;
    int res, rres;
    int i;

    ENTERING("ftt_chall");
    CKNULL("ftt_descriptor", d);

    rres = 0;
    pp = ftt_list_all(d);
    /* 
     * Do the stat on each device file if file doesn't exist
     * skip it but rport other errors 
     */
    for( i = 0; pp[i] != 0; i++){
        res = stat(pp[i],&sbuf);
	if (res < 0) {
	    if ( errno == ENOENT ) {
		continue; 
	    } else {
		rres = ftt_translate_error(d,FTT_OPN_CHALL,"ftt_chall",res,
						"stat",1);
		continue;
	    }
	}
#ifndef WIN32
	res = chmod(pp[i],mode);
	if (res < 0) {
            rres = ftt_translate_error(d,FTT_OPN_CHALL,"ftt_chall",res,"chmod",1);
	}
	res = chown(pp[i],uid,gid);
	if (res < 0) {
            rres = ftt_translate_error(d,FTT_OPN_CHALL,"ftt_chall",res,"chown",1);
	}
#endif
    }
    return rres;
}

static char *comptable[] = {"uncompressed", "compressed"};
char *
ftt_avail_mode(ftt_descriptor d, int density, int mode, int fixedblock) {
    int i;
    char *dname;

    ENTERING("ftt_avail_mode");
    PCKNULL("ftt_descriptor", d);
    
    for( i = 0; d->devinfo[i].device_name != 0; i++ ){
	if( d->devinfo[i].density == density &&
		    d->devinfo[i].mode == mode &&
		    d->devinfo[i].rewind == 0 &&
		    d->devinfo[i].fixed  == fixedblock) {
	    return d->devinfo[i].device_name;
	}
    }
    dname = ftt_density_to_name(d, density);
    ftt_eprintf("ftt_avail_mode: mode %s(%d) density %s(%d) %s is not avaliable on device %s", 
	    comptable[mode], mode, 
	    dname, density, 
	    fixedblock?"fixed block" : "variable block", d->basename);
    ftt_errno = FTT_ENODEV;
    return 0;
}

char *
ftt_get_mode(ftt_descriptor d, int *density, int* mode, int *blocksize){

    ENTERING("ftt_get_mode");
    PCKNULL("ftt_descriptor", d);

    if (density) *density = d->devinfo[d->which_is_default].density;
    if (mode)    *mode = d->devinfo[d->which_is_default].mode;
    if (blocksize) *blocksize = d->devinfo[d->which_is_default].fixed ? 
		    d->default_blocksize : 0;
    return d->devinfo[d->which_is_default].device_name;
}
char *
ftt_set_mode(ftt_descriptor d, int density, int mode, int blocksize) {
    int i;
    char *dname;

    ENTERING("ftt_set_mode");
    PCKNULL("ftt_descriptor", d);
    
    ftt_close_dev(d);
    d->density_is_set = 0;
    for( i = 0; d->devinfo[i].device_name != 0; i++ ){
	if (d->devinfo[i].density == density &&
		    d->devinfo[i].mode == mode &&
		    (d->devinfo[i].fixed == 0) == (blocksize == 0) && 
		    d->devinfo[i].rewind == 0) {

	    /* clear flag if we are switching density */

	    if (d->devinfo[i].hwdens != d->devinfo[d->which_is_default].hwdens){
		d->density_is_set = 0;
	    }

	    d->which_is_default = i;
	    d->default_blocksize = blocksize;
	    return d->devinfo[i].device_name;
	}
    }
    dname = ftt_density_to_name(d, density);
    ftt_eprintf("ftt_set_mode: mode %s(%d) density %s(%d) blocksize %d is not avaliable on device %s", 
	    comptable[mode], mode, 
	    dname , density, 
		blocksize, d->basename);
    ftt_errno = FTT_ENODEV;
    return 0;
}

int 
ftt_get_mode_dev(ftt_descriptor d, char *devname, int *density, 
			int *mode, int *blocksize, int *rewind) {
    int i;
    int hwdens;
    int found;

    ENTERING("ftt_get_mode_dev");
    CKNULL("ftt_descriptor", d);
    
    hwdens = ftt_get_hwdens(d,devname);
    for( i = 0; d->devinfo[i].device_name != 0; i++ ) {
	if (0 == strcmp(d->devinfo[i].device_name, devname)){
	    found = 1;
	    if (density)   *density = d->devinfo[i].density;
	    if (mode)      *mode = d->devinfo[i].mode;
	    if (blocksize) *blocksize =  d->devinfo[i].fixed;
	    if (rewind)    *rewind = d->devinfo[i].rewind;

	    if (d->devinfo[i].hwdens == hwdens) {
		/* hardware density match is a better match */
		/* otherwise keep looking */
		break; 
	    }
	}
    }
    if (found) {
	return 0;
    } else {
	ftt_eprintf("ftt_get_mode_dev: device name %s was not found in the ftt tables for basename %s\n",
	    devname, d->basename);
	ftt_errno = FTT_ENODEV;
	return -1;
    }
}

int 
ftt_set_mode_dev(ftt_descriptor d, const char *devname, int force) {
    int i;

    ENTERING("ftt_set_mode_dev");
    CKNULL("ftt_descriptor", d);
    CKNULL("device name", devname);
    
    for( i = 0; d->devinfo[i].device_name != 0; i++ ){
	if (0 == strcmp(d->devinfo[i].device_name, devname)) {
	    d->which_is_default = i;
	    if ( d->devinfo[i].fixed == 0) {
			d->default_blocksize = 0;
		} else {
			d->default_blocksize = -1;
		}
	    return 0;
	}
    }
    if (force) { 

	/* not found in table, but force bit was set... */

        if (i >= MAXDEVSLOTS - 1){
	    /* there isn't room in the table for it */

	    ftt_errno = FTT_ENOMEM;
	    ftt_eprintf("ftt_set_mode_dev: tried to add a new device entry to the table when there was not room for it");
	    return -1;
	}
	/* clear flag if we are switching density */
	if (d->devinfo[i].hwdens != d->devinfo[d->which_is_default].hwdens) {
	    d->density_is_set = 0;
	}
	/* so add it to the table */
     
	d->devinfo[i].device_name = strdup(devname);
	d->which_is_default = i;

	/* and we know/set nothing ... */
	d->devinfo[i].mode = -1;
	d->devinfo[i].density = -1;
	d->devinfo[i].fixed = -1;
	d->default_blocksize = -1; 

	/* make sure sentinel null is in table */
	d->devinfo[i+1].device_name = 0;

	return 0;
    }
    ftt_eprintf("ftt_set_mode_dev: device name %s was not found in the ftt tables for basename %s and the force bit was not set.",
	devname, d->basename);
    ftt_errno = FTT_ENODEV;
    return -1;
}

int
ftt_set_data_direction( ftt_descriptor d, int value ) {
    ENTERING("ftt_set_data_direction");
    CKNULL("ftt_descriptor", d);
    if (value != FTT_DIR_READING && value != FTT_DIR_WRITING ) {
	ftt_errno = FTT_ENXIO;
	ftt_eprintf("ftt_set_data_direction: an invalid value of %d was given for the data direction.", value);
	return -1;
    }
    d->data_direction = value;
    return 0;
}
                                                                                                                                                                            home/vsergeev/ftt/ftt_lib/ftt_label.c                                                               0100660 0022366 0022366 00000012656 06532622306 020443  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include "ftt_private.h"
#include <string.h>

#define pack(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

int ftt_format_label_version( char *, int, char *, int, int, char );

char *ftt_label_type_names[] = {
    /* FTT_ANSI_HEADER         0 */ "FTT_ANSI_HEADER",
    /* FTT_FMB_HEADER          1 */ "FTT_FMB_HEADER",
    /* FTT_TAR_HEADER          2 */ "FTT_TAR_HEADER",
    /* FTT_CPIO_HEADER         3 */ "FTT_CPIO_HEADER",
    /* FTT_UNKNOWN_HEADER      4 */ "FTT_UNKNOWN_HEADER",
    /* FTT_BLANK_HEADER        5 */ "FTT_BLANK_HEADER",
    /* FTT_DONTCHECK_HEADER    6 */ "FTT_DONTCHECK_HEADER",
    /* FTT_MAX_HEADER	       7 */ "FTT_MAX_HEADER", 
};

int
ftt_guess_label(char *buf, int length, char **vol, int *vlen) {
    char *p;
    
    /* don't clear errors yet, need to look at ftt_errno */

    char *_name = "ftt_guess_label";
    DEBUG1(stderr, "Entering %s\n", _name);
    CKNULL("label data buffer pointer", buf);

    if (-1 == length && ftt_errno == FTT_EBLANK) {
	/* read returned EBLANK... */
	ftt_eprintf("Ok\n");
	if (vol) *vol = "";
	if (vlen) *vlen = 0;
	ftt_errno = FTT_SUCCESS;
	return FTT_BLANK_HEADER;
    } else if ( -1 == length ) {
	return -1;
    } else if ( length < 80 ) {
	/* no known header is < 80 bytes long */
	ftt_eprintf("Ok\n");
	if (vol) *vol = "";
	if (vlen) *vlen = 0;
	ftt_errno = FTT_SUCCESS;
	return FTT_UNKNOWN_HEADER;
    }

    /* okay, now we can clear errors... */

    ftt_eprintf("Ok\n");
    ftt_errno = FTT_SUCCESS;

    /* pick the ones we can with the first 4 bytes */

    switch(pack(buf[0],buf[1],buf[2],buf[3])) {

    case pack('V','O','L','1'):
	if (vol) *vol = buf+4;
	/* trim blanks -- loop has to stop at least when it hits the '1' */
	p = buf+10;
	while (' ' == *p) {
	    p--;
	}
	if (vlen) *vlen = (p - (buf + 4)) + 1;
	return FTT_ANSI_HEADER;

    case pack('0','7','0','7'):
	if (vol)  *vol = buf + 0156;
	if (vlen) *vlen = strlen(*vol);
	return FTT_CPIO_HEADER;
    }

    /* check for a tar header */

    if (pack('u','s','t','a')==pack(buf[0401],buf[0402],buf[0403],buf[0404])) {
	if (vol) *vol = buf;
	if (vlen) *vlen = strlen(*vol);
	return FTT_TAR_HEADER;
    }

    /* check for an fmb header -- newline separated ascii */

    p = strchr(buf,'\n');
    if (0 != p && (length % 1024 == 0)) {
	if (vol) *vol = buf;
	if (vlen) *vlen = p - buf;
	return FTT_FMB_HEADER;
    }

    /* if all else failed, we don't know what it was... */
    if (vol) *vol = "";
    if (vlen) *vlen = 0;
    return FTT_UNKNOWN_HEADER;
}

void
ftt_to_upper( char *p ) {
   int i = 0;
   while( p[i] ) {
	p[i] = p[i] >= 'a' && p[i] <= 'z' ? p[i]-'a'+'A' : p[i];
	i++;
   }
}

int
ftt_format_label( char *buf, int length, char *vol, int vlen, int type) {
   return ftt_format_label_version(buf, length, vol, vlen, type, 0);
}

int
ftt_format_label_version( char *buf, int length, char *vol, int vlen, int type, char version) {

#define BLEN 512
    static char volbuf[BLEN];
    ENTERING("ftt_format_label");
    CKNULL("label buffer pointer", buf);
    
    if (vlen >= BLEN) {
	ftt_eprintf("volume label too long; maximum is %d", BLEN-1);
	ftt_errno = FTT_EFAULT;
	return -1;
    }
    memcpy( volbuf, vol, vlen );
    volbuf[vlen] = 0;

    switch(type) {
    case FTT_ANSI_HEADER:
	if ( version == 0 ) { /* default is 4 */
	    version = '4';
	}
	ftt_to_upper(volbuf);
	if (length >= 80) {
	    sprintf(buf, "VOL1%-6.6s%-1.1s%-13.13s%-13.13s%-14.14s%-28.28s%-1.1d", 
				volbuf, " ", " ", "ftt", " ", " " , version);
	    return 80;
	 } else {
	    ftt_errno = FTT_EBLKSIZE;
	    ftt_eprintf("ftt_format_label: the buffer size of %d is too small for the indicated header type.");
	    return -1;
	}
	break;

    case FTT_FMB_HEADER:
	if (length >= 2048) {
	    sprintf(buf, "%s\n%s\n%s\n%s\n",
			volbuf, "never", "cpio", "16k");
	    return 2048;
	 } else {
	    ftt_errno = FTT_EBLKSIZE;
	    ftt_eprintf("ftt_format_label: the buffer size of %d is too small for the indicated header type.");
	    return -1;
	}
	break;
    case FTT_CPIO_HEADER:
	 if (length >= 512) {
	     memset(buf, 0, (size_t)512); 
	     sprintf(buf, "070701000086f6000081a4000006c5000011ad0000000130f68764000000000000001e0000000500000000000000000000000a00000000%s", volbuf);
	     sprintf(buf + strlen(buf) +1 , "0007070100000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000b00000000TRAILER!!!");
	     return 512;
	 } else {
	    ftt_errno = FTT_EBLKSIZE;
	    ftt_eprintf("ftt_format_label: the buffer size of %d is too small for the indicated header type.");
	    return -1;
	 }
	 break;
    case FTT_TAR_HEADER:
	 if (length >= 10240) {
	     memset(buf, 0, (size_t)10240); 
	     sprintf(buf,     "%s", volbuf);
	     sprintf(buf+0144,"000644 ");
	     sprintf(buf+0154,"003305 ");
	     sprintf(buf+0164,"00000000000 06075503544 014150");
	     sprintf(buf+0232, " 0");
	     sprintf(buf+0401, "ustar");
	     sprintf(buf+0410, "00%s", "nobody");
	     sprintf(buf+0451, "00%s", "other");
	     return 10240;
	 } else {
	    ftt_errno = FTT_EBLKSIZE;
	    ftt_eprintf("ftt_format_label: the buffer size of %d is too small for the indicated header type.");
	    return -1;
	 }
    }

    ftt_errno = FTT_ENOTSUPPORTED;
    if ( type < FTT_MAX_HEADER ) {
      ftt_eprintf("ftt_format_label: unsupported label type %s\n",
		ftt_label_type_names[type]);
    } else{
      ftt_eprintf("ftt_format_label: unsupported label type %d\n",
		type);
    }
    return -1;
}
                                                                                  home/vsergeev/ftt/ftt_lib/ftt_main.c                                                                0100660 0022366 0022366 00000001006 06252127464 020277  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <ftt.h>

ftt_descriptor d;

char buf[128];

main() {
	ftt_debug = 3;
	d = ftt_open("/dev/bogus", 1);
	printf("result %s\n", ftt_get_error(0));
	d = ftt_open("/dev/rmt/jag1d10nrv.8500", 1);
	printf("result %s\n", ftt_get_error(0));
	if (d  == 0) {
		exit(1);
	}
	ftt_read(d,buf,128);
	printf("result %s\n", ftt_get_error(0));
	ftt_read(d,buf,128);
	printf("result %s\n", ftt_get_error(0));
	ftt_close(d);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          home/vsergeev/ftt/ftt_lib/ftt_open.c                                                                0100660 0022366 0022366 00000036515 07246516330 020327  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
extern char ftt_version[];
static char *rcslink = ftt_version;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ftt_private.h>

extern int errno;

#ifdef WIN32
#include <io.h>
#include <windows.h>
#include <winioctl.h>

int ftt_translate_error_WIN();

#else
#include <ctype.h>
#include <sys/file.h>
#include <unistd.h>
#endif

#ifndef FNONBLOCK
# define FNONBLOCK      O_NONBLOCK
#endif

ftt_descriptor
ftt_open(const char *name, int rdonly) {
    static char alignname[512];
    char *basename;
    char *os, *drivid;
    ftt_descriptor res;
    
    ENTERING("ftt_open");
    PCKNULL("base name", name);
    
    DEBUG2(stderr, "ftt_open( %s, %d )\n", name, rdonly);
    strcpy(alignname, name);
    os=ftt_get_os();
    DEBUG2(stderr,"os is %s\n", os);
    if( 0 == os ){
	ftt_eprintf("ftt_open: unable to determine operating system type");
	ftt_errno=FTT_ENOTSUPPORTED;
	return 0;
    }

    basename=ftt_strip_to_basename(alignname, os);
    DEBUG2(stderr,"basename is %s\n", basename);
    if ( basename == 0 ) {
	ftt_eprintf("ftt_open: unable to determine drive basename.\n");
	ftt_errno=FTT_ENOTSUPPORTED;
	return 0;
    }

    drivid=ftt_get_driveid(basename, os);
    DEBUG2(stderr,"drivid is %s\n", drivid);
    if( 0 == drivid ){
	ftt_eprintf("ftt_open: Warning unable to determine tape drive type.\n");
	drivid=strdup("unknown");
    }

    res = ftt_open_logical(basename, os, drivid, rdonly);
    free(basename);
    return res;
}

/*
** lookup a real name for drives with ailases
*/

char *
ftt_unalias( const char *s1 ) {
    extern ftt_id_alias ftt_alias_table[];
    ftt_id_alias *p;

    p = ftt_alias_table;
    while( p->alias && !ftt_matches( p->alias, s1 )) {
       p++;
    }
    if( p->real ) {
       return p->real;
    } else {
       return s1;
    }
}

/* prefix match comparator -- returns true if either string is
**		a prefix of the other
*/
int
ftt_matches( const char *s1, const char *s2 ) {
    DEBUG4(stderr, "Matching '%s' against '%s'\n", s1, s2);
    while( 0 != *s1 && 0 != *s2 && tolower(*s1) == tolower(*s2)){
        s1++;
        s2++;
    }
    DEBUG4(stderr, "Returning %d\n", *s1 == 0 || *s2 == 0);
    return *s1 == 0 || *s2 == 0;
}

/*
** ftt_open_logical -- create a descriptor table
*/
ftt_descriptor
ftt_open_logical(const char *name, char *os, char *drivid, int rdonly) {
    static char buf[512];
    static union { int n; char s[512];} s1, s2, s3;
    static ftt_descriptor_buf d;
    char *basename;
    int i,j;
    ftt_descriptor pd;
    char *lastpart;

    /* find device type and os in table */

    ENTERING("ftt_open_logical");
    PCKNULL("base name",name);
    PCKNULL("operating system name", os);
    PCKNULL("drive id prefix", drivid);

    basename = ftt_strip_to_basename(name, os);
    if ( basename == 0 ) {
	ftt_eprintf("ftt_open_logical: unable to determine drive basename.\n");
	ftt_errno=FTT_ENOTSUPPORTED;
	return 0;
    }

    /* look up in table, note that table order counts! */
    drivid = ftt_unalias(drivid);
    i = ftt_findslot(basename, os, drivid, &s1, &s2, &s3);

    DEBUG3(stderr, "Picked entry %d number %d\n", i, s2.n);

    /* if it wasn't found, it's not supported */

    if ( i < 0 ) {
        DEBUG3(stderr, "Unsupported...\n");
	ftt_eprintf("ftt_open_logical: device type %s on platform %s unsupported\n", drivid, os);
	ftt_errno=FTT_ENOTSUPPORTED;
	return 0;
    }

    /* string together device names and flags into our descriptor */

    d.controller = devtable[i].controller;
    d.current_blocksize = -1;
    d.which_is_default = 0;
    d.which_is_open = -1;
    d.scsi_descriptor = -1;
    d.readonly = rdonly;
    d.scsi_ops = devtable[i].scsi_ops;
    d.flags = devtable[i].flags;
    d.errortrans = devtable[i].errortrans;
    d.densitytrans = devtable[i].densitytrans;
    d.basename = basename;
    d.prod_id = strdup(drivid);
    d.os = devtable[i].os;
    d.last_pos = -1;
    d.nretries = 0;
    d.nfailretries = 0;
    d.nresets = 0;
    d.nharderrors = 0;

    if( 0 == d.prod_id ) {
	ftt_eprintf("ftt_open_logical: out of memory allocating string for \"%s\" errno %d" , drivid, errno);
	ftt_errno = FTT_ENOMEM;
	return 0;
    }

    /*
    ** the tables only deal with the last 2 components of the path
    ** (that is the last directory and the filename compnent)
    ** [The last 2 components 'cause we turn /dev/rmt/xxx into /dev/scsi/xxx
    ** sometimes.]
    */
    strcpy(buf, basename);
    lastpart = ftt_find_last_part(buf);

    for( j = 0; devtable[i].devs[j].device_name != 0; j++ ) {
	/*
	** first item in the format can be either a string or a digit;
	** check for strings -- "%s..."
	** this ought to be more generic, but for now it's okay -- mengel
	*/
	if ( devtable[i].devs[j].device_name[1] == 's') {
            sprintf(lastpart, devtable[i].devs[j].device_name, s1.s, s2.n,s3.n);
	} else {
            sprintf(lastpart, devtable[i].devs[j].device_name, s1.n, s2.n,s3.n);
	}

	d.devinfo[j].device_name = strdup(buf);

	if( 0 == d.devinfo[j].device_name ) {
	    ftt_eprintf("fft_open_logical: out of memory allocating string for \"%s\" errno %d" , buf, errno);
	    ftt_errno = FTT_ENOMEM;
	    return 0;
	}
	d.devinfo[j].density = devtable[i].devs[j].density;
	d.devinfo[j].mode    = devtable[i].devs[j].mode;
	d.devinfo[j].hwdens  = devtable[i].devs[j].hwdens;
	d.devinfo[j].rewind  = devtable[i].devs[j].rewind;
	d.devinfo[j].fixed   = devtable[i].devs[j].fixed;
	d.devinfo[j].passthru= devtable[i].devs[j].passthru;
	d.devinfo[j].first   = devtable[i].devs[j].first;
        d.devinfo[j].max_blocksize = devtable[i].devs[j].max_blocksize;
    }
    d.devinfo[j].device_name = 0;

    pd = malloc(sizeof(ftt_descriptor_buf));
    if (pd == 0) {
	ftt_eprintf("ftt_open_logical: out of memory allocating descriptor, errno %d", errno);
	ftt_errno = FTT_ENOMEM;
	return 0;
    }
    memcpy(pd, &d, sizeof(d));
    return pd;
}

int
ftt_close(ftt_descriptor d){
    int j;
    int res;

    ENTERING("ftt_close");
    CKNULL("ftt_descriptor", d);

    /* valiant attempt at idiot proofing
    **
    ** When we close the descriptor, we shove a -3
    ** in the which_is_open field, which should never happen
    ** in normal operation.
    ** 
    ** if we see the -3 here, someone is trying to close us
    ** twice in a row...
    */
    if (d->which_is_open == -3) {
	ftt_errno = FTT_EFAULT;
	ftt_eprintf("ftt_close: called twice on the same descriptor!\n");
	return -1;
    }
    res = ftt_close_dev(d);
    for(j = 0; 0 != d->devinfo[j].device_name ; j++ ) {
	free(d->devinfo[j].device_name);
	d->devinfo[j].device_name = 0;
    }
    if (d->basename) {
	free(d->basename);
	d->basename = 0;
    } if (d->prod_id) {
	free(d->prod_id);
	d->prod_id = 0;
    }
    d->which_is_open = -3;
    free(d);
    return res;
}
/* This is internal function to make ftt_open_dev shorter and clear */
static int
ftt_open_status (ftt_descriptor d ) {
	int status_res = 0;
/*
	** it looks like we should just do a readonly open if we're a 
	** read-only, descriptor and a read/write open if we're read/write 
	** descriptor.  
	**
	** Unfortunately on some platforms a read/write open on a write 
	** protected tape will fail.  So to make it behave the same 
	** everywhere, if we are opening read/write, we first make it 
	** readonly, and check for write protection.  We let ftt_status
	** call us recursively if it needs the device open; we won't
	** recurse infinitely 'cause the recursive call will be readonly.
	** we dont go ahead and open it readonly 'cause status may need
	** the scsi device open instead.
	**
	** Also, we need to set density if the drive is opened read/write
	** and it is a different density than we currently have.  This
	** needs to fail if we're not at BOT.  If we're readonly, drives
	** will autosense, so we ignore the density we've been given.
	**
	** The more disgusting qualities of the following are due to
	** the fact that 
	** 1) changing density in mid-tape doesn't work
	** 2) setting densities on AIX causes the next
	**    open to rewind (thats right, the next *open*),
	**    even if you are setting it to the same density.
	**    therefore we go to great lengths to make sure
	**    we only change density if we need to, at BOT
	**    when we are read/write...
	*/
	if (d->readonly == FTT_RDWR ) {

			d->readonly = FTT_RDONLY;

	    /* note that this will lead to either a 1-deep recursive call
	       (which can't get here 'cause it is now read-only) to open 
	       the regular device, *or* a scsi open to get status that way */

		    status_res = ftt_status(d,0);
			DEBUG3(stderr,"ftt_status returned %d\n", status_res);

		/* close dev and scsi dev in case ftt_status used them... */
			ftt_close_dev(d);
	
		/* put back readonly flag */
			d->readonly = FTT_RDWR;


	    /* coimplain if neccesary */
			if (status_res > 0 && (status_res & FTT_PROT)) {
				ftt_errno = FTT_EROFS;
				ftt_eprintf("ftt_open_dev: called with a read/write ftt_descriptor and a write protected tape.");
				return -1;
			}
	    
		} /* end taking status */
	return status_res;
}
/* this is the internal function to make ftt_open_dev shorter and clear */
static int 
ftt_open_set_blocksize (ftt_descriptor d) {
	int res = 0;
	if (-1 != d->default_blocksize || d->default_blocksize != d->current_blocksize ) {
	    res = ftt_set_blocksize(d, d->default_blocksize);
	    if (res < 0) {
	       return res;
	    } 
		
	    d->current_blocksize = d->default_blocksize;
	}
	return 0;
}
/* this is the internal function to make ftt_open_dev shorter and clear */
static int 
ftt_open_set_mode (ftt_descriptor d,int status_res) {
	int res = 0;
	/*
	 * set density *regardless* of read/write, it may matter 
	 * mainly for OCS, who may be doing ocs_setdev before doing
	 * a mount -- the tape we have may be readonly, etc. but we
	 * may be setting it for the *next* tape
	 */
	if (d->flags & FTT_FLAG_NO_DENSITY) {
	     /* pretend we already did it ... */
	     d->density_is_set = 1;
	}

	if (!d->density_is_set) {
	    if (d->scsi_ops && FTT_OP_SETCOMPRESSION ) {
	        res = ftt_scsi_set_compression(d,d->devinfo[d->which_is_default].mode);
	    } else {
	        res = ftt_set_compression(d,d->devinfo[d->which_is_default].mode);
	    }
	    if (res < 0) {
			return res;
	    }
	    if (ftt_get_hwdens(d,d->devinfo[d->which_is_default].device_name) 
				!= d->devinfo[d->which_is_default].hwdens) {
			if ((status_res & FTT_ABOT)|| !(status_res & FTT_ONLINE)) {
				DEBUG3(stderr,"setting density...\n");
				res = ftt_set_hwdens(d, d->devinfo[d->which_is_default].hwdens);
				if (res < 0) {
					return res;
				}
				d->density_is_set = 1;
			} else {
				ftt_errno = FTT_ENOTBOT;
				ftt_eprintf("ftt_open_dev: Need to change tape density for writing, but not at BOT");
				return -1;
			}
	    } else {
			d->density_is_set = 1;
	    }
	}
	return 0;
}
/*
 * This function just open device 
 */
int
ftt_open_io_dev(ftt_descriptor d) {
	
    ENTERING("ftt_open_io_dev");
    CKNULL("ftt_descriptor", d);

    if (d->which_is_default < 0 ) {
		ftt_errno = FTT_EFAULT;
		ftt_eprintf("ftt_open_io_dev: called with invalid (closed?) ftt descriptor");
		return -1;
    }
	/* correnct  device is already open */
	if ( d->which_is_open == d->which_is_default ) return 0;

	/* different device is open - this shouldn't happend and this is why it is checked */
	if ( d->which_is_open >= 0 ) {
		ftt_errno = FTT_EFAULT;
			ftt_eprintf("ftt_open_io_dev: called when the different device is open");
		return -1;
        }
	
	d->which_is_open = d->which_is_default;
	DEBUG1(stderr,"Actually opening file \n");

#ifndef WIN32

	d->file_descriptor = open(
		d->devinfo[d->which_is_default].device_name,
		(d->readonly?O_RDONLY:O_RDWR)|FNONBLOCK|O_EXCL,
		0);
	if ( d->file_descriptor < 0 ) { /* file wasn't open */
	  d->file_descriptor = ftt_translate_error(d,FTT_OPN_OPEN, "an open() system call",
						   d->file_descriptor, "ftt_open_dev",1);

#else /* This is NT part */
	{
		HANDLE fh;
		fh =  CreateFile(d->devinfo[d->which_is_default].device_name,
						(d->readonly)? GENERIC_READ : GENERIC_WRITE | GENERIC_READ,	
						0,0,OPEN_EXISTING,0,NULL);
		d->file_descriptor = (int)fh;
	}
	if ( (HANDLE)d->file_descriptor ==  INVALID_HANDLE_VALUE ) {
		/* file wasn't open */
	    ftt_translate_error_WIN(d,FTT_OPN_OPEN, "CreateFile system call",GetLastError(), "ftt_open_dev",1);
#endif
		
	    d->which_is_open = -1;
	    return -1;
	}
	DEBUG1(stderr,"File %s is OPEN : id = %d : IO = %s \n",
	       d->devinfo[d->which_is_default].device_name,d->file_descriptor,
	       ( d->readonly?"READ":"READ-WRITE"));
	return 0;
}

int
ftt_open_dev(ftt_descriptor d) {
    int status_res = 0;

    ENTERING("ftt_open_dev");
    CKNULL("ftt_descriptor" , d);

    if (d->which_is_default < 0 ) {
		ftt_errno = FTT_EFAULT;
		ftt_eprintf("ftt_open_dev: called with invalid (closed?) ftt descriptor");
		return -1;
    }


    if ( d->which_is_open == d->which_is_default ) {
      return (0);
    }


    /* can't have scsi passthru and regular device open at the same time */
    ftt_close_scsi_dev(d);


	if ( d->which_is_open >= 0 ) { 
		if ( d->which_is_open != d->which_is_default ) {
			/* different device is open -close it */
			if ( 0 > ftt_close_dev(d) ) return -1 ;
		}
	} else {
	/* Now no device is open */
		if ( 0 > ( status_res = ftt_open_status(d) )) {
			return status_res;
		}
		if (! (d->flags&FTT_FLAG_MODE_AFTER) && (status_res & FTT_ABOT)) { 
			if ( 0> ftt_open_set_mode (d,status_res)  ) return -1;
		}
		if (!(d->flags&FTT_FLAG_BSIZE_AFTER) ) {
			if ( 0 > ftt_open_set_blocksize(d) ) return -1;
		}
	}
	/* 
	** now we've checked for the ugly read-write with write protected
	** tape error, and set density if needed, we can go on and open the 
	** device with the appropriate flags.
	*/
	if ( 0 > ftt_open_io_dev(d) ) return -1;

    if ( (d->flags&FTT_FLAG_MODE_AFTER) && (status_res & FTT_ABOT)) {
      if ( 0 > ftt_open_set_mode (d,status_res) ) return -1;
    }
    if ( d->flags&FTT_FLAG_BSIZE_AFTER ) {
      if ( 0 > ftt_open_set_blocksize(d) ) return -1;
    }
	    
    DEBUG4(stderr,"Returning %d\n", d->file_descriptor);
    return d->file_descriptor;
}

/*
 * set compression, mode and  blocksize
 */
int
ftt_setdev(ftt_descriptor d) {

    int  status_res;

    ENTERING("ftt_setdev");
    CKNULL("ftt_descriptor",d);

	status_res = ftt_status(d,0);
	(void)ftt_close_dev(d);

	DEBUG3(stderr,"ftt_status returned %d\n", status_res);
	if (status_res < 0) {
		/* should we fail here or ??? */
		return status_res;
	}
	if ( 0 > ftt_open_set_mode(d,status_res) ) return -1;

	if ( 0 > ftt_open_set_blocksize(d)       ) return -1;

    return 0;
}

int
ftt_close_io_dev(ftt_descriptor d) {
    int res = 0;
    extern int errno;

    ENTERING("ftt_close_io_dev");
    CKNULL("ftt_descriptor", d);

    if ( d->which_is_open >= 0 ) {
		ftt_write_fm_if_needed(d);
        DEBUG1(stderr,"Actually closing\n");

#ifndef WIN32
		res = close(d->file_descriptor); 
		DEBUG2(stderr,"close returns %d errno %d\n", res, errno);
#else
		res = (CloseHandle((HANDLE)d->file_descriptor)) ? 0 : -1;
		DEBUG2(stderr,"close returns %d errno %d\n", res, (int)GetLastError());
#endif
		DEBUG1(stderr,"File %s is CLOSE : id = %d : IO = %s \n",
		       d->devinfo[d->which_is_default].device_name,
		       d->file_descriptor,
		       d->readonly?"READ":"READ-WRITE");

		
		d->which_is_open = -1;
		d->file_descriptor = -1;
    }
    return res;
}

int
ftt_close_dev(ftt_descriptor d) {
    int res;
	
	res = ftt_close_io_dev(d);
    if (res < 0) return res;
    res = ftt_close_scsi_dev(d);
    return res;
}
                                                                                                                                                                                   home/vsergeev/ftt/ftt_lib/ftt_partition.c                                                           0100660 0022366 0022366 00000031066 07144077446 021402  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftt_private.h"

ftt_partbuf 	
ftt_alloc_parts() {
     ftt_partbuf res;
     res = malloc(sizeof(*res));
     if (res != 0) {
            memset(res, sizeof(*res),0);
     }
     return res;
}

void 		
ftt_free_parts(ftt_partbuf p) {
   free(p);
}

int 		
ftt_extract_nparts(ftt_partbuf p) {
   return p->n_parts;
}

void 		
ftt_set_maxparts(ftt_partbuf p, int n) {
   p->max_parts = n;
}

int 		
ftt_extract_maxparts(ftt_partbuf p) {
   return p->max_parts;
}

long 		
ftt_extract_part_size(ftt_partbuf p,int n) {
    if ( n > p->n_parts || n < 0) {
        ftt_eprintf("not that many partitions in buffer");
	ftt_errno = FTT_EFAULT;
        return -1;
    }
    return p->partsizes[n];
}

int 		
ftt_set_nparts(ftt_partbuf p,int n) {
    if ( n <= p->max_parts) {
	p->n_parts = n;
        return 0;
    } else {
        return -1;
    }
}

int 		
ftt_set_part_size(ftt_partbuf p,int n,long sz) {
    if ( n > p->n_parts || n < 0) {
        ftt_eprintf("not that many partitions in buffer");
	ftt_errno = FTT_EFAULT;
        return -1;
    }
    p->partsizes[n] = sz;
    return 0;
}
#define pack(a,b,c,d) \
     (((unsigned long)(a)<<24) + ((unsigned long)(b)<<16) + ((unsigned long)(c)<<8) + (unsigned long)(d))

#include "ftt_dbd.h"

static unsigned char wp_buf[BD_SIZE+136];

int
ftt_part_util_get(ftt_descriptor d) {
    static unsigned char cdb_modsen11[6] = {0x1a, DBD, 0x11, 0x00,BD_SIZE+136, 0x00};
    return  ftt_do_scsi_command(d,"Get Partition table", cdb_modsen11, 6, wp_buf, BD_SIZE+136, 10, 0);
}

int ftt_part_util_set(ftt_descriptor d,  ftt_partbuf p ) {
    int res, i;
    int len;
    int timeout;
    static unsigned char cdb_modsel[6] = {0x15, 0x10, 0x00, 0x00,BD_SIZE+136, 0x00};

    wp_buf[0] = 0;
    wp_buf[1] = 0;

    len = wp_buf[BD_SIZE+1] + BD_SIZE + 2;

    if ( len < BD_SIZE + 10 + 2 * p->n_parts ) {
	len =  BD_SIZE + 10 + 2 * p->n_parts;
	wp_buf[BD_SIZE + 1] = 8 + 2 * p->n_parts;
    }

    cdb_modsel[4] = len;

    DEBUG3(stderr,"Got length of %d\n", len);

    /* set number of partitions */
    wp_buf[BD_SIZE+3] = p->n_parts;

    /* set to write initiator defined partitions, in MB */
    wp_buf[BD_SIZE+4] = 0x20 | 0x10;

    /* fill in partition sizes... */
    for( i = 0 ; i <= p->n_parts; i++ ) {
	wp_buf[BD_SIZE+8 + 2*i + 0] = (p->partsizes[i] & 0xff00) >> 8;
	wp_buf[BD_SIZE+8 + 2*i + 1] = p->partsizes[i] & 0x00ff;
    }
    for( i = p->n_parts + 1 ; i <= p->max_parts; i++ ) {
	wp_buf[BD_SIZE+8 + 2*i + 0] = 0;
	wp_buf[BD_SIZE+8 + 2*i + 1] = 0;
    }

    timeout = 1800;
    if ( 0 == strncmp(ftt_unalias(d->prod_id),"SDX-", 4) ) {
        timeout *= 10;
    }

    res = ftt_do_scsi_command(d,"Put Partition table", cdb_modsel, 6, wp_buf, len, timeout, 1);
    return res;
}

int		
ftt_get_partitions(ftt_descriptor d,ftt_partbuf p) {
    static unsigned char buf[BD_SIZE+136];
    static unsigned char cdb_modsen11[6] = {0x1a, DBD, 0x11, 0x00,140, 0x00};
    int res;
    int i;

    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {
	ftt_close_dev(d);
	switch(ftt_fork(d)){
	case -1:
		return -1;

	case 0:  /* child */
		fflush(stdout);	/* make async_pf stdout */
		fflush(d->async_pf_parent);
		close(1);
		dup2(fileno(d->async_pf_parent),1);
		fclose(d->async_pf_parent);

		if (ftt_debug) {
		    execlp("ftt_suid", "ftt_suid", "-x",  "-p", d->basename, 0);
		} else {
		     execlp("ftt_suid", "ftt_suid", "-p", d->basename, 0);
		}
		break;

	default: /* parent */
		ftt_undump_partitions(p,d->async_pf_child);
		res = ftt_wait(d);
	}

    } else {

        ftt_part_util_get(d);
	if (res < 0) return res;
	p->max_parts = wp_buf[BD_SIZE+2];
	p->n_parts = wp_buf[BD_SIZE+3];
	for( i = 0 ; i <= p->n_parts; i++ ) {
	    p->partsizes[i] = pack(0,0,wp_buf[BD_SIZE+8+2*i],wp_buf[BD_SIZE+8+2*i+1]);
	}
	return 0;
   }
}



int		
ftt_write_partitions(ftt_descriptor d,ftt_partbuf p) {
    int res, i;
    int len;
    int pd[2];
    FILE *topipe;


    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {

        res = pipe(pd); if (res < 0) return -1; 

	DEBUG2(stderr,"pipe is (%d,%d)\n", pd[0], pd[1]);
        fflush(stderr);
  
	ftt_close_dev(d);

	switch(ftt_fork(d)){
	case -1:
		return -1;

	case 0:  /* child */
		/* output -> async_pf */
		fflush(stdout);	/* make async_pf stdout */
		fflush(d->async_pf_parent);
		close(1);
		dup2(fileno(d->async_pf_parent),1);
                fclose(d->async_pf_parent);

		/* stdin <- pd[0] */
                fclose(stdin);
		close(pd[1]);
		dup2(pd[0],0);
                close(pd[0]);

		if (ftt_debug) {
		    execlp("ftt_suid", "ftt_suid", "-x",  "-u", d->basename, 0);
		} else {
		     execlp("ftt_suid", "ftt_suid", "-u", d->basename, 0);
		}
		break;

	default: /* parent */
		/* close the read end of the pipe... */
                close(pd[0]);

		/* send the child the partition data */
		topipe = fdopen(pd[1],"w");
		ftt_dump_partitions(p,topipe);
  		fclose(topipe);

		res = ftt_wait(d);
	}

    } else {
        res = ftt_part_util_get( d );
	if (res < 0) return res;
	res =  ftt_part_util_set(d, p);
	if (res < 0) return res;
    }
    return res;
}

int
ftt_cur_part(ftt_descriptor d) {
    static ftt_stat_buf b;
    
    if (0 == b) {
	b = ftt_alloc_stat();
    }
    ftt_get_stats(d,b);
    return atoi(ftt_extract_stats(b,FTT_CUR_PART));
}

int		
ftt_skip_part(ftt_descriptor d,int nparts) {
    int cur;

    cur = ftt_cur_part(d);
    cur += nparts;
    return ftt_locate_part(d, 0, cur);
}

int		
ftt_locate_part(ftt_descriptor d, int blockno, int part) {
    int cur;
    int res = 0;

    if ( blockno == 0 ) {
	d->current_block = 0;
	d->current_file = 0;
	d->current_valid = 1;
    } else {
	d->current_valid = 0;
    }
    d->data_direction = FTT_DIR_READING;
    d->last_pos = -1;   /* we skipped backwards, so this can't be valid */

    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {
        static char buf1[10],buf2[10];

        sprintf(buf1,"%d",blockno);
        sprintf(buf2,"%d",part);

	ftt_close_dev(d);
	switch(ftt_fork(d)){
	case -1:
		return -1;

	case 0:  /* child */
		fflush(stdout);	/* make async_pf stdout */
		fflush(d->async_pf_parent);
		close(1);
		dup2(fileno(d->async_pf_parent),1);
		if (ftt_debug) {
		    execlp("ftt_suid", "ftt_suid", "-x",  "-L", buf1, buf2, d->basename, 0);
		} else {
		     execlp("ftt_suid", "ftt_suid", "-L", buf1, buf2, d->basename, 0);
		}
		break;

	default: /* parent */
		res = ftt_wait(d);
	}

    } else {

	static unsigned char 
	    locate_cmd[10] = {0x2b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	locate_cmd[1] = 0x02;
	locate_cmd[3] = (blockno >> 24) & 0xff;
	locate_cmd[4] = (blockno >> 16) & 0xff;
	locate_cmd[5] = (blockno >> 8)  & 0xff; 
	locate_cmd[6] = blockno & 0xff;
	locate_cmd[8] = part;

	res = ftt_do_scsi_command(d,"Locate",locate_cmd,10,NULL,0,300,0);

	res = ftt_describe_error(d,0,"a SCSI pass-through call", res,res,"Locate", 0);
        if (res < 0 && blockno == 0 && part == 0 && (ftt_errno == FTT_EBLANK || ftt_errno == FTT_ENOSPC ))  {
             res = 0;
             ftt_errno = 0;
             ftt_eprintf("Ok");
        }

    }
    return res;
}

/* shared printf formats for dump/undump */
char *curfmt = "Cur: %d\n";
char *maxfmt = "Max: %d\n";
char *parfmt = "P%d: %u MB\n";

ftt_dump_partitions(ftt_partbuf parttab, FILE *pf) {
    int i;

    fprintf(pf,"Partition table:\n");
    fprintf(pf,"================\n");
    fprintf(pf, curfmt, ftt_extract_nparts(parttab));
    fprintf(pf, maxfmt, ftt_extract_maxparts(parttab));
    for( i = 0; i <= parttab->n_parts; i++) {
	 fprintf(pf,parfmt, i, ftt_extract_part_size(parttab,i));
    }
    fflush( pf );
    return;
}

ftt_undump_partitions(ftt_partbuf p, FILE *pf) {
    static char buf[80];
    int i,junk;

    buf[0] = 'x';
    while (buf[0] != '=') {
	fgets(buf,80,pf);
	DEBUG2(stderr,"skipping line %s\n", buf);
    }
    fscanf(pf, curfmt, &(p->n_parts));
    DEBUG2(stderr,"got n_parts of %d\n", p->n_parts);
    fscanf(pf, maxfmt, &(p->max_parts));
    DEBUG2(stderr,"got max_parts of %d\n", p->max_parts);
    for( i = 0 ; i <= p->n_parts; i++ ) {
	fscanf(pf, parfmt, &junk, &(p->partsizes[i]));
    }
}

int		
ftt_set_mount_partition(ftt_descriptor d, int partno) {
    int res = 0;

    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {
        static char buf2[10];

        sprintf(buf2,"%d",partno);

	ftt_close_dev(d);
	switch(ftt_fork(d)){
	case -1:
		return -1;

	case 0:  /* child */
		fflush(stdout);	/* make async_pf stdout */
		fflush(d->async_pf_parent);
		close(1);
		dup2(fileno(d->async_pf_parent),1);
		if (ftt_debug) {
		    execlp("ftt_suid", "ftt_suid", "-x",  "-M", buf2, d->basename, 0);
		} else {
		     execlp("ftt_suid", "ftt_suid", "-M", buf2, d->basename, 0);
		}
		break;

	default: /* parent */
		res = ftt_wait(d);
	}

    } else {

	ftt_partbuf p;
	static unsigned char buf[BD_SIZE+6];
	static unsigned char cdb_modsense[6] = {0x1a, DBD, 0x21, 0x00, BD_SIZE+6, 0x00};
	static unsigned char cdb_modsel[6] = {0x15, 0x10, 0x00, 0x00, BD_SIZE+6, 0x00};
	int len;
	int max;

	/* get maximum number of partitions.. */
	p = ftt_alloc_parts();
	ftt_get_partitions(d,p);
	max = ftt_extract_maxparts(p);
	ftt_free_parts(p);

	/* -1 means highest supported partition */
	if ( partno < 0 || partno > max ) partno = max;

	res = ftt_do_scsi_command(d,"Mode Sense, 0x21", cdb_modsense, 6, buf, BD_SIZE+6, 10, 0);
	if (res < 0) return res;

	buf[0] = 0;
	buf[1] = 0;

	len = buf[BD_SIZE+1] + BD_SIZE + 2;

	/* set load partition */
	buf[BD_SIZE+3] &= ~0x7e;
	buf[BD_SIZE+3] |= (partno << 1) & 0x7e;

	/* reserved fields */
	buf[BD_SIZE+2] = 0;
	buf[BD_SIZE+4] = 0;
	buf[BD_SIZE+5] = 0;

	res = ftt_do_scsi_command(d,"Mode Select, 0x21", cdb_modsel, 6, buf, len, 10, 1);
    }
    return res;
}


int
ftt_format_ait(ftt_descriptor d, int on, ftt_partbuf pb) {

   int   res;
   int   i;

    static unsigned char
        mod_sen31[6] = {0x1a, 0x00, 0x31, 0x00, 0x16, 0x00 },
        mod_sel31[6] = {0x15, 0x10, 0x00, 0x00, 0x16, 0x00 },

        ait_conf_buf[4+8+10];


    ENTERING("ftt_format_ait");

    CKNULL("ftt_descriptor", d);
    DEBUG2(stderr, "Entering ftt_format_ait\n");
    res = 0;
    if ((d->flags&FTT_FLAG_SUID_SCSI) == 0 || 0 == geteuid()) {

        res = ftt_open_scsi_dev(d);
        if(res < 0) return res;

        res = ftt_part_util_get(d);
        if(res < 0) return res;
        
        /* get the AIT Device Configuration page 0x31 */
        DEBUG2(stderr, "CALLING ----- mod_sen31\n");
        res = ftt_do_scsi_command(d, "Mode Sense 0x31", mod_sen31, 6,
           ait_conf_buf, 4+8+10, 5, 0);
        if (res < 0) return res;

        /* switch device into native AIT mode */
        ait_conf_buf[0] = 0x00;                        /* reserved */
        ait_conf_buf[1] = 0x00;                        /* reserved */
        ait_conf_buf[4+8+0] &= 0x3f;                   /* reserved */
        if ( on ) {
          if ((ait_conf_buf[4+8+4] & 0x80) != 0 ) {
            /* volume has a MIC */
            ait_conf_buf[4+8+2] |= 0xf3;               /* enable full AIT mode */
          } else {
            /* volume has no MIC */
            ait_conf_buf[4+8+2] |= 0xe0;               /* enable AIT mode */
          }
       } else {
          ait_conf_buf[4+8+2] &= ~0xc0;             /* disable AIT mode */
          ait_conf_buf[4+8+2] |=  0x10;             /* turn on ULPBOT */
       }

        /* set the AIT Device Configuration page and switch format mode */
        DEBUG2(stderr, "CALLING ----- mod_sel31\n");
        res = ftt_do_scsi_command(d, "Mode Select 0x31", mod_sel31, 6,
           ait_conf_buf, 4+8+10, 180, 1);

        if(res < 0) return res;

	res =  ftt_part_util_set(d, pb);

    } else {
        int pd[2];
        FILE *topipe;
        pipe(pd);
        ftt_close_dev(d);
        ftt_close_scsi_dev(d);
	switch(ftt_fork(d)){
	static char s1[10];
	case -1:
		return -1;

	case 0:  /* child */
		/* output -> async_pf */
		fflush(stdout);	/* make async_pf stdout */
		fflush(d->async_pf_parent);
		close(1);
		dup2(fileno(d->async_pf_parent),1);
                fclose(d->async_pf_parent);

		/* stdin <- pd[0] */
                fclose(stdin);
		close(pd[1]);
		dup2(pd[0],0);
                close(pd[0]);

		sprintf(s1, "%d", on);

		if (ftt_debug) {
		    execlp("ftt_suid", "ftt_suid", "-x",  "-A", s1, d->basename, 0);
		} else {
		     execlp("ftt_suid", "ftt_suid", "-A", s1, d->basename, 0);
		}
		break;

	default: /* parent */
		/* close the read end of the pipe... */
                close(pd[0]);

		/* send the child the partition data */
		topipe = fdopen(pd[1],"w");
		ftt_dump_partitions(pb,topipe);
  		fclose(topipe);

		res = ftt_wait(d);
	}
    }
    return res;
}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                          home/vsergeev/ftt/ftt_lib/ftt_rdwr.c                                                                0100640 0022366 0022366 00000014252 10063650602 020324  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <stdlib.h>
#include <ftt_private.h>

#ifdef WIN32
#include <io.h>
#include <windows.h>
#include <winioctl.h>

int ftt_translate_error_WIN();
#else
#include <unistd.h>
#endif

int	 ftt_describe_error();

int
ftt_get_readonly(ftt_descriptor d) {

    return d->readonly;
}

int
ftt_get_position(ftt_descriptor d, int *file, int *block) {

    CKOK(d,"ftt_get_position",0,0);
    CKNULL("ftt_descriptor", d);

   if( file != 0 ){
      *file = d->current_file;
   }
   if( block != 0 ){
      *block = d->current_block;
   }
   if (d->current_valid) {
       return 0;
   } else {
       ftt_errno = FTT_ELOST;
       ftt_eprintf(
"ftt_get_position: unable to determine the current tape position,\n\
	until you do an ftt_rewind, or ftt_status or ftt_get_stats call at BOT.\n");
       return -1;
   }
}
unsigned char	ftt_cdb_read[]  = { 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 },
	  	ftt_cdb_write[] = { 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00 };

int
ftt_read( ftt_descriptor d, char *buf, int length ) {
    int res;

    CKOK(d,"ftt_read",0,0);
    CKNULL("ftt_descriptor", d);
    CKNULL("data buffer pointer", buf);
    
    if ( 0 != (d->scsi_ops & FTT_OP_READ)) {
	DEBUG4(stderr, "SCSI pass-thru\n");
	d->last_operation = FTT_OP_READ;
	if (d->default_blocksize == 0) {
		ftt_set_transfer_length(ftt_cdb_read,length);
	} else {
		ftt_set_transfer_length(ftt_cdb_read,length/d->default_blocksize);
	}
	res = ftt_do_scsi_command(d,"read",ftt_cdb_read, 6, 
				(unsigned char*)buf, length, 60, 0);
	res = ftt_describe_error(d, FTT_OPN_READ, "ftt_read", res, res, "a read SCSI command", 1);
    
	} else {
	
		DEBUG4(stderr,"System Call\n");
		if (0 != (d->last_operation &(FTT_OP_WRITE|FTT_OP_WRITEFM)) &&
			0 != (d->flags& FTT_FLAG_REOPEN_R_W)) {
			ftt_close_dev(d);
		}
		if ( 0 > (res = ftt_open_dev(d))) {
	    		return res;
		}
		d->last_operation = FTT_OP_READ;

#ifndef WIN32

		res = read(d->file_descriptor, buf, length);
		res = ftt_translate_error(d, FTT_OPN_READ, "ftt_read", res, "a read system call",1);
		if (res == FTT_EBLANK) {
			/* we read past end of tape, prevent further confusion on AIX */
			d->unrecovered_error = 1;
		}
#else 
		{ /* ---------------- this is the WIN-NT part -----------------*/
			DWORD	nread,Lerrno;	
			if ( ! ReadFile((HANDLE)d->file_descriptor,(LPVOID)buf,(DWORD)length,&nread,0) ) {
				Lerrno = GetLastError();
				if ( Lerrno == ERROR_FILEMARK_DETECTED ) {
					nread = (DWORD)0;
				} else {
					ftt_translate_error_WIN(d, FTT_OPN_READ, "ftt_read",
						GetLastError(), "a ReadFile call",1);
					nread = (DWORD)-1;
				}
			}
			res = (int)nread;
		}
#endif
	
	
		}
    if (0 == res) { /* end of file */
	if( d->flags & FTT_FLAG_FSF_AT_EOF){
	    ftt_skip_fm(d,1);
	} else if (d->flags & FTT_FLAG_REOPEN_AT_EOF) {
	    ftt_close_dev(d);
	    ftt_open_dev(d);
	} else {
	    /* fix file offset */
	    lseek(d->file_descriptor, 0L, 0);
	}
	d->current_block = 0;
	d->current_file++;
    } else if (res > 0){
	d->readlo += res;
	d->readkb += d->readlo >> 10;
	d->readlo &= (1<<10) - 1;
	d->current_block++;
    } else {
	d->nharderrors++;
	DEBUG0(stderr,"HARD error - reading record - error %d \n",res);
    }
    d->nreads++;
    d->data_direction = FTT_DIR_READING;
    return res;
}

#ifdef DEBUGWRITES
int
mywrite( int fd, char *buf, int len ) {
    int res;
    res = write(fd,buf,len);
    fprintf(stderr, "mywrite: write really returned %d, return? " );
    fflush(stderr);
    fscanf(stdin, "%d", &res);
    return res;
}
#define write mywrite

#endif

int
ftt_write( ftt_descriptor d, char *buf, int length ) {
    int res;
    int status;
    int fileno;
    int blockno;
    static ftt_stat_buf		statbuf = NULL;
    char	*peot;
    char 	*eom; 	

    CKOK(d,"ftt_write",1,0);
    CKNULL("ftt_descriptor", d);
    CKNULL("data buffer pointer", buf);


   statbuf = ftt_alloc_stat();
   if (!statbuf) {
      fprintf (stderr,"Could not allocate stat buffer \n");
      return 1;
   }

    if ( 0 != (d->scsi_ops & FTT_OP_WRITE)) {
	DEBUG4(stderr,"SCSI pass-thru\n");
	d->last_operation = FTT_OP_WRITE;
	if (d->default_blocksize == 0) {
		ftt_set_transfer_length(ftt_cdb_write,length);
	} else {
		ftt_set_transfer_length(ftt_cdb_write,length/d->default_blocksize);
	}
	res = ftt_do_scsi_command(d,"write",ftt_cdb_write, 6, 
				(unsigned char *)buf, length, 60,1);
        if (res == -1) {
        }

	res = ftt_describe_error(d, FTT_OPN_WRITE, "ftt_write", res, res, "a write SCSI command", 0);
    } else {
		DEBUG4(stderr,"System Call\n");
		if (0 != (d->last_operation &(FTT_OP_READ)) &&
			0 != (d->flags& FTT_FLAG_REOPEN_R_W)) {
			ftt_close_dev(d);
		}
		if ( 0 > (res = ftt_open_dev(d))) {
			return res;
		}
		d->last_operation = FTT_OP_WRITE;

#ifndef WIN32

		res = write(d->file_descriptor, buf, length);
                if (res == -1) {
                   status = ftt_get_stats (d, statbuf);
                   eom = ftt_extract_stats (statbuf,16);
                   peot = ftt_extract_stats (statbuf,17);

                   if (peot[0] == '1' || eom[0] == '1') {
                      status = ftt_skip_fm (d, -1);
                      status = ftt_skip_fm (d, 1);
                      res = 0;
                   }
                }

		res = ftt_translate_error(d, FTT_OPN_WRITE, "ftt_write", res, "a write() system call",0);
#else
		{ /* ---------------- this is the WIN-NT part -----------------*/
			DWORD	nwrt;	

			if ( !  WriteFile((HANDLE)d->file_descriptor,(LPVOID)buf,(DWORD)length,&nwrt,0 )) {
				ftt_translate_error_WIN(d, FTT_OPN_READ, "ftt_write",
					GetLastError(), "a WriteFile call",1);
				nwrt = (DWORD)-1;
			}
			res = (int)nwrt;
		}
#endif
	
	}
    if (res > 0) {
	d->writelo += res;
	d->writekb += d->writelo >> 10;
	d->writelo &= (1<<10) - 1;
	d->current_block++;
        if ( res < length ) {
	    ftt_errno = FTT_EPARTIALWRITE;
            ftt_eprintf("Error: wrote fewer bytes than requested.");
        }
    } else if (res == 0)  {
	ftt_eprintf("Notice: end of tape/partition encountered");
	ftt_errno = FTT_ENOSPC;
    } else {
	DEBUG0(stderr,"HARD error - writing record - error %d \n",res);
	d->nharderrors++;
    }
    d->nwrites++;
    d->data_direction = FTT_DIR_WRITING;
    return res;
}
                                                                                                                                                                                                                                                                                                                                                      home/vsergeev/ftt/ftt_lib/ftt_rdwr.orig.c                                                           0100664 0022366 0022366 00000013377 10031051633 021272  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <stdlib.h>
#include <ftt_private.h>

#ifdef WIN32
#include <io.h>
#include <windows.h>
#include <winioctl.h>

int ftt_translate_error_WIN();
#else
#include <unistd.h>
#endif

int	 ftt_describe_error();

int
ftt_get_readonly(ftt_descriptor d) {

    return d->readonly;
}

int
ftt_get_position(ftt_descriptor d, int *file, int *block) {

    CKOK(d,"ftt_get_position",0,0);
    CKNULL("ftt_descriptor", d);

   if( file != 0 ){
      *file = d->current_file;
   }
   if( block != 0 ){
      *block = d->current_block;
   }
   if (d->current_valid) {
       return 0;
   } else {
       ftt_errno = FTT_ELOST;
       ftt_eprintf(
"ftt_get_position: unable to determine the current tape position,\n\
	until you do an ftt_rewind, or ftt_status or ftt_get_stats call at BOT.\n");
       return -1;
   }
}
unsigned char	ftt_cdb_read[]  = { 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 },
	  	ftt_cdb_write[] = { 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00 };

int
ftt_read( ftt_descriptor d, char *buf, int length ) {
    int res;

    CKOK(d,"ftt_read",0,0);
    CKNULL("ftt_descriptor", d);
    CKNULL("data buffer pointer", buf);
    
    if ( 0 != (d->scsi_ops & FTT_OP_READ)) {
	DEBUG4(stderr, "SCSI pass-thru\n");
	d->last_operation = FTT_OP_READ;
	if (d->default_blocksize == 0) {
		ftt_set_transfer_length(ftt_cdb_read,length);
	} else {
		ftt_set_transfer_length(ftt_cdb_read,length/d->default_blocksize);
	}
	res = ftt_do_scsi_command(d,"read",ftt_cdb_read, 6, 
				(unsigned char*)buf, length, 60, 0);
	res = ftt_describe_error(d, FTT_OPN_READ, "ftt_read", res, res, "a read SCSI command", 1);
    
	} else {
	
		DEBUG4(stderr,"System Call\n");
		if (0 != (d->last_operation &(FTT_OP_WRITE|FTT_OP_WRITEFM)) &&
			0 != (d->flags& FTT_FLAG_REOPEN_R_W)) {
			ftt_close_dev(d);
		}
		if ( 0 > (res = ftt_open_dev(d))) {
	    		return res;
		}
		d->last_operation = FTT_OP_READ;

#ifndef WIN32

		res = read(d->file_descriptor, buf, length);
		res = ftt_translate_error(d, FTT_OPN_READ, "ftt_read", res, "a read system call",1);
		if (res == FTT_EBLANK) {
			/* we read past end of tape, prevent further confusion on AIX */
			d->unrecovered_error = 1;
		}
#else 
		{ /* ---------------- this is the WIN-NT part -----------------*/
			DWORD	nread,Lerrno;	
			if ( ! ReadFile((HANDLE)d->file_descriptor,(LPVOID)buf,(DWORD)length,&nread,0) ) {
				Lerrno = GetLastError();
				if ( Lerrno == ERROR_FILEMARK_DETECTED ) {
					nread = (DWORD)0;
				} else {
					ftt_translate_error_WIN(d, FTT_OPN_READ, "ftt_read",
						GetLastError(), "a ReadFile call",1);
					nread = (DWORD)-1;
				}
			}
			res = (int)nread;
		}
#endif
	
	
		}
    if (0 == res) { /* end of file */
	if( d->flags & FTT_FLAG_FSF_AT_EOF){
	    ftt_skip_fm(d,1);
	} else if (d->flags & FTT_FLAG_REOPEN_AT_EOF) {
	    ftt_close_dev(d);
	    ftt_open_dev(d);
	} else {
	    /* fix file offset */
	    lseek(d->file_descriptor, 0L, 0);
	}
	d->current_block = 0;
	d->current_file++;
    } else if (res > 0){
	d->readlo += res;
	d->readkb += d->readlo >> 10;
	d->readlo &= (1<<10) - 1;
	d->current_block++;
    } else {
	d->nharderrors++;
	DEBUG0(stderr,"HARD error - reading record - error %d \n",res);
    }
    d->nreads++;
    d->data_direction = FTT_DIR_READING;
    return res;
}

#ifdef DEBUGWRITES
int
mywrite( int fd, char *buf, int len ) {
    int res;
    res = write(fd,buf,len);
    fprintf(stderr, "mywrite: write really returned %d, return? " );
    fflush(stderr);
    fscanf(stdin, "%d", &res);
    return res;
}
#define write mywrite

#endif

int
ftt_write( ftt_descriptor d, char *buf, int length ) {
    int res;

    CKOK(d,"ftt_write",1,0);
    CKNULL("ftt_descriptor", d);
    CKNULL("data buffer pointer", buf);

    if ( 0 != (d->scsi_ops & FTT_OP_WRITE)) {
	DEBUG4(stderr,"SCSI pass-thru\n");
	d->last_operation = FTT_OP_WRITE;
	if (d->default_blocksize == 0) {
		ftt_set_transfer_length(ftt_cdb_write,length);
	} else {
		ftt_set_transfer_length(ftt_cdb_write,length/d->default_blocksize);
	}
	res = ftt_do_scsi_command(d,"write",ftt_cdb_write, 6, 
				(unsigned char *)buf, length, 60,1);
        if (res == -1) {
        fprintf (stderr,"FTT_WRITE res %u scsi_opt %u\n",res, d->scsi_ops);
        }

	res = ftt_describe_error(d, FTT_OPN_WRITE, "ftt_write", res, res, "a write SCSI command", 0);
    } else {
		DEBUG4(stderr,"System Call\n");
		if (0 != (d->last_operation &(FTT_OP_READ)) &&
			0 != (d->flags& FTT_FLAG_REOPEN_R_W)) {
			ftt_close_dev(d);
		}
		if ( 0 > (res = ftt_open_dev(d))) {
                fprintf (stderr,"ftt_write open dev %u\n",res);
			return res;
		}
		d->last_operation = FTT_OP_WRITE;

#ifndef WIN32

		res = write(d->file_descriptor, buf, length);
                if (res == -1) {
                fprintf (stderr,"ftt_write write %u\n",res);
                
                }
		res = ftt_translate_error(d, FTT_OPN_WRITE, "ftt_write", res, "a write() system call",0);
#else
		{ /* ---------------- this is the WIN-NT part -----------------*/
			DWORD	nwrt;	

			if ( !  WriteFile((HANDLE)d->file_descriptor,(LPVOID)buf,(DWORD)length,&nwrt,0 )) {
				ftt_translate_error_WIN(d, FTT_OPN_READ, "ftt_write",
					GetLastError(), "a WriteFile call",1);
				nwrt = (DWORD)-1;
			}
			res = (int)nwrt;
		}
#endif
	
	}
    if (res > 0) {
	d->writelo += res;
	d->writekb += d->writelo >> 10;
	d->writelo &= (1<<10) - 1;
	d->current_block++;
        if ( res < length ) {
	    ftt_errno = FTT_EPARTIALWRITE;
            ftt_eprintf("Error: wrote fewer bytes than requested.");
        }
    } else if (res == 0)  {
	ftt_eprintf("Notice: end of tape/partition encountered");
	ftt_errno = FTT_ENOSPC;
    } else {
	DEBUG0(stderr,"HARD error - writing record - error %d \n",res);
	d->nharderrors++;
    }
    d->nwrites++;
    d->data_direction = FTT_DIR_WRITING;
    return res;
}
                                                                                                                                                                                                                                                                 home/vsergeev/ftt/ftt_lib/ftt_skip.c                                                                0100660 0022366 0022366 00000034021 10063650630 020313  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";

#include <stdlib.h>
#include <stdio.h>
#include <ftt_private.h>
#include <string.h>
#include <ftt_mtio.h> 

#ifdef WIN32
#include <malloc.h>
#include <io.h>
#include <process.h>
#include <windows.h>
#include <winioctl.h>

int ftt_translate_error_WIN();

#define geteuid() -1

#else
#include <unistd.h>
#endif

extern int errno;

int ftt_describe_error();

/*
** ioctlbuf is the tapeop strcture
** (struct mtio, struct tapeio, etc.)
** that we'll use to do all the I/O
*/
static struct tapeop ioctlbuf;

/*
** ftt_mtop has all of the common code for rewind, retenstion, etc.
** factored into it.  It starts async operations, and cleans up
** after them (using a single level of recursion).
**
** we then decide if the operation is a pass-thru, and do it that
** way if needed,
** otherwise we make sure the device is open,
** and if it has been successfully opened, we fill in an mtio
** block and perform the mtio call, performing the appropriate
** error translation if it fails.
*/
static int 
ftt_mtop(ftt_descriptor d, int n, int mtop, int opn, char *what, unsigned char *cdb) {
    int res;

    ENTERING("ftt_mtop");
    CKNULL("ftt_descriptor", d);
    CKNULL("operation name", what);
    CKNULL("operation SCSI CDB", cdb);
    DEBUG1(stderr,"ftt_mtop operation %d n %d to do %s\n", opn, n, what);


    if ( 0 != (d->scsi_ops & (1 << opn))) {
		DEBUG2(stderr, "SCSI pass-thru\n");
		if (opn == FTT_OPN_RSKIPREC || opn == FTT_OPN_RSKIPFM) {
			ftt_set_transfer_length(cdb,-n);
		} else {
			ftt_set_transfer_length(cdb,n);
		}
		res = ftt_do_scsi_command(d,what,cdb, 6, 0, 0, 120, 0);
		res = ftt_describe_error(d,opn,"a SCSI pass-through call", res,res, what, 0);
    
	} else {
	
		DEBUG2(stderr,"System Call\n");

		if ( 0 > (res = ftt_open_dev(d))) {
			DEBUG3(stderr,"open returned %d\n", res);
			return res;
		} else {

#ifndef WIN32

			ioctlbuf.tape_op = mtop;
			ioctlbuf.tape_count = n;
			res = ioctl(d->file_descriptor, FTT_TAPE_OP, &ioctlbuf);
			DEBUG3(stderr,"ioctl returned %d\n", res);
			res = ftt_translate_error(d, opn, "an mtio ioctl() call", res, what,0);
			/*
			** we do an lseek to reset the file offset counter
			** so the OS doesn't get hideously confused if it 
			** overflows...  We may need this to be a behavior
			** flag in the ftt_descriptor and device tables.
			*/
			(void) lseek(d->file_descriptor, 0L, 0);
#else
		{
			DWORD LowOff,fres;
			HANDLE fh = (HANDLE)d->file_descriptor;
			int Count=0;

			if (opn == FTT_OPN_RSKIPFM || opn == FTT_OPN_RSKIPREC ) { LowOff = (DWORD) -n;
			} else LowOff = (DWORD)n;

			if ( opn == FTT_OPN_RSKIPFM || opn == FTT_OPN_SKIPFM ) {
				fres = SetTapePosition(fh,TAPE_SPACE_FILEMARKS,0,LowOff,0,0);
			
			} else if ( opn == FTT_OPN_RSKIPREC || opn == FTT_OPN_SKIPREC ) {
				fres = SetTapePosition(fh,TAPE_SPACE_RELATIVE_BLOCKS,0,LowOff,0,0);
			} else if ( opn == FTT_OPN_WRITEFM ) {
				fres = WriteTapemark(fh,TAPE_LONG_FILEMARKS,n,0); /* can be LONG or SHORT */
			} else if ( opn == FTT_OPN_RETENSION ) {
				/* go to the end of tape */
				do {
					fres = SetTapePosition(fh,TAPE_SPACE_FILEMARKS,0,99999,0,0); 
				} while ( fres == NO_ERROR);
				fres = SetTapePosition(fh,TAPE_SPACE_END_OF_DATA,0,0,0,0);
				/* now rewind */
				do {
					fres = SetTapePosition(fh,TAPE_SPACE_FILEMARKS,0,(DWORD)-99999,0,0); 
				} while ( fres == NO_ERROR);
				fres = SetTapePosition(fh,TAPE_REWIND,0,0,0,0); 
				
			} else if ( opn == FTT_OPN_UNLOAD  ) {
				fres = PrepareTape(fh,TAPE_UNLOAD,0);
			} else if ( opn == FTT_OPN_REWIND  ) {
				/* this is the trick to avoid Bus reset */
				do {
					fres = SetTapePosition(fh,TAPE_SPACE_FILEMARKS,0,(DWORD)-99999,0,0); 
				} while ( fres == NO_ERROR);
				fres = SetTapePosition(fh,TAPE_REWIND,0,0,0,0); 
			} else if ( opn == FTT_OPN_ERASE   ) {
				fres = EraseTape(fh,TAPE_ERASE_LONG,0); /* can be SHORT */
			} else {
				fres = (DWORD)-1;
			}
			res = ftt_translate_error_WIN(d, opn, "win - tape functions ", fres, what,0);
		}	
#endif	
	  }
	}
    if (res < 0) {
                DEBUG0(stderr,"HARD error doing ftt_mtop operation %d n %d to do %s - error \n", opn, n, what,res);
		d->nharderrors++;
    }
    d->last_operation = (1 << opn);
    return res;
}

unsigned char ftt_cdb_skipfm[]	= {0x11, 0x01, 0x00, 0x00, 0x00, 0x00};
unsigned char ftt_cdb_skipbl[]	= {0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ftt_cdb_rewind[]	= {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ftt_cdb_unload[]	= {0x1b, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ftt_cdb_retension[]= {0x1b, 0x00, 0x00, 0x00, 0x03, 0x00};
unsigned char ftt_cdb_erase[]	= {0x19, 0x01, 0x00, 0x00, 0x00, 0x00};
unsigned char ftt_cdb_writefm[]	= {0x10, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
** The remaining calls just invoke mtop with the right options
*/
int
ftt_skip_fm(ftt_descriptor d, int n) {
    int res, res2;

    CKOK(d,"ftt_skip_fm",0,1);
    CKNULL("ftt_descriptor", d);

    if ( n < 0 ) {
        d->last_pos = -1;/* we skipped backwards, so this can't be valid */
		res = ftt_write_fm_if_needed(d); 	if (res < 0) {return res;}
    }

    res = ftt_skip_fm_internal(d,n); 
	if (res   < 0 ) {
		if ( ftt_errno == FTT_ELEADER )
			ftt_eprintf("ftt_skip_fm: At BOT after doing a skip filemark");
		else if (ftt_errno == FTT_EBLANK ) 
			ftt_eprintf("ftt_skip_fm: At EOT after doing a skip filemark");
		return res;
	}
	
    res2 = ftt_status(d,0);
    DEBUG3(stderr, "ftt_status returns %d after skip\n", res2);

    if ((res   < 0 && ftt_errno == FTT_ELEADER ) ||
		( res2 > 0 && (res2 & FTT_ABOT))) {
		d->unrecovered_error = 2;
		ftt_errno = FTT_ELEADER;
		ftt_eprintf("ftt_skip_fm: At BOT after doing a skip filemark");
		res =  -1;
    }
    if ((res   < 0 && ftt_errno == FTT_EBLANK ) ||
		( res2 > 0 && (res2 & FTT_AEOT) )) {
		d->unrecovered_error = 2;
		ftt_errno = FTT_EBLANK;
		ftt_eprintf("ftt_skip_fm: At EOT after doing a skip filemark");
		res = -1;
	}
	
    return res;
}

int
ftt_skip_fm_internal(ftt_descriptor d, int n) {

    d->current_file += n;
    d->current_block = 0;

    if (n < 0) {
	return ftt_mtop(d, -n,  FTT_TAPE_RSF,  FTT_OPN_RSKIPFM, "ftt_skip_fm", ftt_cdb_skipfm);
   } else {
	return ftt_mtop(d, n,  FTT_TAPE_FSF,  FTT_OPN_SKIPFM, "ftt_skip_fm", ftt_cdb_skipfm);
   }

}

int
ftt_skip_rec(ftt_descriptor d, int n){
    int res;

    CKOK(d,"ftt_skip_rec",0,0);
    CKNULL("ftt_descriptor", d);

    d->current_block += n;
    if ( n < 0 ) {
        d->last_pos = -1;/* we skipped backwards, so this can't be valid */
	    res = ftt_write_fm_if_needed(d);
	    if (res < 0){return res;}
        return ftt_mtop(d, -n, FTT_TAPE_RSR, FTT_OPN_RSKIPREC, "ftt_skip_rec", 
			ftt_cdb_skipbl);
    } else {
        return ftt_mtop(d, n, FTT_TAPE_FSR, FTT_OPN_SKIPREC, "ftt_skip_rec", 
			ftt_cdb_skipbl);
    }
}

int
ftt_rewind(ftt_descriptor d){
    int res, res2;

    CKOK(d,"ftt_rewind",0,2);
    CKNULL("ftt_descriptor", d);

    res = ftt_write_fm_if_needed(d);
    d->data_direction = FTT_DIR_READING;
    d->current_block = 0;
    d->current_file = 0;
    d->current_valid = 1;
    d->last_pos = -1;	/* we skipped backwards, so this can't be valid */
    /*
    ** we rewind twice in case the silly OS has the 
    ** asynchronous rewind bit turned on, in which case 
    ** the second one waits for the first one to complete.
    ** Also, rewinding twice doesn't hurt...
    */
    (void) ftt_mtop(d, 0, FTT_TAPE_REW, FTT_OPN_REWIND,
		"ftt_rewind", ftt_cdb_rewind);
    res2 = ftt_mtop(d, 0, FTT_TAPE_REW, FTT_OPN_REWIND,
	"ftt_rewind", ftt_cdb_rewind);

    /* we cleared unrecoverable errors if we succesfully rewound */
    /* and we're hosed if we didn't */
    d->unrecovered_error = (res2 < 0) ? 2 : 0;
    return res < 0 ? res : res2;
}

int
ftt_retension(ftt_descriptor d) {
    int res, res2;

    CKOK(d,"ftt_retension",0,2);
    CKNULL("ftt_descriptor", d);

    res = ftt_write_fm_if_needed(d);
    d->data_direction = FTT_DIR_READING;
    d->current_block = 0;
    d->current_file = 0;
    d->current_valid = 1;
    res2 = ftt_mtop(d, 0, FTT_TAPE_RETEN, FTT_OPN_RETENSION,
		"ftt_retension", ftt_cdb_retension);

    /* we cleared unrecoverable errors if we succesfully retensioned */
    /* and we're hosed if we didn't */
    d->unrecovered_error = (res2 < 0) ? 2 : 0;
    return res < 0 ? res : res2;
}

int
ftt_unload(ftt_descriptor d){
    int res, res2;

    CKOK(d,"ftt_unload",0,2);
    CKNULL("ftt_descriptor", d);

    d->data_direction = FTT_DIR_READING;
    res = ftt_write_fm_if_needed(d);
    d->current_block = 0;
    d->current_file = 0;
    d->current_valid = 1;
    res2 =  ftt_mtop(d, 0, FTT_TAPE_UNLOAD, FTT_OPN_UNLOAD,
			"ftt_unload", ftt_cdb_unload);

    /* we cleared unrecoverable errors if we succesfully unloaded  */
    /* and we're hosed if we didn't */
    d->unrecovered_error = (res2 < 0) ? 2 : 0;
    return res < 0 ? res : res2;
}

int
ftt_erase(ftt_descriptor d) {
    int res;


    CKOK(d,"ftt_erase",0,2);
    CKNULL("ftt_descriptor", d);

    /* currently erase hoses up on most platforms on most drives,
       due to timeout problems, etc.  So for the first release
       we're punting... */
    ftt_eprintf("Sorry, erase is not functioning properly in this release.");
    return FTT_ENOTSUPPORTED;

    d->current_block = 0;
    d->current_file = 0;
    d->current_valid = 1;

    if ((d->scsi_ops & FTT_OP_ERASE) && (d->flags & FTT_FLAG_SUID_SCSI) 
							&& 0 != geteuid()) {
        ftt_close_dev(d);
        switch(ftt_fork(d)){
        case -1:
                return -1;

        case 0:  /* child */
                fflush(stdout); /* make async_pf stdout */
                fflush(d->async_pf_parent);
                close(1);
                dup2(fileno(d->async_pf_parent),1);
                execlp("ftt_suid", "ftt_suid", "-e", d->basename, 0);

        default: /* parent */
                res = ftt_wait(d);
        }
    } else {
        res =  ftt_mtop(d, 0, FTT_TAPE_ERASE, FTT_OPN_ERASE,
		"ftt_erase", ftt_cdb_erase);
    }

    /* we cleared unrecoverable errors if we succesfully erased  */
    /* and we're hosed if we didn't */
    d->unrecovered_error = (res < 0) ? 2 : 0;
    return res;
}

int
ftt_writefm(ftt_descriptor d) {

    CKOK(d,"ftt_writefm",1,0);
    CKNULL("ftt_descriptor", d);

    if (d->flags & FTT_FLAG_CHK_BOT_AT_FMK) {

	/* 
	** call ftt_status to see if we're at BOT 
	** we should only do this check on machines that don't 
	** need to close the device to get the status.
	** Note that we need to check current_file and current_block
	** *first* because ftt_status will reset them if it notices
	** we're at BOT.
	*/
        (void)ftt_mtop(d, 0, FTT_TAPE_WEOF, FTT_OPN_WRITEFM,
		"write filemark 0 == flush", ftt_cdb_writefm);

	if ((d->current_file != 0 || d->current_block > 2) &&
		(ftt_status(d,0) & FTT_ABOT)) {
	    ftt_errno = FTT_EUNRECOVERED;
	    ftt_eprintf(
"ftt_writefm: supposed to be at file number %d block number %d, actually at BOT\n\
	indicating that there was a SCSI reset or other error which rewound\n\
	the tape behind our back.", d->current_file, d->current_block );
	    d->unrecovered_error = 2;
	    d->nresets++;
	    return -1;
	}
    }
    d->data_direction = FTT_DIR_WRITING;
    d->current_block = 0;
    d->current_file++;
    return ftt_mtop(d, 1, FTT_TAPE_WEOF, FTT_OPN_WRITEFM,
		"ftt_writefm", ftt_cdb_writefm);
}

int
ftt_skip_to_double_fm(ftt_descriptor d) {
    char *buf;
    int blocksize;
    int res;

    CKOK(d,"ftt_skip_to_double_fm",0,0);
    CKNULL("ftt_descriptor", d);

    blocksize = ftt_get_max_blocksize(d);
    buf = (char *)malloc(blocksize);
    if (buf == 0) {
	ftt_errno = FTT_ENOMEM;
	ftt_eprintf("ftt_skip_to_double_fm: unable to allocate %d byte read buffer, errno %d", blocksize, errno);
	return -1;
    }
	
    ftt_open_dev(d);
    do {
	res = ftt_skip_fm(d,1);		   if(res < 0) {free(buf);return res;}
	res = ftt_read(d, buf, blocksize); if(res < 0) {free(buf);return res;}
    } while ( res > 0 );
    /* res == 0 so we got an end of file after a skip filemark... */
    free(buf);
    return ftt_skip_fm(d,-1);
}

int
ftt_write_fm_if_needed(ftt_descriptor d) {
    int res=0;
    int savefile, saveblock, savedir;

    CKOK(d,"ftt_write_fm_if_needed",0,0);
    CKNULL("ftt_descriptor", d);

    if (FTT_OP_WRITE == d ->last_operation ||
	    FTT_OP_WRITEFM == d ->last_operation ) {

	savefile = d->current_file;
	saveblock = d->current_block;
	savedir = d->data_direction;
	DEBUG3(stderr,"Writing first filemark...\n");
	res = ftt_writefm(d); 			if (res < 0) { return res; } 
	DEBUG3(stderr,"Writing second filemark...\n");
	res = ftt_writefm(d); 			if (res < 0) { return res; }
        DEBUG3(stderr,"skipping -2 filemarks...\n");
        res = ftt_skip_fm_internal(d, -2);	if (res < 0) { return res; }
	d->last_operation = FTT_OP_SKIPFM;
	d->current_file = savefile;
	d->current_block = saveblock;
	d->data_direction = savedir;
    }
    return 0;
}

int
ftt_write2fm(ftt_descriptor d) {

    int res;
    CKOK(d,"ftt_write2fm",1,0);
    CKNULL("ftt_descriptor", d);

    if (d->flags & FTT_FLAG_CHK_BOT_AT_FMK) {

	/* 
	** call ftt_status to see if we're at BOT 
	** we should only do this check on machines that don't 
	** need to close the device to get the status.
	** Note that we need to check current_file and current_block
	** *first* because ftt_status will reset them if it notices
	** we're at BOT.
	*/
        (void)ftt_mtop(d, 0, FTT_TAPE_WEOF, FTT_OPN_WRITEFM,
		"write filemark 0 == flush", ftt_cdb_writefm);

	if ((d->current_file != 0 || d->current_block > 2) &&
		(ftt_status(d,0) & FTT_ABOT)) {
	    ftt_errno = FTT_EUNRECOVERED;
	    ftt_eprintf(
"ftt_write2fm: supposed to be at file number %d block number %d, actually at BOT\n\
	indicating that there was a SCSI reset or other error which rewound\n\
	the tape behind our back.", d->current_file, d->current_block );
	    d->unrecovered_error = 2;
	    d->nresets++;
	    return -1;
	}
    }
    d->data_direction = FTT_DIR_WRITING;
    d->current_block = 0;
    d->current_file += 2;
    res = ftt_mtop(d, 2, FTT_TAPE_WEOF, FTT_OPN_WRITEFM,
		"ftt_write2fm", ftt_cdb_writefm);
    /* we've done a double filemark, so we can forget we were writing */
    /* (see check in ftt_write_fm_if_needed, above) */
    d->last_operation = 0;
    return res;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               home/vsergeev/ftt/ftt_lib/ftt_stats.c                                                               0100640 0022366 0022366 00000104762 10037773175 020526  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ftt_private.h>
#include <ftt_dbd.h>

#ifdef WIN32
#include <io.h>
#include <process.h>
#include <windows.h>
#include <winioctl.h>

#define geteuid() -1
DWORD ftt_win_get_paramters();

#else
#include <unistd.h>
#endif

int ftt_open_io_dev();

extern int errno;

char *
ftt_get_prod_id(ftt_descriptor d) {
    return d->prod_id;
}

ftt_stat_buf
ftt_alloc_stat() {
    void *res;

    ENTERING("ftt_alloc_stat");
    res =  malloc(sizeof(ftt_stat));
    if (0 != res) {
	memset(res,0,sizeof(ftt_stat));
	return res;
    } else {
	ftt_eprintf("ftt_alloc_stat: unable to allocate statistics buffer, errno %d\n", errno);
	ftt_errno = FTT_ENOMEM;
	return res;
    }
}

ftt_statdb_buf
ftt_alloc_statdb() {
    void *res;

    ENTERING("ftt_alloc_statdb");
    res = malloc(sizeof(ftt_statdb));
    if (0 != res) {
        memset(res,0,sizeof(ftt_statdb));
        return res;
    } else {
	ftt_eprintf("ftt_alloc_stat: unable to allocate statistics buffer, errno %d\n", errno);
	ftt_errno = FTT_ENOMEM;
        return res;
    }
}

int
ftt_free_stat(ftt_stat_buf b) {
    int i;

    ENTERING("ftt_free_stat");
    CKNULL("statistics buffer pointer", b);

    for (i = 0; i < FTT_MAX_STAT; i++) {
	if (b->value[i]) {
		free(b->value[i]);
		b->value[i] = 0;
	}
    }
    free(b);
    return 0;
}

int 
ftt_free_statdb(ftt_statdb_buf b) {
    int i;
    ENTERING("ftt_free_statdb");
    CKNULL("statistics DB buffer pointer", b);

    for (i = 0; i <FTT_MAX_NUMDB; i++) {
        if (b->value[i]) {
           free(b->value[i]);
           b->value[i] = 0;
        }
    }
    free(b);
    return 0;
}

#ifdef WIN32
	static char *
	ftt_itoa_Large(LARGE_INTEGER n) {
		static char buf[128];
		sprintf(buf,"%ld", n.LowPart);
		return buf;
	}
#endif

char *
ftt_itoa(long n) {
	static char buf[128];

	sprintf(buf,"%ld", n);
	return buf;
}

static char *
ftt_dtoa(double n) {
	static char buf[128];

	sprintf(buf,"%.0f", n);
	return buf;
}

/* set_stat
**
** handy routine to fill in the n-th slot in the stat buffer
*/
static void
set_stat( ftt_stat_buf b, int n, char *pcStart, char *pcEnd) {
    char save = 'n';
    int do_freeme = 0;
	char *freeme;

    /* clean out old value */
    if (b->value[n] != 0) {
		do_freeme = 1;
		freeme = b->value[n];
		b->value[n] = 0;
    }

    /* if null, leave it */
    if (pcStart != 0) {
	
		/* null terminate at pcEnd, copy the string, 
		** and then put the byte back where we scribbled the null
		** ... after eating blanks off the end
		*/
		if (pcEnd == 0) {
		pcEnd = pcStart + strlen(pcStart); /* pcEnd = pcStart if string has ) length */
		}
		if ( *pcEnd ) {
		  do {
		    pcEnd--;
		  } while(*pcEnd == ' ');
		  pcEnd++;
		  save = *pcEnd;
		  *pcEnd = 0;
		}
	DEBUG3(stderr,"Setting stat %d(%s) to %s\n",n,ftt_stat_names[n],pcStart);	
		b->value[n] = strdup(pcStart);
		/* why is this "if" here??? I forget now... mengel*/
		if ( save != 'n' ) *pcEnd = save;
	}
    if (do_freeme) {
         free(freeme);
    }
}

int ftt_numeric_tab[FTT_MAX_STAT] = {
    /*  FTT_VENDOR_ID		0 * 0,*/4,/*0-3  header for dump DB*/ 
    /*  FTT_PRODUCT_ID		1 * 0,*/4,
    /*  FTT_FIRMWARE		2 * 0,*/4,
    /*  FTT_SERIAL_NUM		3 * 0,*/4,
    /*  FTT_CLEANING_BIT	5*/ 0,
    /*  FTT_READ_COUNT		6*/ 1,
    /*  FTT_WRITE_COUNT		7*/ 1,
    /*  FTT_READ_ERRORS		8*/ 1,
    /*  FTT_WRITE_ERRORS	9*/ 1,
    /*  FTT_READ_COMP		11*/ 0,
    /*  FTT_FILE_NUMBER		12*/ 0,
    /*  FTT_BLOCK_NUMBER	13*/ 0,
    /*  FTT_BOT			14*/ 0,
    /*  FTT_READY		15*/ 0,
    /*  FTT_WRITE_PROT		16*/ 0,
    /*  FTT_FMK			17*/ 0,
    /*  FTT_EOM			18*/ 0,
    /*  FTT_PEOT		19*/ 0,
    /*  FTT_MEDIA_TYPE		20*/ 0,
    /*  FTT_BLOCK_SIZE		21*/ 0,
    /*  FTT_BLOCK_TOTAL		22*/ 0,
    /*  FTT_TRANS_DENSITY	23*/ 0,
    /*  FTT_TRANS_COMPRESS	24*/ 0,
    /*  FTT_REMAIN_TAPE		25   0,*/4,
    /*  FTT_USER_READ		26*/ 1,
    /*  FTT_USER_WRITE		27*/ 1,
    /*  FTT_CONTROLLER		28*/ 0,
    /*  FTT_DENSITY		29   0,*/4,
    /*  FTT_ILI			30*/ 0,
    /*  FTT_SCSI_ASC		31*/ 0,
    /*  FTT_SCSI_ASCQ		32*/ 0,
    /*  FTT_PF			33*/ 0,
    /*  FTT_CLEANED_BIT	        34*/ 0,
    /*  FTT_WRITE_COMP		35*/ 0,
    /*  FTT_TRACK_RETRY		36*/ 0,
    /*  FTT_UNDERRUN		37*/ 1,
    /*  FTT_MOTION_HOURS	38*/ 1,
    /*  FTT_POWER_HOURS		39*/ 1,
    /*  FTT_TUR_STATUS		40*/ 0,
    /*  FTT_BLOC_LOC		41*/ 1,
    /*  FTT_COUNT_ORIGIN	42*/ 0,
    /*  FTT_N_READS		43*/ 1,
    /*  FTT_N_WRITES		44*/ 1,
    /*  FTT_TNP			45*/ 0,
    /*  FTT_SENSE_KEY		46*/ 0,
    /*  FTT_TRANS_SENSE_KEY	47*/ 0,
    /*  FTT_RETRIES		48*/ 1,
    /*  FTT_FAIL_RETRIES	49*/ 1,
    /*  FTT_RESETS		50*/ 1,
    /*  FTT_HARD_ERRORS		51*/ 1,
    /*  FTT_UNC_WRITE		50*/ 1,
    /*  FTT_UNC_READ		51*/ 1,
    /*  FTT_CMP_WRITE		52*/ 1,
    /*  FTT_CMP_READ		53*/ 1,
    /*  FTT_ERROR_CODE		54*/ 0,
    /*  FTT_CUR_PART		54*/ 0,
    /*  FTT_MOUNT_PART		54*/ 0,
};

void
ftt_add_stats(ftt_stat_buf b1, ftt_stat_buf b2, ftt_stat_buf res){
    int i;

    ENTERING("ftt_add_stats");
    VCKNULL("statistics buffer pointer 1", b1);
    VCKNULL("statistics buffer pointer 2", b2);
    VCKNULL("statistics buffer pointer 3", res);

    for( i = 0; i < FTT_MAX_STAT; i++) {
        if( ftt_numeric_tab[i] && b1->value[i] && b2->value[i] ) {
 	    set_stat(res, i, ftt_itoa((long)atoi(b1->value[i]) + atoi(b2->value[i])),0);
        } else if ( b2->value[i] ) {
 	    set_stat(res, i, b2->value[i],0);
        } else if ( b1->value[i] ) {
 	    set_stat(res, i, b1->value[i],0);
	}
    }
}

void
ftt_sub_stats(ftt_stat_buf b1, ftt_stat_buf b2, ftt_stat_buf res){
    int i;

    ENTERING("ftt_sub_stats");
    VCKNULL("statistics buffer pointer 1", b1);
    VCKNULL("statistics buffer pointer 2", b2);
    VCKNULL("statistics buffer pointer 3", res);

    for( i = 0; i < FTT_MAX_STAT; i++) {
        if( ftt_numeric_tab[i] && b1->value[i] && b2->value[i] ) {
 	    set_stat(res, i, ftt_itoa((long)atoi(b1->value[i]) - atoi(b2->value[i])),0);
        } else if ( b1->value[i] ) {
 	    set_stat(res, i, b1->value[i],0);
        } else if ( b2->value[i] && ftt_numeric_tab[i] ) {
 	    set_stat(res, i, ftt_itoa(-(long)atoi(b2->value[i])),0);
        } else if ( b2->value[i] ) {
 	    set_stat(res, i, b2->value[i],0);
	}
    }
}

int
ftt_get_statdb (ftt_descriptor d, ftt_statdb_buf b) {
    ftt_stat_buf res1;
    int ires1, i, j = 0;
    char *stat_value;

    ENTERING("ftt_get_statdb");
    VCKNULL("statistics buffer pointer 1", b);

    res1 = ftt_alloc_stat();
    ires1 = ftt_get_stats(d, res1);
    for (i = 0; ftt_stat_names[i] != 0; i++) {

        if (ftt_numeric_tab[i]) {
           stat_value = ftt_extract_stats(res1, i);
           b ->value[j] = stat_value;
           j++;
        }
    }
    return ires1;
}



/*
** handy macros to increase readability:
** pack -- smoosh 4 bytes into an int, msb to lsb
** bit  -- return the nth bit of a byte
*/
#define pack(a,b,c,d) \
     (((unsigned long)(a)<<24) + ((unsigned long)(b)<<16) + ((unsigned long)(c)<<8) + (unsigned long)(d))

#define bit(n,byte) (unsigned long)(((byte)>>(n))&1)

/*
** unpack_ls does the error and total kb for read and write data
** you pass in the statbuf pointer, the log sense data buffer
** and the page code and statistic number for each.
*/
static double
decrypt_ls(ftt_stat_buf b,unsigned char *buf, int param, int stat, double divide) {
    static char printbuf[128];
    unsigned char *page;
    int thisparam, thislength;
    int i;
    int length;
    double value;

    DEBUG3(stderr,"entering decrypt_ls for parameter %d stat %d\n", param, stat);
    page = buf + 4;
    length = pack(0,0,buf[2],buf[3]);
    DEBUG3(stderr,"decrypt_ls: length is %d \n", length);
    while( page < (buf + length) ) {
	thisparam = pack(0,0,page[0],page[1]);
	thislength = page[3];
	value = 0.0;
        if (thislength < 8) {
	    for(i = 0; i < thislength ; i++) {
		value = value * 256 + page[4+i];
	    }
        }
	DEBUG3(stderr, "parameter %d, length %d value %g\n", thisparam, thislength, value);
	if ( thisparam == param ) {
	    if ( value / divide > 1.0e+127 ) {
		/* do something if its really huge... */
		sprintf(printbuf, "%g", value / divide );
	    } else {
		sprintf(printbuf, "%.0f", value / divide);
            }
          if ( (buf[0] != 0x32) || ((buf[0] == 0x32) && (param == 0 || param == 1)))
          {
	    set_stat(b,stat,printbuf,0);
	    DEBUG3(stderr," stat %d - value %s = %g \n",stat,printbuf,value / divide);
          }
            return;
	}
	page += 4 + thislength;
    }
}

int
ftt_get_stat_ops(char *name) {
    int i;

    DEBUG4(stderr, "Entering: get_stat_ops\n");
    if (*name == 0) {
	return 0; /* unknown device id */
    }

    for (i = 0; ftt_stat_op_tab[i].name != 0; i++ ) {
	if (ftt_matches(name, ftt_stat_op_tab[i].name)) {
            DEBUG2(stderr, "found stat_op == %x\n", i);
	    return ftt_stat_op_tab[i].stat_ops;
	}
    }
    return 0;
}

int
ftt_get_stats(ftt_descriptor d, ftt_stat_buf b) {
    int res;

    int current_partition = 0;

#ifndef WIN32
    int hwdens;
    int failures = 0;
    int i;
    unsigned char buf[2048];
    long int tape_size, error_count, data_count;
    double remain_tape;
    int n_blocks, block_length;
    int stat_ops;
#endif

    ENTERING("ftt_get_stats");
    CKNULL("ftt_descriptor", d);
    CKNULL("statistics buffer pointer", b);


    memset(b,0,sizeof(ftt_stat));


    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {
	ftt_close_dev(d);
	switch(ftt_fork(d)){
	case -1:
		return -1;

	case 0:  /* child */
		fflush(stdout);	/* make async_pf stdout */
		fflush(d->async_pf_parent);
		close(1);
		dup2(fileno(d->async_pf_parent),1);
		if (d->data_direction == FTT_DIR_WRITING) {
		     if (ftt_debug) {
		         execlp("ftt_suid", "ftt_suid", "-x",  "-w", "-s", d->basename, 0);
		     } else {
		          execlp("ftt_suid", "ftt_suid", "-w", "-s", d->basename, 0);
		     }
		} else {
		     if (ftt_debug) {
		         execlp("ftt_suid", "ftt_suid", "-x", "-s", d->basename, 0);
		     } else {
		         execlp("ftt_suid", "ftt_suid", "-s", d->basename, 0);
		     }
		}

	default: /* parent */
		ftt_undump_stats(b,d->async_pf_child);
		res = ftt_wait(d);
	}
    }

    /* Things we know without asking, and the suid program won't know */
    set_stat(b,FTT_FILE_NUMBER, ftt_itoa((long)d->current_file), 0);
    set_stat(b,FTT_BLOCK_NUMBER, ftt_itoa((long)d->current_block), 0);
    set_stat(b,FTT_USER_READ,ftt_itoa((long)d->readkb), 0);
    set_stat(b,FTT_USER_WRITE,ftt_itoa((long)d->writekb), 0);
    set_stat(b,FTT_N_READS,ftt_itoa((long)d->nreads), 0);
    set_stat(b,FTT_N_WRITES,ftt_itoa((long)d->nwrites), 0);
    set_stat(b,FTT_RETRIES,ftt_itoa((long)d->nretries), 0);
    set_stat(b,FTT_FAIL_RETRIES,ftt_itoa((long)d->nfailretries), 0);
    set_stat(b,FTT_RESETS,ftt_itoa((long)d->nresets), 0);
    set_stat(b,FTT_HARD_ERRORS,ftt_itoa((long)d->nharderrors), 0);

#ifndef WIN32

    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {
	return res;
    }

    set_stat(b,FTT_CONTROLLER,d->controller, 0);

    if (d->current_valid==1) {
        /* we think we know where we are */
        if (d->current_block == 0 && d->current_file == 0) {
	    set_stat(b,FTT_BOT,"1",0);
	} else {
	    set_stat(b,FTT_BOT,"0",0);
	}
    }

    /* various mode checks */
    if (0 == strncmp(d->controller,"SCSI",4)) {
        stat_ops = FTT_DO_TUR|FTT_DO_RS|FTT_DO_INQ;
    } else {
        stat_ops = 0;
    }

    /*
    ** First do a request sense, and check for any error conditions
    ** etc. that an inquiry might clear.
    ** Then we'll do an inquiry, and find out what kind of drive
    ** this *really* is, then *another* request sense, and check
    ** for drive specific data.
    */
    if (stat_ops & FTT_DO_TUR) {
        static unsigned char cdb_tur[]	     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	res = ftt_do_scsi_command(d,"Test Unit Ready", cdb_tur, 6, 0, 0, 10, 0);
	set_stat(b,FTT_TUR_STATUS,ftt_itoa((long)-res), 0);
	if (res < 0) {
	    set_stat(b,FTT_READY,"0",0);
	    set_stat(b,FTT_TNP,"1",0);
	} else {
	    set_stat(b,FTT_READY,"1",0);
	}
    }

    if (stat_ops & FTT_DO_RS) {
	static unsigned char cdb_req_sense[] = {0x03, 0x00, 0x00, 0x00,   18, 0x00};

        /* 
	** if the request sense data from the test unit ready is negative 
        ** then ftt_do_scsi_command already did a request sense, and
        ** the data is in the global ftt_sensebuf.
	** otherwise we need to do one...
        */
  
        if (res < 0) {
            extern char ftt_sensebuf[18];
	    memcpy(buf, ftt_sensebuf, 18);   
	    res = 0;
        } else {
	    res = ftt_do_scsi_command(d,"Req Sense", cdb_req_sense, 6, buf, 18, 10, 0);
        }
	if(res < 0){
	    failures++;
	} else {
	    static char *sense_key_trans[] = {
		"NO_SENSE", "NOT_USED", "NOT_READY", "MEDIUM_ERROR",
		"HARDWARE_ERROR", "ILLEGAL_REQUEST", "UNIT_ATTENTION",
		"DATA_PROTECT", "BLANK_CHECK", "EXABYTE", "COPY_ABORTED",
		"ABORTED_COMMAND", "NOT_USED", "VOLUME_OVERFLOW",
		"NOT_USED", "RESERVED",
	    };
	    set_stat(b,FTT_ERROR_CODE, ftt_itoa((long)buf[0]&0xf), 0);
	    set_stat(b,FTT_SENSE_KEY, ftt_itoa((long)buf[2]&0xf), 0);
	    set_stat(b,FTT_TRANS_SENSE_KEY, sense_key_trans[buf[2]&0xf], 0);
	    set_stat(b,FTT_FMK, ftt_itoa((long)bit(7,buf[2])), 0);
	    set_stat(b,FTT_EOM, ftt_itoa((long)bit(6,buf[2])),0);
	    set_stat(b,FTT_ILI, ftt_itoa((long)bit(5,buf[2])),0);
	    set_stat(b,FTT_SCSI_ASC,ftt_itoa((long)buf[12]),0);
	    set_stat(b,FTT_SCSI_ASCQ,ftt_itoa((long)buf[13]),0);

	    /* ASC/ASCQ data parsing
	    **
	    ** these are the codes from the DLT book, because 
	    ** it appears from the book that we may sometimes
	    ** get them filled in with a sense code of 0 to
	    ** indicate end of tape, etc.
	    **
	    ** it is not clear that this has ever actually happened,
	    ** but we wanted to be complete.
	    */
	    switch( pack(0,0,buf[12],buf[13]) ) {
	    case 0x0005: /* peot */
			set_stat(b,FTT_PEOT,"1",0);
			break;
	    case 0x0400: /* volume not mounted */
	    case 0x0401: /* rewinding or loading */
	    case 0x0402: /* load needed */
	    case 0x0403: /* manual intervention needed */
	    case 0x3a00: /* medium not present */
	    case 0x3a80: /* cartridge not present */
			set_stat(b,FTT_READY,"0",0);
			set_stat(b,FTT_TNP,"1",0);
			break;
	    case 0x0002: /* EOM encountered */
			set_stat(b,FTT_EOM,"1",0);
			break;
	    case 0x0004:
			set_stat(b,FTT_BOT,"1",0);
			break;
	    case 0x8002:
			set_stat(b,FTT_CLEANING_BIT,"1",0);
			break;
	    }
	}
    }

    if (stat_ops & FTT_DO_INQ) {
	static unsigned char cdb_inquiry[]   = {0x12, 0x00, 0x00, 0x00,   56, 0x00};

	/* basic scsi inquiry */
	res = ftt_do_scsi_command(d,"Inquiry", cdb_inquiry, 6, buf, 56, 10, 0);
	if(res < 0){
	    failures++;
	} else {
	    set_stat(b,FTT_VENDOR_ID,  (char *)buf+8,  (char *)buf+16);
	    set_stat(b,FTT_PRODUCT_ID, (char *)buf+16, (char *)buf+32);
/*	    set_stat(b,FTT_FIRMWARE,   (char *)buf+32, (char *)buf+35);             */
	    if ( 0 != strcmp(d->prod_id, ftt_extract_stats(b,FTT_PRODUCT_ID))) {
		char *tmp;

		/* update or product id and stat_ops if we were wrong */

		tmp = d->prod_id;
		d->prod_id = strdup(ftt_extract_stats(b,FTT_PRODUCT_ID));
		free(tmp);
	    }
/***********************************************************************
*         different drives has different length of a Product Revision Number
*         for STK T9940A/B it takes bytes 32-39
*/
                if ((d->prod_id[1] == '9') && (d->prod_id[3] == '4')) {
                    set_stat(b,FTT_FIRMWARE,   (char *)buf+32, (char *)buf+40);
                 } else {
                    set_stat(b,FTT_FIRMWARE,   (char *)buf+32, (char *)buf+36);
                   }

	    /*
	     * look up based on ANSI version *and* product id, so
	     * we can have generic SCSI-2 cases, etc.
	     */
	   sprintf(buf, "%d%s", buf[2] & 0x3, ftt_unalias(d->prod_id)); 
	    stat_ops = ftt_get_stat_ops(buf);
	}
    }

    /* 
    ** Get other vendor specific request sense available data now that we know for sure
    ** what kind of drive this is from the inquiry data.
    */

    if (stat_ops & FTT_DO_VSRS) {
	static unsigned char cdb_req_sense[] = {0x03, 0x00, 0x00, 0x00,   32, 0x00};

	/* request sense data */
	res = ftt_do_scsi_command(d,"Req Sense", cdb_req_sense, 6, buf, 32, 10, 0);
	if(res < 0){
	    failures++;
	} else {
	    if (stat_ops & FTT_DO_EXBRS) {
		set_stat(b,FTT_BOT,         ftt_itoa((long)bit(0,buf[19])), 0);
		set_stat(b,FTT_TNP,	    ftt_itoa((long)bit(1,buf[19])), 0);
		set_stat(b,FTT_PF,          ftt_itoa((long)bit(7,buf[19])), 0);
		set_stat(b,FTT_WRITE_PROT,  ftt_itoa((long)bit(5,buf[20])), 0);
		set_stat(b,FTT_PEOT,        ftt_itoa((long)bit(2,buf[21])), 0);
		set_stat(b,FTT_CLEANING_BIT,ftt_itoa((long)bit(3,buf[21])), 0);
		set_stat(b,FTT_CLEANED_BIT, ftt_itoa((long)bit(4,buf[21])), 0);

		remain_tape=(double)pack(0,buf[23],buf[24],buf[25]);
		error_count = pack(0,buf[16],buf[17],buf[18]);

		if (d->prod_id[5] == '9') {
		     
                     DEBUG2(stderr, "remain_tape 8900 case... \n");
		     /* 8900's count 16k blocks, not 1k blocks */
		     remain_tape *= 16.0;

		} else if (d->prod_id[5] == 't') {
		     
                     DEBUG2(stderr, "remain_tape Mammoth2 case... \n");
		     /* 8900's count 16k blocks, not 1k blocks */
		     remain_tape *= 33.0;
		} else {
                     DEBUG2(stderr, "remain_tape non-8900 case... \n");
		     ;
		}
		set_stat(b,FTT_REMAIN_TAPE,ftt_dtoa(remain_tape),0);
		if (d->data_direction ==  FTT_DIR_READING) {
	            set_stat(b,FTT_READ_ERRORS,ftt_itoa((long)error_count),0);
		} else {
	            set_stat(b,FTT_WRITE_ERRORS,ftt_itoa((long)error_count),0);
		}
            }
	    if (stat_ops & FTT_DO_05RS) {
		set_stat(b,FTT_TRACK_RETRY, ftt_itoa((long)buf[26]), 0);
		set_stat(b,FTT_UNDERRUN,    ftt_itoa((long)buf[11]), 0);
	    }
	    if (stat_ops & FTT_DO_DLTRS) {
		set_stat(b,FTT_MOTION_HOURS,ftt_itoa((long)pack(0,0,buf[19],buf[20])),0);
		set_stat(b,FTT_POWER_HOURS, ftt_itoa((long)pack(buf[21],buf[22],buf[23],buf[24])),0);
                set_stat(b,FTT_REMAIN_TAPE, ftt_dtoa((double)pack(buf[25],buf[26],buf[27],buf[28])*4),0);  
	    }
	    if (stat_ops & FTT_DO_AITRS) {
		remain_tape=(double)pack(buf[22],buf[23],buf[24],buf[25]);
		set_stat(b,FTT_REMAIN_TAPE,ftt_dtoa(remain_tape),0);
		set_stat(b,FTT_CLEANING_BIT,ftt_itoa((long)bit(3,buf[26])), 0);
                /* AIT sets R.S. remain tape to zero when empty, so
                   if its zero and we're not at EOM, then... */
                if ( remain_tape == 0 && !(buf[2]&0x40) ) {
	           set_stat(b,FTT_READY,"0", 0);
		   set_stat(b,FTT_TNP,  "1", 0);
                } else {
		   set_stat(b,FTT_TNP,  "0", 0);
                }
            }
	}
    }
    if (stat_ops & FTT_DO_SN) {
        static unsigned char cdb_inq_w_sn[]  = {0x12, 0x01, 0x80, 0x00,   14, 0x00};

	/* scsi inquiry w/ serial number */
	res = ftt_do_scsi_command(d,"Inquiry", cdb_inq_w_sn, 6, buf, 14, 10, 0);
	if(res < 0){
	    failures++;
	} else {
	    set_stat(b,FTT_SERIAL_NUM, (char *)buf+4, (char *)buf+14);
	}
    }
    if (stat_ops & FTT_DO_MS) {

	static unsigned char cdb_mode_sense[]= {0x1a, 0x00, 0x00, 0x00,   12, 0x00};

	res = ftt_do_scsi_command(d,"mode sense",cdb_mode_sense, 6, buf, 12, 10, 0);
	if (res == -2) {
	    /* retry on a CHECK CONDITION, it may be okay */
	    res = ftt_do_scsi_command(d,"mode sense",cdb_mode_sense, 6, buf, 12, 10, 0);
	}
	if(res < 0){
	    failures++;
	} else {

	    hwdens = buf[4];
	    set_stat(b,FTT_DENSITY,  ftt_itoa((long)hwdens), 0);
	    set_stat(b,FTT_WRITE_PROT,  ftt_itoa((long)bit(7,buf[2])),0);
	    set_stat(b,FTT_MEDIA_TYPE,  ftt_itoa((long)buf[1]), 0);


	    n_blocks =     pack(0,buf[5],buf[6],buf[7]);
	    block_length = pack(0,buf[9],buf[10],buf[11]);
                if (d->prod_id[5] == '9') {
                     DEBUG2(stderr, "total block 8900 case... \n");
                     /* 8900's count 16k blocks, not 1k blocks */
                     n_blocks *= 16.0;

                } else if (d->prod_id[5] == 't') {

                     DEBUG2(stderr, "total bloks Mammoth2 case... \n");
                     /* Mammoth's count 33k blocks, not 1k blocks */
                     n_blocks *= 33.0;
                } else {
                     DEBUG2(stderr, "remain_tape non-8900 case... \n");
                     ;
}
	    tape_size =    n_blocks;

	    if (0 == b->value[FTT_TNP]) {
		if (hwdens == 0) {
		   set_stat(b,FTT_TNP,  "1", 0);
		} else {
		   set_stat(b,FTT_TNP,  "0", 0);
		}
            }

	    set_stat(b,FTT_BLOCK_SIZE,  ftt_itoa((long)block_length),0);
	    set_stat(b,FTT_BLOCK_TOTAL, ftt_itoa((long)n_blocks),    0);

	    if (stat_ops & FTT_DO_EXBRS) {
		/* 
		** the following lies still allow reasonable results
		** from doing before/after deltas
		** we'll override them with log sense data if we have it.
		** The following is a fudge factor for the amount of
		** tape thats shows as the difference between tape size
		** and remaining tape on an EXB-8200 when rewound
		*/
#define 	EXB_8200_FUDGE_FACTOR 1279
		
		if (stat_ops & FTT_DO_EXB82FUDGE) {
			data_count = tape_size - remain_tape - EXB_8200_FUDGE_FACTOR;
		} else {
			data_count = tape_size - remain_tape;
		}
		if (d->data_direction ==  FTT_DIR_READING) {
		    set_stat(b,FTT_READ_COUNT,ftt_itoa(data_count),0);
		} else {
		    set_stat(b,FTT_WRITE_COUNT,ftt_itoa(data_count),0);
		}
		set_stat(b,FTT_COUNT_ORIGIN,"Exabyte_Extended_Sense",0);
	    }

	    for ( i = 0; d->devinfo[i].device_name !=0 ; i++ ) {
		if( buf[4] == d->devinfo[i].hwdens ) {
		    set_stat(b,FTT_TRANS_DENSITY, ftt_itoa((long)d->devinfo[i].density),0);
		    set_stat(b,FTT_TRANS_COMPRESS, ftt_itoa((long)d->devinfo[i].mode),0);
		    break;
		}
	    }
	}
    }
    if (stat_ops & FTT_DO_MS_Px0f) {
	static unsigned char cdb_mode_sense_p09[]= 
			{ 0x1a, 0x08, 0x0f, 0x00,   20, 0x00};

	res = ftt_do_scsi_command(d,"mode sense",cdb_mode_sense_p09, 
				  6, buf, 20, 10, 0);
	if(res < 0){
	    ftt_errno = FTT_EPARTIALSTAT;
	} else {
	    set_stat(b,FTT_TRANS_COMPRESS,     ftt_itoa((long)((buf[4+2]>>7)&1)), 0);
	}
    }
    if (stat_ops & FTT_DO_MS_Px10) {

	static unsigned char cdb_mode_sense_p10[]= 
			{ 0x1a, 0x08, 0x10, 0x00,   20, 0x00};


	res = ftt_do_scsi_command(d,"mode sense",cdb_mode_sense_p10, 
				  6, buf, 20, 10, 0);
	if(res < 0){
	    ftt_errno = FTT_EPARTIALSTAT;
	} else {
	    set_stat(b,FTT_TRANS_COMPRESS,     ftt_itoa((long)buf[4+14]), 0);
	}
	
    }
    if (stat_ops & FTT_DO_MS_Px20_EXB && hwdens == 0) {
	static unsigned char cdb_mode_sense_p20[]= 
			{ 0x1a, 0x08, 0x20, 0x00, 0x0a, 0x00};

	res = ftt_do_scsi_command(d,"mode sense",cdb_mode_sense_p20, 
				  6, buf, 20, 10, 0);
	if(res < 0){
	    ftt_errno = FTT_EPARTIALSTAT;
	} else {
	    set_stat(b,FTT_TRANS_DENSITY,     ftt_itoa(!bit(5,buf[7])), 0);
	    set_stat(b,FTT_TRANS_COMPRESS,    ftt_itoa( bit(6,buf[7])), 0);
	}
    }
    if (stat_ops & FTT_DO_RP || stat_ops & FTT_DO_RP_SOMETIMES) {
	static unsigned char cdb_read_position[]= {0x34, 0x00, 0x00, 0x00, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00};

	res = ftt_do_scsi_command(d,"Read Position", cdb_read_position, 10, 
				  buf, 20, 10, 0);
	
        /* 
        ** 850x drives (for example) don't do RP in lower densities, so if it
        ** fails there its not really a failure
        */
	if (!(stat_ops & FTT_DO_RP_SOMETIMES) && res < 0) {
	    failures++;
	} 
        if ( res >= 0 ) {
	    set_stat(b,FTT_BOT,     ftt_itoa(bit(7,buf[0])), 0);
	    set_stat(b,FTT_PEOT,    ftt_itoa(bit(6,buf[0])), 0);
	    set_stat(b,FTT_BLOC_LOC,ftt_itoa(pack(buf[4],buf[5],buf[6],buf[7])),0);
	    set_stat(b,FTT_CUR_PART,ftt_itoa(buf[1]),0);
            current_partition = buf[1];
	}
    }
    if (stat_ops & FTT_DO_LS) {
 	int npages;
	static char buf2[1028];
	static unsigned char cdb_log_sense[]= {0x4d, 0x00, 0x00, 0x00, 0x00, 
						   0x00, 0x00, 4, 4, 0};


        /* check supported page list, we want 0x32 or 0x39... */
	cdb_log_sense[2] = 0;
	res = ftt_do_scsi_command(d,"Log Sense", cdb_log_sense, 10, 
				  buf2, 1028, 10, 0);

        npages = pack(0,0,buf2[2],buf2[3]);
	for( i = 0; i <= npages; i++ ) {
	    int do_page;
            int slot;
            int ubytesw, cbytesw, j;
            int ubytesr, cbytesr;
            long umbytesw, cmbytesw, total, blocks;
            long umbytesr, cmbytesr;

	    do_page = buf2[4+i];
            switch( do_page ) {
	       	case 0x02:
		case 0x03:
		case 0x2e:
		case 0x30:
	       	case 0x31: 
		case 0x32:
	       	case 0x39:
		case 0x3c:

		    cdb_log_sense[2] = 0x40 | do_page;
		    res = ftt_do_scsi_command(d,"Log Sense", cdb_log_sense, 10, 
					      buf, 1028, 10, 0);
		    if(res < 0) {
		    	failures++;
		    } else { 
		        switch( do_page ) {

		        case 0x02:
			    (void)decrypt_ls(b,buf,3,FTT_WRITE_ERRORS,1.0);
			    (void)decrypt_ls(b,buf,5,FTT_WRITE_COUNT,1024.0);
			    break;

		        case 0x03:
			    (void)decrypt_ls(b,buf,3,FTT_READ_ERRORS,1.0);
			    (void)decrypt_ls(b,buf,5,FTT_READ_COUNT,1024.0);
			    break;

			case 0x2e:
			    /* stk Tape Alert page */
			    if (0 == strncmp(d->prod_id,"9840",4) ||
                                0 == strncmp(d->prod_id,"T9940A",6) ||
                                0 == strncmp(d->prod_id,"T9940B",6)) {
			    (void)decrypt_ls(b,buf,0x15,FTT_CLEANING_BIT,1.0);
                            }
			    break;

			case 0x30:
			    /* stk 9840 vendor unique */
			    if (0 == strncmp(d->prod_id,"9840",4) ||
                                0 == strncmp(d->prod_id,"T9940A",6) ||
                                0 == strncmp(d->prod_id,"T9940B",6)) {
			    (void)decrypt_ls(b,buf,0x17,FTT_REMAIN_TAPE,0.25);
			    (void)decrypt_ls(b,buf,0x0f,FTT_UNC_READ,1.0);
			    (void)decrypt_ls(b,buf,0x11,FTT_UNC_WRITE,1.0);
			    (void)decrypt_ls(b,buf,0x10,FTT_CMP_READ,1.0);
			    (void)decrypt_ls(b,buf,0x12,FTT_CMP_WRITE,1.0);
                            }
			    break;

		        case 0x31:
			    /* 
			    ** we want the remaining tape of the
                            ** current partition if we can get it. 
			    ** this computes a log sense slot based on
			    ** slot  part  data
			    ** ----  ----  -----
			    **   1      0  remain
			    **   2      1  remain
			    **   3      0  max
			    **   4      1  max
			    **   5      2  remain
			    **   6      3  remain
			    **   7      2  max
			    **   8      3  max
			    **  ...
			    ** 125     62  remain
			    ** 126     63  remain
			    ** 127     62  max
			    ** 128     63  max
			    ** So looking at this, it's easier to compute
			    ** the parameter - 1 and then add 1 to it.
			    ** (i.e. so that 0 -> 0)
			    ** So there are 64/2 chunks, each with 4
			    ** parameters, (2 remain and 2 max).
	                    ** so we take the chunk number which is
			    **   part / 2
			    ** or bit-bashing:
			    **   (part & 0x7f) >> 1
                            ** and convert it to the parameter slot
			    **   ((part & 0x7f) >> 1) << 2)
                            ** which simplifies to
			    **   (part & 0x7f) << 1
			    ** and then or-in the slot in that chunk
		            ** which is either 0 or 1, or the low bit,
			    ** since we never want the max values
			    */

			    if( b->value[FTT_REMAIN_TAPE] == 0) {
			      slot = ((current_partition & 0x7f) << 1) |
				      (current_partition & 0x01);

			      (void)decrypt_ls(b,buf,slot+1,FTT_REMAIN_TAPE,1.0);
                                   blocks = 0;
                                   for (j=0; j < 4; j++) {
                                       blocks = blocks*256 + buf[8+j];
                                   }
                                   blocks = blocks*1000;
                            set_stat(b,FTT_REMAIN_TAPE,ftt_itoa((long)blocks), 0);
                              blocks = 0;
                                   for (j=0; j < 4; j++) {
                                       blocks = blocks*256 + buf[24+j];
                                   }
                                   blocks = blocks*1000;
                               set_stat(b,FTT_BLOCK_TOTAL,ftt_itoa((long)blocks), 0);
			    }
			    break;

		        case 0x32:
			    (void)decrypt_ls(b,buf,0,FTT_READ_COMP,1.0);
			    (void)decrypt_ls(b,buf,1,FTT_WRITE_COMP,1.0);
			    (void)decrypt_ls(b,buf,3,FTT_UNC_READ,1.0);
			    (void)decrypt_ls(b,buf,5,FTT_CMP_READ,1.0);
			    (void)decrypt_ls(b,buf,7,FTT_UNC_WRITE,1.0);
			    (void)decrypt_ls(b,buf,9,FTT_CMP_WRITE,1.0);
                            break;

		        case 0x39: {
			    double uw, ur, cw, cr;
			    uw = decrypt_ls(b,buf,5,FTT_UNC_WRITE,1024.0);
			    ur = decrypt_ls(b,buf,6,FTT_UNC_READ,1024.0);
			    cw = decrypt_ls(b,buf,7,FTT_CMP_WRITE,1024.0);
			    cr = decrypt_ls(b,buf,8,FTT_CMP_READ,1024.0);
                            if (ur != 0.0) {
			       set_stat(b,FTT_READ_COMP,  ftt_itoa((long)(100.0*cr/ur)), 0);
			    } else {
			       set_stat(b,FTT_READ_COMP,  "100", 0);
                            }
                            if (uw != 0.0) {
			       set_stat(b,FTT_WRITE_COMP, ftt_itoa((long)(100.0*cw/uw)), 0);
			    } else {
			       set_stat(b,FTT_WRITE_COMP,  "100", 0);
			    }
			    }
                            break;

			case 0x3c:
			    /* mammoth vendor unique info we want */
			    if (0 == strncmp(d->prod_id,"EXB-89",6) ||
			        0 == strncmp(d->prod_id,"Mammoth",7) ) {
			
			       decrypt_ls(b,buf,8,FTT_POWER_HOURS,60.0);
			       decrypt_ls(b,buf,9,FTT_MOTION_HOURS,60.0);
                            }
			    break;
		        }
                    }
		    break;
             }
	
        }
	
    }
    if (stat_ops & FTT_DO_MS_Px21 ) {
	static unsigned char cdb_ms21[6] = {0x1a, DBD, 0x21, 0x00, 10, 0x00};
        int loadpart;
        
	res = ftt_do_scsi_command(d,"Mode Sense, 0x21", cdb_ms21, 6, buf, 10, 10, 0);
	loadpart = (buf[BD_SIZE+3] >> 1) & 0x3f;
	set_stat(b,FTT_MOUNT_PART,ftt_itoa(loadpart),0);
    }
    /*
    ** if we have stats failures, return EPARTIALSTAT, but only if there's
    ** a tape present -- if we don't have a tape its okay if some stats fail...
    */
    if (failures > 0 && !( b->value[FTT_TNP] && atoi(b->value[FTT_TNP])) ) {
	ftt_eprintf("ftt_get_stats, %d scsi requests failed\n", failures);
	ftt_errno = FTT_EPARTIALSTAT;
	return -1;
    } else {
        return 0;
    }
#else /* this is the WIN32 part */
	{
		DWORD fres,par,pos,pos2;
		
		HANDLE fh ;
		TAPE_GET_MEDIA_PARAMETERS gmp;
		TAPE_GET_DRIVE_PARAMETERS gdp;
		if ( ftt_open_io_dev(d) < 0 ) {
			ftt_eprintf("ftt_get_stats, Device is not opened \n");
				ftt_errno = FTT_EPARTIALSTAT;
				return -1;
		}
		fh = (HANDLE)d->file_descriptor;
		fres = ftt_win_get_paramters(d,&gmp,&gdp);
		if ( fres < 1100 ) {
			set_stat(b,FTT_BLOCK_SIZE,ftt_itoa(gmp.BlockSize),0);
			set_stat(b,FTT_WRITE_PROT,ftt_itoa((int)gmp.WriteProtected),0);
		
			if ( gdp.FeaturesLow & TAPE_DRIVE_TAPE_REMAINING ) {
				set_stat(b,FTT_REMAIN_TAPE, ftt_itoa_Large(gmp.Remaining),0);
			}
			
			set_stat(b,FTT_TRANS_COMPRESS,ftt_itoa(gdp.Compression),0);
			set_stat(b,FTT_TRANS_DENSITY,"0",0); /*this has to be 0*/

		}
		else {
			ftt_eprintf("ftt_get_stats, Getting Media & Drive Parameters Failed \n");
			ftt_errno = FTT_EPARTIALSTAT;
			return -1;
		}
		par = pos = pos = (DWORD)-1;
		fres = GetTapePosition(fh,TAPE_LOGICAL_POSITION,&par,&pos,&pos2);
		if ( pos >= 0 ) { 
			set_stat(b,FTT_BOT,ftt_itoa((pos == 0 )?1:0),0);
		}
	}
#endif
}

int
ftt_clear_stats(ftt_descriptor d) {
    static unsigned char buf[256];
    int stat_ops, res;

    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {
	switch(ftt_fork(d)){
	case -1:
		return -1;

	case 0:  /* child */
		fflush(stdout);	/* make async_pf stdout */
		close(1);
		dup(fileno(d->async_pf_parent));
		return execlp("ftt_suid", "ftt_suid", "-c", d->basename, 0);

	default: /* parent */
		return ftt_wait(d);
	}
    }


    if (0 == strncmp(d->controller,"SCSI",4)) {
        stat_ops = FTT_DO_RS|FTT_DO_INQ;
    } else {
	stat_ops = 0;
    }
    if (stat_ops & FTT_DO_TUR) {
        static unsigned char cdb_tur[]	     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	res = ftt_do_scsi_command(d,"Test Unit Ready", cdb_tur, 6, 0, 0, 10, 0);
	res = ftt_do_scsi_command(d,"Test Unit Ready", cdb_tur, 6, 0, 0, 10, 0);
    }
    if (stat_ops & FTT_DO_INQ) {
	static unsigned char cdb_inquiry[]   = {0x12, 0x00, 0x00, 0x00,   56, 0x00};

	/* double check our id... */
	res = ftt_do_scsi_command(d,"Inquiry", cdb_inquiry, 6, buf, 56, 10, 0);
	buf[32] = 0;
	if ( 0 != strcmp((char *)d->prod_id,(char *)buf+16)) {
	    char *tmp;

	    /* update or product id and stat_ops if we were wrong */
	    tmp = d->prod_id;
	    d->prod_id = strdup((char *)buf+16);
	    free(tmp);
	    stat_ops = ftt_get_stat_ops(d->prod_id);
	}
    }
    if (stat_ops & FTT_DO_EXBRS) {
    	static unsigned char cdb_clear_rs[]  = { 0x03, 0x00, 0x00, 0x00, 30, 0x80 };
	res = ftt_do_scsi_command(d,"Clear Request Sense", cdb_clear_rs, 6, buf, 30, 10, 0);
	if (res < 0) return res;
    }
    if (stat_ops & FTT_DO_LS) {
        static unsigned char cdb_clear_ls[] = { 0x4c, 0x02, 0x40, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0x00};
	res = ftt_do_scsi_command(d,"Clear Request Sense", cdb_clear_ls, 10, 0, 0, 10, 1);
	if (res < 0) return res;
    }
    return res;
}

char *
ftt_extract_stats(ftt_stat_buf b, int stat) {

    ENTERING("ftt_extract_stats");
    PCKNULL("statistics buffer pointer",b);

    if (stat < FTT_MAX_STAT && stat >= 0 ) {
	return b->value[stat];
    } else {
	ftt_eprintf("ftt_extract_stats was given an out of range statistic number.");
	ftt_errno= FTT_EFAULT;
	return 0;
    }
}

int
ftt_extract_statdb(ftt_statdb_buf *b, FILE *pf, int name) {
     int i, stat = -1;

     ENTERING("ftt_extract_statdb");
     CKNULL("statistics db data pointer",b);
     CKNULL("stdio file handle", pf);

     for (i = 0; i <= name; i++) {
          if (ftt_numeric_tab[i] != 0) stat++;
     }

        for (i = 0; i < FTT_MAX_NUMDB; i++) {
            fprintf(pf, "%s\t",b[i]->value[stat]);
        }
        fprintf(pf, "\n");
        return 0;
}

int
ftt_stats_status(ftt_descriptor d, int time_out) {
    static ftt_stat block;
    int res;
    char *p;

    res = ftt_get_stats(d,&block);
    if (res < 0) {
	if (ftt_errno == FTT_EBUSY) {
	    return FTT_BUSY;
	} else {
	    return res;
	}
    }

	while (time_out > 0 ) {
		p = ftt_extract_stats(&block, FTT_READY);
		if ( p && atoi(p)) {
			break;
		}
		sleep(1);
		time_out--;
		res = ftt_get_stats(d,&block);
	}
    res = 0;
    p = ftt_extract_stats(&block, FTT_BOT);
    if ( p && atoi(p)) {
	DEBUG3(stderr,"setting ABOT flag\n");
	res |= FTT_ABOT;
    }
    p = ftt_extract_stats(&block, FTT_EOM);
    if ( p && atoi(p)) {
	DEBUG3(stderr,"setting AEOT flag\n");
	res |= FTT_AEOT;
	res |= FTT_AEW;
    }
    p = ftt_extract_stats(&block, FTT_WRITE_PROT);
    if ( p && atoi(p)) {
	DEBUG3(stderr,"setting PROT flag\n");
	res |= FTT_PROT;
    }
    p = ftt_extract_stats(&block, FTT_READY);
    if ( p && atoi(p)) {
	DEBUG3(stderr,"setting ONLINE flag\n");
	res |= FTT_ONLINE;
    }
    return res;
}
              home/vsergeev/ftt/ftt_lib/ftt_suid.c                                                                0100660 0022366 0022366 00000007277 07006176523 020335  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ftt_private.h>

#ifndef WIN32
#include <unistd.h>
#endif

void
usage(void) {
   fprintf(stderr, "usage: ftt_suid [-w] -s basename	 # print stats\n");
   fprintf(stderr, "       ftt_suid -c basename		 # clear stats\n");
   fprintf(stderr, "       ftt_suid -b arg basename      # set blocksize\n");
   fprintf(stderr, "       ftt_suid -C arg basename      # set compression\n");
   fprintf(stderr, "       ftt_suid -d arg basename      # set density\n");
   fprintf(stderr, "       ftt_suid -p basename          # get/dump parts\n");
   fprintf(stderr, "       ftt_suid -u basename          # undump/write parts\n");
   fprintf(stderr, "       ftt_suid -l blck basename  	 # locate to blk\n");
   fprintf(stderr, "       ftt_suid -L blck prt basename # locate w/partition\n");
   fprintf(stderr, "       ftt_suid -M prt basename      # set mount partition\n");
   fprintf(stderr, "       ftt_suid -A flag basename     # Special AIT init\n");
   exit(-1);
}

int
main(int argc, char **argv) {
	ftt_descriptor d;
	ftt_stat_buf b;
	ftt_partbuf pb;
	int n, res;
	char *pc;
	char *basename;
	char command;
	int  arg, arg2;
	int direction = FTT_DIR_READING;

	if (argc <= 2 || argv[1][0] != '-') {
		usage();
	}

	while(1){
		switch (argv[1][1]) {
		case 'x':
			ftt_debug = 4;
			argv++;
			argc--;
			continue;
		case 'w':
			direction = FTT_DIR_WRITING;
			argv++;
			argc--;
			continue;
		case 'e':
		case 'c':
		case 's': 
		case 'i':
		case 'v':
	        case 'p':
	        case 'u':
			if (argc != 3) {
				usage();
			}
			command = argv[1][1];
			basename = argv[2];
			break;
	        case 'A':
		case 'C':
		case 'b': 
		case 'd': 
		case 'l': 
		case 'M': 
			if (argc != 4) {
				usage();
			}
			command = argv[1][1];
			arg = atoi(argv[2]);
			basename = argv[3];
			break;
		case 'L': 
			if (argc != 5) {
				usage();
			}
			command = argv[1][1];
			arg = atoi(argv[2]);
			arg2 = atoi(argv[3]);
			basename = argv[4];
			break;
		default:
			usage();
		}
		break;
	}

	if (geteuid() != 0) {
		fprintf(stderr,"ftt_suid executable not setuid root!\n");
		exit(-2);
	}

	/* ftt_debug = 3; */
	d = ftt_open(basename,FTT_RDONLY);

	d->data_direction = direction;

	if (0 == d) {
		/* fake an ftt_report to stdout */
		printf("-1\n");
		pc = ftt_get_error(&n);
		printf("%d\n%s\n", n, pc);
		exit(1);
	}

	/* attach our ftt_report() channel to stdout */
	d->async_pf_parent = stdout;

	switch(command){
	case 'i':
		printf("%s\n", d->prod_id );
		res = 0;
		break;
	case 'e':
		res = ftt_erase(d);
		break;
	case 's':
		b = ftt_alloc_stat(); 		if (b == 0) break;
		res = ftt_get_stats(d,b);	if (res < 0) break;
		ftt_dump_stats(b,stdout);
		ftt_free_stat(b);
		break;
	case 'c':
		ftt_clear_stats(d);
		break;
	case 'C':
		res = ftt_set_compression(d,arg);
		break;
	case 'b':
		res = ftt_set_blocksize(d,arg);
		break;
	case 'd':
		ftt_set_mode_dev(d, basename,0);
		res = ftt_set_hwdens(d,arg);
		break;
	case 'v':
		res = ftt_verify_blank(d);
		break;
        case 'p':
		pb = ftt_alloc_parts();
		res = ftt_get_partitions(d,pb);
		ftt_dump_partitions(pb,stdout);
		ftt_free_parts(pb);
		break;
        case 'u':
		pb = ftt_alloc_parts();
		ftt_undump_partitions(pb,stdin);
		res = ftt_write_partitions(d,pb);
		ftt_free_parts(pb);
		break;
        case 'l':
		res = ftt_scsi_locate(d, arg);
		break;
        case 'L':
		res = ftt_locate_part(d, arg, arg2);
		break;
        case 'M':
                ftt_set_mount_partition(d, arg);
		break;
	case 'A':
		pb = ftt_alloc_parts();
		ftt_undump_partitions(pb,stdin);
		res = ftt_format_ait(d,arg,pb);
		ftt_free_parts(pb);
		break;
	}
	ftt_report(d);
	return 0;
}
                                                                                                                                                                                                                                                                                                                                 home/vsergeev/ftt/ftt_lib/ftt_suid_scsi.c                                                           0100660 0022366 0022366 00000000445 06252127501 021336  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
int
ftt_do_suid_scsi_command(ftt_descriptor d,char *pcOp,unsigned char *pcCmd, 
	int nCmd, unsigned char *pcRdWr, int nRdWr, int delay, int iswrite){
    scsi_handle n;
    int res;
    char *devname;
}

                                                                                                                                                                                                                           home/vsergeev/ftt/ftt_lib/ftt_tables.c                                                              0100660 0022366 0022366 00000722147 07777337700 020656  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               #include <stdio.h>
#include <ftt_private.h>

/* drives who masquerade as other SCSI ID's go here. */

ftt_id_alias ftt_alias_table[] = {
    { "AIT-1", "SDX-300C" },
    { "AIT1", "SDX-300C" },
    { "AIT-2", "SDX-500C" },
    { "AIT2", "SDX-500C" },
/*    { "ULT3580-TD1", "ULT06242-XXX" },  */
    { 0,0 },
};

int ftt_trans_open[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_ENOENT,
	/*    5	EIO	*/	FTT_ENOTAPE,
	/*    6	ENXIO	*/	FTT_ENODEV,
	/*    7	E2BIG	*/	FTT_EFAULT,
	/*    8	ENOEXEC	*/	FTT_EFAULT,
	/*    9	EBADF	*/	FTT_ENOENT,
	/*   10	ECHILD	*/	FTT_ENOTSUPPORTED,
	/*   11	EAGAIN	*/	FTT_ENOTAPE,
	/*   12	ENOMEM	*/	FTT_ENOMEM,
	/*   13	EACCES	*/	FTT_EPERM,
	/*   14	EFAULT	*/	FTT_ENOENT,
	/*   15	ENOTBLK	*/	FTT_ENOTSUPPORTED,
	/*   16	EBUSY	*/	FTT_EBUSY,
	/*   17	EEXIST	*/	FTT_ENOTSUPPORTED,
	/*   18	EXDEV	*/	FTT_ENOTSUPPORTED,
	/*   19	ENODEV	*/	FTT_ENODEV,
	/*   20	ENOTDIR	*/	FTT_ENOENT,
	/*   21	EISDIR	*/	FTT_ENODEV,
	/*   22	EINVAL	*/	FTT_ENOENT,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EBLKSIZE,
	/*   28	ENOSPC	*/	FTT_EFAULT,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};

int ftt_trans_in[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EIO,
	/*    6	ENXIO	*/	FTT_ENOSPC,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   22	EINVAL	*/	FTT_ERANGE,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EBLKSIZE,
	/*   28	ENOSPC	*/	FTT_EBLANK,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};

int ftt_trans_in_Linux[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EBLANK,
	/*    6	ENXIO	*/	FTT_ENOSPC,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   22	EINVAL	*/	FTT_ERANGE,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EBLKSIZE,
	/*   28	ENOSPC	*/	FTT_EBLANK,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};
int ftt_trans_in_AIX[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EBLANK,
	/*    6	ENXIO	*/	FTT_ENOSPC,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   31	EMLINK	*/	FTT_EIO,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};

int ftt_trans_skiprec[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EFILEMARK,
	/*    6	ENXIO	*/	FTT_ENXIO,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
	/*    8	ENOEXEC	*/	FTT_ENOEXEC,
	/*    9	EBADF	*/	FTT_ENOENT,
	/*   10	ECHILD	*/	FTT_ENOTSUPPORTED,
	/*   11	EBUSY	*/	FTT_EBUSY,
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
	/*   22	EINVAL	*/	FTT_ENOENT,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EFILEMARK,
	/*   28	ENOSPC	*/	FTT_EFILEMARK,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};
int ftt_trans_skipf[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EBLANK,
	/*    6	ENXIO	*/	FTT_ENXIO,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   22	EINVAL	*/	FTT_ENOENT,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EBLANK,
	/*   28	ENOSPC	*/	FTT_EBLANK,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};
int ftt_trans_skipf_AIX[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EBLANK,
	/*    6	ENXIO	*/	FTT_ENXIO,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   22	EINVAL	*/	FTT_ENOENT,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EBLANK,
	/*   28	ENOSPC	*/	FTT_EBLANK,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_EBLANK,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};
int ftt_trans_skipr[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_ELEADER,
	/*    6	ENXIO	*/	FTT_ENXIO,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   22	EINVAL	*/	FTT_ENOENT,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_ELEADER,
	/*   28	ENOSPC	*/	FTT_ELEADER,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};

int ftt_trans_skipr_AIX[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_ELEADER,
	/*    6	ENXIO	*/	FTT_ENXIO,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   22	EINVAL	*/	FTT_ENOENT,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_ELEADER,
	/*   28	ENOSPC	*/	FTT_ELEADER,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ELEADER,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};

int ftt_trans_skiprew_AIX[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_ENOTAPE,
	/*    6	ENXIO	*/	FTT_ENOTAPE,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   22	EINVAL	*/	FTT_ENOENT,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_ENOTAPE,
	/*   28	ENOSPC	*/	FTT_ENOTAPE,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_ENOTAPE,
	/*   31	EMLINK	*/	FTT_ELEADER,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};

int ftt_trans_out_AIX[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EROFS,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EIO,
	/*    6	ENXIO	*/	FTT_ENOSPC,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   28	ENOSPC	*/	FTT_ENOSPC,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_EIO,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};

int ftt_trans_out[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EROFS,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EIO,
	/*    6	ENXIO	*/	FTT_ENOSPC,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
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
	/*   28	ENOSPC	*/	FTT_ENOSPC,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EROFS,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};

int ftt_trans_chall[MAX_TRANS_ERRNO] = {
	/*    0 NOERROR	*/	FTT_SUCCESS,
	/*    1	EPERM	*/	FTT_EPERM,
	/*    2	ENOENT	*/	FTT_ENOENT,
	/*    3	ESRCH	*/	FTT_ENOENT,
	/*    4	EINTR	*/	FTT_EUNRECOVERED,
	/*    5	EIO	*/	FTT_EIO,
	/*    6	ENXIO	*/	FTT_ENXIO,
	/*    7	E2BIG	*/	FTT_EBLKSIZE,
	/*    8	ENOEXEC	*/	FTT_EPERM,
	/*    9	EBADF	*/	FTT_EPERM,
	/*   10	ECHILD	*/	FTT_ENOTSUPPORTED,
	/*   11	EAGAIN	*/	FTT_ENOTAPE,
	/*   12	ENOMEM	*/	FTT_ENOMEM,
	/*   13	EACCES	*/	FTT_EPERM,
	/*   14	EFAULT	*/	FTT_EFAULT,
	/*   15	ENOTBLK	*/	FTT_ENOTSUPPORTED,
	/*   16	EBUSY	*/	FTT_EBUSY,
	/*   17	EEXIST	*/	FTT_ENOENT,
	/*   18	EXDEV	*/	FTT_ENOENT,
	/*   19	ENODEV	*/	FTT_ENOENT,
	/*   20	ENOTDIR	*/	FTT_ENOENT,
	/*   21	EISDIR	*/	FTT_ENOENT,
	/*   22	EINVAL	*/	FTT_ENOENT,
	/*   23	ENFILE	*/	FTT_ENFILE,
	/*   24	EMFILE	*/	FTT_ENFILE,
	/*   25	ENOTTY	*/	FTT_ENOTTAPE,
	/*   26	ETXTBSY	*/	FTT_ENOENT,
	/*   27	EFBIG	*/	FTT_EBLKSIZE,
	/*   28	ENOSPC	*/	FTT_EIO,
	/*   29	ESPIPE	*/	FTT_ENOTSUPPORTED,
	/*   30	EROFS	*/	FTT_EPERM,
	/*   31	EMLINK	*/	FTT_ENOTSUPPORTED,
	/*   32 EMPIPE  */	FTT_EPIPE,
	/*   33 EDOM  */	FTT_ENOTTAPE,
	/*   34 ERANGE  */	FTT_ENOTSUPPORTED,
	/*   35 ENOMSG  */	FTT_ENOTSUPPORTED,
	/*   36 EIDRM  */	FTT_ENOTSUPPORTED,
	/*   37 ECHRNG  */	FTT_ENOTSUPPORTED,
	/*   38 EL2NSYNC  */	FTT_ENOTSUPPORTED,
	/*   39 EL3HLT  */	FTT_ENOTSUPPORTED,
	/*   40 EL3RST  */	FTT_ENOTSUPPORTED,
	/*   41 ELNRNG  */	FTT_ENOTSUPPORTED,
	/*   42 EUNATCH  */	FTT_ENOTSUPPORTED,
	/*   43 ENOCSI  */	FTT_ENOTSUPPORTED,
	/*   44 EL2HLT  */	FTT_ENOTSUPPORTED,
	/*   45 EDEADLK  */	FTT_EBUSY,
/* AIX specific */
	/*   46 ENOTREADY*/	FTT_ENOTAPE,
	/*   47 EWRPROTECT */	FTT_EROFS,
	/*   48 EFORMAT */	FTT_EBLANK,
	/*   50 default */	FTT_EIO,
};
int *ftt_trans_table_AIX[] = {
    /* none...but just to be safe0 */ ftt_trans_in,
    /* FTT_OPN_READ		 1 */ ftt_trans_in_AIX,
    /* FTT_OPN_WRITE		 2 */ ftt_trans_out_AIX,
    /* FTT_OPN_WRITEFM		 3 */ ftt_trans_out_AIX,
    /* FTT_OPN_SKIPREC		 4 */ ftt_trans_skiprec,
    /* FTT_OPN_SKIPFM		 5 */ ftt_trans_skipf_AIX,
    /* FTT_OPN_REWIND		 6 */ ftt_trans_skiprew_AIX,
    /* FTT_OPN_UNLOAD		 7 */ ftt_trans_skipf,
    /* FTT_OPN_RETENSION	 8 */ ftt_trans_skipf,
    /* FTT_OPN_ERASE		 9 */ ftt_trans_out,
    /* FTT_OPN_STATUS		10 */ ftt_trans_in,
    /* FTT_OPN_GET_STATUS	11 */ ftt_trans_in,
    /* FTT_OPN_ASYNC 		12 */ ftt_trans_in,
    /* FTT_OPN_PASSTHRU     13 */ ftt_trans_out,
    /* FTT_OPN_CHALL        14 */ ftt_trans_chall,
    /* FTT_OPN_OPEN         15 */ ftt_trans_open,
    /* FTT_OPN_RSKIPREC		16 */ ftt_trans_skiprec,
    /* FTT_OPN_RSKIPFM		17 */ ftt_trans_skipr_AIX,
};
int *ftt_trans_table_Linux[] = {
    /* none...but just to be safe0 */ ftt_trans_in,
    /* FTT_OPN_READ		     1 */ ftt_trans_in_Linux,
    /* FTT_OPN_WRITE		 2 */ ftt_trans_out,
    /* FTT_OPN_WRITEFM		 3 */ ftt_trans_out,
    /* FTT_OPN_SKIPREC		 4 */ ftt_trans_skiprec,
    /* FTT_OPN_SKIPFM		 5 */ ftt_trans_skipf,
    /* FTT_OPN_REWIND		 6 */ ftt_trans_skipr,
    /* FTT_OPN_UNLOAD		 7 */ ftt_trans_skipf,
    /* FTT_OPN_RETENSION	 8 */ ftt_trans_skipf,
    /* FTT_OPN_ERASE		 9 */ ftt_trans_out,
    /* FTT_OPN_STATUS		10 */ ftt_trans_in,
    /* FTT_OPN_GET_STATUS	11 */ ftt_trans_in,
    /* FTT_OPN_ASYNC 		12 */ ftt_trans_in,
    /* FTT_OPN_PASSTHRU     13 */ ftt_trans_out,
    /* FTT_OPN_CHALL        14 */ ftt_trans_chall,
    /* FTT_OPN_OPEN         15 */ ftt_trans_open,
    /* FTT_OPN_RSKIPREC		16 */ ftt_trans_skiprec,
    /* FTT_OPN_RSKIPFM		17 */ ftt_trans_skipr,
};
int *ftt_trans_table[] = {
    /* none...but just to be safe0 */ ftt_trans_in,
    /* FTT_OPN_READ		     1 */ ftt_trans_in,
    /* FTT_OPN_WRITE		 2 */ ftt_trans_out,
    /* FTT_OPN_WRITEFM		 3 */ ftt_trans_out,
    /* FTT_OPN_SKIPREC		 4 */ ftt_trans_skiprec,
    /* FTT_OPN_SKIPFM		 5 */ ftt_trans_skipf,
    /* FTT_OPN_REWIND		 6 */ ftt_trans_skipr,
    /* FTT_OPN_UNLOAD		 7 */ ftt_trans_skipf,
    /* FTT_OPN_RETENSION	 8 */ ftt_trans_skipf,
    /* FTT_OPN_ERASE		 9 */ ftt_trans_out,
    /* FTT_OPN_STATUS		10 */ ftt_trans_in,
    /* FTT_OPN_GET_STATUS	11 */ ftt_trans_in,
    /* FTT_OPN_ASYNC 		12 */ ftt_trans_in,
    /* FTT_OPN_PASSTHRU     13 */ ftt_trans_out,
    /* FTT_OPN_CHALL        14 */ ftt_trans_chall,
    /* FTT_OPN_OPEN         15 */ ftt_trans_open,
    /* FTT_OPN_RSKIPREC		16 */ ftt_trans_skiprec,
    /* FTT_OPN_RSKIPFM		17 */ ftt_trans_skipr,
};

char *Generic_density_trans[MAX_TRANS_DENSITY] = {
	"unknown",
	"low",
	"med",
	"hi",
	0
};

char *Exabyte_density_trans[MAX_TRANS_DENSITY] = {
	"unknown",
	"8200",
	"8500",
	"8900",
	"Mammoth2",
	0
};

char *DLT_density_trans[MAX_TRANS_DENSITY] = {
	"unknown",
	"6667bpi",
	"10000bpi",
	"2.6G 42500bpi 24trk",
	"6G 42500bpi 56trk",
	"10G 62500bpi 64trk",
	"20G 62500bpi 128trk",
 	"35G 62500bpi 224trk",
	"SDLT220 133000bpi 56trk",
        "SDLT320 190000bpi 56trk",
	0
};

char *AIT_density_trans[MAX_TRANS_DENSITY] = {
	"unknown",
	"AIT-1",
	"AIT-2",
	0
};
char *Ultrium_density_trans[MAX_TRANS_DENSITY] = {
        "unknown",
        "LTO-1",
        0
};
char *STK_density_trans[MAX_TRANS_DENSITY] = {
        "unknown",
        "STK-1",
        "STK-2",
        "STK-3",
        0
};

/*
** commands to popen to read device id-s
** and echo the device type as the result
** note the %d is filled by a printf first
*/
static char LINUXfind[] =
    "IFS=\" \t\n\";\
     PATH=\"/bin:/usr/bin:/sbin:/usr/sbin\";\
     count=0; export count ;\
     cat /proc/scsi/scsi | while read a b c d e f; do \
        if [ \"$c\" = \"Model:\" ]; then mod=$d; fi ; \
        if [ \"$b\" = Sequential-Access ] ; then \
		if [ \"$count\" = \"%d\" ] ; then \
		    echo $mod; exit; \
		fi; \
		count=`expr $count + 1`; \
	fi; \
       done";

static char LINUXrmtfind[] =
    "IFS=\" \t\n\";\
     PATH=\"/bin:/usr/bin:/sbin:/usr/sbin\";\
     cat /proc/scsi/scsi | while read a b c d e f g h; do \
        if [ \"$b\" = \"scsi%d\" -a \"$f\" = \"%02d\" ] ; then \
	    read a b c d e f g h ;\
	    echo $d; exit; \
	fi; \
       done";

static char AIXfind[] =
    "IFS=\" \t\n\";\
     PATH=\"/bin:/usr/bin:/sbin:/usr/sbin\";\
     /usr/sbin/lscfg -v -l rmt%d | \
       grep 'Machine Type' |       \
       sed -e 's/.*\\.//'";

static char IRIXfind[] =
    "IFS=\" \t\n\";\
     PATH=\"/bin:/usr/bin:/sbin:/usr/sbin\";\
     /sbin/hinv | \
       grep 'Tape drive: unit %2$d on SCSI controller %1$d:' | \
       sed -e 's/.*: *//' -e 's/8mm *(\\(.*\\)) *cartridge */\\1/' -e 's/^8.0.$/Exb-&/'";

static char IRIXfindVME[] =
    "IFS=\" \t\n\";\
     PATH=\"/bin:/usr/bin:/sbin:/usr/sbin\";\
     /sbin/hinv | \
       grep 'Tape drive: unit %2$d on VME-SCSI controller %1$d:' | \
       sed -e 's/.*:  *//' -e 's/8mm(\\(.*\\).) cartridge */EXB-\\1/'";

static char OSF1find[] = 
   "IFS=\" \t\n\"\n\
    PATH=\"/usr/sbin:/sbin:/etc:/bin:/usr/bin\"\n\
    export PATH \n\
    minor_num=`ls -l /dev/rmt%dh | sed -e 's/.*, *\\([0-9]*\\).*/\\1/'` \n\
    tmp=`expr $minor_num /  1024` \n\
    bus=`expr $tmp / 16` \n\
    target=`expr $tmp %% 16` \n\
    scu show edt bus $bus target $target lun 0 full | \n\
	    grep 'Product Id' | \n\
	    sed -e 's/.*: //' \n";

static char SunOSfind_dev[] =
   "#d\n\
    PATH=\"/bin:/usr/bin:/sbin:/usr/sbin\";\
    drive=%d\n\
    adr=\"`ls -l /dev/rmt/${drive} | awk '{print substr($NF,index($NF,\\\"st@\\\"),length($NF)-index($NF,\\\"st@\\\"))}'`\" \n\
    ctr=\"`ls -l /dev/rmt/${drive} | awk '{if(index($NF,\\\"/scsi@\\\")==0) {print substr($NF,index($NF,\\\"device\\\")+7,index($NF,\\\"/st@\\\")-index($NF,\\\"device\\\")-7)} else{print substr($NF,index($NF,\\\"device\\\")+7,index($NF,\\\"/scsi@\\\")-index($NF,\\\"device\\\")-6)}}'`\" \n\
    id=\"`dmesg | grep $ctr | grep $adr | awk '{print $1; exit}'`\" \n\
    line=\"`dmesg | grep \\^${id}: `\" \n\
    case \"$line\" in \n\
    *Vendor*)  \n\
        echo \"$line\" | sed -e 's/.*Product..\\([^ ]*\\).*>/\\1/' | tail -1 \n\
        ;; \n\
    *) \n\
        echo \"$line\" | sed -e 's/[^<]*<[^ ]* \\([^ ]*\\).*>/\\1/' | tail -1 \n\
        ;; \n\
    esac \n";

static char Win32find_dev[] = "";

/*
** device id's
*/

/* The following tables are based on:
**
**
**typedef struct {
**    char *os;                    OS+Version (i.e. IRIX+5.3) string 
**    char *drivid;                SCSI Drive-id prefix 
**    char *controller;            controller name string 
**    long flags;                  FTT_FLAG_XXX bits for behavior 
**    long scsi_ops;               FTT_OP_XXX bits for ops to use SCSI 
**    int **errortrans;            errortrans[FTT_OPN_XXX][errno]->ftt_errno
**
**    char **densitytrans;         density names 
**    char *baseconv_in;           basename parser scanf string 
**    char *baseconv_out;          basename parser printf string 
**    int nconv;                   number of items scanf should return 
**    char *drividcmd;             printf this to get shell command->driveid
**    ftt_devinfo devs[MAXDEVSLOTS];  drive specs with printf strings 
**} ftt_dev_entry;
**
** Note -- ftt_findslot searches this table with a prefix string match,
**         and takes the first match.  Therefore if you put a null string
**         or one type that is a prefix of the other (for either OS name
**         or drive-id) that will prevent longer strings from matching
**	   further down the table, for example, if you have
**         {"OSNAME", "", ...},
**         {"OSNAME", "EXB-8200", ...},...
**	   the first one will always match before the second one, and
**         you might as well not have it.  So put your null-string
**         default cases *last*.
*/
#define EXB_MAX_BLKSIZE  245760
#define IRIX_MAX_BLKSIZE 131072
#define SUN_MAX_BLKSIZE   EXB_MAX_BLKSIZE
#define WIN_MAX_BLKSIZE  120000
#ifdef NICE_WORLD
#define LINUX_MAX_BLKSIZE 65536
#else
#define LINUX_MAX_BLKSIZE 32768
#endif


ftt_dev_entry devtable[] = {
    {"Linux", "SDX-3", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, AIT_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",  0,  0, 0x00,  0,  0,      0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",    -1,  0, -1,    1,  0,  0,      1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  0,  1, 0x30,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  0, 0x30,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   0,  1, 0x30,  0,  0,FTT_RWOC, 1, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   0,  0, 0x30,  0,  0,FTT_RWOC, 0, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "SDX-5", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, AIT_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",  1,  0, 0x00,  0,  0,      0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",    -1,  0, -1,    1,  0,  0,      1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  0,  0, 0x30,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   1,  0, 0x31,  0,  0,FTT_RWOC, 1, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   0,  0, 0x30,  0,  0,FTT_RWOC, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  1,  1, 0x31,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  1, 0x30,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   1,  1, 0x31,  0,  0,FTT_RWOC, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   0,  1, 0x30,  0,  0,FTT_RWOC, 0, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "DLT7", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table_Linux, DLT_density_trans, 
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",    6,  0, 0x84, 0,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",     -1,  0, -1,  1,  0,       0,     1, LINUX_MAX_BLKSIZE},
    /* Other Densities */
       { "rmt/tps%dd%dn",    6,  1, 0x85, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    4,  0, 0x80, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    4,  1, 0x81, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    5,  0, 0x1A, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    5,  0, 0x82, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    5,  1, 0x83, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    4,  0, 0x19, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    3,  0, 0x18, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    2,  0, 0x17, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    1,  0, 0x16, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    0,  0, 0x0A, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "DLT4", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table_Linux, DLT_density_trans, 
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",    5,  0, 0x1A, 0,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",     -1,  0, -1,  1,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Other Densities */
       { "rmt/tps%dd%dn",    4,  0, 0x80, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    4,  1, 0x81, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    5,  0, 0x82, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    5,  1, 0x83, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    4,  0, 0x19, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    3,  0, 0x18, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    2,  0, 0x17, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    1,  0, 0x16, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    0,  0, 0x0A, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "DLT2", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table_Linux, DLT_density_trans, 
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",    4,  0, 0x80, 0,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",     -1,  0, -1,  1,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Other Densities */
       { "rmt/tps%dd%dn",    4,  1, 0x81, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    5,  0, 0x82, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    5,  1, 0x83, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    0,  0, 0x0A, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    4,  0, 0x19, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    3,  0, 0x18, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    2,  0, 0x17, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    1,  0, 0x16, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "SDLT320", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table_Linux, DLT_density_trans, 
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",    9,  0, 0x49, 0,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",     -1,  0, -1,  1,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Other Densities */
       { "rmt/tps%dd%dn",    9,  1, 0x49, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    8,  0, 0x48, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",    8,  1, 0x48, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "EXB-82", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Exabyte_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",     0,  0,  0,  0,  0,      0,    1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",     -1,  0, -1,  1,  0,      0,    1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "EXB-8505", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Exabyte_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn", 	  1,  0, 0x15,  0,  0,      0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",     -1,  0, -1,    1,  0,  0,      1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn", 	  1,  1, 0x8c,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn", 	  1,  0, 0x15,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn", 	  0,  1, 0x90,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn", 	  0,  0, 0x14,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",      1,  0, 0x15,  0,  0,FTT_RWOC, 1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "EXB-85", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Exabyte_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",  1,  0, 0x15,  0,  0,      0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",    -1,  0, -1,    1,  0,  0,      1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  1,  0, 0x15,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  0, 0x14,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   1,  0, 0x15,  0,  0,FTT_RWOC, 1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "Mammoth2", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Exabyte_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",  3,  1, 0x28,  0,  0,      0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",     -1,  0, -1,    1,  0,  0,     1, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  2,  0, 0x27,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  3,  0, 0x28,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  1,  1, 0x8c,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  1,  0, 0x15,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  1,  0, 0x15,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  0, 0x14,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  1, 0x90,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   1,  0, 0x15,  0,  0,FTT_RWOC, 1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "EXB-89", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Exabyte_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",  2,  1, 0x27,  0,  0,      0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",     -1,  0, -1,   1,  0,      0,  1, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  2,  0, 0x27,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  1,  1, 0x8c,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  1,  0, 0x15,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  1,  0, 0x15,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  0, 0x14,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  1, 0x90,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   1,  0, 0x15,  0,  0,FTT_RWOC, 1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "ULT3580-TD1", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,Ultrium_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "rmt/tps%dd%dn",  0,  0, 0x40,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",    -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  0,  0, 0x40,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  1, 0x40,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "ULT06242-XXX", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,Ultrium_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "rmt/tps%dd%dn",  0,  0, 0x40,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",    -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  1, 0x40,  0,  0,       0,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%d",   0,  1, 0x40,  0,  0,       0,  0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  0,  0, 0x40,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  0,  1, 0x40,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "T9940A", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,STK_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "rmt/tps%dd%dn",  1,  0, 0x43,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",    -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  1,  0, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  1,  1, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "T9940B", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,STK_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "rmt/tps%dd%dn",  3,  0, 0x44,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",    -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  3,  0, 0x44,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  3,  1, 0x44,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  1,  0, 0x43,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  1,  0, 0x43,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "9840", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,STK_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "rmt/tps%dd%dn",  2,  0, 0x42,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",    -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%dn",  2,  0, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "rmt/tps%dd%dn",  2,  1, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "DLT7", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, DLT_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "dev/nst%d",    6,  0, 0x84, 0,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",     -1,  0, -1,  1,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Other Densities */
       { "dev/nst%d",    6,  1, 0x85, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    4,  0, 0x80, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    4,  1, 0x81, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    5,  0, 0x1A, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    5,  0, 0x82, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    5,  1, 0x83, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    4,  0, 0x19, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    3,  0, 0x18, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    2,  0, 0x17, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    1,  0, 0x16, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    0,  0, 0x0A, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/st%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "DLT4", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, DLT_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "dev/nst%d",    5,  0, 0x1A, 0,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",     -1,  0, -1,  1,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Other Densities */
       { "dev/nst%d",    4,  0, 0x80, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    4,  1, 0x81, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    5,  0, 0x82, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    5,  1, 0x83, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    4,  0, 0x19, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    3,  0, 0x18, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    2,  0, 0x17, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    1,  0, 0x16, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    0,  0, 0x0A, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/st%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "DLT2", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, DLT_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "dev/nst%d",    4,  0, 0x80, 0,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",     -1,  0, -1,  1,  0,       0,   1, LINUX_MAX_BLKSIZE},
    /* Other Densities */
       { "dev/nst%d",    4,  1, 0x81, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    5,  0, 0x82, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    5,  1, 0x83, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    0,  0, 0x0A, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    4,  0, 0x19, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    3,  0, 0x18, 0,  0,       0,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    2,  0, 0x17, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",    1,  0, 0x16, 0,  0,FTT_RDNW,   0, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/st%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "EXB-82", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Exabyte_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "dev/nst%d",     0,  0,  0,  0,  0,      0,    1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",     -1,  0, -1,  1,  0,      0,    1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/nst%d", 	  0,  1, 0x90,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d", 	  1,  0, 0x15,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d", 	  1,  1, 0x8c,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d", 	  0,  0, 0x14,  0, 0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/st%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "EXB-8505", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Exabyte_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "dev/nst%d", 	  1,  0, 0x15,  0,  0,      0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",     -1,  0, -1,    1,  0,  0,      1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/nst%d", 	  0,  1, 0x90,  0,  0,FTT_RDNW, 0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d", 	  1,  0, 0x15,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d", 	  1,  1, 0x8c,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d", 	  0,  0, 0x14,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/st%d",      1,  0, 0x15,  0,  0,FTT_RWOC, 1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "EXB-85", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Exabyte_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "dev/nst%d", 	  1,  0, 0x15,  0,  0,      0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",     -1,  0, -1,    1,  0,  0,      1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/nst%d", 	  1,  0, 0x15,  0,  0,      0,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d", 	  0,  0, 0x14,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/st%d",      1,  0, 0x15,  0,  0,FTT_RWOC, 1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "ULT3580-TD1", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,Ultrium_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "dev/nst%d",      0,  0, 0x00,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",      -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/nst%d",      0,  0, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE}, 
       { "dev/nst%d",      0,  1, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/st%d",       1,  0, 0x00,  0,  0,FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},

    {"Linux", "ULT06242-XXX", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,Ultrium_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "dev/nst%d",      0,  0, 0x00,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",      -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/nst%d",      0,  0, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",      0,  1, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/st%d",       1,  0, 0x00,  0,  0,FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "T9940A", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,STK_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "dev/nst%d",      1,  0, 0x43,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",      -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/nst%d",      1,  0, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",      1,  1, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/st%d",       1,  0, 0x00,  0,  0,FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "T9940B", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,STK_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "dev/nst%d",      3,  0, 0x44,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",      -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/nst%d",      3,  0, 0x44,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",      3,  1, 0x44,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/st%d",       3,  0, 0x44,  0,  0,FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",      1,  0, 0x43,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",      1,  1, 0x43,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/st%d",       1,  0, 0x43,  0,  0,FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "9840", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_Linux,STK_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod   hwd pas fxd   rewind 1st */
    /*   ======          === ===   === === ===   ====== === */
    /* Default */
       { "dev/nst%d",      2,  0, 0x42,  0,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",      -1,  0,   -1,  1,  0,       0,  1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/nst%d",      2,  0, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/nst%d",      2,  1, 0x00,  0,  0,FTT_RDNW,  0, LINUX_MAX_BLKSIZE},
       { "dev/st%d",       2,  0, 0x00,  0,  0,FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Generic_density_trans,
       "rmt/tps%dd%d", "rmt/tps%dd%dn", 2, LINUXrmtfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "rmt/tps%dd%dn",     0,  0,  0,  0,  0,  0,        1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "sc/sc%dd%d",     -1,  0, -1,  1,  0,  0,        1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/tps%dd%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},
    {"Linux", "", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER , FTT_OP_GET_STATUS, ftt_trans_table_Linux, Generic_density_trans,
       "dev/%*[ns]t%d", "dev/nst%d", 1, LINUXfind, {
    /*   string          den mod hwd  pas fxd rewind   1st */
    /*   ======          === === ===  === === ======   === */
    /* Default */
       { "dev/nst%d",     0,  0,  0,  0,  0,  0,        1, LINUX_MAX_BLKSIZE},
    /* Default, passthru  */
       { "dev/sg%d",     -1,  0, -1,  1,  0,  0,        1, LINUX_MAX_BLKSIZE},
    /* Descriptive */
       { "dev/st%d",      0,  0,  0,  0,  0, FTT_RWOC,  1, LINUX_MAX_BLKSIZE},
       { 0 },
    }},

    {"SunOS+5", "Mammoth2", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, passthru  */
       { "rmt/%dcbn", 	  3,  1, 0x28,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhbn", 	  3,  0, 0x28,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhbn", 	  2,  1, 0x27,  1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Usable */
       { "rmt/%dlbn", 	  2,  0, 0x27,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
    /* Densitites */
       { "rmt/%dub", 	  0,  1, 0x90,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb", 	  1,  0, 0x15,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb", 	  1,  1, 0x8c,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb", 	  0,  0, 0x14,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db", 	  1,  0, 0x15,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dubn", 	  0,  1, 0x90,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn", 	  1,  0, 0x15,  1,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dbn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb", 	  1,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "EXB-8900", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, passthru  */
       { "rmt/%dhbn", 	  2,  0, 0x27,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  2,  1, 0x27,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhbn", 	  2,  1, 0x27,  1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Usable */
       { "rmt/%dhbn", 	  2,  0, 0x27,  0,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Densitites */
       { "rmt/%dub", 	  0,  1, 0x90,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb", 	  1,  0, 0x15,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb", 	  1,  1, 0x8c,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb", 	  0,  0, 0x14,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db", 	  1,  0, 0x15,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dubn", 	  0,  1, 0x90,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn", 	  1,  0, 0x15,  1,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dbn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb", 	  1,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "EXB-8200", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, pass-thru */
       { "rmt/%dubn", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dubn", 	  0,  0,  0,    1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dhbn", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlbn", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dub", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb", 	  0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb", 	  0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb", 	  0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb", 	  1,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "ULT3580-TD1", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,Ultrium_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, pass-thru */
       { "rmt/%dubn",     0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dubn",     0,  0,  0,    1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dhbn",     0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn",     0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlbn",     0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dub",      0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb",      0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb",      0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb",      0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun",      0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn",      0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn",      0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln",      0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du",       0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh",       0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm",       0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl",       0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d",        1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb",      1,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn",     1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn",      1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn",       1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "ULT06242-XXX", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,Ultrium_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, pass-thru */
       { "rmt/%dubn",     0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dubn",     0,  0,  0,    1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dhbn",     0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn",     0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlbn",     0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dub",      0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb",      0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb",      0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb",      0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun",      0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn",      0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn",      0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln",      0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du",       0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh",       0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm",       0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl",       0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d",        1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb",      1,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn",     1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn",      1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn",       1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "T9940A", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, STK_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, passthru  */
       { "rmt/%dhbn",     1,  0, 0x43,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn",     1,  1, 0x43,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhbn",     1,  1, 0x43,  1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Usable */
       { "rmt/%dhbn",     1,  0, 0x43,  0,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Densitites */
       { "rmt/%dub",      1,  1, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb",      1,  0, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb",      1,  1, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb",      1,  0, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db",       1,  0, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dubn",     1,  1, 0x43,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn",     1,  0, 0x43,  1,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dbn",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du",       1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d",        1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb",      1,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn",     1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn",      1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn",       1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "T9940B", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, STK_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, passthru  */
       { "rmt/%dhbn",     3,  0, 0x44,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn",     3,  1, 0x44,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhbn",     3,  1, 0x44,  1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Usable */
       { "rmt/%dhbn",     3,  0, 0x44,  0,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Densitites */
       { "rmt/%dub",      3,  1, 0x44,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb",      3,  0, 0x44,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb",      3,  1, 0x44,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb",      3,  0, 0x44,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db",       3,  0, 0x44,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dub",      1,  1, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb",      1,  0, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb",      1,  1, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb",      1,  0, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db",       1,  0, 0x43,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dubn",     3,  1, 0x44,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn",     3,  0, 0x44,  1,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dubn",     1,  1, 0x43,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn",     1,  0, 0x43,  1,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dbn",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln",      1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du",       1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d",        1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc",       1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb",      1,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn",     1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn",      1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn",       1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "9840", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, STK_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, passthru  */
       { "rmt/%dhbn",     2,  0, 0x42,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn",     2,  1, 0x42,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhbn",     2,  1, 0x42,  1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Usable */
       { "rmt/%dhbn",     2,  0, 0x42,  0,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Densitites */
       { "rmt/%dub",      2,  1, 0x42,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb",      2,  0, 0x42,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb",      2,  1, 0x42,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb",      2,  0, 0x42,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db",       2,  0, 0x42,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dubn",     2,  1, 0x42,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn",     2,  0, 0x42,  1,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dbn",      2,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun",      2,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn",      2,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn",      2,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln",      2,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du",       2,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh",       2,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm",       2,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl",       2,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d",        2,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc",       2,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb",      2,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn",     2,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn",      2,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn",       1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "EXB-85", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, passthru  */
       { "rmt/%dmbn", 	  1,  0, 0x15,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
    /* Usable Varirable */
       { "rmt/%dlbn", 	  0,  0, 0x14,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
    /* Densitites */
       { "rmt/%dub", 	  0,  1, 0x90,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb", 	  1,  0, 0x15,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb", 	  1,  1, 0x8c,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb", 	  0,  0, 0x14,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db", 	  1,  0, 0x15,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dubn", 	  0,  1, 0x90,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn", 	  1,  0, 0x15,  1,  0, FTT_RDNW|       0, 0, SUN_MAX_BLKSIZE},
       { "rmt/%dhbn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dbn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb", 	  1,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "DLT", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, DLT_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, Passthru  */
       { "rmt/%dubn", 	  5,  0, 0x1A,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dubn", 	  5,  1, 0x1A,  0,  0,                 0, 0, EXB_MAX_BLKSIZE},
       { "rmt/%dubn", 	  5,  0, 0x00,  1,  0,                 0, 0, EXB_MAX_BLKSIZE},
       { "rmt/%dhbn", 	  4,  0, 0x19,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dhbn", 	  4,  1, 0x19,  0,  0,                 0, 0, EXB_MAX_BLKSIZE},
       { "rmt/%dmbn", 	  3,  0, 0x18,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dlbn", 	  2,  0, 0x17,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Densitites */
       { "rmt/%d", 	  0,  0, 0x0A,  0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  1,  0, 0x16,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  2,  0, 0x17,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  3,  0, 0x18,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  4,  0, 0x80,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  4,  1, 0x81,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  4,  0, 0x19,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  4,  1, 0x19,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  5,  0, 0x82,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  5,  1, 0x83,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  5,  1, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dcbn", 	  5,  0, 0x1A,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dbn", 	  5,  0, 0x1A,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dub", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dcb", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dhb", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dmb", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dlb", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "rmt/%db", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "rmt/%dun", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb", 	  1,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn", 	  1,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "EXB-8200", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
       "%[^/]/st@%d,0:", "%s/st@%d,0:", 2, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, pass-thru */
       { "%s/st@%d,0:ubn", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ubn", 	  0,  0,  0,    1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "%s/st@%d,0:hbn", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mbn", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ub", 	  0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hb", 	  0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mb", 	  0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lb", 	  0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:un", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hn", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mn", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ln", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:u", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:h", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:m", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:l", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:c", 	  0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cb", 	  0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cbn", 	  0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cn", 	  0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:n", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "ULT3580-TD1", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,Ultrium_density_trans,
       "%[^/]/st@%d,0:", "%s/st@%d,0:", 2, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, pass-thru */
       { "%s/st@%d,0:ubn",        0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ubn",        0,  1,  0,    1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "%s/st@%d,0:hbn",        0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mbn",        0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ub",         0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hb",         0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mb",         0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lb",         0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:un",         0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hn",         0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mn",         0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ln",         0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:u",          0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:h",          0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:m",          0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:l",          0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:",           0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:c",          0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cb",         0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cbn",        0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cn",         0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:n",          0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "ULT06242-XXX", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,Ultrium_density_trans,
       "%[^/]/st@%d,0:", "%s/st@%d,0:", 2, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, pass-thru */
       { "%s/st@%d,0:ubn",        0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ubn",        0,  1,  0,    1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "%s/st@%d,0:hbn",        0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mbn",        0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ub",         0,  0,  0,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hb",         0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mb",         0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lb",         0,  0,  0,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:un",         0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hn",         0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mn",         0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ln",         0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:u",          0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:h",          0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:m",          0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:l",          0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:",           0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:c",          0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cb",         0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cbn",        0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cn",         0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:n",          0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "T9940A", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,STK_density_trans,
       "%[^/]/st@%d,0:", "%s/st@%d,0:", 2, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, pass-thru */
       { "%s/st@%d,0:ubn",        1,  0, 43,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ubn",        1,  1, 43,    1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "%s/st@%d,0:hbn",        1,  0, 43,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mbn",        1,  0, 43,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        1,  0, 43,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ub",         1,  0, 43,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hb",         1,  0, 43,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mb",         1,  0, 43,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lb",         1,  0, 43,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:un",         1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hn",         1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mn",         1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ln",         1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:u",          1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:h",          1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:m",          1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:l",          1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:",           1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:c",          1,  1, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cb",         1,  1, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cbn",        1,  1, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cn",         1,  1, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:n",          1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "T9940B", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,STK_density_trans,
       "%[^/]/st@%d,0:", "%s/st@%d,0:", 2, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, pass-thru */
       { "%s/st@%d,0:ubn",        3,  0, 44,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ubn",        3,  1, 44,    1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "%s/st@%d,0:hbn",        3,  0, 44,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mbn",        3,  0, 44,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        3,  0, 44,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ub",         3,  0, 44,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hb",         3,  0, 44,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mb",         3,  0, 44,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lb",         3,  0, 44,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:un",         3,  0, 44,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hn",         3,  0, 44,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mn",         3,  0, 44,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ln",         3,  0, 44,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:u",          3,  0, 44,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:h",          3,  0, 44,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:m",          3,  0, 44,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:l",          3,  0, 44,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:",           3,  0, 44,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:c",          3,  1, 44,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cb",         3,  1, 44,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cbn",        3,  1, 44,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cn",         3,  1, 44,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        3,  0, 44,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:n",          3,  0, 44,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hbn",        1,  0, 43,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mbn",        1,  0, 43,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        1,  0, 43,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ub",         1,  0, 43,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hb",         1,  0, 43,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mb",         1,  0, 43,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lb",         1,  0, 43,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:un",         1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hn",         1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mn",         1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ln",         1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:u",          1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:h",          1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:m",          1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:l",          1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:",           1,  0, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:c",          1,  1, 43,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cbn",        1,  1, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cn",         1,  1, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:n",          1,  0, 43,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "9840", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,STK_density_trans,
       "%[^/]/st@%d,0:", "%s/st@%d,0:", 2, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, pass-thru */
       { "%s/st@%d,0:ubn",        2,  0, 42,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ubn",        2,  1, 42,    1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "%s/st@%d,0:hbn",        2,  0, 42,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mbn",        2,  0, 42,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        2,  0, 42,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ub",         2,  0, 42,    0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hb",         2,  0, 42,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mb",         2,  0, 42,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lb",         2,  0, 42,    0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:un",         2,  0, 42,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hn",         2,  0, 42,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mn",         2,  0, 42,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ln",         2,  0, 42,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:u",          2,  0, 42,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:h",          2,  0, 42,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:m",          2,  0, 42,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:l",          2,  0, 42,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:",           2,  0, 42,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:c",          2,  1, 42,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cb",         2,  1, 42,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cbn",        2,  1, 42,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cn",         2,  1, 42,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn",        2,  0, 42,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:n",          2,  0, 42,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "EXB-85", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
       "%[^/]/st@%d,0:", "%s/st@%d,0:", 2, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, passthru  */
       { "%s/st@%d,0:mbn", 	  1,  0, 0x15,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
    /* Usable Varirable */
       { "%s/st@%d,0:lbn", 	  0,  0, 0x14,  0,  0,FTT_RDNW|        0, 1, SUN_MAX_BLKSIZE},
    /* Densitites */
       { "%s/st@%d,0:ub", 	  0,  1, 0x90,  0,  0,FTT_RDNW| FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hb", 	  1,  0, 0x15,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mb", 	  1,  1, 0x8c,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lb", 	  0,  0, 0x14,  0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:b", 	  1,  0, 0x15,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "%s/st@%d,0:ubn", 	  0,  1, 0x90,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mbn", 	  1,  0, 0x15,  1,  0, FTT_RDNW|       0, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hbn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:bn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:un", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ln", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:u", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:h", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:m", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:l", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:c", 	  0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cb", 	  0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cbn", 	  0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cn", 	  0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:n", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "DLT", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, DLT_density_trans,
       "%[^/]/st@%d,0:", "%s/st@%d", 2, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
       { "%s/st@%d,0:ubn", 	  5,  0, 0x1A,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:ubn", 	  5,  1, 0x1A,  0,  0,                 0, 0, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:ubn", 	  5,  0, 0x00,  1,  0,                 0, 0, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:hbn", 	  4,  0, 0x19,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:hbn", 	  4,  1, 0x19,  0,  0,                 0, 0, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:mbn", 	  3,  0, 0x18,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn", 	  2,  0, 0x17,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Densitites */
       { "%s/st@%d,0:", 	  0,  0, 0x0A,  0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  1,  0, 0x16,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  2,  0, 0x17,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  3,  0, 0x18,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  4,  0, 0x80,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  4,  1, 0x81,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  4,  0, 0x19,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  4,  1, 0x19,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  5,  0, 0x82,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  5,  1, 0x83,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  5,  1, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 0, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "%s/st@%d,0:cbn", 	  5,  0, 0x1A,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:bn", 	  5,  0, 0x1A,  0,  0,                 0, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:ub", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:cb", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:hb", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:mb", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:lb", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:b", 	  5,  0, 0x1A,  0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
       { "%s/st@%d,0:un", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:hn", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:mn", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:ln", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:u", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:h", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:m", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:l", 	  5,  0, 0x1A,  0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:c", 	  0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cb", 	  0,  1,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cbn", 	  0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:cn", 	  0,  1,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:lbn", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "%s/st@%d,0:n", 	  0,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},

    {"SunOS+5", "SDX-3", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, AIT_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, passthru  */
       { "rmt/%dhbn", 	  0,  0, 0x00,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhbn", 	  0,  0, 0x00,  1,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Usable */
       { "rmt/%dcbn", 	  0,  1, 0x00,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
    /* Densitites */
       { "rmt/%dhbn", 	  0,  0, 0x30,  0,  0,                 0, 0, SUN_MAX_BLKSIZE},
       { "rmt/%dub", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dubn", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dbn", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "SDX-5", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, AIT_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string          den mod hwd   pas fxd rewind            1st */
    /*   ======          === === ===   === === ======            === */
    /* Default, passthru  */
       { "rmt/%dcbn", 	  1,  1, 0x31,  1,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  1,  1, 0x00,  0,  0,                 0, 0, SUN_MAX_BLKSIZE},
    /* Usable */
       { "rmt/%dlbn", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  1,  1, 0x31,  0,  0,                 0, 0, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn", 	  0,  1, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dub", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dubn", 	  1,  0, 0x31,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn", 	  1,  0, 0x31,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dbn", 	  1,  0, 0x31,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun", 	  1,  0, 0x31,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn", 	  1,  0, 0x31,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn", 	  1,  0, 0x31,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln", 	  0,  0, 0x30,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl", 	  0,  0, 0x30,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%d", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dc", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcb", 	  1,  0, 0x31,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcbn", 	  1,  0, 0x31,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dcn", 	  1,  0, 0x31,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dn", 	  1,  0, 0x31,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"SunOS+5", "", "SCSI", FTT_FLAG_SUID_SCSI|FTT_FLAG_VERIFY_EOFS, FTT_OP_GET_STATUS, ftt_trans_table, Generic_density_trans,
       "rmt/%d", "rmt/%d", 1, SunOSfind_dev, {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, passthru  */
       { "rmt/%dmbn", 	  1,  0, 0x00,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
    /* Usable Varirable */
       { "rmt/%dlbn", 	  0,  0, 0x00,  0,  0,                 0, 1, SUN_MAX_BLKSIZE},
    /* Densitites */
       { "rmt/%dub", 	  3,  0, 0x00,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhb", 	  2,  0, 0x00,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmb", 	  1,  0, 0x00,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dlb", 	  0,  0, 0x00,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%db", 	  1,  0, 0x00,  0,  0,          FTT_RWOC, 1, SUN_MAX_BLKSIZE},
    /* Descriptive */
       { "rmt/%dubn", 	  3,  0, 0x00,  0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmbn", 	  1,  0, 0x00,  1,  0, FTT_RDNW|       0, 0, SUN_MAX_BLKSIZE},
       { "rmt/%dhbn", 	  2,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dbn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dun", 	  3,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dhn", 	  2,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dmn", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dln", 	  0,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%du", 	  1,  0,  0,    0,  0, FTT_RDNW|       0, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dh", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dm", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { "rmt/%dl", 	  1,  0,  0,    0,  0, FTT_RDNW|FTT_RWOC, 1, SUN_MAX_BLKSIZE},
       { 0 },
    }},
    {"OSF1", "EXB-8200", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
	"dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dl",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
	{ 0 },
     }},
    {"OSF1", "ULT3580-TD1", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,Ultrium_density_trans,
        "dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dl",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { 0 },
     }},
    {"OSF1", "ULT06242-XXX", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,Ultrium_density_trans,
        "dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dl",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { 0 },
      }},
    {"OSF1", "T9940A", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,STK_density_trans,
        "dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dl",         1,  0,0x43, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         1,  0,0x43, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         1,  0,0x43, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         1,  0,0x43, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { 0 },
      }},
    {"OSF1", "T9940A", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,STK_density_trans,
        "dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dl",         3,  0,0x44, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          3,  0,0x44, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          3,  0,0x44, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         3,  0,0x44, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          3,  0,0x44, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         3,  0,0x44, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          3,  0,0x44, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         3,  0,0x44, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dl",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         1,  0,0x43, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         1,  0,0x43, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         1,  0,0x43, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { 0 },
    }},
    {"OSF1", "9840", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,STK_density_trans,
        "dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dl",         2,  0,0x42, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          2,  0,0x42, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          2,  0,0x42, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         2,  0,0x42, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          2,  0,0x42, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         2,  0,0x42, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          2,  0,0x42, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         2,  0,0x42, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { 0 },
      }},
     {"OSF1", "SDX-5", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, AIT_density_trans,
	"dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dh",         1,  0,0x31, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Usable */
        { "dev/nrmt%dh",         1,  1,0x31, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         0,  0,0x30, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         0,  1,0x30, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          0,  1,0x30, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          0,  1,0x30, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         0,  1,0x30, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          0,  1,0x30, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         1,  1,0x31, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          1,  1,0x31, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         1,  1,0x31, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
	{ 0 },
    }},
     {"OSF1", "SDX-3", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, AIT_density_trans,
	"dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dh",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Usable */
        { "dev/nrmt%dh",         0,  1,0x30, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         0,  0,0x30, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         0,  1,0x30, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          0,  1,0x30, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          0,  1,0x30, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         0,  1,0x30, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          0,  1,0x30, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         0,  1,0x30, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          0,  1,0x30, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         0,  1,0x30, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
	{ 0 },
    }},
     {"OSF1", "Mammoth2", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
	"dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dh",         3,  1,0x28, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",        -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Usable */
        { "dev/nrmt%dh",         3,  0,0x28, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         2,  0,0x27, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         2,  1,0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          2,  1,0x27, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          2,  1,0x27, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         2,  1,0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          2,  1,0x27, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         2,  1,0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          2,  1,0x27, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         2,  1,0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* densities */
        { "dev/nrmt%da",         0,  1,0x90, 0,  0,FTT_RDNW| FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         1,  1,0x8c, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         1,  0,0x15, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         0,  0,0x14, 0,  0,FTT_RDNW| FTT_RDNW, 0, EXB_MAX_BLKSIZE},
	{ 0 },
    }},
    {"OSF1", "EXB-8900", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
	"dev/%*[nr]mt%d", "dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/nrmt%dl",         2,  1,0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         2,  0,0x27, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descritptive */
        { "dev/rmt%dl",          2,  1,0x27, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          2,  1,0x27, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         2,  1,0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          2,  1,0x27, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",         2,  1,0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          2,  1,0x27, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         2,  1,0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* densities */
        { "dev/nrmt%da",         0,  1,0x90, 0,  0,FTT_RDNW| FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         1,  1,0x8c, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         1,  0,0x15, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         0,  0,0x14, 0,  0,FTT_RDNW| FTT_RDNW, 0, EXB_MAX_BLKSIZE},
	{ 0 },
     }},
    {"OSF1", "EXB-8500", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
	"dev/%*[nr]mt%d","dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, passthru */
        { "dev/nrmt%dh",         1,  0,0x15, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Usable */
        { "dev/nrmt%dm",         1,  0,0x15, 0,  0,          FTT_RDNW, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         0,  0,0x14, 0,  0,FTT_RDNW| FTT_RDNW, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         1,  0,0x15, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%dm",          1,  0,0x15, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          1,  0,0x15, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dl",          0,  0,0x14, 0,  0,FTT_RDNW| FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          1,  0,0x15, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          1,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
	{ 0 },
     }},
#ifdef OSF1KERNELTABLES
    {"OSF1", "EXB-8505", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
	"dev/%*[nr]mt%d","dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, passthru */
        { "dev/nrmt%dh",         1,  0,0x15, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Usable */
        { "dev/nrmt%dl",         0,  0,0x14, 0,  0,FTT_RDNW| FTT_RDNW, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         1,  1,0x8c, 0,  0,          FTT_RDNW, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         0,  1,0x90, 0,  0,          FTT_RDNW, 1, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%dl",          0,  0,0x14, 0,  0, FTT_RDNW|FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          1,  1,0x8c, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          1,  0,0x15, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          0,  1,0x90, 0,  0,FTT_RDNW| FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          1,  0,0x00, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
	{ 0 },
     }},
#else
    {"OSF1", "EXB-8505", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans,
	"dev/%*[nr]mt%d","dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, passthru */
        { "dev/nrmt%dh",         1,  0,0x15, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Usable */
        { "dev/nrmt%dm",         1,  0,0x15, 0,  0,          FTT_RDNW, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         0,  0,0x14, 0,  0,FTT_RDNW| FTT_RDNW, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         1,  0,0x15, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%dm",          1,  0,0x15, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          1,  0,0x15, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dl",          0,  0,0x14, 0,  0, FTT_RDNW|FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          1,  0,0x15, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
	{ 0 },
     }},
#endif
    {"OSF1", "DLT", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, DLT_density_trans,
	"dev/%*[nr]mt%d","dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru */
        { "dev/nrmt%dh",         5,  0,0x1A, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Usable */
        { "dev/nrmt%dl",         2,  0,0x17, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dl",         3,  0,0x18, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         4,  0,0x19, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         4,  1,0x19, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         5,  1,0x1A, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Other Densities */
        { "dev/rmt%dh",          0,  0,0x0A, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          1,  0,0x16, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          2,  0,0x17, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          3,  0,0x18, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          4,  0,0x19, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          5,  0,0x1A, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          5,  0,0x80, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          5,  1,0x81, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          5,  0,0x82, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          5,  1,0x83, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%dh",          5,  0,0x82, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dl",          4,  0,0x19, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          4,  1,0x19, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          5,  0,0x1A, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          5,  1,0x1A, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
	{ 0 },
     }},
    {"OSF1", "", "SCSI", FTT_FLAG_SUID_DRIVEID|FTT_FLAG_SUID_SCSI|FTT_FLAG_BSIZE_AFTER, FTT_OP_GET_STATUS, ftt_trans_table, Generic_density_trans,
	"dev/%*[nr]mt%d","dev/rmt%d", 1, OSF1find,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru */
        { "dev/nrmt%dh",         2,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dh",        -1,  0,  -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Usable */
        { "dev/nrmt%dl",         0,  0,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%dm",         1,  1,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/nrmt%da",         3,  1,0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%dh",          2,  0,0x00, 0,  0,          FTT_RWOC, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%dl",          0,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dm",          1,  1,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%dh",          2,  0,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%da",          3,  1,0x00, 0,  0,          FTT_RWOC, 1, EXB_MAX_BLKSIZE},
	{ 0 },
     }},
    {"AIX", "Mammoth2", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table_AIX, Exabyte_density_trans, 
	"dev/rmt%d", "dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/rmt%d.5",        3,  1, 0x28, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        3,  0, 0x28, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Variable useable */
        { "dev/rmt%d.5",        3,  0, 0x28, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  1, 0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x27, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  1, 0x27, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed useable */
        { "dev/rmt%d.5",        2,  1, 0x27, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x27, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d.5",        0,  0, 0x14, 0,  0,FTT_RDNW| FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  1, 0x90, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  0, 0x15, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  1, 0x8c, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x14, 0,  1,FTT_RDNW| FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  1, 0x90, 0,  1,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  0, 0x15, 0,  1,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  1, 0x8c, 0,  1,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
    /* Descriptive/translators */
        { "dev/rmt%d",         -1,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.1",       -1,  0, 0x00, 0,  1,          FTT_RDNW, 1, 512},
        { "dev/rmt%d.1",        0,  0, 0x14, 0,  1,FTT_RDNW| FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        1,  0, 0x15, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        1,  1, 0x8c, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        0,  1, 0x90, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        2,  0, 0x27, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        2,  1, 0x27, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.2",       -1,  0, 0x00, 0,  1, FTT_RWOC|FTT_RTOO, 1, 512},
        { "dev/rmt%d.3",       -1,  0, 0x00, 0,  1,          FTT_RTOO, 1, 512},
        { "dev/rmt%d.4",        0,  0, 0x14, 0,  0, FTT_RWOC|FTT_RDNW, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        0,  0, 0x14, 0,  0, FTT_RWOC|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x14, 0,  0,        0|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "EXB-8900", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table_AIX, Exabyte_density_trans, 
	"dev/rmt%d", "dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/rmt%d.5",        2,  1, 0x27, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Variable useable */
        { "dev/rmt%d.5",        2,  0, 0x27, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  1, 0x27, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed useable */
        { "dev/rmt%d.5",        2,  1, 0x27, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x27, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d.5",        0,  0, 0x14, 0,  0,FTT_RDNW| FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  1, 0x90, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  0, 0x15, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  1, 0x8c, 0,  0,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x14, 0,  1,FTT_RDNW| FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  1, 0x90, 0,  1,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  0, 0x15, 0,  1,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  1, 0x8c, 0,  1,          FTT_RDNW, 0, EXB_MAX_BLKSIZE},
    /* Descriptive/translators */
        { "dev/rmt%d",         -1,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.1",       -1,  0, 0x00, 0,  1,          FTT_RDNW, 1, 512},
        { "dev/rmt%d.1",        0,  0, 0x14, 0,  1,FTT_RDNW| FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        1,  0, 0x15, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        1,  1, 0x8c, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        0,  1, 0x90, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        2,  0, 0x27, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        2,  1, 0x27, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.2",       -1,  0, 0x00, 0,  1, FTT_RWOC|FTT_RTOO, 1, 512},
        { "dev/rmt%d.3",       -1,  0, 0x00, 0,  1,          FTT_RTOO, 1, 512},
        { "dev/rmt%d.4",        0,  0, 0x14, 0,  0, FTT_RWOC|FTT_RDNW, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        0,  0, 0x14, 0,  0, FTT_RWOC|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x14, 0,  0,        0|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "EXB-8505", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table_AIX, Exabyte_density_trans, 
	"dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
        { "dev/rmt%d.1",        1,  0, 0x15, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Usable, variable */
        { "dev/rmt%d.5",        0,  0, 0x14, 0,  0, FTT_RDNW|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  1, 0x90, 0,  0,FTT_RDNW|        0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",        1,  1, 0x8c, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Usable Fixed */
        { "dev/rmt%d.5",        0,  0, 0x14, 0,  1, FTT_RDNW|       0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  1, 0x90, 0,  1,FTT_RDNW|        0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",        1,  0, 0x15, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",        1,  1, 0x8c, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
   /* Descriptive */
        { "dev/rmt%d.4",        0,  0, 0x14, 0,  1, FTT_RWOC|FTT_RDNW, 1, 512},
        { "dev/rmt%d.1",        1,  0, 0x00, 0,  1,                 0, 0, 512},
        { "dev/rmt%d.5",        0,  0, 0x14, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.5",        1,  0, 0x15, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        1,  1, 0x8c, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.1",        0,  1, 0x90, 0,  1,          FTT_RDNW, 0, 512},
        { "dev/rmt%d.6",        0,  0, 0x14, 0,  1, FTT_RWOC|FTT_RTOO, 1, 512},
        { "dev/rmt%d.7",        0,  0, 0x14, 0,  1,          FTT_RTOO, 1, 512},
        { "dev/rmt%d",          1,  0, 0x00, 0,  0, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.2",        1,  0, 0x00, 0,  0, FTT_RWOC|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.3",        1,  0, 0x00, 0,  0,        0|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "EXB-8500", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table_AIX, Exabyte_density_trans, 
	"dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                den mod hwd   pas fxd rewind            1st */
    /*   ======                === === ===   === === ======            === */
    /* Default, passthru */
        { "dev/rmt%d.1",        1,  0, 0x15, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Variable Usable */
        { "dev/rmt%d.5",        0,  0, 0x14, 0,  0,FTT_RDNW|        0, 1, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.5",        0,  0, 0x14, 0,  1,FTT_RDNW|        0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",        1,  0, 0x15, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d.4",        0,  0, 0x14, 0,  1, FTT_RWOC|FTT_RDNW, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        0,  0, 0x14, 0,  1, FTT_RWOC|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x14, 0,  1,          FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d",          1,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d",          1,  0, 0x15, 0,  1, FTT_RWOC|       0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.2",        1,  0, 0x15, 0,  1, FTT_RWOC|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.3",        1,  0, 0x15, 0,  1,        0|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "EXB-8200", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table_AIX, Exabyte_density_trans, 
	"dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind         1st */
    /*   ======                  === === ===   === === ======         === */
    /* Default */
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d",          0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.2",        0,  0, 0x00, 0,  1, FTT_RWOC|FTT_RTOO, 1, 512},
        { "dev/rmt%d.3",        0,  0, 0x00, 0,  1,          FTT_RTOO, 1, 512},
        { "dev/rmt%d.4",        0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x00, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.6",        0,  0, 0x00, 0,  1, FTT_RWOC|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x00, 0,  1,        0|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "ULT3580-TD1", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_AIX,Ultrium_density_trans,
        "dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind         1st */
    /*   ======                  === === ===   === === ======         === */
    /* Default */
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d",          0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.2",        0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.3",        0,  0, 0x00, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.4",        0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x00, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.6",        0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x00, 0,  1,        0|       0, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "ULT06242-XXX", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_AIX,Ultrium_density_trans,
        "dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind         1st */
    /*   ======                  === === ===   === === ======         === */
    /* Default */
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d",          0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.2",        0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.3",        0,  0, 0x00, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.4",        0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x00, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.6",        0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x00, 0,  1,        0|       0, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "T9940A", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_AIX,STK_density_trans,
        "dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind         1st */
    /*   ======                  === === ===   === === ======         === */
    /* Default */
        { "dev/rmt%d.1",        1,  0, 0x43, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.1",        1,  0, 0x43, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d",          1,  0, 0x43, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.1",        1,  0, 0x43, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.2",        1,  0, 0x43, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.3",        1,  0, 0x43, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.4",        1,  0, 0x43, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  0, 0x43, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.6",        1,  0, 0x43, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        1,  0, 0x43, 0,  1,        0|       0, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "T9940A", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_AIX,STK_density_trans,
        "dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind         1st */
    /*   ======                  === === ===   === === ======         === */
    /* Default */
        { "dev/rmt%d.1",        3,  0, 0x44, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.1",        3,  0, 0x44, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d",          3,  0, 0x44, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.1",        3,  0, 0x44, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.2",        3,  0, 0x44, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.3",        3,  0, 0x44, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.4",        3,  0, 0x44, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        3,  0, 0x44, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.6",        3,  0, 0x44, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        3,  0, 0x44, 0,  1,        0|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d",          1,  0, 0x43, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.1",        1,  0, 0x43, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.2",        1,  0, 0x43, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.3",        1,  0, 0x43, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.4",        1,  0, 0x43, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  0, 0x43, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.6",        1,  0, 0x43, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        1,  0, 0x43, 0,  1,        0|       0, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "9840", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table_AIX,STK_density_trans,
        "dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind         1st */
    /*   ======                  === === ===   === === ======         === */
    /* Default */
        { "dev/rmt%d.1",        2,  0, 0x42, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.1",        2,  0, 0x42, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d",          2,  0, 0x42, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.1",        2,  0, 0x42, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.2",        2,  0, 0x42, 0,  1, FTT_RWOC|       0, 1, 512},
        { "dev/rmt%d.3",        2,  0, 0x42, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.4",        2,  0, 0x42, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x42, 0,  1,                 0, 1, 512},
        { "dev/rmt%d.6",        2,  0, 0x42, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        2,  0, 0x42, 0,  1,        0|       0, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "DLT7", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS|FTT_OP_ERASE,ftt_trans_table_AIX, DLT_density_trans, 
	"dev/rmt%d", "dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, passthru */
        { "dev/rmt%d.5",        6,  0, 0x84, 0,  0,                          0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",       -1,  0, 0x00, 1,  0,                          0, 0, EXB_MAX_BLKSIZE},
    /* Other Densities */
        { "dev/rmt%d.5",        4,  0, 0x80, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  1, 0x81, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        5,  0, 0x1A, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        5,  0, 0x82, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        5,  1, 0x83, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        6,  1, 0x85, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x0A, 0,  0,                   FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  0, 0x16, 0,  0,                   FTT_RDNW, 0, EXB_MAX_BLKSIZE},
    /* Variable useable */
        { "dev/rmt%d.5",        5,  1, 0x1A, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  0, 0x19, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  1, 0x19, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        3,  0, 0x18, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x17, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.5",        5,  0, 0x1A, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  0, 0x19, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        5,  1, 0x1A, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  1, 0x19, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        3,  0, 0x18, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x17, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d",         -1,  0, 0x00, 0,  1, FTT_RWOC|       0, 	 1, 512},
        { "dev/rmt%d.1",       -1,  0, 0x00, 0,  1,                 0, 	 1, 512},
        { "dev/rmt%d.2",       -1,  0, 0x00, 0,  1, FTT_RWOC|FTT_RTOO, 	 1, 512},
        { "dev/rmt%d.3",       -1,  0, 0x00, 0,  1,          FTT_RTOO, 	 1, 512},
        { "dev/rmt%d.4",        5,  0, 0x1A, 0,  0, FTT_RWOC|       0|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        5,  0, 0x1A, 0,  0, FTT_RWOC|FTT_RTOO|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        5,  0, 0x1A, 0,  0,          FTT_RTOO|       0, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "DLT4", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS|FTT_OP_ERASE,ftt_trans_table_AIX, DLT_density_trans, 
	"dev/rmt%d", "dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind            1st */
    /*   ======                  === === ===   === === ======            === */
    /* Default, passthru */
        { "dev/rmt%d.5",        5,  0, 0x1A, 0,  0,                          0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",       -1,  0, 0x00, 1,  0,                          0, 0, EXB_MAX_BLKSIZE},
    /* Other Densities */
        { "dev/rmt%d.5",        4,  0, 0x80, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  1, 0x81, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        5,  0, 0x82, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        5,  1, 0x83, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x0A, 0,  0,                   FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  0, 0x16, 0,  0,                   FTT_RDNW, 0, EXB_MAX_BLKSIZE},
    /* Variable useable */
        { "dev/rmt%d.5",        5,  1, 0x1A, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  0, 0x19, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  1, 0x19, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        3,  0, 0x18, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x17, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.5",        5,  0, 0x1A, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  0, 0x19, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        5,  1, 0x1A, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  1, 0x19, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        3,  0, 0x18, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x17, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d",         -1,  0, 0x00, 0,  1, FTT_RWOC|       0, 	 1, 512},
        { "dev/rmt%d.1",       -1,  0, 0x00, 0,  1,                 0, 	 1, 512},
        { "dev/rmt%d.2",       -1,  0, 0x00, 0,  1, FTT_RWOC|FTT_RTOO, 	 1, 512},
        { "dev/rmt%d.3",       -1,  0, 0x00, 0,  1,          FTT_RTOO, 	 1, 512},
        { "dev/rmt%d.4",        5,  0, 0x1A, 0,  0, FTT_RWOC|       0|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        5,  0, 0x1A, 0,  0, FTT_RWOC|FTT_RTOO|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        5,  0, 0x1A, 0,  0,          FTT_RTOO|       0, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "DLT2", "SCSI", FTT_FLAG_VERIFY_EOFS|FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS|FTT_OP_ERASE,ftt_trans_table_AIX, DLT_density_trans, 
	"dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                  den mod hwd   pas fxd rewind                   1st blk */
    /*   ======                  === === ===   === === ======                   === === */
    /* Default, passthru */
        { "dev/rmt%d.5",        4,  0, 0x19, 0,  0,                          0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",       -1,  0, 0x00, 1,  0,                          0, 0, EXB_MAX_BLKSIZE},
    /* Variable useable */
        { "dev/rmt%d.5",        4,  1, 0x19, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        3,  0, 0x18, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x17, 0,  0,                          0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.5",        4,  0, 0x19, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        4,  1, 0x19, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        3,  0, 0x18, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        2,  0, 0x17, 0,  1,                          0, 0, EXB_MAX_BLKSIZE},
    /* Other Densities */
        { "dev/rmt%d.5",        0,  0, 0x0A, 0,  0,                   FTT_RDNW, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        1,  0, 0x16, 0,  0,                   FTT_RDNW, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d",         -1,  0, 0x00, 0,  1, FTT_RWOC|       0, 	 1, 512},
        { "dev/rmt%d.1",       -1,  0, 0x00, 0,  1,                 0, 	 1, 512},
        { "dev/rmt%d.2",       -1,  0, 0x00, 0,  1, FTT_RWOC|FTT_RTOO, 	 1, 512},
        { "dev/rmt%d.3",       -1,  0, 0x00, 0,  1,          FTT_RTOO, 	 1, 512},
        { "dev/rmt%d.4",        4,  0, 0x19, 0,  0, FTT_RWOC|       0|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        4,  0, 0x19, 0,  0, FTT_RWOC|FTT_RTOO|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        4,  0, 0x19, 0,  0,          FTT_RTOO|       0, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "SDX-3", "SCSI", FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table_AIX, AIT_density_trans, 
	"dev/rmt%d", "dev/rmt%d", 1, AIXfind,  {
    /*   string                den mod hwd   pas fxd rewind            1st */
    /*   ======                === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/rmt%d.1",        0,  0, 0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Variable useable */
        { "dev/rmt%d.1",        0,  0, 0x30, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",        0,  1, 0x30, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x30, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Fixed useable */
        { "dev/rmt%d.1",        0,  0, 0x30, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x30, 0,  1,                 0, 1, EXB_MAX_BLKSIZE},
    /* descriptive */
        { "dev/rmt%d",          0,  0, 0x30, 0,  0,FTT_RWOC|        0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.2",        0,  0, 0x30, 0,  0,FTT_RTOO|FTT_RWOC , 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.3",        0,  0, 0x30, 0,  0,FTT_RTOO|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.4",        0,  0, 0x30, 0,  0,FTT_RWOC|        0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        0,  0, 0x30, 0,  0,FTT_RTOO|FTT_RWOC , 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x30, 0,  0,FTT_RTOO|        0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d",          0,  0, 0x30, 0,  1,FTT_RWOC|        0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.2",        0,  0, 0x30, 0,  1,FTT_RTOO|FTT_RWOC , 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.3",        0,  0, 0x30, 0,  1,FTT_RTOO|       0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.4",        0,  0, 0x30, 0,  1,FTT_RWOC|        0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        0,  0, 0x30, 0,  1,FTT_RTOO|FTT_RWOC , 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x30, 0,  1,FTT_RTOO|        0, 0, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "SDX-5", "SCSI", FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table_AIX, AIT_density_trans, 
	"dev/rmt%d", "dev/rmt%d", 1, AIXfind,  {
    /*   string                den mod hwd   pas fxd rewind            1st */
    /*   ======                === === ===   === === ======            === */
    /* Default, Passthru  */
        { "dev/rmt%d.1",        1,  0, 0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Variable useable */
        { "dev/rmt%d.1",        1,  1, 0x31, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x30, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  1, 0x30, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed useable */
        { "dev/rmt%d.1",        1,  0, 0x31, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  0, 0x30, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",        1,  1, 0x31, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.5",        0,  1, 0x30, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* descriptive */
        { "dev/rmt%d",          1,  0, 0x31, 0,  0,FTT_RWOC|        0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.2",        1,  0, 0x31, 0,  0,FTT_RTOO|FTT_RWOC , 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.3",        1,  0, 0x31, 0,  0,FTT_RTOO|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.4",        0,  0, 0x30, 0,  0,FTT_RWOC|        0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        0,  0, 0x30, 0,  0,FTT_RTOO|FTT_RWOC , 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x30, 0,  0,FTT_RTOO|        0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d",          1,  0, 0x31, 0,  1,FTT_RWOC|        0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.2",        1,  0, 0x31, 0,  1,FTT_RTOO|FTT_RWOC , 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.3",        1,  0, 0x31, 0,  1,FTT_RTOO|       0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.4",        0,  0, 0x30, 0,  1,FTT_RWOC|        0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        0,  0, 0x30, 0,  1,FTT_RTOO|FTT_RWOC , 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x30, 0,  1,FTT_RTOO|        0, 0, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    {"AIX", "", "SCSI", FTT_FLAG_HOLD_SIGNALS|FTT_FLAG_SUID_SCSI,FTT_OP_STATUS|FTT_OP_GET_STATUS,ftt_trans_table_AIX, Generic_density_trans, 
	"dev/rmt%d","dev/rmt%d", 1, AIXfind,  {
    /*   string                den mod hwd   pas fxd rewind            1st */
    /*   ======                === === ===   === === ======            === */
    /* Default, passthru */
        { "dev/rmt%d.1",        1,  0, 0x00, 0,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",       -1,  0,   -1, 1,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Variable Usable */
        { "dev/rmt%d.5",        0,  0, 0x00, 0,  0,                 0, 0, EXB_MAX_BLKSIZE},
    /* Fixed Usable */
        { "dev/rmt%d.5",        0,  0, 0x00, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.1",        1,  0, 0x00, 0,  1,                 0, 0, EXB_MAX_BLKSIZE},
    /* Descriptive */
        { "dev/rmt%d.4",        0,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.6",        0,  0, 0x00, 0,  1, FTT_RWOC|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.7",        0,  0, 0x00, 0,  1,          FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d",          1,  0, 0x00, 0,  1, FTT_RWOC|       0, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d",          1,  0, 0x00, 0,  1, FTT_RWOC|       0, 0, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.2",        1,  0, 0x00, 0,  1, FTT_RWOC|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { "dev/rmt%d.3",        1,  0, 0x00, 0,  1,        0|FTT_RTOO, 1, EXB_MAX_BLKSIZE},
        { 0, },
    }},
    /* note that %*[rmt] matches "rmt" or "mt" (or mrt, so sue me) -- mengel */
    /*      all of which we re-write into rmt */
    {"IRIX", "SDX-5","SCSI",  FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, AIT_density_trans, 
	"%*[rmt]/tps%dd%d%*[nrsv.]","rmt/tps%dd%d", 2, IRIXfind,  {
	    /*   string             den mod hwd  pas fxd rewind        sf,1st */
	    /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
	{ "rmt/tps%dd%dnrv",         1,  0,0x31, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrvc",        1,  1,0x31, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/tps%dd%dnrnsv",       1,  0,0x31, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsvc",      1,  1,0x31, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/tps%dd%dnr",          1,  0,0x31, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrc",         1,  1,0x31, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns",        1,  0,0x31, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsc",       1,  1,0x31, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            1,  0,0x31, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dc",           1,  1,0x31, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",          1,  0,0x31, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrc",         1,  1,0x31, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs",         1,  0,0x31, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsc",        1,  1,0x31, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",        1,  0,0x31, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsvc",       1,  1,0x31, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",          1,  0,0x31, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsc",         1,  1,0x31, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsvc",        1,  1,0x31, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",         1,  0,0x31, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",           1,  0,0x31, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsvc",         1,  1,0x31, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",          1,  0,0x31, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dvc",          1,  1,0x31, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",           1,  0,0x31, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ 0,},
    }},
    {"IRIX", "ULT3580-TD1", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,Ultrium_density_trans,
        "%*[rmt]/tps%dd%d%*[nrsv.]","rmt/tps%dd%d", 2, IRIXfind,  {
            /*   string             den mod hwd  pas fxd rewind        sf,1st */
            /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
        { "rmt/tps%dd%dnrv",         1,  0,0x00, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrvc",        1,  1,0x00, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Variable */
        { "rmt/tps%dd%dnrnsv",       1,  0,0x00, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsvc",      1,  1,0x00, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
        { "rmt/tps%dd%dnr",          1,  0,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         1,  1,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrns",        1,  0,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsc",       1,  1,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
        { "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%d",            1,  0,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dc",           1,  1,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnr",          1,  0,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         1,  1,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrs",         1,  0,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsc",        1,  1,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsv",        1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsvc",       1,  1,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dns",          1,  0,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsc",         1,  1,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsvc",        1,  1,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsv",         1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%ds",           1,  0,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsvc",         1,  1,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsv",          1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dvc",          1,  1,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dv",           1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0,},
    }},
    {"IRIX", "ULT06242-XXX", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,Ultrium_density_trans,
        "%*[rmt]/tps%dd%d%*[nrsv.]","rmt/tps%dd%d", 2, IRIXfind,  {
            /*   string             den mod hwd  pas fxd rewind        sf,1st */
            /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
        { "rmt/tps%dd%dnrv",         1,  0,0x00, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrvc",        1,  1,0x00, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Variable */
        { "rmt/tps%dd%dnrnsv",       1,  0,0x00, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsvc",      1,  1,0x00, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
        { "rmt/tps%dd%dnr",          1,  0,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         1,  1,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrns",        1,  0,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsc",       1,  1,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
        { "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%d",            1,  0,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dc",           1,  1,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnr",          1,  0,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         1,  1,0x00, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrs",         1,  0,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsc",        1,  1,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsv",        1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsvc",       1,  1,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dns",          1,  0,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsc",         1,  1,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsvc",        1,  1,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsv",         1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%ds",           1,  0,0x00, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsvc",         1,  1,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsv",          1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dvc",          1,  1,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dv",           1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0,},
    }},
    {"IRIX", "T9940A", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,STK_density_trans,
        "%*[rmt]/tps%dd%d%*[nrsv.]","rmt/tps%dd%d", 2, IRIXfind,  {
            /*   string             den mod hwd  pas fxd rewind        sf,1st */
            /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
        { "rmt/tps%dd%dnrv",         1,  0,0x43, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrvc",        1,  1,0x43, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Variable */
        { "rmt/tps%dd%dnrnsv",       1,  0,0x43, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsvc",      1,  1,0x43, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
        { "rmt/tps%dd%dnr",          1,  0,0x43, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         1,  1,0x43, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrns",        1,  0,0x43, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsc",       1,  1,0x43, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
        { "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%d",            1,  0,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dc",           1,  1,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnr",          1,  0,0x43, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         1,  1,0x43, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrs",         1,  0,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsc",        1,  1,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsv",        1,  0,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsvc",       1,  1,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dns",          1,  0,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsc",         1,  1,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsvc",        1,  1,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsv",         1,  0,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%ds",           1,  0,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsvc",         1,  1,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsv",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dvc",          1,  1,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dv",           1,  0,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0,},
    }},
    {"IRIX", "T9940B", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,STK_density_trans,
        "%*[rmt]/tps%dd%d%*[nrsv.]","rmt/tps%dd%d", 2, IRIXfind,  {
            /*   string             den mod hwd  pas fxd rewind        sf,1st */
            /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
        { "rmt/tps%dd%dnrv",         3,  0,0x44, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrvc",        3,  1,0x44, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Variable */
        { "rmt/tps%dd%dnrnsv",       3,  0,0x44, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsvc",      3,  1,0x44, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
        { "rmt/tps%dd%dnr",          3,  0,0x44, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         3,  1,0x44, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrns",        3,  0,0x44, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsc",       3,  1,0x44, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
        { "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%d",            3,  0,0x44, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dc",           3,  1,0x44, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnr",          3,  0,0x44, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         3,  1,0x44, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrs",         3,  0,0x44, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsc",        3,  1,0x44, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsv",        3,  0,0x44, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsvc",       3,  1,0x44, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dns",          3,  0,0x44, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsc",         3,  1,0x44, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsvc",        3,  1,0x44, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsv",         3,  0,0x44, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%ds",           3,  0,0x44, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsvc",         3,  1,0x44, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsv",          3,  0,0x44, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dvc",          3,  1,0x44, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dv",           3,  0,0x44, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%d",            1,  0,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dc",           1,  1,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnr",          1,  0,0x43, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         1,  1,0x43, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrs",         1,  0,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsc",        1,  1,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsv",        1,  0,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsvc",       1,  1,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dns",          1,  0,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsc",         1,  1,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsvc",        1,  1,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsv",         1,  0,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%ds",           1,  0,0x43, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsvc",         1,  1,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsv",          1,  0,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dvc",          1,  1,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dv",           1,  0,0x43, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0 },
    }},
    {"IRIX", "9840", "SCSI",  FTT_FLAG_VERIFY_EOFS|FTT_FLAG_MODE_AFTER|FTT_FLAG_BSIZE_AFTER, FTT_OP_STATUS, ftt_trans_table,STK_density_trans,
        "%*[rmt]/tps%dd%d%*[nrsv.]","rmt/tps%dd%d", 2, IRIXfind,  {
            /*   string             den mod hwd  pas fxd rewind        sf,1st */
            /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
        { "rmt/tps%dd%dnrv",         2,  0,0x42, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrvc",        2,  1,0x42, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Variable */
        { "rmt/tps%dd%dnrnsv",       2,  0,0x42, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsvc",      2,  1,0x42, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
        { "rmt/tps%dd%dnr",          2,  0,0x42, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         2,  1,0x42, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrns",        2,  0,0x42, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrnsc",       2,  1,0x42, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
        { "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%d",            2,  0,0x42, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dc",           2,  1,0x42, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnr",          2,  0,0x42, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrc",         2,  1,0x42, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrs",         2,  0,0x42, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsc",        2,  1,0x42, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsv",        2,  0,0x42, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnrsvc",       2,  1,0x42, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dns",          2,  0,0x42, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsc",         2,  1,0x42, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsvc",        2,  1,0x42, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dnsv",         2,  0,0x42, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%ds",           2,  0,0x42, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsvc",         2,  1,0x42, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dsv",          2,  0,0x42, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dvc",          2,  1,0x42, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { "rmt/tps%dd%dv",           2,  0,0x42, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0,},
    }},
    {"IRIX", "SDX-3","SCSI",  FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, AIT_density_trans, 
	"%*[rmt]/tps%dd%d%*[nrsv.]","rmt/tps%dd%d", 2, IRIXfind,  {
	    /*   string             den mod hwd  pas fxd rewind        sf,1st */
	    /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
	{ "rmt/tps%dd%dnrv",         0,  0,0x30, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrvc",        0,  1,0x30, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/tps%dd%dnrnsv",       0,  0,0x30, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsvc",      0,  1,0x30, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/tps%dd%dnr",          0,  0,0x30, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrc",         0,  1,0x30, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns",        0,  0,0x30, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsc",       0,  1,0x30, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            0,  0,0x30, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dc",           0,  1,0x30, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",          0,  0,0x30, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrc",         0,  1,0x30, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs",         0,  0,0x30, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsc",        0,  1,0x30, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",        0,  0,0x30, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsvc",       0,  1,0x30, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",          0,  0,0x30, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsc",         0,  1,0x30, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsvc",        0,  1,0x30, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",         0,  0,0x30, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",           0,  0,0x30, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsvc",         0,  1,0x30, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",          0,  0,0x30, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dvc",          0,  1,0x30, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",           0,  0,0x30, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ 0,},
    }},
    {"IRIX", "Mammoth2","SCSI",  FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans, 
	"%*[rmt]/tps%dd%d%*[nrsv]","rmt/tps%dd%d", 2, IRIXfind,  {
	    /*   string             den mod hwd  pas fxd rewind        sf,1st*/ 
	    /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
	{ "rmt/tps%dd%dnrvc",        3,  1,0x28, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",         3,  0,0x28, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/tps%dd%dnrnsv",       3,  0,0x28, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv",       2,  1,0x27, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv",       2,  0,0x27, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/tps%dd%dnr",          2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns",        2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            2,  1,0x27, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            1,  0,0x15, 0,  1,          FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            0,  1,0x90, 0,  1,FTT_RDNW| FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            0,  0,0x14, 0,  1,FTT_RDNW| FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            1,  1,0x8c, 0,  1,          FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",          2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs",         2,  1,0x27, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",        2,  1,0x27, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",          2,  1,0x27, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",         2,  1,0x27, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",           2,  1,0x27, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",          2,  1,0x27, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",           2,  1,0x27, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ 0,},
    }},
    {"IRIX", "EXB-89","SCSI",  FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans, 
	"%*[rmt]/tps%dd%d%*[nrsv]","rmt/tps%dd%d", 2, IRIXfind,  {
	    /*   string             den mod hwd  pas fxd rewind        sf,1st */
	    /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
	{ "rmt/tps%dd%dnrvc",        2,  1,0x27, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/tps%dd%dnrnsv",       2,  0,0x27, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/tps%dd%dnr",          2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns",        2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            2,  1,0x27, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            1,  0,0x15, 0,  1,          FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            0,  1,0x90, 0,  1,FTT_RDNW| FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            0,  0,0x14, 0,  1,FTT_RDNW| FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            1,  1,0x8c, 0,  1,          FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",          2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs",         2,  1,0x27, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",        2,  1,0x27, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",          2,  1,0x27, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",         2,  1,0x27, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",           2,  1,0x27, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",          2,  1,0x27, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",           2,  1,0x27, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ 0,},
    }},
    {"IRIX", "EXB-82","SCSI",  FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans, 
	"%*[rmt]/tps%dd%d%*[nrsv.]890%d","rmt/tps%dd%d.8900", 3, IRIXfind,  {
	    /*   string             den mod hwd  pas fxd rewind        sf,1st */
	    /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
	{ "rmt/tps%dd%dnrv",         2,  1,0x27, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/tps%dd%dnrnsv",       2,  1,0x27, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/tps%dd%dnr",          2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns",        2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            2,  1,0x27, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            1,  0,0x15, 0,  1,          FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            0,  1,0x90, 0,  1,FTT_RDNW| FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            0,  0,0x14, 0,  1,FTT_RDNW| FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            1,  1,0x8c, 0,  1,          FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",          2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs",         2,  1,0x27, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",        2,  1,0x27, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",          2,  1,0x27, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",         2,  1,0x27, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",           2,  1,0x27, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",          2,  1,0x27, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",           2,  1,0x27, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ 0,},
    }},
    {"IRIX", "DLT2", "SCSI", FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS|FTT_OP_ERASE, ftt_trans_table, DLT_density_trans, 
	"%*[rmt]/tps%dd%d","rmt/tps%dd%d", 2, IRIXfind, {
    /*   string                    den mod hwd   pas fxd rewind            ,1st*/
    /*   ======                    === === ===   === === ======            === */
    /* Default, passthru */
	{ "rmt/tps%dd%dnrv",   4,  0,0x19, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",   -1,  0,  -1,   1,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/tps%dd%dnrv",   4,  0,0x19, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsvc",4,  1,0x19, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",   3,  0,0x18, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",   2,  0,0x17, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/tps%dd%dnr",    4,  0,0x19, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrc",   4,  1,0x19, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",    3,  0,0x18, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",    3,  0,0x17, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
    /* Other Densities */
	{ "rmt/tps%dd%d",      4,  0,0x80, 0,  1,          FTT_RDNW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dc",     4,  1,0x81, 0,  1,          FTT_RDNW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      0,  0,0x0A, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      1,  0,0x16, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      2,  0,0x17, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      3,  0,0x18, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      4,  0,0x19, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      5,  0,0x1A, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/tps%dd%dstat", -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs" ,  4,  0,0x1A, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsc" , 4,  1,0x1A, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",  4,  0,0x1A, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsvc", 4,  1,0x1A, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",    4,  0,0x1A, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsc",   4,  1,0x1A, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",   4,  0,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsvc",  4,  1,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",     4,  0,0x1A, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsc",    4,  1,0x1A, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",    4,  0,0x1A, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsvc",   4,  1,0x1A, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",     4,  0,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dvc",    4,  1,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0,},
    }},
    {"IRIX", "DLT4", "SCSI", FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS|FTT_OP_ERASE, ftt_trans_table, DLT_density_trans, 
	"%*[rmt]/tps%dd%d","rmt/tps%dd%d", 2, IRIXfind, {
    /*   string                    den mod hwd   pas fxd rewind            ,1st*/
    /*   ======                    === === ===   === === ======            === */
    /* Default, passthru */
	{ "rmt/tps%dd%dnrv",   5,  0,0x1A,   0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",   -1,  0,  -1,   1,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/tps%dd%dnrvc",  5,  1,0x1A, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsvc",5,  1,0x1A, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",   4,  0,0x19, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsvc",4,  1,0x19, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",   3,  0,0x18, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",   2,  0,0x17, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/tps%dd%dnr",    5,  0,0x1A, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrc",   5,  1,0x1A, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",    4,  0,0x19, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrc",   4,  1,0x19, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",    3,  0,0x18, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",    3,  0,0x17, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
    /* Other Densities */
	{ "rmt/tps%dd%d",      4,  0,0x80, 0,  1,          FTT_RDNW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dc",     4,  1,0x81, 0,  1,          FTT_RDNW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      5,  0,0x82, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dc",     5,  1,0x83, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      0,  0,0x0A, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      1,  0,0x16, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      2,  0,0x17, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      3,  0,0x18, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      4,  0,0x19, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      5,  0,0x1A, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/tps%dd%dstat", -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns",  5,  0,0x1A, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsc", 5,  1,0x1A, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv", 5,  0,0x1A, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs" ,  5,  0,0x1A, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsc" , 5,  1,0x1A, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",  5,  0,0x1A, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsvc", 5,  1,0x1A, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",    5,  0,0x1A, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsc",   5,  1,0x1A, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",   5,  0,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsvc",  5,  1,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",     5,  0,0x1A, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsc",    5,  1,0x1A, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",    5,  0,0x1A, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsvc",   5,  1,0x1A, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",     5,  0,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dvc",    5,  1,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0,},
    }},
    {"IRIX", "DLT7", "SCSI", FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS|FTT_OP_ERASE, ftt_trans_table, DLT_density_trans, 
	"%*[rmt]/tps%dd%d","rmt/tps%dd%d", 2, IRIXfind, {
    /*   string               den mod hwd   pas fxd rewind            ,1st*/
    /*   ======               === === ===   === === ======            === */
    /* Default, passthru */
	{ "rmt/tps%dd%dnrv",   6,  0,0x84, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",   -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/tps%dd%dnrvc",  6,  1,0x85, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",   5,  0,0x1A, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrvc",  5,  1,0x1A, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsvc",5,  1,0x1A, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",   4,  0,0x19, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsvc",4,  1,0x19, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",   3,  0,0x18, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",   2,  0,0x17, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/tps%dd%dnr",    5,  0,0x1A, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrc",   5,  1,0x1A, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",    4,  0,0x19, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrc",   4,  1,0x19, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",    3,  0,0x18, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",    3,  0,0x17, 0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
    /* Other Densities */
	{ "rmt/tps%dd%d",      4,  0,0x80, 0,  1,          FTT_RDNW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dc",     4,  1,0x81, 0,  1,          FTT_RDNW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      5,  0,0x82, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dc",     5,  1,0x83, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      0,  0,0x0A, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      1,  0,0x16, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      2,  0,0x17, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      3,  0,0x18, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      4,  0,0x19, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",      5,  0,0x1A, 0,  1,          FTT_RDNW, 0, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/tps%dd%dstat", -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns",  5,  0,0x1A, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsc", 5,  1,0x1A, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv", 5,  0,0x1A, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs" ,  5,  0,0x1A, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsc" , 5,  1,0x1A, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",  5,  0,0x1A, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsvc", 5,  1,0x1A, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",    5,  0,0x1A, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsc",   5,  1,0x1A, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",   5,  0,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsvc",  5,  1,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",     5,  0,0x1A, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsc",    5,  1,0x1A, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",    5,  0,0x1A, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsvc",   5,  1,0x1A, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",     5,  0,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dvc",    5,  1,0x1A, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0,},
    }},
    {"IRIX", "EXB-85", "SCSI", FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans, 
	"%*[rmt]/tps%dd%d","rmt/tps%dd%d",  2, IRIXfind,  {
	    /*   string            den mod hwd   pas fxd rewind            1st */
	    /*   ======            === === ===   === === ======            === */
     /* Default, passthru */
	{ "rmt/tps%dd%dnrnsv.8500",  1,  0,0x15, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
     /* Usable Variable */
	{ "rmt/tps%dd%dnrnsv.8200",  0,  0,0x14, 0,  0,FTT_RDNW|        0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv.8200c", 0,  1,0x8C, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv.8500c", 1,  1,0x90, 0,  0,FTT_RDNW|        0, 1, IRIX_MAX_BLKSIZE},
     /* Usable Fixed */
	{ "rmt/tps%dd%dnrns.8200",   0,  0,0x14, 0,  1,FTT_RDNW|        0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns.8200c",  0,  1,0x8C, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns.8500",   1,  0,0x15, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns.8500c",  0,  1,0x90, 0,  1, FTT_RDNW|       0, 1, IRIX_MAX_BLKSIZE},
     /* Descriptive */
	{ "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            1,  0,0x15, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d.8200c",      0,  1,0x8C, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d.8200",       0,  0,0x14, 0,  1,FTT_RDNW| FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d.8500",       1,  0,0x15, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d.8500c",      1,  1,0x90, 0,  1,FTT_RDNW| FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",          1,  0,0x15, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr.8200",     0,  0,0x14, 0,  1,FTT_RDNW|        0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr.8200c",    0,  1,0x8C, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr.8500",     1,  0,0x15, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr.8500c",    0,  1,0x90, 0,  1,FTT_RDNW|        0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrns",        1,  0,0x15, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv",       1,  0,0x15, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv.8200",  0,  0,0x14, 0,  0,FTT_RDNW|        0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv.8200c", 0,  1,0x8C, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv.8500",  1,  0,0x15, 0,  0,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrnsv.8500c", 1,  1,0x90, 0,  0, FTT_RDNW|       0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs",         1,  0,0x15, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs.8200",    0,  0,0x14, 0,  1,FTT_RDNW| FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs.8200c",   0,  1,0x8C, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs.8500",    1,  0,0x15, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs.8500c",   1,  1,0x90, 0,  1,FTT_RDNW| FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",        1,  0,0x15, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv.8200",   0,  0,0x14, 0,  0,FTT_RDNW| FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv.8200c",  0,  1,0x8C, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv.8500",   1,  0,0x15, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv.8500c",  1,  1,0x90, 0,  0, FTT_RDNW|FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv",         1,  0,0x15, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv.8200",    0,  0,0x15, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv.8200c",   0,  1,0x8C, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv.8500",    1,  0,0x15, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrv.8500c",   1,  1,0x90, 0,  0,FTT_RDNW|        0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",          1,  0,0x15, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns.8200",     0,  0,0x14, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns.8200c",    0,  1,0x8C, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns.8500",     1,  0,0x15, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns.8500c",    1,  1,0x90, 0,  1, FTT_RDNW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",         1,  0,0x15, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv.8200",    0,  0,0x14, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv.8200c",   0,  1,0x8C, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv.8500",    1,  0,0x15, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv.8500c",   1,  1,0x90, 0,  0,FTT_RDNW| FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",           1,  0,0x15, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds.8200",      0,  0,0x14, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds.8200c",     0,  1,0x8C, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds.8500",      1,  0,0x15, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds.8500c",     1,  1,0x90, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",          1,  0,0x15, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv.8200",     0,  0,0x14, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv.8200c",    0,  1,0x8C, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv.8500",     1,  0,0x15, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv.8500c",    1,  1,0x90, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",           1,  0,0x15, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv.8200",      0,  0,0x14, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv.8200c",     0,  1,0x8C, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv.8500",      1,  0,0x15, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv.8500c",     1,  1,0x90, 0,  0,FTT_RDNW| FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv.8500",      1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0,},
    }},
    {"IRIX", "EXB-85", "JAG", FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans, 
	"%*[rmt]/jag%dd%d","rmt/jag%dd%d", 2, IRIXfindVME, {
	    /*   string                  den mod hwd pas fxd rewind            1st */
	    /*   ======                  === === === === === ======            === */
    /* Default, passthru */
	{ "rmt/jag%dd%dnrv.8500",    1,  0,0x15, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/jag%dd%dl0",        -1,  0,  -1, 1,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/jag%dd%dnrv.8200",    0,  0,0x14, 0,  0,FTT_RDNW|        0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/jag%dd%dnr.8500",     1,  0,0x15, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnr.8200",     0,  0,0x14, 0,  1,FTT_RDNW|        0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/jag%dd%dstat",       -1,  0,0x15, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d",            1,  0,0x15, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d.8200",       0,  0,0x14, 0,  1,FTT_RDNW| FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d.8500",       1,  0,0x15, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnr",          1,  0,0x15, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnrv",         1,  0,0x15, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dv",           1,  0,0x15, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dv.8200",      0,  0,0x14, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dv.8500",      1,  0,0x15, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dv",           1,  0,0x00, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
        { 0,},
    }},
    {"IRIX", "EXB-82","JAG",  FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans, 
	"%*[rmt]/ja8900g%dd%d","rmt/ja8900g%dd%d", 2, IRIXfindVME, {
	    /*   string             den mod hwd  pas fxd rewind        sf,1st */
	    /*   ======             === === ===  === === ======        ==  = */
    /* Default, Passthru */
	{ "rmt/jag%dd%dnrv",         2,  1,0x27, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/jag%dd%dnrnsv",       2,  1,0x27, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/jag%dd%dnr",          2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnrns",        2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/jag%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d",            2,  1,0x27, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d",            1,  0,0x15, 0,  1,          FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d",            0,  1,0x90, 0,  1,FTT_RDNW| FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d",            0,  0,0x14, 0,  1,FTT_RDNW| FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d",            1,  1,0x8c, 0,  1,          FTT_RWOC, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnr",          2,  1,0x27, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnrs",         2,  1,0x27, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnrsv",        2,  1,0x27, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dns",          2,  1,0x27, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnsv",         2,  1,0x27, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%ds",           2,  1,0x27, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dsv",          2,  1,0x27, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dv",           2,  1,0x27, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ 0,},
    }},
    {"IRIX", "DLT","JAG",  FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, DLT_density_trans, 
	"%*[rmt]/jag%dd%d","rmt/jag%dd%d", 2, IRIXfindVME, {
    /*   string                     den mod hwd pas fxd rewind         sf,1st */
    /*   ======                     === === === === === ======         === */
	/* Default, passthru */
	{ "rmt/jag%dd%dnrv",         5,  0, 0x1A,0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/jag%dd%dl0",        -1,  0,  -1, 1,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	/* Usable Fixed */
	{ "rmt/jag%dd%dnr",          5,  0, 0x1A,0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnr",          4,  0, 0x19,0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnr",          3,  0, 0x18,0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dnr",          2,  0, 0x17,0,  1,                 0, 0, IRIX_MAX_BLKSIZE},
	/* Descriptive */
	{ "rmt/jag%dd%dstat",       -1,  0,  -1, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d",            5,  0, 0x1A,0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dv",           5,  0, 0x1A,0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ 0,},
    }},
    {"IRIX", "","JAG",  FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, Generic_density_trans, 
	"%*[rmt]/jag%dd%d","rmt/jag%dd%d", 2, IRIXfindVME, {
    /*   string                          den mod hwd pas fxd rewind         sf,1st */
    /*   ======                          === === === === === ======         === */
	/* Default, passthru */
	{ "rmt/jag%dd%dnrv",         0,  0,   0, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/jag%dd%dl0",        -1,  0,  -1, 1,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	/* Usable Fixed */
	{ "rmt/jag%dd%dnr",          0,  0,   0, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	/* Descriptive */
	{ "rmt/jag%dd%dstat",       -1,  0,   0, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%d",            0,  0,   0, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/jag%dd%dv",           0,  0,   0, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ 0,},
    }},
    {"IRIX", "","SCSI",  FTT_FLAG_CHK_BOT_AT_FMK|FTT_FLAG_BSIZE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, Generic_density_trans, 
	"%*[rmt]/tps%dd%d","rmt/tps%dd%d", 2, IRIXfind,  {
	    /*   string                  den mod hwd  pas fxd rewind            sf,1st */
	    /*   ======                  === === ===  === === ======            === */
    /* Default, Passthru */
	{ "rmt/tps%dd%dnrv",         0,  0,   0, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "scsi/sc%dd%dl0",         -1,  0,  -1, 1,  0,                 0, 1, EXB_MAX_BLKSIZE},
    /* Usable Variable */
	{ "rmt/tps%dd%dnrnsv",       0,  0,   0, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Usable Fixed */
	{ "rmt/tps%dd%dnrns",        0,  0,   0, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
    /* Descriptive */
	{ "rmt/tps%dd%dstat",       -1,  0,  -1, 0,  0,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%d",            0,  0,   0, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnr",          0,  0,   0, 0,  1,                 0, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrs",         0,  0,   0, 0,  1,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnrsv",        0,  0,   0, 0,  0,          FTT_BTSW, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dns",          0,  0,   0, 0,  1,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dnsv",         0,  0,   0, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%ds",           0,  0,   0, 0,  1, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dsv",          0,  0,   0, 0,  0, FTT_BTSW|FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ "rmt/tps%dd%dv",           0,  0,   0, 0,  0,          FTT_RWOC, 1, IRIX_MAX_BLKSIZE},
	{ 0,},
    }},
{"WINNT", "", "", FTT_FLAG_BSIZE_AFTER|FTT_FLAG_MODE_AFTER,FTT_OP_GET_STATUS, ftt_trans_table, Exabyte_density_trans, 
	".\\tape%d",".\\tape%d", 1, Win32find_dev, {
	 /* string		den mod hwd pas fxd rewind                   1st */
	 /* ======		=== === === === === ======                   === */
	{ ".\\tape%d",	 0,  0,	  0, 0,  0,                 0, 1, WIN_MAX_BLKSIZE},
	{ ".\\tape%d",	 0,  1,	  0, 0,  0,                 0, 0, WIN_MAX_BLKSIZE},
	{ ".\\tape%d",	 0,  0,	  0, 0,  1,                 0, 0, WIN_MAX_BLKSIZE},
	{ ".\\tape%d",	 0,  1,	  0, 0,  1,                 0, 0, WIN_MAX_BLKSIZE},
	{ ".\\tape%d",	-1,  0,	 -1, 1,  0,                 0, 0, WIN_MAX_BLKSIZE},
    { 0,},
    }},
#ifdef TABLE_TEST_TESTER
/* 
** the following two entries are for testing the table tester 
** these should get lots of errors
*/
    {"TABLE_TEST", "TABLE_TEST","NONE",  0, 0, 0, 0,
	"x/x%dx%d","x/x%dx%d", 2, IRIXfindVME, {
	/*   string      den mod hwd pas fxd rewind 1st */
	/*   ======      === === === === === ====== === */
	{ "x/x%dx%d",      0, 0,  0,  0,  0,  0,     0, SUN_MAX_BLKSIZE},
	{ "x/x%dx%d",      0, 0,  0,  0,  0,  0,     0, SUN_MAX_BLKSIZE},
	{ "x/x%d/%d/%d/%d",0, 0,  0,  0,  0,  0,     1, SUN_MAX_BLKSIZE},
	{ "x/x%d%d%d",     0, 0,  0,  0,  0,  0,     1, SUN_MAX_BLKSIZE},
	{ "x/x%g%d%g",     0, 0,  0,  0,  0,  0,     1, SUN_MAX_BLKSIZE},
	{ 0 , },
    }},
    {"TABLE_TEST", "TABLE_TEST","NONE",  0, 0, 0, 0,
	"x/x%dx%d","x/x%dx%d", 2, IRIXfindVME, {
	/*   string   den mod hwd pas fxd rewind 1st */
	/*   ======   === === === === === ====== === */
	{ "foo%d%d",   0,  0,  0,  0,  0,  0,     1, SUN_MAX_BLKSIZE},
	{ 0 , },
    }},
#endif
/* 
** Generic we-dont-know-what-it-is device
*/
    {"", "", "unknown", FTT_FLAG_REOPEN_AT_EOF, 0, ftt_trans_table, 
	Generic_density_trans, "%s", "%s", 1, "echo", {
	/*   string   den mod hwd    pas fxd rewind 1st */
	/*   ======   === === ===    === === ====== === */
	{ "%s",      0,  0,  0,  0,  0,  0,     1, SUN_MAX_BLKSIZE},
	{ 0,},
    }},
    {0, },
};

int devtable_size = sizeof(devtable);


ftt_stat_entry ftt_stat_op_tab[] = {
    {"2SDX",
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_MS_Px0f|FTT_DO_RS|
	FTT_DO_SN|FTT_DO_LS|FTT_DO_RP|FTT_DO_VSRS|FTT_DO_AITRS},

    {"1EXB-8200", 
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS|FTT_DO_EXBRS|
	FTT_DO_EXB82FUDGE},

    {"2EXB-8200", 
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS|FTT_DO_EXBRS|
	FTT_DO_EXB82FUDGE},

    {"2EXB-8510", 
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS|
	FTT_DO_EXBRS|FTT_DO_SN|FTT_DO_RP_SOMETIMES},

    {"1EXB-8500", 
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS|
	FTT_DO_EXBRS|FTT_DO_SN|FTT_DO_RP_SOMETIMES},

    {"2EXB-8500", 
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS|
	FTT_DO_EXBRS|FTT_DO_SN|FTT_DO_RP_SOMETIMES},

    {"2EXB-8505", 
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_MS_Px20_EXB|FTT_DO_RS|
	FTT_DO_EXBRS| FTT_DO_05RS|FTT_DO_SN|FTT_DO_LS|
	FTT_DO_RP_SOMETIMES},

    {"2EXB-8205", 
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_MS_Px20_EXB|FTT_DO_RS|
	FTT_DO_EXBRS| FTT_DO_05RS|FTT_DO_SN|FTT_DO_LS|
	FTT_DO_RP_SOMETIMES},

    {"2EXB-8900", 
	FTT_DO_VSRS|
        FTT_DO_MS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_RS|
	FTT_DO_EXBRS|FTT_DO_05RS|FTT_DO_SN|FTT_DO_MS_Px0f|
	FTT_DO_LS|FTT_DO_RP_SOMETIMES|FTT_DO_MS_Px21},

    {"2Mammoth2", 
	FTT_DO_VSRS|
        FTT_DO_MS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_RS|
	FTT_DO_EXBRS|FTT_DO_05RS|FTT_DO_SN|FTT_DO_MS_Px0f|
	FTT_DO_LS|FTT_DO_RP_SOMETIMES|FTT_DO_MS_Px21},
    
    {"3ULT3580-TD1",
        FTT_DO_VSRS|
        FTT_DO_MS|
        FTT_DO_TUR|FTT_DO_INQ|FTT_DO_RS|
        FTT_DO_SN|FTT_DO_MS_Px0f|FTT_DO_MS_Px10|
        FTT_DO_LS|FTT_DO_RP_SOMETIMES},
    
    {"3ULT06242-XXX",
        FTT_DO_VSRS|
        FTT_DO_MS|
        FTT_DO_TUR|FTT_DO_INQ|FTT_DO_RS|
        FTT_DO_SN|FTT_DO_MS_Px0f|FTT_DO_MS_Px10|
        FTT_DO_LS|FTT_DO_RP_SOMETIMES},

    {"2ULT3580-TD1",
        FTT_DO_VSRS|
        FTT_DO_MS|
        FTT_DO_TUR|FTT_DO_INQ|FTT_DO_RS|
        FTT_DO_SN|FTT_DO_MS_Px0f|FTT_DO_MS_Px10|
        FTT_DO_LS|FTT_DO_RP_SOMETIMES},

   {"3T9940A",
        FTT_DO_VSRS|
        FTT_DO_MS|
        FTT_DO_TUR|FTT_DO_INQ|FTT_DO_RS|
        FTT_DO_SN|FTT_DO_MS_Px0f|FTT_DO_MS_Px10|
        FTT_DO_LS|FTT_DO_RP_SOMETIMES},

   {"3T9940B",
        FTT_DO_VSRS|
        FTT_DO_MS|
        FTT_DO_TUR|FTT_DO_INQ|FTT_DO_RS|
        FTT_DO_SN|FTT_DO_MS_Px0f|FTT_DO_MS_Px10|
        FTT_DO_LS|FTT_DO_RP_SOMETIMES},

   {"39840",
        FTT_DO_VSRS|
        FTT_DO_MS|
        FTT_DO_TUR|FTT_DO_INQ|FTT_DO_RS|
        FTT_DO_SN|FTT_DO_MS_Px0f|FTT_DO_MS_Px10|
        FTT_DO_LS|FTT_DO_RP_SOMETIMES},

    {"2DLT",      
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_MS_Px10|FTT_DO_RS|
	FTT_DO_DLTRS|FTT_DO_SN|FTT_DO_LS|FTT_DO_RP},

    {"2SDLT320",      
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_MS_Px10|FTT_DO_RS|
	FTT_DO_DLTRS|FTT_DO_SN|FTT_DO_LS|FTT_DO_RP},

    {"2TZ8",      
	FTT_DO_VSRS|
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_MS_Px0f|FTT_DO_RS|
	FTT_DO_DLTRS|FTT_DO_SN|FTT_DO_LS|FTT_DO_RP},

    {"3", 
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_MS_Px0f|FTT_DO_RS|
	FTT_DO_SN|FTT_DO_LS|FTT_DO_RP},

    {"2", 
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_MS_Px0f|FTT_DO_RS|
	FTT_DO_SN|FTT_DO_LS|FTT_DO_RP},

    {"1", 
	FTT_DO_TUR|FTT_DO_INQ|FTT_DO_MS|FTT_DO_RS },

    {"", 0},

    {0,0}
};
                                                                                                                                                                                                                                                                                                                                                                                                                         home/vsergeev/ftt/ftt_lib/ftt_tables.old.c                                                          0100660 0022366 0022366 00000040024 06252127513 021400  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            home/vsergeev/ftt/ftt_lib/ftt_version.c                                                             0100660 0022366 0022366 00000000146 06421473340 021037  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               
char ftt_version[] = "ftt release v2_0 : $Id$";
                                                                                                                                                                                                                                                                                                                                                                                                                          home/vsergeev/ftt/ftt_lib/listdemo.c                                                                0100660 0022366 0022366 00000000241 06341113765 020314  0                                                                                                    ustar   vsergeev                        vsergeev                                                                                                                                                                                                               static char rcsid[] = "@(#)$Id$";
#include <stdio.h>
#include <ftt.h>

main() {
	ftt_list_supported(stdout);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               