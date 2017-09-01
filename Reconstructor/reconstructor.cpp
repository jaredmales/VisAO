/************************************************************
 *    reconstructor.cpp
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Definitions for a class to perform real-time reconstruction of wavefronts.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file reconstructor.cpp
 * \author Jared R. Males
 * \brief Definitions for a class to perform real-time reconstruction of wavefronts.
 *
 *
 */

#include "reconstructor.h"


namespace VisAO
{

reconstructor * global_reconstructor;

reconstructor::reconstructor(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   Create();
}

reconstructor::reconstructor(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   Create();
}

void reconstructor::Create()
{
   valid_recmat = 0;
   
   std::string pathtmp;
   
   std::string visao_root = getenv("VISAO_ROOT");
   
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

   setup_fifo_list(4);
   setup_baseApp(0, 1, 1, 0, false); //local and script fifos
 
   std::string pingin = ping_fifo_path + "reconstructor39_ping_in";
   std::string pingout = ping_fifo_path + "reconstructor39_ping_out";
   set_fifo_list_channel(&fl, PING_IN_FIFO, RWBUFF_SZ, pingin.c_str(), pingout.c_str(), 0, (void *)this);

   pingin = ping_fifo_path + "frameselector_ping_in";
   pingout = ping_fifo_path + "frameselector_ping_out";
   set_fifo_list_channel(&fl, FS_PING_FIFO, RWBUFF_SZ, pingout.c_str(), pingin.c_str(), 0, (void *)this);
   
   try
   {
      slopes_shmem_key = (int)(ConfigDictionary())["bcu39_shmem_key"];
   }
   catch(Config_File_Exception)
   {
      slopes_shmem_key = DATA_framegrabber39;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the shared memory key for slopes (bcu39_shmem_key): %i", slopes_shmem_key);
    
   try
   {
      strehls_shmem_key = (int)(ConfigDictionary())["strehls_shmem_key"];
   }
   catch(Config_File_Exception)
   {
      strehls_shmem_key = DATA_strehls;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the shared memory key for strehls (strehls_shmem_key): %i", strehls_shmem_key);
   
   try
   {
      num_strehls = (int)(ConfigDictionary())["num_strehls"];
   }
   catch(Config_File_Exception)
   {
      num_strehls = 2500;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the number of strehls in circular buffer to (num_strehls): %i", num_strehls);
   
   pthread_mutex_init(&reconMutex, NULL);

   //Init the status board
   rsb = 0;
   statusboard_shmemkey = STATUS_reconstructor;
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
      rsb = (reconstructor_status_board *) statusboard_shmemptr;
   }

#ifdef REC_USE_GPU
   try
   {
      rmat.rec_tech = (int)(ConfigDictionary())["rec_tech"];
   }
   catch(Config_File_Exception)
   {
      rmat.rec_tech = REC_ATLAS;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the reconstruction technique to (rec_tech): %i", rmat.rec_tech);
#endif

   try
   {
      rmat.tel_diam = (double)(ConfigDictionary())["tel_diam"];
      rmat.median_r0 = (double)(ConfigDictionary())["median_r0"];     
      rmat.median_r0_lam = (double)(ConfigDictionary())["median_r0_lam"];  
      rmat.fitting_A = (double)(ConfigDictionary())["fitting_A"];      
      rmat.fitting_B = (double)(ConfigDictionary())["fitting_B"];
      rmat.reflection_gain = (double)(ConfigDictionary())["reflection_gain"];
   }
   catch(Config_File_Exception)
   {
      rmat.tel_diam = 6.5;
      rmat.median_r0 = 18.;     
      rmat.median_r0_lam = .55;  
      rmat.fitting_A = 0.232555;      
      rmat.fitting_B = -0.840466;
      rmat.reflection_gain = 2.;
   }
   
   
       
   FSPing_enabled = 1;
   
   //Set a default lambda
   set_lambda(765.);

   strehldata = 0;
   wfedata = 0;
   cal_a = 1.0;
   cal_b = 0.0;
   
   
   
   rmsAccum.resize(5,330.);
   
}

int reconstructor::set_matint(std::string fname)
{
   std::string dir = getenv("VISAO_ROOT");
   
   if(fname == "none" || fname == "") return -1;
   
   recFileName = fname;
   
   dir += "/calib/visao/reconstructor/RecMats/";
   dir += fname;
      
   pthread_mutex_lock(&reconMutex);
   
   if(rmat.load_recmat_LBT(dir) == 0)
   {
      valid_recmat = 1;
      
      logss.str("");
      logss << "set recmat: " << dir;
      _logger->log(Logger::LOG_LEV_INFO, logss.str().c_str());
      
      n_modes = rmat.n_modes;
      ho_middle = 200;
      if(n_modes <= 200) ho_middle = 150;
      if(n_modes <= 100) ho_middle = 66;
      
      std::cout << "n_modes = " <<  n_modes << "\n";
      std::cout << "ho_middle = " << ho_middle << "\n";
      pthread_mutex_unlock(&reconMutex);
      return 0;      
   }
   else
   {   
      std::cout << "setting invalid recmat\n";
      valid_recmat = 0;

      logss.str("");
      logss << "failed to set recmat: " << dir;
      _logger->log(Logger::LOG_LEV_ERROR, logss.str().c_str());
      
      n_modes = 0;
      ho_middle = 0;
      
      std::cout << n_modes << "\n";
      std::cout << ho_middle << "\n";
      
      pthread_mutex_unlock(&reconMutex);
      return -1;
   }

}

int reconstructor::set_lambda(float lam)
{
   lambda = lam;

   lamgain = -pow(2.*3.14159/lam,2);

   logss.str("");
   logss << "set lambda: " << lambda;
   _logger->log(Logger::LOG_LEV_INFO, logss.str().c_str());
   
   return 0;
}

int reconstructor::set_filter(std::string fname)
{
   filter_name = fname;

   fir.read_coef_file(fname);

   fir.print_filter();
   if(strehldata == 0) delete[] strehldata;
   strehldata = new float[fir.get_order()];

   if(wfedata == 0) delete[] wfedata;
   wfedata = new float[fir.get_order()];
   
   for(int i=0; i< fir.get_order(); i++)
   {
      wfedata[i] = 0;
      strehldata[i] = 0;
   }

   logss.str("");
   logss << "set filter: " << fname;
   _logger->log(Logger::LOG_LEV_INFO, logss.str().c_str());
   
   return 0;
}

int reconstructor::set_cal_a(float a)
{
   cal_a = a;

   logss.str("");
   logss << "set cal_a: " << cal_a;
   _logger->log(Logger::LOG_LEV_INFO, logss.str().c_str());
   
   return 0;
}

int reconstructor::set_cal_b(float b)
{
   cal_b = b;
   
   logss.str("");
   logss << "set cal_b: " << cal_b;
   _logger->log(Logger::LOG_LEV_INFO, logss.str().c_str());
   
   return 0;
}

int reconstructor::reconstruct()
{
   float sumvar;
   static int last_image = -1;
   int curr_image;
   
   if(pthread_mutex_trylock(&reconMutex) != 0) 
   {
      return -1;
   }
  
   if(!valid_recmat) 
   {
      pthread_mutex_unlock(&reconMutex);
      return -1;
   }
   
   curr_image = slopes_sis.get_last_image();

   if(curr_image == last_image || curr_image < -1) 
   {
      pthread_mutex_unlock(&reconMutex);
      return 0;
   }
   
   slopes_sim = slopes_sis.get_image(curr_image);
   last_image = curr_image;
   
   slopes = (float *) (slopes_sim.imdata+12832);

   rmat.reconstruct(slopes);
     
   //We check here because there could be time differences between when
   //gainSetter updates are received and when Arbitrator updates are received
   //Future: have the recmat be based on gainSet, and guarantee that they are synched.
   if(aosb)
      ho_middle = aosb->homiddle;
   if(ho_middle == 0)
      ho_middle = n_modes;
   
   rmat.calc_sumvar(&sumvar);
   //rmat.calc_sumvar(&tt_wfe, 0, 2, false);
   rmat.calc_sumvar(&ho1_wfe, 2, ho_middle, false);
   rmat.calc_sumvar(&ho2_wfe, ho_middle, -1, false);

   //Calculate RMS values
   std::vector<float> forRms(5);
   forRms[0] = sqrt(sumvar);
   forRms[1] = rmat.amp[0]*rmat.reflection_gain*rmat.unit_conversion;
   forRms[2] = rmat.amp[1]*rmat.reflection_gain*rmat.unit_conversion;;
   forRms[3] = sqrt(ho1_wfe);
   forRms[4] = sqrt(ho2_wfe);
   rmsAccum.addValues(forRms);
   rmsAccum.calcRmss();
   
   rawstrehl = exp(lamgain * sumvar);
   float filtstrehl, filtwfe;
   //Cycle strehldata
   if(fir.get_order() > 0)
   {
      for(int i=0;i<fir.get_order()-1;i++)
      {
         wfedata[i] = wfedata[i+1];
         strehldata[i] = strehldata[i+1];
      }
      wfedata[fir.get_order()-1] = sqrt(sumvar);
      strehldata[fir.get_order()-1] = rawstrehl;

      filtstrehl = fir.apply_filter(strehldata, fir.get_order());
      filtwfe = fir.apply_filter(wfedata, fir.get_order());
      
   }
   else filtstrehl = rawstrehl;
   
   strehl_sim = strehl_sis.set_next_image(REC_TELEM_N, 1);
   strehl_sim->depth = 32;
   strehl_sim->frameNo = slopes_sim.frameNo;
   
   strehl_sim->frame_time = slopes_sim.frame_time;
   
   strehl_sim->imdata[0] = sumvar;
   strehl_sim->imdata[1] = filtwfe;
   strehl_sim->imdata[2] = rmat.amp[0]*rmat.reflection_gain*rmat.unit_conversion;;
   strehl_sim->imdata[3] = rmat.amp[1]*rmat.reflection_gain*rmat.unit_conversion;;
   strehl_sim->imdata[4] = rawstrehl;
   strehl_sim->imdata[5] = rawstrehl*cal_a + cal_b;
   strehl_sim->imdata[6] = filtstrehl;
   strehl_sim->imdata[7] = filtstrehl*cal_a + cal_b;
   
   strehl_sim->imdata[10] = ho1_wfe;
   strehl_sim->imdata[12] = ho2_wfe;
   
   strehl_sim->imdata[14] = rmsAccum.rmss[0];
   strehl_sim->imdata[15] = rmsAccum.rmss[1];
   strehl_sim->imdata[16] = rmsAccum.rmss[2];
   strehl_sim->imdata[17] = rmsAccum.rmss[3];
   strehl_sim->imdata[18] = rmsAccum.rmss[4];
  
   if(rsb)
   {
      if(sumvar > 0)  rsb->inst_wfe = -1;//sqrt(sumvar);
      else rsb->inst_wfe = -1.;
   }
   
   strehl_sis.enable_next_image();

   if(FSPing_enabled) write_fifo_channel(FS_PING_FIFO, "1", 2, 0);
   
   pthread_mutex_unlock(&reconMutex);
   return 0;
   
}//void reconstructor::reconstruct()


int reconstructor::Run()
{

   //set_matint("/home/aosup/visao/calib/visao/reconstructor/RecMats/Rec_20111125_165625.fits");
   set_matint("Rec_20161116_215652.fits");
   
   if(slopes_sis.attach_shm(slopes_shmem_key) < 0)
   {
      ERROR_REPORT("Error attaching to shared memory for slopes.  Waiting and trying again.");
      while(slopes_sis.attach_shm(slopes_shmem_key) < 0)
      {
         sleep(1);
         if(TimeToDie) return -1;
      }
   }
   
   if(strehl_sis.create_shm(strehls_shmem_key, num_strehls, sizeof(sharedim_stack_header) + num_strehls*sizeof(intptr_t) + (num_strehls)*(sizeof(sharedim<float>) + REC_TELEM_N*sizeof(float))) != 0)
   {
      ERROR_REPORT("Error attaching to shared memory for sharedim_stack.");
      return -1;
   }
   
   strehl_sis.header->save_sequence = 0;

   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }
   
   signal(SIGIO, SIG_IGN);
   
   //Startup the I/O signal handling thread
   if(start_signal_catcher() != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }
   sleep(1); 
   //Now Block all I/O signals in this thread.
   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }
   
   LOG_INFO("starting up . . .");

   //Setup to catch the ping in this high priority thread
   //The low priority signal catcher will block it.
   global_reconstructor = this;

   fcntl(fl.fifo_ch[PING_IN_FIFO].fd_in, F_SETOWN, getpid());
   
   int rv = fcntl(fl.fifo_ch[PING_IN_FIFO].fd_in, F_SETSIG, RTSIGPING);
   if(rv < 0)
   {
      std::cerr << "Error changing signal.\n";
      perror("reconstructor");
   }

   struct sigaction act;
   sigset_t sset;

   act.sa_sigaction = &frame_ready;
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
      
   //wait for VisAOI to become available
   
   aosb = 0;
   size_t sz;
   
   while(!TimeToDie && !aosb)
   {
      aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);
      sleep(1);
   }
   
   int last_loop_on = -1;
   
   
   while(!TimeToDie)
   {
      sleep(1);
      read_fifo_channel(&fl.fifo_ch[PING_IN_FIFO]);
      
      if(recFileName != aosb->reconstructor)
      {
         set_matint(aosb->reconstructor);
      } 
   }

   pthread_join(signal_thread, 0);
   
   return 0;
   
}//int reconstructor::Run()

std::string reconstructor::local_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received local command: %s.", com.c_str());
   resp = common_command(com, CMODE_LOCAL);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to local command: %s.", resp.c_str());
   return resp;
}

