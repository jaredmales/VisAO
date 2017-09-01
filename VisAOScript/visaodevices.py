##@file   visaodevices.py
#
#   The visaodevices python module provides interfaces to all of the scriptable VisAO processes.
#
#   Each process has a class dervied from VisAOFifoDev
#


"""@module   visaodevices

   The visaodevices python module provides interfaces to all of the scriptable VisAO processes.

   Each process has a class dervied from VisAOFifoDev
"""


from visaofifos import *

      
class FocusCtrl(VisAOFifoDev):
   """
      Class to control the Focus stage.
   """
   def __init__(self):
      self.base_name = 'focusmotor'
      self.setup_fifo_names()
      self.connected = 0
      self.control = 0

   def get_pos(self):
      """
         Get the current position of the stage.
      """
      if self.connected != 1:
         self.connect()
      ans = self.write_fifoch('pos?')
      return float(ans.rstrip('\n\x00'))

   def abort(self):
      """
         Abort a move in progress.
      """
      if self.connected != 1:
         self.connect()
      resp = int(self.write_fifoch('abort').rstrip('\n\x00'))
      return resp
       
   def pos(self, newpos):
      """
         Move to a new position, specified in microns.
      """
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return -1

      argstr = 'pos ' + str(newpos)
      resp = self.write_fifoch(argstr).rstrip('\n\x00')
   
      if resp != '1':
         print 'Error from ' + self.base_name

      return int(resp)

   def preset(self):
      """
         Move to a preset focus position based on the filter selection.
      """
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return -1


      resp = self.write_fifoch("preset").rstrip('\n\x00')
   
      if resp != '0':
         print 'Error from ' + self.base_name

      return int(resp)
     
   def wait_move(self):
      """
         Wait for a move to complete.
      """
      if self.connected != 1:
         self.connect()

      time.sleep(0.1)
      stat = int(self.write_fifoch('ismoving?').rstrip('\n\x00'))

      while stat != 0:
         time.sleep(0.1)
         stat = int(self.write_fifoch('ismoving?').rstrip('\n\x00'))

      return 1

