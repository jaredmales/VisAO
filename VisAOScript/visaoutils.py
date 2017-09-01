##@file   visaoutils.py
#
#   Various utilities for use in VisAOScript
#
#

"""@package   visaoutils

   Various utilities for use in VisAOScript

"""

# -*- coding: utf-8 -*-

import sys, os, time, calendar, glob, math, copy, pyfits, shutil


def get_visao_filename(prefix,  t):
   """
      Gets the standard VisAO filename (without the leading base name) for a time t.
      Input: 
             prefix   =  prefix of the filename
             t        = the time in seconds since the epoch.
      Returns:  a string containing the VisAO filename time portion.
   """
   gmt = time.gmtime(t)
   t_usec = int((t - math.floor(t))*1e6)

   res = "%s_%04i%02i%02i%02i%02i%02i%06i" % (prefix, gmt.tm_year, gmt.tm_mon, gmt.tm_mday, gmt.tm_hour, gmt.tm_min, gmt.tm_sec, t_usec)

   return res

def get_visao_filename_now(prefix):
   """
      Gets a filename
   """
   sd = get_visao_filename(prefix, time.time())
   
   return sd

def visao_wait(waitt):
   """
      Signal proof pause
   """
   t = time.mktime(time.localtime())
   while(time.mktime(time.localtime()) - t < waitt):
         time.sleep(waitt)
         
         
def sdssFilters():
   """
      Return a sequence containing the names of the VisAO bandpasses
   """
   filters=["r'", "i'", "z'", "Ys"]
   return filters

def xDither(scale = 1.0):
   """
      Return a the x offsets for a standard 5 point dither pattern (starting from 0)
      
      scale = multiplicative scale to multiply by, default is 1.0
   """
   dx = [-.5*scale, 1.*scale, 0.*scale, -1.*scale]
   
   return dx

def yDither(scale = 1.0):
   """
      Return the y offsets for a standard 5 point dither pattern (starting from 0)
      
      scale = multiplicative scale to multiply by, default is 1.0
   """
   dy = [.5*scale, 0.*scale, -1.*scale, 0.*scale]
   
   return dy
  
def visao_say(txt):
   """
      Say something using the MagAO sound server as the VisAO personality.
      
      txt = the words to say
   """
   cmd = "echo vicki %s | nc zorro.lco.cl 50000" % (txt) 
   os.system(cmd)
   
def visao_script_complete():
   visao_say("viz ay oh  script complete")
   
def visao_script_error():
   visao_say("viz ay oh script airor")
   
def visao_script_stopped():
   visao_say("viz ay oh script stopped")
   
def visao_alert(n=1):
   """
      Issue an audible alert to the terminal.
      
      n = the number of alerts to issue with a 2 second wait between.
   """
   if n == 1:
      print('\a')
   else:
      for i in range(n):
         print('\a')
         visao_wait(2.)
    
    
def parseSysLog(fname):
   f=open(fname, 'r')

   res = list()
   for line in f:
     
     elem = line.split()
     
     tv_sec = float(elem[0])
     tv_used = float(elem[1])
      
     fsec = tv_sec + tv_used/1e6
     
     elem[0] = fsec
     elem.pop(1)
     
     res.append(elem)
     #print elem
     
   f.close()
   
   return res


def timestampFileName(fname, prefix="V47_"):
  off = len(prefix)
  yr = int(fname[off:off+4])
  mo = int(fname[off+4:off+6])
  d = int(fname[off+6:off+8])
  hr = int(fname[off+8:off+10])
  mn = int(fname[off+10:off+12])
  s = int(fname[off+12:off+14])
  usec = int(fname[off+14:off+20])
  
  tv = calendar.timegm([yr, mo, d, hr, mn, float(s)+float(usec)/1e6, '', '' , ''])
  
  return tv
  
  
