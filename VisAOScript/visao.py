##@file   visao.py
#
#   The visao python module provides a python intervace to the VisAO camera
#
#   The components of VisAO are combined in the VisAO class
#


"""@module   visao

   The visao python module provides a python intervace to the VisAO camera

   The components of VisAO are combined in the VisAO class
"""

from visaodevices import *

class Ctrl_C_Except(Exception):
   def __init__(self, value):
      self.value = value
   def __str__(self):
      return repr(self.value)
   
   
def sigint_handler(signum, frame):
   """
      Handle SIGINT so that ctrl-c will interrup a script at any point
   """
   raise Ctrl_C_Except('script interrupted.')
   
class VisAO:
   """
      Class to control the VisAO Camera.
   """
   def __init__(self):
      self.focus = FocusCtrl()
      self.ccd47 = CCD47Ctrl()
      self.gimbal = Gimbal()
      self.shutter = Shutter()
      self.fw2 = FilterWheel2()
      self.fw3 = FilterWheel3()
      self.bcu39 = framegrabber39()
      self.fw39 = framewriter39()
      
      signal.signal(signal.SIGINT, sigint_handler)
      
      print "********************************"
      print "VisAO script control initialized"
      print "********************************"
      
   def connect(self):
      self.focus.connect()
      self.ccd47.connect()
      self.shutter.connect()
      self.fw2.connect()
      self.fw3.connect()
      self.gimbal.connect()
      
   def take_control(self):
      self.focus.take_control()
      self.ccd47.take_control()
      self.shutter.take_control(1)
      self.fw2.take_control()
      self.fw3.take_control()
      
   def giveup_control(self):
      self.focus.giveup_control()
      self.ccd47.giveup_control()
      self.shutter.giveup_control()
      self.fw2.giveup_control()
      self.fw3.giveup_control()
      
   def post_acq(self):
      self.take_control()
      self.gimbal.take_control()
      self.ccd47.write_fifoch('serve 0')
      self.giveup_control()
      
   def setup(self, filter2, filter3):
      self.fw2.set_filter(filter2)
      self.fw3.set_filter(filter3)
      
      visao_wait(2)
      
      self.focus.preset()
      
      resp = self.focus.wait_move()
      if(resp != 1):
         raise "setup: Focus motor error. "
      
      resp = self.fw2.wait_move()
      if(resp != 0):
         raise "setup: F/W 2 motor error. "
      
      resp = self.fw3.wait_move()
      if(resp != 0):
         raise "setup: F/W 3 motor error. "
      
   def take_darks(self, ndarks):
      print "take_darks: taking %i darks" % ndarks
      if self.shutter.get_state() != -1:
         et = 1./self.ccd47.get_framerate()
         print "take_darks: closing shutter"
         self.shutter.shut()
         if et >= 5.0:
            self.ccd47.stop()
            self.ccd47.start()
         else:
            visao_wait(et)
            if et < .1:
               visao_wait(.1)
               
         if self.shutter.get_state() != -1:
            raise "take_darks: could not close shutter"
         
      print "take_darks: shutter closed"
                  
     
      self.ccd47.savedark(ndarks)
      self.ccd47.wait_save()
      
      print "take_darks: done"
      
   def take_flats(self, nflt):
      print "taking %i flats" % nflt
      if self.shutter.get_state() != 1:
         et = 1./self.ccd47.get_framerate()
         print "  exptime = %f" % et
         print "  opening shutter"
         self.shutter.open()
         visao_wait(et)
         
      print "  shutter open"
      self.ccd47.imtype(4)
      visao_wait(.1)
      self.ccd47.save(nflt)
      self.ccd47.wait_save()
      print "  done"
      
   def take_science(self, nsci):
      print "take_science: taking %i science images" % nsci
      if self.shutter.get_state() != 1:
         et = 1./self.ccd47.get_framerate()
         print "take_science: opening shutter"
         self.shutter.open()
         if et >= 5.0:
            self.ccd47.stop()
            self.ccd47.start()
         else:
            visao_wait(et)
            if et < .1:
               visao_wait(.1)
               
         if self.shutter.get_state() != 1:
            raise "take_science: could not open shutter"
            
      print "take_science: shutter open"
      
      self.ccd47.savescience(nsci)
      
      self.ccd47.wait_save()
      
      print "take_science: done"
      
   def take_linearity(self, nims, etimes):
      """
         Take a linearity curve.  
       
         Takes darks and science frames at a sequence of exposure times.
       
         nims     =   number of darks and science frames to take at each exposure time
         etimes   = exposure times, e.g. [.3, .5, .75, 1.0, 2.0, 3.0], in secs
       """
      print "starting linearity curve . . ."
      sd = get_visao_filename_now("linearity_")
    
      self.ccd47.subdir(sd)
    
      self.ccd47.reps(0)
      
      self.take_darks(nims)
      
      for i in range(len(etimes)):
         print "  step %i: " % i  
         self.ccd47.exptime(etimes[i])         
         visao_wait(.5)
         et = 1./self.ccd47.get_framerate()
         print "  ccd47 exposure time set to %f" % et
      
         self.take_science(nims)
      
      print "  linearity curve complete"
      
      self.ccd47.subdir(".")
      
   def science(self, totexp, exptime, masterdarks, darkint, ndark, dirname="science", appendts=1, subroutine=0):
      """
         Take a science data set  
       
         Takes science images until a specified total exposure time is reached.  Takes darks at intervals.
         Other than exposure time, assumes camera is set up as you prefer.
       
         totexp      =   total science exposure time, in seconds
         exptime     =   exposure time of individual images, in seconds
         masterdarks = number of darks to take at beginning of script
         darkint     =   interval between dark sets, in seconds
         ndark       =   number of dark frames to take 
         dirname     =   base name of directory to store images in, default is "science"
         appendts    =   append the standard VisAO timestamp, default is 1 (yes).  set to 0 for no. 
      """
      try:
         
         if exptime == 0:
            exptime = 1/self.ccd47.get_framerate()
            print "science: using current exposure time = %f" % exptime
            
         nscitot = math.floor(totexp/exptime + 0.99999)
         nsci = math.floor(darkint/exptime)
         if darkint <= 0:
            nsci = nscitot
            ndarksets = 0
         else:
            ndarksets = math.floor(nscitot/darkint)
      
         if appendts:
            sd  = get_visao_filename_now(dirname)
         else:
            sd = dirname
            
         print '\n**********************************************\n'
         print ' Will take VisAO science data:'
         print '  %i total science images, %f sec total science exposure' % (nscitot , (nscitot*exptime))
         print '  in sets of %i images with %i darks in between' % (nsci , ndark)      
         print '  Total time = %f sec' % (nscitot*exptime + ndarksets*ndark*exptime)
         print "  Saving images to directory: %s" % sd
         print '\n'
         
         if subroutine == 0:
            foo = raw_input('Proceed ? [y/n] (n): ')
            if (foo != 'y'):
               print 'Stopping'
               return
      
            print 'Proceeding . . .'
         
         self.connect()
         self.take_control()
         
         self.ccd47.subdir(sd)

         self.focus.preset()
         visao_wait(1.)
         resp = self.focus.wait_move()
         if(resp != 1):
            raise "science: focus motor error. "
      
         print "science: focus motor is focused."
      
         #Do this after the focus check to give it time to propagate
         set_subdir = self.ccd47.get_subdir()
         if(set_subdir != sd):
            visao_wait(1.)
            set_subdir = self.ccd47.get_subdir()
            if(set_subdir != sd):
               print set_subdir
               print sd
               raise "science: ccd47 subdirectory error"
         
         
         self.ccd47.exptime(exptime)
         nsci_taken = 0
         
         if (masterdarks > 0):
            self.take_darks(masterdarks)
         
         for i in range(int(nscitot/nsci)):
            
            self.take_science(nsci)
            
            if(ndark > 0):
               self.take_darks(ndark)
            
            nsci_taken = nsci_taken + nsci
      
         self.take_science(nscitot-nsci_taken)
         nsci_taken = nscitot-nsci_taken
      

         print '\n**********************************************\n'
         print ' Completed taking VisAO science data:'
         print '  %i total science images taken.' % nsci_taken
         print '\n**********************************************\n'
         
         self.ccd47.subdir("./")
         
         if subroutine == 0:
            self.giveup_control()
            visao_alert(1)
            visao_script_complete()
         
      except Ctrl_C_Except:
         visao_alert(1)
         
         self.ccd47.save(0)
         self.ccd47.subdir("./")
         self.shutter.open()
         self.giveup_control()
         
         visao_script_stopped()
         print '******************\n science: script stopped\n******************\n'
                      
      except:
         visao_alert(1)
         self.ccd47.save(0)
         self.ccd47.subdir("./")
         self.shutter.open()
         self.giveup_control()
         visao_script_error()
         print '******************\n science: something bad happened\n******************\n'
         raise
         
         raise
         
         
   def observation(self, observations, repeats, subdir="", appendts=1):
      
      try:
         #Check for valid input
         if type(observations) != tuple:
            raise "observation: invalid observation (must be tuple)"
      
         #Next figure out how many observations there are
         #with the problem being that both 1 and 11 observations must be handled
         #given that an observation has 11 entries
      
         nobs = 0
         if len(observations) == 11:
            if type(observations[2]) is int:
               nobs = 1
            else:
               nobs = len(observations)
         else:
               nobs = len(observations)
            
            
         #now check for valid observations
         if nobs == 1:
            if type(observations) != tuple:
               raise "observation: invalid observation (must be tuple)"
         
            if len(observations) != 11:
               raise "observation: invalid observation (must have 11 elements)"
         
            #have to make this a tuple-tuple
            observations = (observations, observations)
         else:
            for i in range(nobs):
               if type(observations[i]) != tuple:
                  raise "observation: invalid observation number %d (must be tuple)" % (i+1)
         
               if len(observations[i]) != 11:
                  raise "observation: invalid observation number %d (must have 11 elements)" % (i+1)
                      
         if subdir == "":
            subdir = "observation"
      
         if appendts:
            sdbase = get_visao_filename_now(subdir)
         else:
            sdbase = subdir

      
      
         keepgoing = 1
         ntimes = 0
      
         self.connect()
         self.take_control()
      
         while keepgoing:
         
            for i in range(nobs):
            
               print "observation: Changing filter 2 to %s . . ." % observations[i][0]
               resp = self.fw2.set_filter(observations[i][0])
               print "observation: Changing filter 3 to %s . . ." % observations[i][1]
               resp = self.fw3.set_filter(observations[i][1])
            
               #sleep to give the filter wheels time to wake up and get moving
               time.sleep(2)
         
               print "observation: Moving to focus preset"
               self.focus.preset()
         
               print "observation: Programming CCD 47"
               etime = observations[i][5]
               minetime = getCCD47MinExp(observations[i][2],observations[i][3],1)
               if etime < minetime:
                  etime = minetime
               self.ccd47.set_program(observations[i][2],observations[i][3],observations[i][4], etime)
               #ccd47.set_program waits for completion
            
            
               #Now we check on the motor moves.
               resp = self.focus.wait_move()
               resp2 = self.fw2.wait_move()
               resp3 = self.fw3.wait_move()
      
      
               filt2 = self.fw2.get_filter()
               filt3 = self.fw3.get_filter()

               if filt2 != observations[i][0]:
                  raise "observation: filter 2 not correct"

               if filt3 != observations[i][1]:
                  raise "observation: filter 3 not correct"

               if(resp != 1):
                  raise "observation: focus motor error"

               print "observation: In filters %s and %s, and in focus" % (filt2, filt3)


               if(observations[i][10] == ""):
                  filtname2 = filt2.replace(" ", "_")
                  filtname2 = filtname2.replace("'", "p")
         
                  filtname3 = filt3.replace(" ", "_")
                  filtname3 = filtname3.replace("'", "p")
            
                  sd = "%s/%s_%s_%d" % (sdbase, filtname2, filtname3,i)
               else:
                  sd = "%s/%s_%d" % (sdbase, observations[i][10], (ntimes+1))               
              

               #Now call the science script in subroutine mode
               self.science(observations[i][6], etime, observations[i][7], observations[i][8], observations[i][9], sd, 0, 1)


            #check for completion
            ntimes = ntimes + 1
            if ntimes >= repeats and repeats > 0:
               keepgoing = 0
            
         
         print '\n**********************************************\n'
         print ' Completed VisAO observation:'
         print ''
         print '\n**********************************************\n'
         
         self.ccd47.subdir("./")
         
         self.giveup_control()
         visao_alert(1)
         visao_script_complete()
         
      except Ctrl_C_Except:
         visao_alert(1)
         self.giveup_control()         
         visao_script_stopped()
         print '******************\n observation: script stopped\n******************\n'
                      
      except:
         visao_alert(1)
         self.giveup_control()
         visao_script_error()
         print '******************\n observation: something bad happened\n******************\n'
         raise
         
         raise
      
         
   def dither(self, dx, dy, ims, ndarks, subdir="", appendts=1, giveup=1):
      """
         Take data in a dither pattern.
         
         Takes darks at the beginning, then takes science images at each gimbal position specified in dx,dy.
         
         dx = gimbal x positions (mm)
         dy = gimbal y positions (mm)
         ims = number of science images to take at each position.
         ndarks = number of drks to take at the beginning
         subdir = optional subdirectory.  if not specified, then dither_[timestamp] will be used
         appendts = append the standard VisAO timestamp, default is 1 (yes).  set to 0 for no. 
         giveup = if set 1 one, then control will be taken and given up at the end of the dither
      """
      
      try:
         
         if giveup:
            self.take_control()
         
         self.gimbal.take_control()
      
         if subdir=="":
            subdir = get_visao_filename_now("dither_")
         else:
            if appendts:
               subdir  = get_visao_filename_now(subdir)
            
         time.sleep(0.1)   
         self.ccd47.subdir(subdir)
         
         
         #set_subdir = self.ccd47.get_subdir()
         #if(set_subdir != subdir):
            #raise "dither: ccd47 subdirectory error"
         
         startx = self.gimbal.get_xpos()
         starty = self.gimbal.get_ypos()
         
         self.focus.preset()
         visao_wait(1.)
         resp = self.focus.wait_move()
         if(resp != 1):
            raise "dither: focus motor error. "
      
         print "dither: focus motor is focused."


         print "dither: Saving %i darks" % (ndarks)
            
         self.take_darks(ndarks)

         print "dither: saving %i images at dither 1/%i" % (ims, len(dx))
            
         self.take_science(ims)
   
         for i in range(len(dx)):
            print "dither: moving to pos %i/%i:  %f %f . . ." % (i+1,len(dx), dx[i] , dy[i])
            rv = self.gimbal.move_xrel(dx[i])
            if rv != 0:
               raise "dither: error during x dither.  Halting."
      
            rv = self.gimbal.move_yrel(dy[i])
            if rv != 0:
               raise "dither: error during y dither.  Halting."

            rv = self.gimbal.wait_move()
            if rv != 0:
               raise "dither: error during dither.  Halting."
      
            #Wait 1 exposure time to not take a bad frame
            et = 1./self.ccd47.get_framerate()
            visao_wait(et)
         
            print "dither: saving %i images at dither %i/%i" % (ims, i, len(dx))
            
            rv = self.take_science(ims)

         print "dither: returning to start positions"
   
         rv = self.gimbal.move_xabs(startx)
         if rv != 0:
            raise "dither: error during x move.  Halting."
      
         rv = self.gimbal.move_yabs(starty)
         if rv != 0:
            raise "dither: error during y move.  Halting."
            
         rv = self.gimbal.wait_move()
         if rv != 0:
            print "raise: error during dither.  Halting."
    
         self.ccd47.subdir(".")
      
         self.gimbal.giveup_control()
      
         if giveup:
            self.giveup_control()
            visao_script_complete()
                  
      except Ctrl_C_Except:
         if giveup == 0:
            raise Ctrl_C_Except('script interrupted.') #If this is subordinate, pass exception on.
         
         visao_alert(1)
         
         self.ccd47.save(0)
         self.shutter.open()
         self.giveup_control()
         
         visao_script_stopped()
         print '******************\n dither: script stopped\n******************\n'
                      
      except:
         visao_alert(1)
         self.ccd47.save(0)
         self.shutter.open()
         self.giveup_control()
         visao_script_error()
         print '******************\n dither: something bad happened\n******************\n'
         raise
        
   def dither_phot(self, filters, nims, ndarks, dx, dy, subdir="", appendts=1):
      """
         Take data in a dither pattern in multiple filters.
         
         Takes darks at the beginning of each filter, then takes science images at each gimbal position specified in dx,dy.
         Then changes filters and repeats.  Focus stage is moved to appropriate preset after each filter change.
          
         filters = list of filters to cycle through
         nims = the number of science images to take in each dither position per filter
         ndarks = the number of initial darks to take
         dx = gimbal x offsets from initial position, in mm
         dy = gimbal y offsets from initial position, in mm
         subdir = optional subdirectory.  if not specified, then dither_[timestamp] will be used
         #appendts = append the standard VisAO timestamp, default is 1 (yes).  set to 0 for no. 
      """
      try:
         if subdir == "":
            subdir = "phot_dither"
      
         if appendts:
            sdbase = get_visao_filename_now(subdir)
         else:
            sdbase = subdir
         
         self.connect()
         self.take_control()
  
         for i in range(len(filters)):

            print "Changing filter to %s . . ." % filters[i]
            resp = self.fw2.set_filter(filters[i])
            time.sleep(2)
         
            print "Moving to focus preset . . ."
            self.focus.preset()
         
            resp = self.fw2.wait_move()
      
            filt = self.fw2.get_filter()

            print "In filter %s . . ." % filt
      
            if filt!= filters[i]:
               print "Filter not correct."
               break

            if(resp != 0):
               print "Focus motor error. "
               return

            self.focus.wait_move()  
       
            filtname = filt.replace(" ", "_")
            filtname = filt.replace("'", "p")
         
            sd = "%s/%s" % (sdbase, filtname)
            print sd
         
            self.dither(dx, dy, nims, ndarks, sd, 0,0)
   
         self.giveup_control()
         visao_script_complete()
      
      except Ctrl_C_Except:
         visao_alert(1)
         
         self.ccd47.save(0)
         self.shutter.open()
         self.giveup_control()
         
         visao_script_stopped()
         print '******************\n dither_phot: script stopped\n******************\n'
                      
      except:
         visao_alert(1)
         self.ccd47.save(0)
         self.shutter.open()
         self.giveup_control()
         visao_script_error()
         print '******************\n dither_phot: something bad happened\n******************\n'
         raise
      
   def flats_old(self, totexp, exptime, ndark, dirname="flats"):
      try:     
         nsci = math.floor(totexp/exptime + 0.99999)
         
         
         sd  = get_visao_filename_now(dirname)
         print sd
      
         print '\n**********************************************\n'
         print ' Will take VisAO flats data:'
         print '  %i total flats per filter, %f sec total flat exposure' % (nsci, (nsci*exptime))
         print '  With %i darks in between' % (ndark)      
         print "  Saving images to directory: %s" % sd
         print '\n'
         foo = raw_input('Proceed ? [y/n] (n): ')
         if (foo != 'y'):
            print 'Stopping'
            return
      
         print 'Proceeding . . .'
         self.ccd47.subdir(sd)
      
         self.ccd47.exptime(exptime)
         
         nsci_taken = 0
         
