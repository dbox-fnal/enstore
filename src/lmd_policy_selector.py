#!/usr/bin/env python

###############################################################################
# $Id$
# Library Manager Directior policy selector. 
# This module is based on the approach in discipline.py
# Lots of coding was copied from there
# The purpose of this module is to return a library manager name based
# on a search in the policy described in some python dictionary
###############################################################################
# system imports
import os
import re
import copy
import string
import types
# enstore imports
import e_errors
import Trace
import dict_u2a

# Style of policy entry
# Dictinary of rules and resulting library managers
# Each value can be a regular expression
#'LTO3.library_manager':{1: {'rule': {'storage_group': 'G1',
#                                          'file_family': 'F1',
#                                          'wrapper':'cpio_odc'
#                                     }
#                            'minimal_file_size': 2000000000L
#                                          }
#                            'resulting_library': 'new_library'
#                            }
#                        2: .....
#
#                         }   
                            
class Selector:

    # Read policy file
    # @return - (e_errors_OK, None) or (e_errors.ERROR, "description)
    def read_config(self):
        Trace.log(e_errors.INFO, "(Re)loading LMD Policy")
        f = open(self.policy_file,'r')
        code = string.join(f.readlines(),'')

        # Lint hack, otherwise lint can't see where configdict is defined.
        policydict = {}
        del policydict 
        policydict = {}

        # do not use try: except here, the exception will be processed in
        # the caller
        exec(code)
        # ok, we read entire file - now set it to real dictionary
        if  hasattr(self, "policydict"):
            del(self.policydict)
        self.policydict = policydict

        # tickets to lmd_policy engine come via qpid
        # so, all strings are in UTF-8
    
    def __init__(self, policy_file):
        self.policy_file = policy_file
        # do not process exception here
        # it will be processed by the caller
        self.read_config()
        


    # match value from key, value pair with valule from conf_key, value pair
    # @param - ticket providing key, value pair
    # @param - policy entry in the the policy configuration
    # @param - key to match
    def ticket_match(self, ticket, policy, key):
        try:
            pattern = "^%s" % (policy[key])
            item='%s'%(ticket.get(key, 'Unknown'),)
            Trace.trace(10, "pattern %s item %s"%(pattern, item))
            return re.search(pattern, item)
        except Exception, detail :
            Trace.log(e_errors.ERROR,"parse errorr: %s"%(detail,))
            return False


    def make_flat_ticket(self, ticket):
        flat_ticket=copy.deepcopy(ticket)
        if ticket.has_key('wrapper'):
            # ticket has a wrapper dictionary in vc subditionary
            # so, copy this one separately.
            for key in flat_ticket['wrapper'].keys():
                 flat_ticket[key] = flat_ticket['wrapper'][key]
            del(flat_ticket['wrapper'])
        for key in flat_ticket.keys():
            if type(flat_ticket[key]) is type({}):
                for k in flat_ticket[key].keys():
                    if k == 'machine': flat_ticket['host'] = flat_ticket[key][k][1]
                    else: flat_ticket[k] = flat_ticket[key][k]
                del(flat_ticket[key])

        # tickets to lmd_policy engine may come via qpid
        # so, all strings are in UTF-8
        # convert them into regular strings
        flat_ticket = dict_u2a.convert_dict_u2a(flat_ticket)
        return flat_ticket

    # @param - ticket to match
    # @return - (True/False, Library/None)
    def match_found(self, ticket):
        # returns a tuple:
        # True if match was found, False - if not
        # library manager name, None othrewise
        
        # default match settings
        match = False, None
        failed = False

        if not self.policydict:  # no policy configuration info
            return match

        # make a "flat" copy of ticket
        flat_ticket = self.make_flat_ticket(ticket)
        Trace.trace(10, "FLAT TICKET:%s"%(flat_ticket,))
        library = flat_ticket['library']
        library_manager = library + ".library_manager"
        if self.policydict.has_key(library_manager):
            rules = self.policydict[library_manager].keys() # these are policy numberc
            for rule in rules:
                # start matching
                policy_keys = self.policydict[library_manager][rule]['rule'] # these are plicy numbers
                rule_dict = self.policydict[library_manager][rule]['rule']
                nkeys = len(policy_keys)
                nmatches = 0
                for policy in policy_keys:
                    if not self.ticket_match(flat_ticket, rule_dict, policy): break
                    nmatches = nmatches + 1
                if nmatches == nkeys:
                    # match found
                    # check the file size:
                    if ticket['file_size'] < self.policydict[library_manager][rule]['minimal_file_size']:
                        match = (True, self.policydict[library_manager][rule]['resulting_library'])
                        return match
        return match

    # @param - ticket to match
    # @return - (True/False, Library/None)
    def match_found_pe(self, ticket):
        # returns a tuple:
        # True if match was found, False - if not
        # a string uniquly identifying policy 
        # Minimal allowed file size
        
        # default match settings
        match = False, None, 0
        failed = False

        if not self.policydict:  # no policy configuration info
            return match

        # make a "flat" copy of ticket
        flat_ticket = self.make_flat_ticket(ticket)
        Trace.trace(10, "FLAT TICKET:%s"%(flat_ticket,))
        library = flat_ticket['library']
        library_manager = library + ".library_manager"
        if self.policydict.has_key(library_manager):
            rules = self.policydict[library_manager].keys() # these are policy numbers
            for rule in rules:
                policy_string = "%s."%(library,)
                # start matching
                policy_keys = self.policydict[library_manager][rule]['rule'] # these are plicy numbers
                rule_dict = self.policydict[library_manager][rule]['rule']
                nkeys = len(policy_keys)
                nmatches = 0
                Trace.trace(10, "POLICY KEYS %s"%(policy_keys,)) 
                for policy in policy_keys:
                    if not self.ticket_match(flat_ticket, rule_dict, policy): break
                    policy_string = policy_string+"%s."%(rule_dict[policy],)
                    nmatches = nmatches + 1
                if nmatches == nkeys:
                    # match found
                    # check the file size:
                    if ticket['file_size'] < self.policydict[library_manager][rule]['minimal_file_size']:
                        match = (True, policy_string, self.policydict[library_manager][rule]['minimal_file_size'])
                        return match
        return match


