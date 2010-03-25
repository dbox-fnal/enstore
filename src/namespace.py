#!/usr/bin/env python

###############################################################################
#
# $Id$
#
###############################################################################

# system imports
import os
import sys
import string
import types

# enstore modules
import enstore_functions2
import pnfs_agent_client
import configuration_client
import pnfs
import chimera
import option

UNKNOWN = "unknown"  #Same in pnfs and chimera.

pnfs_agent_client_requested = False
pnfs_agent_client_allowed = False

############################################################################

def get_pac():
    config_host = enstore_functions2.default_host()
    config_port = enstore_functions2.default_port()
    csc = configuration_client.ConfigurationClient((config_host, config_port))
    pac = pnfs_agent_client.PnfsAgentClient(csc)
    return pac

############################################################################

__pychecker__ = "no-override"
class StorageFS(pnfs.Pnfs, chimera.ChimeraFS, pnfs_agent_client.PnfsAgentClient):

    def __init__(self, pnfsFilename="", mount_point="", shortcut=None,
                 use_pnfs_agent=False, allow_pnfs_agent=False):
        global pnfs_agent_client_requested
        global pnfs_agent_client_allowed

        #We insist on using the pnfs_agent.  Nothing else will do.
        if use_pnfs_agent or pnfs_agent_client_requested:
            self.use_pnfs_agent = 1
            config_host = enstore_functions2.default_host()
            config_port = enstore_functions2.default_port()
            csc = configuration_client.ConfigurationClient((config_host,
                                                            config_port))
            pnfs_agent_client.PnfsAgentClient.__init__(self, csc)
            self.__class__ = pnfs_agent_client.PnfsAgentClient
        elif pnfsFilename:
            #First, check for FS specific ID strings instead of a "filename"
            # passed into the constructor.
            if chimera.is_chimeraid(pnfsFilename):
                self.use_pnfs_agent = 0
                chimera.ChimeraFS.__init__(self, pnfsFilename,
                                           mount_point, shortcut)
                self.__class__ = chimera.ChimeraFS
            elif pnfs.is_pnfsid(pnfsFilename):
                self.use_pnfs_agent = 0
                pnfs.Pnfs.__init__(self, pnfsFilename, mount_point, shortcut)
                self.__class__ = pnfs.Pnfs
            #elif luster.is_lusterid(pnfsFilename):
            #    self.use_pnfs_agent = 0
            #    luster.LustreFS.__init__(self, pnfsFilename,
            #                             mount_point, shortcut)
            #    self.__class__ = lustre.LusterFS
            

            #Second, check for FS specific paths.
            elif chimera.is_chimera_path(pnfsFilename, check_name_only = 1):
                  # or chimera.is_chimera_path(mount_point, check_name_only=1):
                self.use_pnfs_agent = 0
                chimera.ChimeraFS.__init__(self, pnfsFilename,
                                           mount_point, shortcut)
                self.__class__ = chimera.ChimeraFS
            elif pnfs.is_pnfs_path(pnfsFilename, check_name_only = 1):
                self.use_pnfs_agent = 0
                pnfs.Pnfs.__init__(self, pnfsFilename, mount_point, shortcut)
                self.__class__ = pnfs.Pnfs
            #elif luster.is_luster_path(pnfsFilename, check_name_only = 1):
            #    self.use_pnfs_agent = 0
            #    luster.LustreFS.__init__(self, pnfsFilename,
            #                             mount_point, shortcut)
            #    self.__class__ = lustre.LusterFS

            #Third, optionally try the pnfs_agent.
            elif (allow_pnfs_agent or pnfs_agent_client_allowed) \
                     and is_storage_path(pnfsFilename):
                self.use_pnfs_agent = 1
                config_host = enstore_functions2.default_host()
                config_port = enstore_functions2.default_port()
                csc = configuration_client.ConfigurationClient((config_host,
                                                                config_port))
                pnfs_agent_client.PnfsAgentClient.__init__(self, csc)
                self.__class__ = pnfs_agent_client.PnfsAgentClient
            else:
                self.use_pnfs_agent = 0
        else:
            self.use_pnfs_agent = 0


