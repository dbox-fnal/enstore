import time
import errno
from configuration_client import configuration_client
import callback
from udp_client import UDPClient, TRANSFER_MAX

class LibraryManagerClient :
    def __init__(self, configuration_client, library_name) :
        # we always need to be talking to our configuration server
        self.u = UDPClient()
        self.csc = configuration_client
        self.name = library_name


    def send (self, ticket) :
        # who's our library manger that we should send the ticket to?
        lticket = self.csc.get(self.name)
        # send user ticket and return answer back
        return self.u.send(ticket, (lticket['host'], lticket['port']) )


    def write_to_hsm(self, ticket) :
        return self.send(ticket)


    def read_from_hsm(self, ticket) :
        return self.send(ticket)


    def getwork(self,list) :
        # get a port to talk on and listen for connections
        host, port, listen_socket = callback.get_callback()
        listen_socket.listen(4)
        ticket = {"work"               : "getwork",
                  "user_callback_port" : port,
                  "user_callback_host" : host,
                  "unique_id"          : time.time() }
        # send the work ticket to the library manager
        ticket = self.send(ticket)
        if ticket['status'] != "ok" :
            raise errno.errorcode[errno.EPROTO],"lmc.getwork: sending ticket"\
                  +repr(ticket)

        # We have placed our request in the system and now we have to wait.
        # All we  need to do is wait for the system to call us back,
        # and make sure that is it calling _us_ back, and not some sort of old
        # call-back to this very same port. It is dicey to time out, as it
        # is probably legitimate to wait for hours....
        while 1 :
            control_socket, address = listen_socket.accept()
            new_ticket = callback.read_tcp_socket(control_socket, "library "+\
                                         "manager getwork,  mover call back")
            if ticket["unique_id"] == new_ticket["unique_id"] :
                listen_socket.close()
                break
            else:
                print ("lmc.getwork: imposter called us back, trying again")
                control_socket.close()
        ticket = new_ticket
        if ticket["status"] != "ok" :
            raise errno.errorcode[errno.EPROTO],"lmc.getwork: "\
                  +"1st (pre-work-read) library manager callback on socket "\
                  +repr(address)+", failed to setup transfer: "\
                  +"ticket[\"status\"]="+ticket["status"]

        # If the system has called us back with our own  unique id, call back
        # the library manager on the library manager's port and read the
        # work queues on that port.
        data_path_socket = callback.library_manager_callback_socket(ticket)
        worklist = callback.read_tcp_socket(data_path_socket,"library "+\
                                    "manager getwork, reading worklist")
        data_path_socket.close()

        # Work has been read - wait for final dialog with library manager.
        done_ticket = callback.read_tcp_socket(control_socket, "library "+\
                                      "manager getwork, mover final dialog")
        control_socket.close()
        if done_ticket["status"] != "ok" :
            raise errno.errorcode[errno.EPROTO],"lmc.getwork: "\
                  +"2nd (post-work-read) library manger callback on socket "\
                  +repr(address)+", failed to transfer: "\
                  +"ticket[\"status\"]="+ticket["status"]
        return worklist

    # check on alive status
    def alive(self):
        return self.send({'work':'alive'})

if __name__ == "__main__" :
    import sys
    import getopt
    import pprint
    import string
    # Import SOCKS module if it exists, else standard socket module socket
    try:
        import SOCKS; socket = SOCKS
    except ImportError:
        import socket

    # defaults
    #config_host = "localhost"
    (config_host,ca,ci) = socket.gethostbyaddr(socket.gethostname())
    config_port = "7500"
    config_list = 0
    list = 0
    getwork = 0
    alive = 0

    # see what the user has specified. bomb out if wrong options specified
    options = ["config_host=","config_port=","config_list",
               "getwork","list","verbose","alive","help"]
    optlist,args=getopt.getopt(sys.argv[1:],'',options)
    for (opt,value) in optlist :
        if opt == "--config_host" :
            config_host = value
        elif opt == "--config_port" :
            config_port = value
        elif opt == "--config_list" :
            config_list = 1
        elif opt == "--getwork" :
            getwork = 1
        elif opt == "--alive" :
            alive = 1
        elif opt == "--list" or opt == "--verbose" :
            list = 1
        elif opt == "--help" :
            print "python ",sys.argv[0], options, "library"
            print "   do not forget the '--' in front of each option"
            sys.exit(0)

    # bomb out if can't translate host
    ip = socket.gethostbyname(config_host)

    # bomb out if port isn't numeric
    config_port = string.atoi(config_port)

    # bomb out if we don't have a library
    if len(args) < 1 :
        print "python ",sys.argv[0], options, "library"
        print "   do not forget the '--' in front of each option"
        sys.exit(1)

    if config_list :
        print "Connecting to configuration server at ",config_host,config_port
    csc = configuration_client(config_host,config_port)

    lmc = LibraryManagerClient(csc,args[0])

    if alive:
        ticket = lmc.alive()

    elif  getwork:
        ticket = lmc.getwork(list)


    if ticket['status'] == 'ok' :
        if list:
            pprint.pprint(ticket)
        sys.exit(0)

    else :
        print "BAD STATUS:",ticket['status']
        pprint.pprint(ticket)
        sys.exit(1)
