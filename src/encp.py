import sys
import os
import stat
import time
import errno
import pprint
import pwd
import grp
import socket
import binascii
import regsub
import pdb

# enstore modules
import pnfs
import callback
import log_client
import configuration_client
import udp_client
import EXfer

##############################################################################

def write_to_hsm(input, output, config_host, config_port, list, chk_crc) :
    t0 = time.time()
    tinfo = {}
    tinfo["abs_start"] = t0

    if list>2:
        print "Getting clients, storing/checking local info   cumt=",\
              time.time()-t0
    t1 = time.time() #----------------------------------------------------Start

    # initialize - and get config, udp and log clients
    maxretry = 2
    unique_id = []
    global logc # needs to be global so other defs can use it in this file
    (csc,u,uinfo) = clients(config_host,config_port,list)

    tinfo["clients"] = time.time() - t1 #-----------------------------------End
    if list>2:
        print "  dt:",tinfo["clients"], "   cumt=",time.time()-t0
    if list>3:
        print "csc=",csc
        print "u=",u
        print "logc=",logc
        print "uinfo=",uinfo

    if list>2:
        print "Checking input unix files:",input, "   cumt=",time.time()-t0
    t1 =  time.time() #---------------------------------------------------Start

    # check the input unix files. if files don't exits, we bomb out to the user
    (ninput, inputlist, file_size) = inputfile_check(input)
    if ninput>1:
        delayed_dismount = 30
    else:
        delayed_dismount = 0

    tinfo["filecheck"] = time.time() - t1 #---------------------------------End
    if list>2:
        print "  dt:",tinfo["filecheck"], "   cumt=",time.time()-t0
    if list>3:
        print "ninput=",ninput
        print "inputlist=",inputlist
        print "file_size=",file_size
        print "delayed_dismount=",delayed_dismount

    if list>2:
        print "Checking output pnfs files:",output, "   cumt=",time.time()-t0
    t1 = time.time() #----------------------------------------------------Start

    # check (and generate) the output pnfs files(s) names
    # bomb out if they exist already
    outputlist = outputfile_check(ninput,inputlist,output)
    (junk,library,file_family,width,pinfo,p)=pnfs_information(outputlist,ninput)

    # note: Since multiple input files all go to 1 directory:
    #       all libraries are the same
    #       all file families are the same
    #       all widths are the same
    # be cautious and check to make sure this is indeed correct
    for i in range(1,ninput):
        if library[i]!=library[0] or\
           file_family[i]!=file_family[0] or\
           width[i]!=width[0] :
            print "library=",library
            print "file_family=",file_family
            print "width=",width
            jraise(errno.errorcode[errno.EPROTO]," encp.write_to_hsm: TILT "\
                   " library, file_family, width not all the same")

    tinfo["pnfscheck"] = time.time() - t1 #---------------------------------End
    if list>2:
        print "  dt:",tinfo["pnfscheck"], "   cumt=",time.time()-t0
    if list>3:
        print "outputlist=",outputlist
        print "library=",library
        print "file_family=",file_family
        print "width=",width
        print "pinfo=",pinfo
        print "p=",p

    t1 = time.time() #----------------------------------------------------Start
    if list>1:
        print "Requesting callback ports", "   cumt=",time.time()-t0

    # get a port to talk on and listen for connections
    host, port, listen_socket = callback.get_callback()
    listen_socket.listen(4)

    tinfo["get_callback"] = time.time() - t1 #------------------------------End
    if list>1:
        print " ",host,port
        print "  dt:",tinfo["get_callback"], "   cumt=",time.time()-t0

    if list>1:
        print "Calling Config Server to find",library[0]+".library_manager",\
              "   cumt=",time.time()-t0
    t1 = time.time() #----------------------------------------------------Start

    # ask configuration server what port library manager is using
    # note again:libraries have are identical since there is 1 output directory
    vticket = csc.get(library[0]+".library_manager")

    tinfo["get_libman"] = time.time() - t1 #--------------------------------End
    if list>1:
        print "  ",vticket["host"],vticket["port"]
        print "  dt:",tinfo["get_libman"], "   cumt=",time.time()-t0

    # loop on all input files sequentially
    for i in range(0,ninput):
        unique_id.append(0) # will be set later when submitted

        # delete old tickets in case of a retry
        try:
            del work_ticket
        except NameError:
            pass

        # allow some retries if mover fails
        retry = maxretry
        while retry:  # note that real rates are not correct in retries

            if list:
                print "Sending ticket to",library[i]+".library_manager",\
                      "   cumt=",time.time()-t0
            t1 = time.time() #----------------------------------------Lap Start

            unique_id[i] = time.time()  # note that this is down to mS
            uinfo["fullname"] = outputlist[i]

            # if old ticket exists, that means we are retrying
            #    then just bump priority and change unique id
            try:
                work_ticket["priority"] = workticket["priority"]+4
                work_ticket["unique_id"] = unique_id[i]

            # if no ticket, then this is a not a retry
            except NameError:
                work_ticket = {"work"               : "write_to_hsm",\
                              "delayed_dismount"   : delayed_dismount,\
                              "priority"           : 1,\
                              "library"            : library[i],\
                              "file_family"        : file_family[i],\
                              "file_family_width"  : width[i],\
                              "orig_filename"      : inputlist[i],\
                              "pnfs_info"          : pinfo[i],\
                              "user_info"          : uinfo,\
                              "mtime"              : int(time.time()),\
                              "size_bytes"         : file_size[i],\
                              "sanity_size"        : 5000,\
                              "user_callback_port" : port,\
                              "user_callback_host" : host,\
                              "unique_id"          : unique_id[i]
                              }

            # send the work ticket to the library manager
            tinfo["tot_to_send_ticket"+repr(i)] = t1 -t0
            system_enabled(p) # make sure system still enabled before submitting
            ticket = u.send(work_ticket, (vticket['host'], vticket['port']))
            if ticket['status'] != "ok" :
                jraise(errno.errorcode[errno.EPROTO]," encp.write_to_hsm: "\
                       "from u.send to " +library[i]+" at "\
                       +vticket['host']+"/"+repr(vticket['port'])\
                       +", ticket[\"status\"]="+ticket["status"])

            tinfo["send_ticket"+repr(i)] = time.time() - t1 #-----------Lap End
            if list:
                print "  Q'd:",inputlist[i], library[i],\
                      "family:",file_family[i],\
                      "bytes:", file_size[i],\
                      "dt:",tinfo["send_ticket"+repr(i)],\
                          "   cumt=",time.time()-t0

            if list>1:
                print "Waiting for mover to call back", \
                      "   cumt=",time.time()-t0
            t1 = time.time() #----------------------------------------Lap-Start
            tMBstart = t1

            # We have placed our work in the system and now we have to wait
            # for resources. All we need to do is wait for the system to call
            # us back, and make sure that is it calling _us_ back, and not
            # some sort of old call-back to this very same port. It is dicey
            # to time out, as it is probably legitimate to wait for hours....

            while 1 :
                control_socket, address = listen_socket.accept()
                ticket = callback.read_tcp_socket(control_socket,\
                             "encp write_to_hsm, mover call back")
                callback_id = ticket['unique_id']
                # compare strings not floats (floats fail comparisons)
                if str(unique_id[i])==str(callback_id):
                    break
                else:
                    print("encp write_to_hsm: imposter called us, trying again")
                    control_socket.close()

            # ok, we've been called back with a matched id - how's the status?
            if ticket["status"] != "ok" :
                jraise(errno.errorcode[errno.EPROTO]," encp.write_to_hsm: "\
                       +"1st (pre-file-send) mover callback on socket "\
                       +repr(address)+", failed to setup transfer: "\
                       +"ticket[\"status\"]="+ticket["status"],2)
            data_path_socket = callback.mover_callback_socket(ticket)

            tinfo["tot_to_mover_callback"+repr(i)] = time.time() - t0 #-----Cum
            dt = time.time() - t1 #-------------------------------------Lap-End
            if list>1:
                print " ",ticket["mover_callback_host"],\
                      ticket["mover_callback_port"],\
                      "cum:",tinfo["tot_to_mover_callback"+repr(i)]
                print "  dt:",dt,"   cumt=",time.time()-t0

            if list:
                print "Sending data for file ", outputlist[i],\
                      "   cumt=",time.time()-t0
            t1 = time.time() #----------------------------------------Lap-Start

            # Call back mover on mover's port and send file on that port
            in_file = open(inputlist[i], "r")
            mycrc = 0
            fsize = file_size[i]
            try:
                mycrc = EXfer.usrTo_(in_file,data_path_socket,binascii.crc_hqx,
                                     65536*4, chk_crc )
                retry = 0
            except:
                print "Error with encp EXfer - continuing";traceback.print_exc()
                retry = retry - 1
                data_path_socket.close()
                in_file.close()
                done_ticket = callback.read_tcp_socket(control_socket,
                                  "encp write_to_hsm, error dialog")
                control_socket.close()
                print done_ticket, "retrying"

        # close the data socket and the file, we've sent it to the mover
        data_path_socket.close()
        in_file.close()

        tinfo["sent_bytes"+repr(i)] = time.time()-t1 #------------------Lap-End
        if list>1:
            if tinfo["sent_bytes"+repr(i)]!=0:
                wtrate = 1.*fsize/1024./1024./tinfo["sent_bytes"+repr(i)]
            else:
                wdrate = 0.0
            print "  bytes:",fsize, " Socket Write Rate = ",wtrate," MB/s"
            print "  dt:",tinfo["sent_bytes"+repr(i)],\
                  "   cumt=",time.time()-t0
        if list>1:
            print "Waiting for final mover dialog",\
                  "   cumt=",time.time()-t0
        t1 = time.time() #--------------------------------------------Lap-Start

        # File has been sent - wait for final dialog with mover. We know
        # the file has hit some sort of media.... when this occurs. Create
        #  a file in pnfs namespace with information about transfer.
        done_ticket = callback.read_tcp_socket(control_socket,
                          "encp write_to_hsm, mover final dialog")
        control_socket.close()

        # make sure mover thinks transfer went ok
        if done_ticket["status"] != "ok" :
            jraise(errno.errorcode[errno.EPROTO]," encp.write_to_hsm: "\
                   +"2nd (post-file-send) mover callback on socket "\
                   +repr(address)+", failed to transfer: "\
                   +"done_ticket[\"status\"]="+done_ticket["status"])

        # Check the CRC
            if chk_crc != 0:
                if done_ticket["complete_crc"] != mycrc :
                    jraise(errno.errorcode[errno.EPROTO],\
                           " encp.write_to_hsm: CRC's mismatch: "\
                           +repr(complete_crc)+" "+repr(mycrc))

        tinfo["final_dialog"] = time.time()-t1 #------------------------Lap End
        if list>1:
            print "  dt:",tinfo["final_dialog"], "   cumt=",time.time()-t0

        if list>1:
            print "Adding file to pnfs", "   cumt=",time.time()-t0
        t1 = time.time() #--------------------------------------------Lap Start

        # create a new pnfs object pointing to current output file
        p=pnfs.pnfs(outputlist[i])
        # save the bfid and set the file size
        p.set_bit_file_id(done_ticket["bfid"],file_size[i])
        # create volume map and store cross reference data
        p.set_xreference(done_ticket["file_clerk"]["external_label"],
                         done_ticket["file_clerk"]["bof_space_cookie"])
        # store debugging info about transfer
        done_ticket["tinfo"] = tinfo # store as much as we can into pnfs
        done_formatted  = pprint.pformat(done_ticket)
        p.set_info(done_formatted)

        tinfo["pnfsupdate"] = time.time() - t1 #------------------------Lap End
        if list>1:
            print "  dt:",tinfo["pnfsupdate"], "   cumt=",time.time()-t0


        # calculate some kind of rate - time from beginning to wait for
        # mover to respond until now. This doesn't include the overheads
        # before this, so it isn't a correct rate. I'm assuming that the
        # overheads I've neglected are small so the quoted rate is close
        # to the right one.  In any event, I calculate an overall rate at
        # the end of all transfers
        tnow = time.time()
        if (tnow-tMBstart)!=0:
            tinfo['rate'+repr(i)] = 1.*fsize/1024./1024./(tnow-tMBstart)
        else:
            tinfo['rate'+repr(i)] = 0.0
        format = "  %s -> %s : %d bytes copied to %s at"+\
                 " %s MB/S  requestor:%s     cumt= %f"

        if list or ninput>1:
            print format %\
                  (inputlist[i], outputlist[i], fsize,\
                   done_ticket["external_label"],\
                   tinfo["rate"+repr(i)], uinfo["uname"],\
                   time.time()-t0)

        logc.send(log_client.INFO, 2, format,
                  inputlist[i], outputlist[i], fsize,
                  done_ticket["external_label"],
                  tinfo["rate"+repr(i)], uinfo["uname"],
                  time.time()-t0)


    # we are done transferring - close out the listen socket
    listen_socket.close()

    # Calculate an overall rate: all bytes, all time
    tf=tinfo["total"] = time.time()-t0
    done_ticket["tinfo"] = tinfo
    total_bytes = 0
    for i in range(0,ninput):
        total_bytes = total_bytes+file_size[i]
    tf = time.time()
    if tf!=t0:
        done_ticket["MB_per_S"] = 1.*total_bytes/1024./1024./(tf-t0)
    else:
        done_ticket["MB_per_S"] = 0.0

    if list or ninput>1:
        print "Complete: ",total_bytes," bytes in ",ninput," files",\
              " in",tf-t0,"S.  Overall rate = ",\
              done_ticket["MB_per_S"]," MB/s"

    # tell library manager we are done - this allows it to delete our unique id in
    # its dictionary - this keeps things cleaner and stops memory from growing
    #u.send_no_wait({"work":"done_cleanup"}, (vticket['host'], vticket['port']))

