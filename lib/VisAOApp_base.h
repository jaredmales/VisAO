/************************************************************
*    VisAOApp_base.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the basic VisAO application, implements the command fifos
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOApp_base.h
  * \author Jared R. Males
  * \brief Declarations for VisAOApp_base.
  *
*/

#ifndef __VisAOApp_base_h__
#define __VisAOApp_base_h__

#include <string>
#include <iostream>
#include <sstream>
#include <sched.h>
#include <poll.h>

#include "libvisao.h"
#include "statusboard.h"
#include "profiler.h"

#include "Logger.h"

///Global set by SIGTERM.
extern int TimeToDie;

extern std::string global_app_name;

///The namespace for VisAO software.
namespace VisAO
{



///The base class for VisAO applications.
/** Provides the fifo management and SIGIO handling.  Essentially a wrapper to the \ref fifoutils.h "fifo utilities" of the VisAO library.
  * Does not provide config file or logger facilities.  These are added by instead using \ref VisAOApp_standalone or deriving from
  * \ref Arcetri::AOApp 
*/
class VisAOApp_base
{
public:
   ///Default constructor
   VisAOApp_base();
   
   ///Virtual destructor
   virtual ~VisAOApp_base();
   
private:
   uid_t euid_real; ///< The real user id of the proces
   uid_t euid_called; ///< The user id of the process as called (that is when the constructor gets called).
   uid_t suid; ///< The save-set user id of the process
   
   int RT_priority; ///< The real-time scheduling priority.  Default is 0.
   
   
public:
   int set_euid_called(); ///< Changes the user id of the process to euid_called
   int set_euid_real(); ///< Changes the user id fo the process to the real user id.
   
   
   /// Set the real-time priority of the current process.
   /** Elevates the user id to uid_called.
    * \param prio the desired priority
    * \retval 0 on success
    * \retval -1 on failure
    */
   int set_RT_priority(int prio);
   
   /// Get the real-time priority of the current process.
   int get_RT_priority(){return RT_priority;}
   
protected:
   std::string app_name; ///<The name of the application.

   double pause_time; ///< Time to pause during application main loop.
   
public:

   ///Set the application name.
   int set_app_name(std::string an);

   ///Get the application name.
   virtual std::string get_app_name(){ return app_name;}

   ///Install fifo channels.
   /** Call this after paths set, but before connect_fifo_list.
    * \param usethreads if true the threaded versions of the handlers are installed.
    * \retval 0 on success.
    * \retval -1 on failure.
    */
   virtual int setup_baseApp(bool usethreads = false);
   
   ///Create fifo names, then installs the fifo channels.
   /** Bools provide control over which fifo channels are used.
    * assumes com_path is the base path of the fifos. adds  _com_remote_in (for example) is added.
    * \param remfifo if true the remote fifo is created and installed.
    * \param locfifo if true the local fifo is created and installed.
    * \param scrfifo if true the script fifo is created and installed.
    * \param autfifo if true the auto fifo is created and installed.
    * \param usethreads if true the threaded versions of the handlers are installed.
    * \retval 0 on success.
    * \retval -1 on failure.
    */
   virtual int setup_baseApp(bool remfifo, bool locfifo, bool scrfifo, bool autfifo, bool usethreads = false);
   
   ///Mutex used by the threaded versions of the standard command handlers.
   pthread_mutex_t my_mutex;
   
protected:
   fifo_list fl; ///<The list of named-pipe fifos used for inter-process comms.
   
   std::string com_path; ///< The control fifo path base name
   
   std::string com_path_remote; ///<The path for the remote control fifos
   std::string com_path_local;  ///<The path for the local control fifos
   std::string com_path_script; ///<The path for the script control fifos
   std::string com_path_auto;   ///<The path for the auto control fifos
   
   double wait_to; ///< The timeout for waiting on responses from FIFOs in seconds.
   
   
public:
   ///Allocate the fifo_list.
   /** Calls setup_fifo_list(&fl, nfifos).
    * \param nfifos is the number of fifos we need
    * \retval 0 on success.
    * \retval -1 o failure.
    */
   int setup_fifo_list(int nfifos);
   
