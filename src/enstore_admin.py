#!/usr/bin/env python
###############################################################################
# src/$RCSfile$   $Revision$
#

import enstore

def do_work():
    # user mode
    mode = 0

    en = enstore.Enstore(mode)
    en.do_work()

if __name__ == "__main__" :

    do_work()
