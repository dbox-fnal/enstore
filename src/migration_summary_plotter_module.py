#!/usr/bin/env python
###############################################################################
#
# $Id$
#
###############################################################################

# system imports
import pg
import os
import sys
import time

# enstore imports
import enstore_plotter_module
import enstore_constants

WEB_SUB_DIRECTORY = enstore_constants.MIGRATION_SUMMARY_PLOTS_SUBDIR

"""
In the enstore DB to create the necessary view:
CREATE VIEW remaining_blanks AS
enstoredb-#     SELECT volume.media_type, count(*) AS blanks FROM volume WHERE ((((volume.storage_group)::text = 'none'::text) AND ((volume.file_family)::text = 'none'::text)) AND ((volume.wrapper)::text = 'none'::text)) GROUP BY volume.media_type;
"""

SQL_COMMAND = \
"""
select s2.d2, volume.media_type, s2.closed, s3.started 
from volume

right join
(select date(time) as d2, v2.media_type, count(distinct mh2.src) as closed
	from volume as v2,migration_history as mh2
	where v2.label = mh2.src and v2.library not like '%shelf%'
	group by d2,media_type)
as s2 on s2.media_type = volume.media_type

right join
(select date(swapped) as d3,v3.media_type,count(distinct v3.label) as started
	     from migration as m3,volume as v3,file as f3
	     where f3.bfid = m3.src_bfid and f3.volume = v3.id and
                   v3.library not like '%shelf%'
	     group by d3,media_type)
as s3 on s3.media_type = volume.media_type

where s2.d2 = s3.d3
   and s2.media_type = s3.media_type
   and volume.label not like '%.deleted'
   and volume.library not like '%shelf%'
group by s2.d2,volume.media_type,s2.closed,s3.started;

"""

ACCUMULATED = "accumulated"
DAILY = "daily"

