/************************************************************
 *    GimbalMotorCtrl.cpp
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Definitions for the VisAO gimbal motor controller.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file GimbalMotorCtrl.h
 * \author Jared R. Males
 * \brief Definitions for the gimbal motor controller.
 *
 */

#include "GimbalMotorCtrl.h"

namespace VisAO
{

GimbalMotorCtrl::GimbalMotorCtrl(std::string name, const std::string& conffile) throw (AOException) : ESPMotorCtrl(2), VisAOApp_standalone(name, conffile)
{
   Create();
}

GimbalMotorCtrl::GimbalMotorCtrl( int argc, char**argv) throw (AOException): ESPMotorCtrl(2), VisAOApp_standalone(argc, argv)
{
   Create();
}

GimbalMotorCtrl::~GimbalMotorCtrl()
{
   return;
}

void GimbalMotorCtrl::Create()
{

   //Set up the base app
  
   setup_fifo_list(3);
   setup_baseApp(1, 1, 1, 0, false);

   //Read the ip address from config file.
   try
   {
      ip_addr = (std::string)(ConfigDictionary())["ip_addr"];
   }
   catch(Config_File_Exception)
   {
      ERROR_REPORT("ip_addr is required in configuration.");
      throw;
   }
   _logger->log(Logger::LOG_LEV_INFO, "ip_addr set to %s.", ip_addr.c_str());

   //Read the ip port from config file.
   try
   {
      ip_port = (int)(ConfigDictionary())["ip_port"];
   }
   catch(Config_File_Exception)
   {
      ERROR_REPORT( "ip_port is required in configuration.");
      throw;
   }
   _logger->log(Logger::LOG_LEV_INFO, "ip_port set to %i.", ip_port);

   //Read the x address from config file.
   try
   {
      x_addr = (int)(ConfigDictionary())["x_addr"];
   }
   catch(Config_File_Exception)
   {
      ERROR_REPORT("x_addr is required in configuration.");
      throw;
   }
   _logger->log(Logger::LOG_LEV_INFO, "x_addr set to %i.", x_addr);

   //Read the y address from config file.
   try
   {
      y_addr = (int)(ConfigDictionary())["y_addr"];
   }
   catch(Config_File_Exception)
   {
      ERROR_REPORT("y_addr is required in configuration.");
      throw;
   }
   _logger->log(Logger::LOG_LEV_INFO, "y_addr set to %i.", y_addr);

   xAxisNo = 1;
   yAxisNo = 2;

   //Read the scale from config file.
   try
   {
      scale = (double)(ConfigDictionary())["scale"];
   }
   catch(Config_File_Exception)
   {
      scale = 1.;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Gimbal scale set to %f arcsec/mm.", scale);
   
   //Read the x center from config file.
   try
   {
      x_center = (double)(ConfigDictionary())["x_center"];
   }
   catch(Config_File_Exception)
   {
      ERROR_REPORT("Gimbal x_center is required in configuration.");
   }
   _logger->log(Logger::LOG_LEV_INFO, "Gimbal x_center set to %f.", x_center);

   //Read the y center from config file.
   try
   {
      y_center = (double)(ConfigDictionary())["y_center"];
   }
   catch(Config_File_Exception)
   {
      ERROR_REPORT("Gimbal y_center is required in configuration.");
   }
   _logger->log(Logger::LOG_LEV_INFO, "Gimbal y_center set to %f.", y_center);


   //Read the x dark location from config file.
   try
   {
      x_dark = (double)(ConfigDictionary())["x_dark"];
   }
   catch(Config_File_Exception)
   {
      x_dark = 0.;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Gimbal x_dark set to %f.", x_dark);

   //Read the y dark location from config file.
   try
   {
      y_dark = (double)(ConfigDictionary())["y_dark"];
   }
   catch(Config_File_Exception)
   {
      y_dark = 0.;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Gimbal y_dark set to %f.", y_dark);


   //Init the status board
   statusboard_shmemkey = STATUS_gimbal;
   if(create_statusboard(sizeof(gimbal_status_board)))
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
      _logger->log(Logger::LOG_LEV_ERROR, "Gimbal power_outlet is required in configuration.");
   }
   _logger->log(Logger::LOG_LEV_INFO, "Gimbal power_outlet set to %i.", power_outlet);
   
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
   postHoming = 0;

   fw2sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel2, 0);
   fw3sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel3, 0);
   wsb = (VisAO::wollaston_status_board*) attach_shm(&sz,  STATUS_wollaston, 0);
   aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);

