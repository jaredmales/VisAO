/************************************************************
*    FocusMotorCtrl.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for the VisAO focus stage stepper motor controller.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file FocusMotorCtrl.cpp
  * \author Jared R. Males
  * \brief Definitions for the focus stage stepper motor controller.
  *
*/

#include "FocusMotorCtrl.h"

namespace VisAO
{

FocusMotorCtrl::FocusMotorCtrl( std::string name, const std::string& conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   initapp();
}

FocusMotorCtrl::FocusMotorCtrl( int argc, char**argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   //std::cout << "constructed\n";
   initapp();
}



void FocusMotorCtrl::initapp()
{
   //std::cout << "initing\n";

   std::string pathtmp;
   std::string visao_root = getenv("VISAO_ROOT");
   
   setup_fifo_list(10);
   setup_baseApp(1, 1, 1, 0, false);
   
   //Set up the dioserver channel numbers
   try
   {
      dioch_enable = (int)(ConfigDictionary())["dioch_enable"];
      _logger->log(Logger::LOG_LEV_INFO, "dioserver channel for setting enable (dioch_enable) set to %i.", dioch_enable);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "dioserver channel for setting enable (dioch_enable) not set in config file.");
      throw;
   }
   try
   {
      dioch_dir = (int)(ConfigDictionary())["dioch_dir"];
      _logger->log(Logger::LOG_LEV_INFO, "dioserver channel for setting direction (dioch_dir) set to %i.", dioch_dir);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "dioserver channel for setting direction (dioch_dir) not set in config file.");
      throw;
   }
   try
   {
      dioch_step = (int)(ConfigDictionary())["dioch_step"];
      _logger->log(Logger::LOG_LEV_INFO, "dioserver channel for stepping (dioch_step) set to %i.", dioch_step);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "dioserver channel for stepping (dioch_step) not set in config file.");
      throw;
   }
   
   try
   {
      dioch_pwr = (int)(ConfigDictionary())["dioch_pwr"];
      _logger->log(Logger::LOG_LEV_INFO, "dioserver channel for controller power (dioch_pwr) set to %i.", dioch_pwr);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "dioserver channel for controller power (dioch_pwr) not set in config file.");
      throw;
   }
   
   try
   {
      dioch_limpos = (int)(ConfigDictionary())["dioch_limpos"];
      _logger->log(Logger::LOG_LEV_INFO, "dioserver channel for positive limit switch (dioch_limpos) set to %i.", dioch_limpos);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "dioserver channel for positive limit switch (dioch_limpos) not set in config file.");
      throw;
   }

   try
   {
      dioch_limhome = (int)(ConfigDictionary())["dioch_limhome"];
      _logger->log(Logger::LOG_LEV_INFO, "dioserver channel for home limit switch (dioch_limhome) set to %i.", dioch_limhome);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "dioserver channel for home limit switch (dioch_limhome) not set in config file.");
      throw;
   }

   try
   {
      dioch_limneg = (int)(ConfigDictionary())["dioch_limneg"];
      _logger->log(Logger::LOG_LEV_INFO, "dioserver channel for negative limit switch (dioch_limneg) set to %i.", dioch_limneg);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "dioserver channel for negative limit switch (dioch_limneg) not set in config file.");
      throw;
   }


   //Set up the dio server paths
   char fin[MAX_FNAME_SZ], fout[MAX_FNAME_SZ];
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["diofifo_path"];
   }
   catch(Config_File_Exception)
   {
      pathtmp = "fifos";
   }
   diofifo_path = visao_root + "/" + pathtmp + "/diofifo"; 
   _logger->log(Logger::LOG_LEV_INFO, "Set diofifo_path: %s", diofifo_path.c_str());

   //Trying these without handlers
   /**/
   get_dio_fnames(fin, fout, (char *)diofifo_path.c_str(), dioch_enable);
   set_fifo_list_channel(&fl, FIFOCH_ENABLE, 100,fin, fout, 0, 0);

   get_dio_fnames(fin, fout, (char *)diofifo_path.c_str(), dioch_dir);
   set_fifo_list_channel(&fl, FIFOCH_DIR, 100,fin, fout, 0, 0);

   get_dio_fnames(fin, fout, (char *)diofifo_path.c_str(), dioch_step);
   set_fifo_list_channel(&fl, FIFOCH_STEP, 100,fin, fout, 0, 0);

   get_dio_fnames(fin, fout, (char *)diofifo_path.c_str(), dioch_pwr);
   set_fifo_list_channel(&fl, FIFOCH_PWR, 100,fin, fout, 0, 0);

   get_dio_fnames(fin, fout, (char *)diofifo_path.c_str(), dioch_limneg);
   set_fifo_list_channel(&fl, FIFOCH_NEGL, 100,fin, fout, 0, 0);

   get_dio_fnames(fin, fout, (char *)diofifo_path.c_str(), dioch_limhome);
   set_fifo_list_channel(&fl, FIFOCH_HOMESW, 100,fin, fout, 0, 0);

   get_dio_fnames(fin, fout, (char *)diofifo_path.c_str(), dioch_limpos);
   set_fifo_list_channel(&fl, FIFOCH_POSL, 100,fin, fout, 0, 0);
   /**/

   signal(SIGIO, SIG_IGN);

   try
   {
      step_ratio = (double)(ConfigDictionary())["step_ratio"];
      _logger->log(Logger::LOG_LEV_INFO, "step ratio (step_ratio) set to %0.4f microns per step.", step_ratio);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "step ratio (step_ratio) not set in config file.");
      throw;
   }    
   
   try
   {
      min_step_time = (double)(ConfigDictionary())["min_step_time"];
      if(min_step_time < 1e-6) min_step_time = 1e-6;
      _logger->log(Logger::LOG_LEV_INFO, "minimum step time (min_step_time) set to %0.3g seconds.", min_step_time);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "minimum step time (min_step_time) not set in config file.");
      throw;
   }    
   try
   {
      hw_dir = (int)(ConfigDictionary())["hw_dir"];
      if(hw_dir > 0) hw_dir = 1;
      else hw_dir = 0;
      _logger->log(Logger::LOG_LEV_INFO, "hardware direction (hw_dir) set to %i.", hw_dir);
   }
   catch(Config_File_Exception)
   {
      hw_dir = 0;
      _logger->log(Logger::LOG_LEV_INFO, "hardware direction (hw_dir) set to default %i.", hw_dir);
   }    
   
   try
   {
      sw_limits_only = (ConfigDictionary())["sw_limits_only"];
   }
   catch(Config_File_Exception)
   {
      sw_limits_only = 0;
   }  
   _logger->log(Logger::LOG_LEV_INFO, "software limits only (sw_limits_only) set to %i.", sw_limits_only);

   #ifdef VISAO_SIMDEV
      sw_limits_only = 1;
   #endif
   
   try
   {
      sw_neg_limit = (ConfigDictionary())["sw_neg_limit"];
      _logger->log(Logger::LOG_LEV_INFO, "software negative limit (sw_neg_limit) set to %f.", sw_neg_limit);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "Missing software negative limit (sw_neg_limit) in config file .");
      throw;
   }  
   
   try
   {
      sw_pos_limit = (ConfigDictionary())["sw_pos_limit"];
      _logger->log(Logger::LOG_LEV_INFO, "software positive limit (sw_pos_limit) set to %f.", sw_pos_limit);
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "Missing software positive limit (sw_pos_limit) in config file .");
      throw;
   }


   //Init the status board
   statusboard_shmemkey = 6000;
   if(create_statusboard(sizeof(focusstage_status_board)) != 0)
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


   //Connect to other status boards for presets.
   size_t sz;

   fw2sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel2, 0);
   fw3sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel3, 0);
   wsb = (VisAO::wollaston_status_board*) attach_shm(&sz,  STATUS_wollaston, 0);
   aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);
    

   pthread_mutex_init(&dio_mutex, 0);

   std::string resp;
   
   pending_move = 0;
   next_pos = 0;
   
   is_moving = false;
   stop_moving = false;
   
   gettimeofday(&last_step_time, 0);
   
   if(init_vars)
   {
      try
      {
         cur_pos = (*init_vars)["cur_pos"];
      }
      catch(Config_File_Exception)
      {
         cur_pos = 0;
      }
   }
   
   homing = 0;
   pos_limit_disabled = false;
   
   
   try
   {
      home_pos = (double)(ConfigDictionary())["home_pos"];
      
      _logger->log(Logger::LOG_LEV_INFO, "home position (home_pos) set to %f.", home_pos);
   }
   catch(Config_File_Exception)
   {
      home_pos = 0.;
      _logger->log(Logger::LOG_LEV_INFO, "home position (home_pos) set to %f.", home_pos);
   }
   
   
}//void FocusMotorCtrl::initapp()