std::string reconstructor::script_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received local command: %s.", com.c_str());
   resp = common_command(com, CMODE_LOCAL);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to local command: %s.", resp.c_str());
   return resp;
}

std::string reconstructor::common_command(std::string com, int cmode)
{
   char resp[50];
  

   if(com == "recmat?")
   {
      if(rmat.get_filePath() == "") return " ";//empty string is a fifo error
      return recFileName;
   }
   
   if(com == "cal_a?")
   {
      snprintf(resp, 50, "%f\n", cal_a);
      return resp;
   }
   if(com == "cal_b?")
   {
      snprintf(resp, 50, "%f\n", cal_b);
      return resp;
   }

   if(com == "lambda?")
   {
      snprintf(resp, 50, "%f\n", lambda);
      return resp;
   }
   
   if(com == "filter?")
   {
      if(filter_name == "") return " "; //empty string is a fifo error
      return filter_name;
   }
   
   if(com.length() > 5)
   {
      if(com.substr(0, 5) == "cal_a")
      {
         if(cmode == control_mode)
         {
            std::string a = com.substr(5, com.length()-4);
            if(set_cal_a(strtod(a.c_str(),0)) == 0) return "0\n";
            else return "-1\n";
         }
         else
         {
            return control_mode_response();
         }
      }
      if(com.substr(0, 5) == "cal_b")
      {
         if(cmode == control_mode)
         {
            std::string b = com.substr(5, com.length()-4);
            if(set_cal_b(strtod(b.c_str(),0)) == 0) return "0\n";
            else return "-1\n";
         }
         else
         {
            return control_mode_response();
         }
      }
      
   }
   
   if(com.length() > 6)
   {
      if(com.substr(0, 6)== "recmat")
      {
         if(cmode == control_mode)
         {
            std::string newmat = com.substr(6, com.length()-5);
            if(set_matint(newmat) == 0) return "0\n";
            else return "-1\n";
         }
         else
         {
            return control_mode_response();
         }
      }

      if(com.substr(0, 6)== "lambda")
      {
         if(cmode == control_mode)
         {
            std::string newlam = com.substr(6, com.length()-5);
            if(set_lambda(strtod(newlam.c_str(),0)) == 0) return "0\n";
            else return "-1\n";
         }
         else
         {
            return control_mode_response();
         }
      }

      if(com.substr(0, 6)== "filter")
      {
         if(cmode == control_mode)
         {
            std::string newfil = com.substr(7, com.length()-6);
            if(set_filter(newfil) == 0) return "0\n";
            else return "-1\n";
         }
         else
         {
            return control_mode_response();
         }
      }
      
      if(com.substr(0, 6) == "select")
      {
         if(cmode == control_mode)
         {
            int s = atoi(com.substr(6, com.length()-6).c_str());
            
            if(s) FSPing_enabled = 1;
            else FSPing_enabled = 0;
            return "0\n";
         }
         else
         {
            return control_mode_response();
         }
      }
   }
   return "UNKOWN COMMAND?";
}//std::string reconstructor::common_command(std::string com, int cmode)

