static char rcsid[] = "@(#)$Id$";
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

	res = ftt_do_scsi_command(d,"Get Partition table", cdb_modsen11, 6, buf, BD_SIZE+136, 10, 0);
	if (res < 0) return res;
	p->max_parts = buf[BD_SIZE+2];
	p->n_parts = buf[BD_SIZE+3];
	for( i = 0 ; i <= p->n_parts; i++ ) {
	    p->partsizes[i] = pack(0,0,buf[BD_SIZE+8+2*i],buf[BD_SIZE+8+2*i+1]);
	}
	return 0;
   }
}


int		
ftt_write_partitions(ftt_descriptor d,ftt_partbuf p) {
    static unsigned char buf[BD_SIZE+136];
    static unsigned char cdb_modsen11[6] = {0x1a, DBD, 0x11, 0x00,BD_SIZE+136, 0x00};
    static unsigned char cdb_modsel[6] = {0x15, 0x10, 0x00, 0x00,BD_SIZE+136, 0x00};
    int res, i;
    int len;

    if ((d->flags & FTT_FLAG_SUID_SCSI) && 0 != geteuid()) {
        int pd[2], save;
        FILE *topipe;

        /*
        ** Redirection here is kind of hairy, to avoid re-doing ftt_fork().
        ** we redirect stdin *before* forking to be the read end of a new
	** pipe, saving the original stdin in "save"...
        */
        pipe(pd);
        fseek(stdin,0L,SEEK_CUR);
        save = dup(0);
	close(0);
	dup2(pd[0],0);

        /*
        ** now we make a file handle out of the write end of the pipe
	** that we will ftt_dump_partitions() onto, and fork...
        */
        topipe = fdopen(pd[1],"w");
	ftt_close_dev(d);
	switch(ftt_fork(d)){
	case -1:
		return -1;

	case 0:  /* child */
		/*
		** since stdin is already redirected, we need do nothing...
                ** ... except flush it...
                */
		if (ftt_debug) {
		    execlp("ftt_suid", "ftt_suid", "-x",  "-u", d->basename, 0);
		} else {
		     execlp("ftt_suid", "ftt_suid", "-u", d->basename, 0);
		}
		break;

	default: /* parent */
		/*
                ** Here in the parent, we need to put stdin back...
                */
	        close(0);
                dup2(save,0);
                close(save);
		/* close the read end of the pipe... */
                close(pd[0]);
		/* send the child the partition data */
		ftt_dump_partitions(p,topipe);
  		fclose(topipe);
		res = ftt_wait(d);
	}

    } else {
	res = ftt_do_scsi_command(d,"Get Partition table", cdb_modsen11, 6, buf, BD_SIZE+136, 10, 0);
	if (res < 0) return res;

	buf[0] = 0;
	buf[1] = 0;

	len = buf[BD_SIZE+1] + BD_SIZE + 2;

        cdb_modsel[4] = len;

        DEBUG3(stderr,"Got length of %d\n", len);

	/* set number of partitions */
	buf[BD_SIZE+3] = p->n_parts;

	/* set to write initiator defined partitions, in MB */
	buf[BD_SIZE+4] = 0x20 | 0x10;

	/* fill in partition sizes... */
	for( i = 0 ; i <= p->n_parts; i++ ) {
	    buf[BD_SIZE+8 + 2*i + 0] = (p->partsizes[i] & 0xff00) >> 8;
	    buf[BD_SIZE+8 + 2*i + 1] = p->partsizes[i] & 0x00ff;
	}
	for( i = p->n_parts + 1 ; i <= p->max_parts; i++ ) {
	    buf[BD_SIZE+8 + 2*i + 0] = 0;
	    buf[BD_SIZE+8 + 2*i + 1] = 0;
	}
	res = ftt_do_scsi_command(d,"Put Partition table", cdb_modsel, 6, buf, len, 3600, 1);
	return res;
    }
}

int
ftt_cur_part(ftt_descriptor d) {
    int res;
    static unsigned char buf[20];
    static unsigned char cdb_read_position[]= {0x34, 0x00, 0x00, 0x00, 0x00,
					    0x00, 0x00, 0x00, 0x00, 0x00};

    res = ftt_do_scsi_command(d,"Read Position", cdb_read_position, 10, 
				  buf, 20, 10, 0);
	
    if (res < 0) {
        return -1;
    } else {
        return buf[1];
    }
}

int		
ftt_skip_part(ftt_descriptor d,int nparts) {
    int cur;
    int res = 0;
    static unsigned char 
        locate_cmd[10] = {0x2b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    d->data_direction = FTT_DIR_READING;
    d->current_block = 0;
    d->current_file = 0;
    d->current_valid = 1;
    d->last_pos = -1;   /* we skipped backwards, so this can't be valid */

    cur = ftt_cur_part(d);
    cur += nparts;
    locate_cmd[1] = 0x02;
    locate_cmd[8] = cur;
    res = ftt_do_scsi_command(d,"Locate",locate_cmd,10,NULL,0,300,0);
    res = ftt_describe_error(d,0,"a SCSI pass-through call", res,"Locate", 0);

    return res;
}

int		
ftt_locate_part(ftt_descriptor d, int blockno, int part) {
    int cur;
    int res = 0;
    static unsigned char 
        locate_cmd[10] = {0x2b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    locate_cmd[1] = 0x02;
    locate_cmd[3] = (blockno >> 24) & 0xff;
    locate_cmd[4] = (blockno >> 16) & 0xff;
    locate_cmd[5] = (blockno >> 8)  & 0xff; 
    locate_cmd[6] = blockno & 0xff;
    locate_cmd[8] = part;
    if ( blockno == 0 ) {
	d->current_block = 0;
	d->current_file = 0;
	d->current_valid = 1;
    } else {
	d->current_valid = 0;
    }
    d->data_direction = FTT_DIR_READING;
    d->last_pos = -1;   /* we skipped backwards, so this can't be valid */

    res = ftt_do_scsi_command(d,"Locate",locate_cmd,10,NULL,0,300,0);
    res = ftt_describe_error(d,0,"a SCSI pass-through call", res,"Locate", 0);

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
    return;
}

ftt_undump_partitions(ftt_partbuf p, FILE *pf) {
    char buf[80];
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
ftt_load_partition(ftt_descriptor d, int partno) {
    int res = 0;
    ftt_partbuf p;
    static unsigned char buf[BD_SIZE+6];
    static unsigned char cdb_modsense[6] = {0x1a, DBD, 0x21, 0x00, 10, 0x00};
    static unsigned char cdb_modsel[6] = {0x15, 0x10, 0x00, 0x00, 10, 0x00};
    int len;
    int max;

    /* get maximum number of partitions.. */
    p = ftt_alloc_parts();
    ftt_get_partitions(d,p);
    max = ftt_extract_maxparts(p);
    ftt_free_parts(p);

    /* -1 means highest supported partition */
    if ( partno < 0 || partno > max ) partno = max;

    res = ftt_do_scsi_command(d,"Mode Sense, 0x21", cdb_modsense, 6, buf, 10, 10, 0);
    if (res < 0) return res;

    buf[0] = 0;
    buf[1] = 0;

    len = buf[BD_SIZE+1] + BD_SIZE + 2;

    /* set load partition */
    buf[BD_SIZE+3] = (partno << 1) & 0x7e;

    /* reserved fields */
    buf[BD_SIZE+2] = 0;
    buf[BD_SIZE+4] = 0;
    buf[BD_SIZE+5] = 0;

    res = ftt_do_scsi_command(d,"Mode Select, 0x21", cdb_modsel, 6, buf, len, 10, 1);
    return res;
}