def getFileTimes(dir, prefix="V47_", ext=".fits"):
   
   if(dir[len(dir)-1] != '/'):
     dir = dir + '/'
     
   fnames = glob.glob(dir+prefix+"*"+ext)
   
   res = list()
   
   for dfn in fnames:
      fn = dfn[len(dir):len(dfn)]
      ft = timestampFileName(fn, prefix)
      if(ext == ".fits"):
         joetimes = getJoeTimes(dfn)
      else:
         joetimes = [-1,-1]
         
      res.append([dfn, fn, ft, joetimes[0], joetimes[1]])
     
   return sorted(res,  key=lambda times: times[2])   
   
def getOneFileTime(dir, prefix="V47_", ext=".fits"):
   
   if(dir[len(dir)-1] != '/'):
     dir = dir + '/'
     
   fnames = glob.glob(dir+prefix+"*"+ext)
   fnames.sort(key=os.path.getmtime)

   res = list()
   
   if len(fnames) < 1:
      return res

   dfn = fnames[-1]
   fn = dfn[len(dir):len(dfn)]
   ft = timestampFileName(fn, prefix)
   if(ext == ".fits"):
      joetimes = getJoeTimes(dfn)
   else:
      joetimes = [-1,-1]
         
   res.append([dfn, fn, ft, joetimes[0], joetimes[1]])
     
   return sorted(res,  key=lambda times: times[2])   

def chooseLogFile(ftime, loglist):
   
   for i in range(len(loglist)-1):
      if(loglist[i][2] <= ftime and loglist[i+1][2] >= ftime):
        return i
   
   return -1
   
def getVisAOExpTimes(ftime, rotime, exptime):
 
  expend = ftime-rotime
  expst  = ftime-rotime-exptime
  
  return [expst, expend]
  
def getLogEntries(ftime1, ftime2, loglist):
  
   #exptime1 = getVisAOExpTimes(ftime1, rotime, exptime)
  
   #ist = chooseLogFile(exptime1[0], loglist)
  
   ist = chooseLogFile(ftime1, loglist)
  
   #exptime2 = getVisAOExpTimes(ftime2, rotime, exptime)
  
   #iend = chooseLogFile(exptime2[1], loglist)
   iend = chooseLogFile(ftime2, loglist)
   
   #Just to be safe go one more
   if(iend < len(loglist) - 1):
     iend = iend + 1
     
   entries = list()
  
   entries = parseSysLog(loglist[ist][0])
   
   for i in range(ist+1, iend):
      entries.extend(parseSysLog(loglist[i][0]))
          
   return entries
  
  
def getCCD47MinExp(pxrt,window, bin):
   """
      Get the minimum exposure time (also the readout time) for the VisAO CCD47 image.
      input:
         pxrt = the pixel rate (2500,250, or 80)
         window = the window size (1024, 512,256, 64, or 32)
         bin = the binning (1,2,16)
      output:
         minexptime = the minimum exposure time
   """
   
   minexptime = 0
   if(bin == 1):
      if(window == 1024):
         if(pxrt == 2500):
            minexptime = 1./3.528
         if(pxrt == 250):
            minexptime = 1./0.440
         if(pxrt == 80):
            minexptime = 1./0.144
      if(window == 512):
         if(pxrt == 2500):
            minexptime = 1./6.703
         if(pxrt == 250):
            minexptime = 1./1.487
         if(pxrt == 80):
            minexptime = 1./0.535
      if(window == 256):
         if(pxrt == 80):
            minexptime = 1./1.772
      if(window == 64):
         if(pxrt == 2500):
            minexptime = 1./31.283
      if(window == 32):
         if(pxrt == 2500):
            minexptime = 1./42.779
   if(bin == 2):
      if(window==1024):
         if(pxrt==80):
            minexptime = 1./0.551
   if(bin==16):
      if(window==1024):
         if(pxrt==80):
            minexptime=1./10.420
            
   if(minexptime == 0):
      print 'getCCD47MinExp: Read out time not found for parameters'
      return -1

   return minexptime
  
  
