#include "VisAOIClient.h"

//#define _debug

namespace VisAO
{

pthread_mutex_t threadMutex;

VisAOIClient::VisAOIClient(int argc, char **argv) throw(AOException) : MagAOIClient(argc, argv), VisAOApp_base()
{
   Create();
}

VisAOIClient::VisAOIClient( std::string name, const std::string &conffile) throw (AOException): MagAOIClient(name, conffile), VisAOApp_base()
{
   Create();
}

void VisAOIClient::Create() throw (AOException)
{
   std::string visao_root, fifoin, fifoout;

   LoadConfig();

   setup_fifo_list(3);
   
   wait_to = 1.0;
   visao_root = getenv("VISAO_ROOT");

   //Wollaston remote fifo.
   fifoin = visao_root +  "/" + fifo_path + "/" + wollaston_process + "_com_remote_in";
   fifoout = visao_root + "/" + fifo_path + "/" + wollaston_process + "_com_remote_out";
   set_fifo_list_channel(&fl, WOLL_FIFO, RWBUFF_SZ, fifoout.c_str(),fifoin.c_str(), 0, 0);

   //Focus remote fifo
   fifoin = visao_root +  "/" + fifo_path + "/" + focus_process + "_com_remote_in";
   fifoout = visao_root + "/" + fifo_path + "/" + focus_process + "_com_remote_out";
   set_fifo_list_channel(&fl, FOC_FIFO, RWBUFF_SZ, fifoout.c_str(),fifoin.c_str(), 0, 0);

   //Gimbal remote fifo
   fifoin = visao_root +  "/" + fifo_path + "/" + gimbal_process + "_com_remote_in";
   fifoout = visao_root + "/" + fifo_path + "/" + gimbal_process + "_com_remote_out";
   set_fifo_list_channel(&fl, GIMB_FIFO, RWBUFF_SZ, fifoout.c_str(),fifoin.c_str(), 0, 0);

   //Init the status board
   statusboard_shmemkey = STATUS_aosystem;
   if(create_statusboard(sizeof(aosystem_status_board)) != 0)
   {
      statusboard_shmemptr = 0;
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create status board.");
      aosb = 0;
   }
   else
   {
      VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;
      strncpy(bsb->appname, MyFullName().c_str(), 25);
      bsb->max_update_interval = pause_time;
      aosb = (VisAO::aosystem_status_board *) statusboard_shmemptr;
   }

   //Initialize reconstructor database
   rsb = 0;
   
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
      data_save_path = (std::string)(ConfigDictionary())["data_save_path"];
   }
   catch(Config_File_Exception)
   {
      data_save_path = "data/syslogs";
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set data save path to (data_save_path): %s", data_save_path.c_str());
      
   data_file_prefix = "aosys";
   
   pthread_mutex_init(&threadMutex, 0);
}

int VisAOIClient::LoadConfig()
{
   //Get the fifo path
   try 
   {
      fifo_path   = (std::string) ConfigDictionary()["fifo_path"];
   }
   catch (Config_File_Exception &e) 
   {
      fifo_path = "fifos";
   }

   //Read wollaston process name from config file
   try
   {
      wollaston_process = (std::string)(ConfigDictionary())["wollaston_process"];
      _logger->log(Logger::LOG_LEV_INFO, "Wollaston process name (wollaston_process) set to %s.", wollaston_process.c_str());
   }
   catch(Config_File_Exception)
   {
      wollaston_process = "wollastonstatus";
      _logger->log(Logger::LOG_LEV_INFO, "Wollaston process name (wollaston_process) set to default %s.",
                                                                                         wollaston_process.c_str());
   }

   
   //Read focus process name from config file
   try
   {
      focus_process = (std::string)(ConfigDictionary())["focus_process"];
      _logger->log(Logger::LOG_LEV_INFO, "Focus control name (focus_process) set to %s.", focus_process.c_str());
   }
   catch(Config_File_Exception)
   {
      focus_process = "focusmotor";
      _logger->log(Logger::LOG_LEV_INFO, "Focus control process name (focus_process) set to default %s.",
                                                                                         focus_process.c_str());
   }

   //Read gimbal process name from config file
   try
   {
      gimbal_process = (std::string)(ConfigDictionary())["gimbal_process"];
      _logger->log(Logger::LOG_LEV_INFO, "Gimbal control name (gimbal_process) set to %s.", gimbal_process.c_str());
   }
   catch(Config_File_Exception)
   {
      gimbal_process = "gimbal";
      _logger->log(Logger::LOG_LEV_INFO, "Gimbal control process name (gimbal_process) set to default %s.",
                   gimbal_process.c_str());
   }
   
   try
   {
      orient_useel = (int)(ConfigDictionary())["orient_useel"];
      orient_usepa = (int)(ConfigDictionary())["orient_usepa"];
   }
   catch(Config_File_Exception)
   {
      orient_useel = -1;
      orient_useel = -1;
      _logger->log(Logger::LOG_LEV_INFO, "Orientation params set to defaults.");
   }
      
   return 0;
}

void VisAOIClient::SetupVars()
{
   MagAOIClient::SetupVars();


   try
   {
      wollaston_status = RTDBvar( wollaston_process, "L.STATUS", NO_DIR, INT_VARIABLE, 1,1);
      wollaston_cmode_cur = RTDBvar( wollaston_process, "L.ConMode", CUR_VAR, INT_VARIABLE, 1,1);
      wollaston_cmode_req = RTDBvar( wollaston_process, "L.ConMode", REQ_VAR, INT_VARIABLE, 1,1);
      Notify(wollaston_cmode_req, WollCModeReqChanged);

      wollaston_state_cur = RTDBvar( wollaston_process, "L.state", CUR_VAR, INT_VARIABLE, 1,1);
      wollaston_state_req = RTDBvar( wollaston_process, "L.state", REQ_VAR, INT_VARIABLE, 1,1);
      Notify(wollaston_state_req, WollStateReqChanged);
   }
   catch(...)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "Error creating wollaston status variables.");
      throw;
   }


   try
   {
      focus_cmode_cur = RTDBvar( focus_process, "L.ConMode", CUR_VAR, INT_VARIABLE, 1,1);
      focus_cmode_req = RTDBvar( focus_process, "L.ConMode", REQ_VAR, INT_VARIABLE, 1,1);
      Notify(focus_cmode_req, FocusCModeReqChanged);

      focus_pos_cur = RTDBvar( focus_process, "L.pos", CUR_VAR, REAL_VARIABLE, 1,1);
      focus_pos_req = RTDBvar( focus_process, "L.pos", REQ_VAR, REAL_VARIABLE, 1,1);
      Notify(focus_pos_req, FocusPosReqChanged);

      focus_abort_req = RTDBvar( focus_process, "L.abort", REQ_VAR, INT_VARIABLE, 1,1);
      Notify(focus_abort_req, FocusAbortReqChanged);

      focus_preset_req = RTDBvar( focus_process, "L.preset", REQ_VAR, INT_VARIABLE, 1,1);
      Notify(focus_preset_req, FocusPresetReqChanged);
      
      focus_status = RTDBvar( focus_process, "L.STATUS", NO_DIR, INT_VARIABLE, 1,1);
      focus_limsw_cur = RTDBvar( focus_process, "L.limsw", CUR_VAR, INT_VARIABLE, 1,1);
   }
   catch(...)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "Error creating focus status variables.");
      throw;
   }


   try
   {
      gimbal_xpos_cur = RTDBvar( gimbal_process, "L.xpos", CUR_VAR, REAL_VARIABLE, 1,1);
      gimbal_ypos_cur = RTDBvar( gimbal_process, "L.ypos", CUR_VAR, REAL_VARIABLE, 1,1);
      gimbal_cmode_cur = RTDBvar( gimbal_process, "L.ConMode", CUR_VAR, INT_VARIABLE, 1,1);
      gimbal_cmode_req = RTDBvar( gimbal_process, "L.ConMode", REQ_VAR, INT_VARIABLE, 1,1);
      Notify(gimbal_cmode_req, GimbalCModeReqChanged);
      
      gimbal_center_req = RTDBvar( gimbal_process, "L.center", REQ_VAR, INT_VARIABLE, 1,1);
      gimbal_dark_req = RTDBvar( gimbal_process, "L.dark", REQ_VAR, INT_VARIABLE, 1,1);
      Notify(gimbal_center_req, GimbalCenterReqChanged);
      Notify(gimbal_dark_req, GimbalDarkReqChanged);

      gimbal_status = RTDBvar( gimbal_process, "L.STATUS", NO_DIR, INT_VARIABLE, 1,1);
   }
   catch(...)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "Error creating gimbal status variables.");
      throw;
   }


   try
   {
      var_masterremote_req = RTDBvar( MyFullName(), "Master.Remote", REQ_VAR, INT_VARIABLE, 1,1);
      //var_masterremote_cur = RTDBvar( MyFullName(), "Master.Remote", CUR_VAR, INT_VARIABLE, 1,1);
      Notify(var_masterremote_req, MasterRemoteReqChanged);

      ccd47_cmode_req = RTDBvar( "ccd47.L", "ConMode", REQ_VAR, INT_VARIABLE, 1,1);
      shutter_cmode_req = RTDBvar( "shutterremote.L", "ConMode", REQ_VAR, INT_VARIABLE, 1, 1);
      fw2_cmode_req = RTDBvar( "filterwheel2.L", "ConMode", REQ_VAR, INT_VARIABLE, 1,1);
      fw3_cmode_req = RTDBvar( "filterwheel3.L", "ConMode", REQ_VAR, INT_VARIABLE, 1,1);
      
      var_avgwfe = RTDBvar( "visao", "avgwfe", NO_DIR, REAL_VARIABLE, 1,1);
      var_stdwfe = RTDBvar( "visao", "stdwfe", NO_DIR, REAL_VARIABLE, 1,1);
      var_instwfe = RTDBvar( "visao", "instwfe", NO_DIR, REAL_VARIABLE, 1,1);
   }
   catch(...)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "Error creating Master Remote variables.");
      throw;
   }
}

