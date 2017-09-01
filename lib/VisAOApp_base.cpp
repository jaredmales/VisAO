/************************************************************
*    VisAOApp_base.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for the basic VisAO application, implementing the command fifos
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOApp_base.cpp
  * \author Jared R. Males
  * \brief Definitions for VisAOApp_base.
  *
*/

#include "VisAOApp_base.h"



namespace VisAO
{

//std::string global_app_name;


//Static fields
int VisAOApp_base::control_mode = VisAOApp_base::CMODE_NONE;

int VisAOApp_base::default_control_mode = -1;

VisAOApp_base::VisAOApp_base()
{
   
   init_fifo_list(&fl);
   com_path_remote[0]='\0';
   com_path_local[0]='\0';
   com_path_script[0]='\0';
   com_path_auto[0]='\0';
  
   wait_to = .2;
   
   getresuid(&euid_real, &euid_called, &suid);
   set_euid_called(); //immediately step down to unpriveleged uid.   
   RT_priority = 0;

   pause_time = 1.0;

   pthread_mutex_init(&my_mutex, NULL);
   
   profile = new VisAO::profiler(1000);
   use_profiler = 0;
   
   statusboard_shmemptr = 0; 
   statusboard_shmemkey = 0; 
   statusboard_shmemid = 0;
   
   signal(RTSIGIGN, SIG_IGN);

   dataFileOpen = 0;
   dataFileOpenTime = 0.;
   
   data_save_path = "data/syslogs";
   data_log_time_length = 120.;  
      
}

VisAOApp_base::~VisAOApp_base()
{
   if(dataFileOpen)
   {
      dataof.close();
      dataFileOpen = false;
   }
   
   return;
}
   
int VisAOApp_base::set_euid_called()
{
   errno = 0;
   if(seteuid(euid_called) < 0)
   {
      logss.str("");
      logss << "Setting effective user id to euid_called (" << euid_called << ") failed.  Errno says: " << strerror(errno) << ".";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      return -1;
   }
   
   return 0;
}

int VisAOApp_base::set_euid_real()
{
   errno = 0;
   if(seteuid(euid_real) < 0)
   {
      logss.str("");
      logss << "Setting effective user id to euid_real (" << euid_real << ") failed.  Errno says: " << strerror(errno) << ".";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      return -1;
   }
   
   return 0;
}


int VisAOApp_base::set_RT_priority(int prio)
{
   int rv1, rv;
   struct sched_param schedpar;
   
   schedpar.sched_priority = prio;
      
   rv1 = set_euid_called(); //Get the maximum privileges available
   
   errno = 0;
  
   if(prio > 0) rv = sched_setscheduler(0, VISAO_SCHED_POLICY, &schedpar);
   else rv = sched_setscheduler(0, SCHED_OTHER, &schedpar);
   
   if(rv < 0)
   {
      logss.str("");
      logss << "Setting scheduler priority to " << prio << " failed.  Errno says: " << strerror(errno) << ".  ";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      
      rv += -1;
   }
   else 
   {
      RT_priority = prio;
      logss.str("");
      logss << "Scheduler priority (RT_priority) set to " << RT_priority << ".";
      log_msg(Logger::LOG_LEV_INFO, logss.str());
   }
   
   rv += rv1 + set_euid_real(); //Go back to regular privileges
   
   return rv;
   
}

int VisAOApp_base::set_app_name(std::string an)
{
   app_name = an;
   
   return 0;
}

int VisAOApp_base::setup_baseApp(bool usethreads)
{
   int nsub;
   if(fl.nchan == 0)
   {
      error_report(Logger::LOG_LEV_ERROR, "fifo_list not yet setup.");
      return -1;
   }
   
   if(!usethreads)
   {
      nsub = 1;
      if(com_path_local[0] != '\0')
      {
         set_fifo_list_channel(&fl, fl.nchan - nsub, RWBUFF_SZ, (char *)(com_path_local + "_in").c_str(), (char *)(com_path_local + "_out").c_str(),
                   &com_local_handler, (void *) this);
      nsub++;
      }
      if(com_path_remote[0] != '\0')
      {
         set_fifo_list_channel(&fl, fl.nchan - nsub, RWBUFF_SZ, (char *)(com_path_remote + "_in").c_str(), (char *)(com_path_remote + "_out").c_str(), 
                               &com_remote_handler, (void *) this);
         nsub++;
      }
      if(com_path_script[0] != '\0')
      {
         set_fifo_list_channel(&fl, fl.nchan - nsub, RWBUFF_SZ, (char *)(com_path_script + "_in").c_str(), (char *)(com_path_script + "_out").c_str(), 
                               &com_script_handler, (void *) this);
         nsub++;
      }
      if(com_path_auto[0] != '\0')
      {
         set_fifo_list_channel(&fl, fl.nchan - nsub, RWBUFF_SZ, (char *)(com_path_auto + "_in").c_str(), (char *)(com_path_auto + "_out").c_str(), 
                               &com_auto_handler, (void *) this);
         nsub++;
      }
   }
   else
   {
      nsub = 1;
      if(com_path_local[0] != '\0')
      {
         set_fifo_list_channel(&fl, fl.nchan - nsub, RWBUFF_SZ, (char *)(com_path_local + "_in").c_str(), (char *)(com_path_local + "_out").c_str(),
         &com_local_thread_handler, (void *) this);
         nsub++;
      }
      if(com_path_remote[0] != '\0')
      {
         set_fifo_list_channel(&fl, fl.nchan - nsub, RWBUFF_SZ, (char *)(com_path_remote + "_in").c_str(), (char *)(com_path_remote + "_out").c_str(), 
                               &com_remote_thread_handler, (void *) this);
         nsub++;
      }
      if(com_path_script[0] != '\0')
      {
         set_fifo_list_channel(&fl, fl.nchan - nsub, RWBUFF_SZ, (char *)(com_path_script + "_in").c_str(), (char *)(com_path_script + "_out").c_str(), 
                               &com_script_thread_handler, (void *) this);
         nsub++;
      }
      if(com_path_auto[0] != '\0')
      {
         set_fifo_list_channel(&fl, fl.nchan - nsub, RWBUFF_SZ, (char *)(com_path_auto + "_in").c_str(), (char *)(com_path_auto + "_out").c_str(), 
                               &com_auto_thread_handler, (void *) this);
         nsub++;
      }
   }       
   
   return 0;
}

int VisAOApp_base::setup_baseApp(bool remfifo, bool locfifo, bool scrfifo, bool autfifo, bool usethreads)
{
   if(remfifo)
   {
      com_path_remote = com_path + "_com_remote";
      logss.str("");
      logss << "Set com_path_remote: " << com_path_remote << ".";
      log_msg(Logger::LOG_LEV_INFO, logss.str());
   }
   if(locfifo)
   {
      com_path_local = com_path + "_com_local";
      logss.str("");
      logss << "Set com_path_local: " << com_path_local << ".";
      log_msg(Logger::LOG_LEV_INFO, logss.str());
   }
   if(scrfifo)
   {
      com_path_script = com_path + "_com_script";
      logss.str("");
      logss << "Set com_path_script: " << com_path_script << ".";
      log_msg(Logger::LOG_LEV_INFO, logss.str());
   }
   if(autfifo)
   {
      com_path_auto = com_path + "_com_auto";
      logss.str("");
      logss << "Set com_path_auto: " << com_path_auto << ".";
      log_msg(Logger::LOG_LEV_INFO, logss.str());
   }
   
   return setup_baseApp(usethreads);
}



int VisAOApp_base::setup_fifo_list(int nfifos)
{
   return ::setup_fifo_list(&fl, nfifos);
}

int VisAOApp_base::connect_fifo_list()
{       
   std::cout << "Connecting fifo list\n";
   return ::connect_fifo_list(&fl);
}

int VisAOApp_base::connect_fifo_list_nolock()
{
   return ::connect_fifo_list_nolock(&fl);
}

int VisAOApp_base::setup_sigio()
{
   struct sigaction act;
   sigset_t sset;
   
   act.sa_handler = &catch_fifo_response_list;
   act.sa_flags = 0;
   sigemptyset(&sset);
   act.sa_mask = sset;   
   
   errno = 0;
   
   if(sigaction(SIGIO, &act, 0) < 0)
   {
      logss.str("");
      logss << "Error setting signal handler for SIGIO. Errno says: " << strerror(errno) << ".";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      return -1;
   }
   
   log_msg(Logger::LOG_LEV_INFO, "Installed signal handling for SIGIO");
   
   return 0;
}

int VisAOApp_base::setup_RTsigio()
{
   struct sigaction act;
   sigset_t sset;

   set_fifo_list_rtsig(&fl);
      
   act.sa_sigaction = &catch_fifo_pending_reads;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&sset);
   act.sa_mask = sset;   

