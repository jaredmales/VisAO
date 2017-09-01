/************************************************************
 *    HWPRotatorCtrl.cpp
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Definitions for the VisAO gimbal motor controller.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file HWPRotatorCtrl.h
 * \author Jared R. Males
 * \brief Definitions for the zaber stage controller.
 *
 */

#include "HWPRotatorCtrl.h"
#define _debug

namespace VisAO
{

HWPRotatorCtrl::HWPRotatorCtrl(std::string name, const std::string& conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   Create();
}

HWPRotatorCtrl::HWPRotatorCtrl( int argc, char**argv) throw (AOException): VisAOApp_standalone(argc, argv)
{
   Create();
}

HWPRotatorCtrl::~HWPRotatorCtrl()
{
   return;
}

void HWPRotatorCtrl::Create()
{

   //Set up the base app
  
   setup_fifo_list(3);
   setup_baseApp(1, 1, 1, 0, false);

   
   
   //Read the USB Vendor from config file.
   try
   {
      usbVendor = (std::string)(ConfigDictionary())["usbVendor"];
       _logger->log(Logger::LOG_LEV_INFO, "usbVendor set to %s", usbVendor.c_str());
   }
   catch(Config_File_Exception)
   {
      usbVendor = "1a72";
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
      usbProduct = "1014";
      _logger->log(Logger::LOG_LEV_INFO, "usbProduct set to default %s", usbProduct.c_str());
   }   

   
   
   //Init the status board
   statusboard_shmemkey = STATUS_hwp;
   if(create_statusboard(sizeof(hwp_status_board)))
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
      _logger->log(Logger::LOG_LEV_ERROR, "HWP power_outlet is required in configuration.");
   }
   _logger->log(Logger::LOG_LEV_INFO, "HWP power_outlet set to %i.", power_outlet);
   
   
   
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


   aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);

   pthread_mutex_init(&comMutex, 0);
   
}

int HWPRotatorCtrl::stageSend(const std::string & msg)
{
   int rv;
   pthread_mutex_lock(&comMutex);
   
   rv = fdWrite(msg.c_str(), msg.length(), fileDescrip, 500);
   
   pthread_mutex_unlock(&comMutex);
   
   return rv;
}
   
int HWPRotatorCtrl::stageSend(std::string & resp, const std::string & msg)
{
   
   int rv;
   char res[1024];
   
   pthread_mutex_lock(&comMutex);
   
   rv = fdWriteRead(res, 1024, msg.c_str(), msg.length(), fileDescrip, 500, 500);
   
   resp = res;
   pthread_mutex_unlock(&comMutex);
   
   
   return rv;
}


   
   