   pthread_mutex_init(&comMutex, 0);
}

int GimbalMotorCtrl::setupNetwork()
{
   int stat;

   if(getPowerStatus() == 0)
   {
      curState = STATE_OFF;
      return 1;
   }

   pthread_mutex_lock(&comMutex);
   // Close the previous connection, if any
   SerialClose();
   msleep(1000);
   
   stat = SerialInit( ip_addr.c_str(), ip_port );    // This locks if network is down or host unreachable

   pthread_mutex_unlock(&comMutex);
   if (stat != NO_ERROR)
   {
      logss.str("");
      logss << "Error attempting to connect to " << ip_addr << ":" << ip_port << ".  Connect result: ";
      logss << stat << " - errno says: " << strerror(errno);
      ERROR_REPORT(logss.str().c_str());
      curState = STATE_NOCONNECTION;

      return -1;
   }
   else
   {
      logss.str("");
      logss << "Connected to " << ip_addr << ":" << ip_port;
      LOG_INFO(logss.str().c_str());
      //curState = STATE_CONNECTED;
   }
   
   return NO_ERROR;
}

int GimbalMotorCtrl::checkConnStat()
{
   int newState;
   std::string axstate;

   if(getPowerStatus() == 0)
   {
      curState = STATE_OFF;
      return 1;
   }
   
   if(getCtrlState(xAxisNo, axstate) < 0)
   {
      if(curState != STATE_NOCONNECTION) 
      {
         curState = STATE_ERROR; //STATE_NOCONNECTION set by sendCommand.
                                                                 //otherwise we have a different problem.
         ERROR_REPORT(errStr.str().c_str());
         ERROR_REPORT("error getting x axis state");
      }
      return -1;
   }

   if(axstate[0] == '0') newState = STATE_CONNECTED;
   else if (axstate[0] == '1' && (axstate[1] == 'E' || axstate[1] == 'F'))
   {
      newState = STATE_HOMING;
      postHoming = 1;
   }
   else if (axstate[0] == '2') newState = STATE_OPERATING;
   else if (axstate[0] == '3' && isdigit(axstate[1])) newState = STATE_READY;
   else
   {
      logss.str("");
      logss << "Invalid x axis state:" << axstate << ".";
      ERROR_REPORT(logss.str().c_str());
      newState = STATE_INVALID;
   }

   if(getCtrlState(yAxisNo, axstate)<0)
   {
      if(curState != STATE_NOCONNECTION) curState = STATE_ERROR; //STATE_NOCONNECTION set by sendCommand.
                                                                 //otherwise we have a different problem.
      ERROR_REPORT(errStr.str().c_str());
      ERROR_REPORT("error getting y axis state");
      return -1;
   }

   if(axstate[0] == '0')
   {
      newState = STATE_CONNECTED;
   }
   else if ((axstate[0] == '1' && (axstate[1] == 'E' || axstate[1] == 'F')))
   {
      if(newState != STATE_INVALID)
      {
         newState = STATE_HOMING;
         postHoming = 1;
      }
   }
   else if (axstate[0] == '2')
   {
      if(newState != STATE_HOMING && newState != STATE_INVALID)
      {
         newState = STATE_OPERATING;
      }
   }
   else if (axstate[0] == '3' && isdigit(axstate[1]))
   {
      if(newState != STATE_HOMING && newState != STATE_INVALID && newState != STATE_OPERATING)
      {
         newState = STATE_READY;
      }
   }
   else
   {
      ERROR_REPORT("Invalid y axis state");
      newState = STATE_INVALID;
   }

   if(newState == STATE_READY)
   {
      if(isMoving) moveStop();
      
      if(postHoming)
      {
         int rv;
         std::string presetf;
         std::vector<double> preset(2);
         
         if( get_preset("gimbal", 0, -1, 0, 0, &preset, presetf) < 0)
         {
            logss.str("");
            logss << "Found preset: " << presetf << " -> " << preset[0] << " " << preset[1] << ".  Centering.";
            LOG_INFO(logss.str().c_str());
            
            ERROR_REPORT("Could not get center preset for post homing.");
            preset[0] = 0.;
            preset[1] = 0.;
         }
   
         
         
         rv = gotoAbsPos(xAxisNo, preset[0]);
         if(rv < 0)
         {
            ERROR_REPORT(errStr.str().c_str());
         }
         rv += gotoAbsPos(yAxisNo, preset[1]);
         if(rv < 0)
         {
            ERROR_REPORT(errStr.str().c_str());
         }
         postHoming = 0;
         curState = STATE_OPERATING;
      }
   }

   curState = newState;

   return 0;
   
}

