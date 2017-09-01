/************************************************************
*    WollastonStatus.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the VisAO wollaston lift status maintainer.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file WollastonStatus.h
  * \author Jared R. Males
  * \brief Declarations for the VisAO wollaston lift status maintainer.
  *
*/

#ifndef __WollastonStatus_h__
#define __WollastonStatus_h__

#include "VisAOApp_standalone.h"
#include "libvisao.h"

#include "AOStates.h"

namespace VisAO
{
	
/// The wollaston lift status maintainer.
/** Tracks the status of the wollaston lift.
  *
*/ 
class WollastonStatus : public VisAOApp_standalone
{
public:
   /// Standard constructor with a config file.
   WollastonStatus( std::string name, const std::string& conffile) throw (AOException);
   
   /// Standard constructor with a command line.
   WollastonStatus( int argc, char**argv) throw (AOException);
   
   /// Initialization common to both constructors.
   /** Reads the app specific config detals, and sets up the fifo_list and connects it.
    * Upon exit, SIGIO is blocked.
    */
   void initApp();
   
protected:
   int cur_pos; ///<The current position of the lift, -1 is down, 0 is intermediate, 1 is up.
   
   filterwheel_status_board *fw3sb;
   int last_type;
   int prompt;
   int ignored;
   
public:
   ///Set the value of cur_pos of the motor.
   /** Does not move the motor.
    * \param cp the current position.
    * \retval 0 on success
    * \retval -1 on failure.
    */
   int set_cur_pos(int cp);
   
   ///Get the current positon of the motor.
   /** \retval the value of cur_pos.
    */
   int get_cur_pos(){return cur_pos;}
   
   /// The main loop.
   /** Sets the signal handling for SIGIO then
    * starts pausing()-ing.
    */
   virtual int Run();
   
   /// Overridden from VisAOApp_base::remote_command, here just calls common_command.
   virtual std::string remote_command(std::string com);
   /// Overridden from VisAOApp_base::local_command, here just calls common_command.
   virtual std::string local_command(std::string com);
   
   /// The common command processor for commands received by fifo.
   /** The return value depends on the command received.  Recognized commands are:
    * - POS?  the return value is the current position, 1 for up, 0 for intermediate, -1 for down.
    * - STATE? returns the result of get_state_str()
    * - UP if control_mode allows it, and lift is not up, raises the lift, and returns "1\n". If lift is already up, returns "0\n". Otherwise returns
    *   the control_mode in 1 letter.
    * - DOWN if control_mode allows it, and lift is not down, lowers the lift, and returns "1\n". If lift is already up, returns "0\n". Otherwise
    *   returns the control_mode in 1 letter.
    * - For any other inputs returns "UNKNOWN COMMAND: (str)\n"
    */
   std::string common_command(std::string com, int cmode);
   
   /// Get the state string.
   /** The state string encodes the current state of the controller as:
    * C P
    * Where C is the current control_mode, P is the current position
    * \returns the state string
    */
   std::string get_state_str();
   
   virtual int update_statusboard();
   
}; 

} //namespace VisAO
#endif //__WollastonStatus_h__