int HWPRotatorCtrl::openDevice()
{
   std::cerr << usbVendor.c_str() << " " << usbProduct.c_str() << "\n";
   
   set_euid_called();
   deviceName = ttyUSBdevName(usbVendor.c_str(), usbProduct.c_str());
   set_euid_real();
   
   _logger->log(Logger::LOG_LEV_INFO, "Serial tty deviceName set to %s", deviceName.c_str());
   
   
   
   set_euid_called();

   fileDescrip = open( deviceName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

   
   if(fileDescrip == -1)
   {
      std::cerr << "Failed to open\n";
      return -1;
   }
   
   struct termios termopt;
   tcgetattr(fileDescrip, &termopt);
   
   cfsetispeed(&termopt, B115200);
   cfsetospeed(&termopt, B115200);
   
  // stty -F /dev/ttyUSB0 speed 115200 cs8 -cstopb -parenb -crtscts -echo -echoe raw
   
   //termopt.c_flag |= (CLOCAL | CREAD);
   termopt.c_cflag |= CS8;
   termopt.c_cflag &= ~CSTOPB;
   termopt.c_cflag &= ~PARENB;

   
   termopt.c_lflag &= ~ECHO;
   termopt.c_lflag &= ~ECHOE;
   cfmakeraw(&termopt);
   
   tcsetattr(fileDescrip, TCSANOW, &termopt);
   set_euid_real();
   
   char idn[] = "*idn?\n";
   char res[1024];
   
   int rv = fdWriteRead(res, 1024, idn, strlen(idn), fileDescrip, 500, 500);
   
   if(rv < 0)
   {
      logss.str("");
      logss << "Error opening " << deviceName;
      LOG_INFO(logss.str().c_str());
      return -1;
   }
   
   if(rv == 0)
   {
      logss.str("");
      logss << "No response received from " << deviceName;
      LOG_INFO(logss.str().c_str());
      return -1;
   }
   
   if(strlen(res) != 77)
   {
      logss.str("");
      logss << "Stage id incorrect length";
      LOG_INFO(logss.str().c_str());
      rv = -1;
   }
   
   logss.str("");
   logss << "Stage identified itself as: " << res;
   LOG_INFO(logss.str().c_str());
  
   return rv;
   
}

int HWPRotatorCtrl::Connect()
{
   int stat;

   if(getPowerStatus() == 0)
   {
      curState = STATE_OFF;
      return 1;
   }

   stat = openDevice();
   
   if (stat <= 0)
   {
      logss.str("");
      logss << "Error attempting to connect to HWP stage at " << deviceName << ".  Connect result: ";
      logss << stat << " - errno says: " << strerror(errno);
      ERROR_REPORT(logss.str().c_str());
      curState = STATE_NOCONNECTION;

      return -1;
   }
   else
   {
      logss.str("");
      logss << "Connected to " << deviceName;
      LOG_INFO(logss.str().c_str());
      curState = STATE_CONNECTED;
   }
   
   return NO_ERROR;
}

int HWPRotatorCtrl::setupStage()
{
   char res[1024];
   int rv;
   
   //First reference the stage
   //char ron[] = "RON 1 1\n";
      
   rv = stageSend("RON 1 1\n"); //fdWrite(ron, strlen(ron), fileDescrip, 500);
  
   if( rv < 0)
   {
      logss.str("");
      logss << "Error attempting to reference stage. ";
      ERROR_REPORT(logss.str().c_str());
      curState = STATE_ERROR;
      
      return -1;
   }
   
   //Next set the servo

   //char svo[] = "SVO 1 1\n";
      
   rv = stageSend("SVO 1 1\n");//fdWrite(svo, strlen(svo), fileDescrip, 500);
   
   if( rv < 0)
   {
      logss.str("");
      logss << "Error attempting to set servo. ";
      ERROR_REPORT(logss.str().c_str());
      curState = STATE_ERROR;
      
      return -1;
   }
   
   //And do a fast reference
   //char frf[] = "FRF 1\n";
      
   rv = home();
   
   if( rv < 0)
   {
      return rv;
   }
   
   logss.str("");
   logss << "Stage is setup and homing";
   LOG_INFO(logss.str().c_str());
   
   return 0;
}

int HWPRotatorCtrl::getPowerStatus()
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


int HWPRotatorCtrl::getMoving()
{
   int rv;
   std::string movStr;
   
   rv = stageSend(movStr, "\005\n");
   
   if(rv < 0) return rv;
   
   if( movStr[0] == '0') rv = 0;
   if( movStr[0] == '1') rv = 1;
  
   return rv;
   
}

double HWPRotatorCtrl::getPosition()
{
   int rv;
   std::string posStr;
   
   rv = stageSend(posStr, "POS?\n");
   
   std::cerr << "Pos return: " << posStr << "\n";
   
   return strtod( posStr.c_str()+2, 0);
   
   
}


int HWPRotatorCtrl::getCurState()
{
  

   //Check power status
   if(getPowerStatus() == 0)
   {
      curState = STATE_OFF;
      return 1;
   }
   else
   {
      //If power is on but STATE_OFF is set, then power has been turned on!
      if(curState == STATE_OFF) curState = STATE_NOCONNECTION;
   }
   
   if(curState == STATE_NOCONNECTION)
   {
      if( Connect() != NO_ERROR) return -1;
   }
   
   if(curState == STATE_CONNECTED)
   {
      setupStage();
   }

   if(curState == STATE_ERROR)
   {
      //Check the error
      curState = STATE_BUSY;
   }
   
   if(curState == STATE_HOMING)
   {
      int rv = getMoving();
      if(getMoving() == 0)
      {
         curState = STATE_READY;
      }
   }
   
   if(curState == STATE_READY)
   {
      if(getMoving() == 1)
      {
         curState = STATE_OPERATING;
      }
   }
   
   if(curState == STATE_OPERATING || curState == STATE_BUSY)
   {
      if(getMoving() == 0)
      {
         curState = STATE_READY;
      }
   }
   

   return curState;
//    
}

int HWPRotatorCtrl::home()
{
   int rv;
   
   rv = stageSend("FRF 1\n");//fdWrite(frf, strlen(frf), fileDescrip, 500);
   
   if( rv < 0)
   {
      logss.str("");
      logss << "Error attempting fast reference home. ";
      ERROR_REPORT(logss.str().c_str());
      curState = STATE_ERROR;
      
      return -1;
   }
   
   curState = STATE_HOMING;
   
   return 0;
   
}

   
int HWPRotatorCtrl::move(double npos)
{
   int rv;
   char posStr[128];
   
   snprintf(posStr, 128, "MOV 1 %f\n", npos);
   
   rv = stageSend(posStr);//fdWrite(frf, strlen(frf), fileDescrip, 500);
   
   if( rv < 0)
   {
      logss.str("");
      logss << "Error attempting to move stage. ";
      ERROR_REPORT(logss.str().c_str());
      curState = STATE_ERROR;
      
      return -1;
   }
   
   curState = STATE_OPERATING;
   
   return 0;
}
   
int HWPRotatorCtrl::stop()
{
   int rv;
   
   rv = stageSend("STP\n");//fdWrite(frf, strlen(frf), fileDescrip, 500);
   
   if( rv < 0)
   {
      logss.str("");
      logss << "Error attempting to stop. ";
      ERROR_REPORT(logss.str().c_str());
      curState = STATE_ERROR;
      
      return -1;
   }
   
   return 0;

}

int HWPRotatorCtrl::contRot()
{
   int rv;
   
   rv = home();
      
   if(rv <0) return rv;
   
   int i=0;
   while(getMoving() && i < 1000) 
   {
      msleep(100);
      ++i;
   }
   
   if(getMoving())
   {
      logss.str("");
      logss << "Error waiting for end of home to start continuous rotation. ";
      ERROR_REPORT(logss.str().c_str());
      curState = STATE_ERROR;
      
      return -1;
   }
    

   rv =  move(2000000);
   curState = STATE_BUSY;
   
}

int HWPRotatorCtrl::minus45()
{
   return move( getPosition() - 45.0);
}

int HWPRotatorCtrl::minus225()
{
   return move( getPosition() - 22.5);
}
   
int HWPRotatorCtrl::plus225()
{
   return move( getPosition() + 22.5);
}
   
int HWPRotatorCtrl::plus45()
{
   return move( getPosition() + 45.0);
}
   
int HWPRotatorCtrl::Run()
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
   
   //Connect();
   
   while(!TimeToDie)
   {

      getCurState();
      
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
         case STATE_BUSY:
         #ifdef _debug
               std::cout << "STATE_BUSY\n";
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
         case STATE_ERROR:
         #ifdef _debug
               std::cout << "STATE_ERROR\n";
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

      std::cout << getPosition() << "\n";
      sleep(sleep_time);
      
   }
   
   pthread_join(signal_thread, 0);
   
   return 0;
      
}