FocusMotorCtrl::~FocusMotorCtrl()
{
   save_init();
}

int FocusMotorCtrl::check_limits()
{
   std::string resp;
   //int use_sw_limits = 0;
   //Get limits here
   //Check if 1 is forward, 0 is backward!!!!!!!!!!!!!!!!!

   if(!sw_limits_only)
   {
      pthread_mutex_lock(&dio_mutex);

      resp ="";
      if(write_fifo_channel(FIFOCH_NEGL, "1", 2, &resp) < 0)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error getting front limit ");
         pthread_mutex_unlock(&dio_mutex);
         neg_limit = 1;
         return -1;
      }

      neg_limit = (resp[0] == '0');

      resp = "";
      if(write_fifo_channel(FIFOCH_HOMESW, "1", 2, &resp) < 0)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error getting home switch");
         pthread_mutex_unlock(&dio_mutex);
         home_switch = 1;
         return -1;
      }

      home_switch = (resp[0] == '0');

      resp = "";
      if(write_fifo_channel(FIFOCH_POSL, "1", 2, &resp) < 0)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error in getting back limit");
         pthread_mutex_unlock(&dio_mutex);
         pos_limit = 1;
         return -1;
      }
      pthread_mutex_unlock(&dio_mutex);
      pos_limit = (resp[0] == '0');
   }

   if(cur_pos <= sw_neg_limit)
   {
      neg_limit = true;
   }
   else
   {
      if(sw_limits_only) neg_limit = false;
   }


   if(cur_pos >= sw_pos_limit)
   {
      pos_limit = true;
   }
   else
   {
      if(sw_limits_only) pos_limit = false;
   }
   
   if(sw_limits_only)
   {
      if(cur_pos > home_pos - 2. && cur_pos < home_pos + 2.) home_switch = 1;
      else home_switch = 0;
   }
  
   //Always stop at neg limit
   //unless homing, then turn around
   if(!cur_dir && neg_limit)
   {
      if(homing && homing !=4 ) //If we're homing, but not neg homing
      {
         return set_direction(1); //homing the wrong way, turn arround.
      }

      homing = 0;
      stop_moving = true;
      return 0;
   }
   
   //Moving positive and at pos limit
   if(cur_dir && pos_limit && !pos_limit_disabled)
   {
      if(homing && homing!=5) //If we're homing, but not pos homing
      {
         return set_direction(0);//homing the wrong way, turn around
      }

      homing = 0;
      stop_moving = true;
      return 0;
   }

   //If instead the back limit is disabled and we're homing backwards for any reason, then stop.
   if(pos_limit_disabled && homing && !cur_dir)
   {
      stop_moving = true;

      homing = 0;
   }

   //O.K. to keep moving.
   if(!homing) return 0;

   //Initial homing
   if(homing == 1)
   {
      if(home_switch)
      {
         homing = 3; //Go directly to tertiary homing
         return set_direction(0); //move backward, continue.
      }
      else 
      {
         homing = 2;
         return 0;
      }
   }

   //Main homing, moving forward, and at home switch
   if(homing == 2 && cur_dir && home_switch)
   {
      homing = 0;
      //Do any home state processing here.  set_home()?
      stop_moving = true;
      return 0;
   }
   
   //Main homing, moving backward, and at home switch
   if(homing == 2 && !cur_dir && home_switch)
   {
      homing = 3;  //keeping going back, but enter tertiary homing
      return 0;
   }

   //Secondary homing, moving backward, and home switch clear
   if(homing == 3 && !cur_dir && !home_switch)
   {
      homing = 2; //re-enter secondary homing.
      return set_direction(1); //and now move forward
   }   

   //if we're here, then keep homing
   return 0;
}
   
   
int FocusMotorCtrl::save_init()
{
   std::ofstream of;
   
   of.open((std::string(getenv("VISAO_ROOT")) + "/" + init_file).c_str());
   
   of.precision(10);
   of << "cur_pos    float32      " << cur_pos << "\n";
   
   of.close();
   return 0;
}