double GimbalMotorCtrl::get_x_pos()
{
   return getCurPos(xAxisNo)*scale;
}

int GimbalMotorCtrl::getXMoving()
{
   std::string resp;
   getCtrlState(xAxisNo, resp);
   if(resp.length() != 2) return -1;
   //std::cout << s << " " << resp << "\n";

   if(resp[0] == '2' && resp[1] == '8')
   {
      return 1;
   }
   
   if(resp[0] == '1' && (resp[1] == 'E' || resp[1] == 'F'))
   {
      return 1;
   }
   
   return 0;
}

double GimbalMotorCtrl::get_y_pos()
{
   return getCurPos(yAxisNo)*scale;
}

int GimbalMotorCtrl::getYMoving()
{
   std::string resp;
   
   getCtrlState(yAxisNo, resp);
   if(resp.length() != 2) return -1;
   
   if(resp[0] == '2' && resp[1] == '8')
   {
      return 1;
   }
   
   if(resp[0] == '1' && (resp[1] == 'E' || resp[1] == 'F'))
   {
      return 1;
   }
   
   return 0;
}

int GimbalMotorCtrl::getPowerStatus()
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

int GimbalMotorCtrl::center()
{
   std::string presetf;
   std::vector<double> preset(2);
   
   size_t sz;
   
   if(!fw2sb) fw2sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel2, 0);
   if(!fw3sb) fw3sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel3, 0);
   if(!wsb) wsb = (VisAO::wollaston_status_board*) attach_shm(&sz,  STATUS_wollaston, 0);
   if(!aosb) aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);
   
   if(!aosb || !wsb || !fw2sb || !fw3sb)
   {
      ERROR_REPORT("Not connected to status boards for center preset.");
      return -1;
   }
   
   //Check if there is a specific preset for this combo.  If not, then use the BS only.
   if( get_preset("gimbal", aosb->filter1_reqpos, (int) wsb->cur_pos, fw2sb->req_pos, fw3sb->req_pos, &preset, presetf) < 0)
   {
      if( get_preset("gimbal", aosb->filter1_reqpos, (int) wsb->cur_pos, 0, 0, &preset, presetf) < 0)
      {
         ERROR_REPORT("Could not get exact center preset.");
         return -1;
      }
   }
   
   logss.str("");
   logss << "Found preset: " << presetf << " -> " << preset[0] << " " << preset[1] << ".  Centering.";
   LOG_INFO(logss.str().c_str());
   
   if(gotoAbsPos(xAxisNo, preset[0]/scale) < 0)
   {
      ERROR_REPORT(errStr.str().c_str());
      return -1;
   }
   if(gotoAbsPos(yAxisNo, preset[1]/scale) < 0)
   {
      ERROR_REPORT(errStr.str().c_str());
      return -1;
   }
   
   return 0;
}


int GimbalMotorCtrl::dark()
{

   
   if(gotoAbsPos(xAxisNo, x_dark/scale) < 0)
   {
      ERROR_REPORT(errStr.str().c_str());
      return -1;
   }
   if(gotoAbsPos(yAxisNo, y_dark/scale) < 0)
   {
      ERROR_REPORT(errStr.str().c_str());
      return -1;
   }

   logss.str("");
   logss << "moved to dark position " << x_dark << " " << y_dark << ".";
   LOG_INFO(logss.str().c_str());

   return 0;
}

int GimbalMotorCtrl::savepreset()
{
   std::vector<double> preset(2);
   
   size_t sz;
   
   if(!fw2sb) fw2sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel2, 0);
   if(!fw3sb) fw3sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel3, 0);
   if(!wsb) wsb = (VisAO::wollaston_status_board*) attach_shm(&sz,  STATUS_wollaston, 0);
   if(!aosb) aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);

   if(!aosb || !wsb || !fw2sb || !fw3sb)
   {
      ERROR_REPORT("Not connected to status boards for saving preset.");
      return -1;
   }

   double fw3pos;

   if(fw3sb->type == 2)
   {
      fw3pos = fw3sb->pos;
   }
   else
   {
      fw3pos = 0.;
   }

   preset[0] = get_x_pos();
   preset[1] = get_y_pos();

