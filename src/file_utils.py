#!/usr/bin/env python
###############################################################################
#
# $Author$
# $Date$
# $Id$
#
# collection of utility functions taken from encp.py
# Author: Dmitry Litvintsev (litvinse@fnal.gov) 09/05
#
###############################################################################

# system imports
import sys
import os
import stat
import threading
import types
import errno

import Trace
import atomic

## mode is one of os.F_OK, os.W_OK, os.R_OK or os.X_OK.
## file_stats is the return from os.stat()

#The os.access() and the access(2) C library routine use the real id when
# testing for access.  This function does the same thing but for the
# effective ID.
def e_access(path, mode):

    #Test for existance.
    try:
        file_stats = get_stat(path)
    except OSError:
        return 0

    return e_access_cmp(file_stats, mode)

#Check the bits to see if we have the requested mode access.
def e_access_cmp(file_stats, mode):
    stat_mode = file_stats[stat.ST_MODE]

    #Make sure a valid mode was passed in.
    if mode & (os.F_OK | os.R_OK | os.W_OK | os.X_OK) != mode:
        return 0

    # Need to check for each type of access permission.

    if mode == os.F_OK:
        # In order to get this far, the file must exist.
        return 1

    if mode & os.R_OK:  #Check for read permissions.
        #If the user is user root.
        if os.geteuid() == 0:
            pass
        #Anyone can read this file.
        elif (stat_mode & stat.S_IROTH):
            pass
        #This is the files owner.
        elif (stat_mode & stat.S_IRUSR) and \
           file_stats[stat.ST_UID] == os.geteuid():
            pass
        #The user has group access.
        elif (stat_mode & stat.S_IRGRP) and \
           (file_stats[stat.ST_GID] == os.geteuid() or
            file_stats[stat.ST_GID] in os.getgroups()):
            pass
        else:
            return 0

    if mode & os.W_OK:  #Check for write permissions.
        #If the user is user root.
        if os.geteuid() == 0:
            pass
        #Anyone can write this file.
        elif (stat_mode & stat.S_IWOTH):
            pass
        #This is the files owner.
        elif (stat_mode & stat.S_IWUSR) and \
           file_stats[stat.ST_UID] == os.geteuid():
            pass
        #The user has group access.
        elif (stat_mode & stat.S_IWGRP) and \
           (file_stats[stat.ST_GID] == os.geteuid() or
            file_stats[stat.ST_GID] in os.getgroups()):
            pass
        else:
            return 0

    if mode & os.X_OK:  #Check for execute permissions.
        #If the user is user root.
        if os.geteuid() == 0:
            pass
        #Anyone can execute this file.
        elif (stat_mode & stat.S_IXOTH):
            pass
        #This is the files owner.
        elif (stat_mode & stat.S_IXUSR) and \
           file_stats[stat.ST_UID] == os.geteuid():
            pass
        #The user has group access.
        elif (stat_mode & stat.S_IXGRP) and \
           (file_stats[stat.ST_GID] == os.geteuid() or
            file_stats[stat.ST_GID] in os.getgroups()):
            pass
        else:
            return 0

    return 1

#############################################################################

## This section of code contains wrapper functions around os module functions
## in a thread safe manner with respect to seteuid().

#arg can be: filename, file descritor, file object, a stat object
def get_stat(arg, use_lstat = False):
    try:
        if type(arg) == types.StringType:
            if use_lstat:
                f_stat = os.lstat(arg)
            else:
                f_stat = os.stat(arg)
        elif type(arg) == types.IntType:
            f_stat = os.fstat(arg)
        elif type(arg) == types.FileType:
            f_stat = os.fstat(arg.fileno())
        elif type(arg) == types.TupleType or type(arg) == os.stat_result:
            f_stat = arg
        else:
            raise TypeError("Expected path, file descriptor or file object; "
                            "not %s" % (type(arg),))
    except OSError, msg:
        #If we were denied access and our effective IDS were not root's,
        # set the effective IDS to root so we can try again.
        if msg.errno in [errno.EACCES, errno.EPERM] and \
               os.getuid() == 0 and os.geteuid() != 0:
            acquire_lock_euid_egid()
            current_euid = os.geteuid()
            current_egid = os.getegid()

            try:
                os.seteuid(0)
                os.setegid(0)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                #Calling stat again won't get stuck in a loop since the
                # effective IDS have been changed.
                f_stat = get_stat(arg, use_lstat)
            except OSError:  #Anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            except:  #Un-anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.setegid(current_egid)
                os.seteuid(current_euid)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            release_lock_euid_egid()
        else:
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

    return f_stat

#Because open() is a builtin, pychecker gives a "(open) shadows builtin"
# warning.  This suppresses that warning, but it will do so for all functions
# in this module.
__pychecker__ = "no-shadowbuiltin"