   /// Connect the fifo_list with exclusive locking.
   /** Calls \ref connect_fifo_list(fifo_list *) "connect_fifo_list"(&fl).
    * \retval 0 on success.
    * \retval -1 on failure.
    */
   int connect_fifo_list();
   
   /// Connect the fifo_list without exclusive locking.
   /** Calls \ref connect_fifo_list_nolock(fifo_list *) "connect_fifo_list_nolock"(&fl).
    * \retval 0 on success.
    * \retval -1 on failure.
    */
   int connect_fifo_list_nolock();
   
   /// Setup the SIGIO signal handling
   /** Uses the sigaction facilities.
    * \retval 0 on success.
    * \retval -1 on failure.
    */
   int setup_sigio();
   
   /// Setup SIGIO signal handling using realtime signals
   /** Uses the sigaction facilities.  Also installs the backup handler for standard SIGIO.
    * \retval 0 on success.
    * \retval -1 on failure.
    */
   int setup_RTsigio();
   
   ///Check for pending reads for the fifo list while using the RT signals.
   int check_fifo_list_RTpending();
      
   /// Write data to a fifo_channel, and get a response if desired.
   /** Calls \ref write_fifo_channel(fifo_channel *, const char*, int) "write_fifo_channel(&fl.fifo_ch[ch], str, len)"
    * If the fifo_channel is timedout, this will not write to that channel again until the file is opened on the other end
    * as detected by fstat access time.
    * \param ch the channel to write to.
    * \param com the buffer to write
    * \param comlen the length of the buffer to write, including the '\0' character
    * \param resp is a string pointer to be filled in with the server_response.  Set to 0 if no response is desired.
    * \retval 0 on success
    * \retval -1 on failure
    */
   int write_fifo_channel(int ch, const char *com, int comlen, std::string * resp);
   
   /// Write data to a fifo_channel, with a sequence message attached, and get a response if desired.
   /** Calls \ref write_fifo_channel(fifo_channel *, const char*, int) "write_fifo_channel(&fl.fifo_ch[ch], str, len)"
    * If the fifo_channel is timedout, this will not write to that channel again until the file is opened on the other end
    * as detected by fstat access time.
    * \param ch the channel to write to.
    * \param com the buffer to write
    * \param comlen the length of the buffer to write
    * \param resp is a string pointer to be filled in with the server_response.  Set to 0 if no response is desired.
    * \param sm is the sequence message.
    * \retval 0 on success
    * \retval -1 on failure
    */
   int write_fifo_channel(int ch, const char *com, int comlen, std::string * resp, char *sm);
   
   /// Check, and wait if neccessary, for data on a fifo channel.
   /** Waits for wait_to seconds, then times out.  Reports only the first in any series of timeouts on one channel.
    * \param resp a pointer to a std::string, on exit is a copy of server_response or "" on timeout
    * \param ch is the fifo_list channel number to wait on.
    * \retval 0 on success
    * \retval -1 on failure.
    * \todo get_fifo_channel_response is a busy wait.  should select or poll instead, but with signal safety.
    */
   int get_fifo_channel_response(std::string * resp, int ch);
   
   /// Set the wait timeout for fifo channel responses.
   /** \param to the desired timeout.
    * \retval 0 on success.
    * \retval -1 on failure (if < 0)
    */
   int set_wait_to(double to);
   
   /// Get the wait timeout for fifo channel responses.
   /** \retval wait_to
    */
   double get_wait_to(){return wait_to;}
   
protected:
   ///The current control mode.
   /** Is static so it can be used by static RTDB handlers.
     */
   static int control_mode;

   ///The default control mode.
   /** If not -1, then this mode is always returned to, instead of NONE.
     */
   static int default_control_mode;
   
public:
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
   
   /// Processing for remote commands
   /** The handler calls __remote_command, which in turn calls \ref remote_command, which is where the
    * app specific processing should be done.
    * This contains the \ref control_mode logic for the remote control state, which is processed before
    * the call to \ref remote_command.
    * \param com is the command string to be processed.
    * \returns the application response.
    */
   virtual std::string __remote_command(std::string com, char *seqmsg);
   /// Called by \ref __remote_command after control state logic.
   /** This should be overriden by derived classes.
    * \param com is the command string to be processed.
    * \returns the application response.
    */
   virtual std::string remote_command(std::string com);
   virtual std::string remote_command(std::string com, char *seqmsg);
   
