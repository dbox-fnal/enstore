/*************************************************************
 *
 * $Id$
 *
 *************************************************************/

%module drivestat

%{
#include "drivestat.h"
#include "ds_api.h"

DS_DESCRIPTOR *ds_open(DS_OPEN_PARMS *parms, int flag);
DS_REPORT *ds_getnext(int list_sd);

DS_DESCRIPTOR *ds_open_from_ftt_d(ftt_descriptor ftt_d){
	DS_OPEN_PARMS *parms;
	DS_DESCRIPTOR *ds_desc=NULL;

	parms = (DS_OPEN_PARMS *) malloc(sizeof(DS_OPEN_PARMS));
	if (parms==NULL) {
	    printf("FATAL ERROR: Can not malloc space for parms.\n");
	    return NULL;	
	}
	parms->ftt_d = ftt_d;

	ds_desc = ds_open(parms, FTT_DESC);
	if (ds_desc==NULL) {
	    printf("FATAL ERROR: ds_open returned NULL.\n");
	    /* fall thru and return NULL*/
	}

	return ds_desc;
}

DS_STATS * ds_get_stats_buff(void) {
	DS_STATS *stats_buff;

	stats_buff = (DS_STATS *) malloc(sizeof(DS_STATS));
	if (stats_buff==NULL) {
	    printf("FATAL ERROR: Can not malloc space for stats_buff.\n");
	    /* fall thru and return NULL*/
	}

	return stats_buff;
}

int ds_print_report(void) {
   DS_REPORT *report;
   int n;
   int sd;
   int i;
  
   sd = ds_prepare_list(NULL,NULL,NULL,NULL,NULL,NULL,NULL,&n); 
   if ( sd < 0) {
       printf("ERROR ds_prepare_list status = %d\n",sd);
       return(-1);
   }

   for (i=0;i<n;i++) {
    report = ds_getnext(sd);
    if (report != NULL) {
       printf("Drive Serial Number: %s\n",report->drive_serial_number);
       printf("Vendor: %s\n",report->vendor);
       printf("Product Type: %s\n",report->product_type);
       printf("Tape Volser: %s\n",report->tape_volser);
       printf("Operation: %s\n",report->operation);
       printf("Power Hrs: %d\n",report->power_hrs);
       printf("Motion Hrs: %d\n",report->motion_hrs);
       printf("Cleaning Bit: %d\n",report->cleaning_bit);
       printf("mb_user_read: %d\n",report->mb_user_read);
       printf("mb_user_write: %d\n",report->mb_user_write);
       printf("mb_dev_read: %d\n",report->mb_dev_read);
       printf("mb_dev_write: %d\n",report->mb_dev_write);
       printf("read errs: %d\n",report->read_errs);
       printf("write errs: %d\n",report->write_errs);
       printf("track retries: %d\n",report->track_retries);
       printf("underrun: %d\n",report->underrun);
    } else {
       break;
    }
    ds_close_list(sd);
   }
}


%}

#define CLEAN 3 
#define REPAIR 4 
#define INSTALL 5 
 
#define DELTA 1 
#define SUM_OF_DELTAS 2 
#define BUMP 4 

/*************************************************************************
 *                                                                       *
 * ds_open_from_ftt_d()                                                  *
 *   Create a drivestat descriptor, given an already=open FTT descriptor *
 *                                                                       *
 *************************************************************************/
DS_DESCRIPTOR *ds_open_from_ftt_d(ftt_descriptor ftt_d);

/*************************************************************************
 *                                                                       *
 * ds_get_stats_buff()                                                   *
 *   Create a drivestat DS_STATS buffer                                  *
 *                                                                       *
 *************************************************************************/
DS_STATS * ds_get_stats_buff(void);

/*************************************************************************
 *                                                                       *
 * ds_print_report()                                                     *
 *   Print every record in the database                                  *
 *                                                                       *
 *************************************************************************/
int ds_print_report(void);

/****************************************************************************
 *                                                                          *
 * ds_init_stats()                                                          *
 *   Make a call to ftt and get the initial statistics to be used to compute*
 *   the deltas.                                                            *
 *                                                                          *
 ****************************************************************************/
int ds_init_stats(DS_DESCRIPTOR *d);

/***************************************************************************
 *                                                                         *
 * ds_print_stats()                                                        *
 *   Print stats in current drivestat descriptor to file.  If file is NULL,*
 *   reports are printed to stdout.  Return 0 on success, 1 on failure.    *
 *                                                                         *
 ***************************************************************************/
%typemap(python, in) char *file {
    if ($source == Py_None)
	 $target = (char *)0;
    else
	 $target = PyString_AsString($source);
}
int ds_print_stats(DS_DESCRIPTOR *d, char *file);

/*************************************************************************
 *                                                                       *
 * ds_set_delta()                                                        *
 *    Set the delta portion of the ds_descriptor with the data passed in *
 *    in the stat_buf.                                                   *
 *                                                                       *
 *************************************************************************/
int ds_set_delta(DS_DESCRIPTOR *d, DS_STATS *stat_buf);