/*   //We save presets for each combo of fw2 and fw3, since these should have only small effect on center location.
   for(int i =0;i<5;i++)
   {
      for(int j=0;j<6;j++)
      {*/
         if( save_preset("gimbal", aosb->filter1_pos, (int) wsb->cur_pos, 0., fw3pos, &preset) < 0)
         {
            logss.str("");
            logss << "Could not save center preset: " << (int) aosb->filter1_pos << " " << (int) wsb->cur_pos << " " << 0 << " " << fw3sb << " " << preset[0] << " " << preset[1] << "\n";
            ERROR_REPORT(logss.str().c_str());
            return -1;
         }
/*      }
   }*/
   

   logss.str("");
   logss << "Saved preset " << preset[0] << " " << preset[1] << ".";
   LOG_INFO(logss.str().c_str());

   return 0;
}

int GimbalMotorCtrl::sendCommand(std::string &com, std::string &resp, int timeout)
{
   pthread_mutex_lock(&comMutex);

   char answer[256];
   //char term[3] = "\r\n";
   
   #ifdef _debug
      std::cout << "Sending " << com << "\n";
   #endif

   std::string termcom = com + "\r\n";
   
   if(SerialOut(termcom.c_str(), termcom.length()) != 0)
   {
      ERROR_REPORT("Communication send error in sendCommand");
      curState = STATE_NOCONNECTION;
      pthread_mutex_unlock(&comMutex);
      return -1;
   }

   #ifdef _debug
      std::cout << "Sent.  Getting response.\n";
   #endif

   if(SerialInString(answer, 256, timeout, '\n') < 0)
   {
      resp = "";
      ERROR_REPORT("Communication receive error in sendCommand");
      curState = STATE_NOCONNECTION;
      pthread_mutex_unlock(&comMutex);
      return -1;
   }

   #ifdef _debug
      std::cout << "Response got.\n";
   #endif

   pthread_mutex_unlock(&comMutex);

   resp = answer;

   int cr = resp.find("\r", 0);

   #ifdef _debug
      std::cout << "CR pos = " << cr << ".\n";
   #endif

   if(cr > 0) resp.erase(cr, resp.length()-cr);


   #ifdef _debug
      std::cout << "Received " << resp << "\n";
   #endif
   
   return 0;
}

int GimbalMotorCtrl::sendCommand(std::string &com)
{
   pthread_mutex_lock(&comMutex);
   #ifdef _debug
      std::cout << "Sending " << com << "\n";
   #endif

   std::string termcom = com + "\r\n";
   
   if(SerialOut(termcom.c_str(), termcom.length()) != 0)
   {
      ERROR_REPORT("Communication send error in sendCommand");
      curState = STATE_NOCONNECTION;
      pthread_mutex_unlock(&comMutex);
      return -1;
   }

   pthread_mutex_unlock(&comMutex);
   return 0;
}

int GimbalMotorCtrl::Run()
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
            if(powerOnInit() < 0)
            {
               ERROR_REPORT("Error in power on init:");
               ERROR_REPORT(errStr.str().c_str());
               curState = STATE_ERROR;
            }
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

std::string GimbalMotorCtrl::remote_command(std::string com)
{
   return common_command(com, CMODE_REMOTE);
}

std::string GimbalMotorCtrl::local_command(std::string com)
{
   return common_command(com, CMODE_LOCAL);
}

std::string GimbalMotorCtrl::script_command(std::string com)
{
   return common_command(com, CMODE_SCRIPT);
}

