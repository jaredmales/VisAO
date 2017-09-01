/************************************************************
 *    FocusMotorCtrl.h
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Declarations for the VisAO focus stage stepper motor controller.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file FocusMotorCtrl.h
 * \author Jared R. Males
 * \brief Declarations for the focus stage stepper motor controller.
 *
 */

#ifndef __VisAOFocusMotorCtrl_h__
#define __VisAOFocusMotorCtrl_h__

#include "VisAOApp_standalone.h"
#include "libvisao.h"

#include <fstream>
#include <cmath>

///No pending move
#define MOVE_NONE    0
///Pending offset move
#define MOVE_OFFSET  1
///Pending move to absolute position
#define MOVE_ABS       2

#define MOVE_HOME      3
#define MOVE_NEGHOME   4
#define MOVE_POSHOME   5

///The fifo_list fifo channel for enable
#define FIFOCH_ENABLE 0
///The fifo_list fifo channel for direction
#define FIFOCH_DIR    1
///The fifo_list fifo channel for step
#define FIFOCH_STEP   2

///The fifo_list fifo channel for power
#define FIFOCH_PWR  3

///The fifo_list fifo channel for the front limit
#define FIFOCH_NEGL   4

///The fifo_list fifo channel for the home switch
#define FIFOCH_HOMESW   5

///The fifo_list fifo channel for the back limit
#define FIFOCH_POSL   6

namespace VisAO
{
   

/// The focus stage stepper motor controller class.
/** Manges the digital inputs to the BSD-02 stepper motor controller.
 * Tracks position relative to the home position in microns, as an integer number
 * of steps of size \ref step_ratio.  Is a standalone VisAOApp, and has remote, local
 * and script fifos.  No auto control is provided.  The config file parameters are:
 * - MyName string (required)
 * - ID int (required)
 * - LogLevel string (required)
 * - com_path string (optional)
 * - diofifo_path string (optional)
 * - dioch_enable int (required)
 * - dioch_dir int (required)
 * - dioch_step int (required)
 * - step_ratio double (required)
 * - min_step_time double (required)
 * - hw_dir int (optional)
 */ 
class FocusMotorCtrl : public VisAOApp_standalone
{
public:
   /// Standard constructor with a config file.
   FocusMotorCtrl(std::string name, const std::string& conffile) throw (AOException);
   
   /// Standard constructor with a command line.
   FocusMotorCtrl( int argc, char**argv) throw (AOException);
   
   virtual ~FocusMotorCtrl();
   
   
   /// Initialization common to both constructors.
   /** Reads the app specific config detals, and sets up the fifo_list and connects it.
    * Upon exit, SIGIO is blocked.
    */
   void initapp();
   
protected:
   std::string diofifo_path; ///< The path of the dioserver fifos
   int dioch_enable; ///<The dioserver software channel connected to the enable pin
   int dioch_dir; ///<The dioserver software channel connected to the direction pin
   int dioch_step; ///<The dioserver software channel connected to the step pin
   
   int dioch_pwr; ///<The dioserver software channel connected to +5V terminal
   int dioch_limpos; ///<The dioserver software channel connected to the positive limit switch output
   int dioch_limhome; ///<The dioserver software channel connected to the middle limit switch output
   int dioch_limneg; ///<The dioserver software channel connected to the negative limit switch output

   pthread_mutex_t dio_mutex;

   int power_state; ///Power state of the controller.

   int cur_enabled; ///<Tracks the current setting of the enable pin TTL level
   int DO_ENABLE;///<Flag for main loop.  +1 calls enable, -1 calls disable
   int cur_dir; ///<Tracks the current setting of the direction pin TTL level
   
   double min_step_time; ///<Minimum time between steps, in seconds.  Can't be < 1e-6.
   
   double step_ratio;  ///<Microns per step.  For E35H4N-12-900 with BSD-02H in 1/8 step resolution this is 0.375 
   
   int hw_dir; ///<Sets the polarity of the direction pin relative to the software direction, that is so forward is forward as wired.
   
   double cur_pos; ///<The position, relative to home,  in microns
   