class Tag(pnfs.Tag, chimera.Tag, pnfs_agent_client.PnfsAgentClient):
    def __init__(self, directory=None,
                 use_pnfs_agent=False, allow_pnfs_agent=False):
        global pnfs_agent_client_requested
        global pnfs_agent_client_allowed

       #We insist on using the pnfs_agent.  Nothing else will do.
        if use_pnfs_agent or pnfs_agent_client_requested:
            self.use_pnfs_agent = 1
            config_host = enstore_functions2.default_host()
            config_port = enstore_functions2.default_port()
            csc = configuration_client.ConfigurationClient((config_host,
                                                            config_port))
            pnfs_agent_client.PnfsAgentClient.__init__(self, csc)
            self.__class__ = pnfs_agent_client.PnfsAgentClient
        elif directory:
            #First, check for FS specific ID strings instead of a "filename"
            # passed into the constructor.
            if chimera.is_chimeraid(directory):
                self.use_pnfs_agent = 0
                chimera.Tag.__init__(self, directory)
                self.__class__ = chimera.Tag
            elif pnfs.is_pnfsid(directory):
                self.use_pnfs_agent = 0
                pnfs.Tag.__init__(self, directory)
                self.__class__ = pnfs.Tag
            #elif luster.is_lusterid(directory):
            #    self.use_pnfs_agent = 0
            #    lustre.Tag.__init__(self, directory)
            #    self.__class__ = lustre.Tag

            #Second, check for FS specific paths.
            elif chimera.is_chimera_path(directory, check_name_only = 1):
                self.use_pnfs_agent = 0
                chimera.Tag.__init__(self, directory)
                self.__class__ = chimera.Tag
            elif pnfs.is_pnfs_path(directory, check_name_only = 1):
                self.use_pnfs_agent = 0
                pnfs.Tag.__init__(self, directory)
                self.__class__ = pnfs.Tag
            #elif luster.is_luster_path(directory, check_name_only = 1):
            #    self.use_pnfs_agent = 0
            #    lustre.Tag.__init__(self, directory)
            #    self.__class__ = lustre.Tag

            #Third, optionally try the pnfs_agent.
            elif (allow_pnfs_agent or pnfs_agent_client_allowed) \
                     and is_storage_path(directory):
                self.use_pnfs_agent = 1
                config_host = enstore_functions2.default_host()
                config_port = enstore_functions2.default_port()
                csc = configuration_client.ConfigurationClient((config_host,
                                                                config_port))
                pnfs_agent_client.PnfsAgentClient.__init__(self, csc)
                self.__class__ = pnfs_agent_client.PnfsAgentClient
            else:
                self.use_pnfs_agent = 0
        else:
            self.use_pnfs_agent = 0

############################################################################

def is_storage_local_path(filename, check_name_only = None):
    if pnfs.is_pnfs_path(filename, check_name_only):
        return True
    elif chimera.is_chimera_path(filename, check_name_only):
        return True
    # We can extend it to check for lust path also
    #elif is_luster_path(filename, check_name_only):
    #   return True
    return False

def is_storage_remote_path(filename, check_name_only = None):
    global pnfs_agent_client_requested
    global pnfs_agent_client_allowed

    if pnfs_agent_client_requested or pnfs_agent_client_allowed:
        pac = get_pac()
        rtn = pac.is_pnfs_path(filename, check_name_only = check_name_only)

        if check_name_only:
            #If we get here we only want to determine if the filesystem is
            # valid pnfs filesystem.  Not whether the target actually exists.
            return rtn

        return pac.e_access(filename, os.F_OK)

    return False


def is_storage_path(filename, check_name_only = None):

    pathname = os.path.abspath(filename)

    rtn = is_storage_local_path(pathname, check_name_only)
    if not rtn:
        #If we get here we did not find a matching locally mounted
        # pnfs filesystem.  Ask the pnfs agent.
        rtn = is_storage_remote_path(pathname, check_name_only)
        
    return rtn
    
##############################################################################

