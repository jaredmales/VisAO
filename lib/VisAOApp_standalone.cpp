/************************************************************
*    VisAOApp_standalone.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for the standalone VisAO application.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOApp_standalone.cpp
  * \author Jared R. Males
  * \brief Definitions for the standalone VisAO application.  
  * 
*/


   
#include "VisAOApp_standalone.h"

#include "AOApp.h"
#include "AOStates.h"

namespace VisAO
{
   
int    VisAOApp_standalone::CONFIG_LOADER_LOG_LEVEL = Logger::LOG_LEV_INFO;
int    VisAOApp_standalone::DEFAULT_LOG_LEVEL       = Logger::LOG_LEV_INFO;

Logger* VisAOApp_standalone::_logger = NULL;

Config_File* VisAOApp_standalone::_cfg = NULL;

std::string  VisAOApp_standalone::_configFile;
std::string  VisAOApp_standalone::_logFile;
std::string  VisAOApp_standalone::_myName;
int          VisAOApp_standalone::_ID;
std::string  VisAOApp_standalone::_myFullName;


VisAOApp_standalone::VisAOApp_standalone()  throw (AOException) : VisAOApp_base()
{
   // Before all, temporary initialize the singleton logger (named "MAIN")
   initTempLog();
   
   // Init status
   _cfg             = NULL;
   _myName          = "noname";
   _ID                = 0;
   
   
   // Complete the initialization
   CreateVisAOApp_standalone(Logger::LOG_LEV_INFO);
}

VisAOApp_standalone::VisAOApp_standalone( string name, int id, int logLevel)  throw (AOException) : VisAOApp_base()
{
   // Before all, temporary initialize the singleton logger (named "MAIN")
   initTempLog();
   
   // Init status
   _cfg             = NULL;
   _myName          = name;
   _ID               = id;
   
   
   // Complete the initialization
   CreateVisAOApp_standalone(logLevel);
}

void VisAOApp_standalone::set_conffile( std::string name)  throw (AOException) 
{
   std::string conffile = getConffile(name);

   set_conffile(name, conffile);
}

VisAOApp_standalone::VisAOApp_standalone( std::string name, const std::string& conffile)  throw (AOException) : VisAOApp_base()
{
   set_conffile(name, conffile);
}

void VisAOApp_standalone::set_conffile(std::string name, const std::string& conffile) throw (AOException)
{
   _myName = name;
   set_app_name(_myName);

   // Before all, temporary initialize the singleton logger (named "MAIN")
   initTempLog();
   
   // Init the AOApp status using the configuration file
   _cfg = NULL;


   try
   {
      std::cerr << "=====> " << conffile << "\n";
      SetConfigFile(conffile);
   }
   catch(Config_File_Exception &e)
   {
      // This is a fatal error
      throw AOException("config File Exception in AOApp c'tor");
   }
   
   // Get the log level
   int logLevel = 0;
   
   try
   {
      // Try to get the level from config file...
      if(_cfg != NULL)
      {        
         logLevel = Logger::stringToLevel((*_cfg)["LogLevel"]);
      }
   }
   catch(Config_File_Exception& e)
   {
      //...otherwise use a default value
      logLevel=Logger::LOG_LEV_INFO;
   }
     
   // Complete the initialization
   CreateVisAOApp_standalone(logLevel);
}

VisAOApp_standalone::VisAOApp_standalone( int argc, char**argv)  throw (AOException) : VisAOApp_base()
{
   char c;
   
   // Init the AOApp status using the command line
   std::string conffile = "";
   int log_modifier = 0;
   
   opterr = 0;
   while ((c = getopt (argc, argv, "i:f:hvq")) != -1)
   {
      switch (c)
      {
         case 'i':
            _myName = optarg;
            break;
         case 'f':
            conffile = optarg;
            break;
         case 'v':
            log_modifier++;
            break;
         case 'q':
            log_modifier--;
            break;
         case 'h':
            usage();
            exit(0);
         case '?':
            // Do not want to exit in case of unknown option: can be a valid
            // option for a subclass!!!
            //if (isprint (optopt))
            //    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            //else
            //    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            //exit(0);
            break;
      }
   }
   
   // Before all, instantiate the singleton logger (named "MAIN")
   initTempLog();

   // If no config. file specified, use the identity to find it
   if (conffile == "")
      conffile = getConffile(_myName);
   try
   {
      SetConfigFile(conffile);
      
      set_app_name(_myName);
   }
   catch(Config_File_Exception &e)
   {
      // This is a fatal error
      throw AOException("config File Exception in VisAOApp constructor");
   }
   
   // Get the logging level
   int llevel=Logger::LOG_LEV_INFO;
   if(log_modifier > 0) llevel=Logger::LOG_LEV_INFO + log_modifier;

   std::cout << "0.\n";
   // Complete the initialization
   CreateVisAOApp_standalone(llevel);
}

VisAOApp_standalone::~VisAOApp_standalone()
{
   Logger::get()->log(Logger::LOG_LEV_FATAL, "VisAOApp terminated.");
   std::cerr << app_name << ": VisAOApp terminated." << std::endl;
   // Destroy the loggers pool
   Logger::destroy();
}

string VisAOApp_standalone::getConffile( string identity)
{
   std::cerr << "Conf. file: " << Utils::getConffile(identity) << std::endl;
   
   return Utils::getConffile(identity);
}

void VisAOApp_standalone::usage()
{
   string vId;
   vId=GetVersionID();
   cout << endl << "VisAOApp: " << _myName << " " << vId << endl << endl;
   cout << endl << "Standard options:" << endl << endl;
   cout << "   -i <identity>: identity, used for config lookup etc." << endl;
   cout << "   -f <file>    : configuration file (if not specified, takes the identity as prefix)" << endl;
   cout << "   -v           : increase verbosity over default level (can appear multiple times)" << endl;
   cout << "   -q           : decrease verbosity from default level (can appear multiple times)" << endl;
   cout << "   -h           : prints this message and exits" << endl;
   cout << endl;
}


void VisAOApp_standalone::CreateVisAOApp_standalone(int logLevel) throw (AOException)
{
   std::string pathtmp;
   
   // Useful
   _myFullName = _myName +  "." +  Utils::getAdoptSide();

   Logger::setParentName(_myFullName);

   // This way the log file is unique
   _logFile = _myFullName;

   ////////////////// INIT THE LOGGER /////////////////
   
   // --- Setup the LOGGER POOL --- //
   
   // Setup the logging path
   setLogFile(_logFile);
   
   
   // --- Setup the logger MAIN --- //
   _logger->setGlobalLevel(logLevel);
   
   //Set the realtime priority from config
   try 
   {
      if(_cfg != NULL)  
      {
         set_RT_priority((int)(*_cfg)["RT_priority"]);
      }
      else
      {
         set_RT_priority(0);
      }
   }
   catch(Config_File_Exception& e) 
   {
      //Otherwise don't do anything.
      set_RT_priority(0);
   }
   
   //Set the wait time from config
   try 
   {
      if(_cfg != NULL)  
      {
         set_wait_to((double)(*_cfg)["wait_to"]);
      }
   }
   catch(Config_File_Exception& e) 
   {
      //Otherwise don't do anything.
   }
   
   //Set up the com_paths
   try
   {
      if(_cfg != NULL)  
      {
         pathtmp = (std::string)(*_cfg)["com_path"];
      }
      else pathtmp = "fifos";
   }
   catch(Config_File_Exception)
   {
      pathtmp = "fifos";
   }
   com_path = std::string(getenv("VISAO_ROOT")) + "/" + pathtmp + "/" + _myName;

   //Set up the default_control_mode
   try
   {
      if(_cfg != NULL)
      {
         std::string dfc = (std::string)(*_cfg)["default_control_mode"];
         if(dfc[0] == 'N') default_control_mode = CMODE_NONE;
         else if(dfc[0] == 'R') default_control_mode = CMODE_REMOTE;
         else if(dfc[0] == 'L') default_control_mode = CMODE_LOCAL;
         else if(dfc[0] == 'S') default_control_mode = CMODE_SCRIPT;
         else if(dfc[0] == 'A') default_control_mode = CMODE_AUTO;
         else
         {
            Logger::get()->log(Logger::LOG_LEV_ERROR, "Uknown control mode in config default_control_mode");
         }
      }

      if(default_control_mode > -1) request_control(default_control_mode,1);
   }
   catch(Config_File_Exception)
   {
      //do nuthin'
   }
   
   //Set up the profile_path
   try
   {
      if(_cfg != NULL)  
      {
         pathtmp = (std::string)(*_cfg)["profile_path"];
      }
      else pathtmp = "profile";
   }
   catch(Config_File_Exception)
   {
      pathtmp = "profile";
   }
   profile_path = std::string(getenv("VISAO_ROOT")) + "/" + pathtmp + "/" + MyFullName();
   
   profile->set_base_path(profile_path);
   
   
   //Set use_profile
   try
   {
      if(_cfg != NULL)  
      {
         use_profiler = (int)(*_cfg)["use_profiler"];
      }
      else use_profiler = 0;
   }
   catch(Config_File_Exception)
   {
      use_profiler = 0;
   }
   if(use_profiler != 0) use_profiler = 1;
   
   //Set pause_time
   try
   {
      if(_cfg != NULL)  
      {
         pause_time = (double)(*_cfg)["pause_time"];
      }
      else pause_time = 1.0;
   }
   catch(Config_File_Exception)
   {
      pause_time = 1.0;
   }
   
   
   //Check for initialization file
   try
   {
      if(_cfg != NULL)  
      {
         init_file = (std::string)(*_cfg)["init_path"];
      }
      else init_file = "init";
   }
   catch(Config_File_Exception)
   {
      init_file = "init";
   }
   
   init_file = init_file + "/"+ _myName+".init";

   try
   {
      init_vars = new Config_File(init_file);
   }        
   catch(Config_File_Exception)
   {
      init_vars = 0;
   }    


   pthread_cond_init(&signal_cond, NULL);
   pthread_mutex_init(&signal_mutex, NULL);
   
   //Set up the data file paths
   if(_cfg != NULL)  
   {
      try
      {
         data_save_path = (std::string)(*_cfg)["data_save_path"];
      }
      catch(Config_File_Exception)
      {
         data_save_path = "data/syslogs";
      }
      
      try
      {
         data_log_time_length = (double)(*_cfg)["data_log_time_length"];  
      }
      catch(Config_File_Exception)
      {
         data_log_time_length = 120.;  
      }
      
      try
      {
         data_file_prefix = (std::string)(*_cfg)["data_file_prefix"];
      }
      catch(Config_File_Exception)
      {
         data_file_prefix = _myFullName;  
      }
   }
   else 
   {
      data_save_path = "data/syslogs";
      data_log_time_length = 120.;  
      data_file_prefix = _myFullName;
   }
   

   
   #ifdef VISAO_SIMDEV
      signalth_sleeptime = 1000.;
   #else

   try
   {
      if(_cfg != NULL)  
      {
         signalth_sleeptime = (double)(*_cfg)["signalth_sleeptime"];
      }
      else
      {
         signalth_sleeptime = 1000.;
      }
   }
   catch(Config_File_Exception)
   {
      signalth_sleeptime = 1000.;
   }

   #endif
   

}

void VisAOApp_standalone::setLogFile(string fileName) 
{
   _logger->rename( fileName);
}

void VisAOApp_standalone::initTempLog() 
{
   Logger::enableFileLog();

   // Before configuration read, the logging file name is unknown:
   // a temp name is used, and then the method CreateApp(), which
   // knows the correct name will rename it

   _logger = Logger::get();
   setLogFile(Utils::uniqueFileName("aoapp_"));
}

void VisAOApp_standalone::SetConfigFile(const std::string& conffile) throw (Config_File_Exception)
{
   
   Config_File::setLogLevel(CONFIG_LOADER_LOG_LEVEL);
   
   _configFile = conffile;
   
   try 
   {
      
      _cfg         = new Config_File(_configFile);

   }
   catch (Config_File_Exception &e) 
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, e.what().c_str());
      throw;
   }
   
}

