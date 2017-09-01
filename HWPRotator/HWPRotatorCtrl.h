/************************************************************
 *   HWPRotatorCtrl.h
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Declarations for the VisAO gimbal motor controller.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \fileHWPRotator.h
 * \author Jared R. Males
 * \brief Declarations for the gimbal motor controller.
 *
 */

#ifndef __HWPRotatorCtrl_h__
#define __HWPRotatorCtrl_h__

#include "VisAOApp_standalone.h"
#include "libvisao.h"

#include "AOStates.h"


#include "ttyUSB.hpp"

#include <termios.h>

namespace VisAO
{
   
   //To setup the PI rotation stage:
   // -- create /etc/modprobe.d/ftdi_sio.conf
   // -- and add line: options ftdi_sio vendor=0x1a72 product=0x1014
   
/// The HWP Rotator controller class.
/** Manages the HWP PI stage
  *  This is a standalone VisAOApp, so it doesn't depend on MsgD.  In addition to the optional standard config options inherited from \ref VisAOApp_standalone this class REQUIRES:
  *    - <b>power_outlet</b> <tt>int</tt> - the number of the power strip outlet the stage is plugged in to.
  *    - <b>usbVendor</b> <tt>string</tt> - the vendor id of the stage controller
  *    - <b>usbProduct</b> <tt>string</tt> - the product id of the stage controller 
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
  *      - D = homing state
  *          - -1 = unknown
  *             0 = not homing
  *             1 = homing 
  *      - E = moving state.
  *          - 0 = not moving
  *          - 1 = moving
  *      - F = position (mm)
  *   - "homing?" returns the homing state
  *      - 0 = not homing
  *      - 1 = homing
  *   - "moving?" returns the moving state
  *      - 0 = not moving
  *      - 1 = moving
  *   - "pos?" returns the position in mm
  *   - "stop" stops motion immediately
  *   - "rel x" moves by a relative amount x, in mm.
  *   - "abs x" moves to the absolute position x, in mm
  *   - "home" homes the stage 
  */
class HWPRotatorCtrl : public VisAOApp_standalone
{

   
public:
   /// Standard constructor with a config file.
   HWPRotatorCtrl(std::string name, const std::string& conffile) throw (AOException);
   
    /// Standard constructor with a command line.
   HWPRotatorCtrl( int argc, char**argv) throw (AOException);
   
   virtual ~HWPRotatorCtrl();
   
   
   /// Initialization common to both constructors.
   /** Reads the app specific config detals, and sets up the fifo_list.
    */
   void Create();
   
protected:
   
   int curState; ///<The current controller state (READY, etc.)
      
   int power_outlet; ///<Configuration variable, setting which power outlet to monitor
   int *powerOutletState; ///<The power strip outlet controlling shutter power.
   
   power_status_board * psb;///<This is for monitoring power outlet state

   std::string usbVendor;
   std::string usbProduct;
   std::string deviceName;
   int fileDescrip;
   
   pthread_mutex_t comMutex; ///< Mutex for communicating with the controllers.

   VisAO::aosystem_status_board *aosb; ///<pointer to status board of VisAOI (for presets)
   
   int stageSend(const std::string & msg);
   
   int stageSend(std::string & resp, const std::string & msg);
   
   
public:
   ///Establish the USB connection.
   /** Closes any existing connection, and then attempts to establish the link.
     */
   int openDevice();
   
   ///Check connection status by querying the status of the controller.  Changes curState.
   /** \retval 0 on success
    * \retval -1 on error (other than not being connected).
    */
   int Connect();

   int setupStage();
   
    ///Get the status of the power outlet
   /** \retval 1 if on
    * \retval 0 if off
    * \retval -1 if no status available
    */
   int getPowerStatus();
   
   int getMoving();
   double getPosition();
         
  
   
   
   ///Get the current process state
   int getCurState();


   int home();
   int move(double npos);
   int stop();
   int contRot();
   
   int minus45();
   int minus225();
   int plus225();
   int plus45();
   
   
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

   ///Called when a move starts, for data logging. 
//    virtual void moveStart();
   
   ///Called when a move stops, for data logging.
//    void moveStop();
   
public:
   ///Update the field stop status board
   virtual int update_statusboard();
   
   ///Write field stop position to the data log
   //virtual void dataLogger(timeval tv);
};
   
} //namespace VisAO

#endif //__HWPRotatorCtrl_h__
