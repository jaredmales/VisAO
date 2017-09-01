/************************************************************
*    VisAOJoeCtrl.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for CCD47 control from the AO Supervisor.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOJoeCtrl.h
  * \author Jared R. Males
  * \brief Declarations for CCD47 control from the AO Supervisor. 
  * 
  * Based on the Arcetri JoeCtrl class.
  * 
*/

#ifndef __VisAOJoeCtrl_h__
#define __VisAOJoeCtrl_h__

#include <vector>
#include <string>

#include "stdconfig.h"
#include "base/thrdlib.h"
#include "AOApp.h"
#include "AOStates.h"

#include "CCD47Ctrl.h"

#define _debug

namespace VisAO
{
   
///The interface between the \ref CCD47Ctrl class and the AO Supervisor. 
/** Based on the Arcetri JoeCtrl class, this class works in nearly identical fashion
  * except it passes commands to the CCD47Ctrl process instead of to the little joe.
  */ 
class JoeCtrl: public AOApp, public VisAOApp_base
{

   public:
      ///Config file constructor
      JoeCtrl( std::string name, const std::string &conffile) throw (AOException);

      ///Command line constructor
      JoeCtrl( int argc, char **argv) throw (AOException);

   protected:
      /// CCD number (set from cfg file)
      int  _ccdNum; 

      /// CCD name
      std::string _ccdName;

      // CCD parameters
      int          _ccdDx; ///< Size in x
      int          _ccdDy; ///< Size in y
      unsigned int _ccdNumSpeeds; ///<Number of speeds
      int          _maxNumSpeeds; ///<Maximum number of speeds
      int          _maxNumBins; ///<Maximum number of binnings
      int          _maxNumWins;
      std::vector<int>  _ccdXbins; ///<Vector of x binnings
      std::vector<int>  _ccdYbins; ///<Vector of y binnings
      std::vector<int>  _ccdSpeeds; ///<Vector of speeds
      std::vector<int>  _ccdBlacks; ///<Vector of black levels
      int          _ccdBlacksNum; ///<Nuber of blacks.
      int          _minRep; ///<Minimum repititions (controls integration time)
      int          _maxRep; ///<Maximum repititions (controls integration time)

      int cur_ProgramSet; ///<The Current program set loaded on the little joe
      int cur_Program; ///<The current program running on the little joe
      int cur_Gain; ///<The current gain running on the little joe
      int cur_Reps; ///<The current repititions running on the little joe

      // Default parameters
      int _startProgramSet; ///<The program set currently set to load from flash on little joe power up
      int _ccdDefaultXbin; ///<Default x binning 
      int _ccdDefaultYbin; ///<Default y binning
      int _ccdDefaultSpeed; ///<Default speed
      int _ccdDefaultBlack; ///<Default black levels

      /// Struct representing LittleJoe's internal memory
      littlejoe_programset memory;
 
      /// Struct array representing files that can be uploaded to LittleJoe (each one is a memory dump)
      std::vector<littlejoe_programset> ondisk;

      // RTDB variables

      RTDBvar var_name, var_status, var_errmsg, var_enable_cur, var_enable_req;
      RTDBvar var_dx, var_dx_cur, var_dy, var_dx_req, var_dy_cur,var_dy_req, var_windowxs, var_windowys,var_xbins, var_ybins, var_speeds;
      RTDBvar var_xbin_cur, var_ybin_cur, var_speed_cur, var_black_cur;
      RTDBvar var_xbin_req, var_ybin_req, var_speed_req, var_black_req;
      RTDBvar var_temps, var_framerate_cur, var_framerate_req;
      RTDBvar var_rep_cur, var_rep_req;
      RTDBvar var_gain_cur, var_gain_req;

      RTDBvar var_fanReq;

      RTDBvar var_cmode_cur; ///<The current control mode in the RTDB.
      RTDBvar var_cmode_req; ///<The requested control mode in the RTDB.