   errno = 0;

   if(sigaction(RTSIGIO, &act, 0) < 0)
   {
      logss.str("");
      logss << "Error setting signal handler for RTSIGIO. Errno says: " << strerror(errno) << ".";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      return -1;
   }

   log_msg(Logger::LOG_LEV_INFO, "Installed signal handling for RTSIGIO");

   act.sa_handler = &catch_fifo_standard_sigio;
   act.sa_flags = 0;

   errno = 0;
   if(sigaction(SIGIO, &act, 0) < 0)
   {
      logss.str("");
      logss << "Error setting backup signal handler for SIGIO. Errno says: " << strerror(errno) << ".";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      return -1;
   }
   
   log_msg(Logger::LOG_LEV_INFO, "Installed backup signal handling for SIGIO");
   
   signal(RTSIGIGN, SIG_IGN);
   log_msg(Logger::LOG_LEV_INFO, "Ignoring RTSIGIGN");
   
   return 0;
}


int VisAOApp_base::check_fifo_list_RTpending()
{
   int rv;
   if(fl.RTSIGIO_overflow)
   {
      //Do something here!
      rv = -1;
      error_report(Logger::LOG_LEV_ERROR, "R/T SIGIO overflow.");
      
      catch_fifo_response_list(0); //This will go through all current inputs.
      
      fl.RTSIGIO_overflow = 0;
   }
   while(fl.tot_pending_reads > 0 && !TimeToDie)
   {
      rv += fifo_list_do_pending_read(&fl);
   }
   return rv;
}