int FocusMotorCtrl::delete_init()
{
   remove((std::string(getenv("VISAO_ROOT")) + "/" + init_file).c_str());
   
   return 0;
}

int FocusMotorCtrl::set_cur_pos(double cp)
{
        cur_pos = cp;
        return 0;
}

int FocusMotorCtrl::get_power_state()
{
   std::string resp;

   pthread_mutex_lock(&dio_mutex);
   if(write_fifo_channel(FIFOCH_PWR, "1", 2, &resp) < 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "dioserver response error in get_power_state");
      power_state = 0;
      pthread_mutex_unlock(&dio_mutex);
      return -1;
   }
   pthread_mutex_unlock(&dio_mutex);

#ifdef VISAO_SIMDEV
   power_state = 1;
   return 0;
#endif

   power_state = (resp[0] == '1');


   return 0;
}

int FocusMotorCtrl::set_home()
{
   cur_pos = 0;
   return 0;
}
                

int FocusMotorCtrl::set_enable(bool en)
{
   std::string resp;
   
   pthread_mutex_lock(&dio_mutex);
        if(en) 
   {
      if(write_fifo_channel(FIFOCH_ENABLE, "0", 1, &resp) < 0)
      {
         logss.str("");
         logss << "Error writing to fifo channel " << FIFOCH_ENABLE << ".  Attempting to set enable.";
         log_msg(Logger::LOG_LEV_ERROR, logss.str());
         std::cerr << logss.str() << " (logged) \n";
      }
   }
        else 
   {
      if(write_fifo_channel(FIFOCH_ENABLE, "1", 1, &resp) < 0)
      {
         logss.str("");
         logss << "Error writing to fifo channel " << FIFOCH_ENABLE << ".  Attempting to set disable.";
         log_msg(Logger::LOG_LEV_ERROR, logss.str());
         std::cerr << logss.str() << " (logged) \n";
      }
   }
        pthread_mutex_unlock(&dio_mutex);
        cur_enabled = en;
        return 0;
}

