from configuration_client import configuration_client
from udp_client import UDPClient
from db import do_backup
class FileClerkClient :

    def __init__(self, configuration_client) :
        # we always need to be talking to our configuration server
        self.csc = configuration_client
        self.u = UDPClient()

    def send (self, ticket) :
        # who's our file clerk server that we should send the ticket to?
        vticket = self.csc.get("file_clerk")
        # send user ticket and return answer back
        return self.u.send(ticket, (vticket['host'], vticket['port']) )

    def read_from_hsm(self, ticket) :
        return self.send(ticket)

    def new_bit_file(self, bof_space_cookie \
                         , external_label \
                         , sanity_cookie \
                         , complete_crc ) :
        return self.send({"work"             : "new_bit_file", \
                          "bof_space_cookie" : bof_space_cookie, \
                          "external_label"   : external_label, \
                          "sanity_cookie"    : sanity_cookie, \
                          "complete_crc"     : complete_crc })

    def get_bfids(self):
        return self.send({"work" : "get_bfids"} )

    def bfid_info(self,bfid):
        return self.send({"work" : "bfid_info",\
                          "bfid" : bfid } )

    # check on alive status
    def alive(self):
        return self.send({'work':'alive'})
    def start_backup(self):
    	return self.send({'work':'start_backup'})
    def stop_backup(self):
    	return self.send({'work':'stop_backup'})	


if __name__ == "__main__" :
    import sys
    import getopt
    import string
    import pprint
    # Import SOCKS module if it exists, else standard socket module socket
    # This is a python module that works just like the socket module, but uses
    # the SOCKS protocol to make connections through a firewall machine.
    # See http://www.w3.org/People/Connolly/support/socksForPython.html or
    # goto www.python.org and search for "import SOCKS"
    try:
        import SOCKS; socket = SOCKS
    except ImportError:
        import socket

    # defaults
    #config_host = "localhost"
    (config_host,ca,ci) = socket.gethostbyaddr(socket.gethostname())
    config_port = "7500"
    config_list = 0
    bfid = 0
    bfids = 0
    list = 0
    backup=0	
    alive = 0

    # see what the user has specified. bomb out if wrong options specified
    options = ["config_host=","config_port=","config_list",
               "bfids","bfid=","list","verbose","alive","backup","help"]
    optlist,args=getopt.getopt(sys.argv[1:],'',options)
    for (opt,value) in optlist :
        if opt == "--config_host" :
            config_host = value
        elif opt == "--config_port" :
            config_port = value
        elif opt == "--config_list" :
            config_list = 1
        elif opt == "--bfids" :
            bfids = 1
        elif opt == "--bfid" :
            bfid = value
        elif opt == "--alive" :
            alive = 1
        elif opt == "--list" or opt == "--verbose":
            list = 1
	elif opt == "--backup":
	    backup = 1
        elif opt == "--help" :
            print "python ",sys.argv[0], options
            print "   do not forget the '--' in front of each option"
            sys.exit(0)

    # bomb out if can't translate host
    ip = socket.gethostbyname(config_host)

    # bomb out if port isn't numeric
    config_port = string.atoi(config_port)

    if config_list :
        print "Connecting to configuration server at ",config_host,config_port
    csc = configuration_client(config_host,config_port)

    fcc = FileClerkClient(csc)

    if alive:
        ticket = fcc.alive()
    elif backup:
	ticket = fcc.start_backup()
	do_backup("file")
	ticket = fcc.stop_backup()
    elif bfids :
        ticket = fcc.get_bfids()

    elif bfid :
        ticket = fcc.bfid_info(bfid)

    if ticket['status'] == 'ok' :
        if list:
            pprint.pprint(ticket)
        sys.exit(0)

    else :
        print "BAD STATUS:",ticket['status']
        pprint.pprint(ticket)
        sys.exit(1)