int VisAOApp_base::write_fifo_channel(int ch, const char *com, int comlen, std::string * resp)
{
   int rv;

   //Wait until somebody opens the other end.
   if(fl.fifo_ch[ch].timeout)
   {
      /*struct stat fst;
      fstat(fl.fifo_ch[ch].fd_out, &fst);
      if(fl.fifo_ch[ch].last_atime_out == 0) fl.fifo_ch[ch].last_atime_out = fst.st_atime;

      if(fl.fifo_ch[ch].last_atime_out == fst.st_atime)
      {
         *resp = "";
         return -1;
      }
      else
      {
         fl.fifo_ch[ch].last_atime_out = 0;
      }*/
      //mod time won't clear a timeout due to server slowness.
      //Have to explicitly try and open the channel until we get the error.
      int test_fd = wopen_nonblock(fl.fifo_ch[ch].outfname, 0);

      //std::cout << "test_fd result: " << test_fd << "\n";
      if(test_fd > 0 )
      {
         close(test_fd);
         return -1;
      }
   }
   
   rv = ::write_fifo_channel(&fl.fifo_ch[ch], com, comlen);

   if(rv < 0) 
   {
      logss.str("");
      logss << "Error returned from ::write_fifo_channel on channel " << ch << ".";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      return rv;
   }
   
   if(!resp) return rv;
   
   return get_fifo_channel_response(resp, ch);
}