def getJoeTimes(fname):
   """
      Get the exposure time and readout time for a VisAO CCD47 image.
      input:
         fname = file name
      output:
         [rotime, exptime]
   """
   hdu = pyfits.open(fname)
     
   exptime = float(hdu[0].header["EXPTIME"]) 
   pxrt = float(hdu[0].header["V47PIXRT"])
   window = float(hdu[0].header["V47WINDX"])
   bin = float(hdu[0].header["V47BINX"])
   
   hdu.close()
   
   rotime = 0
   if(bin == 1):
      if(window == 1024):
         if(pxrt == 2500):
            rotime = 0.283446688
         if(pxrt == 250):
            rotime = 1./0.44
         if(pxrt == 80):
            rotime = 1./0.144
      if(window == 512):
         if(pxrt == 2500):
            rotime = 1./6.70
         if(pxrt == 250):
            rotime = 1./1.487
         if(pxrt == 80):
            rotime = 1./0.535
      if(window == 256):
         if(pxrt == 80):
            rotime = 1./1.772
      if(window == 64):
         if(pxrt == 2500):
            rotime = 1./31.480
      if(window == 32):
         if(pxrt == 2500):
            rotime = 1./42.779
   if(bin == 2):
      if(window==1024):
         if(pxrt==80):
            rotime = 1./0.551
   if(bin==16):
      if(window==1024):
         if(pxrt==80):
            rotime=1./10.420
            
   if(rotime == 0):
      print 'Read out time not found for %s' % fname
      return [-1,-1]
   
   
   
   return [rotime, exptime]
  
def getLoopStat(datadir, aosysdir, force = 0):
   """
      Calculates loop status and avg WFE during each image in a directory.
      Loop status is closed iff the loop was closed during the entire exposure.  If exposure time is less than 1 second, the nearest time is used.
      A check of WFE is first made for inf and spuriously large values, which are interpolated. Avg WFE is calculated as the average of the 1 second averages recorded in the system logs.
      If exptime is shorter than 1 second, the nearest WFE entry is used. Std dev of WFE is calculated as the average of the 1 second averages recorded in the system logs.
      
      Writes a data file to the datadir with the wfe, and a file with the second by second system status.
      
      input:
         datadir = directory where the images are located
         aosysdir = directory where the ao system logs are located
         force = perform update even if the file has already been updated.
        
      output:
         list of [full_path, file_name, loop_status, wfe_avg, std_avg]
   """

   #First load a single file just to see if this directory has been processed.
   datalistone = getOneFileTime(datadir) 

   if len(datalistone) < 1:
      print 'No VisAO fits files found in %s' % datadir
      return (['-1', '-1', '-1', '-1', '-1'])

   if checkLoopStat(datalistone) == 1 and force == 0:
      return (['-2', '-2', '-2', '-2', '-2'])

   
   datalist = getFileTimes(datadir) 
