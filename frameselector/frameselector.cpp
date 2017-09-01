/************************************************************
 *    frameselector.cpp
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Definitions for the real time frame selector
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file frameselector.cpp
 * \author Jared R. Males
 * \brief Definitions for the real time frame selector
 *
 *
 */

#include "frameselector.h"
#define SELECTOR_PING_CH  0
#define SHUTTER_AUTO_CH 1

namespace VisAO
{
   
///For the auto command handler, which live in the main thread.
frameselector * global_selector;
   
frameselector::frameselector(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   Create();
}

frameselector::frameselector(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   Create();
}

int frameselector::Create()
{
   std::string pathtmp;

   std::string visao_root = getenv("VISAO_ROOT");

   //block_sigio();
   signal(SIGIO, SIG_IGN);

   //Set up the ping_fifo_path
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["ping_fifo_path"];
   }
   catch(Config_File_Exception)
   {
      pathtmp = "fifos";
   }
   ping_fifo_path = visao_root + "/" + pathtmp +"/";
   _logger->log(Logger::LOG_LEV_INFO, "Set ping_fifo_path: %s", ping_fifo_path.c_str());

   std::string shutter_fifo_path = visao_root + "/fifos/shuttercontrol_com_auto";
   
   _logger->log(Logger::LOG_LEV_INFO, "Set shutter_fifo_path: %s", shutter_fifo_path.c_str());

   std::string sf_in = shutter_fifo_path + "_in";
   std::string sf_out = shutter_fifo_path + "_out";

   setup_fifo_list(5);
   setup_baseApp(0, 1, 1, 1, false);
   //setup_baseApp();

   set_fifo_list_channel(&fl, SELECTOR_PING_CH, RWBUFF_SZ, (char *)std::string(ping_fifo_path + MyName()+"_ping_in").c_str(), (char *)std::string(ping_fifo_path + MyName()+"_ping_out").c_str(), 0, (void *)this);

   set_fifo_list_channel(&fl, SHUTTER_AUTO_CH, RWBUFF_SZ, sf_out.c_str(), sf_in.c_str(),  0, 0);

   try
   {
      shmem_key = (int)(ConfigDictionary())["shmem_key"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "shmem_key is a required config parameter.");
      throw;
   }
   attached_to_shmem = false;

   
   statusboard_shmemkey = STATUS_frameselector;
   
   if(statusboard_shmemkey)
   {
      if(create_statusboard(sizeof(basic_status_board)) != 0)
      {
         statusboard_shmemptr = 0;
         _logger->log(Logger::LOG_LEV_ERROR, "Could not create status board.");
      }
      else
      {
         VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;
         strncpy(bsb->appname, MyFullName().c_str(), 25);
         bsb->max_update_interval = pause_time;
      }
   }

   frame_select = false;
   thresh = 0.;
   pthread_mutex_init(&select_mutex, NULL);
   pthread_cond_init(&strehl_ready_cond, NULL);
   pthread_mutex_init(&strehl_ready_mutex, NULL);

   return 0;
}

int frameselector::connect_shmem()
{
   sis = new sharedim_stack<float>;
   if(sis->attach_shm(shmem_key) != 0)
   {
      ERROR_REPORT("Error attaching to shared memory.");
      attached_to_shmem = false;
      delete sis;
      return -1;
   }
   attached_to_shmem = true;
   return 0;
}

int frameselector::Run()
{
   int totbytes;
   connect_shmem();

   signal(SIGIO, SIG_IGN);
   signal(RTSIGIO, SIG_IGN);

   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }

   //Startup the I/O signal handling thread
   if(start_signal_catcher(false) != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }

   sleep(1);

   //Now Block all I/O signals in this thread.
   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error blocking SIGIO in main thread.");
      return -1;
   }

//Setup to catch the ping in this high priority thread
   //The low priority signal catcher will block it.
   global_selector = this;
   
   fcntl(fl.fifo_ch[SELECTOR_PING_CH].fd_in, F_SETOWN, getpid());
   
   int rv = fcntl(fl.fifo_ch[SELECTOR_PING_CH].fd_in, F_SETSIG, RTSIGPING);
   if(rv < 0)
   {
      std::cerr << "Error changing signal.\n";
      perror("frameselector");
   }
   
   struct sigaction act;
   sigset_t sset;
   
   act.sa_sigaction = &strehl_ready;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&sset);
   act.sa_mask = sset;
   
   errno = 0;
   
   if(sigaction(RTSIGPING, &act, 0) < 0)
   {
      logss.str("");
      logss << "Error setting signal handler for RTSIGPING. Errno says: " << strerror(errno) << ".";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      return -1;
   }

   LOG_INFO("starting up . . .");

   while(!TimeToDie)
   {
      usleep(100);
      totbytes = 1;
      while(totbytes)
      {
         totbytes = read_fifo_channel(&fl.fifo_ch[SELECTOR_PING_CH]);
      }
   }

   pthread_join(signal_thread, 0);

   return 0;
}

