/************************************************************
*    ShutterRemoteControl.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Shutter control from the AO Supervisor.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterRemoteControl.cpp
  * \author Jared R. Males
  * \brief Definitions for shutter control from the AO Supervisor.  
  * 
*/

#include "ShutterRemoteControl.h"


using namespace Arcetri;

// Mutex to lock out other threads during complex operations
pthread_mutex_t threadMutex;

namespace VisAO
{

ShutterRemoteControl::ShutterRemoteControl( std::string name, const std::string &conffile) throw (AOException) : AOApp(name, conffile)
{
    Create();
}

ShutterRemoteControl::ShutterRemoteControl( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

void ShutterRemoteControl::Create() throw (AOException)
{
   
   //int stat;
  
   Logger::get()->log( Logger::LOG_LEV_TRACE, "ShutterRemoteControl Create");

   LoadConfig();
   
   errmsg = "";
   
   ERRMSG_LEN = 20;
   
   pthread_mutex_init(&threadMutex, NULL);
      
}      

int ShutterRemoteControl::LoadConfig()
{
   //Set up the fifo_path
   std::string pathtmp;
   std::string visao_root = getenv("VISAO_ROOT");
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["fifo_path"];
   }
   catch(Config_File_Exception)
   {
      pathtmp = "fifos";
   }
   fifo_path = visao_root + "/" + pathtmp + "/shuttercontrol_com_remote_";
   _logger->log(Logger::LOG_LEV_INFO, "Set fifo_path: %s", fifo_path.c_str());


   setup_fifo_list(1);
   set_fifo_list_channel(&fl, 0, 100,(char *)std::string(fifo_path + "out").c_str(), (char *) std::string(fifo_path + "in").c_str(), &read_fifo_channel, 0);
   
   try
   {
      wait_to = (float32)(ConfigDictionary())["wait_to"];
      _logger->log(Logger::LOG_LEV_INFO, "shutter timeout (wait_to) set to %f sec.", (float) wait_to);
   }
   catch(Config_File_Exception)
   {
      wait_to = DEFAULT_WAIT_TO;
      _logger->log(Logger::LOG_LEV_INFO, "shutter timeout (wait_to) set to default: %f sec.", (float) wait_to);
   }

   try
   {
      wait_sleep = (float32)(ConfigDictionary())["wait_sleep"];
      _logger->log(Logger::LOG_LEV_INFO, "shutter comm. sleep (wait_sleep) set to %f sec.", (float) wait_sleep);
   }
   catch(Config_File_Exception)
   {
      wait_sleep = DEFAULT_WAIT_SLEEP;
      _logger->log(Logger::LOG_LEV_INFO, "shutter comm. sleep (wait_sleep) set to default: %f sec.", (float) wait_sleep);
   }

   return NO_ERROR;
}

void ShutterRemoteControl::SetupVars()
{
   try 
   {
      var_state_cur = RTDBvar( MyFullName(), "STATE", CUR_VAR, INT_VARIABLE, 1,1);
      var_state_req = RTDBvar( MyFullName(), "STATE", REQ_VAR, INT_VARIABLE, 1,1);
      
      var_sw_state = RTDBvar( MyFullName(), "SW_STATE", CUR_VAR, INT_VARIABLE, 1,1);
      var_hw_state = RTDBvar( MyFullName(), "HW_STATE", CUR_VAR, INT_VARIABLE, 1,1);
      
      var_cmode_cur = RTDBvar( MyFullName(), "ConMode", CUR_VAR, INT_VARIABLE, 1,1);
      var_cmode_req = RTDBvar( MyFullName(), "ConMode", REQ_VAR, INT_VARIABLE, 1,1);
   } 
   catch (AOVarException &e)  
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
   
   this->Notify( var_state_req, ((ShutterRemoteControl *)this)->StateReqChanged);
   this->Notify( var_cmode_req, ((ShutterRemoteControl *)this)->CModeReqChanged);
   
}

int ShutterRemoteControl::parse_state_string(std::string ansstr)
{
   //std::cout << ansstr << "\n";
   std::string tmp;
   int cmode;
   int st, swst, hwst, pwrst;
   int sp = ansstr.find_first_of(" \r\n", 0);
   tmp.assign(ansstr, 0, sp);
   switch(tmp[0])
   {
      case 'N':
         cmode = 0;
         break;
      case 'R':
         cmode = 1;
         break;
      case 'L':
         cmode = 2;
         break;
		case 'S':
			cmode = 3;
			break;
      case 'A':
         cmode = 4;
         var_state_cur.Set(0, 0, FORCE_SEND);
         var_sw_state.Set(0, 0, FORCE_SEND);
         var_hw_state.Set(0, 0, FORCE_SEND);
         var_cmode_cur.Set(cmode, 0, FORCE_SEND);
         setCurState(STATE_OPERATING);
         return 0;
      default:
         cmode = -1;
         break;
   }
   
   int sp2 = ansstr.find_first_of(" \r\n", sp+1);
   tmp.assign(ansstr, sp+1, sp2-sp-1);
   st = atoi(tmp.c_str());
   sp = sp2;
   sp2 = ansstr.find_first_of(" \r\n", sp+1);
   tmp.assign(ansstr, sp+1, sp2-sp-1);
   swst = atoi(tmp.c_str());
   sp = sp2;
   sp2 = ansstr.find_first_of(" \r\n", sp+1);
   tmp.assign(ansstr, sp+1, sp2-sp-1);
   hwst = atoi(tmp.c_str());

   sp = sp2;
   sp2 = ansstr.find_first_of(" \r\n", sp+1);
   tmp.assign(ansstr, sp+1, sp2-sp-1);
   pwrst = atoi(tmp.c_str());

   if(pwrst) setCurState(STATE_READY);
   else setCurState(STATE_OFF);

   var_state_cur.Set(st, 0, FORCE_SEND);
   var_sw_state.Set(swst, 0, FORCE_SEND);
   var_hw_state.Set(hwst, 0, FORCE_SEND);
   var_cmode_cur.Set(cmode, 0, FORCE_SEND);
   
   return 0;
}

double get_curr_t()
{
   timeval tp;
   gettimeofday(&tp, 0);
   return (double) tp.tv_sec + ((double) tp.tv_usec)/1e6;
   
}

int ShutterRemoteControl::TestNetwork(void)
{
   int stat;

   pthread_mutex_lock(&mutex);


   // SendCommand() will lock the mutex by itself
   stat = send_shutter_command("state?");
   if (stat<0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Shutter: Error sending test command");
      pthread_mutex_unlock(&mutex);
      return stat;
   }
   
   

   double startT;
   
   startT = get_curr_t();
   read_fifo_channel(&fl.fifo_ch[0]);
   while((fl.fifo_ch[0].server_response[0] == '\0') && (get_curr_t() - startT < wait_to))
   {
      nusleep((int) (wait_sleep * 1e6)); //Sleep for a bit, give shutter time to respond
      
      //SIGIO is blocked if we are here, so start reading
      read_fifo_channel(&fl.fifo_ch[0]); //try again
   }
   if(fl.fifo_ch[0].server_response[0] == '\0')
   {
      _logger->log(Logger::LOG_LEV_ERROR, "ShutterControl response timeout in TestNetwork.  wait_to=%f.", wait_to);
      stat = -1;
   }
   else stat = NO_ERROR;

   pthread_mutex_unlock(&mutex);
  
    // Check if there actually was an answer
   if (stat != 0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Shutter: No answer to test command");
      return COMMUNICATION_ERROR;
   }

   std::string ansstr = fl.fifo_ch[0].server_response;
   
   parse_state_string(ansstr);

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Network test OK - answer %s", fl.fifo_ch[0].server_response);
   return NO_ERROR;
}

void ShutterRemoteControl::Run()
{
   signal(SIGIO, SIG_IGN);
   connect_fifo_list();
  
   global_fifo_list = &fl;


   _logger->log( Logger::LOG_LEV_INFO, "Running...");

   while(!TimeToDie()) 
   {
      try 
      {
         DoFSM();
      } 
      catch (AOException &e) 
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());

         // When the exception is thrown, the mutex was held!
         pthread_mutex_unlock(&threadMutex);
      }
   }
}
      
    
int ShutterRemoteControl::DoFSM()
{

   int status;//, stat;
   static float delay=1.0;

   // Lock out everyone else!!
   pthread_mutex_lock(&threadMutex);

      if(TestNetwork() != NO_ERROR) setCurState(STATE_NOCONNECTION);
      //else setCurState(STATE_OPERATING);
/*   }
      
   switch(status)
   {
      case STATE_NOCONNECTION:*/
            /*if (SetupNetwork() == NO_ERROR)
            {*/
        //       if (TestNetwork() == NO_ERROR) setCurState(STATE_CONNECTED);
               /*else ShutdownNetwork();
            }*/
            //setCurState(STATE_CONNECTED);
      //      break;
/*      case STATE_CONNECTED:
         //Add a ping here.
         //std::cout << "connected.\n";
            //setCurState(STATE_OPERATING);*/
            /*** STATE_OPERATING and STATE_OFF are set by parse_state_string ***/
//            break;
   //}
   // Always set current status (for external watchdog)
   setCurState( getCurState());
   pthread_mutex_unlock(&threadMutex);
   nusleep( (unsigned int)(delay * 1e6));
   
   return NO_ERROR;
}