std::string GimbalMotorCtrl::common_command(std::string com, int cmode)
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

   if(com == "stopx")
   {
      rv = stop(xAxisNo);
      snprintf(str, 256, "%i\n", rv);
      return str;
   }

   if(com == "stopy")
   {
      rv = stop(yAxisNo);
      snprintf(str, 256, "%i\n", rv);
      return str;
   }
   
   if(com == "state?")
   {
      return get_state_str();
   }

   if(com == "scale?")
   {
      snprintf(str, 256, "%f\n", scale);
      return str;
   }
   
   if(com == "xpos?")
   {
      val = get_x_pos()*scale;
      snprintf(str, 256, "%f\n", val);
      return str;
   }

   if(com == "xmoving?")
   {
      snprintf(str, 256, "%i\n", getXMoving());
      return str;
   }

   if(com == "ypos?")
   {
      val = get_y_pos()*scale;
      snprintf(str, 256, "%f\n", val);
      return str;
   }

   if(com == "ymoving?")
   {
      snprintf(str, 256, "%i\n", getYMoving());
      return str;
   }

   if(com.length() > 3 && cmode == control_mode)
   {
      if(com.substr(0,4) == "xrel" && com.length() > 4)
      {
         val = strtod(com.substr(5, com.length()-5).c_str(), 0);
         rv = gotoRelPos(xAxisNo, val/scale);
         if(rv < 0)
         {
            ERROR_REPORT(errStr.str().c_str());
         }
         snprintf(str, 256, "%i\n", rv);
         return str;
      }

      if(com.substr(0,4) == "xabs" && com.length() > 4)
      {
         val = strtod(com.substr(5, com.length()-5).c_str(), 0);
         rv = gotoAbsPos(xAxisNo, val/scale);      
         if(rv < 0)
         {
            ERROR_REPORT(errStr.str().c_str());
         }
         snprintf(str, 256, "%i\n", rv);
         return str;
      }

      if(com.substr(0,4) == "yrel" && com.length() > 4)
      {
         val = strtod(com.substr(5, com.length()-5).c_str(), 0);
         rv = gotoRelPos(yAxisNo, val/scale);
         if(rv < 0)
         {
            ERROR_REPORT(errStr.str().c_str());
         }
         snprintf(str, 256, "%i\n", rv);
         return str;
      }
      
      if(com.substr(0,4) == "yabs" && com.length() > 4)
      {
         val = strtod(com.substr(5, com.length()-5).c_str(), 0);
         rv = gotoAbsPos(yAxisNo, val/scale);
         if(rv < 0)
         {
            ERROR_REPORT(errStr.str().c_str());
         }
         snprintf(str, 256, "%i\n", rv);
         return str;
      }

      if(com == "center")
      {
         rv = center();

         if(rv < 0) return "-1\n";
         return "1\n";
      }

      if(com == "dark")
      {
         rv = dark();

         if(rv < 0) return "-1\n";
         return "1\n";
      }

      if(com == "savepreset")
      {
         rv = savepreset();
         if(rv < 0) return "-1\n";
         return "1\n";
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

std::string GimbalMotorCtrl::get_state_str()
{
   double xp=-1, yp=-1;
   int xm=-1, ym=-1;

   if(curState == STATE_HOMING || curState == STATE_OPERATING || curState == STATE_READY)
   {
      xp = get_x_pos();
      yp = get_y_pos();
      xm = getXMoving();
      ym = getYMoving();
   }
   
   char tmp[100];
   snprintf(tmp, 100, "%c,%i,%i,%i,%f,%i,%f,%f\n", control_mode_response()[0],curState,getPowerStatus(),xm, xp, ym, yp, scale);
   return tmp;
}

void GimbalMotorCtrl::moveStart()
{
   isMoving = 1;
   timeval tv;
   gettimeofday(&tv, 0);
   dataLogger(tv);
}

void GimbalMotorCtrl::moveStop()
{
   isMoving = 0;
   timeval tv;
   gettimeofday(&tv, 0);
   dataLogger(tv);
}

   
int GimbalMotorCtrl::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();

      VisAO::gimbal_status_board * gsb = (VisAO::gimbal_status_board *) statusboard_shmemptr;

      gsb->power = getPowerStatus();

      if(curState == STATE_CONNECTED || curState == STATE_HOMING || curState == STATE_OPERATING || curState == STATE_READY)
      {
         gsb->xpos = get_x_pos();

         gsb->ypos = get_y_pos();
      }
      else
      {
         gsb->xpos = -1;
         gsb->ypos = -1;
      }
   }

   return 0;
}

void GimbalMotorCtrl::dataLogger(timeval tv)
{
   
   checkDataFileOpen();

   dataof << tv.tv_sec << " " << tv.tv_usec << " " << get_x_pos() << " " << get_y_pos() << std::endl;

   if(!dataof.good())
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "Error in Gimbal data file.  Gimbal data may not be logged correctly");
   }

}
   


}//namespace VisAO
