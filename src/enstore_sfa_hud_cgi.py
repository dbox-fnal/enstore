#!/usr/bin/env python

import os
import sys
import string
import time
import enstore_functions2
import configuration_client
import e_errors
import file_utils
import statvfs
from   DBUtils import PooledDB
import  psycopg2
import psycopg2.extras


KB=1<<10

# Obtain the correct values for ENSTORE_CONFIG_HOST and ENSTORE_CONFIG_PORT
# if they are not already available.
def get_environment():
    #This is a very messy way to determine the host and port of the
    # configuration server.  But at least it is in only one place now.
    if os.environ.get('ENSTORE_CONFIG_HOST', None) == None:
        cmd = ". /usr/local/etc/setups.sh; setup enstore; echo $ENSTORE_CONFIG_HOST; echo $ENSTORE_CONFIG_PORT"
        pfile = os.popen(cmd)
        config_host = pfile.readline().strip()
        if config_host:
            os.environ['ENSTORE_CONFIG_HOST'] = config_host
        else:
            print "Unable to determine ENSTORE_CONFIG_HOST."
            sys.exit(1)
        config_port = pfile.readline().strip()
        if config_port:
            os.environ['ENSTORE_CONFIG_PORT'] = config_port
        else:
            print "Unable to determine ENSTORE_CONFIG_PORT."
            sys.exit(1)
        pfile.close()

def select(value,query):
    connectionPool = None
    cursor = None
    db     = None
    try:
        connectionPool = PooledDB.PooledDB(psycopg2,
                                           maxconnections=1,
                                           blocking=True,
                                           host=value.get('dbhost','localhost'),
                                           port=value.get('dbport',5432),
                                           user=value.get('dbuser',"enstore"),
                                           database=value.get('dbname',"enstoredb"))
        db = connectionPool.connection()
        cursor = db.cursor(cursor_factory=psycopg2.extras.RealDictCursor)
        cursor.execute(query)
        res=cursor.fetchall()
        return res
    except:
        exc_type, exc_value = sys.exc_info()[:2]
        sys.stderr.write(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time()))+" : " + str(exc_type)+' '+str(exc_value)+'\n')
        sys.stderr.flush()
        raise
    finally:
        for item in [cursor, db, connectionPool]:
            if item :
                item.close()

def parse_mtab(volumes):
    volume_map={}
    for mtab_file in ["/etc/mtab", "/etc/mnttab"]:
        try:
            fp = file_utils.open(mtab_file, "r")
            try:
                for line in fp:
                    if not line :
                        continue
                    parts=line.split()
                    mps=parts[0].split(":")
                    if len(mps)<2:
                        continue
                    mp=mps[1]
                    for v in volumes:
                        if mp == v :
                            volume_map[v] = parts[1]
            finally:
               fp.close()
            break
        except (KeyboardInterrupt, SystemExit):
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
        except (OSError, IOError), msg:
            if msg.args[0] in [errno.ENOENT]:
                continue
            else:
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
    return volume_map

def print_html(summary):
    print "Content-type: text/html"
    print
    print '<html>'
    print '<head>'
    print '<title> Enstore SFA HUD </title>'
    print '</head>'
    print '<body bgcolor="#ffffd0">'
    print '<hr>'

    print "<center>"
    print '<font size=7 color="#ff0000"> Enstore SFA HUD</font>'
    print '<hr>'

    print "<table border=\"1\">"
    print "<tr><th>Volume</th><th>KB in transition</th><th># Files in Transition</th><th>Used Size(KB)</th><th>Total Size(KB)</th></tr>"
    print "<tr><td>data_area(write cache)</td><td>"+str(summary["data_area"]["size"])+\
          "</td><td>"+str(summary["data_area"]["files"])+\
          "</td><td>"+str(summary["data_area"]["used"])+\
          "</td><td>"+str(summary["data_area"]["total"])+"</td></tr>"
    print "</table>"


    print "<table border=\"1\">"
    print "<tr><th>Volume</th><th>Used Size(KB)</th><th>Total Size(KB)</th></tr>"
    print "<tr><td>archive_area(package staging)</td>" \
          "<td>"+str(summary["archive_area"]["used"])+\
          "</td><td>"+str(summary["archive_area"]["total"])+"</td></tr>"
    print "</table>"


    print "<table border=\"1\">"
    print "<tr><th>Volume</th><th># Files</th><th>Used Size(KB)</th><th>Total Size(KB)</th></tr>"
    print "<tr><td>stage_area(read cache)</td>"+\
          "<td>"+str(summary["stage_area"]["files"])+\
          "</td><td>"+str(summary["stage_area"]["used"])+\
          "</td><td>"+str(summary["stage_area"]["total"])+"</td></tr>"
    print "</table>"

    print '<hr>'

    print "<a href='enstore_sfa_files_in_transition_cgi.py'> FILES IN TRANSITION </a>"

    print '<hr>'

    print "</center>"
    print '</body>'
    print '</html>'


def main():
    # Obtain the correct values for ENSTORE_CONFIG_HOST and ENSTORE_CONFIG_PORT
    # if they are not already available.
    get_environment()

    #Get the configuration server client.
    config_host = enstore_functions2.default_host()
    config_port = enstore_functions2.default_port()
    csc = configuration_client.ConfigurationClient((config_host, config_port))



    # in the beginning ...


    migrator_list   = csc.get_migrators2()

    areas = {}
    summary = {}

    for m in migrator_list:
        for k in ("stage_area",
                  "data_area",
                  "archive_area"):
            areas[k] = m[k]

    volume_map=parse_mtab(areas.values())
    for key,value in volume_map.iteritems():
        tag=""
        for k,v in areas.iteritems():
            if v==key:
                tag=k
                break
        stats  = os.statvfs(value)
        avail  = long(stats[statvfs.F_BAVAIL])*stats[statvfs.F_BSIZE] / KB
        total  = long(stats[statvfs.F_BLOCKS])*stats[statvfs.F_BSIZE] / KB
        summary[tag] = { 'used' : total - avail,
                         'total' : total }

    dbinfo = csc.get("database")

    #
    # files in trasition
    #
    res=select(dbinfo,"select coalesce(sum(size)/1024.,0) as total,count(*) from file where bfid in (select bfid from files_in_transition)")
    summary["data_area"]["files"]=int(res[0]["count"])
    summary["data_area"]["size"]=long(res[0]["total"])


    #
    #  read cache ( stage_area)
    #
    res=select(dbinfo,"select coalesce(sum(size)/1024.,0) as total,count(*) from file where cache_status='CACHED' and location_cookie!=cache_location")
    summary["stage_area"]["files"]=int(res[0]["count"])
    summary["stage_area"]["size"]=long(res[0]["total"])

    print_html(summary)


if __name__ == '__main__':

    main()