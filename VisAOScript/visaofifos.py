##@file   visaofifos.py
#
#   The visaofifos python module provides the basic interface to a VisAO FIFO device.
#
#


"""@module   visaofifos.py

   The visaofifos python module provides the basic interface to a VisAO FIFO device.

"""



import os, time, math, select, errno, signal, numpy

import numpy
from visaoutils import *

      
class VisAOFifoDev:
   """
      A basic VisAO FIFO device.  Establishes basic fifo bichannel communications with a VisAO process.

      Derived classes should provide self.base_name in their __init__ method, and should set connected = 0 and control = 0.
   """
   def setup_fifo_names(self):
      """
         Construct the script fifo paths based on the base_name variable.
      """
      self.base_path = os.environ['VISAO_ROOT']
      self.fifo_out_name = self.base_path + '/fifos/' + self.base_name + '_com_script_in'
      self.fifo_in_name = self.base_path + '/fifos/' + self.base_name + '_com_script_out'

      #print self.fifo_out_name
      #print self.fifo_in_name
      
   def open_fifoch(self):
      """
         Open the script fifo channel to this device.
      """
      self.pollobj = select.poll()
      self.fifo_out = open(self.fifo_out_name, 'w')
      fd = os.open(self.fifo_in_name, os.O_RDONLY|os.O_NONBLOCK)
      self.fifo_in = os.fdopen(fd)
      
      self.pollobj.register(fd, select.POLLIN)

      #Here we clear the input fifo of any waiting garbage.
      while 1:
         try: 
            ready = self.pollobj.poll(1)
         except select.error, v:
            if v[0] != errno.EINTR: raise
         else: break
      if len(ready) > 0: self.fifo_in.read()
      
   def write_fifoch(self, com):
      """
         Write com to the script fifo for this device, read and return the response.
      """
      self.fifo_out.write(com)
      self.fifo_out.flush()
      while 1:
         try: 
            ready = self.pollobj.poll(2000)
         except select.error, v:
            if v[0] != errno.EINTR: raise
         else: break
         
      if len(ready) > 0:
         return self.fifo_in.read()
      else:
         print "timeout"
         return ""
      
   def close_fifoch(self):
      """
         Close the fifo channel.
      """
      self.fifo_out.close()
      self.fifo_in.close()
      self.connected = 0
      
   def connect(self):
      """
         Connect to the script fifos of this device.
      """
      if self.connected == 1:
         self.close_fifoch()
   
      self.open_fifoch()
      self.connected = 1

   def take_control(self, override = 0):
      """
         Take SCRIPT control of this device.

         Input: override Default is 0, no override.  If 1, override.
      """
      if self.connected == 0:
         self.connect()
      if override == 1:
         ans = self.write_fifoch('XSCRIPT')
      else:
         ans = self.write_fifoch('SCRIPT')

      ans = ans.rstrip('\n\x00')
      if ans == 'SCRIPT':
         print 'you have Script Control of ' + self.base_name
         self.control = 1
         return ans
         
      else:
         print 'Error taking Script Control of ' + self.base_name
         print 'Response was: ' + ans
         self.control = 0
         return ans

   def giveup_control(self):
      """
         Give up SCRIPT control of this process, returning to its default.
      """
      if self.connected == 0:
         self.connect()
      ans = self.write_fifoch('~SCRIPT')

      ans = ans.rstrip('\n\x00')
      if ans == 'NONE' or ans == 'REMOTE' or ans == 'LOCAL' or ans == 'AUTO':
         print 'We no longer have Script Control of ' + self.base_name
         self.control = 0
         return ans
         
      else:
         print 'Error relinquishing Script Control of ' + self.base_name
         print 'Response was: ' + ans
         return ans