##############################################################################

def read_from_hsm(input, output, config_host, config_port,list, chk_crc) :
    t0 = time.time()
    tinfo = {}
    tinfo["abs_start"] = t0

    if list>2:
        print "Getting clients, storing/checking local info   cumt=",\
              time.time()-t0
    t1 = time.time() #----------------------------------------------------Start

    # initialize - and get config, udp and log clients
    finfo = []
    vinfo = []
    volume = []
    unique_id = []
    vols_needed = {}
    global logc
    (csc,u,uinfo) = clients(config_host,config_port,list)

    tinfo["clients"] = time.time() - t1 #-----------------------------------End
    if list>2:
        print "  dt:",tinfo["clients"], "   cumt=",time.time()-t0
    if list>3:
        print "csc=",csc
        print "u=",u
        print "logc=",logc
        print "uinfo=",uinfo

    if list>2:
        print "Checking input pnfs files:",input, "   cumt=",time.time()-t0
    t1 =  time.time() #---------------------------------------------------Start

    # check the input unix files. if files don't exits, we bomb out to the user
    (ninput, inputlist, file_size) = inputfile_check(input)
    (bfid,junk,junk,junk,pinfo,p)=pnfs_information(inputlist,ninput)

    tinfo["pnfscheck"] = time.time() - t1 #---------------------------------End
    if list>2:
        print "  dt:",tinfo["pnfscheck"], "   cumt=",time.time()-t0
    if list>3:
        print "ninput=",ninput
        print "inputlist=",inputlist
        print "file_size=",file_size
        print "bfid=",bfid
        print "pinfo=",pinfo
        print "p=",p

    if list>2:
        print "Checking output unix files:",output, "   cumt=",time.time()-t0
    t1 = time.time() #----------------------------------------------------Start

    # check (and generate) the output files(s)
    # bomb out if they exist already
    outputlist = outputfile_check(ninput,inputlist,output)

    tinfo["filecheck"] = time.time() - t1 #---------------------------------End
    if list>2:
        print "  dt:",tinfo["filecheck"], "   cumt=",time.time()-t0
    if list>3:
        print "outputlist=",outputlist

    if list>2:
        print "Requesting callback ports", "   cumt=",time.time()-t0
    t1 = time.time() #----------------------------------------------------Start

    # get a port to talk on and listen for connections
    host, port, listen_socket = callback.get_callback()
    listen_socket.listen(4)

    tinfo["get_callback"] = time.time() - t1 #------------------------------End
    if list>2:
        print " ",host,port
        print "  dt:",tinfo["get_callback"], "   cumt=",time.time()-t0

    if list>1:
        print "Calling Config Server to find file clerk",\
              "   cumt=",time.time()-t0
    t1 = time.time() #----------------------------------------------------Start

    # ask configuration server what port the file clerk is using
    fticket = csc.get("file_clerk")

    tinfo["get_fileclerk"] = time.time() - t1 #-----------------------------End
    if list>1:
        print " ",fticket["host"],fticket["port"]
        print "  dt:", tinfo["get_fileclerk"], "   cumt=",time.time()-t0

    if list>1:
        print "Calling file clerk for file info", "   cumt=",time.time()-t0
    t1 = time.time() # ---------------------------------------------------Start

    # call file clerk and get file info about each bfid
    for i in range(0,ninput):
        t2 = time.time() # -------------------------------------------Lap-Start
        unique_id.append(0) # will be set later when submitted
        binfo  = u.send({'work': 'bfid_info', 'bfid': bfid[i]},
                        (fticket['host'],fticket['port']))
        if binfo['status']!='ok':
            pprint.pprint(binfo)
            jraise(errno.errorcode[errno.EPROTO]," encp.read_from_hsm: "\
                   +" can not get info on bfid"+repr(bfid[i]))
        vinfo.append(binfo['volume_clerk'])
        finfo.append(binfo['file_clerk'])
        label = binfo['file_clerk']['external_label']
        volume.append(label)
        try:
            vols_needed[label] = vols_needed[label]+1
        except KeyError:
            vols_needed[label] = 1
        tinfo['fc'+repr(i)] = time.time() - t2 #------------------------Lap--End

    tinfo['file_clerk'] =  time.time() - t1 #-------------------------------End
    if list>1:
        print "  dt:",tinfo["file_clerk"], "   cumt=",time.time()-t0

    if list:
        print "Sending ticket to file clerk", "   cumt=",time.time()-t0
    t1 = time.time() #----------------------------------------------------Start

    # loop over all volumes that are needed and submit all requests for
    # that volume. Read files from each volume before submitting requests
    # for different volumes.

    for vol in vols_needed.keys():
        t2 = time.time() #--------------------------------------------Lap-Start
        submitted = 0
        Qd=""
        system_enabled(p) # make sure system is still enabled before submitting
        for i in range(0,ninput):
            if volume[i]==vol:
                unique_id[i] = time.time()  # note that this is down to mS
                uinfo["fullname"] = outputlist[i]

                # generate the work ticket
                work_ticket = {"work"               : "read_from_hsm",\
                               "user_info"          : uinfo,\
                               "pnfs_info"          : pinfo[i],\
                               "bfid"               : bfid[i],\
                               "sanity_size"        : 5000,\
                               "user_callback_port" : port,\
                               "user_callback_host" : host,\
                               "unique_id"          : unique_id[i]
                               }

                # send ticket to file clerk who sends it to right library manger
                ticket = u.send(work_ticket, (fticket['host'], fticket['port']))
                if ticket['status'] != "ok" :
                    jraise(errno.errorcode[errno.EPROTO],\
                           " encp.read_from_hsm: from"\
                           +"u.send to file_clerk at "+fticket['host']+"/"\
                           +repr(fticket['port']) +", ticket[\"status\"]="\
                           +ticket["status"])
                submitted = submitted+1
                tinfo["send_ticket"+repr(i)] = time.time() - t2 #------Lap-End
                if list :
                    if len(Qd)==0:
                        format = "  Q'd: %s %s bytes: %d on %s %s "\
                                 "dt: %f   cumt=%f"
                        Qd = format %\
                             (inputlist[i],bfid[i],file_size[i],\
                              finfo[i]["external_label"],\
                              finfo[i]["bof_space_cookie"],\
                              tinfo["send_ticket"+repr(i)],time.time()-t0)
                    else:
                        Qd = "%s\n  Q'd: %s %s bytes: %d on %s %s "\
                             "dt: %f   cumt=%f" %\
                             (Qd,inputlist[i],bfid[i],file_size[i],\
                              finfo[i]["external_label"],\
                              finfo[i]["bof_space_cookie"],\
                              tinfo["send_ticket"+repr(i)],time.time()-t0)

        tinfo["send_ticket"] = time.time() - t1 #---------------------------End
        if list:
            print Qd
        if list>1:
            print "  dt:",tinfo["send_ticket"], "   cumt=",time.time()-t0

        # We have placed our work in the system and now we have to wait for
        # resources. All we need to do is wait for the system to call us
        # back, and make sure that is it calling _us_ back, and not some
        # sort of old call-back to this very same port. It is dicey to time
        # out, as it is probably legitimate to wait for hours....

        for waiting in range(0,submitted):
            if list>1:
                print "Waiting for mover to call back",\
                      "   cumt=",time.time()-t0
            t2 = time.time() #----------------------------------------Lap-Start
            tMBstart = t2

            # listen for a mover - see if id corresponds to one of the tickets
            #   we submitted for the volume
            while 1 :
                control_socket, address = listen_socket.accept()
                ticket = callback.read_tcp_socket(control_socket,\
                             "encp read_from_hsm, mover call back")
                callback_id = ticket['unique_id']
                forus = 0
                for j in range(0,ninput):
                    # compare strings not floats (floats fail comparisons)
                    if str(unique_id[j])==str(callback_id):
                        forus = 1
                        break
                if forus==1:
                    break
                else:
                    print ("encp read_from_hsm: imposter called us back, "\
                           +"trying again")
                    control_socket.close()

            # ok, we've been called back with a matched id - how's the status?
            if ticket["status"] != "ok" :
                jraise(errno.errorcode[errno.EPROTO]," encp.read_from_hsm: "\
                       +"1st (pre-file-read) mover callback on socket "\
                       +repr(address)+", failed to setup transfer: "\
                       +"ticket[\"status\"]="+ticket["status"])
            data_path_socket = callback.mover_callback_socket(ticket)

            tinfo["tot_to_mover_callback"+repr(j)] = time.time() - t0 #-----Cum
            dt = time.time() - t2 #-------------------------------------Lap-End
            if list>1:
                print " ",ticket["mover_callback_host"],\
                      ticket["mover_callback_port"],\
                      "cum:",tinfo["tot_to_mover_callback"+repr(j)]
                print "  dt:",dt,"   cumt=",time.time()-t0

            if list:
                print "Receiving data for file ", outputlist[j],\
                      "   cumt=",time.time()-t0
            t2 = time.time() #----------------------------------------Lap-Start

            # open file that corresponds to the mover call back and read file
            # crc the data if user has request crc check
            l = 0
            mycrc = 0
            f = open(outputlist[j],"w")
            while 1:
                buf = data_path_socket.recv(65536*4)
                l = l + len(buf)
                if len(buf) == 0 : break
                if chk_crc != 0 :
                    mycrc = binascii.crc_hqx(buf,mycrc)
                f.write(buf)
            data_path_socket.close()
            f.close()
            fsize = l

            tinfo["recvd_bytes"+repr(j)] = time.time()-t2 #-------------Lap-End
            if list>1:
                if tinfo["recvd_bytes"+repr(j)]!=0:
                    rdrate = 1.*fsize/1024./1024./tinfo["recvd_bytes"+repr(j)]
                else:
                    rdrate = 0.0
                print "  bytes:",fsize, " Socket read Rate = ",rdrate," MB/s"
                print "  dt:",tinfo["recvd_bytes"+repr(j)],\
                      "   cumt=",time.time()-t0
            if list>1:
                print "Waiting for final mover dialog",\
                      "   cumt=",time.time()-t0
            t2 = time.time() #----------------------------------------Lap-Start

            # File has been read - wait for final dialog with mover.
            done_ticket = callback.read_tcp_socket(control_socket,\
                          "encp read_from_hsm, mover final dialog")
            control_socket.close()

            # make sure the mover thinks the transfer went ok
            if done_ticket["status"] != "ok" :
                jraise(errno.errorcode[errno.EPROTO]," encp.read_from_hsm: "\
                       +"2nd (post-file-read) mover callback on socket "\
                       +repr(address)+", failed to transfer: "\
                       +"done_ticket[\"status\"]="+done_ticket["status"])

            # verify that the crc's match
            if chk_crc != 0 :
                if done_ticket["complete_crc"] != mycrc :
                    jraise(errno.errorcode[errno.EPROTO],\
                           " encp.read_from_hsm: CRC's mismatch: "\
                           +repr(complete_crc)+" "+repr(mycrc))

            tinfo["final_dialog"+repr(j)] = time.time()-t2 #------------Lap-End
            if list>1:
                print "  dt:",tinfo["final_dialog"+repr(j)],\
                "   cumt=",time.time()-t0


            # update the last parked info if we have write access
            if 0:
                if list>1:
                    print "Updating pnfs last parked",\
                          "   cumt=",time.time()-t0
                try:
                    p.set_lastparked(repr(uinfo['fullname']))
                except:
                    print "Failed to update last parked info"
                if list>1:
                    print "  dt:",tinfo["last_parked"],\
                          "   cumt=",time.time()-t0

            # calculate some kind of rate - time from beginning to wait for
            # mover to respond until now. This doesn't include the overheads
            # before this, so it isn't a correct rate. I'm assuming that the
            # overheads I've neglected are small so the quoted rate is close
            # to the right one.  In any event, I calculate an overall rate at
            # the end of all transfers
            tnow = time.time()
            if (tnow-tMBstart)!=0:
                tinfo['rate'+repr(j)] = 1.*fsize/1024./1024./(tnow-tMBstart)
            else:
                tinfo['rate'+repr(j)] = 0.0
            format = "  %s -> %s : %d bytes copied from %s at"+\
                     " %s MB/S  requestor:%s     cumt= %f"

            if list or ninput>1:
                print format %\
                      (inputlist[j], outputlist[j], fsize,\
                       done_ticket["external_label"],\
                       tinfo["rate"+repr(j)], uinfo["uname"],\
                       time.time()-t0)

            logc.send(log_client.INFO, 2, format,
                      inputlist[j], outputlist[j], fsize,
                      done_ticket["external_label"],
                      tinfo["rate"+repr(j)], uinfo["uname"],
                      time.time()-t0)


    # we are done transferring - close out the listen socket
    listen_socket.close()

    # Calculate an overall rate: all bytes, all time
    tf=tinfo["total"] = time.time()-t0
    done_ticket["tinfo"] = tinfo
    total_bytes = 0
    for i in range(0,ninput):
        total_bytes = total_bytes+file_size[i]
    tf = time.time()
    if tf!=t0:
        done_ticket["MB_per_S"] = 1.*total_bytes/1024./1024./(tf-t0)
    else:
        done_ticket["MB_per_S"] = 0.0

    if list or ninput>1:
        print "Complete: ",total_bytes," bytes in ",ninput," files",\
              " in",tf-t0,"S.  Overall rate = ",\
              done_ticket["MB_per_S"]," MB/s"

    # tell file clerk we are done - this allows it to delete our unique id in
    # its dictionary - this keeps things cleaner and stops memory from growing
    #u.send_no_wait({"work":"done_cleanup"}, (fticket['host'], fticket['port']))