   int pending_move; ///<If non zero, a move is pending.  Can be \ref MOVE_OFFSET or \ref MOVE_ABS.  
   double next_pos; ///<Position of next move. Interpreted as either an offset or absolute position depending on \ref pending_move.
   
   bool is_moving; ///<True if currently processing a position change.
   bool stop_moving; ///<If true, the currently processing position change is terminated.
   
   timeval last_step_time; ///<Time of the last step.
   
   bool neg_limit; ///<True if negative limit switch is active
   bool pos_limit; ///<True if positive limit switch is active
   bool home_switch;///<True if home limit switch is active
   
   bool pos_limit_disabled; ///<True if back limit is disabled for dismounting
   
   int sw_limits_only;
   
   double sw_neg_limit;
   double sw_pos_limit;
   
   double home_pos; ///<The position to set at the after homing to the home switch
   
   int homing; //0 = not homing
   //1 = initial homing, first stage of homing to make sure we aren't on the center switch to start
   //2 = main homing, moving to find the limit switch
   //3 = secondary homing, occurs after finding the limit switch moving backwards, lasts until switch clears 
   //4 = negative homing, moving negative until negative switch trips
   //5 = positive homing, moving positive until back switch trips
   //Center home is defined as having tripped the center switch from the back while moving forward.
   
   int check_limits(); ///<Gets the limit switch status from dioserver, and performs homing logic checks.
   
   VisAO::filterwheel_status_board *fw2sb;
   VisAO::filterwheel_status_board *fw3sb;
   VisAO::wollaston_status_board *wsb;
   VisAO::aosystem_status_board *aosb;
   
public:
   ///Get the current position in microns.
   double get_cur_pos(){return cur_pos;}
   ///Set the current position.  This does not move the motor.
   /** \param cp the new value of the current position.
    * \retval 0 on succes (always)
    */ 
   int set_cur_pos(double);
   
   ///Set the current position as the home position.
   /** Equivalent to set_cur_pos(0).
    */
   int set_home();
   
   //status functions:
   /// Get the power state of the controller
   int get_power_state();
   
   //control functions:
   /// Set the enable state of the controller.
   /** The software state is true=enabled, false=disabled.  The 
    * hardware state is low=enabled, high=disabled.  To enable
    * call with en=true.
    * \param en the desired enable state.
    * \retval 0 on success
    * \retval -1 on failure
    */
   int set_enable(bool en);
   ///Call set_enable(true);
   int enable();
   ///Call set_enable(false);
   int disable();
   
   /// Set the direction of the controller.
   /** The software direction determines whether cur_pos increases or decreases.
    * This also changes the hardware direction by setting the direction pin, but the
    * actual direction of motion is set by the wiring and hw_dir.
    * \param dir 1 for forward, 0 for backward
    * \retval 0 on success
    * \retval -1 on failure
    */
   int set_direction(bool dir);
   ///Call set_direction(true);
   int set_forward();
   ///Call set_direction(false);
   int set_backward();
   
   /// The routine to control motion of the stepper motor.
   /** The BSD-02 steps on a low-high transition.  There is a minimum
    * dwell time for each of 1 microsecond.  This loop also waits at least
    * min_step_time between steps, as part of the low dwell time.
    * Enables the controller prior to first step, and updates cur_pos at each step.  
    * Stops moving and exits normally if stop_moving becomes true.
    * On exit, is_moving, pending_move, and stop_moving are all false
    * and the controller is disabled.
    * \param nsteps is the number of steps to take.
    * \retval 0 on success
    * \retval -1 on failure.
    */
   int step(int nsteps);
   
   /// Move by 1 step in current direction.
   /** Calls step(1)
    * \retval 0 on success
    * \retval -1 on failure.
    */
   int step();
   
   /// Move the motor forward by nsteps.
   /** First sets the direction, then calls step(nsteps).
    * \param nsteps is the number of steps to take.
    * \retval 0 on success
    * \retval -1 on failure.
    */
   int step_forward(int nsteps);
   /// Move by 1 step in forward direction.
   /** Calls step(1)
    * \retval 0 on success
    * \retval -1 on failure.
    */
   int step_forward();
   
