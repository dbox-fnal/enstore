
#!/usr/bin/env python

# $Id$

#mover states
IDLE, SETUP, MOUNT_WAIT, SEEK, ACTIVE, HAVE_BOUND, DISMOUNT_WAIT, DRAINING, OFFLINE, CLEANING, ERROR = [
    'IDLE', 'SETUP', 'MOUNT_WAIT', 'SEEK', 'ACTIVE', 'HAVE_BOUND', 'DISMOUNT_WAIT',
    'DRAINING', 'OFFLINE', 'CLEANING', 'ERROR']

#mover modes
READ, WRITE = ['READ','WRITE']

#error sources
TAPE, ROBOT, NETWORK = ['TAPE', 'ROBOT', 'NETWORK']



