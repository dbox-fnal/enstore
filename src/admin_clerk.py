###############################################################################
# src/$RCSfile$   $Revision$
#
# system imports
import sys
import os
import time
import pprint
import copy
import traceback

#enstore imports
import SocketServer
import timeofday
import callback
import log_client
import dict_to_a
import configuration_client
import dispatching_worker
import generic_server
import db
import dbutil
import Trace

class AdminClerkMethods(dispatching_worker.DispatchingWorker) :
   def select(self,ticket):
        ticket["status"] = "ok"
        try:
            self.reply_to_caller(ticket)
        # even if there is an error - respond to caller so he can process it
        except:
            ticket["status"] = str(sys.exc_info()[0])+str(sys.exc_info()[1])
            self.reply_to_caller(ticket)
            return
        joint=0
        if ticket['dbname']=="volume,file":
	    dict=dictV
	    joint=1
	elif ticket['dbname']=="volume":
	    dict=dictV
	elif ticket['dbname']=="file"  :
	    dict=dictF
	else :
	   ticket["status"] = "wrong name "+dbname+"for database table"
	   self.reply_to_caller(ticket) 
	   return
        criteria=self.modifyOpt(ticket['criteria'])
        del ticket['criteria']
        self.get_user_sockets(ticket)
        ticket["status"] = "ok"
	callback.write_tcp_socket(self.data_socket,ticket,
                                  "admin_clerk select, controlsocket")
  	if len(criteria)==0 :
     	   key=dict.next()
      	   while key :
		callback.write_tcp_buf(self.data_socket,repr(dict[key]),
                                  "admin_clerk select, datasocket")
	   	key=dict.next()
  	else :
    	   index=None
    	   for k,val in criteria.items():
		if dict.is_index(k):
		   index=k
		   break
    	   if index != None:
      		for main_crt in criteria[index]:
         		for  key in dict.index(index,main_crt) :
			   value=dict[key]
			   if self.find_item(value,criteria,main_crt):
			      if joint:
				for fkey in dictF.index('external_label',\
						value['external_label']):
				   callback.write_tcp_buf(self.data_socket,\
					repr(dictF[fkey]),"admin_clerk select,\
					datasocket")
			      else:
			           callback.write_tcp_buf(self.data_socket,\
				        repr(value),"admin_clerk select,\
					datasocket")
    	   else:
      		key=dict.next()
      		while key :
        		value=dict[key]
			if self.find_item(value,criteria):
			  if joint:
				for fkey in dictF.index('external_label',\
						value['external_label']):
				   callback.write_tcp_buf(self.data_socket,\
					repr(dictF[fkey]),"admin_clerk select,\
					datasocket")
			  else:
			  	callback.write_tcp_buf(self.data_socket,\
					repr(value),"admin_clerk select, \
					datasocket")
			key=dict.next()
        self.data_socket.close()
        callback.write_tcp_socket(self.control_socket,ticket,
                                  "admin_clerk select, controlsocket")
        self.control_socket.close()
	return
   def modifyOpt(self,criteria):
	if criteria.has_key('first_access'):
	   criteria['first_access']=dbutil.timeOper(criteria['first_access'])
	if criteria.has_key('last_access'):
	   criteria['last_access']=dbutil.timeOper(criteria['last_access'])
	if criteria.has_key('declared'):
	   criteria['declared']=dbutil.timeOper(criteria['declared'])
	if criteria.has_key('capacity'):
	   criteria['capacity']=dbutil.numOper(criteria['capacity'])
	if criteria.has_key('rem_bytes'):
	   criteria['rem_bytes']=dbutil.numOper( criteria['rem_bytes'])
	return criteria
   def find_item(self,value,criteria,main_crt=""):
        found=1
	for k,v in  criteria.items():
           if k==main_crt:
		continue
	   operator=1
	   val=value[k]
	   if type(v)==type(dbutil.numOper()):
		operator=0
		if v.numcmp(val):
		   found=0
		   break
           if operator : 		 	
	        if val not in v:
		   found=0
		   break
	return found
    # get a port for the data transfer
    # tell the user I'm your volume clerk and here's your ticket
   def get_user_sockets(self, ticket) :
        admin_clerk_host, admin_clerk_port, listen_socket =\
                           callback.get_callback()
        listen_socket.listen(4)
        ticket["admin_clerk_callback_host"] = admin_clerk_host
        ticket["admin_clerk_callback_port"] = admin_clerk_port
        self.control_socket = callback.user_callback_socket(ticket)
        data_socket, address = listen_socket.accept()
        self.data_socket = data_socket
        listen_socket.close()

class AdminClerk(AdminClerkMethods, generic_server.GenericServer,
                 SocketServer.UDPServer) :
	pass
if __name__=="__main__":
   import getopt 
   import string
   try:
     import SOCKS; socket = SOCKS
   except ImportError:
     import socket
   Trace.init("AdminClerk")
   Trace.trace(1,"Admin Clerk called with args "+repr(sys.argv))

   # defaults
   (config_host,ca,ci) = socket.gethostbyaddr(socket.gethostname())
   config_port = "7500"
   config_list = 0
   # see what the user has specified. bomb out if wrong options specified
   options = ["config_host=","config_port=","config_list","help"]
   optlist,args=getopt.getopt(sys.argv[1:],'',options)
   for (opt,value) in optlist :
        if opt == "--config_host" :
            config_host = value
        elif opt == "--config_port" :
            config_port = value
        elif opt == "--config_list" :
            config_list = 1
        elif opt == "--help" :
            print "python ",sys.argv[0], options
            print "   do not forget the '--' in front of each option"
            sys.exit(0)

   # bomb out if can't translate host
   ip = socket.gethostbyname(config_host)

   # bomb out if port isn't numeric
   config_port = string.atoi(config_port)

   csc = configuration_client.ConfigurationClient(config_host,config_port,config_list)  
   keys = csc.get("admin_clerk")
   ac =  AdminClerk((keys['hostip'], keys['port']), AdminClerkMethods)
   ac.set_csc(csc)
   logc = log_client.LoggerClient(csc, "", 'logserver', 0)
   ac.set_logc(logc)
   indlst=['media_type','file_family','library']
   dictV = db.DbTable("volume",logc,indlst) 
   indlst=['external_label']
   dictF = db.DbTable("file",logc,indlst)
   while 1:
        try:
            Trace.trace(1,"Admin Clerk (re)starting")
            logc.send(log_client.INFO, 1, "Admin Clerk (re)starting")
            ac.serve_forever()
        except:
            traceback.print_exc()
            format = timeofday.tod()+" "+\
                     str(sys.argv)+" "+\
                     str(sys.exc_info()[0])+" "+\
                     str(sys.exc_info()[1])+" "+\
                     "admin clerk serve_forever continuing"
            print format
            logc.send(log_client.ERROR, 1, format)
            Trace.trace(0,"Admin Clerk error"+format)
            continue

   Trace.trace(1,"Admin Clerk finished") # impossible