void VisAOApp_standalone::log_msg(int LogLevel, std::string lmsg)
{
   Logger::get()->log(LogLevel, lmsg);
}
      

int VisAOApp_standalone::Exec()
{
   struct sigaction act;
   sigset_t set;

   act.sa_sigaction = &sigterm_handler;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&set);
   act.sa_mask = set;

   sigaction(SIGTERM, &act, 0);
   sigaction(SIGQUIT, &act, 0);
   sigaction(SIGINT, &act, 0);
   
   return Run();
}

int VisAOApp_standalone::Run()
{
   logss.str("");
   logss << "Run() not yet reimplemented.  No main loop to execute.";
   error_report(Logger::LOG_LEV_FATAL, logss.str());
   return -1;
}

void VisAOApp_standalone::signal_catcher()
{
   double dt, t0;
   
   //if(connect_fifo_list_nolock() == 0)
   if(connect_fifo_list() == 0)
   {
      //log_msg(Logger::LOG_LEV_DEBUG, "fifo_list connected.");
      logss.str("");
      logss << "fifo_list connected.";
      log_msg(Logger::LOG_LEV_INFO, logss.str());
   }
   else
   {
      //error_report(Logger::LOG_LEV_FATAL, "Error connecting the fifo list.");
      error_report(Logger::LOG_LEV_FATAL, "Error connecting the fifo list.");
      TimeToDie = 1;
      return;
   }

   global_fifo_list = &fl;
   
   setup_RTsigio();

   block_signal(RTSIGPING); //We block this here, so a main thread can catch it.
   
   while(!TimeToDie)
   {
      t0 = get_curr_time();

      while((dt = get_curr_time() - t0) < pause_time && !TimeToDie)
      {
         
         check_fifo_list_RTpending();
         
         pthread_cond_broadcast(&signal_cond);
         
         usleep((int) signalth_sleeptime);
         //if(pause_time - dt >= 1) sleep((int)(pause_time - dt));
         //else usleep((int)((pause_time-dt)*1e6 * .99));
      }
      
      check_fifo_list_RTpending();
      
      pthread_cond_broadcast(&signal_cond);
      
      update_statusboard();
      
   }
   
   pthread_cond_broadcast(&signal_cond);

   kill_me();
   
}