int FocusMotorCtrl::enable()
{
        return set_enable(true);
}

int FocusMotorCtrl::disable()
{
        return set_enable(false);
}

int FocusMotorCtrl::set_direction(bool dir)
{
   std::string resp;
   pthread_mutex_lock(&dio_mutex);
   if(hw_dir)
   {
      if(dir)
      {
         if(write_fifo_channel(FIFOCH_DIR, "1", 1, &resp) < 0)
         {
            logss.str("");
            logss << "Error writing to fifo channel " << FIFOCH_DIR << ".  Attempting to set direction high.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            std::cerr << logss.str() << " (logged) \n";
         }
      }
      else
      {
         if(write_fifo_channel(FIFOCH_DIR, "0", 1, &resp) < 0)
         {
            logss.str("");
            logss << "Error writing to fifo channel " << FIFOCH_DIR << ".  Attempting to set direction low.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            std::cerr << logss.str() << " (logged) \n";
         }
      }
   }
   else
   {
      if(dir)
      {
         if(write_fifo_channel(FIFOCH_DIR, "0", 1, &resp) < 0)
         {
            logss.str("");
            logss << "Error writing to fifo channel " << FIFOCH_DIR << ".  Attempting to set direction low.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            std::cerr << logss.str() << " (logged) \n";
         }
      }
      else
      {
         if(write_fifo_channel(FIFOCH_DIR, "1", 1, &resp) < 0)
         {
            logss.str("");
            logss << "Error writing to fifo channel " << FIFOCH_DIR << ".  Attempting to set direction high.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            std::cerr << logss.str() << " (logged) \n";
         }
      }
   }
   pthread_mutex_unlock(&dio_mutex);
   cur_dir = dir;
   return 0;
}

int FocusMotorCtrl::set_forward()
{
   return set_direction(true);
}

int FocusMotorCtrl::set_backward()
{
   return set_direction(false);
}
                