std::string HWPRotatorCtrl::remote_command(std::string com)
{
   return common_command(com, CMODE_REMOTE);
}

std::string HWPRotatorCtrl::local_command(std::string com)
{
   return common_command(com, CMODE_LOCAL);
}

std::string HWPRotatorCtrl::script_command(std::string com)
{
   return common_command(com, CMODE_SCRIPT);
}

std::string HWPRotatorCtrl::common_command(std::string com, int cmode)
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
// 
//    
   if(com == "state?")
   {
      return get_state_str();
   }
//    
   if(com == "pos?")
   {
      val = getPosition();
      snprintf(str, 256, "%f\n", val);
      return str;
   }
   
   if(com == "moving?")
   {
      snprintf(str, 256, "%i\n", getMoving());
      return str;
   }

   if(com == "home")
   {
      snprintf(str, 256, "%i\n", home());
      return str;
   }
   
   if(com == "controt")
   {
      snprintf(str, 256, "%i\n", contRot());
      return str;
   }
      
   if(com == "m45")
   {
      snprintf(str, 256, "%i\n", minus45());
      return str;
   }
   
   if(com == "m225")
   {
      snprintf(str, 256, "%i\n", minus225());
      return str;
   }
   
   if(com == "p225")
   {
      snprintf(str, 256, "%i\n", plus225());
      return str;
   }
   
   if(com == "p45")
   {
      snprintf(str, 256, "%i\n", plus45());
      return str;
   }
   
   if(com.length() > 3 && cmode == control_mode)
   {
      if(com.substr(0,3) == "pos" && com.length() > 3)
      {
         val = strtod(com.substr(4, com.length()-4).c_str(), 0);
         
         rv = move(val);
         if(rv < 0)
         {
            /** \todo Error handling in zaberStage */
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

std::string HWPRotatorCtrl::get_state_str()
{
   double pos=-1;
   int h=-1, m=-1;

   getCurState();
   
   if(curState == STATE_READY)
   {
      h = 0;
      m = 0;
   }
   
   if(curState == STATE_HOMING)
   {
      h = 1;
      m = 1;
   }
   
   if(curState == STATE_OPERATING)
   {
      h = 0;
      m = 1;
   }
   
   pos = getPosition();
   
   char tmp[100];
   snprintf(tmp, 100, "%c,%i,%i,%i,%i,%f\n", control_mode_response()[0],curState,getPowerStatus(),h, m, pos);
   return tmp;
}


   
int HWPRotatorCtrl::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();

      VisAO::hwp_status_board * hsb = (VisAO::hwp_status_board *) statusboard_shmemptr;

      hsb->state = curState;

      if(curState == STATE_OPERATING || curState == STATE_READY || curState == STATE_BUSY)
      {
         hsb->pos = getPosition();
      }
      else
      {
         hsb->pos = -1;
      }
   }

   return 0;
}
// 
// void HWPRotatorCtrl::dataLogger(timeval tv)
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