if __name__ == "__main__":
    import socket
    import configuration_client
    host = socket.gethostname()
    ip = socket.gethostbyname(host)
    r = Selector('/home/enstore/policy_files/lmd_policy.py')
    print "DICT", r.policydict
    ticket={'lm': {'address': (ip, 7520)}, 'unique_id': '%s-1005321365-0-28872'%(host,), 'infile': '/pnfs/rip6/happy/mam/aci.py',
            'bfid': 'HAMS100471636100000', 'mover': 'MAM01.mover', 'at_the_top': 3, 'client_crc': 1, 'encp_daq': None,
            'encp': {'delayed_dismount': None, 'basepri': 1, 'adminpri': -1, 'curpri': 1, 'agetime': 0, 'delpri':0},
            'fc': {'size': 1434L, 'sanity_cookie': (1434L, 657638438L), 'bfid': 'HAMS100471636100000', 'location_cookie':
                   '0000_000000000_0000001', 'address': ('131.225.84.122', 7501), 'pnfsid': '00040000000000000040F2F8',
                   'pnfs_mapname': '/pnfs/rip6/volmap/alex/MM0001/0000_000000000_0000001', 'drive':
                   'happy:/dev/rmt/tps0d4n:0060112307', 'external_label': 'MM0001', 'deleted': 'no', 'pnfs_name0':
                   '/pnfs/rip6/happy/mam/aci.py', 'pnfsvid': '00040000000000000040F360', 'complete_crc': 657638438L,
                   'status': ('ok', None)},
            'file_size': 1434, 'outfile': '/dev/null', 'volume': 'MM0001',
            'times': {'t0': 1005321364.951048,'in_queue': 14.586493015289307, 'job_queued': 1005321365.7764519, 'lm_dequeued': 1005321380.363162},
            'version': 'v2_14  CVS $Revision$ ', 'retry': 0, 'work': 'read_from_hsm', 'callback_addr': ('131.225.13.132', 1463),
            'wrapper': {'minor': 5, 'inode': 0, 'fullname': '/dev/null', 'size_bytes': 1434, 'rmajor': 0, 'mode': 33268,
                        'pstat': (33204, 71365368, 5L, 1, 6849, 5440, 1434, 1004716362, 1004716362, 1004716329), 'gname': 'hppc',
                        'sanity_size': 65536, 'machine': ('Linux', 'gccensrv2.fnal.gov', '2.2.17-14', '#1 Mon Feb 5 18:48:50 EST 2001', 'i686'),
                        'uname': 'moibenko', 'pnfsFilename': '/pnfs/rip6/happy/mam/aci.py', 'uid': 6849, 'gid': 5440, 'rminor': 0, 'major': 0},
            'vc': {'first_access': 1004716170.54972, 'sum_rd_err': 0, 'last_access': 1004741744.274856, 'media_type': '8MM',
                   'capacity_bytes': 5368709120L, 'declared': 1004474612.7774431, 'remaining_bytes': 20105625600L,
                   'wrapper': 'cpio_odc', 'external_label': 'MM0001', 'system_inhibit': ['none', 'none'],
                   'user_inhibit': ['none', 'none'], 'current_location': '0000_000000000_0000001', 'sum_rd_access': 7,
                   'volume_family': 'D0.alex.cpio_odc', 'address': ('131.225.84.122', 7502), 'file_family': 'alex',
                   'sum_wr_access': 2, 'library': 'mam', 'sum_wr_err': 1, 'non_del_files': 1, 'blocksize': 131072,
                   'eod_cookie': '0000_000000000_0000002', 'storage_group': 'D0', 'status': ('ok', None)},
            'status': ('ok', None)
            }
    ticket1 = {'vc':
               {'storage_group': 'ANM', 'library': 'LTO3', 'file_family': 'gcc1', 'wrapper': 'cpio_odc', 'address': ('131.225.13.187', 7502), 'file_family_width': '1'},
               'outfilepath': '/pnfs/data2/test/moibenko/LTO3/regression_test/encp_test/D3',
               'encp':
               {'delpri': 0, 'basepri': 1, 'adminpri': -1, 'delayed_dismount': None, 'agetime': 0},
               'file_size': 4084853254L, 'ignore_fair_share': None, 'retry': None,
               'wrapper':
               {'major': 0, 'rminor': 0, 'pnfsFilename': '/pnfs/data2/test/moibenko/LTO3/regression_test/encp_test/D3', 'uid': 5744, 'uname': 'enstore', 'type': 'cpio_odc', 'mtime': 1319666280, 'rmajor': 0, 'machine': ('Linux', 'enmvr050.fnal.gov', '2.6.18-238.5.1.el5', '#1 SMP Tue Mar 1 18:58:43 EST 2011', 'x86_64'), 'sanity_size': 65536, 'gid': 6209, 'pstat': (16893, 36872184, 22L, 1, 5744, 6209, 512, 1319494584, 1319494584, 1300483574), 'mode_octal': '0100036', 'mode': 33188, 'gname': 'enstore', 'size_bytes': 4084853254L, 'fullname': '/opt/scratch/DEBUGLOG-2011-07-06', 'inode': 38134800, 'minor': 22},
               'version': 'v3_10c CVS $Revision$ encp',
               'encp_daq': None, 'client_crc': 1,
               'r_a': (('131.225.204.151', 40811), 1L, '131.225.204.151-40811-1319666285.627585-1783-46976464740336'),
               'crc_seed': 0,
               'infile': '/opt/scratch/DEBUGLOG-2011-07-06',
               'outfile': '/pnfs/data2/test/moibenko/LTO3/regression_test/encp_test/.(access)(00020000000000000045E410)',
               'fc': {'pnfsid': '000200', 'unique_id': 'enmvr050.fnal.gov-1319666280-1783-0'}}
    
    ticket2 = ticket1
    ret = r.match_found(ticket)
    print "Match result", ret
    print "========================================"
    ret = r.match_found(ticket1)
    
    print "Match result", ret

    ticket2['file_size'] = 1000000
    print "========================================"
    ret = r.match_found(ticket2)
    
    print "Match result", ret
    