int VisAOIClient::WollStateReqChanged(void *pt, Variable *msgb __attribute__((unused)))
{
   int stat = -1;
   int newstate;
   pthread_mutex_lock(&threadMutex);

   newstate = msgb->Value.Lv[0];
   
   if(newstate == 1) stat = ((VisAOIClient *)pt)->send_wollaston_command("up");
   if(newstate == -1) stat = ((VisAOIClient *)pt)->send_wollaston_command("down");
   if(newstate == -2) stat = ((VisAOIClient *)pt)->send_wollaston_command("ignore");
   
   pthread_mutex_unlock(&threadMutex);
   return stat;
}

int VisAOIClient::WollCModeReqChanged(void *pt, Variable *msgb)
{
   int stat = -1;
   int newcmode;
   pthread_mutex_lock(&threadMutex);

   newcmode = msgb->Value.Lv[0];
   
   if(newcmode == 1) stat = ((VisAOIClient *)pt)->send_wollaston_command("REMOTE");
   if(newcmode == 10) stat = ((VisAOIClient *)pt)->send_wollaston_command("XREMOTE");
   if(newcmode == 0) stat = ((VisAOIClient *)pt)->send_wollaston_command("~REMOTE");
   
   pthread_mutex_unlock(&threadMutex);
   return stat;
}