class NamespaceInterface(option.Interface):

    def __init__(self, args=sys.argv, user_mode=1):
        # fill in the defaults for the possible options
        #self.test = 0
        #self.status = 0
        #self.info = 0
        #self.file = ""
        #self.restore = 0
        #These my be used, they may not.
        #self.duplicate_file = None
        option.Interface.__init__(self, args=args, user_mode=user_mode)

    pnfs_user_options = {
        option.BFID:{option.HELP_STRING:"lists the bit file id for file",
                     option.DEFAULT_VALUE:option.DEFAULT,
                     option.DEFAULT_NAME:"bfid",
                     option.DEFAULT_TYPE:option.INTEGER,
                     option.VALUE_NAME:"file",
                     option.VALUE_TYPE:option.STRING,
                     option.VALUE_USAGE:option.REQUIRED,
                     option.VALUE_LABEL:"filename",
                     option.FORCE_SET_DEFAULT:option.FORCE,
		     option.USER_LEVEL:option.USER
                     },
        option.CAT:{option.HELP_STRING:"see --layer",
                    option.DEFAULT_VALUE:option.DEFAULT,
                    option.DEFAULT_NAME:"layer",
                    option.DEFAULT_TYPE:option.INTEGER,
                    option.VALUE_NAME:"file",
                    option.VALUE_TYPE:option.STRING,
                    option.VALUE_USAGE:option.REQUIRED,
                    option.VALUE_LABEL:"filename",
                    option.FORCE_SET_DEFAULT:option.FORCE,
                    option.USER_LEVEL:option.USER,
                    option.EXTRA_VALUES:[{option.DEFAULT_VALUE:option.DEFAULT,
                                          option.DEFAULT_NAME:"named_layer",
                                          option.DEFAULT_TYPE:option.INTEGER,
                                          option.VALUE_NAME:"named_layer",
                                          option.VALUE_TYPE:option.INTEGER,
                                          option.VALUE_USAGE:option.OPTIONAL,
                                          option.VALUE_LABEL:"layer",
                                          }]
                    },
        #option.DUPLICATE:{option.HELP_STRING:"gets/sets duplicate file values",
        #             option.DEFAULT_VALUE:option.DEFAULT,
        #             option.DEFAULT_NAME:"duplicate",
        #             option.DEFAULT_TYPE:option.INTEGER,
        #             option.VALUE_USAGE:option.IGNORED,
	#	     option.USER_LEVEL:option.ADMIN,
        #             option.EXTRA_VALUES:[{option.DEFAULT_VALUE:"",
        #                                   option.DEFAULT_NAME:"file",
        #                                   option.DEFAULT_TYPE:option.STRING,
        #                                   option.VALUE_NAME:"file",
        #                                   option.VALUE_TYPE:option.STRING,
        #                                   option.VALUE_USAGE:option.OPTIONAL,
        #                                   option.VALUE_LABEL:"filename",
        #                                 option.FORCE_SET_DEFAULT:option.FORCE,
        #                                   },
        #                                  {option.DEFAULT_VALUE:"",
        #                                  option.DEFAULT_NAME:"duplicate_file",
        #                                   option.DEFAULT_TYPE:option.STRING,
        #                                   option.VALUE_NAME:"duplicat_file",
        #                                   option.VALUE_TYPE:option.STRING,
        #                                   option.VALUE_USAGE:option.OPTIONAL,
        #                               option.VALUE_LABEL:"duplicate_filename",
        #                                 option.FORCE_SET_DEFAULT:option.FORCE,
        #                                   },]
        #             },
        #option.ENSTORE_STATE:{option.HELP_STRING:"lists whether enstore " \
        #                                         "is still alive",
        #                 option.DEFAULT_VALUE:option.DEFAULT,
        #                 option.DEFAULT_NAME:"enstore_state",
        #                 option.DEFAULT_TYPE:option.INTEGER,
        #                 option.VALUE_NAME:"directory",
        #                 option.VALUE_TYPE:option.STRING,
        #                 option.VALUE_USAGE:option.REQUIRED,
        #                 option.USER_LEVEL:option.USER,
        #                 option.FORCE_SET_DEFAULT:option.FORCE,
        #             },
        option.FILE_FAMILY:{option.HELP_STRING: \
                            "gets file family tag, default; "
                            "sets file family tag, optional",
                            option.DEFAULT_VALUE:option.DEFAULT,
                            option.DEFAULT_NAME:"file_family",
                            option.DEFAULT_TYPE:option.INTEGER,
                            option.VALUE_TYPE:option.STRING,
                            option.USER_LEVEL:option.USER,
                            option.VALUE_USAGE:option.OPTIONAL,
                   },
        option.FILE_FAMILY_WIDTH:{option.HELP_STRING: \
                                  "gets file family width tag, default; "
                                  "sets file family width tag, optional",
                                  option.DEFAULT_VALUE:option.DEFAULT,
                                  option.DEFAULT_NAME:"file_family_width",
                                  option.DEFAULT_TYPE:option.INTEGER,
                                  option.VALUE_TYPE:option.STRING,
                                  option.USER_LEVEL:option.USER,
                                  option.VALUE_USAGE:option.OPTIONAL,
                   },
        option.FILE_FAMILY_WRAPPER:{option.HELP_STRING: \
                                    "gets file family wrapper tag, default; "
                                    "sets file family wrapper tag, optional",
                                    option.DEFAULT_VALUE:option.DEFAULT,
                                    option.DEFAULT_NAME:"file_family_wrapper",
                                    option.DEFAULT_TYPE:option.INTEGER,
                                    option.VALUE_TYPE:option.STRING,
                                    option.USER_LEVEL:option.USER,
                                    option.VALUE_USAGE:option.OPTIONAL,
                   },
	option.FILESIZE:{option.HELP_STRING:"print out real filesize",
			 option.VALUE_NAME:"file",
			 option.VALUE_TYPE:option.STRING,
			 option.VALUE_LABEL:"file",
                         option.USER_LEVEL:option.USER,
			 option.VALUE_USAGE:option.REQUIRED,
			 },
        option.INFO:{option.HELP_STRING:"see --xref",
                     option.DEFAULT_VALUE:option.DEFAULT,
                     option.DEFAULT_NAME:"xref",
                     option.DEFAULT_TYPE:option.INTEGER,
                     option.VALUE_NAME:"file",
                     option.VALUE_TYPE:option.STRING,
                     option.VALUE_USAGE:option.REQUIRED,
                     option.VALUE_LABEL:"filename",
                     option.USER_LEVEL:option.USER,
                     option.FORCE_SET_DEFAULT:option.FORCE,
                },
        option.LAYER:{option.HELP_STRING:"lists the layer of the file",
                      option.DEFAULT_VALUE:option.DEFAULT,
                      option.DEFAULT_NAME:"layer",
                      option.DEFAULT_TYPE:option.INTEGER,
                      option.VALUE_NAME:"file",
                      option.VALUE_TYPE:option.STRING,
                      option.VALUE_USAGE:option.REQUIRED,
                      option.VALUE_LABEL:"filename",
                      option.FORCE_SET_DEFAULT:option.FORCE,
                      option.USER_LEVEL:option.USER,
                      option.EXTRA_VALUES:[{option.DEFAULT_VALUE:
                                                                option.DEFAULT,
                                            option.DEFAULT_NAME:"named_layer",
                                            option.DEFAULT_TYPE:option.INTEGER,
                                            option.VALUE_NAME:"named_layer",
                                            option.VALUE_TYPE:option.INTEGER,
                                            option.VALUE_USAGE:option.OPTIONAL,
                                            option.VALUE_LABEL:"layer",
                                            }]
                 },
        option.LIBRARY:{option.HELP_STRING:"gets library tag, default; " \
                                      "sets library tag, optional",
                   option.DEFAULT_VALUE:option.DEFAULT,
                   option.DEFAULT_NAME:"library",
                   option.DEFAULT_TYPE:option.INTEGER,
                   option.VALUE_TYPE:option.STRING,
                   option.USER_LEVEL:option.USER,
                   option.VALUE_USAGE:option.OPTIONAL,
                   },
        #option.PNFS_STATE:{option.HELP_STRING:"lists whether pnfs is " \
        #                                      "still alive",
        #              option.DEFAULT_VALUE:option.DEFAULT,
        #              option.DEFAULT_NAME:"pnfs_state",
        #              option.DEFAULT_TYPE:option.INTEGER,
        #              option.VALUE_NAME:"directory",
        #              option.VALUE_TYPE:option.STRING,
        #              option.VALUE_USAGE:option.REQUIRED,
        #              option.USER_LEVEL:option.USER,
        #              option.FORCE_SET_DEFAULT:option.FORCE,
        #              },
        option.STORAGE_GROUP:{option.HELP_STRING:"gets storage group tag, " \
                              "default; sets storage group tag, optional",
                         option.DEFAULT_VALUE:option.DEFAULT,
                         option.DEFAULT_NAME:"storage_group",
                         option.DEFAULT_TYPE:option.INTEGER,
                         option.VALUE_TYPE:option.STRING,
                         option.USER_LEVEL:option.ADMIN,
                         option.VALUE_USAGE:option.OPTIONAL,
                   },
        option.TAG:{option.HELP_STRING:"lists the tag of the directory",
                    option.DEFAULT_VALUE:option.DEFAULT,
                    option.DEFAULT_NAME:"tag",
                    option.DEFAULT_TYPE:option.INTEGER,
                    option.VALUE_NAME:"named_tag",
                    option.VALUE_TYPE:option.STRING,
                    option.VALUE_USAGE:option.REQUIRED,
                    option.VALUE_LABEL:"tag",
                    option.FORCE_SET_DEFAULT:1,
                    option.USER_LEVEL:option.USER,
                    option.EXTRA_VALUES:[{option.DEFAULT_VALUE:"",
                                          option.DEFAULT_NAME:"directory",
                                          option.DEFAULT_TYPE:option.STRING,
                                          option.VALUE_NAME:"directory",
                                          option.VALUE_TYPE:option.STRING,
                                          option.VALUE_USAGE:option.OPTIONAL,
                                         option.FORCE_SET_DEFAULT:option.FORCE,
                                          }]
               },
        option.TAGCHMOD:{option.HELP_STRING:"changes the permissions"
                         " for the tag; use UNIX chmod style permissions",
                         option.DEFAULT_VALUE:option.DEFAULT,
                         option.DEFAULT_NAME:"tagchmod",
                         option.DEFAULT_TYPE:option.INTEGER,
                         option.VALUE_NAME:"permissions",
                         option.VALUE_TYPE:option.STRING,
                         option.VALUE_USAGE:option.REQUIRED,
                         option.FORCE_SET_DEFAULT:option.FORCE,
                         option.USER_LEVEL:option.USER,
                         option.EXTRA_VALUES:[{option.VALUE_NAME:"named_tag",
                                            option.VALUE_TYPE:option.STRING,
                                            option.VALUE_USAGE:option.REQUIRED,
                                            option.VALUE_LABEL:"tag",
                                              },]
                         },
        option.TAGCHOWN:{option.HELP_STRING:"changes the ownership"
                         " for the tag; OWNER can be 'owner' or 'owner.group'",
                         option.DEFAULT_VALUE:option.DEFAULT,
                         option.DEFAULT_NAME:"tagchown",
                         option.DEFAULT_TYPE:option.INTEGER,
                         option.VALUE_NAME:"owner",
                         option.VALUE_TYPE:option.STRING,
                         option.VALUE_USAGE:option.REQUIRED,
                         option.FORCE_SET_DEFAULT:option.FORCE,
                         option.USER_LEVEL:option.USER,
                         option.EXTRA_VALUES:[{option.VALUE_NAME:"named_tag",
                                            option.VALUE_TYPE:option.STRING,
                                            option.VALUE_USAGE:option.REQUIRED,
                                            option.VALUE_LABEL:"tag",
                                              },]
                         },
        option.TAGS:{option.HELP_STRING:"lists tag values and permissions",
                option.DEFAULT_VALUE:option.DEFAULT,
                option.DEFAULT_NAME:"tags",
                option.DEFAULT_TYPE:option.INTEGER,
                option.VALUE_USAGE:option.IGNORED,
                option.USER_LEVEL:option.USER,
                option.EXTRA_VALUES:[{option.DEFAULT_VALUE:"",
                                      option.DEFAULT_NAME:"directory",
                                      option.DEFAULT_TYPE:option.STRING,
                                      option.VALUE_NAME:"directory",
                                      option.VALUE_TYPE:option.STRING,
                                      option.VALUE_USAGE:option.OPTIONAL,
                                      option.FORCE_SET_DEFAULT:option.FORCE,
                                      }]
                },
        option.XREF:{option.HELP_STRING:"lists the cross reference " \
                                        "data for file",
                     option.DEFAULT_VALUE:option.DEFAULT,
                     option.DEFAULT_NAME:"xref",
                     option.DEFAULT_TYPE:option.INTEGER,
                     option.VALUE_NAME:"file",
                     option.VALUE_TYPE:option.STRING,
                     option.VALUE_USAGE:option.REQUIRED,
                     option.VALUE_LABEL:"filename",
                     option.USER_LEVEL:option.USER,
                     option.FORCE_SET_DEFAULT:option.FORCE,
                },
        }

    pnfs_admin_options = {
        option.CP:{option.HELP_STRING:"echos text to named layer of the file",
                   option.DEFAULT_VALUE:option.DEFAULT,
                   option.DEFAULT_NAME:"cp",
                   option.DEFAULT_TYPE:option.INTEGER,
                   option.VALUE_NAME:"unixfile",
                   option.VALUE_TYPE:option.STRING,
                   option.VALUE_USAGE:option.REQUIRED,
                   option.FORCE_SET_DEFAULT:option.FORCE,
                   option.USER_LEVEL:option.ADMIN,
                   option.EXTRA_VALUES:[{option.VALUE_NAME:"file",
                                         option.VALUE_TYPE:option.STRING,
                                         option.VALUE_USAGE:option.REQUIRED,
                                         option.VALUE_LABEL:"filename",
                                         },
                                        {option.VALUE_NAME:"named_layer",
                                         option.VALUE_TYPE:option.INTEGER,
                                         option.VALUE_USAGE:option.REQUIRED,
                                         option.VALUE_LABEL:"layer",
                                         },]
                   },
        option.CONST:{option.HELP_STRING:"Return information about the"
                      " underlying database.  Only PNFS returns valid"
                      " information.",
                      option.DEFAULT_VALUE:option.DEFAULT,
                      option.DEFAULT_NAME:"const",
                      option.DEFAULT_TYPE:option.INTEGER,
                      option.VALUE_NAME:"file",
                      option.VALUE_TYPE:option.STRING,
                      option.VALUE_USAGE:option.REQUIRED,
                      option.VALUE_LABEL:"filename",
                      option.FORCE_SET_DEFAULT:option.FORCE,
                      option.USER_LEVEL:option.ADMIN,
                      },
        option.COUNTERS:{option.HELP_STRING:"Return information about the"
                         " underlying database.  Only PNFS returns valid"
                         " information.",
                         option.DEFAULT_VALUE:option.DEFAULT,
                         option.DEFAULT_NAME:"counters",
                         option.DEFAULT_TYPE:option.INTEGER,
                         option.VALUE_NAME:"file",
                         option.VALUE_TYPE:option.STRING,
                         option.VALUE_USAGE:option.REQUIRED,
                         option.VALUE_LABEL:"filename",
                         option.FORCE_SET_DEFAULT:option.FORCE,
                         option.USER_LEVEL:option.ADMIN,
                         },
        option.COUNTERSN:{option.HELP_STRING:"Return information about the"
                          " underlying database.  Only PNFS returns valid"
                          " information.  (CWD must be under /pnfs)",
                          option.DEFAULT_VALUE:option.DEFAULT,
                          option.DEFAULT_NAME:"countersN",
                          option.DEFAULT_TYPE:option.INTEGER,
                          option.VALUE_NAME:"dbnum",
                          option.VALUE_TYPE:option.STRING,
                          option.VALUE_USAGE:option.REQUIRED,
                          option.FORCE_SET_DEFAULT:option.FORCE,
                          option.USER_LEVEL:option.ADMIN,
                          },
        option.CURSOR:{option.HELP_STRING:"Return information about the"
                       " underlying database.  Only PNFS returns valid"
                       " information.",
                       option.DEFAULT_VALUE:option.DEFAULT,
                       option.DEFAULT_NAME:"cursor",
                       option.DEFAULT_TYPE:option.INTEGER,
                       option.VALUE_NAME:"file",
                       option.VALUE_TYPE:option.STRING,
                       option.VALUE_USAGE:option.REQUIRED,
                       option.VALUE_LABEL:"filename",
                       option.FORCE_SET_DEFAULT:option.FORCE,
                       option.USER_LEVEL:option.ADMIN,
                       },
        option.DATABASE:{option.HELP_STRING:"Return information about the"
                         " underlying database.  Only PNFS returns valid"
                         " information.",
                         option.DEFAULT_VALUE:option.DEFAULT,
                         option.DEFAULT_NAME:"database",
                         option.DEFAULT_TYPE:option.INTEGER,
                         option.VALUE_NAME:"file",
                         option.VALUE_TYPE:option.STRING,
                         option.VALUE_USAGE:option.REQUIRED,
                         option.VALUE_LABEL:"filename",
                         option.FORCE_SET_DEFAULT:option.FORCE,
                         option.USER_LEVEL:option.ADMIN,
                         },
        option.DATABASEN:{option.HELP_STRING:"Return information about the"
                         " underlying database.  Only PNFS returns valid"
                         " information.  (CWD must be under /pnfs)",
                          option.DEFAULT_VALUE:option.DEFAULT,
                          option.DEFAULT_NAME:"databaseN",
                          option.DEFAULT_TYPE:option.INTEGER,
                          option.VALUE_NAME:"dbnum",
                          option.VALUE_TYPE:option.STRING,
                          option.VALUE_USAGE:option.REQUIRED,
                          option.FORCE_SET_DEFAULT:option.FORCE,
                          option.USER_LEVEL:option.ADMIN,
                          },
        #option.DOWN:{option.HELP_STRING:"creates enstore system-down " \
        #                                "wormhole to prevent transfers",
        #        option.DEFAULT_VALUE:option.DEFAULT,
        #        option.DEFAULT_NAME:"down",
        #        option.DEFAULT_TYPE:option.INTEGER,
        #        option.VALUE_NAME:"reason",
        #        option.VALUE_TYPE:option.STRING,
        #        option.VALUE_USAGE:option.REQUIRED,
        #        option.FORCE_SET_DEFAULT:option.FORCE,
        #        option.USER_LEVEL:option.ADMIN,
        #        },
        option.DUMP:{option.HELP_STRING:"dumps info",
              option.DEFAULT_VALUE:option.DEFAULT,
              option.DEFAULT_NAME:"dump",
              option.DEFAULT_TYPE:option.INTEGER,
              option.VALUE_USAGE:option.IGNORED,
              option.USER_LEVEL:option.ADMIN,
              },
        option.ECHO:{option.HELP_STRING:"sets text to named layer of the file",
                     option.DEFAULT_VALUE:option.DEFAULT,
                     option.DEFAULT_NAME:"echo",
                     option.DEFAULT_TYPE:option.INTEGER,
                     option.VALUE_NAME:"text",
                     option.VALUE_TYPE:option.STRING,
                     option.VALUE_USAGE:option.REQUIRED,
                     option.FORCE_SET_DEFAULT:option.FORCE,
                     option.USER_LEVEL:option.ADMIN,
                     option.EXTRA_VALUES:[{option.VALUE_NAME:"file",
                                           option.VALUE_TYPE:option.STRING,
                                           option.VALUE_USAGE:option.REQUIRED,
                                           option.VALUE_LABEL:"filename",
                                           },
                                          {option.VALUE_NAME:"named_layer",
                                           option.VALUE_TYPE:option.INTEGER,
                                           option.VALUE_USAGE:option.REQUIRED,
                                           option.VALUE_LABEL:"layer",
                                           },]
                },
        option.ID:{option.HELP_STRING:"prints the pnfs id",
                   option.DEFAULT_VALUE:option.DEFAULT,
                   option.DEFAULT_NAME:"id",
                   option.DEFAULT_TYPE:option.INTEGER,
                   option.VALUE_NAME:"file",
                   option.VALUE_TYPE:option.STRING,
                   option.VALUE_USAGE:option.REQUIRED,
                   option.VALUE_LABEL:"filename",
                   option.FORCE_SET_DEFAULT:option.FORCE,
                   option.USER_LEVEL:option.ADMIN,
              },
        option.IO:{option.HELP_STRING:"sets io mode (can't clear it easily)",
                   option.DEFAULT_VALUE:option.DEFAULT,
                   option.DEFAULT_NAME:"io",
                   option.DEFAULT_TYPE:option.INTEGER,
                   option.VALUE_NAME:"file",
                   option.VALUE_TYPE:option.STRING,
                   option.VALUE_USAGE:option.REQUIRED,
                   option.VALUE_LABEL:"filename",
                   option.FORCE_SET_DEFAULT:option.FORCE,
                   option.USER_LEVEL:option.ADMIN,
                   },
        option.LS:{option.HELP_STRING:"does an ls on the named layer " \
                                      "in the file",
                   option.DEFAULT_VALUE:option.DEFAULT,
                   option.DEFAULT_NAME:"ls",
                   option.DEFAULT_TYPE:option.INTEGER,
                   option.VALUE_NAME:"file",
                   option.VALUE_TYPE:option.STRING,
                   option.VALUE_USAGE:option.REQUIRED,
                   option.VALUE_LABEL:"filename",
                   option.FORCE_SET_DEFAULT:option.FORCE,
                   option.USER_LEVEL:option.ADMIN,
                   option.EXTRA_VALUES:[{option.DEFAULT_VALUE:option.DEFAULT,
                                         option.DEFAULT_NAME:"named_layer",
                                         option.DEFAULT_TYPE:option.INTEGER,
                                         option.VALUE_NAME:"named_layer",
                                         option.VALUE_TYPE:option.STRING,
                                         option.VALUE_USAGE:option.OPTIONAL,
                                         option.VALUE_LABEL:"layer",
                                         }]
              },
        option.MOUNT_POINT:{option.HELP_STRING:"prints the mount point of " \
                            "the pnfs file or directory",
                            option.DEFAULT_VALUE:option.DEFAULT,
                            option.DEFAULT_NAME:"mount_point",
                            option.DEFAULT_TYPE:option.INTEGER,
                            option.VALUE_NAME:"file",
                            option.VALUE_TYPE:option.STRING,
                            option.VALUE_USAGE:option.REQUIRED,
                            option.VALUE_LABEL:"filename",
                            option.FORCE_SET_DEFAULT:option.FORCE,
                            option.USER_LEVEL:option.ADMIN,
                            },
        option.NAMEOF:{option.HELP_STRING:"prints the filename of the PNFS ID"
                       " or Chimera ID.  (CWD must be under /pnfs)",
                       option.DEFAULT_VALUE:option.DEFAULT,
                       option.DEFAULT_NAME:"nameof",
                       option.DEFAULT_TYPE:option.INTEGER,
                       option.VALUE_NAME:"pnfs_id",
                       option.VALUE_TYPE:option.STRING,
                       option.VALUE_USAGE:option.REQUIRED,
                       option.FORCE_SET_DEFAULT:option.FORCE,
                       option.USER_LEVEL:option.ADMIN,
                       },
        option.PARENT:{option.HELP_STRING:"prints the PNFS ID or Chimera ID"
                       "of the parent directory (CWD must be under /pnfs)",
                       option.DEFAULT_VALUE:option.DEFAULT,
                       option.DEFAULT_NAME:"parent",
                       option.DEFAULT_TYPE:option.INTEGER,
                       option.VALUE_NAME:"pnfs_id",
                       option.VALUE_TYPE:option.STRING,
                       option.VALUE_USAGE:option.REQUIRED,
                       option.FORCE_SET_DEFAULT:option.FORCE,
                       option.USER_LEVEL:option.ADMIN,
                       },
        option.PATH:{option.HELP_STRING:
                     "prints the file path of the PNFS id or Chimera ID.  "
                     "(CWD must be under /pnfs)",
                     option.DEFAULT_VALUE:option.DEFAULT,
                     option.DEFAULT_NAME:"path",
                     option.DEFAULT_TYPE:option.INTEGER,
                     option.VALUE_NAME:"pnfs_id",
                     option.VALUE_TYPE:option.STRING,
                     option.VALUE_USAGE:option.REQUIRED,
                     option.FORCE_SET_DEFAULT:option.FORCE,
                     option.USER_LEVEL:option.ADMIN,
                     },
        option.POSITION:{option.HELP_STRING:"Return information about the"
                         " underlying database.  Only PNFS returns valid"
                         " information.",
                         option.DEFAULT_VALUE:option.DEFAULT,
                         option.DEFAULT_NAME:"position",
                         option.DEFAULT_TYPE:option.INTEGER,
                         option.VALUE_NAME:"file",
                         option.VALUE_TYPE:option.STRING,
                         option.VALUE_USAGE:option.REQUIRED,
                         option.VALUE_LABEL:"filename",
                         option.FORCE_SET_DEFAULT:option.FORCE,
                         option.USER_LEVEL:option.ADMIN,
                         },
        option.RM:{option.HELP_STRING:"deletes (clears) named layer of the file",
                   option.DEFAULT_VALUE:option.DEFAULT,
                   option.DEFAULT_NAME:"rm",
                   option.DEFAULT_TYPE:option.INTEGER,
                   option.VALUE_NAME:"file",
                   option.VALUE_TYPE:option.STRING,
                   option.VALUE_USAGE:option.REQUIRED,
                   option.VALUE_LABEL:"filename",
                   option.FORCE_SET_DEFAULT:option.FORCE,
                   option.USER_LEVEL:option.ADMIN,
                   option.EXTRA_VALUES:[{option.VALUE_NAME:"named_layer",
                                         option.VALUE_TYPE:option.INTEGER,
                                         option.VALUE_USAGE:option.REQUIRED,
                                         option.VALUE_LABEL:"layer",
                                         },]
                   },
        option.SHOWID:{option.HELP_STRING:"prints the PNFS ID information",
                       option.DEFAULT_VALUE:option.DEFAULT,
                       option.DEFAULT_NAME:"showid",
                       option.DEFAULT_TYPE:option.INTEGER,
                       option.VALUE_NAME:"pnfs_id",
                       option.VALUE_TYPE:option.STRING,
                       option.VALUE_USAGE:option.REQUIRED,
                       option.FORCE_SET_DEFAULT:option.FORCE,
                       option.USER_LEVEL:option.ADMIN,
                       },
        option.SIZE:{option.HELP_STRING:"sets the size of the file",
                     option.DEFAULT_VALUE:option.DEFAULT,
                     option.DEFAULT_NAME:"size",
                     option.DEFAULT_TYPE:option.INTEGER,
                     option.VALUE_NAME:"file",
                     option.VALUE_TYPE:option.STRING,
                     option.VALUE_USAGE:option.REQUIRED,
                     option.VALUE_LABEL:"filename",
                     option.FORCE_SET_DEFAULT:option.FORCE,
                     option.USER_LEVEL:option.USER2,
                     option.EXTRA_VALUES:[{option.VALUE_NAME:"filesize",
                                           option.VALUE_TYPE:option.LONG,
                                           option.VALUE_USAGE:option.REQUIRED,
                                           },]
                },
        option.TAGECHO:{option.HELP_STRING:"echos text to named tag",
                        option.DEFAULT_VALUE:option.DEFAULT,
                        option.DEFAULT_NAME:"tagecho",
                        option.DEFAULT_TYPE:option.INTEGER,
                        option.VALUE_NAME:"text",
                        option.VALUE_TYPE:option.STRING,
                        option.VALUE_USAGE:option.REQUIRED,
                        option.FORCE_SET_DEFAULT:option.FORCE,
                        option.USER_LEVEL:option.ADMIN,
                        option.EXTRA_VALUES:[{option.VALUE_NAME:"named_tag",
                                            option.VALUE_TYPE:option.STRING,
                                            option.VALUE_USAGE:option.REQUIRED,
                                            option.VALUE_LABEL:"tag",
                                              },]
                   },

        option.TAGRM:{option.HELP_STRING:"removes the tag (tricky, see DESY "
                                         "documentation)",
                      option.DEFAULT_VALUE:option.DEFAULT,
                      option.DEFAULT_NAME:"tagrm",
                      option.DEFAULT_TYPE:option.INTEGER,
                      option.VALUE_NAME:"named_tag",
                      option.VALUE_TYPE:option.STRING,
                      option.VALUE_USAGE:option.REQUIRED,
                      option.VALUE_LABEL:"tag",
                      option.FORCE_SET_DEFAULT:option.FORCE,
                      option.USER_LEVEL:option.ADMIN,
                 },
        #option.UP:{option.HELP_STRING:"removes enstore system-down wormhole",
        #           option.DEFAULT_VALUE:option.DEFAULT,
        #           option.DEFAULT_NAME:"up",
        #           option.DEFAULT_TYPE:option.INTEGER,
        #           option.VALUE_USAGE:option.IGNORED,
        #           option.USER_LEVEL:option.ADMIN,
        #           },
        }
    
    def valid_dictionaries(self):
        return (self.help_options, self.pnfs_user_options,
                self.pnfs_admin_options)

    # parse the options like normal but make sure we have other args
    def parse_options(self):
        self.pnfs_id = "" #Assume the command is a dir and/or file.
        self.file = ""
        self.dir = ""
        option.Interface.parse_options(self)

        if not self.option_list:
            self.print_usage("No valid options were given.")

        #No pnfs options take extra arguments beyond those specifed in the
        # option dictionaries.  If there are print message and exit.
        self.check_correct_count()

        if getattr(self, "help", None):
            self.print_help()

        if getattr(self, "usage", None):
            self.print_usage()