#   if checkLoopStat(datalist) == 1 and force == 0:
#      return (['-2', '-2', '-2', '-2', '-2'])

   loglist = getFileTimes(aosysdir,'aosys_', '*.txt') 
      
   #print datalist

   if len(datalist) < 1:
     print 'No visao fits files found'
     return (['-1', '-1', '-1', '-1', '-1'])
   
   print 'Getting system logs:'
   print ' Directory: %s' % datadir
   print '   Start time: %f' % datalist[0][2]
   print '     End time: %f' % datalist[len(datalist)-1][2]
   print ' Elapsed time: %f' % (float(datalist[len(datalist)-1][2]) - float(datalist[0][2]))
   
   #get the log entries that apply to this data-set
   #todo: make gLE just use times, and calculate ftime1 here from datalist[0][3] and [4]
   rawentries = getLogEntries(datalist[0][2]-datalist[0][3]-datalist[0][4], datalist[len(datalist)-1][2], loglist)
    
   #make a deep copy so we can compare interpolated vs raw values.
   entries = copy.deepcopy(rawentries)
 
   print 'Starting WFE interpolation'
   #check for and interpolate bad WFE values
   for i in range(1,len(entries)-1):
      if entries[i][2] == 'inf' or float(entries[i][2]) > 1e5:
         n = 0
         q = 1
         while n == 0:
            if(entries[i-q][0] != 'inf' and float(entries[i-q][2] < 1e5)):
               n = q
            q = q+1
            if i-q < 0:
               n=i
          
         t0 = float(entries[i-n][0])
         wfe0 = float(entries[i-n][2])
         std0 = float(entries[i-n][3])
        
         t1 = float(entries[i][0])
        
         if(entries[i+1][0] != 'inf' and float(entries[i+1][2] < 1e5)):
           n = 1
         else:
           n=2
          
         n = 0
         q = 1
         while n == 0:
           if(entries[i+q][0] != 'inf' and float(entries[i+q][2] < 1e5)):
              n = q
           q = q+1
           if i+q > len(entries)-1:
            n = len(entries)-1 - i   
          
          
         t2 = float(entries[i+n][0])
         wfe2 = float(entries[i+n][2])
         std2 = float(entries[i+n][2])
        
         wfe1 = wfe0 + (wfe2-wfe0)/(t2-t0)*(t1-t0)
         std1 = std0 + (std2-std0)/(t2-t0)*(t1-t0)
        
         entries[i][2] = str(wfe1)
         entries[i][3] = str(std1)
        
   #write the wfe time series
   f = file(datadir+'/wfe.txt', 'w')
   for i in range(len(rawentries)):
      f.write("%s %s %s %s %s %s \n" % (rawentries[i][0], rawentries[i][1], rawentries[i][2], rawentries[i][3], entries[i][2], entries[i][3]))
         
   #now get the system status for each exposure
   statent = list()
   for i in range(len(datalist)):
     
      joetime = getJoeTimes(datalist[i][0])
     
      etimes = getVisAOExpTimes(datalist[i][2], joetime[0], joetime[1])
      
      if len(entries) <= 0:
         print 'No AOSYS entries'
         return  (['-1', '-1', '-1', '-1', '-1'])

      #print "%s %f %f %s" %(entries[0][0], etimes[0], etimes[1], entries[len(entries)-1][0])
      j = 0
      while(float(entries[j][0]) <= etimes[0]):
        j=j+1
        if j >= len(entries): break
        
      #j = j
        
      loopst = 1
      wfeavg = 0.
      stdavg = 0.
      
      k = j

      if k < len(entries):

        while(float(entries[k][0]) <= etimes[1] and k < len(entries)-1):
        
          if(int(entries[k][1]) != 1): loopst = 0
          wfeavg = wfeavg + float(entries[k][2])
          stdavg = stdavg + float(entries[k][3])
          k = k + 1
          if k >= len(entries): break
        
        if(k < len(entries)-1):

          if(k == j):
            if(int(entries[j][1]) != 1 or int(entries[j+1][1]) != 1): loopst = 0
            wfeavg = 0.5*(float(entries[j][2]) + float(entries[j][2]))
            if(j < len(entries)-1):
              stdavg = 0.5*(float(entries[j][3]) + float(entries[j+1][3]))
            else:
              stdavg = 0.
          else:
            wfeavg = wfeavg/(k-j)
            stdavg = stdavg/(k-j)
        
      statent.append([datalist[i][0], datalist[i][1], loopst, wfeavg, stdavg])
      
   f = file(datadir+'/status.txt', 'w')
   for i in range(len(statent)):
      f.write("%s %i %f %f\n" % (statent[i][1], statent[i][2], statent[i][3], statent[i][4]))
     
   return statent  
      