int VisAOIClient::send_wollaston_command(std::string com)
{
   std::string resp;
   write_fifo_channel(WOLL_FIFO, com.c_str(), com.length()+1, &resp);

   if(resp[0] != 0) return -1;
   
   return 0;
}

int VisAOIClient::update_wollaston()
{
   std::string resp;


   pthread_mutex_lock(&threadMutex);
   write_fifo_channel(WOLL_FIFO, "state?", 6, &resp);
   pthread_mutex_unlock(&threadMutex);

   int status, cmode, pos, prompt;
   if(resp == "" || resp.length() <= 2)
   {
      status = STATE_NOCONNECTION;
      cmode = -1;
      pos = -2;
      prompt = 0;
   }
   
   if(resp.length() > 2)
   {
      status = STATE_READY;
      switch(resp[0])
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
            break;
         default:
            cmode = -1;
      }

      pos = atoi(resp.substr(2,2).c_str());

      prompt = atoi(resp.substr(4,resp.length()-4).c_str());
   }

   wollaston_status.Set(status, 0, CHECK_SEND);
   wollaston_cmode_cur.Set(cmode, 0, CHECK_SEND);

   if(prompt) pos += 10;

   wollaston_state_cur.Set(pos, 0, CHECK_SEND);
   return 0;

}

int VisAOIClient::FocusPosReqChanged(void *pt, Variable *msgb)
{
   int stat;
   double newpos;
   char posstr[20];

   

   newpos = msgb->Value.Dv[0];

   snprintf(posstr, 20, "pos %0.4f\n", newpos);

   pthread_mutex_lock(&threadMutex);

   stat = ((VisAOIClient *)pt)->send_focus_command(posstr);

   pthread_mutex_unlock(&threadMutex);
   return stat;
}

int VisAOIClient::FocusAbortReqChanged(void *pt, Variable *msgb __attribute__((unused)))
{
   int stat;

   pthread_mutex_lock(&threadMutex);

   stat = ((VisAOIClient *)pt)->send_focus_command("abort");

   pthread_mutex_unlock(&threadMutex);

   return stat;
}

int VisAOIClient::FocusCModeReqChanged(void *pt, Variable *msgb)
{
   int stat = -1;
   int newcmode;
   pthread_mutex_lock(&threadMutex);

   newcmode = msgb->Value.Lv[0];
   
   if(newcmode == 1) stat = ((VisAOIClient *)pt)->send_focus_command("REMOTE");
   if(newcmode == 10) stat = ((VisAOIClient *)pt)->send_focus_command("XREMOTE");
   if(newcmode == 0) stat = ((VisAOIClient *)pt)->send_focus_command("~REMOTE");
   
   pthread_mutex_unlock(&threadMutex);
   return stat;
}

int VisAOIClient::FocusPresetReqChanged(void *pt, Variable *msgb __attribute__((unused)))
{
   int stat;
   pthread_mutex_lock(&threadMutex);

   stat = ((VisAOIClient *)pt)->send_focus_command("preset");
   
   pthread_mutex_unlock(&threadMutex);
   return stat;
}

int VisAOIClient::send_focus_command(std::string com)
{
   std::string resp;
   write_fifo_channel(FOC_FIFO, com.c_str(), com.length()+1, &resp);

   if(resp[0] != 0) return -1;
   
   return 0;
}