##############################################################################

# log the error to the logger, print it to the console and exit

def jraise(errcode,errmsg,exit_code=1) :
    format = "Fatal error:"+str(errcode)+str(errmsg)
    print format
    global logc
    logc.send(log_client.ERROR, 1, format)
    sys.exit(exit_code)

##############################################################################

# get the configuration client and udp client and logger client
# return some information about who we are so it can be used in the ticket

def clients(config_host,config_port,list):

    if list>3 :
        print "Connecting to configuration server at ",config_host,config_port

    # get a configuration server
    csc = configuration_client.configuration_client(config_host,config_port)
    csc.connect()

    # get a udp client
    u = udp_client.UDPClient()

    # get a logger client
    global logc
    logc = log_client.LoggerClient(csc, 'ENCP', 'logserver', 0)

    uinfo = {}
    uinfo['uid'] = os.getuid()
    uinfo['gid'] = os.getgid()
    uinfo['gname'] = grp.getgrgid(uinfo['gid'])[0]
    uinfo['uname'] = pwd.getpwuid(uinfo['uid'])[0]
    uinfo['machine'] = os.uname()
    uinfo['fullname'] = "" # will be filled in later for each transfer

    return (csc,u,uinfo)

##############################################################################

# check if the system is still running by checking the wormhole file

