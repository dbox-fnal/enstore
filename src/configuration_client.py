###############################################################################
# src/$RCSfile$   $Revision$
#
# system imports
import sys
import time
import errno
import pprint

# enstore imports
import generic_client
import interface
import udp_client
import Trace
import callback
import e_errors
import socket

MY_NAME = "CONFIG_CLIENT"
MY_SERVER = "configuration_server"

class ConfigurationClient(generic_client.GenericClient):

    def __init__(self, address):
        self.clear()
	self.print_id = MY_NAME
        self.config_address=address
        self.u = udp_client.UDPClient()

    def send (self, ticket,  rcv_timeout=0, tries=0):
        x = self.u.send( ticket, self.config_address, rcv_timeout, tries )
        return x
    
    # return the address of the configuration server
    def get_address(self):
	return self.config_address

    # get rid of all cached values - go back to server for information
    def clear(self):
        self.cache = {}

    # get value for requested item from server, store locally in own cache
    def get_uncached(self, key, timeout=0, retry=0):
        request = {'work' : 'lookup', 'lookup' : key }
        self.cache[key] = self.send(request, timeout, retry)

        return self.cache[key]

    # return cached (or get from server) value for requested item
    def get(self, key, timeout=0, retry=0):
        if key=='configuration_server':
            ret = {'hostip':self.config_address[0], 'port':self.config_address[1]}
        else:
            ret = self.cache.get(key, None)
        if ret is None:
            ret = self.get_uncached(key, timeout, retry)
        return ret

    # dump the configuration dictionary
    def dump(self, timeout=0, retry=0):
        host, port, listen_socket = callback.get_callback()
        listen_socket.listen(4)
        
        request = {'work' : 'dump',
                   'callback_addr'  : (host,port)
                   }

        x=self.send(request, timeout, retry)
        control_socket, addr = listen_socket.accept()
        d = callback.read_tcp_obj(control_socket)
        return d
        
    # get all keys in the configuration dictionary
    def get_keys(self, timeout=0, retry=0):
        request = {'work' : 'get_keys' }
        keys = self.send(request,  timeout,  retry )
        return keys

    # reload a new  configuration dictionary
    def load(self, configfile, timeout=0, retry=0):
        request = {'work' : 'load' ,  'configfile' : configfile }
        x = self.send(request, timeout, retry)
        return x



    def alive(self, server, rcv_timeout=0, tries=0):
        try:
            x = self.send({'work':'alive'}, rcv_timeout, tries)
        except errno.errorcode[errno.ETIMEDOUT]:
	    x = {'status' : (e_errors.TIMEDOUT, None)}
        return x

    # get list of the Library manager movers
    def get_movers(self, library_manager, timeout=0, retry=0):
        request = {'work' : 'get_movers' ,  'library' : library_manager }
        x = self.send(request, timeout, retry)
        return x

    # get media changer associated with a library manager
    def get_media_changer(self, library_manager, timeout=0, retry=0):
        request = {'work' : 'get_media_changer' ,
                   'library' : library_manager }
        x = self.send(request, timeout, retry)
        return x

	
    #get list of library managers
    def get_library_managers(self, ticket, timeout=0, retry=0):
        request = {'work': 'get_library_managers'}
        x = self.send(request, timeout, retry)
        return x


    #get list of media changers
    def get_media_changers(self, ticket, timeout=0, retry=0):
        request = {'work': 'get_media_changers'}
        x = self.send(request, timeout, retry)
        return x


    # get the configuration dictionary element(s) that contain the specified
    # key, value pair
    def get_dict_entry(self, keyValue, timeout=0, retry=0):
        request = {'work': 'get_dict_element',
                   'keyValue': keyValue }
        x = self.send(request, timeout, retry)
        return x


        
class ConfigurationClientInterface(generic_client.GenericClientInterface):
    def __init__(self, flag=1, opts=[]):
        # fill in the defaults for the possible options
        self.do_parse = flag
        self.restricted_opts = opts
        self.config_file = ""
        self.show = 0
        self.load = 0
        self.alive_rcv_timeout = 0
        self.alive_retries = 0
        self.summary = 0
        generic_client.GenericClientInterface.__init__(self)

        # if we are using the default host and port, warn the user
        interface.check_for_config_defaults()

    # define the command line options that are valid
    def options(self):
        if self.restricted_opts:
            return self.restricted_opts
        else:
            return self.client_options()+[
                "config_file=","summary","show","load"]

def do_work(intf):

    csc = ConfigurationClient((intf.config_host, intf.config_port))
    csc.csc = csc
    result = csc.handle_generic_commands(MY_SERVER, intf)
    if result:
        pass
    elif intf.show:
        result = csc.dump(intf.alive_rcv_timeout,intf.alive_retries)
        pprint.pprint(result['dump'])
        
    elif intf.load:
        result= csc.load(intf.config_file, intf.alive_rcv_timeout,
	                intf.alive_retries)

    elif intf.summary:
        result= csc.get_keys(intf.alive_rcv_timeout,intf.alive_retries)
        pprint.pprint(result['get_keys'])
        
    else:
	intf.print_help()
        sys.exit(0)

    csc.check_ticket(result)

if __name__ == "__main__":
    Trace.init(MY_NAME)

    # fill in interface
    intf = ConfigurationClientInterface()

    do_work(intf)