int VisAOApp_base::write_fifo_channel(int ch, const char *com, int comlen, std::string * resp, char *sm)
{
   int rv;

   //Wait until somebody opens the other end.
   if(fl.fifo_ch[ch].timeout)
   {
      struct stat fst;
      fstat(fl.fifo_ch[ch].fd_out, &fst);
      if(fl.fifo_ch[ch].last_atime_out == 0) fl.fifo_ch[ch].last_atime_out = fst.st_atime;
      
      if(fl.fifo_ch[ch].last_atime_out == fst.st_atime)
      {
         *resp = "";
         return -1;
      }
      else
      {
         fl.fifo_ch[ch].last_atime_out = 0;
      }
   }
   
   rv = ::write_fifo_channel_seqmsg(&fl.fifo_ch[ch], com, comlen, sm);
   
   if(rv < 0) 
   {
      logss.str("");
      logss << "Error returned from ::write_fifo_channel on channel " << ch << ".";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      return rv;
   }
   
   if(!resp) return rv;
   
   return get_fifo_channel_response(resp, ch);
}

int VisAOApp_base::get_fifo_channel_response(std::string * resp, int ch)
{
   double startT;
   int nottimedout = 0;
   startT = get_curr_time();  
   
   //if(using_signal_thread)
   if(fl.fifo_ch[ch].input_handler)
   {
      //Here the signal handling thread should take care of reading the fifo for us.
      while((fl.fifo_ch[ch].server_response[0] == '\0') && (get_curr_time() - startT < wait_to) &&!TimeToDie)
      {

         sched_yield();
         usleep(10); //give up the scheduler in case the signal thread is waiting.
                     //So far usleep(1) is too short
                     //usleep(10) works.
      }
   }
   else
   {
      //Here we're either blocked or just using the RT signals without the separate thread
      //so we have to do the work.
      //std::cerr << "Polling     " << get_curr_time() << "\n";
      
      pollfd pfd;
      pfd.fd = fl.fifo_ch[ch].fd_in;
      pfd.events = POLLIN;
      pfd.revents = 0;

      errno = 0;
      if(fl.fifo_ch[ch].timeout)
      {
         nottimedout = poll(&pfd, 1, (int) (.1*wait_to*1000.));
      }
      else
      {
         nottimedout = poll(&pfd, 1, (int) (wait_to*1000.));
      }
      if(nottimedout < 0)
      {
         logss.str("");
         logss << "Poll returned error (not a timeout).  errno says: " <<  strerror(errno) << "\n";
         error_report(Logger::LOG_LEV_ERROR, logss.str());
         return -1;
      }
      read_fifo_channel(&fl.fifo_ch[ch]); 
      
      //std::cerr << "Done Polling " << get_curr_time() << "\n";
   }
   
   //Check for timeout
   if(fl.fifo_ch[ch].server_response[0] == '\0' || !nottimedout)
   {
      *resp = "";
      if(!fl.fifo_ch[ch].timeout)
      {   
         logss.str("");
         logss << "FIFO response timeout.  wait_to = " << wait_to << "secs.";
         error_report(Logger::LOG_LEV_ERROR, logss.str());
         fl.fifo_ch[ch].timeout = 1;
         struct stat fst;
         fstat(fl.fifo_ch[ch].fd_out, &fst);
         fl.fifo_ch[ch].last_atime_out = fst.st_atime;
      }
      return -1;
   }

   fl.fifo_ch[ch].last_atime_out = 0;
   
   //If not a timeout, check if this clears a timeout
   if(fl.fifo_ch[ch].timeout)
   {
      logss.str("");
      logss << "FIFO response timeout clear.";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      fl.fifo_ch[ch].timeout = 0;
   }
   
   //Ok, now copy and getouta.
   *resp = fl.fifo_ch[ch].server_response;
   
   return 0; 
   
}

int VisAOApp_base::set_wait_to(double to)
{
   if(to < 0) 
   {
      wait_to = 0;
      error_report(Logger::LOG_LEV_ERROR, "Attempt to set wait_to to negative number.");
   }
   else  wait_to = to;
   
   logss.str("");
   logss << "Wait timeout (wait_to) set to " << wait_to << " secs.";
   
   log_msg(Logger::LOG_LEV_INFO, logss.str());
              
   if(to < 0) return -1;
   else return 0;
}

int VisAOApp_base::request_control(int ctype)
{
   return request_control(ctype, 0);
}

