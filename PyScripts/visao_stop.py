#!/usr/bin/env python
#
# Stops all VisAO processes

from AdOpt import cfg, processControl
from VisAO import VisAOprocessControl

import sys, os, time, getopt

def usage():
  print """
visao_stop.py: Orderly stop up the VisAO processes.

Usage:
       visao_stop.py [-h]
       visao_stop.py [-l] [-q]

where:
      -l:    Stop only local processes, excluding the msgd clients
      -q:    Terse output
      -h:    print this message and exit

"""


def main():
   verbose=True

   boot_name = 'visao_boot_all'

   try:
      opts,args = getopt.getopt(sys.argv[1:], "ldq")
   except getopt.GetoptError, err:
      print str(err)
      usage()
      sys.exit(2)

   dryrun=False

   for o,a in opts:
      if o == '-l':
         boot_name = 'visao_boot'
      elif o=='-q':
         verbose=False
      elif o=='-h':
         usage()
         sys.exit(2)
      else:
         assert False, "unhandled option"

   # Check side and subsystem, cfg won't work otherwise

   side = os.getenv('ADOPT_SIDE')

   subs = os.getenv('ADOPT_SUBSYSTEM')

   goon_ok=True

   if side==None:
      print 'No side specified, and no ADOPT_SIDE environment variable set.'
      print 'Please set the ADOPT_SIDE environment variable, or specify a side (L or R) on the command line.'
      goon_ok=False

   if subs==None:
      print 'No ADOPT_SUBSYSTEM environment variable set.'
      print 'Please set the ADOPT_SUBSYSTEM environment variable to either "WFS" or "ADSEC"'
      goon_ok=False

   if not goon_ok: sys.exit(0)

   # New-style staged boot
   if hasattr(cfg, boot_name):

      if boot_name == 'visao_boot_all':
         stagelist = cfg.visao_boot_all
      else:
         stagelist = cfg.visao_boot
   
      for stage in stagelist:
   
         procList = stage.split()
   
         for process in procList:
            # Send TERMINATE messages, if possible
            VisAOprocessControl.stopProcessByName(process, verbose=False)
      
      time.sleep(5)

      for stage in stagelist:
         procList = stage.split()
   
         for process in procList:
            # Kill everything
            while VisAOprocessControl.stopProcessByName(process, kill = True) == True:
               pass

if __name__ == '__main__': main()
