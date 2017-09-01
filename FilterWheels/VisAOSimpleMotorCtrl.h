/************************************************************
*    VisAOSimpleMotorCtrl.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the VisAOSimpleMotorCtrl class, which adapts
* the SimpleMotorCtrl class for use as a VisAO App.
* 
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOSimpleMotorCtrl.h
  * \author Jared R. Males
  * \brief Declarations for the VisAOSimpleMotorCtrl class. 
  * 
*/

#ifndef __VisAOSimpleMotorCtrl_h__
#define __VisAOSimpleMotorCtrl_h__

#include "SimpleMotorCtrl.h"
#include "VisAOFilterWheel.h"

#include "VisAOApp_base.h"

#include <cmath>


namespace VisAO
{

/// A class to provide VisAO functionality to the adopt motor controller.
/** Uses the control logic unchanged from SimpleMotorCtrl, and adds the
  * VisAOApp_base command fifos for local and script control using a signal thread.
  * No remote fifo is provided, as remote control is via the RTDB.  Intercepts
  * the PosReqChanged notification to test if in remote control, and provides
  * the facility for changing control mode from RTDB.
*/ 
class VisAOSimpleMotorCtrl : public SimpleMotorCtrl
{
   public:
      ///Standard adopt style config file constructor.
      VisAOSimpleMotorCtrl( std::string name, const std::string &conffile) throw (AOException);
   
      ///Standard adopt style command line constructor.
      VisAOSimpleMotorCtrl( int argc, char **argv) throw (AOException);
   
   protected:
      RTDBvar var_cmode_cur; ///<The current control mode in the RTDB.
      RTDBvar var_cmode_req; ///<The requested control mode in the RTDB.
   
      double cur_pos;
     
      ///An iterator for accessing the custom pos list.
      map<std::string, float>::iterator cpos_it;
   
      /// Custom filter types list
      multimap<int, float> _customTypes;
      ///An iterator for accessing the custom pos list.
      multimap<int, float>::iterator ctype_it;

      /// Virtual function to setup variables in RTDB
      void SetupVars();

      /// Overridden to test if this is a VisAOFilterWheel.
      /** Handle notifications if it is.  If  not this behaves
        * as a standard adopt SimpleMotorCtrl.
        */
      virtual SimpleMotor *CreateMotor();

      /// Update position variables in RTDB
      virtual void updatePos( bool force=false);
      
      ///Get the position name from wheel position
      std::string GetPosName(double pos);
      
      ///Get the position type from the wheel position
      int GetType(double pos);
      
   public:
      ///The current control mode.
      /** Is static so it can be used by static RTDB handlers.
       */
      static int control_mode;
      
      ///The default control mode.
      /** If not -1, then this mode is always returned to, instead of NONE.
       */
      static int default_control_mode;
      
      ///The control modes.
      enum control_modes{CMODE_NONE, CMODE_REMOTE, CMODE_LOCAL, CMODE_SCRIPT, CMODE_AUTO, CMODE_max};
      
      virtual int request_control(int cmode); ///< Calls \ref request_control(int,bool) "request_control(cmode, 0)"
      
      ///Attempts to change the control type.
      /** \param cmode The requested control mode \see control_modes.
       * \param override If not true, forces the change no matter what current mode is.
       * \retval control_mode if successful.
       * \retval -1 if unsuccessful.
       */
      virtual int request_control(int cmode, bool override);
   
   public:
      /// RTDB handler for a control mode change request from the AO Supervisor.
      static int CModeReqChanged(void *pt, Variable *msgb);

      /// RTDB handler for a position change by the AO Supervisor
      /** After checking if control_mode == CMODE_REMOTE, just calls
        * SimpleMotorCtrl::PosReqChanged.
        */
      static int VisAO_PosReqChanged( void *pt, Variable *msgb);

      /// RTDB handler for a position change by the AO Supervisor
      /** After checking if control_mode == CMODE_LOCAL, just calls
       * SimpleMotorCtrl::PosReqChanged.
       */
      static int VisAO_PosLocalReqChanged( void *pt, Variable *msgb);

      /// RTDB handler for a position change by the AO Supervisor
      /** After checking if control_mode == CMODE_SCRIPT, just calls
       * SimpleMotorCtrl::PosReqChanged.
       */
      static int VisAO_PosScriptReqChanged( void *pt, Variable *msgb);
      
      /// Exposes the abort Position (_abortPos) for use in PosReqChanged (which is static)
      double getAbortPos(){ return _abortPos;}

};

} //namespace VisAO

#endif //__VisAOSimpleMotorCtrl_h__