int VisAOIClient::update_focus()
{
   std::string resp;


   pthread_mutex_lock(&threadMutex);
   write_fifo_channel(FOC_FIFO, "state?", 6, &resp);
   pthread_mutex_unlock(&threadMutex);

   int connected, cmode, enabled, is_moving, homing, neg_limit, pos_limit, power, pos_limit_disabled;
   double pos, remaining;

   if(resp.length() < 16)
   {
      connected = 0;
      cmode = -1;
      pos = -1e12;
      is_moving = -2;
      remaining = -1e-12;
      power = 0;
   }
   else
   {
      connected = 1;
      switch(resp[0])
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
            break;
         default:
            cmode = -1;
      }
      
      pos = strtod(resp.substr(2,12).c_str(),0);

      enabled = atoi(resp.substr(15,1).c_str());
      
      power = atoi(resp.substr(17,1).c_str());
      
      is_moving = atoi(resp.substr(19,1).c_str());
      
      homing = atoi(resp.substr(21,1).c_str());
      
      neg_limit = atoi(resp.substr(23,1).c_str());
      pos_limit = atoi(resp.substr(27,1).c_str());
      
      remaining = strtod(resp.substr(29,12).c_str(),0);
      
      pos_limit_disabled = atoi(resp.substr(42,1).c_str());
      
   }

   focus_cmode_cur.Set(cmode, 0, CHECK_SEND);

   if(power == 0)
   {
      if(connected)
      {
         focus_status.Set(STATE_OFF, 0, CHECK_SEND);
      }
      else focus_status.Set(STATE_NOCONNECTION, 0, CHECK_SEND);

      focus_pos_cur.Set(pos, 0, CHECK_SEND);
      focus_limsw_cur.Set(0, 0, CHECK_SEND);
   }
   else
   {
      if(is_moving && !homing) focus_status.Set(STATE_OPERATING, 0, CHECK_SEND);
      else if(homing) focus_status.Set(STATE_HOMING, 0, CHECK_SEND);
      else focus_status.Set(STATE_READY, 0, CHECK_SEND);

      if(neg_limit && pos_limit) focus_limsw_cur.Set(-10, 0, CHECK_SEND); //Send an error condition first
      else if(pos_limit_disabled) focus_limsw_cur.Set(-5, 0, CHECK_SEND); //Then send pos_limit_disabled
      else //otherwise normal limit switch logic.
      {
         if(neg_limit && !pos_limit) focus_limsw_cur.Set(-1, 0, CHECK_SEND);
         else if(!neg_limit && pos_limit) focus_limsw_cur.Set(1, 0, CHECK_SEND);         
         else focus_limsw_cur.Set(0, 0, CHECK_SEND);
      }

      focus_pos_cur.Set(pos, 0, CHECK_SEND);
   }
   return 0;

}

int VisAOIClient::GimbalCModeReqChanged(void *pt, Variable *msgb)
{
   int stat = -1;
   int newcmode;
   pthread_mutex_lock(&threadMutex);
   
   newcmode = msgb->Value.Lv[0];
   
   if(newcmode == 1) stat = ((VisAOIClient *)pt)->send_gimbal_command("REMOTE");
   if(newcmode == 10) stat = ((VisAOIClient *)pt)->send_gimbal_command("XREMOTE");
   if(newcmode == 0) stat = ((VisAOIClient *)pt)->send_gimbal_command("~REMOTE");
   
   pthread_mutex_unlock(&threadMutex);
   return stat;
}

int VisAOIClient::GimbalCenterReqChanged(void *pt, Variable *msgb __attribute__((unused)))
{
   int stat;
   pthread_mutex_lock(&threadMutex);

   stat = ((VisAOIClient *)pt)->send_gimbal_command("center");
   
   pthread_mutex_unlock(&threadMutex);
   return stat;
}

int VisAOIClient::GimbalDarkReqChanged(void *pt, Variable *msgb __attribute__((unused)))
{
   int stat;
   pthread_mutex_lock(&threadMutex);

   stat = ((VisAOIClient *)pt)->send_gimbal_command("dark");
   
   pthread_mutex_unlock(&threadMutex);
   return stat;
}

int VisAOIClient::send_gimbal_command(std::string com)
{
   std::string resp;
   write_fifo_channel(GIMB_FIFO, com.c_str(), com.length()+1, &resp);
   
   if(resp[0] != 0) return -1;
   
   return 0;
}

int VisAOIClient::update_gimbal()
{
   std::string resp, tmp;


   pthread_mutex_lock(&threadMutex);
   write_fifo_channel(GIMB_FIFO, "state?", 6, &resp);
   pthread_mutex_unlock(&threadMutex);

   int epos, spos;
   
   int connected, cmode, is_moving, curState, pwrState, xMoving, yMoving;
   double xPos, yPos;

   

   if(resp != "" && resp.length() < 5)
   {
      return 0; //This means we got somebody elses response on a fifo.
      //This happens in the GUI, but probably not here.
   }


   if(resp == "" || resp.length()<5)
   {
      connected = 0;
      cmode = -1;
   
      return 0;

   }
   else
   {
      connected = 1;
      switch(resp[0])
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
            break;
         default:
            cmode = -1;
      }

      

      spos = resp.find(',', 0);
      epos = resp.find(',', spos+1);
      
      tmp = resp.substr(spos+1, epos-spos-1);

      

      curState = atoi(tmp.c_str());
      
      spos = resp.find(',', epos);
      epos = resp.find(',', spos+1);
      
      pwrState = atoi(tmp.c_str());
      
      spos = resp.find(',', epos);
      epos = resp.find(',', spos+1);
      
      tmp = resp.substr(spos+1, epos-spos-1);

  

      xMoving = atoi(tmp.c_str());
      
      spos = resp.find(',', epos);
      epos = resp.find(',', spos+1);
      
      tmp = resp.substr(spos+1, epos-spos-1);

   
      xPos = strtod(tmp.c_str(),0);
      
      //** y moving **//
      spos = resp.find(',', epos);
      epos = resp.find(',', spos+1);
      
      tmp = resp.substr(spos+1, epos-spos-1);
    
      yMoving = atoi(tmp.c_str());

      is_moving = yMoving + xMoving;
      
      //** y position **//
      spos = resp.find(',', epos);
      epos = resp.find(',', spos+1);
      if(epos == -1) epos = resp.length();
      
      tmp = resp.substr(spos+1, epos-spos-1);

      yPos = strtod(tmp.c_str(),0);
      
      //** Scale ** //
      spos = resp.find(',', epos);
      epos = resp.find(',', spos+1);
      if(epos == -1) epos = resp.length();
      
      tmp = resp.substr(spos+1, epos-spos-1);

      
   }

   

   gimbal_cmode_cur.Set(cmode, 0, CHECK_SEND);

   
   if(!connected)
   {

      gimbal_status.Set(STATE_NOCONNECTION, 0, CHECK_SEND);
   
   }
   else
   {
      if(is_moving) gimbal_status.Set(STATE_OPERATING, 0, CHECK_SEND);
      else gimbal_status.Set(STATE_READY, 0, CHECK_SEND);
   
      gimbal_xpos_cur.Set(xPos, 0, CHECK_SEND);
      gimbal_ypos_cur.Set(yPos, 0, CHECK_SEND);
   
   }
   
   return 0;
   
}