##############################################################################

def do_work(intf):
    rtn = 0

    try:
        if intf.file:
            p=StorageFS(intf.file)
            t=None
            n=None
        elif intf.pnfs_id:
            p=StorageFS(intf.pnfs_id, shortcut=True)
            t=None
            n=None
        elif hasattr(intf, "dbnum") and intf.dbnum:
            p=None
            t=None
            n=pnfs.N(intf.dbnum)
        else:
            p=None
            if intf.dir:
                t=Tag(intf.dir)
            else:
                t=Tag(os.getcwd())
            n=None
    except OSError, msg:
        print str(msg)
        return 1
        
    for arg in intf.option_list:
        if string.replace(arg, "_", "-") in intf.options.keys():
            arg = string.replace(arg, "-", "_")
            for instance in [t, p, n]:
                if getattr(instance, "p"+arg, None):
                    try:
                        #Not all functions use/need intf passed in.
                        rtn = apply(getattr(instance, "p" + arg), ())
                    except TypeError:
                        rtn = apply(getattr(instance, "p" + arg), (intf,))
                    break
            else:
                print "p%s not found" % arg 
                rtn = 1

    return rtn

##############################################################################
if __name__ == "__main__":

    intf = NamespaceInterface(user_mode=0)

    intf._mode = "admin"

    sys.exit(do_work(intf))