/*************************************************************************
 *                                                                       *
 * ds_set_deltasum()                                                     *
 *    Set the delta sum portion of the ds_descriptor with the data passed*
 *    in the stat_buf.                                                   *
 *                                                                       *
 *************************************************************************/
int ds_set_deltasum(DS_DESCRIPTOR *d, DS_STATS *stat_buf);

/*************************************************************************
 *                                                                       *
 * ds_set_init()                                                         *
 *    Set the init portion of the ds_descriptor with the data passed in  *
 *    in the stat_buf.                                                   *
 *                                                                       *
 *************************************************************************/
int ds_set_init(DS_DESCRIPTOR *d, DS_STATS *stat_buf);

/************************************************************************
 *                                                                      *
 * ds_set_operation()                                                   *
 *   Set the operation field in the drivestat descriptor.               *
 *                                                                      *
 ************************************************************************/
int ds_set_operation(DS_DESCRIPTOR *d,char *s);

/************************************************************************
 *                                                                      *
 * ds_compute_delta                                                     *
 *   Gathers the stats from the tape drive, and computes the delta based*
 *   on the current stats and the stats in the init portion of the      *
 *   descriptor.                                                        *
 *                                                                      *
 ************************************************************************/
int ds_compute_delta(DS_DESCRIPTOR *d);

/************************************************************************
 *                                                                      *
 * ds_prepare_list()                                                    *
 *   Sets up a list to be fetched using the getnext function.  A socket *
 *   is established with a child of the server, and passed back through *
 *   the pointer cursor_sd.  This is used in subsequent getnext()       *
 *   calls.  The number of records that will be retrieved is returned.  *
 *                                                                      *
 ************************************************************************/
int ds_prepare_list(char *drive,char *vendor,char *prod_type,
		    char *host, char *vsn,char *bdate,
		    char *edate,int *n);

/************************************************************************
 *                                                                      *
 * ds_getnext()                                                         *
 *  Fetches the next record from the list prepared by ds_prepare_list().*
 *                                                                      *
 ************************************************************************/
DS_REPORT *ds_getnext(int list_sd);

/************************************************************************
 *                                                                      *
 * ds_close_list()                                                      *
 *    Shuts down the connection to the list processing process of the   *
 *    server.                                                           *
 *                                                                      *
 ************************************************************************/
int ds_close_list(int list_sd);

/************************************************************************
 *                                                                      *
 * i_get_current_drive_stats                                            *
 *   internal routine to get the current drive statistics.              *
 *                                                                      *
 ************************************************************************/
int i_get_current_drive_stats(DS_STATS stats);


/************************************************************************
 *                                                                      *
 * ds_send_stats()                                                      *
 *   Send the stats to the drivestat server.  <timeout> if non-0 will   *
 *   cause the command to block for <timeout> seconds.  <flag> instructs*
 *   what to send:                                                      *
 *      DELTA - Send the delta in the drivestat descriptor.             *
 *      SUM - Send the sum of deltas in the drivestat descriptor.       *
 *                                                                      *
 *   The format of the packet sent will be:                             *
 *     01|<DSN>|<VEN>|<PRD>|<TAPEVS>|<PHRS>|<MHRS>|<CBIT>|<USRREAD>|    *
 *     <USRWR>|<DEVREAD>|<DEVWRITE>|<RDERR>|<WRERR>|<RET>|<UND>|<MTCT>  *
 *     <HOST>|<LOGICAL_DRIVE>^                                          *
 *   Return 0 on success, -1 on error.                                  *
 *                                                                      *
 *   The ds_server port and host should be stored in the users          *
 *   environment (DS_SERVER_PORT, DS_SERVER_HOST).  This is established *
 *   if the user does a ups setup.                                      *
 *                                                                      *
 ************************************************************************/
int ds_send_stats(DS_DESCRIPTOR *d, int timeout, int flag);

/************************************************************************
 *                                                                      *
 * ds_drive_maintenace()                                                *
 *   Perform maintenace on the drive:                                   *
 *     INSTALL                                                          *
 *     REPAIR                                                           *
 *     CLEAN                                                            *
 *                                                                      *
 ************************************************************************/

%typemap(python, in) char *host1 {
    if ($source == Py_None)
	 $target = (char *)0;
    else
	 $target = PyString_AsString($source);
}

%typemap(python, in) char *logical_drive_name1 {
    if ($source == Py_None)
	 $target = (char *)0;
    else
	 $target = PyString_AsString($source);
}

int ds_drive_maintenance(DS_DESCRIPTOR *d,int flag,
			 char *host1,char *logical_drive_name1);

/************************************************************************
 *                                                                      *
 * connect_to_server()                                                  *
 *   connect to drivestat server.                                       *
 *                                                                      *
 ************************************************************************/
int connect_to_server(char *serv_host,char *serv_port);

/************************************************************************
 *                                                                      *
 * send_data()                                                          *
 *   Send the data to the server.  On success, it returns the socket    *
 *   descriptor.                                                        *
 *                                                                      *
 ************************************************************************/
int send_data(char *buf);

/*****************************************************************************
 *                                                                           *
 * _write_sock(sd,buff)                                                      *
 *    Utility routine to write n bytes over a socket.                        *
 *                                                                           *
 *****************************************************************************/
int _write_sock(int sd,char *buf);