def system_enabled(p):                 # p is a  pnfs object
    running = p.check_pnfs_enabled()
    if running != pnfs.enabled :
        jraise(errno.errorcode[errno.EACCES]," encp.system_enabeld: "\
               +"system disabled"+running)

##############################################################################

# return pnfs information,
# and an open pnfs object so you can check if  the system is enabled.

def pnfs_information(filelist,nfiles):

    bfid = []
    pinfo = []
    library = []
    file_family = []
    width = []

    for i in range(0,nfiles):
        p = pnfs.pnfs(filelist[i])         # get the pnfs object
        bfid.append(p.bit_file_id)         # get the bit file id
        library.append(p.library)          # get the library
        file_family.append(p.file_family)  # get the file family
        width.append(p.file_family_width)  # get the width

        # get some debugging info for the ticket
        pinf = {}
        for k in [ 'pnfsFilename','gid', 'gname','uid', 'uname',\
                   'major','minor','rmajor','rminor',\
                   'mode','pstat' ] :
            exec("pinf["+repr(k)+"] = p."+k)
        pinfo.append(pinf)

    return (bfid,library,file_family,width,pinfo,p)

##############################################################################

# generate the full path name to the file

def fullpath(filename):
    machine = socket.gethostbyaddr(socket.gethostname())[0]
    dir, file = os.path.split(filename)

    # if the directory is empty - get the users current working directory
    if dir == '' :
        dir = os.getcwd()
    command="(cd "+dir+";pwd)"
    try:
        dir = regsub.sub("\012","",os.popen(command,'r').readlines()[0])
        filename = dir+"/"+file
    except:
        pass

    # take care of any inadvertant extra "/"
    # Note: as far as I know, this only happens when the user specifies the
    #       the filename as /
    filename = regsub.sub("//","/",filename)
    dir = regsub.sub("//","/",dir)
    file = regsub.sub("//","/",file)

    return (machine, filename, dir, file)