int reconstructor::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      
      int startn = strehl_sis.get_last_image();
      if(startn < 1) return 0;
     
      
      strehl_simavg = strehl_sis.get_image(startn-1);
      double t0, t1;
      
      t0 = tv_to_curr_time(&strehl_simavg.frame_time);
      t1 = t0;
            
      double avgwfe=0, avgwfe2 = 0;
      double n = 0; 
      double tmpwfe2;
      while(fabs(t0 - t1) < 1.0 )
      {    
         
         tmpwfe2 = strehl_simavg.imdata[0];
         if(tmpwfe2 < 1e8 && !isnan(tmpwfe2))
         {
            avgwfe2 += tmpwfe2;
            avgwfe += sqrt(tmpwfe2);
            n++;
         }
         startn--;
         if(startn < 1) startn = strehl_sis.get_n_images()-1;
         
         
         strehl_simavg = strehl_sis.get_image(startn);
         
         t1 = tv_to_curr_time(&strehl_simavg.frame_time);
         
      }

      avgwfe /= n;
      avgwfe2 /= n;
       
      rsb->avgwfe_1_sec = avgwfe;
      
      if(avgwfe2 >= avgwfe*avgwfe)  rsb->stdwfe_1_sec = sqrt(avgwfe2 - avgwfe*avgwfe);
      else rsb->stdwfe_1_sec = -1;
          
      std::cout << "homiddle: " << ho_middle << "\n";
      
   }
   return 0;
}//int reconstructor::update_statusboard()



void frame_ready(int signum __attribute__((unused)), siginfo_t *siginf, void *ucont __attribute__((unused)))
{
   if(siginf->si_code == POLL_IN && !TimeToDie)
   {
      global_reconstructor->reconstruct();
   }
}

} //namespace VisAO
