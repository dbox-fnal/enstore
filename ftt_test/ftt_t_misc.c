static char rcsid[] = "#(@)$Id$";
/*


Authors:        Margaret Votava
e-mail:         "votava@fnal.gov"
 
Revision history:-
=================
17-Oct-1995 MEV created	Wrapping routine for test commands 
 
Include files:-
===============
*/

#include <time.h>
#include "ftt.h"
#include "ftt_t_parse.h"
#include "ftt_t_macros.h"


/* ============================================================================

ROUTINE:
	ftt_t_date
 	
	print date to stderr
==============================================================================*/
int	ftt_t_date		(int argc, char **argv)
{

time_t	stime;			/* time */
int 		status;
ftt_t_argt	argt[] = {{NULL, FTT_T_ARGV_END, NULL, NULL}};

/* parse command line
   ------------------ */

status = ftt_t_parse (&argc, argv, argt);
FTT_T_CHECK_PARSE(status,argt,argv[0]);

/* echo date
   --------- */

stime = time(NULL);					/* display time */
fprintf (stderr, asctime(localtime(&stime)));
return 0;						/* success */
}


/* ============================================================================

ROUTINE:
	ftt_t_echo
 	
	print string to stderr
==============================================================================*/
int	ftt_t_echo		(int argc, char **argv)
{
int 		status;				/* status */
static char	*mystring;			/* string */
ftt_t_argt	argt[] = {
	{"<mystring>",	FTT_T_ARGV_STRING,	NULL,		&mystring},
	{NULL,		FTT_T_ARGV_END,		NULL,		NULL}};

/* parse command line
   ------------------ */

mystring = NULL;
status = ftt_t_parse (&argc, argv, argt);
FTT_T_CHECK_PARSE (status, argt, argv[0]);

/* echo string to stderr
   --------------------- */

fprintf (stderr,"%s\n",mystring);
return 0;
}

/* ============================================================================

ROUTINE:
	ftt_t_debug_level
 	
	set/display the debug level

==============================================================================*/
int	ftt_t_debug_level	(int argc, char **argv)
{
int 		status;			/* status */
static int	testflag;		/* test flag */
static int	level;			/* string */
ftt_t_argt	argt[] = {
	{"[level]",	FTT_T_ARGV_INT,		NULL,		&level},
	{"-test",	FTT_T_ARGV_CONSTANT,	(char *)TRUE,	&testflag},
	{NULL,		FTT_T_ARGV_END,		NULL,		NULL}};

/* parse command line
   ------------------ */

level = -1; testflag = FALSE;
status = ftt_t_parse (&argc, argv, argt);
FTT_T_CHECK_PARSE (status, argt, argv[0]);

/* either set or show debug level
   ------------------------------ */

if (level == -1)					/* show current level */
   {
   fprintf (stderr,"Current ftt debug level is: %d\n",ftt_debug);
   fprintf (stderr,"Current test debug level is: %d\n",ftt_t_debug);
   }
else							/* change level */
   {
   if (testflag)
      ftt_t_debug = level;
   else
      ftt_debug = level;
   }

return 0;
}
/* ============================================================================

ROUTINE:
	ftt_t_max_errors
 	
	set/display the maximum number of errors before exit

==============================================================================*/
int	ftt_t_max_errors	(int argc, char **argv)
{
int 		status;			/* status */
static int	maxerror;	
ftt_t_argt	argt[] = {
	{"[maxerror]",	FTT_T_ARGV_INT,		NULL,		&maxerror},
	{NULL,		FTT_T_ARGV_END,		NULL,		NULL}};

/* parse command line
   ------------------ */

maxerror = -1; 
status = ftt_t_parse (&argc, argv, argt);
FTT_T_CHECK_PARSE (status, argt, argv[0]);

/* either set or show debug level
   ------------------------------ */

if (maxerror == -1)					/* show */
   fprintf(stderr,"Current maximum number of errors is : %d\n",ftt_t_max_error);
else							/* change */
   ftt_t_max_error = maxerror;

return 0;
}