def updateLoopStat(statlist):
       
   #print statlist

   for im in statlist:
      #print im[0]
      hdu = pyfits.open(im[0], 'update')
   
      if im[2] == 1:
         hdu[0].header.update('AOLOOPST', "CLOSED",comment="AO loop status during exposure") 
         #hdu[0].header.comments['AOLOOPST'] = 
      else:
         hdu[0].header.update('AOLOOPST', "OPEN",comment="AO loop status during exposure") 
         
      hdu[0].header.update('AVGWFE', im[3],comment='Avg WFE (nm rms phase)')    
      hdu[0].header.update('STDWFE', im[4],comment='Std Dev of WFE (nm rms phase)')
      
      hdu[0].header.update('HISTORY',  'VisAO system status update applied %s' % time.asctime(time.localtime(time.time())))
      
      hdu.close()
      
def checkLoopStat(statlist):
  #Check if this list has already been processed

   if len(statlist) <= 0:
      return 0

   im = statlist[0]
   hdu = pyfits.open(im[0], 'update')

   if hdu[0].header.get('AOLOOPST') != 'NOT PROCESSED':
      return 1

   return 0


def getGoodIms(dir, movedir):
  """
     run this after headers updated
  """
  if(dir[len(dir)-1] != '/'):
     dir = dir + '/'
     
  if(movedir[len(movedir)-1] != '/'):
     movedir = movedir + '/'
     
  fnames = glob.glob(dir+"V47_*.fits")
   
  good = list()
   
  nopen = 0
  nsci = 0
  ndrk = 0
  
  for dfn in fnames:
     hdu = pyfits.open(dfn)
     fn = dfn[len(dir):len(dfn)]
     
     if (hdu[0].header["AOLOOPST"] == 'OPEN' and hdu[0].header["VIMTYPE"] == 'SCIENCE'):
        nopen = nopen + 1
        
        
     if (hdu[0].header["AOLOOPST"] == 'CLOSED' and hdu[0].header["VIMTYPE"] == 'SCIENCE'):
        nsci = nsci + 1
        good.append([dfn, fn])
        
     if (hdu[0].header["VIMTYPE"] == 'DARK'):
        ndrk = ndrk + 1
        good.append([dfn, fn])
         
  print nopen
  print nsci
  print ndrk
  
  
  for f in good:
    newf = movedir + f[1]
    shutil.copyfile(f[0], newf)
    
     
def getDarks(dir, movedir):
  """
     run this after headers updated
  """
  if(dir[len(dir)-1] != '/'):
     dir = dir + '/'
     
  if(movedir[len(movedir)-1] != '/'):
     movedir = movedir + '/'
     
  fnames = glob.glob(dir+"V47_*.fits")
   
  good = list()
   
  nopen = 0
  nsci = 0
  ndrk = 0
  
  for dfn in fnames:
     hdu = pyfits.open(dfn)
     fn = dfn[len(dir):len(dfn)]
        
     if (hdu[0].header["VIMTYPE"] == 'DARK'):
        ndrk = ndrk + 1
        good.append([dfn, fn])
         
  print nopen
  print nsci
  print ndrk
  
  
  for f in good:
    newf = movedir + f[1]
    shutil.copyfile(f[0], newf)
    
def sdssFilters():
   """
     Returns a list of the standard VisAO filters
   """
   filters=["r'", "i'", "z'", "Ys"]
   return filters

def xDither(scale = 1.0):
   """
     Returns a list of x offsets for a gimbal dither.  Units are mm.
     
     scale = total width of the pattern in mm
   """
   dx = [-.5*scale, 1.*scale, 0.*scale, -1.*scale]
   
   return dx

def yDither(scale = 1.0):
   """
     Returns a list of y offsets for a gimbal dither.  Units are mm.
     
     scale = total width of the pattern in mm
   """
   dy = [.5*scale, 0.*scale, -1.*scale, 0.*scale]
   
   return dy
  
def visao_say(txt):
   cmd = "echo vicki %s | nc zorro.lco.cl 50000" % (txt) 
   os.system(cmd)
   
def visao_script_complete():
   visao_say("viz ay oh  script complete")
   
def visao_script_error():
   visao_say("viz ay oh script airor")
   
def visao_script_stopped():
   visao_say("viz ay oh script stopped")
   
   
   