#pos1_name        string  "SDI H alpha"
#pos2_name        string  "SDI [S II]"
#pos3_name        string  "SDI [O I]"

         nds=["Open"]#, "ND 3.22", "ND 3 R .15"]
         
         filters=["r'"]
         
         self.fw2.set_filter(filters[0])
         
         for n in range(len(nds)):
            print "Changing ND to %s . . ." % nds[n]
            resp = self.fw3.set_filter(nds[n])
            time.sleep(4)
            resp = self.fw3.wait_move()
            time.sleep(1)
            ndn = self.fw3.get_filter()
            
            print "In ND %s . . ." % ndn
      
            if ndn!= nds[n]:
               print "ND not correct."
               return
            
            for i in range(len(filters)):
               print "Changing filter to %s . . ." % filters[i]
               resp = self.fw2.set_filter(filters[i])  
               time.sleep(4)
               resp = self.fw2.wait_move()
               time.sleep(1)
               filt = self.fw2.get_filter()

               print "In filter %s . . ." % filt
     
               if filt!= filters[i]:
                  print "Filter not correct."
                  return
               self.take_darks(ndark)
               self.take_flats(nsci)
         
         visao_script_complete()
         visao_alert(1)
      except:
         visao_script_error()
         print '******************\nSomething bad happened\n******************\n'
         visao_alert(1)
         raise
            
            
   def telemetry_cal(self, tottime, fast_darks, slow_darks, pset=0):
      """
         Take a telemetry data set with simultaneous focal plane data.  Takes fast frames and slow frames
         
         tottime = total time of the measurement in seconds
         fast_darks = number of fast darks to take
         slow_darks = number of slow dards to take
         
         pset = the program set.  pset=0 means 1024 and 64, pset=1 means 512 and 32
         
      """   
      
      #the set commands to send to the CCD
      fast_set = 'set 0 1 0 0' #program set 0, 64x64 
      slow_set = 'set 0 0 0 0' #program set 0, 1024x1024
      
      if pset == 1:
         fast_set = 'set 1 1 0 0' #program set 1, 32x32
         slow_set = 'set 1 0 0 0' #program set 1, 512x512
            
      #the exposure times
      exptime_s = 0.031
      exptime_l = 0.283
      
      if pset == 1:
         exptime_s = .023 #0.0317662
         exptime_l = 0.149
  
      self.bcu39.take_control()
      


      #Make sure we're focused
      self.focus.preset()
      visao_wait(1.)
      resp = self.focus.wait_move()
      if(resp != 1):
         print "Focus motor error. "
         return
      
      #configure for the fast frames
      self.ccd47.write_fifoch(fast_set)
      visao_wait(.5)
      
      self.ccd47.exptime(exptime_s)
      
      #set subdirectories
      f = get_visao_filename_now('shuttercal_short')
      self.ccd47.subdir('47_' + f)
      self.fw39.take_control(1)
      self.fw39.subdir('39_' + f)
      
      #take fast darks
      self.take_darks(fast_darks)
      self.shutter.open()
      
      #start saving: BCU 39 continuous, while ccd47 takes a set number of images
      self.bcu39.save(-1)
      
      self.take_science(int(tottime/exptime_s+0.99999999))
      
      self.bcu39.save(0)
      
      #configure for slow frames
      self.ccd47.write_fifoch(slow_set)
      visao_wait(.5)
      
      #set subdirectories
      f = get_visao_filename_now('shuttercal_long')
      self.ccd47.subdir('47_' + f)
      self.fw39.take_control(1)
      self.fw39.subdir('39_' + f)
      
      self.take_darks(slow_darks)
      self.shutter.open()
      
      #start saving: BCU 39 continuous, while ccd47 takes a set number of images
      self.bcu39.save(-1)
      
      self.take_science(int(tottime/exptime_l+0.99999999))
      
      self.bcu39.save(0)
      
      visao_script_complete()
      visao_alert(1)
         
   def take_ron(self, nf2500, nf250, nf80):
   
      self.shutter.shut()

      if nf2500 > 0:
         #2500, 1024x1024, 1x1,  gain=0 
         print '******************\n2500 kHz, 1024x1024, 1x1, gain 0\n******************\n'
         self.ccd47.write_fifoch('set 0 0 0 0')
         sd = get_visao_filename_now("ron2500_g0")
         self.ccd47.subdir(sd)
         visao_wait(1.)
         self.ccd47.save(nf2500, 2)
         self.ccd47.wait_save()
      
         #2500, 1024x1024, 1x1,  gain=0 
         print '******************\n2500 kHz, 1024x1024, 1x1, gain 1\n******************\n'
         self.ccd47.write_fifoch('set 0 0 1 0')
         sd = get_visao_filename_now("ron2500_g1")
         self.ccd47.subdir(sd)
         visao_wait(1.)
         self.ccd47.save(nf2500, 2)
         self.ccd47.wait_save()
      
         #2500, 1024x1024, 1x1,  gain=0 
         print '******************\n2500 kHz, 1024x1024, 1x1, gain 2\n******************\n'
         self.ccd47.write_fifoch('set 0 0 2 0')
         sd = get_visao_filename_now("ron2500_g2")
         self.ccd47.subdir(sd)
         visao_wait(1.)
         self.ccd47.save(nf2500, 2)
         self.ccd47.wait_save()
      
         #2500, 1024x1024, 1x1,  gain=0 
         print '******************\n2500 kHz, 1024x1024, 1x1, gain 3\n******************\n'
         self.ccd47.write_fifoch('set 0 0 3 0')
         sd = get_visao_filename_now("ron2500_g3")
         self.ccd47.subdir(sd)
         visao_wait(1.)
         self.ccd47.save(nf2500, 2)
         self.ccd47.wait_save()
            
      if nf250 > 0:
         #250, 1024x1024, 1x1,  gain=0 
         print '******************\n250 kHz, 1024x1024, 1x1, gain 0\n******************\n'
         self.ccd47.write_fifoch('set 0 2 0 0')
         sd = get_visao_filename_now("ron250_g0")
         self.ccd47.subdir(sd)
         visao_wait(3.)
         self.ccd47.save(nf250, 2)
         self.ccd47.wait_save()
      
         #250, 1024x1024, 1x1,  gain=1 
         print '******************\n250 kHz, 1024x1024, 1x1, gain 1\n******************\n'
         self.ccd47.write_fifoch('set 0 2 1 0')
         sd = get_visao_filename_now("ron250_g1")
         self.ccd47.subdir(sd)
         visao_wait(3.)
         self.ccd47.save(nf250, 2)
         self.ccd47.wait_save()
      
         #250, 1024x1024, 1x1,  gain=2 
         print '******************\n250 kHz, 1024x1024, 1x1, gain 2\n******************\n'
         self.ccd47.write_fifoch('set 0 2 2 0')
         sd = get_visao_filename_now("ron250_g2")
         self.ccd47.subdir(sd)
         visao_wait(3.)
         self.ccd47.save(nf250, 2)
         self.ccd47.wait_save()
      
         #250, 1024x1024, 1x1,  gain=3 
         print '******************\n250 kHz, 1024x1024, 1x1, gain 3\n******************\n'
         self.ccd47.write_fifoch('set 0 2 3 0')
         sd = get_visao_filename_now("ron250_g3")
         self.ccd47.subdir(sd)
         visao_wait(3.)
         self.ccd47.save(nf250, 2)
         self.ccd47.wait_save()
      
      if nf80 > 0:      
         #80, 1024x1024, 1x1,  gain=0 
         print '******************\n80 kHz, 1024x1024, 1x1, gain 0\n******************\n'
         self.ccd47.write_fifoch('set 0 4 0 0')
         sd = get_visao_filename_now("ron80_g0")
         self.ccd47.subdir(sd)
         visao_wait(3.)
         self.ccd47.save(nf80, 2)
         self.ccd47.wait_save()
      
         #80, 1024x1024, 1x1,  gain=1 
         print '******************\n80 kHz, 1024x1024, 1x1, gain 1\n******************\n'
         self.ccd47.write_fifoch('set 0 4 1 0')
         sd = get_visao_filename_now("ron80_g1")
         self.ccd47.subdir(sd)
         visao_wait(3.)
         self.ccd47.save(nf80, 2)
         self.ccd47.wait_save()
      
         #80, 1024x1024, 1x1,  gain=2 
         print '******************\n80 kHz, 1024x1024, 1x1, gain 2\n******************\n'
         self.ccd47.write_fifoch('set 0 4 2 0')
         sd = get_visao_filename_now("ron80_g2")
         self.ccd47.subdir(sd)
         visao_wait(3.)
         self.ccd47.save(nf80, 2)
         self.ccd47.wait_save()
      
         #80, 1024x1024, 1x1,  gain=3 
         print '******************\n80 kHz, 1024x1024, 1x1, gain 3\n******************\n'
         self.ccd47.write_fifoch('set 0 4 3 0')
         sd = get_visao_filename_now("ron80_g3")
         self.ccd47.subdir(sd)
         visao_wait(3.)
         self.ccd47.save(nf80, 2)
         self.ccd47.wait_save()
         
         
      return