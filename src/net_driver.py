#!/usr/bin/env python

# $Id$

import select
import socket
import time
import errno

import setpath
import driver
import e_errors
import strbuffer

verbose=1

class NetDriver(driver.Driver):

    def __init__(self):
        self.sock = -1
        self._bytes_transferred = 0
        self._start_time = None
        self._rate = self._last_rate = 0
        
    def fdopen(self, sock):
        self.sock = sock
        size = 1024*1024
        for opt in (socket.SO_RCVBUF, socket.SO_SNDBUF):
            try:
                sock.setsockopt(socket.SOL_SOCKET, opt, size)
                Trace.log(e_errors.INFO, "tcp buffer size %s %s" % (
                    opt, sock.getsockopt(socket.SOL_SOCKET, opt)))
            except socket.error, msg:
                Trace.log(e_errors.ERROR, "setting tcp buffer size:  %s %s %s" % (
                    opt, size, msg))

        self._last_rate = 0
        self._rate = 0
        self._bytes_transferred = 0            
        return self.sock
        
    def fileno(self):
        return self.sock.fileno()
    
    def close(self):
        r = self.sock.close()
        self.sock = -1
        return r

    def read(self, buf, offset, nbytes):
        t0 = time.time()
        r =  strbuffer.buf_recv(self.fileno(), buf, offset, nbytes)
        
        if r<0 and strbuffer.cvar.errno == errno.EAGAIN:
            r=0
            
        if r > 0:
            now = time.time()
            self._last_rate = r/(now-t0)
            if self._bytes_transferred == 0:
                self._start_time = t0
            self._bytes_transferred = self._bytes_transferred + r
            self._rate = self._bytes_transferred/(now - self._start_time)
        if r <= 0:
            pass #raise exception?
        return r
                                  
    def write(self, buf, offset, nbytes):
        t0 = time.time()
        r = strbuffer.buf_send_dontwait(self.fileno(), buf, offset, nbytes)
        if r<0 and strbuffer.cvar.errno == errno.EAGAIN:
            r = 0
        if r > 0:
            now = time.time()
            self._last_rate = r/(now - t0)
            if self._bytes_transferred == 0:
                self._start_time = t0
            self._bytes_transferred = self._bytes_transferred + r
            self._rate = self._bytes_transferred/(now - self._start_time)
        return r
        
    def rates(self):
        """returns a tuple (overall rate, instantaneous rate)"""
        return self._rate, self._last_rate
        
    def ready_to_read(self):
        r,w,x = select.select([self], [], [], 0)
        return r

    def ready_to_write(self):
        r,w,x = select.select([], [self],  [], 0)
        return w
    
            
        