int VisAOIClient::MasterRemoteReqChanged(void *pt, Variable *msgb)
{

   int stat, newcmode;
   pthread_mutex_lock(&threadMutex);
      
   newcmode = msgb->Value.Lv[0];

   
   stat = ((VisAOIClient *)pt)->setMasterRemote(newcmode);
      
   pthread_mutex_unlock(&threadMutex);
   return stat;
}
                                                                  
int VisAOIClient::setMasterRemote(int cmode)
{
   if(cmode == 1)
   {
      send_gimbal_command("REMOTE");
      send_focus_command("REMOTE");
   }
   else if(cmode == 10)
   {
      send_gimbal_command("XREMOTE");
      send_focus_command("XREMOTE");
   }
   else if(cmode == 0)
   {
      send_gimbal_command("~REMOTE");
      send_focus_command("~REMOTE");
   }

   ccd47_cmode_req.Set(cmode, 0, FORCE_SEND);
   shutter_cmode_req.Set(cmode, 0, FORCE_SEND);
   fw2_cmode_req.Set(cmode, 0, FORCE_SEND);
   fw3_cmode_req.Set(cmode, 0, FORCE_SEND);

   return 0;
}

void VisAOIClient::Run()
{
   _logger->log( Logger::LOG_LEV_INFO, "Running...");

   init_DD(); //strtod(aoi.side.ao.loop_gains.substr(p+4, 4).c_str(), 0);

   init_statusboard();
   
   signal(SIGIO, SIG_IGN);
   connect_fifo_list();
  
   global_fifo_list = &fl;

   while(!TimeToDie()) 
   {
      
      try 
      {
         sleep(1);
         
         update_statusboard();
         update_wollaston();
         update_focus();
         update_gimbal();
         update_recon();
         
      } 
      catch (AOException &e) 
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());
                 
         sleep(1);
      }

   }
}

void VisAOIClient::post_update_DD_var(DD_RTDBVar &var)
{

   if(statusboard_shmemptr)
   {
//       if(var.RTVar.complete_name() == "ADSEC.L.G_GAIN_A@M_ADSEC")
//       {
// 
//          std::cout << "-------> " << aoi.side.ao.loop_gains << std::endl;
//          
//          int p = aoi.side.ao.loop_gains.find("g_tt_", 0);
//          if(p < 0 || p > (int)aoi.side.ao.loop_gains.length() - 4)
//          {
//             aosb->loop_gain_tt = 0.;
//          }
//          else
//          {
//             
//             aosb->loop_gain_tt = strtod(aoi.side.ao.loop_gains.substr(p+5, 4).c_str(), 0);
//             
//             
//             
//             p = aoi.side.ao.loop_gains.find("_ho1_", 0);
//             std::cout << "p: " << p << "\n";
//             if(p < 0 || p > (int)aoi.side.ao.loop_gains.length() - 5)
//             {
//                aosb->loop_gain_ho1 = 0.;
//             }
//             else
//             {
//                aosb->loop_gain_ho1 = strtod(aoi.side.ao.loop_gains.substr(p+5, 4).c_str(), 0);
//             }
//             
//             p = aoi.side.ao.loop_gains.find("_ho2_", p);
//             if(p < 0 || p > (int)aoi.side.ao.loop_gains.length() - 5)
//             {
//                aosb->loop_gain_ho2 = 0.;
//             }
//             else
//             {
//                aosb->loop_gain_ho2 = strtod(aoi.side.ao.loop_gains.substr(p+5, 4).c_str(), 0);
//             }
//          }
/*         
         return;
      }//"ADSEC.L.G_GAIN_A@M_ADSEC"*/
      
      if(var.RTVar.complete_name() == "optloopdiag.L.COUNTS")
      {
         aosb->wfs_counts = aoi.side.wfs1.counts;
         
         return;
      }//"optloopdiag.L.COUNTS"
      
      if(var.RTVar.complete_name() == "wfsarb.L.FSM_STATE")
      {
         update_loopon();         
      }//"wfsarb.L.FSM_STATE"
      
      if(var.RTVar.complete_name() == "MagAOI.cmds.NodRaDec.InProgress@M_ADSEC")
      {
         aosb->nodInProgress = aoi.nodInProgress;
         std::cout << "nodInProgress: " << aosb->nodInProgress << "\n"; 
      }
   }// if(statusboard_shmemptr)
}