   /// Processing for local commands
   /** The handler calls __local_command, which in turn calls \ref local_command, which is where the
    * app specific processing should be done.
    * This contains the \ref control_mode logic for the local control state, which is processed before
    * the call to \ref local_command.
    * \param com is the command string to be processed.
    * \returns the application response.
    */
   virtual std::string __local_command(std::string com, char *seqmsg);
   
   /// Called by \ref __local_command after control state logic.
   /** This should be overriden by derived classes.
    * \param com is the command string to be processed.
    * \returns the application response.
    */
   virtual std::string local_command(std::string com);
   virtual std::string local_command(std::string com, char *seqmsg);
   
   /// Processing for script commands - not normally overriden.
   /** The handler calls __script_command, which in turn calls \ref script_command, which is where the
    * app specific processing should be done.
    * This contains the \ref control_mode logic for the script control state, which is processed before
    * the call to \ref script_command.
    * \param com is the command string to be processed.
    * \returns the application response.
    */
   virtual std::string __script_command(std::string com, char *seqmsg);
   
   /// Called by \ref __script_command after control state logic.
   /** This should be overriden by derived classes.
    * \param com is the command string to be processed.
    * \returns the application response.
    */
   virtual std::string script_command(std::string com);
   virtual std::string script_command(std::string com, char *seqmsg);
   
   /// Processing for auto commands - to be overriden.
   /** The handler always calls auto_command, which after app specific processing should call \ref post_auto_command.
    * This is so app specific processing happens first in auto mode, for speed.
    * \param com is the command string to be processed.
    * \returns the application response.
    */
   virtual std::string auto_command(std::string com, char *seqmsg);
   
   /// Processing for auto commands - not normally overriden.
   /** The handler calls auto_command, which in turn should call post_auto_command, which
    * contains the \ref control_mode logic for the auto control state, which is processed after
    * the call to \ref auto_command for speed.
    * \param com is the command string to be processed.
    * \returns the application response.
    */
   virtual std::string post_auto_command(std::string com, char *seqmsg = 0);

   ///Convenience function to return the control type string, e.g. "REMOTE"
   std::string control_mode_string();
   
   ///Convenience function to return the control type response string, e.g. "A\n"
   std::string control_mode_response();
   
    /** @name Error reporting and logging
      * Some generic error reporting and logging facilities
      */
   //@{
   protected:
      std::ostringstream logss; ///< Conveninence string stream for building log messages.
   
   public:
      /// Report an error.  Also calls log_msg.
      /** \param emsg the message to report.
        * \param LogLevel is the logger level for the error report.  For errors this is usually Logger::LOG_LEVEL_ERROR or Logger::LOG_LEVEL_FATAL
        */
      virtual void error_report(int LogLevel, std::string emsg);
   
      /// Log a message.
      /** \param lmsg the message to log.
        * \param LogLevel is the logger level for the message.
        */
      virtual void log_msg(int LogLevel, std::string lmsg);
   //@}
      
   /** @name profiler
     * interface to the VisAO profiler 
     */
   //@{
   protected:
      VisAO::profiler * profile;
      std::string profile_path;
      int use_profiler;
   
   public:
      VisAO::profiler * get_profile(){return profile;}
      int get_use_profiler(){return use_profiler;}
      int start_profiler();
   //@}
      
   /** @name statusboard facilities
     * manage the shared memory status board 
     */
   //@{
   protected:
      void * statusboard_shmemptr; ///<The pointer to the shared memory block for the statusboard
      key_t statusboard_shmemkey; ///<The key used to lookup the shared memory
      int   statusboard_shmemid; ///<The ID of the shared memory block.
   
      ///  Creates and attaches to the statusboard shared memory
      /** \param sz is the desired size (should be sizeof(xxxxx_status_board))
        * \retval 0 on success
        * \retval -1 on failure
        */
      int create_statusboard(size_t sz);
   