int VisAOApp_base::request_control(int ctype, bool override)
{
   if(ctype >= control_mode || override)
   {
      control_mode = ctype;
      return control_mode;
   }
   else return -1;
}

std::string VisAOApp_base::__remote_command(std::string com, char *seqmsg)
{
   if(com == "") return "";//ignore spurious SIGIO events

   //Before we do anything else, check if AUTO
   if(control_mode == CMODE_AUTO && com[0] != 'X')
   {
      return "A\n";
   }

   std::string response;

   //Is this an override to remote control command?
   if(control_mode > CMODE_REMOTE && com[0] == 'X')
   {
      if(com == "XREMOTE")
      {
         if(request_control(CMODE_REMOTE, 1) == CMODE_REMOTE) return "REMOTE\n";
         else return "FAIL\n";
      }
      else return "UNKOWN COMMAND:" + com + "\n";
   }

   //Is this a request to take remote control in a non-override situation?
   if(com == "REMOTE" || com == "XREMOTE")
   {
      if(control_mode <= CMODE_REMOTE) 
      {
         if(request_control(CMODE_REMOTE) == CMODE_REMOTE) return "REMOTE\n";
         else return "FAIL\n";
      }
      else
      {         
         if(control_mode == CMODE_LOCAL) return "L\n";
         else return "A\n";  // cant' get here.
      }
   }

   //Is this a request to give up remote control?
   if(control_mode == CMODE_REMOTE && com == "~REMOTE")
   {
      if(default_control_mode > -1 && default_control_mode == CMODE_REMOTE)
      {
         if(request_control(default_control_mode,1) == default_control_mode) return control_mode_string() +"\n";
         else return "FAIL\n";
      }
      else
      {
         if(request_control(CMODE_NONE,1) == CMODE_NONE) return "NONE\n";
         else return "FAIL\n";
      }
   }
   
   //If we're here, must be an app specific command
   return remote_command(com, seqmsg);
}

std::string VisAOApp_base::remote_command(std::string com)
{
   return "UNKOWN COMMAND: " + com + "\n";
}

std::string VisAOApp_base::remote_command(std::string com, char *seqmsg __attribute__((unused)))
{
   seqmsg = 0; //just to suppress the unused warning.
   return remote_command(com);
}

std::string VisAOApp_base::__local_command(std::string com, char *seqmsg)
{

   if(com == "") return "";//ignore spurious SIGIO events
        
   //Before we do anything else, check if AUTO
   if(control_mode == CMODE_AUTO && com[0] != 'X')
   {
      return "A\n";
   }

   int pos = com.find_first_of("\n\r", 0);
   if(pos > -1) com.erase(pos, com.length()-pos);

   std::string response;
   
   //Is this an override to local control command?
   if(control_mode > CMODE_LOCAL && com[0] == 'X')
   {
      if(com == "XLOCAL")
      {
         if(request_control(CMODE_LOCAL, 1) == CMODE_LOCAL) return "LOCAL\n";
         else return "FAIL\n";
      }
      else return "UNKOWN COMMAND:" + com + "\n";
   }

   //Is this a request to take local control in a non-override situation?
   if(com == "LOCAL" || com == "XLOCAL")
   {
      if(control_mode <= CMODE_LOCAL) 
      {
         if(request_control(CMODE_LOCAL) == CMODE_LOCAL) return "LOCAL\n";
         else return "FAIL\n";
      }
      else
      {
         return "A\n";
      }
   }
   
   //Is this a request to relinquish local control?
   if(control_mode == CMODE_LOCAL && com == "~LOCAL")
   {
      if(default_control_mode > -1 && default_control_mode != CMODE_LOCAL)
      {
         if(request_control(default_control_mode,1) == default_control_mode) return control_mode_string() +"\n";
         else return "FAIL\n";
      }
      else
      {
         if(request_control(CMODE_NONE,1) == CMODE_NONE) return "NONE\n";
         else return "FAIL\n";
      }
   }
   
   return local_command(com,seqmsg);
}

std::string VisAOApp_base::local_command(std::string com)
{
   return "UNKOWN COMMAND: " + com + "\n";
}