void VisAOIClient::update_loopon()
{
   int old_loop_on = aosb->loop_on;
   
   if(aoi.side.wfs1.status == "LoopClosed") aosb->loop_on = 1;
   else if(aoi.side.wfs1.status == "LoopPaused") aosb->loop_on = 2;
   else aosb->loop_on = 0;
   
   if (aosb->loop_on == 1)
   {
      //Check gains
      if(aosb->loop_gain_tt == 0.0000) aosb->loop_on = 2;
   }
   
   if(old_loop_on == 1 && aosb->loop_on != 1) aosb->loop_open_counter++;
   
   timeval tv;
   gettimeofday(&tv, 0);
   dataLogger(tv);
   
}

void VisAOIClient::init_statusboard()
{
   if(statusboard_shmemptr)
   {
      

      aosb->dateobs[0] = '\0';
      aosb->ut = 0;
      aosb->epoch = 0;
      aosb->ra = aoi.ra;
      aosb->dec = aoi.dec;
      aosb->az = aoi.az.position;
      aosb->el = aoi.el.position;
      aosb->am = aoi.am;
      aosb->pa = aoi.pa;
      aosb->ha = aoi.ha;
      aosb->zd = aoi.zd;
      aosb->st = aoi.st;
      aosb->filter1_pos = 0.;
      aosb->filter1_reqpos = 0.;

      aosb->correctedmodes = aoi.side.ao.correctedmodes;
      strncpy(aosb->filter1_name, aoi.side.wfs1.filter1.c_str(), 256);
      
      for(int i=0;i<3;i++)
      {
         aosb->tt_amp[i] = aoi.side.ao.tt_amp[i];
         aosb->tt_freq[i] = aoi.side.ao.tt_freq[i];
         aosb->tt_offset[i] = aoi.side.ao.tt_offset[i];
      }

      
      
      
      
      aosb->ccd39_freq = aoi.side.wfs1.ccdfreq;
      aosb->ccd39_bin = aoi.side.wfs1.ccdbin;
      std::cout << "-------> " << aoi.side.ao.loop_gains << std::endl;
      int p = aoi.side.ao.loop_gains.find("g_tt_", 0);
      if(p < 0 || p > (int)aoi.side.ao.loop_gains.length() - 4)
      {
         aosb->loop_gain_tt = 0.;
      }
      else
      {
         aosb->loop_gain_tt = strtod(aoi.side.ao.loop_gains.substr(p+5, 4).c_str(), 0);
         p = aoi.side.ao.loop_gains.find("_ho1_", p);
         if(p < 0 || p > (int) aoi.side.ao.loop_gains.length() - 4)
         {
            aosb->loop_gain_ho1 = 0.;
         }
         else
         {
            aosb->loop_gain_ho1 = strtod(aoi.side.ao.loop_gains.substr(p+5, 4).c_str(), 0);
         }
         p = aoi.side.ao.loop_gains.find("_ho2_", p);
         if(p < 0 || p > (int) aoi.side.ao.loop_gains.length() - 5)
         {
            aosb->loop_gain_ho2 = 0.;
         }
         else
         {
            aosb->loop_gain_ho2 = strtod(aoi.side.ao.loop_gains.substr(p+5, 4).c_str(), 0);
         }
      }

      update_loopon();
      
      aosb->wfs_counts = aoi.side.wfs1.counts;
      
      aosb->loop_open_counter = 0;
      
   }
}

//Parse the reconstructor file name.
std::string parse_for_fits(std::string & path)
{
   int fitstart = path.rfind(".fits");
   
   int slash = path.rfind("/", fitstart);
   std::cout << path << "\n";
   std::cout << fitstart << " " << slash << "\n";
   
   if(slash > -1)   return path.substr(slash+1);
   else return "none";
}
   
