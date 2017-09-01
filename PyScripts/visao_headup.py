#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#@File: visao_headup.py
#
# Update visao fits files headers

import visao
import sys, os, time
import getopt

def usage():
  print """
visao_headup.py: Update VisAO fits headers with 

Usage:
       visao_headup.py [-h]
       visao_headup.py [-r] [dir]

where:
       dir:  name of directory to run in
      -r:    recursive - run on all images in subdirectories
      -h:    print this message and exit

"""


def main():
   verbose=True


   try:
      opts,args = getopt.getopt(sys.argv[1:], "rhf")
   except getopt.GetoptError, err:
      print str(err)
      usage()
      sys.exit(2)
    
   recurse = 0
   force = 0
   for o,a in opts:
      if o == '-r':
        recurse = 1
      elif o=='-h':
         usage()
         sys.exit(2)
      elif o=='-f':
         force = 1
      else:
         assert False, "unhandled option"


   if len(args) == 1:
     dir = args[0]
   else:
     dir = './'
     
   aosysdir = os.getenv('AOSYS_LOGDIR')
   
   if recurse == 0:
      #First load the AO system data for the directory
      loopst = visao.getLoopStat(dir, aosysdir, force)   

      #Now if a single directory, update the fits files in it.
      if loopst[0] != '-1' and loopst[0] != '-2':
         visao.updateLoopStat(loopst)
      elif loopst[0] == '-2':
         print 'Directory already updated'
      elif loopst[0] == '-1':
         print 'Not enough data found'
      
   #Or if recursive, now do subdirectories
   if recurse == 1:
     for name in os.listdir(dir):
       if os.path.isdir(os.path.join(dir,name)):
         print os.path.join(dir,name)
         loopst = visao.getLoopStat(os.path.join(dir,name), aosysdir, force)
         if loopst[0] != '-1' and loopst[0] != '-2':
            visao.updateLoopStat(loopst)
         elif loopst[0] != [-2]:
            ndir = os.path.join(dir,name)
            for name2 in os.listdir(ndir):
               if os.path.isdir(os.path.join(ndir,name2)):
                  loopst = visao.getLoopStat(os.path.join(ndir,name2), aosysdir, force)
                  if loopst[0] != '-1' and loopst[0] != '-2':
                     visao.updateLoopStat(loopst)


if __name__ == '__main__': main()