std::string VisAOApp_base::local_command(std::string com, char *seqmsg __attribute__((unused)))
{
   seqmsg = 0; //just to suppress the unused warning.
   return local_command(com);
}

std::string VisAOApp_base::__script_command(std::string com, char *seqmsg)
{
 
   if(com == "") return "";//ignore spurious SIGIO events
        
   //Before we do anything else, check if AUTO
   if(control_mode == CMODE_AUTO && com[0] != 'X')
   {
      return "A\n";
   }

   int pos = com.find_first_of("\n\r", 0);
   if(pos > -1) com.erase(pos, com.length()-pos);

   std::string response;
   
        //Is this an override to script control command?
   if(control_mode > CMODE_SCRIPT && com[0] == 'X')
   {
      if(com == "XSCRIPT")
      {
         if(request_control(CMODE_SCRIPT, 1) == CMODE_SCRIPT) return "SCRIPT\n";
         else return "FAIL\n";
      }
      else return "UNKOWN COMMAND:" + com + "\n";
   }

   //Is this a request to take script control in a non-override situation?
   if(com == "SCRIPT" || com == "XSCRIPT")
   {
      if(control_mode <= CMODE_SCRIPT) 
      {
         if(request_control(CMODE_SCRIPT) == CMODE_SCRIPT) return "SCRIPT\n";
         else return "FAIL\n";
      }
      else
      {
         return "A\n";
      }
   }
   
   //Is this a request to relinquish script control?
   if(control_mode == CMODE_SCRIPT && com == "~SCRIPT")
   {
      if(default_control_mode > -1 && default_control_mode != CMODE_SCRIPT)
      {
         if(request_control(default_control_mode,1) == default_control_mode) return control_mode_string() +"\n";
         else return "FAIL\n";
      }
      else
      {
         if(request_control(CMODE_NONE,1) == CMODE_NONE) return "NONE\n";
         else return "FAIL\n";
      }
   }
   
   return script_command(com,seqmsg);
}

std::string VisAOApp_base::script_command(std::string com)
{
   return "UNKOWN COMMAND: " + com + "\n";
}

std::string VisAOApp_base::script_command(std::string com, char *seqmsg __attribute__((unused)))
{
   seqmsg = 0; //just to suppress the unused warning.
   return script_command(com);
}

std::string VisAOApp_base::auto_command(std::string com, char *seqmsg)
{
   return post_auto_command(com,seqmsg);
}


std::string VisAOApp_base::post_auto_command(std::string com, char *seqmsg __attribute__((unused)))
{
   
   if(com == "") return "";//ignore spurious SIGIO events
   
   int pos = com.find_first_of("\n\r", 0);
   if(pos > -1) com.erase(pos, com.length()-pos);
        
   
   //Is this a request to take auto control?
   if(com == "AUTO" || com == "XAUTO")
   {
      if(control_mode <= CMODE_AUTO) 
      {
         if(request_control(CMODE_AUTO) == CMODE_AUTO) return "AUTO\n";
         else return "FAIL\n";
      }
      else
      {         
         return "?\n";  // can't get here.
      }
   }

   
   //Is this a request to relinquish auto control?
   if(control_mode == CMODE_AUTO && com == "~AUTO")
   {
      
      if(default_control_mode > -1 && default_control_mode != CMODE_AUTO)
      {
         if(request_control(default_control_mode,1) == default_control_mode) return control_mode_string() +"\n";
         else return "FAIL\n";
      }
      else
      {
         if(request_control(CMODE_NONE, 1) == CMODE_NONE) return "NONE\n";
         else return "FAIL\n";
      }
   }
   
   seqmsg = 0; //to suppress the warning.
   return "UNKOWN COMMAND: " + com + "\n";
}

std::string VisAOApp_base::control_mode_string()
{
   switch(control_mode)
   {
      case CMODE_REMOTE:
         return "REMOTE";
      case CMODE_LOCAL:
         return "LOCAL";
      case CMODE_SCRIPT:
         return "SCRIPT";
      case CMODE_AUTO:
         return "AUTO";
      default:
         return "NONE";
   }
}

