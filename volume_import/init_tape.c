/* $Id$
Utility to initialize tapes before use
Labels tape and  creates database branch for new volume
*/

#include "volume_import.h"

void Usage()
{
    fprintf(stderr,
   "Usage: %s [-v] [-E] [-f tape-device] [-d tape-db] vol_label\n\
    tape-device can be set using environment variable $TAPE\n\
    tape-db (db directory) can be set using environment variable $TAPE_DB\n", 
	    progname);
    
    exit(-1);
}

int clear_db_volume();
    
int
main(int argc, char **argv)
{
    int i;
    int erase=0;

    char label[80];
    char *cp;

    tape_device = getenv("TAPE");
    tape_db = getenv("TAPE_DB");

    progname = argv[0];

    for (i=1; i<argc; ++i) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {

	    case 'E':
		erase = 1;
		break;
	    case 'f':
	    case 't':
		if (++i >= argc) {
		    fprintf(stderr, "%s: -f option requres an argument\n", 
			    progname);
		    Usage();
		} else 
		    tape_device = argv[i];
		break;
	    case 'd':
		if (++i >= argc) {
		    fprintf(stderr, "%s: -d option requres an argument\n", 
			    progname);
		    Usage();
		} else 
		    tape_db = argv[i];
		break;
	    case 'v':
		verbose = 1;
		break;
	    default:
		fprintf(stderr,"%s: unknown option %s\n", progname, argv[i]);
		Usage();
	    }
	} else
	    break;
    }
    
    if (i==argc) {
	fprintf(stderr,"%s: no volume label specified\n", progname);
	Usage();
    } else {
	volume_label = argv[i++];
    }

    if (!tape_device) {
	fprintf(stderr, "%s: no tape device specified\n", progname);
	Usage();
    }

    if (!tape_db) {
	fprintf(stderr, "%s: no tape db specified\n", progname);
	Usage();
    }

    if (erase)
	clear_db_volume();
	    
    if (verify_tape_device()
	||verify_tape_db(1)
	||verify_db_volume(1))
	{
	    fprintf(stderr, "%s failed\n", progname); 
	    return -1; /* don't clear_db_volume here because the error
			  may be that db volume already exists*/
	}

    if (open_tape() 
	||rewind_tape())
	goto cleanup;
    
    /* check if it's labeled */
    if (!erase){
	verbage("Checking for existing label\n");
	if (read_tape(label,80)==80){
	    label[79] = 0;
	    verbage("Got %s\n", label);
	    if (!strncmp(label,"VOL1",4)){
		for (cp=label+4; *cp && *cp!=' '; ++cp)
		    ;
		*cp = 0;
		fprintf(stderr,"This tape is already labeled\nLabel=%s\n", label+4);
		fprintf(stderr,"Use the -e (erase) option to relabel it\n");
		goto cleanup;
	    }
	} else {
	    verbage("Couldn't read 80 bytes\n");
	}
	if (rewind_tape())
	    goto cleanup;
    }
		
    if (write_vol1_header()
	||write_eof_marks(2)
	||write_eot1_header(0)
	||backward_record(1)
	||close_tape())
	goto cleanup;
    /* all is well */
    return 0;

  cleanup:
    clear_db_volume();
    fprintf(stderr, "%s failed\n", progname);
    return -1;
}

int clear_db_volume(){
    char cmd[MAX_PATH_LEN + 8];
    sprintf(cmd, "/bin/rm -rf %s/volumes/%s", tape_db, volume_label);
    verbage(cmd);
    return system(cmd); /* XXX I was lazy when I wrote this, there must be a nicer way */
}




