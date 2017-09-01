#!/usr/bin/env python
#
#@File: start_process.py
#
# Starts a single visao process

from AdOpt import processControl, cfg
import sys, os
import getopt

def usage():
  print """
start_process.py: Start one or more processes by its processList name

Usage:
       start_process.py [-h]
       start_process.py pname [pname2] [-q]

where:
       -q:     Terse output
       -h:     print this message and exit
       pname:  the name of the process to start
       pname2: name(s) of additional processes to start
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
      processControl.startProcessByName(process,verbose=verbose)

if __name__ == '__main__': main()