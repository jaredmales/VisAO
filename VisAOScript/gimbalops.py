
import visao
import numpy
import time
from Numeric import *


#to do: Move these to the main visao module.
   
def dither_phot(filters, nims, ndarks, dx, dy, subdir="", appendts=1):

   try:
      if subdir == "":
         subdir = "phot_dither"
      
   
      if appendts:
         sdbase = visao.get_visao_filename_now(subdir)
      else:
         sdbase = subdir
         
      vis=visao.VisAO()
      vis.take_control()
  
      for i in range(len(filters)):

         print "Changing filter to %s . . ." % filters[i]
         resp = vis.fw2.set_filter(filters[i])
         time.sleep(2)
         
         print "Moving to focus preset . . ."
         vis.focus.preset()
         
         resp = vis.fw2.wait_move()
      
         filt = vis.fw2.get_filter()

         print "In filter %s . . ." % filt
      
         if filt!= filters[i]:
            print "Filter not correct."
            break

         if(resp != 0):
            print "Focus motor error. "
            return

         vis.focus.wait_move()  
       
         filtname = filt.replace(" ", "_")
         filtname = filt.replace("'", "p")
         
         sd = "%s/%s" % (sdbase, filtname)
         print sd
         
         vis.dither(dx, dy, nims, ndarks, sd, 0,0)

   
      vis.giveup_control()
      visao_script_complete()
      
   except:
      print '******************\nSomething bad happened\n******************\n'
      visao.visao_alert(1)
      raise


def scan(xcen, ycen, r0, dr, ang, ims, ndarks, subdir=""):
   
   ang = ang*3.14159/180.0
   
   x0 = xcen - r0*cos(ang)
   y0 = ycen - r0*sin(ang)
   
   vis = visao.VisAO()
   
   dx = range(2*r0/dr)
   
   vis.take_control()
   
   #gimb = visao.Gimbal()
   vis.gimbal.take_control()

   #ccd = VisAOScript.CCD47Ctrl()
   #vi.ccd.take_control()

   if subdir=="":
      subdir = visao.get_visao_filename_now("scan_")
   
   print "Moving to %f %f . . ." % (x0 , y0)
   
   rv = vis.gimbal.move_xabs(x0)
   if rv != 0:
     print "Error during x dither.  Halting."
     return
      
   rv = vis.gimbal.move_yabs(y0)
   if rv != 0:
     print "Error during y dither.  Halting."
     return
      
   vis.ccd47.subdir(subdir)

   vis.take_darks(ndarks)

   print "Saving %i images at position 1/%i" % (ims, len(dx))
   vis.take_science(ims)
   
   for i in range(len(dx)):
      x0 = xcen - r0*cos(ang) + dr*cos(ang)*i
      y0 = ycen - r0*sin(ang) + dr*sin(ang)*i
   
      print "Dither %i/%i:  %f %f . . ." % (i,len(dx), x0 , y0)
      rv = vis.gimbal.move_xabs(x0)
      if rv != 0:
        print "Error during x dither.  Halting."
        break
      
      rv = vis.gimbal.move_yabs(y0)
      if rv != 0:
        print "Error during y dither.  Halting."
        break

      rv = vis.gimbal.wait_move()
      if rv != 0:
        print "Error during dither.  Halting."
        break
      
      rv = vis.take_science(ims)

   rv = vis.gimbal.move_xabs(xcen)
      
   rv = vis.gimbal.move_yabs(ycen)
   
   rv = vis.gimbal.wait_move()
   
   vis.ccd47.subdir(".")
     
   #vis.giveup_control()
   #gimb.giveup_control()   
   #ccd.giveup_control()
 