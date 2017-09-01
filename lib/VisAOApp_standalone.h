/************************************************************
*    VisAOApp_standalone.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the standalone VisAO application.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOApp_standalone.h
  * \author Jared R. Males
  * \brief Declarations for the standalone VisAO application.  
  * 
*/

#ifndef __VisAOApp_standalone_h__
#define __VisAOApp_standalone_h__

//Adopt includes:
#include "Logger.h"
#include "stdconfig.h"

#include <fstream>

#include "VisAOApp_base.h"


#define SIG_MAINTHREAD  (SIGRTMIN+6)

#ifndef ERROR_REPORT
#define ERROR_REPORT(er) if(global_error_report) (*global_error_report)(er,__FILE__,__LINE__);
#endif

#ifndef LOG_INFO
#define LOG_INFO(li) if(global_log_info) (*global_log_info)(li);
#endif



namespace VisAO
{      
/// The standalone VisAO application, does not interface with the AO Supervisor.
/** This class reproduces the configuration and logging capabilities of the Arcetri::AOApp, but does
  * not connect to the AO Supervisor.
  * 
  * Standard config options, none of which are required, are listed below.  Default values are given between [] and units, where applicable, are given between ().
  *  - <b>LogLevel</b> <tt>string</tt> [INF] - the logger verbosity level: VTR, TRA, DEB, INF, WAR, ERR, FAT.
  *  - <b>RT_priority</b> <tt>int</tt> [0] - The real time priority, should only be changed if you are an
  *     expert user.  Only works if the executable is setuid.
  *  - <b>wait_to</b> <tt>double</tt> [0.2] (seconds) - The timeout for fifo reads.
  *  - <b>com_path</b> <tt>string</tt> [fifos] - the directory for the command fifos, relative to VISAO_ROOT
  *  - <b>profile_path</b> <tt>string</tt> [profile] - the directory for profiler output, relative to VISAO_ROOT
  *  - <b>use_profiler</b> <tt>int</tt> [0] - flag specifying whether or no the profiler is used.
  *  - <b>pause_time</b> <tt>double</tt> [1.0] - the time between statusboard updates and other housekeeping in
  *     the main loop.
  *  - <b>init_path</b> <tt>string</tt> [init] - the directory to search for an initialization file, relative to
  *     VISAO_ROOT
  *  - <b>data_save_path</b> <tt>string</tt> [data/syslogs] - the directory relative to VISAO_ROOT where the process data log is written
  *  - <b>data_file_prefix</b> <tt>string</tt> [full app name] - the prefix for data logs
  *  - <b>data_log_time_length</b> <tt>double</tt> [120.0] - the length of time, in seconds, to allow a single data log file to remain open.
  * 
  * A VisAOApp accepts at least the following command line options:
  * - <b>-i</b> identity, used for config lookup etc.  This is often not required, but it is good practice to
  *   use it.
  * - <b>-f</b> configuration (if not specified, takes the identity as prefix) [optional].
  * - <b>-v</b> increase verbosity over default level (can appear multiple times) [optional].
  * - <b>-q</b> decrease verbosity from default level (can appear multiple times) [optional].
  * - <b>-h</b> prints a help message documenting these options, and exits [optional].
  * 
  */ 
class VisAOApp_standalone : public VisAOApp_base
{
public:
   /// Default constructor
   VisAOApp_standalone() throw (AOException);
   
   /// Complicated constructor
   VisAOApp_standalone( string name, int id, int logLevel = Logger::LOG_LEV_INFO) throw (AOException);

   void set_conffile( std::string name) throw (AOException);

   /// Config file constructor
   VisAOApp_standalone( std::string name, const std::string& conffile) throw (AOException);

   ///For use if default constructed
   void set_conffile(std::string name, const std::string& conffile) throw (AOException);
   
   /// Command line constructor
   VisAOApp_standalone( int argc, char**argv) throw (AOException);

   
   virtual ~VisAOApp_standalone();
   
