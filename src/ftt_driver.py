#!/usr/bin/env python

# $Id$

import time
import select
import string
import exceptions

import Trace
import driver
import e_errors
import ftt

KB=1024
MB=KB*KB
GB=MB*KB


class FTTDriver(driver.Driver):
    mount_delay = 15
    def __init__(self):
        self.fd = -1
        self.ftt = None
        self._bytes_transferred = 0
        self._start_time = None
        self._total_time = 0
        self._rate = self._last_rate = 0
        
    def open(self, device=None, mode=None, retry_count=10):
        """Open will return 1 if there's a volume, 0 if there is no volume
        but otherwise OK, -1 or exception on errors"""
        if mode not in (0,1):
            raise ValueError, ("illegal mode", mode)

        self.device = device
        if self.ftt and mode != self.mode:
            self.ftt.close()
            self.ftt = None

        if not self.ftt:
            self.ftt = ftt.open(
                self.device,
                ftt.RDWR) ##Note: always open r/w since mode-switching causes
                          ## loss of location information XXX
            ###{0:ftt.RDONLY, 1:ftt.RDWR}[mode])

        self.mode = mode
        self._last_rate = 0
        self._rate = 0
        self._bytes_transferred = 0
        Trace.trace(25, "ftt_open returns %s" % (self.ftt,))

        self._open_dev(retry_count)
            
        if self.fd is None:
            return -1 #or exception?

        Trace.trace(25, "ftt_open_dev returns %s" % (self.fd,))
        
        for retry in xrange(retry_count):
            if retry:
                Trace.trace(25, "retrying status %d" % retry)
            status = self.ftt.status(5)
            Trace.trace(25, "ftt status returns %s"%(status,))
            if status & ftt.ONLINE:
                break
            Trace.trace(25, "closing ftt device to force status update")
            self.ftt.close_dev()
            self._open_dev(2)
        else:
            return 0 #this is BADSWMOUNT
        
        self._rate = self._last_rate = self._bytes_transferred = 0

        return 1
    
    def _open_dev(self, retry_count):
        self.fd = None
        for retry in xrange(retry_count):
            if retry:
                Trace.trace(25, "retrying open %s"%(retry,))
            try:
                self.fd = self.ftt.open_dev()
                break
            except ftt.FTTError, detail:
                Trace.log(e_errors.ERROR, "ftt open dev: %s %s" %(detail, detail.value))
                if detail.errno == ftt.EBUSY:
                    time.sleep(5)
                elif detail.errno == ftt.EROFS:
                    ###XXX HACK!  Tape may have write-protect tab set.  But we really
                    ### ought to get readonly status of the tape from the volume database
                    Trace.log(e_errors.INFO, "ftt open dev: %s %s: reopening read-only" %(detail, detail.value))
                    self.ftt.close()
                    self.ftt = ftt.open(self.device, ftt.RDONLY)
                else:
                    break
                
    def rewind(self):
        try:
            r = self.ftt.rewind()
            Trace.trace(25, "ftt_rewind returns %s" % (r,))
            return r
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "rewind: %s %s" % (detail, detail.value))
            return -1

    def tell(self):
        if not self.ftt:
            Trace.log(e_errors.ERROR, "tell: no ftt descriptor")
            return None
        try:
            file, block = self.ftt.get_position()
            Trace.trace(25, "ftt_get_position returns %s %s" % (file, block))
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "tell: %s %s" % (detail, detail.value))
            return -1
        return file
    
    def seek(self, target, eot_ok=0): #XXX is eot_ok needed?
        if type(target)==type(""):
            target = long(target)
        try:
            file, block = self.ftt.get_position()
        except ftt.FTTError, detail: 
            if detail.errno == ftt.ELOST: 
                self.rewind() #don't know tape position, must rewind
            else:
                raise ftt.FTTError, detail #some other FTT error

        file, block = self.ftt.get_position()
        if block==0 and file == target:
            return 0
        else:
            Trace.trace(25,"seek: current = %s,%s target=%s" %(file, block, target))
        current = file
        if target>current:
            try:
                self.ftt.skip_fm(target-current)
            except ftt.FTTError, detail:
                if detail.errno == ftt.EBLANK and eot_ok: ##XXX is eot_ok needed?
                    ### XXX Damn, this is unrecoverable (for AIT2, at least). What to do?
                    pass
                else:
                    Trace.log(e_errors.ERROR, "seek: %s %s" % (detail, detail.value))
                    raise ftt.FTTError, detail
        else:
            try:
                self.ftt.skip_fm(target-current-1)
                self.ftt.skip_fm(1)
            except ftt.FTTError, detail:
                Trace.log(e_errors.ERROR, "skip_fm: %s %s" % (detail, detail.value))
                raise ftt.FTTError, detail
        current = self.tell()
        Trace.trace(25,"seek2: current=%s target=%s" % (current, target))
        if current != target:
            raise "XXX Positioning error", (current, target)
        
    def fileno(self):
        return self.fd

    def flush(self):
        Trace.trace(25, "flushing %s" % (self.ftt))
        now=time.time()
        Trace.trace(25, "transferred %s bytes in %s seconds"%(
            self._bytes_transferred, now-self._start_time))
        if now>self._start_time and self._bytes_transferred:
            Trace.trace(25,  "rate: %.3g MB/sec" % (self._bytes_transferred/(now-self._start_time)/MB))
        try:
            r = self.ftt.close_dev()
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "close_dev %s %s" % (detail, detail.value))
        Trace.trace(25, "ftt_close_dev returns %s" % (r,))
        self.fd = -1
        return r

    def close(self):
        Trace.trace(25, "closing %s" % (self.ftt,))
        try:
            r = self.ftt.close()
            Trace.trace(25, "ftt_close returns %s" % (r,))
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "close %s %s" % (detail, detail.value))
            r = -1
        self.ftt = None
        self.fd = -1
        return r

    def read(self, buf, offset, nbytes):
