#!/usr/bin/env python

import pg
import configuration_client
import option
import time
import sys
import popen2

def day_after(date, offset):
	t = date.split('-')
	return time.strftime("%Y-%m-%d", time.localtime(time.mktime((int(t[0]), int(t[1]), int(t[2])+offset, 0, 0, 0, 0, 0, 0))))

intf = option.Interface()
csc = configuration_client.ConfigurationClient((intf.config_host, intf.config_port))

dbinfo = csc.get('database')
outfile = None
gscript = """
set term postscript landscape enhanced color solid 'Helvetica' 10
set output '%s.ps'
set title "Migration/Duplication per day"
set xlabel 'Date'
set ylabel 'Volumes'
set timefmt '%%Y-%%m-%%d'
set xdata time
set format x '%%m-%%d'
set style line 1 lt 3 lw 25
set xrange ['%s':'%s']
set yrange [:%d]
set mxtics 2
plot '-' using 1:2 t "LTO2" w imp ls 1 
"""

if len(sys.argv) > 1 and sys.argv[1] == '--default-log':
	outfile = csc.get('crons').get('html_dir')+'/tape_inventory/MIGRATION_SUMMARY'
	sys.stdout = open(outfile+'.html', 'w')
	print "<pre>"

db = pg.DB(host=dbinfo['dbhost'], port=dbinfo['dbport'], dbname=dbinfo['dbname'])
print time.ctime(time.time())
print
print "================="
print "Migration per day"
print "================="
print
res = db.query("select date(time) as date, count(distinct src) from migration_history where time > '2008-01-01' group by date order by date;")
print res
if outfile:
	res1 = res.getresult()
	m = 0
	for i in res1:
		if i[1] > m:
			m = i[1]
	(out, gp) = popen2.popen2("gnuplot; convert -rotate 90 %s.ps %s.jpg; convert -rotate 90 -geometry 120x120 -modulate 80 %s.ps %s_stamp.jpg"%(outfile, outfile, outfile, outfile))
	gp.write(gscript%(outfile, day_after(res1[0][0], -1), day_after(res1[-1][0], 1), (m+5)/5*5))
	for i in res1:
		gp.write("%s %d\n"%(i[0], i[1]))
	gp.write("e\n")
	gp.write("quit\n")
	gp.close()

	print '<img src="MIGRATION_SUMMARY.jpg">'
	print '<a href="MIGRATION_SUMMARY.ps">Postscript version</a>'
	print '<p>'

print
print db.query("select count(label) as \"migrated volumes\" from volume where system_inhibit_1 = 'migrated' and not label like 'PRI%';")
print
print db.query("select count(distinct dst) as \"tapes written\" from migration_history where not src like 'PRI%';")

print
print "================="
print "Migration History"
print "================="
print
print db.query("select * from migration_history where not src like 'PRI%' order by time;")
if outfile:
	print "</pre>"