   static const std::string &     ConfigFile() {return _configFile;}
   static const std::string &     MyName() {return _myName;}
   static const std::string &     MyFullName() {return _myFullName;}
   static int                     ID() {return _ID;}
   static Config_File &           ConfigDictionary() {return (*_cfg);}
   static int                          Verbosity() {return _logger->getLevel();}
   
   
   /// Return configuration file path based on identity   
   static string getConffile( string identity);

   void usage();

   virtual std::string get_app_name() {return _myName;}
protected:
   	static void     SetMyName(const std::string& name) {_myName=name;}
   	static void     SetConfigFile(const std::string& conffile) throw (Config_File_Exception);
		static void     SetID(int id) {_ID=id;}

		static Logger*  _logger;

      std::string     init_file;
      Config_File*    init_vars;
      
	private:
		static int 	  CONFIG_LOADER_LOG_LEVEL;	// Used to log config file loading

		static int 	  DEFAULT_LOG_LEVEL;
		
		void 	setLogFile(string fileName);
      
		void 	initTempLog();
      
		void  CreateVisAOApp_standalone(int logLevel) throw (AOException);
	
		static std::string    _configFile;
		static std::string    _logFile;
		static std::string    _myName;
		static int            _ID;
		static std::string    _myFullName;
		static Config_File*    _cfg;
      
      
   public:
      /// Report an error.  Also calls log_msg.  Overloaded from VisAOApp_base.
      /** \param emsg the message to report.
        * \param LogLevel is the logger level for the error report.  For erros this is usually Logger::LOG_LEVEL_ERROR or Logger::LOG_LEVEL_FATAL 
        */ 
      //void error_report(int LogLevel, std::string emsg);
      
      /// Log a message.
      /** \param lmsg the message to log.  Overloaded from VisAOApp_base.
       * \param LogLevel is the logger level for the message.
        */ 
      void log_msg(int LogLevel, std::string lmsg);

      
      
   //Execution of a VisAOApp:
   protected:
      pthread_t main_thread; ///<Identifier for the main thread
      pthread_t signal_thread; ///<Identifier for the separate signal handling thread
      pthread_mutex_t signal_mutex; ///<Mutex for the condition signaling
      pthread_cond_t signal_cond; ///<Condition for telling main thread that something changed
      
      double signalth_sleeptime;
      
   public:
      ///Installs the term and XXXX signal handlers, and calls Run().
      /** Normaly doesn't need to be re-implemented.
        */
      virtual int Exec(); 
      
      ///The application main loop, to be re-implemented in derived classes.
      virtual int Run();
      
      ///Signal loop, normally won't need to be overridden.
      /** This does need to be started by the Run() function if it is desired.
        * Before starting, must have setup and connected the fifo list. 
        * Can be started with start_signal_catcher().  The main loop
        * should then block SIGIO and RTSIGIO.
        */ 
      void signal_catcher(); 
      
      ///Starts the signal catching loop
      /** \param inherit_sched if true (default) then the signal_catcher is started with the same RT priority as the main loop.
        */
      virtual int start_signal_catcher(bool inherit_sched = true);
      
      ///Install the SIG_MAINTHREAD signal catcher
      virtual int install_sig_mainthread_catcher();

      ///Sets the signal mask to block signal signum
      virtual int block_signal(int signum);
      
      ///Sets the signal mask to block SIGIO and RTSIGIO
      virtual int block_sigio();

      ///Handle a timetodie condition upon exiting the signal catcher thread (e.g. tell main thread it is about to die)
      /** This isn't usually necessary, but see framewriter for an example of using it.
        */
      virtual int kill_me();      
};


///Thread starter for the signal catcher.
/** Casts the void pointer to VisAOApp_standalone and calls start_signal_catcher.
  */ 
void * __start_signal_catcher(void *); 

///Catches the signal to the main thread.
/** Does absolutely nothing, just provides a facility to wake the main thread up if it is blocking or paused.
  */ 
void sig_mainthread_catcher(int signum, siginfo_t *siginf, void *ucont);

///Catches SIGTERM and sets TimeToDie
void sigterm_handler(int signum, siginfo_t *siginf, void *ucont);

} //namespace VisAO


#endif //__VisAOApp_standalone_h__

