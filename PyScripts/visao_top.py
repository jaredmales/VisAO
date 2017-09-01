#!/usr/bin/env python
#
#@File: visao_start.py
#
# Main startup script. Starts all AO-related processes

from AdOpt import cfg, processControl
import sys, os, time
import getopt


def gettopcmd():
   cmd = "top"

   stagelist = cfg.visao_boot_all
      
   for stage in stagelist:
      procList = stage.split()
      
      for process in procList:
         pid = processControl.getProcessID(process)
         if pid != 0:
            cmd = cmd + " -p" + str(pid)
         
   return cmd
   
   
   
def main():
   verbose=True
   
   cmd = gettopcmd()

   if(cmd == 'top'):
      print "no visao processes detected"
      sys.exit(2)
      
   os.system(cmd)
    
   print "visao_top.py done"
   
if __name__ == '__main__': main()
