 #! /bin/bash
cd /fnal/ups/prd/www_pages/enstore

html_file=cms_pnfs_ls.html
tmp_file=${html_file}.tmp

rm -f $tmp_file
echo "<HTML><HEAD><TITLE>CMS PNFS DIRECTORY LISTING</TITLE></HEAD>" > $tmp_file
echo "<BODY TEXT=#000066 BGCOLOR=#FFFFFF LINK=#0000EF VLINK=#55188A ALINK=#FF0000 BACKGROUND=enstore.gif>" >> $tmp_file
echo "<BR><BR><FONT SIZE=+4><B>Directory Listing of /pnfs/cms</B></FONT>" >> $tmp_file
echo "<PRE>" >> $tmp_file

ls -l -F -R /pnfs/cms >> $tmp_file

echo "</PRE></BODY></HTML>" >> $tmp_file
mv $tmp_file $html_file

