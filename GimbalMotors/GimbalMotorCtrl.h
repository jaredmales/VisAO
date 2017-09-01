/************************************************************
 *    GimbalMotorCtrl.h
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Declarations for the VisAO gimbal motor controller.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file GimbalMotorCtrl.h
 * \author Jared R. Males
 * \brief Declarations for the gimbal motor controller.
 *
 */

#ifndef __GimbalMotorCtrl_h__
#define __GimbalMotorCtrl_h__

#include "VisAOApp_standalone.h"
#include "libvisao.h"
#include "ESPMotorCtrl.h"
#include "AOStates.h"
extern "C"
{
#include "hwlib/netseriallib.h"
}

namespace VisAO
{
   
/// The gimbal motor controller class.
/** Manages the X and Y axis controllers of the Gimbal mirror.
  *  This is a standalone VisAOApp, so it doesn't depend on MsgD.  In addition to the optional standard config options inherited from \ref VisAOApp_standalone this class REQUIRES:
  *    - <b>x_addr</b> <tt>int</tt> - the address of the x axis controller
  *    - <b>y_addr</b> <tt>int</tt> - the address of teh y axis controller
  *    - <b>power_outlet</b> <tt>int</tt> - the number of the power strip outlet the controllers are plugged in to.
  *    - <b>scale</b> <tt>double</tt> (arcsec/mm) - the conversion from encoder mm to arcsec on the CCD47
  *    - <b>x_center</b> <tt>double</tt> (arcsec) - the scaled encoder position of the x center of travel
  *    - <b>y_center</b> <tt>double</tt> (arcsec) - the scaled encoder position of the y senter of travel
  *     
  *
  * See \ref VisAOApp_standalone for command line arguments.  There are no additional command line arguments for GimbalCtrl.
  *
  * This app exposes the remote, local, and script command fifos.  The following commands are recognized on all channels:
  *   - "state?" returns a state string "A,B,C,D,E,F,E\n" where the letters have the following meaning
  *      - A = a one character representation of control mode, N, R, L, or S.
  *      - B = the connection status with the controllers (STATE_READY, etc.)
  *      - C = the power state of the controllers, derived from the power outlet status.
  *          - 0 = off
  *          - 1 = on
  *          - -1 = unknown
  *      - D = x moving state.
  *          - 0 = not moving
  *          - 1 = moving
  *      - E = x position (arcsec)
  *      - F = y moving state.
  *          - 0 = not moving
  *          - 1 = moving
  *      - G = y position (arcsec)
  *      - E = scale (arcsec/mm)
  *   - "xpos?" returns the x position in mm
  *   - "xmoving?" returns the x moving state
  *      - 0 = not moving
  *      - 1 = moving
  *   - "ypos?" returns the y position in mm
  *   - "ymoving?" returns the y moving state
  *      - 0 = not moving
  *      - 1 = moving
  *   - "stop" stops motion on all axes
  *   - "stopx" stops motion on the x axis
  *   - "stopy" stops motion on the axis
  *   - "center" - center both axes
  *   - "savepreset" - save current position as preset
  *   - "xrel x" moves x by a relative amount x, in mm.
  *   - "xabs x" moves x to the absolute position x, in mm
  *   - "yrel x" moves y by a relative amount x, in mm.
  *   - "yabs x" moves y to the absolute position x, in mm
  */
class GimbalMotorCtrl : public ESPMotorCtrl, public VisAOApp_standalone
{
   
public:
   /// Standard constructor with a config file.
   GimbalMotorCtrl(std::string name, const std::string& conffile) throw (AOException);
   
   /// Standard constructor with a command line.
   GimbalMotorCtrl( int argc, char**argv) throw (AOException);
   
   virtual ~GimbalMotorCtrl();
   
   
   /// Initialization common to both constructors.
   /** Reads the app specific config detals, and sets up the fifo_list.
    */
   void Create();
   
protected:
   std::string ip_addr; ///<ip address of the serial converter
   int ip_port; ///<ip port on which the motor controllers are connected
   
