/************************************************************
*    FilterWheelLocal.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the FilterWheelLocal class, which interfaces
* with the VisAOSimpleMotorCtrl class via the MsgD.
* 
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file FilterWheelLocal.h
  * \author Jared R. Males
  * \brief Declarations for the FilterWheelLocal class, which interfaces with the VisAOSimpleMotorCtrl class via the MsgD.
  * 
*/

#ifndef __FilterWheelLocal_h__
#define __FilterWheelLocal_h__

#include "AOApp.h"
#include "AOStates.h"

#include "VisAOApp_base.h"

#include <cmath>


namespace VisAO
{

/// A class to provide Filter Wheel functionality for the VisAO Camera.
/** Exposes Local and Script fifos, and passes commands on to the
  * F/W controllers on the supervisor via the MsgD. 
  */ 
class FilterWheelLocal : public AOApp, public VisAOApp_base
{
   
public:
   ///Standard adopt style config file constructor.
   FilterWheelLocal( std::string name, const std::string &conffile) throw (AOException);
   
   ///Standard adopt style command line constructor.
   FilterWheelLocal( int argc, char **argv) throw (AOException);
   
protected:
   ///Basic setup of the class, called by both constructors.
   /** Reads the com_path parameter from config, and sets up the
    * \ref fifo_list to have local and script command fifos.
    * Also established the COMMODE RTDB varialbe and its notifier.
    */
   void setupVisAOApp();

   ///Read the corresponding filter wheel controller configuration file.
   void readFilterConfig();
   
   RTDBvar var_status;///<The current status of the controller, STATE_CONNECTED, STATE_OPERATING,STATE_HOMING, etc.

   RTDBvar var_cmode_cur; ///<The current control mode in the RTDB.
   RTDBvar var_cmode_req; ///<The requested control mode in the RTDB.
   
   RTDBvar var_pos_cur; ///<The current position
   RTDBvar var_pos_req; ///<The requested position

   RTDBvar var_pos_local_req;///<Position requester for local control
   RTDBvar var_pos_script_req; ///<Position requester for script control

   double _abortPos;
   double _homingPos;
   float _startingPos;

   map<string, float>  _customPos;
   int cur_state;
   double cur_pos;
   double req_pos;

   std::string aoapp_name;

   /// Virtual function to setup variables in RTDB
   void SetupVars();
   
   ///An iterator for accessing the custom pos list.
   map<std::string, float>::iterator cpos_it;
   
   /// Custom filter types list
   multimap<int, float> _customTypes;
   ///An iterator for accessing the custom pos list.
   multimap<int, float>::iterator ctype_it;

   ///Get the position name from wheel position
   std::string GetPosName(double pos);

   ///Get the position type from the wheel position
   int GetType(double pos);
   
   ///The main loop.
   void Run();

   /// Local FSM
   int DoFSM();

   /// Update position variables in RTDB
   virtual void updatePos( double pos);
   /// Update req position variables in RTDB
   virtual void updateReq( double pos);

   void set_control_mode(int cmode);
   void set_status(int stat);

   pthread_t signal_thread; ///< The thread where fifo async signals are caught.

   public:
      ///Signal loop, normally won't need to be overridden.
      /** This does need to be started by the Run() function if it is desired.
        * Before starting, must have setup and connected the fifo list.
        * Can be started with start_signal_catcher().  The main loop
        * should then block SIGIO and RTSIGIO.
        */
      void signal_catcher();
  
   protected:
      ///Starts the signal catching loop
      /** \param inherit_sched if true (default) then the signal_catcher is started with the same RT priority as the main loop.
        */
      virtual int start_signal_catcher(bool inherit_sched = true);

   
   public:

      virtual int request_control(int cmode); ///< Calls \ref request_control(int,bool) "request_control(cmode, 0)"
      
      ///Attempts to change the control type.
      /** \param cmode The requested control mode \see control_modes.
       * \param override If not true, forces the change no matter what current mode is.
       * \retval control_mode if successful.
       * \retval -1 if unsuccessful.
       */
      virtual int request_control(int cmode, bool override);


   /// RTDB handler for a control mode change request from the AO Supervisor.
   static int CModeCurChanged(void *pt, Variable *msgb);
   
   /// RTDB handler for a position change by the AO Supervisor
   /** After checking if control_mode == CMODE_REMOTE, just calls
     * SimpleMotorCtrl::PosReqChanged.
     */
   static int PosCurChanged( void *pt, Variable *msgb);

   static int PosReqChanged( void *pt, Variable *msgb);

   static int StatusChanged( void *pt, Variable *msgb);

   /// An interface to SimpleMotorCtrl::PosReqChanged for the fifo command processors.
   /** We don't actually reimplement any motor control logic.
     */
   int ChangePos(double pos);

   /// Exposes the abort Position (_abortPos) for use in PosReqChanged (which is static)
   double getAbortPos(){ return _abortPos;}

public:
   ///VisAO remote command fifo handler.
   /** Though no fifo is created, this is used to process the
     * RTDB control mode changes.
     */
   virtual std::string remote_command(std::string com);
   
   ///VisAO local command fifo handler
   virtual std::string local_command(std::string com);
   
   ///VisAO script command fifo handler
   virtual std::string script_command(std::string com);
   
   ///Where the command logic actually happens, since it is common.
   std::string common_command(std::string com, int);

   ///Produce the state string for response to state?
   std::string get_state_str();

   /// Update the status board.
   /** Calls this as VisAOApp_base::update_statusboard so the basics are taken care of.
     * \retval 0 on success
     * \retval -1 on failure
     */
   virtual int update_statusboard();
};

} //namespace VisAO

#endif //__FilterWheelLocal_h__