class MigrationSummaryPlotterModule(enstore_plotter_module.EnstorePlotterModule):
    def __init__(self,name,isActive=True):
        enstore_plotter_module.EnstorePlotterModule.__init__(self,name,isActive)


    #Write out the file that gnuplot will use to plot the data.
    # plot_filename = The file that will be read in by gnuplot containing
    #                 the gnuplot commands.
    # data_filename = The data file that will be read in by gnuplot
    #                 containing the data to be plotted.
    # ps_filename = The postscript file that will be created by gnuplot.
    # key = The key to access information in the various instance member
    #       variables.  Here it is the media_type.
    def write_plot_file(self, plot_filename, data_filename, ps_filename,
                        key, plot_type, columns):
        plot_fp = open(plot_filename, "w+")

        plot_fp.write('set terminal postscript color solid\n')
        plot_fp.write('set output "%s"\n' % (ps_filename,))
        plot_fp.write('set title "Migration summary %s for %s" ' \
                      'font "TimesRomanBold,16"\n' % \
                      (plot_type, key,))
        plot_fp.write('set ylabel "Volumes Done"\n')
        plot_fp.write('set xdata time\n')
        plot_fp.write('set timefmt "%Y-%m-%d"\n')
        plot_fp.write('set format x "%Y-%m-%d"\n')
        plot_fp.write('set grid\n')
        plot_fp.write('set nokey\n')
        plot_fp.write('set label "Plotted %s " at graph .99,0 rotate font "Helvetica,10"\n' % (time.ctime(),))
        plot_fp.write('set yrange[ 0 : ]\n')
        
        #First plot is the accumulated volumes.
        plot_line = "plot "
        for column in columns:
            if plot_line != "plot ":
                #If we are on the first plot, don't append the comma.
                plot_line = "%s, " % (plot_line,)
            plot_line = plot_line + '"%s" using 1:%d with impulses lw 10' % (data_filename, column)
        plot_line = "%s\n" % (plot_line,)
        
        #plot_fp.write('plot "%s" using 1:%d with impulses linewidth 10' % (data_filename, column))
        plot_fp.write(plot_line)
        
        plot_fp.close()

        os.system("cat %s" % (data_filename,))

    #######################################################################
    # The following functions must be defined by all plotting modueles.
    #######################################################################

    def book(self, frame):
        #Get cron directory information.
        cron_dict = frame.get_configuration_client().get("crons", {})

        #Pull out just the information we want.
        self.temp_dir = cron_dict.get("tmp_dir", "/tmp")
        html_dir = cron_dict.get("html_dir", "")

        #Handle the case were we don't know where to put the output.
        if not html_dir:
            sys.stderr.write("Unable to determine html_dir.\n")
            sys.exit(1)
        self.web_dir = os.path.join(html_dir, WEB_SUB_DIRECTORY)
        if not os.path.exists(self.web_dir):
            os.makedirs(self.web_dir)

    def fill(self, frame):

        #  here we create data points 

        edb = frame.get_configuration_client().get("database", {})
        db = pg.DB(host   = edb.get('dbhost', "localhost"),
                   dbname = edb.get('dbname', "enstoredb"),
                   port   = edb.get('dbport', 5432),
                   user   = edb.get('dbuser', "enstore")
                   )

        #This query is for volumes that are all done.
        res = db.query(SQL_COMMAND).getresult()

        #Open the datafiles.
        self.pts_files = {}
        self.summary_done = {}
        self.summary_started = {}
        for row in res:
            #row[0] is the date (YYYY-mm-dd)
            #row[1] is the media type
            #row[2] is the number of migrated tapes completed
            #row[3] is the number of migrated tapes attempted
            if not self.pts_files.get(row[1], None):
                fname = os.path.join(self.temp_dir,
                                     "migration_summary_%s.pts" % (row[1],))
                self.pts_files[row[1]] = open(fname, "w")
                self.summary_done[row[1]] = 0L #Set the key values to zeros.
                self.summary_started[row[1]] = 0L #Set the key values to zeros.

        #Output to temporary files the data that gnuplot needs to plot.
        for row in res:
            self.summary_done[row[1]] = self.summary_done[row[1]] + row[2]
            self.summary_started[row[1]] = \
                                         self.summary_started[row[1]] + row[3]
            self.pts_files[row[1]].write("%s %s %s %s %s\n" % (
                row[0], row[2], row[3], self.summary_done[row[1]],
                self.summary_started[row[1]]))

        #Avoid resource leaks.
        for key in self.pts_files.keys():
            self.pts_files[key].close()

    def plot(self):
        for key in self.pts_files.keys():
            #Key at this point is unique media types (that have been migrated).

            
            self._plot(key, ACCUMULATED)
            self._plot(key, DAILY)

            #Cleanup the temporary files for this loop.
            try:
                os.remove(self.pts_files[key].name)
            except:
                pass

    #Plot type is either ACCUMULATED or DAILY.
    def _plot(self, key, plot_type):

            #Get some filenames for the various files that get created.
            ps_filename = os.path.join(self.web_dir,
                      "migration_summary_%s_%s.ps" % (plot_type, key,))
            jpeg_filename = os.path.join(self.web_dir,
                      "migration_summary_%s_%s.jpg" % (plot_type, key,))
            jpeg_filename_stamp = os.path.join(self.web_dir,
                      "migration_summary_%s_%s_stamp.jpg" % (plot_type, key,))
            plot_filename = os.path.join(self.temp_dir,
                      "migration_summary_%s_%s.plot" % (plot_type, key,))

            if plot_type == ACCUMULATED:
                columns = [4, 5]
            elif plot_type == DAILY:
                columns = [2, 3]
            else:
                columns = [0]  #What happens when we get here?
            
            #Write the gnuplot command file(s).
            self.write_plot_file(plot_filename, self.pts_files[key].name,
                                 ps_filename, key, plot_type, columns)

            #Make the plot and convert it to jpg.
            os.system("gnuplot < %s" % plot_filename)
            os.system("convert -rotate 90  %s %s\n"
                      % (ps_filename, jpeg_filename))
            os.system("convert -rotate 90 -geometry 120x120 -modulate 80 %s %s\n"
                      % (ps_filename, jpeg_filename_stamp))

            #Cleanup the temporary files for this loop.
            try:
                os.remove(plot_filename)
            except:
                pass