int FocusMotorCtrl::step(int nsteps)
{
   int i;
   timeval currtime, tv;
   double stepdt;
   double avstepdt = 0;
   std::string resp;
 
   #ifdef _debug
      std::cout << "In Step\nnsteps = " << nsteps << "\nhoming = " << homing << "\n";
      std::cout << "stop_moving = " << stop_moving << "\n" << "cur_dir = " << cur_dir << "\n\n";
   #endif
      
   if(!homing)  _logger->log(Logger::LOG_LEV_TRACE, "Starting move of %i steps.", nsteps);
   else _logger->log(Logger::LOG_LEV_TRACE, "Starting homing of type %i.", homing);
   
   gettimeofday(&tv,0);
   dataLogger(tv);
   
   enable();
   
   delete_init();
   
   is_moving = true;
   
   //actually step here.
   //with wait times
   i = 0;
   if(check_limits() != 0)
   {
      logss.str("");
      logss << "Error checking limit switch status. Stopping move.";
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //std::cerr << logss.str() << " (logged)\n";
      stop_moving = true;
   }
   
   if(get_power_state() != 0)
   {
      logss.str("");
      logss << "Error checking power status. Stopping move.";
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //std::cerr << logss.str() << " (logged)\n";
      stop_moving = true;
   }

#ifdef _debug
std::cout << "Beginning loop\nnsteps = " << nsteps << "\nhoming = " << homing << "\n";
std::cout << "stop_moving = " << stop_moving << "\n" << "cur_dir = " << cur_dir << "\n\n";
#endif

   while((i < nsteps || homing) && !stop_moving && !TimeToDie && power_state)
   {
      gettimeofday(&currtime,0);
      stepdt = ((double)currtime.tv_sec+((double)currtime.tv_usec)/1e6) - ((double)last_step_time.tv_sec+((double)last_step_time.tv_usec)/1e6);

      while(stepdt < min_step_time) //Need to recheck in case the sleep is interrupted by a signal
      {
         gettimeofday(&currtime,0);
         stepdt = ((double)currtime.tv_sec+((double)currtime.tv_usec)/1e6) - ((double)last_step_time.tv_sec+((double)last_step_time.tv_usec)/1e6);
      }
      if(!stop_moving  && !TimeToDie && power_state) //Check in case it was set during the min_step_time wait.
      {
         pthread_mutex_lock(&dio_mutex);
         if(write_fifo_channel(FIFOCH_STEP, "1", 1, &resp) < 0)
         {
            logss.str("");
            logss << "Error writing to fifo channel " << FIFOCH_STEP << ".  Attempting to set step high.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            //std::cerr << logss.str() << " (logged) \n";
         }
         pthread_mutex_unlock(&dio_mutex);
         
         if(resp[0] != '1')
         {
            logss.str("");
            logss << "Step failed on setting step to high.  Response from dioserver: " << resp << "  Position not updated.  Stopping move.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            //std::cerr << logss.str() << " (logged)\n";
            break;
         }
         if(i > 0) avstepdt += stepdt;
         set_cur_pos(cur_pos + (-1 + 2*cur_dir) * step_ratio);
         usleep(1); //Minimum high time

         pthread_mutex_lock(&dio_mutex);
         if(write_fifo_channel(FIFOCH_STEP, "0", 1, &resp) < 0)
         {
            logss.str("");
            logss << "Error writing to fifo channel " << FIFOCH_STEP << ".  Attempting to set step low.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            //std::cerr << logss.str() << " (logged) \n";
         }
         pthread_mutex_unlock(&dio_mutex);

         gettimeofday(&last_step_time,0);
         if(resp[0] != '0')
         {
            logss.str("");
            logss << "Step failed (bad return from dioserver) on setting step to low. Stopping move.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            //std::cerr << logss.str() << " (logged)\n";
            break;
         }
         
         if (check_limits() != 0)
         {
            logss.str("");
            logss << "Error checking limit switch status. Stopping move.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            //std::cerr << logss.str() << " (logged)\n";
            break;
         }
         
         if(get_power_state() != 0)
         {
            logss.str("");
            logss << "Error checking power status. Stopping move.";
            log_msg(Logger::LOG_LEV_ERROR, logss.str());
            //std::cerr << logss.str() << " (logged)\n";
            stop_moving = true;
         }
         
         update_statusboard();
         i++;
      }
   }
   
   gettimeofday(&tv,0);
   dataLogger(tv);
   
   double itmp;
   if(i > 0 ) itmp = i;
   else itmp = 1e34; 
   logss.str("");
   logss << "Completed move. " << i << " steps, ave time: " << avstepdt/itmp << " secs/step";
   //std::cerr << logss.str() << "\n";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   
   //std::cerr << "Setting low (cleanup)\n";
   //Cleanup and prepare for next move.
   pthread_mutex_lock(&dio_mutex);
   if(write_fifo_channel(FIFOCH_STEP, "0", 1, &resp) < 0) //Unecessary and redundant, but just to make sure we can't leave with step high
   {                                                      // due an asynchronous event.  Would cause missing steps.
      logss.str("");
      logss << "Error writing to fifo channel " << FIFOCH_STEP << ".  Attempting to set direction low during move cleanup.";
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //std::cerr << logss.str() << " (logged)\n";
   }
   pthread_mutex_unlock(&dio_mutex);
   //need to check steps
   is_moving = false;
   stop_moving = false;
   homing = 0;
   disable();
   pending_move = 0;
   
   save_init();
   #ifdef _debug
   std::cout << "Leaving by the back\nnsteps = " << nsteps << "\nhoming = " << homing << "\n";
   std::cout << "stop_moving = " << stop_moving << "\n" << "cur_dir = " << cur_dir << "\n\n";
   #endif
   _logger->log(Logger::LOG_LEV_TRACE, "Completed %i steps.", i);
   
   return 0;
}//step(int)
   
