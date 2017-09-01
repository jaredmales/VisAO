/************************************************************
 *    zaberStageCtrl.cpp
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Definitions for the VisAO gimbal motor controller.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file zaberStageCtrl.h
 * \author Jared R. Males
 * \brief Definitions for the zaber stage controller.
 *
 */

#include "zaberStageCtrl.h"
#define _debug

namespace VisAO
{

zaberStageCtrl::zaberStageCtrl(std::string name, const std::string& conffile) throw (AOException) : zaberStage(), VisAOApp_standalone(name, conffile)
{
   Create();
}

zaberStageCtrl::zaberStageCtrl( int argc, char**argv) throw (AOException): zaberStage(), VisAOApp_standalone(argc, argv)
{
   Create();
}

zaberStageCtrl::~zaberStageCtrl()
{
   return;
}

void zaberStageCtrl::Create()
{

   //Set up the base app
  
   setup_fifo_list(3);
   setup_baseApp(1, 1, 1, 0, false);

   
   
   //Read the scale from config file.
   try
   {
      scale = (double)(ConfigDictionary())["scale"];
   }
   catch(Config_File_Exception)
   {
      scale = 0.000047625;
   }
   _logger->log(Logger::LOG_LEV_INFO, "zaber scale set to %f arcsec/mm.", scale);
   

   //Read the USB Vendor from config file.
   try
   {
      usbVendor = (std::string)(ConfigDictionary())["usbVendor"];
      _logger->log(Logger::LOG_LEV_INFO, "usbVendor set to %s", usbVendor.c_str());
   }
   catch(Config_File_Exception)
   {
      usbVendor = "0403";
      _logger->log(Logger::LOG_LEV_INFO, "usbVendor set to default %s", usbVendor.c_str());
   }

   //Read the USB Product from config file.
   try
   {
      usbProduct = (std::string)(ConfigDictionary())["usbProduct"];
      _logger->log(Logger::LOG_LEV_INFO, "usbProduct set to %s", usbProduct.c_str());
   }
   catch(Config_File_Exception)
   {
      usbProduct = "6001";
      _logger->log(Logger::LOG_LEV_INFO, "usbProduct set to default %s", usbProduct.c_str());
   }   
   
   //Read the Flat Field preset position
   try
   {
      presetFlat = (double)(ConfigDictionary())["presetFlat"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "zaber presetFlat is required in configuration.");
   }
   _logger->log(Logger::LOG_LEV_INFO, "zaber presetFlat set to %f.", presetFlat);
   
   //Read the HWP preset position
   try
   {
      presetHWP = (double)(ConfigDictionary())["presetHWP"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "zaber presetHWP is required in configuration.");
   }
   _logger->log(Logger::LOG_LEV_INFO, "zaber presetHWP set to %f.", presetHWP);
   

   //Init the status board
   statusboard_shmemkey = STATUS_zaber;
   if(create_statusboard(sizeof(zaber_status_board)))
   {
      statusboard_shmemptr = 0;
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create status board.");
   }
   else
   {
      VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;
      strncpy(bsb->appname, MyFullName().c_str(), 25);
      bsb->max_update_interval = pause_time*2.;
   }

   //Read the power outlet from config file.
   try
   {
      power_outlet = (int)(ConfigDictionary())["power_outlet"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "zaber power_outlet is required in configuration.");
   }
   _logger->log(Logger::LOG_LEV_INFO, "zaber power_outlet set to %i.", power_outlet);
   
   //Set up power outlet monitoring
   size_t sz;
   psb = (VisAO::power_status_board*) attach_shm(&sz,  STATUS_power, 0);
   
   if(psb)
   {
      switch(power_outlet)
      {
         case 1: powerOutletState = &psb->outlet1_state; break;
         case 2: powerOutletState = &psb->outlet2_state; break;
         case 3: powerOutletState = &psb->outlet3_state; break;
         case 4: powerOutletState = &psb->outlet4_state; break;
         case 5: powerOutletState = &psb->outlet5_state; break;
         case 6: powerOutletState = &psb->outlet6_state; break;
         case 7: powerOutletState = &psb->outlet7_state; break;
         case 8: powerOutletState = &psb->outlet8_state; break;
         default: powerOutletState = 0;
      }
   }

   isMoving = 0;

   aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);

   //deviceName = "/dev/ttyUSB0";
  
   
   set_euid_called();
   deviceName = ttyUSBdevName(usbVendor.c_str(), usbProduct.c_str());
   set_euid_real();
   
   deviceNumber = 1;
}