##############################################################################

# check the input file list for consistency

def inputfile_check(input):

    # create internal list of input unix files even if just 1 file passed in
    try:
        ninput = len(input)
        inputlist = input
    except TypeError:
        inputlist = [input]
        ninput = 1

    # we need to know how big each input file is
    file_size = []

    # check the input unix file. if files don't exits, we bomb out to the user
    for i in range(0,ninput):

        # get fully qualified name
        (machine, fullname, dir, basename) = fullpath(inputlist[i])
        inputlist[i] = dir+'/'+basename

        # input files must exist
        command="if test -r "+inputlist[i]+"; then echo ok; else echo no; fi"
        readable = os.popen(command,'r').readlines()
        if "ok\012" != readable[0] :
            jraise(errno.errorcode[errno.EACCES]," encp.inputfile_check: "\
                   +inputlist[i]+", NO read access to file")

        # get the file size
        statinfo = os.stat(inputlist[i])
        file_size.append(statinfo[stat.ST_SIZE])

        # input files can't be directories
        if not stat.S_ISREG(statinfo[stat.ST_MODE]) :
            jraise(errno.errorcode[errno.EPROTO]," encp.inputfile_check: "\
                   +input[i]+" is not a regular file")

    return (ninput, inputlist, file_size)

##############################################################################