std::string VisAOApp_base::control_mode_response()
{
   switch(control_mode)
   {
      case CMODE_REMOTE:
         return "R\n";
      case CMODE_LOCAL:
         return "L\n";
      case CMODE_SCRIPT:
         return "S\n";
      case CMODE_AUTO:
         return "A\n";
      default:
         return "N\n";
   }
}


void VisAOApp_base::error_report(int LogLevel, std::string emsg)
{
   std::cerr << get_app_name() << ": " << emsg << "\n";
   log_msg(LogLevel, emsg);
}
      
void VisAOApp_base::log_msg(int LogLevel, std::string lmsg)
{
   std::cout << get_app_name() << ": " << lmsg << " (log level " << LogLevel << ")\n";
   return;
}

int VisAOApp_base::start_profiler()
{
   struct sched_param schedpar;
   schedpar.sched_priority = 0;

   pthread_t th;
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   //pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
   pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
   pthread_attr_setschedparam(&attr, &schedpar);

   return pthread_create(&th, &attr, &__start_profiler, (void *) profile);
}

int VisAOApp_base::create_statusboard(size_t sz)
{
   int rv;
   size_t sizecheck;
   
   rv = create_shmem(&statusboard_shmemid, statusboard_shmemkey, sz);
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck,  statusboard_shmemkey, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else rv = -1;
   }
   
   return rv;
   
}
 
int VisAOApp_base::set_statusboard_shmemkey(key_t mkey)
{
   statusboard_shmemkey = mkey;
   return 0;
}
      
int VisAOApp_base::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;

      bsb->control_mode = control_mode;

      clock_gettime(CLOCK_REALTIME, &bsb->last_update);
   }
   return 0;
}


std::string VisAOApp_base::getDataFileName()
{
   std::string fname;
   char buffer[21];
   timeval tv;

   gettimeofday(&tv, 0);
   get_visao_filename(buffer, &tv);
   
   fname = getenv("VISAO_ROOT");
   fname += "/";
   fname += data_save_path;
   fname += "/";
   fname += data_file_prefix;
   fname += "_";
   fname += buffer;
   fname += ".txt";

   return fname;
}

int VisAOApp_base::checkDataFileOpen()
{
   //Close the file if it has been opened for long enough
   if(dataFileOpen && (get_curr_time()-dataFileOpenTime > data_log_time_length))
   {
      dataof.close();
      dataFileOpen = false;
   }
   
   if(!dataFileOpen)
   {
      std::string fname;
      fname = getDataFileName();
      dataof.open(fname.c_str());
      if(!dataof.good())
      {
         Logger::get()->log(Logger::LOG_LEV_ERROR, "Error opening data file.  AOsystem data may not be logged correctly");
         dataFileOpen = false;
         dataFileOpenTime = 0;
         return -1;
      }
      else
      {
         dataFileOpen = true;
         dataFileOpenTime = get_curr_time();
      }
   }      
   
   return 0;
}

void VisAOApp_base::dataLogger(timeval tv __attribute__((unused)))
{
   return;
}

 
int com_remote_handler(fifo_channel * fc)
{
   std::string tmp, com, response;
   VisAOApp_base * vab = (VisAOApp_base *)fc->auxdata;

   read_fifo_channel(fc);

   com = fc->server_response;
   
   //Process each line read
   while(getnextline(&tmp, &com))
   {
      response = vab->__remote_command(tmp,0);
      
      write_fifo_channel(fc, (char *)response.c_str(), response.length());
   }

   return 0;
}

int com_local_handler(fifo_channel * fc)
{
   std::string tmp, com, response;
   VisAOApp_base * vab = (VisAOApp_base *)fc->auxdata;
   
   read_fifo_channel(fc);

   com = fc->server_response;

   //Process each line read
        while(getnextline(&tmp, &com))
        {
      if(tmp == "")
      {
         std::cerr << "Fix this bug! " << __FILE__ << " line " << __LINE__ << "\n";
         break;
      }
        response = vab->__local_command(tmp,0);
            
        write_fifo_channel(fc, (char *)response.c_str(), response.length());
        }
        
   return 0;
}

