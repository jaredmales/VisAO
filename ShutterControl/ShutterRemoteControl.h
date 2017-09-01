/************************************************************
*    ShutterRemoteControl.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Shutter control from the AO Supervisor.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterRemoteControl.h
  * \author Jared R. Males
  * \brief Declarations for shutter control from the AO Supervisor.  
  * 
*/

#ifndef __ShutterRemoteControl_h__
#define __ShutterRemoteControl_h__

#include "AOApp.h"
#include "AOStates.h"

#include "VisAOApp_base.h"

extern "C" 
{
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>		// atof()
#include <math.h>       // fabs()
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <pthread.h>
}

#include "RTDBvar.h"

#define DEFAULT_WAIT_TO 0.01
#define DEFAULT_WAIT_SLEEP 0.0001

namespace VisAO
{

///Class to provide remote control (via the AO system) of the Shutter.
class ShutterRemoteControl : public AOApp, public VisAOApp_base
{
   public:
      ShutterRemoteControl(std::string name, const std::string &conffile) throw(AOException);
      ShutterRemoteControl( int argc, char **argv) throw(AOException);
      
      static int StateReqChanged(void *pt, Variable *msgb);
      static int CModeReqChanged(void *pt, Variable *msgb);
      int send_shutter_command(const char * com);
      int parse_state_string(std::string ansstr);
      
   protected:
      void Create(void) throw (AOException);
      void updateState(bool force = false);

      std::string fifo_path;
      double wait_to; //timeout for waiting for shutter response
      double wait_sleep; //time to sleep waiting for shutter response

   protected:
      int LoadConfig();

	   // VIRTUAL - Setup variables in RTDB
      void SetupVars();

	   // VIRTUAL - Run
      void Run();
      
      const char *errmsg;
      int  ERRMSG_LEN;

      virtual int TestNetwork(void);

   protected:

      // HANDLERS
     

      // Internal state

      int   _shutter_state;
      int   _shutter_sw_state;
      int   _shutter_hw_state;
      int   _shutter_cmode;
      
      RTDBvar var_state_cur, var_state_req, var_sw_state, var_hw_state;
      RTDBvar var_cmode_cur, var_cmode_req;
      
      pthread_mutex_t mutex;     // Mutex to lock communication
      
      int DoFSM();
      
      
};

} //namespace VisAO

#endif //__ShutterRemoteControl_h__