class CCD47Ctrl(VisAOFifoDev):
   """
      Class to control the ccd47.
   """
   def __init__(self):
      self.base_name = 'ccd47ctrl'
      self.setup_fifo_names()
      self.connected = 0
      self.control = 0

   def get_state(self):
      """
         Get the current state of the ccd47 controller.
      """
      if self.connected != 1:
         self.connect()
      stat = self.write_fifoch('state?').rstrip('\n\x00')

      return stat

   def stop(self):
      if self.connected != 1:
         self.connect()
      stat = self.write_fifoch('stop').rstrip('\n\x00')

      return stat
      
   def start(self):
      if self.connected != 1:
         self.connect()
      stat = self.write_fifoch('start').rstrip('\n\x00')

      return stat
      
   def save(self, n, im_type=0):
      """
         Save data.
         n = -1 save continuously
         n = 0 stop saving
         n = >0 save n images
      """
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return 'error'
         
      self.imtype(im_type)
      visao_wait(.1)
      
      argstr = 'save ' + str(n)
      
      stat = int(self.write_fifoch(argstr).rstrip('\n\x00'))
      
      return stat
   
   def savescience(self, n):
      """
         Save science data.
         n = -1 save continuously
         n = 0 stop saving
         n = >0 save n images
      """
      
      return self.save(n, 0)
   
   def savedark(self, n):
      """
         Save data.
         n = -1 save continuously
         n = 0 stop saving
         n = >0 save n images
      """
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return 'error'
         
      self.imtype(2)
      visao_wait(.1)
      
      argstr = 'savedark ' + str(n)
      
      stat = int(self.write_fifoch(argstr).rstrip('\n\x00'))
      
      return stat
      
   def wait_save(self):
      """
         Wait for a save sequence to complete.
      """
      if self.connected != 1:
         self.connect()

      time.sleep(0.3)
      stat = int(self.write_fifoch('save?').rstrip('\n\x00'))

      while stat != 0:
         time.sleep(0.3)
         stat = int(self.write_fifoch('save?').rstrip('\n\x00'))

      return 1

   def subdir(self, sd):
      """
         Change to a new sub-directory for saving.
      """
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return 'error'
      com = "subdir %s" % sd
      res = self.write_fifoch(com)

      return res

   def imtype(self, it):
      """
         Change image type
      """
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return 'error'
      com = "imtype %s" % it
      res = int(self.write_fifoch(com).rstrip('\n\x00'))

      return res
 
   def reps(self, rp):
      """
         Set ccd 47 accumulator reps
      """
      argstr = 'reps ' + str(rp)
      resp = self.write_fifoch(argstr).rstrip('\n\x00')
      #pause for program load
      visao_wait(.2)
      
   def exptime(self, et):
      """
         Set ccd 47 exposure time
      """
      argstr = 'exptime ' + str(et)
      resp = self.write_fifoch(argstr).rstrip('\n\x00')
      #pause for program load
      visao_wait(.2)
         
   def get_framerate(self):
      """
         Get current framerate
      """
      if self.connected != 1:
         self.connect()
      fr = self.write_fifoch('framerate?').rstrip('\n\x00')
      return float(fr) 
      
   def get_reps(self):
      """
         Get current repititions
      """
      if self.connected != 1:
         self.connect()
      fr = self.write_fifoch('rep?').rstrip('\n\x00')
      return float(fr) 
      
   def get_subdir(self):
      """
         Get current subdirectory
      """
      if self.connected != 1:
         self.connect()
      fr = self.write_fifoch('subdir?').rstrip('\n\x00')
      return fr 
      
   def set_program(self, pixrate, window, gain, etime):
      """
         Set the CCD47 program.
         
         pixrate = integer, the pixel rate.  Must be one of 2500, 250, or 80.
         window = integer, the window size, depending on pixel rate can be 1024, 512, 256, 64,32
         gain = string the gain.  Choices are 'H', 'MH', 'ML', or 'L'
         etime = exposure time, in seconds.  0 is the minimum exposure time for the other settings
      """
      
      pset = -1
      pno = -1
      pgain = -1
      preps = 0
      
      if pixrate == 2500:
         if window == 1024:
            pset = 0
            pno = 0
         if window == 64:
            pset = 0
            pno = 1
         if window == 512:
            pset = 1
            pno = 0
         if window == 32:
            pset = 1
            pno = 1
      if pixrate == 250:
         if window == 1024:
            pset = 0
            pno = 2
         if window == 512:
            pset = 1
            pno = 2
      
      if gain == 'H':
         pgain = 0
      if gain == 'MH':
         pgain = 1
      if gain == 'ML':
         pgain = 2
      if gain == 'L':
         pgain = 3
         
      
      if pset == -1 or pno == -1 or pgain == -1:
         raise "CCD47Ctrl: unknown or unsupported program"
      
      
      pstr = 'set %d %d %d 0' % (pset, pno, pgain)
      
      if self.connected != 1:
         self.connect()
        
      st = self.get_state()
      if st[0] != 'S':
         raise "CCD47Ctrl: you do no have SCRIPT control of CCD47"
               
      self.write_fifoch(pstr)
      time.sleep(0.25)
      
      st = self.get_state()
      
      print 'CCD47Ctrl: reprogramming little joe'
      
      while(st[2] == '3'):
         time.sleep(1)
         st = self.get_state()
       
      print 'CCD47Ctrl: little joe reprogrammed'
      
      self.exptime(etime)
      
      time.sleep(0.25)
      
      return 0
         
