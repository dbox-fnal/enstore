#!/usr/bin/env python
###############################################################################
#
# $Author$
# $Date$
# $Id$
#
# Histogram class (equal binning)
# Author: Dmitry Litvintsev (litvinse@fnal.gov) 08/05
#
###############################################################################

import random
import math
import sys
import os
import time

class Histogram1D:

    def __init__(self, name, title, nbins=10, xlow=0, xhigh=1):
        self.name=name
        self.title=title
        self.nbins=int(nbins)
        self.low=xlow
        self.high=xhigh
        self.entries=0
        self.binarray = []
        self.sumarray = []
        self.underflow=0
        self.overflow=0
        self.mean=0
        self.mean_error=0
        self.rms=0
        self.rms_error=0
        self.sum=0
        self.sum2=0
        self.data_file_name="%s_data.pts"%(self.name,)
        self.bw=(self.high-self.low)/float(self.nbins)
        self.maximum=0
        self.minimum=0
        self.time_axis=False # time axis assumes unix time stamp
        self.profile=False
        #
        # atributes
        #
        self.ylabel=""
        self.xlabel=""
        self.logy=False
        self.logx=False

        for i in range(self.nbins):
            self.binarray.append(0.)
            self.sumarray.append(0.)
    #
    # non trivial methods 
    #

    def find_bin(self,x):
        if ( x < self.low ):
            self.underflow=self.underflow+1
            return None
        elif ( x > self.high ):
            self.overflow=self.overflow+1
            return None
        bin = int (float(self.nbins)*(x-self.low)/(self.high-self.low));
        if ( bin == self.nbins ) :
            bin = bin-1
        return bin

    def fill(self,x,w=1.):
        bin = self.find_bin(x)
        if bin:
            self.sum=self.sum+x
            self.sum2=self.sum2+x*x
            self.entries=self.entries+1
            if ( self.profile ) :
                sum=self.sumarray[bin]
                sum=sum+1
                self.sumarray[bin]=sum
            count=self.binarray[bin]
            count=count+1.*w
            self.binarray[bin]=count
            self.mean=self.sum/float(self.entries)
            rms2=self.sum2-2.*self.sum*self.mean+float(self.entries)*self.mean*self.mean
            self.rms=math.sqrt(rms2/float(self.entries))
            self.mean_error=self.rms/math.sqrt(float(self.entries))
            self.rms_error=self.rms/math.sqrt(2.*float(self.entries))
            if ( count > self.maximum ) :
                self.maximum=count
            if ( count < self.minimum ) :
                self.minimum=count
    #
    # setters 
    #
                
    def set_title(self,txt):
        self.title=txt

    def set_name(self,txt):
        self.name=txt

    def set_data_file_name(self,txt):   # name of the file with data points
        self.data_file_name=txt

    #
    # Plotting features
    #

    def set_ylabel(self,txt):
        self.ylabel=txt

    def set_xlabel(self,txt):
        self.xlabel=txt

    def set_logy(self,yes=True):
        self.logy=yes

    def set_logx(self,yes=True):
        self.logx=yes

    def set_time_axis(self,yes=True):
        self.time_axis=yes

    def set_profile(self,yes=True):
        self.profile=yes

    #
    # getters
    #

    def n_entries(self):
        return self.entries

    def n_bins(self):
        return self.nbins

    def axis_low(self):
        return self.low
    
    def axis_high(self):
        return self.high

    def get_bin_content(self,bin):
        if ( bin >= self.nbins or bin < 0 ) :
            return None
        return self.binarray[bin]

    def get_bin_center(self,bin):
        if ( bin >= self.nbins or bin < 0 ) :
            return None
        return self.low+(bin+0.5)*(self.high-self.low)/float(self.nbins)

    def get_bin_width(self,bin):
        return self.bw

    def get_bin_low_edge(self,bin):
        if ( bin >= self.nbins or bin < 0 ) :
            return None
        return self.low+float(bin)*(self.high-self.low)/float(self.nbins)

    def get_bin_high_edge(self,bin):
        if ( bin >= self.nbins or bin < 0 ) :
            return None
        return self.low+(bin+1.)*(self.high-self.low)/float(self.nbins)

    #
    # Statistis
    #

    def get_mean(self):
        return self.mean

    def get_mean_error(self):
        return self.mean_error

    def get_rms(self):
        return self.rms

    def get_rms_error(self):
        return self.rms_error

    def get_maximum(self): # maximum bin content
        return self.maximum

    def get_minimum(self): # minimum bin content
        return self.minimum

    def get_logy(self):
        return self.logy

    def get_time_axis(self):
        return self.time_axis

    def get_profile(self):
        return self.profile

    def get_logx(self):
        return self.logx

    def get_title(self):
        return self.title

    def get_name(self):
        return self.name

    def get_ylabel(self):
        return self.ylabel

    def get_xlabel(self):
        return self.xlabel

    def get_data_file_name(self):
        return self.data_file_name;

    #
    # Write data to point file, format x dx y dy
    #

    def save_data(self,fname):
        data_file=open(fname,'w')
        for i in range(self.nbins):
            x = self.get_bin_center(i)
            y = self.get_bin_content(i)
            dy = math.sqrt(self.get_bin_content(i))
            dx = 0.5*self.bw
            if ( self.profile ) :
                if ( self.sumarray[i] > 0 ) :
                    y = y  / self.sumarray[i]
                    dy = dy / self.sumarray[i]
            if ( self.time_axis ) : 
                data_file.write("%s %f %f %f \n"%(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(x)),dx,y,dy))
            else :
                data_file.write("%f %f %f %f\n"%(x,dx,y,dy))
        data_file.close()

    #
    # "plot" is the act of creating an image
    #

    def plot(self,dir="./"):
        full_file_name=dir+self.data_file_name
        self.save_data(full_file_name)
        gnu_file_name = "tmp_%s_gnuplot.cmd"%(self.name)
        gnu_cmd = open(gnu_file_name,'w')
        long_string=""
        if (  self.time_axis ) : 
            long_string=long_string+"set output '"+self.name+".ps'\n"+ \
                         "set terminal postscript color solid\n"\
                         "set title '"+self.title+" %s'"%(time.strftime("%Y-%b-%d %H:%M:%S",time.localtime(time.time())))+"\n" \
                         "set xlabel 'Date (year-month-day)'\n"+ \
                         "set timefmt \"%Y-%m-%d:%H:%M:%S\"\n"+ \
                         "set xdata time\n"+ \
                         "set xrange [ : ]\n"+ \
                         "set size 1.5,1\n"+ \
                         "set grid\n"+ \
                         "set format x \"%y-%m-%d\"\n"
            if ( self.get_logy() ) :
                long_string=long_string+"set logscale y\n"
                long_string=long_string+"set yrange [ 0.99  : ]\n"
            if ( self.get_logx() ) :
                long_string=long_string+"set logscale x\n"
            long_string=long_string+"set ylabel '%s'\n"%(self.ylabel)+ \
                         "set xlabel '%s'\n"%(self.xlabel)+ \
                         "plot '"+full_file_name+\
                         "' using 1:4 t '' with points\n "
        else :
            long_string=long_string+"set output '"+self.name+".ps'\n"+ \
                         "set terminal postscript color solid\n"\
                         "set title '"+self.title+" %s'"%(time.strftime("%Y-%b-%d %H:%M:%S",time.localtime(time.time())))+"\n" \
                         "set xrange [ : ]\n"+ \
                         "set size 1.5,1\n"+ \
                         "set ylabel '%s'\n"%(self.ylabel)+ \
                         "set xlabel '%s'\n"%(self.xlabel)+ \
                         "set grid\n"
            #                     "set style fill solid 1.000000 \n" (not working:)
            if ( self.get_logy() ) :
                long_string=long_string+"set logscale y\n"
                long_string=long_string+"set yrange [ 0.99  : ]\n"
            if ( self.get_logx() ) :
                long_string=long_string+"set logscale x\n"
            long_string=long_string+"set key right top Left samplen 20 title \""+\
                         "Mean : %.2e"%(self.mean)+"+-%.2e"%(self.mean_error)+\
                         "\\n RMS : %.2e"%(self.rms)+"+-%.2e"%(self.rms_error)+\
                         "\\nEntries : %d"%(self.entries)+\
                         "\\n Overflow : %d"%(self.overflow)+\
                         "\\n Underflow : %d"%(self.underflow)+"\"\n"+\
                         "plot '"+full_file_name+\
                         "' using 1:3 t '' with boxes\n "
            
        gnu_cmd.write(long_string)
        gnu_cmd.close()
        os.system("gnuplot %s"%(gnu_file_name))
        os.system("convert -rotate 90 -modulate 80 %s.ps %s.jpg"%(self.name,self.name))
        os.system("convert -rotate 90 -geometry 120x120 -modulate 80 %s.ps %s_stamp.jpg"%(self.name,self.name))
        os.system("rm -f %s"%full_file_name) # remove pts file
        os.system("rm -f %s"%gnu_file_name)  # remove gnu file
       
    def dump(self):
        print repr(self.__dict__)
        
if __name__ == "__main__":

    if (1) :
        hist=Histogram1D("try","try",100,0,10)
        while ( hist.n_entries() < 10000 ) :
            x=random.gauss(5,0.5)
            hist.fill(x)
    else:
        now    = int(time.time())
        then   = now - 30*3600*24
        middle = now - 15*3600*24
        width  = 4*3600*24
        print float(middle),float(width)
        hist=Histogram1D("try","try",100,then,now)
        while ( hist.n_entries() < 10000 ) :
            x=random.gauss(float(middle),float(width))
            hist.fill(x)
        hist.set_time_axis(True)

    hist.set_ylabel("Counts")
    hist.set_xlabel("x variable")
    hist.set_logy(True)
    hist.plot()
    os.system("display %s.jpg&"%(hist.get_name()))
    sys.exit(0)

