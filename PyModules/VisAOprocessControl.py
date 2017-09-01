from AdOpt import processControl
import os


def stopProcessByName( process, kill= False, verbose=True):
   '''
   Stops a process, given its name in the processList configuration file,
   using a TERMINATE message or a kill -9.
   Proper terminating requires calling this function from an AOApp.
   '''

   p = processControl.getProcess(process)

   if p:
      id = processControl.getProcessID(process)
      if id != 0:
         if kill:
            #print 'ID=',id
            command = "/bin/kill -s 9 "+str(id) # Shouldn be -9 !!!
            os.system( command)
            if verbose: print 'stopProcessByName(): process %s killed' % process
            return True
         else:
            #Otherwise attempt to SIGTERM
            command = "/bin/kill " + str(id)
            os.system( command)
            if verbose: print 'stopProcessByName(): SIGTERM sent to process %s' % process
            return True
      else:
         if verbose: print 'stopProcessByName(): process %s down' % process
         return False


   else:
      print 'stopProcessByName(): process %s not found in configuration!' % process
      return False