class FilterWheel(VisAOFifoDev):
   """"
      Class to control the a filter wheel.  You should use the FilterWheel2 or FilterWheel3 for a specific device.
   """
   def get_filter(self):
      """
         Get the current filter.
      """
      if self.connected != 1:
         self.connect()
      filt = self.write_fifoch('filter?').rstrip('\n\x00')

      return filt

   def set_filter(self, filt):
      """
         Set filter to filt, moves the wheel.

         filt should exactly match the names of filter used in its conf file (e.g. "SDSS z'")
      """
      if self.connected != 1:
         self.connect()
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return 'error'
      
      com = "filter %s" % filt
      resp = self.write_fifoch(com).rstrip('\n\x00')

      return resp

   def home(self):
      """
         Home the wheel.
      """
      if self.connected != 1:
         self.connect()

      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return 'error'
      
      resp = self.write_fifoch("home").rstrip('\n\x00')

      return resp
      
   def wait_move(self):
      """
         Wait for a filter wheel move to complete.
      """
      if self.connected != 1:
         self.connect()

      time.sleep(1.)
      stat = int(self.write_fifoch('moving?').rstrip('\n\x00'))
      #print "Moving: %i" % stat
      
      while stat == 1:
         time.sleep(0.1)
         stat = int(self.write_fifoch('moving?').rstrip('\n\x00'))
         #print "Moving: %i" % stat
      return stat

   def wait_home(self):
      """
         Wait for a filter wheel home to complete.
      """
      if self.connected != 1:
         self.connect()

      time.sleep(0.5)
      stat1 = int(self.write_fifoch('homing?').rstrip('\n\x00'))
      while stat1 == 1:
         time.sleep(0.1)
         stat1 = int(self.write_fifoch('homing?').rstrip('\n\x00'))
         

      time.sleep(0.5)
      stat2 = int(self.write_fifoch('moving?').rstrip('\n\x00'))
      while stat2 == 1:
         time.sleep(0.1)
         stat2 = int(self.write_fifoch('moving?').rstrip('\n\x00'))
         
      if stat1 != 0:
        return stat1

      if stat2 != 0:
        return stat2

      return 0
      
class FilterWheel2(FilterWheel):
   """
      Class to control the FilterWheel2.
   """
   def __init__(self):
      self.base_name = 'filterwheel2Local'
      self.setup_fifo_names()
      self.connected = 0
      self.control = 0

class FilterWheel3(FilterWheel):
   """
      Class to control the FilterWheel2.
   """
   def __init__(self):
      self.base_name = 'filterwheel3Local'
      self.setup_fifo_names()
      self.connected = 0
      self.control = 0
 
class Shutter(VisAOFifoDev):
   """
      Class to control the Shutter.
   """
   def __init__(self):
      self.base_name = 'shuttercontrol'
      self.setup_fifo_names()
      self.connected = 0
      self.control = 0
      
   def get_state(self):
      """
         Get the current state of the shutter.
      """
      if self.connected != 1:
         self.connect()
      shutstate = self.write_fifoch('state?').rstrip('\n\x00')
      return int(shutstate[2]+shutstate[3]) #it is either 1 or -1


   def open(self):
      """
         Open the shutter.
      """
      if self.connected != 1:
         self.connect()
      self.write_fifoch('open')

   def shut(self):
      """
         Close the shutter.
      """
      if self.connected != 1:
         self.connect()
      self.write_fifoch('close')
      
