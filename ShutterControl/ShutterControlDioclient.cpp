/************************************************************
*    ShutterControlDioclient.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Generic shutter controller as a client of the dioserver
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterControlDioclient.cpp
  * \author Jared R. Males (jrmales@email.arizona.edu)
  * \brief Definitions for the shutter controller as a client of the dioserver
  *
*/

#include "ShutterControlDioclient.h"

#define SHUTTER_AUTO_FIFO_CH  2

namespace VisAO
{
   
///For the auto command handler, which live in the main thread.
ShutterControlDioclient * global_shutter;

ShutterControlDioclient::ShutterControlDioclient(int argc, char **argv) throw (AOException) :  VisAOApp_standalone(argc, argv)
{
   initialize_ShutterControl();
}


ShutterControlDioclient::ShutterControlDioclient(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   initialize_ShutterControl();
}


ShutterControlDioclient::~ShutterControlDioclient()
{
   saveInit();
}


int ShutterControlDioclient::initialize_ShutterControl() throw (AOException)
{
   char fin[MAX_FNAME_SZ], fout[MAX_FNAME_SZ];
   
   setup_fifo_list(6);
   
   std::string pathtmp;
   std::string visao_root = getenv("VISAO_ROOT");
   
   //Set up the dioserver channel numbers
   try
   {
      dio_ch_set = (int)(ConfigDictionary())["dio_ch_set"];
      _logger->log(Logger::LOG_LEV_INFO, "dioserver channel for setting state (dio_ch_set) set to %i.", dio_ch_set);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "dioserver channel for setting state (dio_ch_set) not set in config file.");
      throw;
   }
   try
   {
      dio_ch_get = (int)(ConfigDictionary())["dio_ch_get"];
      _logger->log(Logger::LOG_LEV_INFO, "dioserver channel for getting state (dio_ch_get) set to %i.", dio_ch_get);
   }
   catch(Config_File_Exception)
   {
     _logger->log(Logger::LOG_LEV_FATAL, "dioserver channel for getting state (dio_ch_get) not set in config file.");
      throw;
   }