int FocusMotorCtrl::step()
{
        return step(1);
}

int FocusMotorCtrl::step_forward(int nsteps)
{
        set_forward();
        stop_moving = 0; //Check limits may have set this if at a limit switch moving the other way
        return step(nsteps);
}

int FocusMotorCtrl::step_forward()
{
        return step_forward(1);
}
                
int FocusMotorCtrl::step_backward(int nsteps)
{
        set_backward();
        stop_moving = 0; //Check limits may have set this if at a limit switch moving the other way
        return step(nsteps);
}

int FocusMotorCtrl::step_backward()
{
        return step_backward(1);
}

int FocusMotorCtrl::offset_pos(double dpos)
{
   double dsteps = dpos / step_ratio;
   next_pos = cur_pos + dpos;
   if(dsteps > 0)
   {
      return step_forward((int)(dsteps+.5));
   }
   if(dsteps < 0)
   {
      return step_backward((int)(-1*dsteps-.5));
   }
   is_moving = false;
   pending_move = MOVE_NONE;
   return 0;
}

int FocusMotorCtrl::goto_pos(double pos)
{
   double dpos = pos - cur_pos;
   return offset_pos(dpos);
}

int FocusMotorCtrl::start_pending_move()
{
   if(pending_move == MOVE_OFFSET) return offset_pos(next_pos);
   if(pending_move == MOVE_ABS) return goto_pos(next_pos);
   if(pending_move == MOVE_HOME)
   {
      if(cur_pos < home_pos) set_forward();
      else set_backward();
      stop_moving = 0;
      homing = 1;
      return step(0);
   }
   if(pending_move == MOVE_NEGHOME)
   {
      set_backward();
      stop_moving = 0;
      homing = 4;
      return step(0);
   }
   if(pending_move == MOVE_POSHOME)
   {
      set_forward();
      stop_moving = 0;
      homing = 5;
      return step(0);
   }

   return 0;
}

int FocusMotorCtrl::get_preset(double &fcal, std::vector<double> & preset, std::string & presetf)
{
   size_t sz;
   
   if(!fw2sb) fw2sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel2, 0);
   if(!fw3sb) fw3sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel3, 0);
   if(!wsb) wsb = (VisAO::wollaston_status_board*) attach_shm(&sz,  STATUS_wollaston, 0);
   if(!aosb) aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);
   
   if(!aosb || !wsb || !fw2sb || !fw3sb)
   {
      logss.str("");
      logss << "Not connected to status boards for preset.";
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //ERROR_REPORT("Not connected to status boards for preset.");
      return -1;
   }
   
   if( get_focuscal(&fcal) < 0)
   {
      logss.str("");
      logss << "Could not get focus cal.";
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //ERROR_REPORT(logss.str().c_str());
      return -1;
   }
   
   
   //std::string presetf;
   if(::get_preset("focus", aosb->filter1_reqpos, (int) wsb->cur_pos, fw2sb->req_pos, fw3sb->req_pos, &preset, presetf) < 0)
   {
      logss.str("");
      logss << "Could not get preset: " << presetf << ".";
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //ERROR_REPORT(logss.str().c_str());
      return -1;
   }

   return 0;
}