int frameselector::selector()
{
   static int last_image = -1;
   int curr_image;

   if(!frame_select) return 0;

   if(pthread_mutex_trylock(&select_mutex) != 0 )
   {
      return 0;
   }
   
   curr_image = sis->get_last_image();

   if(curr_image == last_image || curr_image < -1) return 0;

   sim = sis->get_image(curr_image);
   last_image = curr_image;

   if(sim.imdata[7] >= thresh)
   {
      write_fifo_channel(SHUTTER_AUTO_CH, "1", 1, 0);
   }
   else
   {
      write_fifo_channel(SHUTTER_AUTO_CH, "0", 1, 0);
   }
   fl.fifo_ch[SHUTTER_AUTO_CH].timeout = 0;
   

   pthread_mutex_unlock(&select_mutex);
   
   return 0;
}

std::string  frameselector::remote_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received remote command: %s.", com.c_str());
   resp = common_command(com, CMODE_REMOTE);
   if(resp == "") resp = (std::string("UNKOWN COMMAND: ") + com + "\n");
   _logger->log(Logger::LOG_LEV_TRACE, "Response to remote command: %s.", resp.c_str());
   return resp;
}

std::string frameselector::local_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received local command: %s.", com.c_str());
   resp = common_command(com, CMODE_LOCAL);
   if(resp == "") resp = (std::string("UNKOWN COMMAND: ") + com + "\n");
   _logger->log(Logger::LOG_LEV_TRACE, "Response to local command: %s.", resp.c_str());
   return resp;
}

std::string  frameselector::script_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received script command: %s.", com.c_str());
   resp = common_command(com, CMODE_SCRIPT);
   if(resp == "") resp = (std::string("UNKOWN COMMAND: ") + com + "\n");
   _logger->log(Logger::LOG_LEV_TRACE, "Response to script command: %s.", resp.c_str());
   return resp;
}

std::string frameselector::auto_command(std::string com, char *seqmsg)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received auto command: %s.", com.c_str());
   resp = common_command(com, CMODE_AUTO);
   seqmsg = 0; //just to avoid the warning
   if(resp == "") resp = post_auto_command(com);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to auto command: %s.", resp.c_str());
   return resp;
}

std::string frameselector::common_command(std::string com, int cmode)
{
   int rv;
  
   char str[256];
   std::string resp;

   if(com == "thresh?")
   {
      snprintf(str, 256, "%0.4f\n", thresh);
      return str;
   }

   if(com == "state?")
   {
      return get_state_str();
   }
   
   if(com == "frame_select?")
   {
      snprintf(str, 256, "%i\n", frame_select);
      return str;
   }
   
   if(com == "start" && cmode == control_mode)
   {
      logss.str("");
      logss << "start selecting - thresh: " << thresh;
      
      _logger->log(Logger::LOG_LEV_INFO, logss.str().c_str());
            
      write_fifo_channel(SHUTTER_AUTO_CH, "AUTO", 4, 0);
      frame_select = true;
      fl.fifo_ch[SHUTTER_AUTO_CH].timeout = 0;
      return "0\n";
   }

   if(com == "stop" && cmode == control_mode)
   {
      write_fifo_channel(SHUTTER_AUTO_CH, "1", 1, 0);
      fl.fifo_ch[SHUTTER_AUTO_CH].timeout = 0;
      frame_select = false;
      sleep(1);
      write_fifo_channel(SHUTTER_AUTO_CH, "~AUTO", 5, 0);
      
      logss.str("");
      logss << "stop selecting";
      
      _logger->log(Logger::LOG_LEV_INFO, logss.str().c_str());
      
      return "0\n";
   }

   if(com.length() > 6)
   {
      if(com.substr(0,6) == "thresh" && cmode == control_mode)
      {
         double newthresh = strtod(com.substr(6, com.length()-6).c_str(), 0);
         if(newthresh >= 0. && newthresh < 1.0)
         {
            thresh = newthresh;
            return "0\n";
         }
         else return "-1\n";
      }

   }


   return "";
}

std::string frameselector::get_state_str()
{
   std::string sstr;
   char schr[50];   
   
   
   switch(control_mode)
   {
      case CMODE_NONE:
         sstr = "N";
         break;
      case CMODE_REMOTE:
         sstr = "R";
         break;
      case CMODE_LOCAL:
         sstr = "L";
         break;
      case CMODE_SCRIPT:
         sstr = "S";
         break;
      case CMODE_AUTO:
         sstr = "A";
         return sstr;
      default:
         sstr = "E";
         break;
   }
   
   snprintf(schr, 50, "%c %i %f", sstr[0], frame_select, thresh);
   
   return schr;
}


int frameselector::kill_me()
{
   pthread_cond_broadcast(&strehl_ready_cond);
   return 0;
}

int frameselector::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
     
      frameselector_status_board * fssb = (frameselector_status_board *) statusboard_shmemptr;
      
      fssb->frame_select = frame_select;
      fssb->thresh = thresh;
      
   }
   
   return 0;

}

void strehl_ready(int signum, siginfo_t *siginf, void *ucont)
{
   if(siginf->si_code == POLL_IN)
   {   
      global_selector->selector();
   }
}




}//namespace VisAO