int zaberStageCtrl::setupNetwork()
{
   int stat;

   if(getPowerStatus() == 0)
   {
      curState = STATE_OFF;
      return 1;
   }

   stat = connect();
   
   if (stat != 0)
   {
      logss.str("");
      logss << "Error attempting to connect to zaber stage" << ".  Connect result: ";
      logss << stat << " - errno says: " << strerror(errno);
      ERROR_REPORT(logss.str().c_str());
      curState = STATE_NOCONNECTION;

      return -1;
   }
   else
   {
      logss.str("");
      logss << "Connected to ";
      LOG_INFO(logss.str().c_str());
      //curState = STATE_CONNECTED;
   }
   
   return NO_ERROR;
}

int zaberStageCtrl::checkConnStat()
{
   int newState;
   std::string axstate;

   if(getPowerStatus() == 0)
   {
      curState = STATE_OFF;
      return 1;
   }
   
   checkStatus();
   
   if(statusConnected <= 0)
   {
      curState = STATE_NOCONNECTION;
      return 0;
   }

   if(statusHomed <= 0 && statusHoming <= 0)
   {
      newState = STATE_CONNECTED;
   }
   else if(statusHoming == 1)
   {
      newState = STATE_HOMING;
   }
   else if(statusMoving == 1)
   {
      newState = STATE_OPERATING;
   }
   else
   {
      newState = STATE_READY;
   }
   
   

   curState = newState;

   return 0;
   
}

double zaberStageCtrl::getScaledPosition()
{
   checkPosition();
   return statusPosition*scale;
}


int zaberStageCtrl::getPowerStatus()
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
            case 1: powerOutletState = &psb->outlet1_state; break;
            case 2: powerOutletState = &psb->outlet2_state; break;
            case 3: powerOutletState = &psb->outlet3_state; break;
            case 4: powerOutletState = &psb->outlet4_state; break;
            case 5: powerOutletState = &psb->outlet5_state; break;
            case 6: powerOutletState = &psb->outlet6_state; break;
            case 7: powerOutletState = &psb->outlet7_state; break;
            case 8: powerOutletState = &psb->outlet8_state; break;
            default: powerOutletState = 0;
         }
      }
   }
   
   if(psb && powerOutletState)
   {
      if(get_curr_time() - ts_to_curr_time(&psb->last_update) > 3.*psb->max_update_interval)
      {
         powerState = -1;
      }
      else 
      {
         powerState = *powerOutletState;
      }
   }
   else powerState = -1;
   
   return powerState;
}


int zaberStageCtrl::gotoFlat()
{
   return moveAbs( (int) (presetFlat/scale));
}

int zaberStageCtrl::gotoHWP()
{
   return moveAbs( (int) (presetHWP/scale));
}


int zaberStageCtrl::Run()
{
   int sleep_time = 1;
   signal(SIGIO, SIG_IGN);
   signal(RTSIGIO, SIG_IGN);
   
   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }
    
   //Startup the I/O signal handling thread
   if(start_signal_catcher() != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }
   
   //Now Block all I/O signals in this thread.
   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error blicking SIGIO.");
      return -1;
   }
   
   LOG_INFO("starting up . . .");
   
   sleep(1); //let signal thread get started

   curState = STATE_NOCONNECTION;
   
   while(!TimeToDie)
   {

      if(curState == STATE_NOCONNECTION || curState == STATE_OFF)
      {
         setupNetwork();
         if(checkConnStat() == 0) 
         {
            curState = STATE_CONNECTED;
         }
      }
      else if(curState != STATE_OFF)
      {
         if(checkConnStat() < 0) curState = STATE_NOCONNECTION;
      }

      switch(curState)
      {
         case STATE_CONNECTED:
            #ifdef _debug
               std::cout << "STATE_CONNECTED\n";
            #endif
            break;
         case STATE_HOMING:   
            #ifdef _debug
               std::cout << "STATE_HOMING\n";
            #endif
            break;
         case STATE_OPERATING:
            #ifdef _debug
               std::cout << "STATE_OPERATING\n";
            #endif
            break;
         case STATE_READY:
            #ifdef _debug
               std::cout << "STATE_READY\n";
            #endif
            break;
         case STATE_NOCONNECTION:
            #ifdef _debug
               std::cout << "STATE_NOCONNECTION\n";
            #endif
            break;
         case STATE_OFF:
            #ifdef _debug
               std::cout << "STATE_OFF\n";
            #endif
            break;
         default:
            #ifdef _debug
               std::cout << "STATE_?\n";
            #endif
            logss.str("");
            logss << "Bad state in main loop " << curState;
            ERROR_REPORT(logss.str().c_str());
      }

      sleep(sleep_time);
      
   }
   
   pthread_join(signal_thread, 0);
   
   return 0;
      
}