int VisAOIClient::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      strncpy(aosb->dateobs, aoi.dateobs.c_str(), 256);
      aosb->ut = aoi.ut;
      aosb->epoch = aoi.epoch;
      aosb->ra = aoi.ra;
      aosb->dec = aoi.dec;
      aosb->az = aoi.az.position;
      aosb->el = aoi.el.position;
      aosb->am = aoi.am;
      aosb->pa = aoi.pa;
      aosb->ha = aoi.ha;
      aosb->zd = aoi.zd;
      aosb->st = aoi.st;

      aosb->istracking = aoi.istracking;
      aosb->isguiding = aoi.isguiding;
      aosb->isslewing = aoi.isslewing;
      aosb->guider_ismoving = aoi.guider_ismoving;

      aosb->catra = aoi.cat.ra;
      aosb->catdec = aoi.cat.dec;
      aosb->catep = aoi.cat.epoch;
      aosb->catro = aoi.cat.rotOff;
   
      strncpy(aosb->catrm, aoi.cat.rotMode.c_str(), 256);
      strncpy(aosb->catobj, aoi.cat.obj.c_str(), 256);

      strncpy(aosb->obsinst, aoi.cat.obsinst.c_str(), 256);
      strncpy(aosb->obsname, aoi.cat.obsname.c_str(), 256);
      
      aosb->rotang = aoi.rotator.angle;
      aosb->rotoffset = aoi.rotator.offset;
      aosb->rotfollowing = aoi.rotator.following;

      aosb->wxtemp = aoi.environ.wxtemp;
      aosb->wxpres = aoi.environ.wxpres;
      aosb->wxhumid =  aoi.environ.wxhumid;
      aosb->wxwind = aoi.environ.wxwind;
      aosb->wxwdir = aoi.environ.wxwdir;
      aosb->wxdewpoint = aoi.environ.wxdewpoint;
      aosb->wxpwvest = aoi.environ.wxpwvest;
      aosb->ttruss = aoi.environ.ttruss;
      aosb->tcell = aoi.environ.tcell;
      aosb->tseccell = aoi.environ.tseccell;
      aosb->tambient = aoi.environ.tambient;
      aosb->dimmfwhm = aoi.environ.dimmfwhm;
      aosb->dimmtime = aoi.environ.dimmtime;
      aosb->mag1fwhm = aoi.environ.mag1fwhm;
      aosb->mag1time = aoi.environ.mag1time;
      aosb->mag2fwhm = aoi.environ.mag2fwhm;
      aosb->mag2time = aoi.environ.mag2time;
      
      aosb->baysidex = aoi.side.wfs1.baysidex;      
      aosb->baysidex_enabled = aoi.side.wfs1.baysidex_enabled;
      aosb->baysidey = aoi.side.wfs1.baysidey;
      aosb->baysidey_enabled = aoi.side.wfs1.baysidey_enabled;
      aosb->baysidez = aoi.side.wfs1.baysidez;
      aosb->baysidez_enabled = aoi.side.wfs1.baysidez_enabled;

      aosb->filter1_pos = aoi.side.wfs1.filter1_pos;

      if(aoi.side.wfs1.filter1_reqpos == aoi.side.wfs1.filter1_homingpos)
      {
         aosb->filter1_reqpos = aoi.side.wfs1.filter1_startpos;
      }
      else if(aoi.side.wfs1.filter1_reqpos == aoi.side.wfs1.filter1_abortpos)
      {
         aosb->filter1_reqpos = aoi.side.wfs1.filter1_pos;
      }
      else
      {  
         aosb->filter1_reqpos = aoi.side.wfs1.filter1_reqpos;
      }  
      
      
      strncpy(aosb->filter1_name, aoi.side.wfs1.filter1.c_str(), 256);


      aosb->correctedmodes = aoi.side.ao.correctedmodes;
      strncpy(aosb->filter1_name, aoi.side.wfs1.filter1.c_str(), 256);

      aosb->n_modes = aoi.side.ao.nmodes;
      aosb->homiddle = aoi.side.ao.homiddle;
      aosb->loop_gain_tt = aoi.side.ao.gain_tt;
      aosb->loop_gain_ho1 = aoi.side.ao.gain_ho1;
      aosb->loop_gain_ho2 = aoi.side.ao.gain_ho2;
      
      for(int i=0;i<3;i++)
      {
         aosb->tt_amp[i] = aoi.side.ao.tt_amp[i];
         aosb->tt_freq[i] = aoi.side.ao.tt_freq[i];
         aosb->tt_offset[i] = aoi.side.ao.tt_offset[i];
      }


      aosb->ccd39_freq = aoi.side.wfs1.ccdfreq;
      aosb->ccd39_bin = aoi.side.wfs1.ccdbin;

      strncpy(aosb->reconstructor,parse_for_fits(aoi.side.ao.reconstructor).c_str(), 256);
      
      aosb->orient_useel = orient_useel;
      aosb->orient_usepa = orient_usepa;
      
      update_loopon();
      
      //#ifdef _debug
      dump_statusboard();
      //#endif
   }
   return 0;
}//int VisAOIClient::update_statusboard()