##        if self.mode != 0:
##            raise ValueError, "file not open for reading"
        if offset != 0:
            raise ValueError, "offset must be 0"
        t0 = time.time()
        try:
            r = self.ftt.read(buf, nbytes)
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "read %s %s" % (detail, detail.value))
            raise e_errors.READ_ERROR, detail
        if r > 0:
            now = time.time()
            self._last_rate = r/(now-t0)
            if self._bytes_transferred == 0:
                self._start_time = t0
            self._bytes_transferred = self._bytes_transferred + r
            self._rate = self._bytes_transferred/(now - self._start_time)
        return r
    
    def write(self, buf, offset, nbytes):
        if self.mode != 1:
            raise ValueError, "file not open for writing"
        if offset != 0:
            raise ValueError, "offset must be 0"
        t0 = time.time()
        try:
            r = self.ftt.write(buf, nbytes)
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "write %s %s" % (detail, detail.value))
            raise e_errors.WRITE_ERROR, detail
        if r > 0:
            now = time.time()
            self._last_rate = r/(now - t0)
            if self._bytes_transferred == 0:
                self._start_time = t0
            self._bytes_transferred = self._bytes_transferred + r
            self._rate = self._bytes_transferred/(now - self._start_time)
        return r
        
    def writefm(self):
        r=0
        try:
            r = self.ftt.writefm()
            #### XXX Hack! Avert your eyes, innocent ones!
            ## We don't want a subsequent "close" to write extra filemarks.
            ## ftt_close_dev is being too helpful in the case where the last operation
            ## was a writefm.  So we tell a lie to ftt...
            ftt._ftt.ftt_set_last_operation(self.ftt.d, 0)
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "write %s %s" % (detail, detail.value))
        if r==-1:
            ftt.raise_ftt()
        return r

    def eject(self):
        try:
            return self.ftt.unload()
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "eject %s %s" % ( detail, detail.value))
            return -1
        
    def set_mode(self, density=None, compression=None, blocksize=None):
        ##HACK: this seems to trigger a core dump in ftt, and it's
        ## not clear we're really changing the mode anyhow.
        ## XXX investigate this!
        return 0
    
        r = -1
        try:
            mode = self.ftt.get_mode()
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "get_mode %s %s" % (detail, detail.value))
            return -1
        
        if density is None:
            density = mode[1]
        if compression is None:
            compression = mode[2]
        if blocksize is None:
            blocksize = mode[3]

        try:
            r = self.ftt.set_mode(density, compression, blocksize)
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "set_mode %s %s" % (detail, detail.value))
            return -1

        try:
            self.fd = self.ftt.open_dev()
        except ftt.FTTError, detail:
            Trace.log(e_errors.ERROR, "open_dev %s %s" % (detail, detail.value))
            return -1

        return r

    def verify_label(self, volume_label=None, mode=0):
        
        buf=80*' '
        try:
            Trace.log(25, "rewinding tape to check volume label")
            
            self.rewind()
            self.set_mode(compression = 0, blocksize = 0)

            if self.fd is None:
                return {0:e_errors.READ_BADSWMOUNT, 1:e_errors.WRITE_BADSWMOUNT}[mode], None
            try:
                nbytes=self.read(buf, 0, 80)
            except e_errors.READ_ERROR, detail:
                nbytes = 0
            if nbytes != 80:
                Trace.trace(25, "read %s bytes checking label" % nbytes)
                return {0:e_errors.READ_VOL1_READ_ERR, 1:e_errors.WRITE_VOL1_READ_ERR}[mode], None
            Trace.trace(25, "verify_label: read %s" % buf)
            if buf[:4] != "VOL1":
                return {0:e_errors.READ_VOL1_MISSING, 1:e_errors.WRITE_VOL1_MISSING}[mode], None
            s = string.split(buf[4:])
            if not s:
                return {0:e_errors.READ_VOL1_MISSING, 1:e_errors.WRITE_VOL1_MISSING}[mode], None
            if volume_label is None:
                return e_errors.OK, s[0]
            if s[0] != volume_label:
                return {0:e_errors.READ_VOL1_WRONG, 1:e_errors.WRITE_VOL1_WRONG}[mode], s[0]

            return e_errors.OK, None
        except exceptions.Exception, detail:
            Trace.log(e_errors.ERROR, "reading VOL1 label: %s" % (detail,))
            return {0:e_errors.READ_VOL1_READ_ERR, 1:e_errors.WRITE_VOL1_READ_ERR}[mode], str(detail)
        
    def rates(self):
        """returns a tuple (overall rate, instantaneous rate)"""
        return self._rate, self._last_rate
    
    
        
if __name__ == '__main__':

    print "TEST ME!"
    

            

    