      RTDBvar var_preset_req; ///<Preset to program set 0, program 0, for presetVisAO.
      
      // Temperature thresholds for fan on/off
      int _fanCtrlActive;
      int _fanOnTemp;
      int _fanOffTemp;

   protected:
      ///Read the JoeCtrl specific config vars and connect to the CCD47Ctrl
      void Create(void) throw (AOException);

       ///VIRTUAL - Perform post-initialization settings
      void PostInit();
      
      ///VIRTUAL - Setups variables in RTDB
      void SetupVars(void);
      
      ///VIRTUAL - StateChange
      /** Used to invalidate CUR variables when losing connection
        * Automatically called by AOApp when a state change occurs.
        * Used here to invalidate "CUR" variables when losing connections
        */
      void StateChange( int oldstate, int state);

      ///VIRTUAL - Run
      /**Switches the stage from one state to the other
        */
      void Run();

      ///Perform controller functions and manage states
      /**Switches the controller from one state to another. States can be changed asynchronously
        * from this thread or the listening thread, and this function will properly react to a new state.
        * Many states include a msleep() function to slow down the thread when immediate action is not necessary
        */
      int DoFSM(void);      

      ///Send a start command to the CCD47Ctrl
      int Start();
      
      ///Send a stop command to the CCD47Ctrl
      int Stop();
      
      ///Calculate and send a reprogram request to the CCD47Ctrl
      int ReprogramJoe( int force=0);

      ///Test the link with the CCD47Ctrl, and get its state.
      int TestCCD47CtrlLink();

      ///Send a command to the CCD47 and get its response.
      std::string send_ccd47_command(std::string com);
      
      int GetProgramPos( int speed, int xbin, int ybin, int *need_upload);
      
      int UpdateJoeMemory( unsigned int uploaded_file);
      
      int LoadJoeDiskFiles(void);
      
      int EraseLocalMemory(void);
      
      int SetLocalMemory( unsigned int programset_num);

      int ComputeFramerate();
   
      int ChangeFramerate( double framerate, int bestspeed=1);
   
      littlejoe_program *GetProgram( int speed, int xbin, int ybin);
   
      int ExposeSpeeds();

      int ExposeWindows();

      int insert_value( std::vector<int> &array, int value);

      littlejoe_program ReadProgram( Config_File &cfg);
      
      littlejoe_programset ReadProgramSet( Config_File &cfg);

      /// RTDB handler for an enable change request form the AO Supervisor
      static int EnableReqChanged( void *pt, Variable *var);
      
      /// RTDB handler for an x-bin change request form the AO Supervisor
      static int XbinReqChanged( void *pt, Variable *var);
      
      /// RTDB handler for a y-bin change request form the AO Supervisor
      static int YbinReqChanged( void *pt, Variable *var);
      
      /// RTDB handler for a speed change request form the AO Supervisor
      static int SpeedReqChanged( void *pt, Variable *var);
      
      /// RTDB handler for a black level change request form the AO Supervisor
      static int BlackReqChanged( void *pt, Variable *var);
      
      /// RTDB handler for a frame rate change request form the AO Supervisor
      static int FrameRateReqChanged( void *pt, Variable *var);
      
      /// RTDB handler for a repetitions change request form the AO Supervisor
      static int RepReqChanged( void *pt, Variable *var);

      /// RTDB handler for a gain change request form the AO Supervisor
      static int GainReqChanged( void *pt, Variable *var);

      /// RTDB handler for a control mode change request from the AO Supervisor.
      static int CModeReqChanged(void *pt, Variable *msgb);

      /// RTDB handler for preset request from the AO Supervisor.
      static int PresetReqChanged(void *pt, Variable *msgb);
      
      ///Gets the complete path of a program/pattern file
      std::string getCompletePath( std::string filename);
      
};

} //namespace VisAO

#endif //__VisAOJoeCtrl_h__