# check the output file list for consistency
# generate names based on input list if required

def outputfile_check(ninput,inputlist,output):

    # can only handle 1 input file  copied to 1 output file
    #  or      multiple input files copied to 1 output directory
    # this is just the current policy - nothing fundamental about it
    try:
        noutput = len(output)
        jraise(errno.errorcode[errno.EPROTO]," encp.outputfile_check: "\
               +"can not handle multiple output files: "+output)
    except TypeError:
        pass

    # if user specified multiple input files, then output must be a directory
    outputlist = []
    if ninput!=1:
        try:
            statinfo = os.stat(output[0])
        except os.error:
            jraise(errno.errorcode[errno.EPROTO]," encp.outputfile_check: "\
                   "multiple input files can not be copied to non-existant "\
                   +"directory "+output[0])
        if not stat.S_ISDIR(statinfo[stat.ST_MODE]) :
            jraise(errno.errorcode[errno.EPROTO]," encp.outputfile_check: "\
                   "multiple input files must be copied to a directory, not "\
                   +output[0])

    outputlist = []

    # Make sure we can open the files. If we can't, we bomb out to user
    # loop over all input files and generate full output file names
    for i in range(0,ninput):
        outputlist.append(output[0])

        # see if output file exists as user specified
        try:
            statinfo = os.stat(outputlist[i])
            itexists = 1

        # if output doesn't exist, then at least directory must exist
        except os.error:
            itexists = 0
            (omachine, ofullname, odir, obasename) = fullpath(outputlist[i])
            try:
                statinfo = os.stat(odir)
            # directory doesn't exist - error
            except os.error:
                jraise(errno.errorcode[errno.EEXIST]," encp.outputfile_check:"\
                       " base directory doesn't exist for "+outputlist[i])

        # note: removed from itexist=1 try block to isolate errors
        if itexists:
            # if output file exists, then it must be a directory
            if stat.S_ISDIR(statinfo[stat.ST_MODE]) :
                (omachine, ofullname, odir, obasename) = fullpath(outputlist[i])
                (imachine, ifullname, idir, ibasename) = fullpath(inputlist[i])
                # take care of missing filenames (just directory or .)
                if obasename=='.' or len(obasename)==0:
                    outputlist[i] = odir+'/'+ibasename
                else:
                    outputlist[i] = ofullname+'/'+ibasename
                (omachine, ofullname, odir, obasename) = fullpath(outputlist[i])
                # need to make sure generated filename doesn't exist
                try:
                    statinfo = os.stat(outputlist[i])
                    # generated filename already exists - error
                    jraise(errno.errorcode[errno.EEXIST],\
                           " encp.outputfile_check: "+outputlist[i]+\
                           " already exists")
                except os.error:
                    pass # ok, generated name doesn't exist
            # filename already exists - error
            else:
                jraise(errno.errorcode[errno.EEXIST]," encp.outputfile_check: "\
                       +outputlist[i]+" already exists")

        # need to check that directory is writable
        # since all files go to one output directory, one check is enough
        if i==0:
            command="if test -w "+odir+"; then echo ok; else echo no; fi"
            writable = os.popen(command,'r').readlines()
            if "ok\012" != writable[0] :
                jraise(errno.errorcode[errno.EACCES]," encp.write_to_hsm: "\
                       +" NO write access to directory"+odir)

    return outputlist