int VisAOApp_standalone::start_signal_catcher(bool inherit_sched)
{
   struct sched_param schedpar;
   pthread_attr_t attr;
   
   pthread_attr_init(&attr);
   
   if(!inherit_sched)
   {
      //Start the signal catcher as a lower priority thread.
      
      schedpar.sched_priority = 0;

      pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
      pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
      pthread_attr_setschedparam(&attr, &schedpar);      
   }
   else
   {
      //Start with inherit sched, use the same priority as the main thread
      pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
   }
   
   //This is weird, but the doc for pthread_create says it returns an "error number" instead of -1
   if(pthread_create(&signal_thread, &attr, &__start_signal_catcher, (void *) this) == 0) return 0;
   else return -1;

}
  
int VisAOApp_standalone::install_sig_mainthread_catcher()
{
   struct sigaction act;
   sigset_t set;

   //Install the main thread handler for signals from the signal catcher
   main_thread = pthread_self();
   act.sa_sigaction = &sig_mainthread_catcher;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&set);
   act.sa_mask = set;

   return sigaction(SIG_MAINTHREAD, &act, 0);

}

int VisAOApp_standalone::block_signal(int signum)
{
   sigset_t set;
   
   sigemptyset(&set);
   sigaddset(&set, signum);

   errno = 0;
   if(pthread_sigmask(SIG_BLOCK, &set, 0) == 0) return 0;
   else
   {
      perror("block_signal");
      return -1;
   }
}