#Open the file fname.  Mode has same meaning as builtin open().
def open(fname, mode = "r"):
    try:
        file_p = __builtins__['open'](fname, mode)
    except (OSError, IOError), msg:
        #If we were denied access and our effective IDS were not root's,
        # set the effective IDS to root so we can try again.
        if msg.errno in [errno.EACCES, errno.EPERM] and \
               os.getuid() == 0 and os.geteuid() != 0:
            acquire_lock_euid_egid()
            current_euid = os.geteuid()
            current_egid = os.getegid()

            try:
                os.seteuid(0)
                os.setegid(0)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                file_p = __builtins__['open'](fname, mode)
            except (OSError, IOError), msg:  #Anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            except:  #Un-anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.setegid(current_egid)
                os.seteuid(current_euid)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            release_lock_euid_egid()
        else:
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

    return file_p

#Open the file fname.  This is a wrapper for os.open() (atomic.open() is
# another level of wrapper for os.open()).
def open_fd(fname, flags, mode = 0777):
    #Call atomic.open() if we expect to crate the file.  Use os.open() if
    # the file should already exist.
    try:
        if flags & os.O_CREAT:
            Trace.log(99, "%s %s %s" % (fname, flags, mode))
            file_fd = atomic.open(fname, flags, mode)
        else:
            file_fd = os.open(fname, flags, mode)
    except (OSError, IOError), msg:
        Trace.log(99, str(msg))
        #If we were denied access and our effective IDS were not root's,
        # set the effective IDS to root so we can try again.
        if msg.errno in [errno.EACCES, errno.EPERM] and \
               os.getuid() == 0 and os.geteuid() != 0:
            acquire_lock_euid_egid()
            current_euid = os.geteuid()
            current_egid = os.getegid()

            try:
                os.seteuid(0)
                os.setegid(0)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                if flags & os.O_CREAT:
                    file_fd = atomic.open(fname, flags, mode)
                else:
                    file_fd = os.open(fname, flags, mode)
            except (OSError, IOError), msg:  #Anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            except:  #Un-anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.setegid(current_egid)
                os.seteuid(current_euid)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            release_lock_euid_egid()
        else:
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

    return file_fd

#Obtain the contents of the specified directory.
def listdir(dname):
    try:
        directory_listing = os.listdir(dname)
    except (OSError, IOError), msg:
        #If we were denied access and our effective IDS were not root's,
        # set the effective IDS to root so we can try again.
        if msg.errno in [errno.EACCES, errno.EPERM] and \
               os.getuid() == 0 and os.geteuid() != 0:
            acquire_lock_euid_egid()
            current_euid = os.geteuid()
            current_egid = os.getegid()

            try:
                os.seteuid(0)
                os.setegid(0)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                directory_listing = os.listdir(dname)
            except (OSError, IOError), msg:  #Anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            except:  #Un-anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.setegid(current_egid)
                os.seteuid(current_euid)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            release_lock_euid_egid()
        else:
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

    return directory_listing


#Change the permissions of file fname.  Perms have same meaning as os.chmod().
def chmod(fname, perms):
    try:
        os.chmod(fname, perms)
    except (OSError, IOError), msg:
        #If we were denied access and our effective IDS were not root's,
        # set the effective IDS to root so we can try again.
        if msg.errno in [errno.EACCES, errno.EPERM] and \
               os.getuid() == 0 and os.geteuid() != 0:
            acquire_lock_euid_egid()
            current_euid = os.geteuid()
            current_egid = os.getegid()

            try:
                os.seteuid(0)
                os.setegid(0)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.chmod(fname, perms)
            except (OSError, IOError), msg:  #Anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            except:  #Un-anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.setegid(current_egid)
                os.seteuid(current_euid)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            release_lock_euid_egid()
        else:
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

#Change the owner of file fname.  Perms have same meaning as os.chmod().
def chown(fname, uid, gid):
    try:
        os.chown(fname, uid, gid)
    except (OSError, IOError), msg:
        #If we were denied access and our effective IDS were not root's,
        # set the effective IDS to root so we can try again.
        if msg.errno in [errno.EACCES, errno.EPERM] and \
               os.getuid() == 0 and os.geteuid() != 0:
            acquire_lock_euid_egid()
            current_euid = os.geteuid()
            current_egid = os.getegid()

            try:
                os.seteuid(0)
                os.setegid(0)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.chown(fname, uid, gid)
            except (OSError, IOError), msg:  #Anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            except:  #Un-anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.setegid(current_egid)
                os.seteuid(current_euid)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            release_lock_euid_egid()
        else:
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