##############################################################################

if __name__  ==  "__main__" :
    import getopt
    import string

    # defaults
    #config_host = "localhost"
    (config_host,ca,ci) = socket.gethostbyaddr(socket.gethostname())
    config_port = "7500"
    config_list = 0
    list = 0
    chk_crc = 1

    # see what the user has specified. bomb out if wrong options specified
    options = ["config_host=","config_port=","config_list",\
               "nocrc","list","verbose=","help"]
    optlist,args=getopt.getopt(sys.argv[1:],'',options)
    for (opt,value) in optlist :
        if opt == "--config_host" :
            config_host = value
        elif opt == "--config_port" :
            config_port = value
        elif opt == "--config_list" :
            config_list = 1
        elif opt == "--nocrc":
            chk_crc = 0
        elif opt == "--list":
            list = 1
        elif opt == "--verbose":
            list = string.atoi(value)
        elif opt == "--help" :
            print "python", sys.argv[0], options, "inputfilename outputfilename"
            print "   do not forget the '--' in front of each option"
            sys.exit(0)

    # bomb out if can't translate host
    ip = socket.gethostbyname(config_host)

    # bomb out if port isn't numeric
    config_port = string.atoi(config_port)

    # bomb out if we don't have an input and an output
    arglen = len(args)
    if arglen < 2 :
        print "python",sys.argv[0], options, "inputfilename outputfilename"
        print "-or-"
        print "python",sys.argv[0], options, "inputfilename1 ... inputfilenameN outputdirectory"
        print "   do not forget the '--' in front of each option"
        sys.exit(1)

    # get fullpaths to the files
    p = []
    for i in range(0,arglen):
        (machine, fullname, dir, basename) = fullpath(args[i])
        args[i] = dir+'/'+basename
        p.append(string.find(dir,"/pnfs/"))

    # all files on the hsm system have /pnfs/ as the 1st part of their name
    # scan input files for /pnfs - all have to be the same
    p1 = p[0]
    p2 = p[arglen-1]
    input = [args[0]]
    output = [args[arglen-1]]
    for i in range(1,len(args)-1):
        if p[i]!=p1:
            if p1:
                print "ERROR: Not all input files are /pnfs/... files"
            else:
                print "ERROR: Not all input files are unix files"
            sys.exit(1)
        else:
            input.append(args[i])

    # have we been called "encp unixfile hsmfile" ?
    if p1==-1 and p2==0 :
        write_to_hsm(input,  output, config_host, config_port, list, chk_crc)

    # have we been called "encp hsmfile unixfile" ?
    elif p1==0 and p2==-1 :
        read_from_hsm(input, output, config_host, config_port, list, chk_crc)

    # have we been called "encp unixfile unixfile" ?
    elif p1==-1 and p2==-1 :
        print "encp copies to/from hsm. It is not involved in copying "\
              +input," to ",output

    # have we been called "encp hsmfile hsmfile?
    elif p1==0 and p2==0 :
        print "encp hsm to hsm is not functional. "\
              +"copy hsmfile to local disk and them back to hsm"

    else:
        print "ERROR: Can not process arguments "+args