int VisAOApp_standalone::block_sigio()
{
   //sigset_t set;
   
   //sigemptyset(&set);
   //sigaddset(&set, SIGIO);
   //sigaddset(&set, RTSIGIO);

   if(block_signal(SIGIO) < 0) return -1;
   if(block_signal(RTSIGIO) < 0) return -1;
   //if(pthread_sigmask(SIG_BLOCK, &set, 0) == 0) return 0;
   //else return -1;

   return 0;
}

int VisAOApp_standalone::kill_me()
{
   return 0;
}

void * __start_signal_catcher(void * ptr)
{
   ((VisAOApp_standalone *) ptr)->signal_catcher();
   return 0;
}

void sig_mainthread_catcher(int signum __attribute__((unused)), siginfo_t *siginf __attribute__((unused)), void *ucont __attribute__((unused)))
{
   //do nothing
   return;
}

void sigterm_handler(int signum, siginfo_t *siginf __attribute__((unused)), void *ucont __attribute__((unused)))
{
   std::string signame;
   TimeToDie = 1;
   
   switch(signum)
   {
      case SIGTERM:
         signame = "SIGTERM";
         break;
      case SIGINT:
         signame = "SIGINT";
         break;
      case SIGQUIT:
         signame = "SIGQUIT";
         break;
      default:
         signame = "OTHER";
   }

   Logger::get()->log(Logger::LOG_LEV_FATAL, "Caught signal %s.  TimeToDie is set.", signame.c_str());
   std::cerr << "Caught signal " << signame << " TimeToDie is set." << std::endl;
   return;
}

} //namespace VisAO

/// Error reporting function for global_error_report.
void error_report(const char* er, const char* file, int lno)
{

   Logger::get()->log(Logger::LOG_LEV_ERROR, "%s.  File: %s  Line: %i", er, file, lno);
   std::cerr  << er << " File: " << file << " Line: " << lno << std::endl;
}

/// Info logging function for global_log_info.
void log_info(const char* li)
{

   Logger::get()->log(Logger::LOG_LEV_INFO, "%s", li);
   std::cout  << li << "\n";
}

set_global_error_report(&error_report);

set_global_log_info(&log_info);