void VisAOIClient::dump_statusboard()
{
   
   VisAO::aosystem_status_board * aosb = (VisAO::aosystem_status_board *) statusboard_shmemptr;
   std::cout << "VisAOIClient ao status board\n";
   std::cout << "Tgt:\t" << aosb->catobj << "\n";
   std::cout << "Tracking:\t" << aosb->istracking << "\n";
   std::cout << "Guiding:\t" << aosb->isguiding << "\n\n";
   std::cout << "DATEOBS:\t" << aosb->dateobs << "\n";
   std::cout << "UT:\t" << aosb->ut << "\n";
   std::cout << "EPOCH:\t" << aosb->epoch << "\n";
   std::cout << "RA:\t" << aosb->ra << "\n";
   std::cout << "DEC:\t" << aosb->dec << "\n";
   std::cout << "AZ:\t" << aosb->az << "\n";
   std::cout << "EL:\t" << aosb->el << "\n";
   std::cout << "AM:\t" << aosb->am << "\n";
   std::cout << "PA:\t" << aosb->pa << "\n";
   std::cout << "HA:\t" << aosb->ha << "\n";
   std::cout << "ZD:\t" << aosb->zd << "\n";
   std::cout << "ST:\t" << aosb->st << "\n";
   
   std::cout << "Rot-ang:\t" << aosb->rotang << "\n";
   std::cout << "Rot-offset:\t" << aosb->rotoffset << "\n";
   std::cout << "Rot-following:\t" << aosb->rotfollowing << "\n";
   
   std::cout << "obsinst:\t" << aosb->obsinst << "\n";
   std::cout << "obsname:\t" << aosb->obsname << "\n";
   
   std::cout << "wxtemp:\t" << aosb->wxtemp << "\n";
   std::cout << "wxpres:\t" << aosb->wxpres << "\n";
   std::cout << "wxhumid:\t" << aosb->wxhumid << "\n";
   std::cout << "wxwind:\t" << aosb->wxwind << "\n";
   std::cout << "wxwdir:\t" << aosb->wxwdir << "\n";
   std::cout << "ttruss:\t" << aosb->ttruss << "\n";
   std::cout << "tcell:\t" << aosb->tcell << "\n";
   std::cout << "tseccell:\t" << aosb->tseccell << "\n";
   std::cout << "tambient:\t" << aosb->tambient << "\n";
   std::cout << "dimmfwhm:\t" << aosb->dimmfwhm << "\n";
   std::cout << "mag1fwhm:\t" << aosb->mag1fwhm << "\n";
   std::cout << "mag2fwhm:\t" << aosb->mag2fwhm << "\n";
   
   std::cout << "baysidex:\t" << aosb->baysidex << "\n";      
   std::cout << "baysidex_enabled:\t" << aosb->baysidex_enabled << "\n";
   std::cout << "baysidey:\t" << aosb->baysidey << "\n";
   std::cout << "baysidey_enabled:\t" << aosb->baysidey_enabled << "\n";
   std::cout << "baysidez:\t" << aosb->baysidez << "\n";
   std::cout << "baysidez_enabled:\t" << aosb->baysidez_enabled << "\n";   
   
   std::cout << "FW 1:\t" << aosb->filter1_pos << " " << aosb->filter1_name << "\n";
   std::cout << "FW 1 REQ:\t" << aosb->filter1_reqpos << "\n";
   
   std::cout << "Reconstructor:\t" << aosb->reconstructor << "\n";
   std::cout << "Modes:\t" << aosb->correctedmodes << "\n";
   std::cout << "TT offset\t freq \t amp\n";
   for(int i=0;i<3;i++)  std::cout << "\t" << aosb->tt_offset[i] << "\t" << aosb->tt_freq[i] << "\t" << aosb->tt_amp[i] << "\n";
   std::cout << "Loop: " << aosb->loop_on << "\n";
   std::cout << "Modes: " << aosb->correctedmodes << "\n";
   std::cout << "Speed: " << aosb->ccd39_freq << "\n";
   std::cout << "Bin: " << aosb->ccd39_bin << "\n";
   std::cout << "Gainset nmodes: " << aosb->n_modes << "\n";
   std::cout << "Gainset homiddle: " << aosb->homiddle << "\n";
   std::cout << "Gains: " << aosb->loop_gain_tt << " " << aosb->loop_gain_ho1 << " " << aosb->loop_gain_ho2 << "\n";
   std::cout << "Counts: " << aosb->wfs_counts << "\n";
   
   std::cout << std::endl;
}//int VisAOIClient::dump_statusboard()

void VisAOIClient::update_recon()
{
   size_t sz;
   
   if(!rsb)
   {
      rsb = (VisAO::reconstructor_status_board*) attach_shm(&sz,  STATUS_reconstructor, 0);
   }
   
   if(rsb)
   {
      if((get_curr_time() - ts_to_curr_time(&rsb->last_update)) < 2.*rsb->max_update_interval)
      {
         var_avgwfe.Set(rsb->avgwfe_1_sec,0, FORCE_SEND);
         var_stdwfe.Set(rsb->stdwfe_1_sec,0, FORCE_SEND);
         var_instwfe.Set(rsb->inst_wfe,0, FORCE_SEND);
      }
      else
      {
         var_avgwfe.Set(-1.,0, FORCE_SEND);
         var_stdwfe.Set(-1.,0, FORCE_SEND);
         var_instwfe.Set(-1.,0, FORCE_SEND);
         
      }
      
   }
}



void VisAOIClient::dataLogger(timeval tv)
{
   checkDataFileOpen();

   dataof << tv.tv_sec << " " << tv.tv_usec << " " << aosb->loop_on;
   
   if(rsb) dataof << " " << rsb->avgwfe_1_sec << " " << rsb->stdwfe_1_sec;
   else dataof << " " << -1.0 << " " << -1.0;
   dataof << "\n";

   if(!dataof.good())
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "Error in AOsystem data file.  AO system data may not be logged correctly");
   }

  
}

} //namespace VisAO