int com_script_handler(fifo_channel * fc)
{
   std::string tmp, com, response;
   VisAOApp_base * vab = (VisAOApp_base *)fc->auxdata;
   
   read_fifo_channel(fc);
   
   com = fc->server_response;
   
   //Process each line read
   while(getnextline(&tmp, &com))
   {
      response = vab->__script_command(tmp,0);
      
      write_fifo_channel(fc, (char *)response.c_str(), response.length());
   }
   
   return 0;
}

int com_auto_handler(fifo_channel * fc)
{
   std::string response;
   VisAOApp_base * vab = (VisAOApp_base *)fc->auxdata;
   
   read_fifo_channel(fc);

   //No checking for multiple lines done.
   response = vab->auto_command(fc->server_response, fc->seqmsg);
   
   write_fifo_channel(fc, (char *)response.c_str(), response.length());
    
   return 0;
}

int com_remote_thread_handler(fifo_channel * fc)
{
   pthread_t th;
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);

   pthread_create(&th, &attr, &__com_remote_thread_handler, (void *) fc);
        
   return 0;
}

void * __com_remote_thread_handler(void * ptr)
{
   fifo_channel *fc = (fifo_channel *)ptr;
   VisAOApp_base * vab = (VisAOApp_base *)fc->auxdata;
        
   pthread_mutex_lock(&vab->my_mutex);
        
   com_remote_handler(fc);
        
   pthread_mutex_unlock(&vab->my_mutex);
        
   return 0;
}

int com_local_thread_handler(fifo_channel * fc)
{
   pthread_t th;
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);

   pthread_create(&th, &attr, &__com_local_thread_handler, (void *) fc);

   return 0;
}

void * __com_local_thread_handler(void *ptr)
{
   fifo_channel *fc = (fifo_channel *)ptr;
   VisAOApp_base * vab = (VisAOApp_base *)fc->auxdata;
   
   pthread_mutex_lock(&vab->my_mutex);
   
   com_local_handler(fc);
   
   pthread_mutex_unlock(&vab->my_mutex);
   
   return 0;
}

int com_script_thread_handler(fifo_channel * fc)
{
   pthread_t th;
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
   
   pthread_create(&th, &attr, &__com_script_thread_handler, (void *) fc);
   return 0;
}

void *__com_script_thread_handler(void *ptr)
{
   fifo_channel *fc = (fifo_channel *)ptr;
   VisAOApp_base * vab = (VisAOApp_base *)fc->auxdata;
   
   pthread_mutex_lock(&vab->my_mutex);
   
   com_script_handler(fc);
   
   pthread_mutex_unlock(&vab->my_mutex);
   
   return 0;
}

int com_auto_thread_handler(fifo_channel * fc)
{
   pthread_t th;
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
   
   pthread_create(&th, &attr, &__com_auto_thread_handler, (void *) fc);
   return 0;
}

void *__com_auto_thread_handler(void *ptr)
{
   fifo_channel *fc = (fifo_channel *)ptr;
   VisAOApp_base * vab = (VisAOApp_base *)fc->auxdata;
   
   pthread_mutex_lock(&vab->my_mutex);
   
   com_auto_handler(fc);
   
   pthread_mutex_unlock(&vab->my_mutex);
   
   return 0;
}

int getnextline(std::string *comln, std::string * com)
{
   std::string tmp;
   int pos;
   
   if(com->length() == 0)
   {
      return 0;
   }
   pos = com->find_first_of("\n\r", 0);
   if(pos == -1)
   {
      *comln = *com;
      *com = "";
      return 1;
   }
   if(pos < (int)com->length()-1)
   {
      if((*com)[pos+1] == '\n' || (*com)[pos+1] == '\r') com->erase(pos, 1);
   }
   
   *comln = com->substr(0, pos);
   com->erase(0, pos+1);
   
   return 1;
}


void * __start_profiler(void * ptr)
{
   ((VisAO::profiler *) ptr)->start();
   return 0;
}

} //namespace VisAO