   //Set up the fifo_path
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["diofifo_path"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_INFO, "Setting diofifo_path to default");
      pathtmp = "fifos";
   }
   diofifo_path = visao_root + "/" + pathtmp + "/diofifo";
   _logger->log(Logger::LOG_LEV_INFO, "Set diofifo_path: %s", diofifo_path.c_str());	
   
   get_dio_fnames(fin, fout, (char *)diofifo_path.c_str(), dio_ch_set);
   set_fifo_list_channel(&fl, 0, 100,fin, fout, 0, 0);

   get_dio_fnames(fin, fout, (char *)diofifo_path.c_str(), dio_ch_get);
   set_fifo_list_channel(&fl, 1, 100, fin, fout, 0, 0);
   
   
   setup_baseApp(1, 1, 1, 0, false);

   com_path_auto = com_path + "_com_auto";
   set_fifo_list_channel(&fl, SHUTTER_AUTO_FIFO_CH, RWBUFF_SZ, (char *)(com_path_auto + "_in").c_str(), (char *)(com_path_auto + "_out").c_str(), 0, (void *) this);
   //fl.fifo_ch[SHUTTER_AUTO_FIFO_CH].input_handler = 0;

   
   //Read dead_time from config file
   try
   {
      dead_time = (float32)(ConfigDictionary())["dead_time"];
      _logger->log(Logger::LOG_LEV_INFO, "Shutter dead time (dead_time) set to %f sec.", (float) dead_time);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "Shutter dead time (dead_time) not set in config file.");
      throw;
   }
   
   //Read ignore_hw_state from config file.
   try
   {
      ignore_hw_state = (int)(ConfigDictionary())["ignore_hw_state"] != 0;
      
      if(ignore_hw_state)   _logger->log(Logger::LOG_LEV_INFO, "Shutter set to ignore hardware state (ignore_hw_state)");
      else _logger->log(Logger::LOG_LEV_INFO, "Shutter set to use hardware state (ignore_hw_state)");
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_INFO, "Shutter set to use hardware state (ignore_hw_state)");
   }

    //Read the power outlet from config file.
 
   try
   {
      power_outlet = (int)(ConfigDictionary())["powerOutlet"] != 0;
      
      _logger->log(Logger::LOG_LEV_INFO, "Shutter powerOutlet set to %i.", power_outlet);
   }
   catch(Config_File_Exception)
   {
      power_outlet = 4;
      _logger->log(Logger::LOG_LEV_INFO, "Shutter powerOutlet set to default %i.", power_outlet);
   }

   //Set the shared memory key
   try
   {
      shmem_key = (int)(ConfigDictionary())["shmem_key"];
   }
   catch(Config_File_Exception)
   {
      shmem_key = 8001;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the shared memory key (shmem_key): %i", shmem_key);
   
   //Set the number of timestamps in the shared ring buffer
   try
   {
      num_timestamps = (int)(ConfigDictionary())["num_timestamps"];
   }
   catch(Config_File_Exception)
   {
      num_timestamps = 72000;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set number of images in the shared image buffer (num_timestamps): %i", num_timestamps);


   try
   {
      initSaveInterval = (int)(ConfigDictionary())["initSaveInterval"];
   }
   catch(Config_File_Exception)
   {
      initSaveInterval = 200;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set interval to save init info to disk (initSaveInterval): %i", initSaveInterval);

   
   if(init_vars)
   {
      try
      {
         cycleNo = (uint64) (*init_vars)["cycleNo"];
      }
      catch(Config_File_Exception)
      {
         cycleNo = 0;
      }
   }
   

   try
   {
      logger_pause = (double)(ConfigDictionary())["logger_pause"];
   }
   catch(Config_File_Exception)
   {
      logger_pause = 5.;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set logger pause time to (logger_pause): %f secs", logger_pause);

   try
   {
      data_log_time_length = (double)(ConfigDictionary())["data_log_time_length"];
   }
   catch(Config_File_Exception)
   {
      data_log_time_length = 120.;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set logger file length to (data_log_time_length): %f secs", data_log_time_length);


   try
   {
      save_path = (std::string)(ConfigDictionary())["save_path"];
   }
   catch(Config_File_Exception)
   {
      save_path = "data/syslogs";
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set logger save path to (save_path): %s", save_path.c_str());

   //Now make the directory
   std::string com = "mkdir -p ";
   com += getenv("VISAO_ROOT");
   com += "/";
   com += save_path;
   system(com.c_str());
   
   size_t sz;
   psb = (VisAO::power_status_board*) attach_shm(&sz,  13000, 0);

   if(psb)
   {
      switch(power_outlet)
      {
         case 1: powerOutlet = &psb->outlet1_state; break;
         case 2: powerOutlet = &psb->outlet2_state; break;
         case 3: powerOutlet = &psb->outlet3_state; break;
         case 4: powerOutlet = &psb->outlet4_state; break;
         case 5: powerOutlet = &psb->outlet5_state; break;
         case 6: powerOutlet = &psb->outlet6_state; break;
         case 7: powerOutlet = &psb->outlet7_state; break;
         case 8: powerOutlet = &psb->outlet8_state; break;
         default: powerOutlet = 0;
      }
   }

   //Init the status board
   statusboard_shmemkey = STATUS_shutterctrl;

   if(create_statusboard(sizeof(shutter_status_board)) != 0)
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


   //profile->set_base_path("/home/jaredmales/Source/Magellan/visao_base/profile/ShutterControl");
   return 0;

}

int ShutterControlDioclient::getPowerStatus()
{
   int powerState;
   if(!psb)
   {
      size_t sz;
      psb = (VisAO::power_status_board*) attach_shm(&sz,  13000, 0);

      if(psb)
      {
         switch(power_outlet)
         {
            case 1: powerOutlet = &psb->outlet1_state; break;
            case 2: powerOutlet = &psb->outlet2_state; break;
            case 3: powerOutlet = &psb->outlet3_state; break;
            case 4: powerOutlet = &psb->outlet4_state; break;
            case 5: powerOutlet = &psb->outlet5_state; break;
            case 6: powerOutlet = &psb->outlet6_state; break;
            case 7: powerOutlet = &psb->outlet7_state; break;
            case 8: powerOutlet = &psb->outlet8_state; break;
            default: powerOutlet = 0;
         }
      }
   }

   if(psb && powerOutlet)
   {
      if(get_curr_time() - ts_to_curr_time(&psb->last_update) > 3.*psb->max_update_interval) powerState = -1;
      else powerState = *powerOutlet;
   }
   else powerState = 0;

   return powerState;
}

int ShutterControlDioclient::Run()
{
   return start_ShutterControl();
}

int ShutterControlDioclient::start_ShutterControl()
{
   if(sis.create_shm(shmem_key, num_timestamps, sizeof(sharedim_stack_header) + num_timestamps*sizeof(intptr_t) + (num_timestamps)*(sizeof(sharedim<char>) + 1*sizeof(char))) != 0)
   {
      ERROR_REPORT("Error attaching to shared memory for sharedim_stack.");
      return -1;
   }
   
   sis.header->save_sequence = 0;

   //Startup the data logging thread
   if(launchDataLogger() != 0)
   {
      ERROR_REPORT("Error starting data logging thread.");
      return -1;
   }
   
   signal(SIGIO, SIG_IGN);
   signal(RTSIGIO, SIG_IGN);
   
   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }

   //start_profiler();

   //Startup the I/O signal handling thread, without INHERIT_SCHED
   if(start_signal_catcher(false) != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }

   sleep(1); //let signal thread get started
   
   //Now Block all I/O signals in this thread.
   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error blicking SIGIO.");
      return -1;
   }
   
   LOG_INFO("starting up . . .");

   //Setup to catch the ping in this high priority thread
   //The low priority signal catcher will block it.
   global_shutter = this;
   
   fcntl(fl.fifo_ch[SHUTTER_AUTO_FIFO_CH].fd_in, F_SETOWN, getpid());
   
   int rv = fcntl(fl.fifo_ch[SHUTTER_AUTO_FIFO_CH].fd_in, F_SETSIG, RTSIGPING);
   if(rv < 0)
   {
      std::cerr << "Error changing signal.\n";
      perror("shuttercontrol");
   }
   
   struct sigaction act;
   sigset_t sset;
   
   act.sa_sigaction = &shutter_auto_handler;
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
 
   init_shutter_open();
   
   sw_state = 1;

   DO_OPENSHUT = 0;

   while(!TimeToDie)
   {
      pthread_mutex_lock(&signal_mutex);
      pthread_cond_wait(&signal_cond, &signal_mutex);
      pthread_mutex_unlock(&signal_mutex);

      if(DO_OPENSHUT == 1) open_shutter();
      if(DO_OPENSHUT == -1) close_shutter();
      DO_OPENSHUT = 0;

      pthread_cond_broadcast(&profile->thcond);
   }

   pthread_join(logger_th, 0);
   pthread_join(signal_thread, 0);

   return 0;
}//int ShutterControlDioclient::start_ShutterControl()

/**/
int ShutterControlDioclient::shutdown_ShutterControl()
{
   do_shutter_open(0);

   return 0;
}

int ShutterControlDioclient::init_shutter_open()
{
   std::string resp;
   
   if(write_fifo_channel(0, "1", 1, &resp) < 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error in init_shutter_open");
      return -2;
   }
      
   if(resp[0] != '1') return -1;
   return 0;
   
}

int ShutterControlDioclient::do_shutter_open(void *adata)
{
   //double startT;
   //char dso[2] = "0";
   struct timespec ts, ts1;
   std::string resp;

   
   if(adata)
   {
      clock_gettime(CLOCK_REALTIME, &ts);
      profile->logseqmsg((char *) adata, "dso1", &ts);
      
      if(write_fifo_channel(0, "1", 1, &resp, (char *) adata) < 0)
      {
         DO_OPENSHUT = 0;
         return -2;
         _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error in do_shutter_open");
      }
      
      clock_gettime(CLOCK_REALTIME, &ts1);
      profile->logseqmsg((char *) adata, "dso2", &ts);
   }
   else
   {
      clock_gettime(CLOCK_REALTIME, &ts);
      if(write_fifo_channel(0, "1", 1, &resp) < 0)
      {
         DO_OPENSHUT = 0;
         return -2;
         _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error in do_shutter_open");
      }
   }

   sim = sis.set_next_image(1, 1);
   sim->depth = 8;
   sim->frameNo = ++cycleNo;
   sim->frame_time.tv_sec = ts.tv_sec;
   sim->frame_time.tv_usec = ts.tv_nsec/1000;
   sim->imdata[0] = 1;
   sis.enable_next_image();
   
   DO_OPENSHUT = 0;
   
   if(resp[0] != '1') return -1;
   return 0;
  
}

int ShutterControlDioclient::do_shutter_close(void *adata)
{
   timespec ts, ts1;	
   std::string resp;

   if(adata)
   {
      clock_gettime(CLOCK_REALTIME, &ts);
      profile->logseqmsg((char *) adata, "dsc1", &ts);
      
      if(write_fifo_channel(0, "0", 1, &resp, (char *) adata) < 0)
      {
         DO_OPENSHUT = 0;
         return -2;
         _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error in do_shutter_close");
      }
      
      clock_gettime(CLOCK_REALTIME, &ts);
      profile->logseqmsg((char *) adata, "dsc2", &ts1);
   }
   else
   {
      clock_gettime(CLOCK_REALTIME, &ts);
      if(write_fifo_channel(0, "0", 1, &resp) < 0)
      {
         DO_OPENSHUT = 0;
         return -2;
         _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error in do_shutter_close");
      }
   }

   sim = sis.set_next_image(1, 1);
   sim->depth = 8;
   sim->frameNo = ++cycleNo;
   sim->frame_time.tv_sec = ts.tv_sec;
   sim->frame_time.tv_usec = ts.tv_nsec/1000;
   sim->imdata[0] = -1;
   sis.enable_next_image();
   
   DO_OPENSHUT = 0;

   if(resp[0] != '0') return -1;
   return 0;
   
}

int ShutterControlDioclient::saveInit()
{
   std::ofstream of;
   
   of.open((std::string(getenv("VISAO_ROOT")) + "/" + init_file).c_str());
   
   of << "cycleNo    uint64      " << cycleNo << "\n";
   
   of.close();
   
   return 0;
}

std::string ShutterControlDioclient::getDataFileName()
{
   std::string fname;
   char buffer[21];
   timeval tv;

   gettimeofday(&tv, 0);
   get_visao_filename(buffer, &tv);
   
   fname = getenv("VISAO_ROOT");
   fname += "/";
   fname += save_path;
   fname += "/shut_";
   fname += buffer;
   fname += ".txt";

   return fname;
}

int ShutterControlDioclient::launchDataLogger()
{
   struct sched_param schedpar;
   schedpar.sched_priority = 0;
   
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   
   pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
   pthread_attr_setschedparam(&attr, &schedpar);

   return pthread_create(&logger_th, &attr, &__launch_data_logger, (void *) this);
}

void ShutterControlDioclient::dataLogger()
{
   std::ofstream of;
   std::string fname;
   double st;
   sharedim<char> currData;
   static int last_image_abs = -1, last_image = 0;

   bool fileOpen = false;
   double fileOpenTime = 0;

   std::string sst;
   char _sst;
   try
   {

   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error blocking SIGIO in data logger thread.");
   }
   block_signal(RTSIGPING);
   
   std::cout << "Data logger started.\n";

   //Wait for first loggable event to happen
   while(!TimeToDie && sis.get_last_image_abs() == -1)
   {
      //TimeToDie-proof sleeping - make sure we don't take more than a second to die.
      st = get_curr_time();
      while(logger_pause - (get_curr_time() - st)  > 1 && !TimeToDie && sis.get_last_image_abs() == -1)
      {
         sleep(1);
      }
      while(logger_pause - (get_curr_time() - st) > 0 && !TimeToDie&& sis.get_last_image_abs() == -1)
      {
         usleep((int)(logger_pause-(get_curr_time()-st))*1000000);
      }
   }

   //last_image_abs = -1;  This is true from declaration.
   
   while(1)
   {
      //Don't check TimeToDie here, always do one last save.
      while(last_image_abs < sis.get_last_image_abs())
      {
         currData = sis.get_image(last_image);

         if(currData.nx)
         {
            if(!fileOpen)
            {
               fname = getDataFileName();
               of.open(fname.c_str());
               if(!of.good())
               {
                  ERROR_REPORT("Error opening shutter data file.  Shutter data may not be logged correctly");
                  break;
               }
               else
               {
                  fileOpen = true;
                  fileOpenTime = get_curr_time();
                  of << "#Cycle # of first entry: " << currData.frameNo << "\n";
               }
            }

            _sst = currData.imdata[0];
            if(_sst < 0) sst = "-1";
            else if(_sst > 0) sst = "1";
            else sst = "0";

            of << currData.frame_time.tv_sec << " " << currData.frame_time.tv_usec << " " << sst << "\n";

            if(!of.good())
            {
               ERROR_REPORT("Error in shutter data file.  Shutter data may not be logged correctly");
            }
            
            last_image_abs++;
            last_image++;
            if(last_image >= sis.get_max_n_images()) last_image = 0;
         }

         
         //Check here, since we will sometimes encounter this while behind
         if(fileOpen && (get_curr_time()-fileOpenTime > data_log_time_length))
         {
            of.close();
            fileOpen = false;
         }
      }

      if(TimeToDie) break;
      
      //TimeToDie-proof sleeping - make sure we don't take more than a second to die.
      st = get_curr_time();
      while(logger_pause - (get_curr_time() - st)  > 1 && !TimeToDie && last_image_abs >= sis.get_last_image_abs())
      {
         sleep(1);
      }
      while(logger_pause - (get_curr_time() - st) > 0 && !TimeToDie && last_image_abs >= sis.get_last_image_abs())
      {
         usleep((int)(logger_pause-(get_curr_time()-st))*1000000);
      }
      
      //Close the file if it has been opened for long enough
      if(fileOpen && (get_curr_time()-fileOpenTime > data_log_time_length))
      {
         of.close();
         fileOpen = false;
      }
   }

   if(fileOpen) of.close();
   std::cout << "Data logging stopped." << std::endl;
   
   }
   catch(...)
   {
      ERROR_REPORT("Exception thrown in data logging thread.  Shutter data no longer being logged");
      throw;
   }
}

int ShutterControlDioclient::get_hw_state()
{
   int hws;
   std::string resp;

   if(!ignore_hw_state)
   {
      if(write_fifo_channel(1, "1", 1, &resp) < 0) 
      {
         _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error in get_hw_state");
         hw_state = 0;
         return 0;
      }
   
      hws = (resp[0] == '1');
      if(hws) hw_state = -1;
      else hw_state = 1;
      return hw_state;
   }
   else return sw_state;
}
      /*
      write_fifo_channel(1, gws, 1);

      startT = get_curr_t();
      while((fl.fifo_ch[1].server_response[0] == '\0') && (get_curr_t() - startT < wait_to))
      {
         //SIGIO is blocked if we are here, so start reading
         read_fifo_channel(&fl.fifo_ch[1]);
      }
      if(fl.fifo_ch[1].server_response[0] == '\0')
      {
         _logger->log(Logger::LOG_LEV_ERROR, "dioserver response timeout in get_hw_state.  wait_to=%f.", wait_to);
         hw_state = 0;
         return 0;
      }
      hws = (fl.fifo_ch[1].server_response[0] == '1');
      if(hws) hw_state = -1;
      else hw_state = 1;

      return hw_state;
   }
   else return sw_state;*/


std::string ShutterControlDioclient::get_state_str()
{
   std::string sstr;
   char schr[50];   
	
	//sstr = control_mode_response();
	//sstr.erase(1,1);
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
   
   snprintf(schr, 50, "%c %i %i %i %i", sstr[0], get_state(), get_sw_state(), get_hw_state(), getPowerStatus());
   //sstr = schr;
   
   return schr;//sstr;
}

std::string ShutterControlDioclient::remote_command(std::string com, char *seqmsg __attribute__((unused)))
{
   std::string rstr;
   
   rstr = common_command(com, CMODE_REMOTE);

   if(rstr == "") return "UNKOWN COMMAND: " + com + "\n";

   seqmsg = 0;//warning suppression
   return rstr;
}

std::string ShutterControlDioclient::local_command(std::string com, char *seqmsg __attribute__((unused)))
{
   std::string rstr;

   //std::cerr << "Shutter Local Command: " << com << std::endl;

   rstr = common_command(com, CMODE_LOCAL);

   if(rstr == "") return "UNKOWN COMMAND: " + com + "\n";

   return rstr;
}

std::string ShutterControlDioclient::script_command(std::string com, char *seqmsg __attribute__((unused)))
{
   std::string rstr;

   rstr = common_command(com, CMODE_SCRIPT);

   if(rstr == "") return "UNKOWN COMMAND: " + com + "\n";

   return rstr;
}


std::string ShutterControlDioclient::auto_command(std::string com, char *seqmsg)
{
   //Before we do anything else, check if 0 or 1
   if(control_mode == CMODE_AUTO && com[0] == '0')
   {
      //glob_seqmsg = seqmsg;
      close_shutter(seqmsg);
      //if(seqmsg[0]) rv = close_shutter((void *) seqmsg);
      //else rv = close_shutter(0);
      //DO_OPENSHUT = -1;
      //pthread_cond_broadcast(&signal_cond);

      //Pause to hold seqmsg
//       if(seqmsg)
//       {
//          while(DO_OPENSHUT == -1) usleep(10);
//       }
      return "0\n";

      //if(rv == -1) return "-1\n";
      //else if (rv == 0) return "0\n";
      //else return "1\n";
   }
   
   if(control_mode == CMODE_AUTO && com[0] == '1')
   {
      //glob_seqmsg = seqmsg;
      open_shutter(seqmsg);
      //if(seqmsg[0]) rv = open_shutter((void *) seqmsg);
      //else rv = open_shutter(0);
      //DO_OPENSHUT = 1;
      //pthread_cond_broadcast(&signal_cond);

      //Pause to hold seqmsg
//       if(seqmsg)
//       {
//          while(DO_OPENSHUT == 1) usleep(10);
//       }
      return "0\n";

      //if(rv == 1) return "1\n";
      //else if (rv == 0) return "0\n";
      //else return "-1\n";
   }

   std::string rstr;
   
   rstr = common_command(com, CMODE_AUTO);
   
   if(rstr != "") return rstr;
   
   return post_auto_command(com, seqmsg);
	
}

std::string ShutterControlDioclient::common_command(std::string com, int calling_cmode)
{
   int rv;
   char rvstr[50];

   int pos = com.find_first_of("\n\r", 0);
   if(pos > -1) com.erase(pos, com.length()-pos);

   if(com == "open" || com == "1")
   {
      if(control_mode == calling_cmode)
      {
         if(!getPowerStatus())
         {
            return("-1\n");
         }
         //rv = open_shutter();
         glob_seqmsg = 0;
         DO_OPENSHUT = 1;
         pthread_cond_broadcast(&signal_cond);
         rv = 0;
         snprintf(rvstr, 5, "%i\n", rv);
         return rvstr;
      }
      else
      {
         return control_mode_response();
      }
   }

   if(com == "close" || com == "-1")
   {
      if(control_mode == calling_cmode)
      {
         if(!getPowerStatus())
         {
            return("-1\n");
         }
         //rv = close_shutter();
         glob_seqmsg = 0;
         DO_OPENSHUT = -1;
         pthread_cond_broadcast(&signal_cond);
         rv = -1;
         snprintf(rvstr, 5, "%i\n", rv);
         return rvstr;
      }
      else
      {
        return control_mode_response();
      }
   }

   if(com == "state?")
   {
      //std::cout << "getting state" << " " << get_state_str().c_str() << std::endl;
      snprintf(rvstr, 50, "%s\n", get_state_str().c_str());
      return rvstr;
   }

   return "";
}

int ShutterControlDioclient::update_statusboard()
{
   static int lastInitSave = 0;
   
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      VisAO::shutter_status_board * ssb = (VisAO::shutter_status_board *) statusboard_shmemptr;

      if(control_mode == CMODE_AUTO)
      {
         ssb->in_auto = 1;
         ssb->sw_state = 0;
         ssb->hw_state = 0;
      }
      else
      {
         ssb->in_auto = 0;
         ssb->sw_state = get_sw_state();
         ssb->hw_state = get_hw_state();
      }

      ssb->sync_enabled = !ignore_hw_state;

      ssb->power_state = getPowerStatus();

   }

   //Regardless, save the init data
   lastInitSave++;
   if(lastInitSave > initSaveInterval)
   {
      
      saveInit();
      lastInitSave = 0;
   }
   
   return 0;
}//int ShutterControlDioclient::update_statusboard()


void * __launch_data_logger(void * Sctrl)
{
   ((ShutterControlDioclient *) Sctrl)->dataLogger();
   return 0;
}


void shutter_auto_handler(int signum, siginfo_t *siginf, void *ucont)
{
   int totbytes;
   std::string resp;
   fifo_channel * fc;

      
   if(siginf->si_code == POLL_IN)
   {
      fc = &global_fifo_list->fifo_ch[SHUTTER_AUTO_FIFO_CH];
      
      totbytes = read_fifo_channel(fc);
      
      resp = global_shutter->auto_command(fc->server_response, fc->seqmsg);
      
      write_fifo_channel(fc, (char *)resp.c_str(), resp.length());
   }
}

} //namespace VisAO

