#!/usr/bin/env python
#
#@File: visao_start.py
#
# Main startup script. Starts all VisAO-related processes

from AdOpt import processControl, cfg
import sys, os, time
import getopt

def usage():
  print """
visao_start.py: Orderly start up the AO processes.

Usage:
       visao_start.py [-h]
       visao_start.py [-l] [-q]

where:
      -l:     Start only local processes, excluding the msgd clients
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
            processControl.startProcessByName(process,verbose=verbose,dryrun=dryrun)
            time.sleep(0.5)
 
   else:
      print "No visao_boot in processList.conf\n"
      print "Can't boot VisAO system.\n"
      sys.exit(0)

if __name__ == '__main__': main()
