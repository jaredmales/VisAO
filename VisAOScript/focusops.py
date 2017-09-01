"""
   FocusOps

   The FocusOps python module contains several scripts related to the focus stage..
"""

import visao
import visaoutils
import numpy
import time
from Numeric import *
import os
import _idl

def calibration(p = 0):
   """
      Script to calibrate the focus stage step-ratio.  Requires manual input of a micrometer measurement, in mm.

      Moves to the front or back of the travel range, whichever is closer when invoked.
   """
   vis = visao.VisAO()
   vis.take_control()
   
#   foc = vis.FocusCtrl()
   vis.focus.take_control()
   
   if p == 0:
     d = vis.focus.get_pos()
     if d > 5500.:
       poss = 11500. - array(range(9))*2500.
     else:
       poss = -9500. + array(range(9))*2500.
   else:
     poss = p

   cal_pos = list()
   meas_pos = list()
   
   for i in range(len(poss)):
      print "************************"
      print "Moving to %f microns." % poss[i]

      resp = vis.focus.pos(poss[i])

      if(resp != 1):
        break
    
      resp = vis.focus.wait_move()

      if(resp != 1):
        break

      pos = vis.focus.get_pos()
      
      print "At %f microns." % pos
      meas = float(raw_input("Enter measurement (in mm): "))
      print "%f %f" % (pos, meas)


      cal_pos.append(pos)
      meas_pos.append(meas)


   x = array(cal_pos)
   y = array(meas_pos)*1000.
   A = numpy.vstack([x, ones(len(x))]).T

   m,b = numpy.linalg.lstsq(A,y)[0]
   
   yfit = m*x + b
   resid = yfit-y
   print "\n\nCalibration Complete\n\n"
   print "y = %f + %fx" % (b, m)
   print "\nStd dev of residuals: %f microns\n" % numpy.std(resid)
   print "Data:\nPosition\tMeasured\tResidual"
   for i in range(len(cal_pos)):
     print "%f\t%f\t%f" %(cal_pos[i], meas_pos[i], resid[i])

   vis.giveup_control()
   


def focus_old(start, end, stepsz, ims=5, settle_delay = 1.):
   """
   Peform a focus measurement.

   First take darks, saving 5*ims images with gimbal in dark position.  The moves from position start to end, in steps of size stepsz.  At each position, the script pauses for settle_delay to allow vibrations to dissipate, and then take ims images.  Images are saved in a sub-directory with the start time (standard VisAO filename) in the name. The visao_focuscurve.pro idl procedure is called to analyze the data, and the preset is updated.

   start        = the starting focus position (microns)
   end          = the ending focus position (microns)
   stepsz       = size of steps to take between focus positions (microns)
   ims          = number of images to save at each position
   settle_delay = the time, in seconds, to pause after each move to settle vibrations
   """
   try:
      foc = vis.FocusCtrl()
      foc.take_control()
   
      ccd = vis.CCD47Ctrl()
      ccd.take_control()
      
      gimb = vis.Gimbal()
      gimb.take_control()
      
      _idl.ex(".reset")
      
      dist = abs( end - start)
   
      if abs(stepsz) < 5.:
         print "Stepsize too small."
         return
      
      if end < start:
         if stepsz > 0:
            stepsz = stepsz * -1.
      else:
         if stepsz < 0:
            stepsz = stepsz * -1.
   
      if ims <= 0:
         print "Number of images must be > 0."
         return
      
      
      sd = vis.get_visao_filename_now("focus_")
      
      print "Moving focus stage to %f microns." % start
      resp = foc.pos(start)
      
      if(resp != 1):
         print "Focus motor error. "
         return
      
      
      
      print "************************"
      print "Taking CCD 47 Darks"
      
      darksd = "%s/darks" % sd
      ccd.subdir(darksd)
      
      gimbx = gimb.get_xpos()
      gimby = gimb.get_ypos()
      
      print "Gimbal start position: %f %f" % (gimbx, gimby)
      
      gimb.dark()
      
      gimb.wait_move()
      
      print "Saving %i dark images" % (ims*5)
   
      resp = ccd.save(ims*5)
      if(resp != 0):
         print "CCD47 error. "
         return
   
      resp = ccd.wait_save()
      if(resp != 1):
         print "CCD47 error. "
         return
         
      print "Saving complete."
      
      print "Moving gimbal to %f %f" % (gimbx, gimby)
      
      resp = gimb.move_xabs(gimbx)
      if(resp != 0):
         print "Gimbal error. (moving x) "
         return
      
      resp = gimb.move_yabs(gimby)
      if(resp != 0):
         print "Gimbal error. (moving y) "
         return
    
      resp = gimb.wait_move()
      if(resp != 0):
         print "Gimbal error (wait_move()). "
         return
      
      print "Gimbal move complete"
   
      resp = foc.wait_move()
   
      if(resp != 1):
         print "Focus motor error. "
         return
   
      pos = foc.get_pos()
      
      print "At %f microns.\n\nBeginning focus curve acquisition . . ." % pos
   
      nextpos = pos
   
      ccd.subdir(sd)
      
      while abs(nextpos - start) < (dist + .1*abs(stepsz)):
         print "************************"
         print "Moving to %f microns . . ." % nextpos
         
         resp = foc.pos(nextpos)
   
         if(resp != 1):
            print "Focus motor error."
            return
      
         resp = foc.wait_move()
   
         if(resp != 1):
            print "Focus motor error."
            return
   
         pos = foc.get_pos()
   
         print "Move complete.  Settling for %f seconds . . ." % settle_delay
         
         t = time.mktime(time.localtime())
         while(time.mktime(time.localtime()) - t < settle_delay):
            time.sleep(settle_delay)
            
         print "Saving %i images at %f microns." % (ims, pos)
   
         resp = ccd.save(ims)
         if(resp != 0):
            print "CCD47 error. "
            return
   
         resp = ccd.wait_save()
         if(resp != 1):
            print "CCD47 error. "
            return
         
         print "Saving complete."
   
         nextpos = pos + stepsz
         
      ccd.subdir(".")
      foc.giveup_control()
      ccd.giveup_control()
      gimb.giveup_control()
      
      print "\n\nFocus Curve acquistion complete.\n\n"
      print "Analyzing . . .\n"
      
      idlcmd = "visao_focuscurve, /interact, subdir='" + os.getenv('VISAO_ROOT') + "/data/ccd47/" + sd + "/'"
      
      _idl.ex(idlcmd)
   except:
      print '******************\nSomething bad happened\n******************\n'
      vis.visao_alert(1)
      raise
   