int ShutterRemoteControl::StateReqChanged(void *pt, Variable *msgb)
{
   int stat, newstate;
   
   pthread_mutex_lock(&threadMutex);
   
   newstate = msgb->Value.Lv[0];

   if(newstate == 1) stat = ((ShutterRemoteControl *)pt)->send_shutter_command("open");
   if(newstate == -1) stat = ((ShutterRemoteControl *)pt)->send_shutter_command("close");

   pthread_mutex_unlock(&threadMutex);
   
   return stat;
}

int ShutterRemoteControl::CModeReqChanged(void *pt, Variable *msgb)
{
   int stat, newstate;
   pthread_mutex_lock(&threadMutex);
   
   newstate = msgb->Value.Lv[0];
   
   if(newstate == 1) stat = ((ShutterRemoteControl *)pt)->send_shutter_command("REMOTE");
   if(newstate == 10) stat = ((ShutterRemoteControl *)pt)->send_shutter_command("XREMOTE");
   if(newstate == 0) stat = ((ShutterRemoteControl *)pt)->send_shutter_command("~REMOTE");
   
   pthread_mutex_unlock(&threadMutex);
   return stat;
}

int ShutterRemoteControl::send_shutter_command(const char * com)
{
   //std::cout << "Sending " << com << "\n";
   //return SerialOut(com, strlen(com));
   return write_fifo_channel(0, com, strlen(com)+1, 0);
}

} //namespace VisAO