int FocusMotorCtrl::check_preset()
{
   double fcal;
   std::vector<double> preset(1);
   std::string presetf;
         
   if(get_preset(fcal, preset, presetf) != 0)
   {
      return 0;
   }
   
   if(fabs(fcal+preset[0] - cur_pos) < 2.) return 1;
 
   
   return 0;
}   
   
int FocusMotorCtrl::goto_preset()
{
   double fcal;
   std::vector<double> preset(1);
   std::string presetf;
   
   if(get_preset(fcal, preset, presetf) != 0)
   {
      return -1;
   }
   
   logss.str("");
   logss << "Found focus cal: " << fcal;
   LOG_INFO(logss.str().c_str());
   
   
   logss.str("");
   logss << "Found preset: " << presetf << " -> " << preset[0] << ".";
   LOG_INFO(logss.str().c_str());

   logss.str("");
   logss << "Presetting to: " << fcal + preset[0];
   LOG_INFO(logss.str().c_str());
   
   pending_move = MOVE_ABS;
   next_pos = fcal + preset[0];

   return 0;
}


int FocusMotorCtrl::Run()
{
   std::string resp;
   std::cout.precision(20);
   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }

   //Startup the I/O signal handling thread
   if(start_signal_catcher(true) != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }

   //Now Block all I/O signals in this thread.
   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error blocking SIGIO in main thread.");
      return -1;
   }

   LOG_INFO("starting up . . .");

   sleep(1); //wait for signal thread to get going.

   pthread_mutex_lock(&dio_mutex);
   if(write_fifo_channel(FIFOCH_STEP, "0", 1, &resp) < 0) //Set the step to low just in case.
   {
      logss.str("");
      logss << "Error writing to fifo channel " << FIFOCH_STEP << ".  Attempting to set step to low.";
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //std::cerr << logss.str() << " (logged) \n";
   }
   pthread_mutex_unlock(&dio_mutex);

   get_power_state();
   disable(); 
   set_direction(1);
   check_limits(); //Just get the true position.

   double last_update = get_curr_time();

   while(!TimeToDie)
   {
      pthread_mutex_lock(&signal_mutex); 
      pthread_cond_wait(&signal_cond, &signal_mutex);
      pthread_mutex_unlock(&signal_mutex);

      if(pending_move) start_pending_move();

      else if(DO_ENABLE != 0)
      {
         if(DO_ENABLE > 0) enable();
         if(DO_ENABLE < 0) disable();
         DO_ENABLE = 0;
      }

      if(get_curr_time() - last_update > pause_time)
      {
         check_limits();
         get_power_state();
         last_update = get_curr_time();
      }
   }

   pthread_join(signal_thread, 0);

   return 0;

}


std::string  FocusMotorCtrl::remote_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received remote command: %s.", com.c_str());
   resp = common_command(com, CMODE_REMOTE);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to remote command: %s.", resp.c_str());
   return resp;
}

std::string  FocusMotorCtrl::local_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received local command: %s.", com.c_str());
   resp = common_command(com, CMODE_LOCAL);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to local command: %s.", resp.c_str());
   return resp;
}

std::string  FocusMotorCtrl::script_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received script command: %s.", com.c_str());
   resp = common_command(com, CMODE_SCRIPT);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to script command: %s.", resp.c_str());
   return resp;
}