std::string zaberStageCtrl::remote_command(std::string com)
{
   return common_command(com, CMODE_REMOTE);
}

std::string zaberStageCtrl::local_command(std::string com)
{
   return common_command(com, CMODE_LOCAL);
}

std::string zaberStageCtrl::script_command(std::string com)
{
   return common_command(com, CMODE_SCRIPT);
}

std::string zaberStageCtrl::common_command(std::string com, int cmode)
{
   int rv;
   double val;
   char str[256];
   std::string resp;

   #ifdef _debug
      std::cout << "Common Command " << com << "\n";
   #endif

   if(com == "stop")
   {
      rv = stop();
      snprintf(str, 256, "%i\n", rv);
      return str;
   }

   
   if(com == "state?")
   {
      return get_state_str();
   }
   
   if(com == "pos?")
   {
      val = getScaledPosition();
      snprintf(str, 256, "%f\n", val);
      return str;
   }
   
   if(com == "homing?")
   {
      snprintf(str, 256, "%i\n", statusHoming);
      return str;
   }
   
   if(com == "moving?")
   {
      snprintf(str, 256, "%i\n", statusMoving);
      return str;
   }

   if(com == "gotoFlat")
   {
      snprintf(str, 256, "%i\n", gotoFlat());
      return str;
   }
   
   if(com == "gotoHWP")
   {
      snprintf(str, 256, "%i\n", gotoHWP());
      return str;
   }
   
   if(com.length() > 3 && cmode == control_mode)
   {
      if(com.substr(0,3) == "rel" && com.length() > 3)
      {
         val = strtod(com.substr(4, com.length()-4).c_str(), 0);
         int sp = statusPosition; //get fixed value in case another thread changes it
         int npos = sp + (int) (val/scale);
         if(npos < 0) npos = 0;
         if(npos > maxPosition) npos = maxPosition;
         
         rv = moveRel(npos-sp);
         if(rv < 0)
         {
            /** \todo Error handling in zaberStage */
            //ERROR_REPORT(errStr.str().c_str());
         }
         snprintf(str, 256, "%i\n", rv);
         return str;
      }

      if(com.substr(0,3) == "abs" && com.length() > 3)
      {
         val = strtod(com.substr(4, com.length()-4).c_str(), 0);
         
         int npos = ( (int) (val/scale));
         if(npos > maxPosition) npos = maxPosition;
         
         rv = moveAbs(npos);      
         if(rv < 0)
         {
            //ERROR_REPORT(errStr.str().c_str());
         }
         snprintf(str, 256, "%i\n", rv);
         return str;
      }
      if(com.substr(0,4) == "home")
      {
         rv = home();      
         if(rv < 0)
         {
            //ERROR_REPORT(errStr.str().c_str());
         }
         snprintf(str, 256, "%i\n", rv);
         return str;
      }
      
   }
   else
   {
      if(cmode!= control_mode)
      {
         return control_mode_response();
      }
   }
      
   return "UNKOWN COMMAND";
}

std::string zaberStageCtrl::get_state_str()
{
   double pos=-1;
   int h=-1, m=-1;

   if(curState == STATE_HOMING || curState == STATE_OPERATING || curState == STATE_READY)
   {
      checkStatus();
      pos = getScaledPosition();
      h = statusHoming;
      m = statusMoving;
   }
   
   char tmp[100];
   snprintf(tmp, 100, "%c,%i,%i,%i,%i,%f\n", control_mode_response()[0],curState,getPowerStatus(),h, m, pos);
   return tmp;
   return "";
}


   
int zaberStageCtrl::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();

      VisAO::zaber_status_board * zsb = (VisAO::zaber_status_board *) statusboard_shmemptr;

      zsb->state = curState;

      if(curState == STATE_OPERATING || curState == STATE_READY)
      {
         zsb->pos = getScaledPosition();
      }
      else
      {
         zsb->pos = -1;
      }
   }

   return 0;
}
// 
// void zaberStageCtrl::dataLogger(timeval tv)
// {
//    
//    checkDataFileOpen();
// 
//    dataof << tv.tv_sec << " " << tv.tv_usec << " " << get_x_pos() << " " << get_y_pos() << std::endl;
// 
//    if(!dataof.good())
//    {
//       Logger::get()->log(Logger::LOG_LEV_ERROR, "Error in Gimbal data file.  Gimbal data may not be logged correctly");
//    }
// 
// }
   


}//namespace VisAO
