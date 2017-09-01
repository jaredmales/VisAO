#!/usr/bin/env python
#
#@File: restart_process.py
#
# Restarts one or more visao process

from AdOpt import processControl, cfg
from VisAO import VisAOprocessControl
import sys, os, time
import getopt

def usage():
  print """
restart_process.py: restart one or more processes by its processList name

Usage:
       restart_process.py [-h]
       restart_process.py pname [pname2] [-q]

where:
       -q:     Terse output
       -h:     print this message and exit
       pname:  the name of the process to restart
       pname2: name(s) of additional processes to restart
"""

def main():
   verbose=True

   try:
      opts,args = getopt.getopt(sys.argv[1:], "hq")
   except getopt.GetoptError, err:
      print str(err)
      usage()
      sys.exit(2)

   for o,a in opts:
      if o=='-q':
         verbose=False
      elif o=='-h':
         usage()
         sys.exit(2)
      else:
         print "Unknown option."
         usage()
         sys.exit(2)

   for process in args:
      p = processControl.getProcess(process)
      
      if p:
         id = processControl.getProcessID(process)
         if id != 0:
            VisAOprocessControl.stopProcessByName(process, verbose=verbose)
            
         time.sleep(5)
         
         id = processControl.getProcessID(process)
         
         if id != 0:
            VisAOprocessControl.stopProcessByName(process, kill=True, verbose=verbose)
         else:
            if verbose: print 'process %s is down' % process
            
         processControl.startProcessByName(process,verbose=verbose)

      else:
         print "No such process: %s" % process
         
if __name__ == '__main__': main()