std::string FocusMotorCtrl::common_command(std::string com, int cmode)
{
   char rstr[50];
   double npos;


   if(com == "pos?")
   {
      snprintf(rstr, 50, "%0.4f\n", get_cur_pos());
      return rstr;
   }
   
   if(com == "ismoving?")
   {
      if(is_moving) return "1\n";
      else return "0\n";
   }
   
   if(com == "state?")
   {
      return get_state_str();
   }
   
   if(com == "enable?")
   {
      if(cur_enabled) return "1\n";
      else return "0\n";
   }
   
   if(com == "power?")
   {
      if(power_state) return "1\n";
      else return "0\n";
   }
   
   if(com == "abort")
   {
      //Everybody can always abort.
      stop_moving = true;
      pthread_kill(main_thread, SIG_MAINTHREAD);
      return "0\n";
   }

  
   //int pst = com.find("preset", 0);
   if(com == "preset")
   {
      if(control_mode == cmode)
      {
         if(is_moving || pending_move) return "-1\n";
         
         goto_preset();
         return "0\n";
      }
      else return control_mode_response();
   }
         
   if(com.substr(0,3) == "pos")
   {
      if(control_mode == cmode)
      {
         if(is_moving || pending_move) return "0\n";
      
         npos = strtod(com.substr(3,com.length()-3).c_str(),0);
         pending_move = MOVE_ABS;
         next_pos = npos;
         return "1\n";
      }
      else return control_mode_response();
   }
   
   if(com.substr(0,4) == "dpos")
   {
      if(control_mode == cmode)
      {
         if(is_moving || pending_move) return "0\n";
      
      npos = strtod(com.substr(4,com.length()-4).c_str(),0);
      pending_move = MOVE_OFFSET;
      next_pos = npos;
      return "1\n";
      }
      else return control_mode_response();
   }
   
   if(com == "home")
   {
      if(control_mode == cmode)
      {
         return "-1\n";
      }
      else return control_mode_response();
   }
   
   if(com == "homeneg")
   {
      if(control_mode == cmode)
      {
         if(is_moving || pending_move) return "0\n";

         pending_move = MOVE_NEGHOME;
         return "1\n";
      }
      else return control_mode_response();
   }
   
   if(com == "homepos")
   {
      if(control_mode == cmode)
      {
         if(is_moving || pending_move) return "0\n";

         pending_move = MOVE_POSHOME;
         return "1\n";
      }
      else return control_mode_response();
   }
   
   if(com == "enable")
   {
      if(control_mode == cmode)
      {
         if(is_moving || pending_move) return "0\n";
      
      DO_ENABLE = 1;
      return "1\n";
      }
      else return control_mode_response();
   }
   
   if(com == "disable")
   {
      if(control_mode == cmode)
      {
         if(is_moving || pending_move) return "0\n";
      
      DO_ENABLE = -1;
      return "1\n";
      }
      else return control_mode_response();
   }
   
   if(com == "disable_pos_limit")
   {
      if(control_mode == CMODE_LOCAL)
      {
         pos_limit_disabled = true;
         return "1\n";
      }
      else return control_mode_response();
   }

   /*re enabling the back limit requires a restart
    *  if(com == "enable_back_limit")
    *  {
    *     back_limit_disabled = false;
    *     return "1\n";
    * 
    }*/

   return (std::string("UNKOWN COMMAND: ") + com + "\n");
}

std::string FocusMotorCtrl::get_state_str()
{
        char statestr[100];
        std::string str = control_mode_response();

        snprintf(statestr, 100, "%c %012.4f %i %i %i %i %i %i %i %012.4f %i\n", str[0], cur_pos, cur_enabled, power_state, is_moving, homing, neg_limit, home_switch, pos_limit, next_pos-cur_pos, pos_limit_disabled);

        return statestr;
}


int FocusMotorCtrl::update_statusboard()
{

   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();

      VisAO::focusstage_status_board * fsb = (VisAO::focusstage_status_board *) statusboard_shmemptr;

      fsb->cur_pos = cur_pos;
      fsb->homing = homing;
      fsb->is_moving = is_moving;
      fsb->cur_dir = cur_dir;
      fsb->cur_enabled = cur_enabled;
      fsb->power_state = power_state;
      fsb->is_focused = check_preset();
   }
   return 0;
}

void FocusMotorCtrl::dataLogger(timeval tv)
{
   checkDataFileOpen();

   dataof << tv.tv_sec << " " << tv.tv_usec << " " << cur_pos << std::endl;

   if(!dataof.good())
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "Error in Focus motor data file.  Focus data may not be logged correctly");
   }

}

} //namespace VisAO