   int curState; ///<The current controller state (READY, etc.)
   int postHoming; ///<When 1, the gimbal will center after homing

   
   int x_addr; ///<The controller address of the x axis
   int y_addr; ///<The controller address of the y axis
   int xAxisNo; ///<The axis number of x, e.g. 0
   int yAxisNo; ///<The axis number of y, e.g. 1
      
   int power_outlet; ///<Configuration variable, setting which power outlet to monitor
   int *powerOutletState; ///<The power strip outlet controlling shutter power.
   
   power_status_board * psb;///<This is for monitoring power outlet state

   double scale; ///<The conversion scale from encoder mm to arcsec on the CCD47, i.e. arcsec/mm
   double x_center; ///<The configurable location of the default x-center
   double x_dark;///<The configurable location of the x dark location

   int xMoving; ///<Status of moving in the x direction
   
   double y_center; ///<The configurable location of the default x-center
   double y_dark;///<The configurable location of the y dark location

   bool isMoving; ///<True if moving - set when move starts and updating data logger.
   
   
   pthread_mutex_t comMutex; ///< Mutex for communicating with the controllers.


   VisAO::filterwheel_status_board *fw2sb;  ///<pointer to status board of F/W 2 (for presets)
   VisAO::filterwheel_status_board *fw3sb; ///<pointer to status board of F/W 3 (for presets)
   VisAO::wollaston_status_board *wsb; ///<pointer to status board of wollaston (for presets)
   VisAO::aosystem_status_board *aosb; ///<pointer to status board of VisAOI (for presets)
   
public:
   ///Establish the serial-over-ip connection.
   /** Closes any existing connection, and then attempts to establish the link.
     */
   int setupNetwork();
   
   ///Check connection status by querying the status of each controller.  Changes curState.
   /** \retval 0 on success
    * \retval -1 on error (other than not being connected).
    */
   int checkConnStat();

   ///Get the current process state
   int getCurState(){return curState;}

   /// Get the x position
   double get_x_pos();

   /// Get the moving status of the x axis.
   int getXMoving();
   
   /// Get the y position
   double get_y_pos();

   /// Get the moving status of the y axis.
   int getYMoving();
   
   ///Get the status of the power outlet
   /** \retval 1 if on
     * \retval 0 if off
     * \retval -1 if no status available
     */
   int getPowerStatus();

   ///Read the preset center position for current board conditions, and gimbal to it.
   int center();

   ///Gimbal to the dark position.
   int dark();

   ///Save the current position as the preset for this board setup.
   int savepreset();
   
   ///Implementation of ESPMotorCtrl sendCommand
   virtual int sendCommand(std::string &com, std::string &resp, int timeout);

   ///Implementation of ESPMotorCtrl sendCommand
   virtual int sendCommand(std::string &com);

   
   /// The main loop.
   /** Sets the signal handling for SIGIO then 
     * starts pausing()-ing.
     */
   virtual int Run();
   
   /// Overridden from VisAOApp_base::remote_command, here just calls common_command.
   virtual std::string remote_command(std::string com);
   /// Overridden from VisAOApp_base::local_command, here just calls common_command.
   virtual std::string local_command(std::string com);
   /// Overridden from VisAOApp_base::script_command, here just calls common_command.
   virtual std::string script_command(std::string com);
   
   /// The common command processor for commands received by fifo.
   std::string common_command(std::string com, int cmode);
   
   /// Get the state string.
   /** The state string encodes the current state of the controller as:
    */
   std::string get_state_str();

   ///Called when a move starts, for data logging. Overriden from ESPMotorCtrl
   virtual void moveStart();
   
   ///Called when a move stops, for data logging.
   void moveStop();
   
public:
   ///Update the gimbal status board
   virtual int update_statusboard();
   
   ///Write gimbal positions to the data log
   virtual void dataLogger(timeval tv);
};
   
} //namespace VisAO

#endif //__GimbalMotorCtrl_h__