def focus(start, end, stepsz, ims=5, settle_delay = 1., ndarks=-1):
   """
   Peform a focus measurement.

   First take darks, saving 5*ims images with gimbal in dark position.  The moves from position start to end, in steps of size stepsz.  At each position, the script pauses for settle_delay to allow vibrations to dissipate, and then take ims images.  Images are saved in a sub-directory with the start time (standard VisAO filename) in the name. The visao_focuscurve.pro idl procedure is called to analyze the data, and the preset is updated.

   start        = the starting focus position (microns)
   end          = the ending focus position (microns)
   stepsz       = size of steps to take between focus positions (microns)
   ims          = number of images to save at each position
   settle_delay = the time, in seconds, to pause after each move to settle vibrations
   """
   try:
      
      _idl.ex(".reset")
      
      if ndarks == -1:
         ndarks = ims
         
      vis = visao.VisAO()
           
      vis.take_control()
     
      visao.visao_wait(.25)
      
      dist = abs( end - start)
   
      if abs(stepsz) < 5.:
         print "Stepsize too small."
         return
      
      if end < start:
         if stepsz > 0:
            stepsz = stepsz * -1.
      else:
         if stepsz < 0:
            stepsz = stepsz * -1.
   
      if ims <= 0:
         print "Number of images must be > 0."
         return
      
      sd = visaoutils.get_visao_filename_now("focus")
      vis.ccd47.subdir(sd)
            
      print "Moving focus stage to %f microns." % start
      resp = vis.focus.pos(start)
      
      if(resp != 1):
         print "Focus motor error. "
         return
      
      vis.take_darks(ndarks)
           
      resp = vis.focus.wait_move()
   
      if(resp != 1):
         print "Focus motor error. "
         return
   
      pos = vis.focus.get_pos()
      
      print "At %f microns.\n\nBeginning focus curve acquisition . . ." % pos
   
      nextpos = pos
   
      while abs(nextpos - start) < (dist + .1*abs(stepsz)):
         print "************************"
         print "Moving to %f microns . . ." % nextpos
         
         resp = vis.focus.pos(nextpos)
   
         if(resp != 1):
            print "Focus motor error."
            return
      
         resp = vis.focus.wait_move()
   
         if(resp != 1):
            print "Focus motor error."
            return
   
         pos = vis.focus.get_pos()
   
         print "Move complete.  Settling for %f seconds . . ." % settle_delay
         
         visao.visao_wait(settle_delay)
         
            
         print "Saving %i images at %f microns." % (ims, pos)
   
         vis.take_science(ims)
                 
         print "Saving complete."
   
         nextpos = pos + stepsz
         
      vis.ccd47.subdir(".")
      
      vis.giveup_control()
      
      visao.visao_alert(1)
      print "\n\nFocus Curve acquistion complete.\n\n"
      print "Analyzing . . .\n"
      
      idlcmd = "visao_focuscurve, /interact, subdir='" + os.getenv('VISAO_ROOT') + "/data/ccd47/" + sd + "/'"
      
      _idl.ex(idlcmd)
   except:
      print '******************\nSomething bad happened\n******************\n'
      visao.visao_alert(1)
      raise
         
def visao_focus(stepsz = 1000., ims = 5, settle_delay=1.):

   foc = vis.FocusCtrl()
   foc.take_control()
   
   
   d = foc.get_pos()

   if(fabs(d-9000) > fabs(d+9000)):
     start = -9000
     end = 9000
   else:
     start = 9000
     end = -9000
   
   return focus(start, end, stepsz, ims, settle_delay)
     
  
def filter2_focus(filters=["SDSS r'", "SDSS i'", "SDSS z'", "950 Long Pass"], homefirst=0, stepsz=1000, ims=5, settle_delay = 1.):
   """
      Peform a focus measurement, in each of a sequence of filter wheel 2 positions.

      The filters are specified as a python list, i.e. ["SDSS r'", "SDSS i'", "SDSS z'"]

      If homefirst is true, the wheel is homed before beginning.
      
      In each filter, the focus script is executed.
   """
   fw2 = vis.FilterWheel2()

   fw2.take_control()

   if homefirst:
      print "Homing FilterWheel2. . ."
      resp = fw2.home()

      resp = fw2.wait_home()
      time.sleep(5)
      print "Homing complete."

   for i in range(len(filters)):

      print "Changing filter to %s . . ." % filters[i]
      resp = fw2.set_filter(filters[i])

      
      resp = fw2.wait_move()
      time.sleep(2)
      filt = fw2.get_filter()

      print "In filter %s . . ." % filt
      
      if filt!= filters[i]:
        print "Filter not correct."
        return
      
      visao_focus(stepsz, ims, settle_delay)

   fw2.giveup_control()
   