   public:
      /// Get the status board shared memory pointer.
      /** \retval the shared memory pointer void *
        */
      void * get_statusboard_shmemptr(){return statusboard_shmemptr;}
   
      /// Set the status board shared memory key.  Does nothing else.
      /** \param mkey is the key
        * \retval 0 on success (always)
        */
      int set_statusboard_shmemkey(key_t mkey);
   
      /// Get the status board shared memory key.
      /** \retval the shared memory key
        */
      key_t get_statusboard_shmemkey(){return statusboard_shmemkey;}
   
      /// Update the status board.
      /** Overridden versions should call this as VisAOApp_bas::update_statusboard so the basics are taken care of.
        * \retval 0 on success
        * \retval -1 on failure
        */
      virtual int update_statusboard();
   //@}
      
   /** @name Data Logging
     * Provides logging of process data at intervals, or upon a change in state. 
     */
   //@{
   protected:   
      std::ofstream dataof;
   
      bool dataFileOpen;
      double dataFileOpenTime;
      
      double data_log_time_length;
      std::string data_save_path;
      std::string data_file_prefix;
      
      std::string getDataFileName();
      
      int checkDataFileOpen();
   
   public:
      ///Log data at intervals
      virtual void dataLogger(timeval tv);
   //@}
         
};

int com_remote_handler(fifo_channel *); ///<\ref __fifo_channel "fifo_channel" handler for remote commands
int com_local_handler(fifo_channel *); ///<\ref __fifo_channel "fifo_channel" handler for local commands
int com_script_handler(fifo_channel *); ///<\ref __fifo_channel "fifo_channel" handler for script commands
int com_auto_handler(fifo_channel *); ///<\ref __fifo_channel "fifo_channel"  handler for auto commands

/// Thread version of the \ref __fifo_channel "fifo_channel"  handler for remote commands.
/** Creates a thread using \ref __com_remote_thread_handler in the detached state.
* Note: The thread fifo handlers are no longer maintained as of 3 July 2011.  Use with caution.
 */
int com_remote_thread_handler(fifo_channel * fc);
/// The start function for remote handling with a thread.
/** Note: The thread fifo handlers are no longer maintained as of 3 July 2011.  Use with caution.
*/
void * __com_remote_thread_handler(void * ptr);

/// Thread version of the \ref __fifo_channel "fifo_channel"  handler for local commands.
/** Creates a thread using \ref __com_local_thread_handler in the detached state.
* Note: The thread fifo handlers are no longer maintained as of 3 July 2011.  Use with caution.
 */
int com_local_thread_handler(fifo_channel * fc);

/// The start function for local handling with a thread.
/** Note: The thread fifo handlers are no longer maintained as of 3 July 2011.  Use with caution.
*/
void * __com_local_thread_handler(void * ptr);

/// Thread version of the \ref __fifo_channel "fifo_channel"  handler for script commands.
/** Creates a thread using \ref __com_script_thread_handler in the detached state.
* Note: The thread fifo handlers are no longer maintained as of 3 July 2011.  Use with caution.
 */
int com_script_thread_handler(fifo_channel * fc);
/// The start function for script handling with a thread.
/** Note: The thread fifo handlers are no longer maintained as of 3 July 2011.  Use with caution.
*/
void * __com_script_thread_handler(void * ptr);

/// Thread version of the \ref __fifo_channel "fifo_channel"  handler for auto commands.
/** Creates a thread using \ref __com_auto_thread_handler in the detached state.
 * Note: The thread fifo handlers are no longer maintained as of 3 July 2011.  Use with caution.
 */
int com_auto_thread_handler(fifo_channel * fc);
/// The start function for auto handling with a thread.
/** Note: The thread fifo handlers are no longer maintained as of 3 July 2011.  Use with caution.
*/
void * __com_auto_thread_handler(void * ptr);

///Utility function for parsing command buffers line by line.
/** This alters com, so make a copy if we need it somewhere else.
*/
int getnextline(std::string *comln, std::string * com);



/***** profiler stuff *****/

void * __start_profiler(void * ptr);

} //namespace VisAO
#endif //__VisAOApp_base_h__