class Gimbal(VisAOFifoDev):
   def __init__(self):
      self.base_name = 'gimbal'
      self.setup_fifo_names()
      self.connected = 0
      self.control = 0
           
   def get_xpos(self):
      if self.connected != 1:
         self.connect()
      xpos = self.write_fifoch('xpos?').rstrip('\n\x00')
      return float(xpos) #it is either 1 or -1

   def get_ypos(self):
      if self.connected != 1:
         self.connect()
      ypos = self.write_fifoch('ypos?').rstrip('\n\x00')
      return float(ypos)

   def move_xabs(self, newx):
      if self.connected != 1:
         self.connect()
         
      argstr = 'xabs ' + str(newx)
      resp = self.write_fifoch(argstr).rstrip('\n\x00')

      if resp == 'N' or resp == 'L' or resp == 'A':
        print 'Do not have SCRIPT control of ' + self.base_name
        return -1
   
      if resp != '0':
         print 'Error from ' + self.base_name
         print 'Response: ' + resp
         return -1
      
      return 0

   def move_yabs(self, newy):
      if self.connected != 1:
         self.connect()
         
      argstr = 'yabs ' + str(newy)
      resp = self.write_fifoch(argstr).rstrip('\n\x00')

      if resp == 'N' or resp == 'L' or resp == 'A':
        print 'Do not have SCRIPT control of ' + self.base_name
        return -1
      
      if resp != '0':
         print 'Error from ' + self.base_name
         print 'Response: ' + resp
         return -1
      
      return 0
      
   def move_xrel(self, dx):
      if self.connected != 1:
         self.connect()
         
      argstr = 'xrel ' + str(dx)
      resp = self.write_fifoch(argstr).rstrip('\n\x00')

      if resp == 'N' or resp == 'L' or resp == 'A':
        print 'Do not have SCRIPT control of ' + self.base_name
        return -1

      if resp != '0':
         print 'Error from ' + self.base_name
         print 'Response: ' + resp
         return -1
      
      return 0
      
   def move_yrel(self, dy):
      if self.connected != 1:
         self.connect()
         
      argstr = 'yrel ' + str(dy)
      resp = self.write_fifoch(argstr).rstrip('\n\x00')

      if resp == 'N' or resp == 'L' or resp == 'A':
        print 'Do not have SCRIPT control of ' + self.base_name
        return -1

      if resp != '0':
         print 'Error from ' + self.base_name
         print 'Response: ' + resp
         return -1
      
      return 0

   def dark(self):
      if self.connected != 1:
         self.connect()
       
      resp = self.write_fifoch('dark').rstrip('\n\x00')
      
      if resp == 'N' or resp == 'L' or resp == 'A':
        print 'Do not have SCRIPT control of ' + self.base_name
        return -1

      #if resp != '0':
         #print 'Error from ' + self.base_name
         #print 'Response: ' + resp
         #return -1
      
      return 0
      
   def center(self):
      if self.connected != 1:
         self.connect()
       
      resp = self.write_fifoch('center').rstrip('\n\x00')
      
      if resp == 'N' or resp == 'L' or resp == 'A':
        print 'Do not have SCRIPT control of ' + self.base_name
        return -1

      #if resp != '0':
         #print 'Error from ' + self.base_name
         #print 'Response: ' + resp
         #return -1
      
      return 0
      
   def wait_move(self):
      """
         Wait for a gimbal move to complete.
      """
      if self.connected != 1:
         self.connect()

      time.sleep(2.)
      statx = int(self.write_fifoch('xmoving?').rstrip('\n\x00'))
      #print statx
      staty = int(self.write_fifoch('ymoving?').rstrip('\n\x00'))
      #print staty
      while statx > 0  or staty > 0:
         time.sleep(0.1)
         statx = int(self.write_fifoch('xmoving?').rstrip('\n\x00'))
         staty = int(self.write_fifoch('ymoving?').rstrip('\n\x00'))

      return statx + staty


class framegrabber39(VisAOFifoDev):
   def __init__(self):
      self.base_name = 'framegrabber39'
      self.setup_fifo_names()
      self.connected = 0
      self.control = 0
      
   def save(self, n):
      """
         Save data.
         n = -1 save continuously
         n = 0 stop saving
         n = >0 save n images
      """
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return 'error'
         
      argstr = 'save ' + str(n)
      
      stat = int(self.write_fifoch(argstr).rstrip('\n\x00'))
      
      return stat
   
   def wait_save(self):
      """
         Wait for a save sequence to complete.
      """
      if self.connected != 1:
         self.connect()

      time.sleep(0.3)
      stat = int(self.write_fifoch('save?').rstrip('\n\x00'))

      while stat != 0:
         time.sleep(0.3)
         stat = int(self.write_fifoch('save?').rstrip('\n\x00'))

      return 1

   def subdir(self, sd):
      """
         Change to a new sub-directory for saving.
      """
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return 'error'
      com = "subdir %s" % sd
      res = self.write_fifoch(com)

      return res

class framewriter39(VisAOFifoDev):
   def __init__(self):
      self.base_name = 'framewriter39'
      self.setup_fifo_names()
      self.connected = 0
      self.control = 0
      

   def subdir(self, sd):
      """
         Change to a new sub-directory for saving.
      """
      if self.control != 1:
         print 'Must take control of ' + self.base_name + ' first.'
         return 'error'
      com = "subdir %s" % sd
      res = self.write_fifoch(com)

      return res