#Update the times of file fname.  Access time and modification time are
# the same as os.chown().
def utime(fname, times):
    try:
        os.utime(fname, times)
    except (OSError, IOError), msg:
        #If we were denied access and our effective IDS were not root's,
        # set the effective IDS to root so we can try again.
        if msg.errno in [errno.EACCES, errno.EPERM] and \
               os.getuid() == 0 and os.geteuid() != 0:
            acquire_lock_euid_egid()
            current_euid = os.geteuid()
            current_egid = os.getegid()

            try:
                os.seteuid(0)
                os.setegid(0)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.utime(fname, times)
            except (OSError, IOError), msg:  #Anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            except:  #Un-anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.setegid(current_egid)
                os.seteuid(current_euid)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            release_lock_euid_egid()
        else:
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

#Remove the file fname from the filesystem.
def remove(fname):
    try:
        os.remove(fname)
    except (OSError, IOError), msg:
        #If we were denied access and our effective IDS were not root's,
        # set the effective IDS to root so we can try again.
        if msg.errno in [errno.EACCES, errno.EPERM] and \
               os.getuid() == 0 and os.geteuid() != 0:
            acquire_lock_euid_egid()
            current_euid = os.geteuid()
            current_egid = os.getegid()

            try:
                os.seteuid(0)
                os.setegid(0)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.remove(fname)
            except (OSError, IOError), msg:  #Anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            except:  #Un-anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.setegid(current_egid)
                os.seteuid(current_euid)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            release_lock_euid_egid()
        else:
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]


#############################################################################

#If root is running the process, we may need to change the euid.  Is this
# only applicable to migration?
euid_lock = threading.RLock()

def acquire_lock_euid_egid():
    if euid_lock._RLock__count > 0 and \
           euid_lock._RLock__owner == threading.currentThread():
        Trace.log(67, "lock count: %s" % (euid_lock._RLock__count,))
        Trace.log_stack_trace(severity = 67)
    euid_lock.acquire()

def release_lock_euid_egid():
    try:
        euid_lock.release()
    except RuntimeError:
        pass  #Already unlocked.


#Match the effective uid/gid of a file.
# arg: could be a pathname, fileno or file object.
#
# We need to do this when user root, so that migration modify non-/pnfs/fs
# (and non-trusted) pnfs mount points.
def match_euid_egid(arg):

    if os.getuid() == 0 or os.getgid() == 0:

        f_stat = get_stat(arg)

        #Acquire the lock.
        acquire_lock_euid_egid()

        try:
            set_euid_egid(f_stat[stat.ST_UID], f_stat[stat.ST_GID])
        except:
            release_lock_euid_egid()
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

#Set the effective uid/gid.
# euid - The new effective user ID.
# egid - The new effective group ID.
def set_euid_egid(euid, egid):

    if os.getuid() == 0:

        #We need to set these back to root, with uid first.  If the group
        # is changed we need to also set the euid, to that we will have
        # permissions to set the egid to another non-root user ID.
        if euid != 0  or egid != 0:
            os.seteuid(0)
        if egid != 0:
            os.setegid(0)

        #First look at the gid for setting them.
        if egid != os.getegid():
            os.setegid(egid)
        #Then look a the uid.
        if euid != os.geteuid():
            os.seteuid(euid)

#Release the lock.
def end_euid_egid(reset_ids_back = False):

    if os.getuid() == 0 or os.getgid() == 0:
        if reset_ids_back:
            os.seteuid(0)
            os.setegid(0)

        release_lock_euid_egid()

#############################################################################

# this function does "rm -f path"
def rmdir(path):
    if (os.path.isdir(path)):
        for direntry in os.listdir(os.path.abspath(path)):
            rmdir(os.path.join(path,direntry))
        os.rmdir(path)
    else:
        os.unlink(path)
#
# wrapper to call os functions that takes care of euid/eid
#
def wrapper(function,args=()):
    try:
        if type(args) != types.TupleType:
            rtn = apply(function, (args,))
        else:
            rtn = apply(function, args)
    except (OSError, IOError), msg:
        if msg.errno in [errno.EACCES, errno.EPERM] and \
               os.getuid() == 0 and os.geteuid() != 0:
            acquire_lock_euid_egid()
            current_euid = os.geteuid()
            current_egid = os.getegid()
            try:
                os.seteuid(0)
                os.setegid(0)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            try:
                if type(args) != types.TupleType:
                    rtn =  apply(function, (args,))
                else:
                    rtn = apply(function, args)
            except (OSError, IOError), msg:  #Anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]
            except:  #Un-anticipated errors.
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            try:
                os.setegid(current_egid)
                os.seteuid(current_euid)
            except:
                release_lock_euid_egid()
                raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

            release_lock_euid_egid()
        else:
            raise sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]

    return rtn