   /// Move the motor backward by nsteps.
   /** First sets the direction, then calls step(nsteps).
    * \param nsteps is the number of steps to take.
    * \retval 0 on success
    * \retval -1 on failure.
    */
   int step_backward(int nsteps);
   /// Move by 1 step in backward direction.
   /** Calls step(1)
    * \retval 0 on success
    * \retval -1 on failure.
    */
   int step_backward();
   
   /// Offset current position (in microns).
   /** Calculates the number of steps to move, and direction, then calls
    * either step_forward(nsteps) or step_backward(nsteps) as appropriate.
    * \param dpos the size of the offset desired, in microns
    * \retval 0 on success.
    * \retval -1 on failure.
    */ 
   int offset_pos(double dpos);
   
   /// Move to an absolute position (in microns).
   /** Calculates the number of steps to move, and direction, then calls
    * either step_forward(nsteps) or step_backward(nsteps) as appropriate.
    * \param pos the position to move to, in microns
    * \retval 0 on success.
    * \retval -1 on failure.
    */
   int goto_pos(double pos);
   
   /// Calls either offset_pos(next_pos) or goto_pos(next_pos) when the main loop detects a pending move.
   /** The main loop pause() is interrupted by a SIGIO signal, and once control returns it checks
    * pending_move, and if ture calls this routine.  The move will continue until complete or stopped.
    * \retval 0 on success
    * \retval -1 on failure.
    */            
   int start_pending_move();

   ///Reads the preset file for the current setup as determined from the shared memory status boards.
   /** If a preset file exists for current conditions it is read and the arguments are populated.
    */
   int get_preset(double &fcal, std::vector<double> & preset, std::string & presetf);
         
   ///Check if the focus stage is currently at the preset (focused) position (within 2 microns)
   int check_preset();
   
   ///Move the focus stage to the preset position for the current VisAO setup.
   /** Gets the current preset from get_preset.  If a preset exists, the stage is moved.
     */
   int goto_preset();
   
   /// The main loop.
   /** Sets the signal handling for SIGIO then 
    * starts pause()-ing.
    */
   virtual int Run();
   
   /// Overridden from VisAOApp_base::remote_command, here just calls common_command.
   virtual std::string remote_command(std::string com);
   /// Overridden from VisAOApp_base::local_command, here just calls common_command.
   virtual std::string local_command(std::string com);
   /// Overridden from VisAOApp_base::script_command, here just calls common_command.
   virtual std::string script_command(std::string com);
   
   /// The common command processor for commands received by fifo.
   /** The return value depends on the command received.  Recognized commands are:
    * - POS?  the return value is the current position in microns as "x.XXXX\n"
    * - ISMOVING? the return value is "1\n" if moving, "0\n" if not moving.
    * - STATE? returns the result of get_state_str()
    * - ABORT if control_mode allows it, sets stop_moving = true and returns "0\n".  Otherwise returns the control_mode in 1 letter.
    * - POS x.x if control_mode allows it and not already moving, sets pending_move = \ref MOVE_ABS and next_pos = x.x, and returns "1\n".
    *   if  control_mode would allow it but already moving does nothing and returns "0\n".  Othewrise, returns the control mode in 1 letter.
    * - DPOS x.x if control_mode allows it and not already moving, sets pending_move = \ref MOVE_OFFSET and next_pos = x.x, and returns "1\n".
    *   if  control_mode would allow it but already moving does nothing and returns "0\n".  Othewrise, returns the control mode in 1 letter.
    * - For any other inputs returns "UNKNOWN COMMAND: (str)\n"
    */
   std::string common_command(std::string com, int cmode);
   
   /// Get the state string.
   /** The state string encodes the current state of the controller as:
    * C PPPPPPP.PPPP M RRRRRRR.RRRR
    * Where C is the current control_mode, PPPPPPP.PPPP is the current position
    * M is 1 if moving, 0 if not, and RRRRRRR.RRRR is the remaining distance to travel.
    * \returns the state string
    */
   std::string get_state_str();
   
protected:
   ///Save the initialization data to disk
   int save_init();
   
   ///Delete the initialization file
   int delete_init();
   
public:
   virtual int update_statusboard();
   
   ///Write focus positions to the data log
   virtual void dataLogger(timeval tv);
};
   
} //namespace VisAO

#endif //__VisAOFocusMotorCtrl_h__
