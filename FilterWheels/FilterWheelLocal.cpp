/************************************************************
*    FilterWheelLocal.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the FilterWheelLocal class, which adapts
* the SimpleMotorCtrl class for use as a VisAO App.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file FilterWheelLocal.cpp
  * \author Jared R. Males
  * \brief Declarations for the FilterWheelLocal class.
  *
*/

#include "FilterWheelLocal.h"

int need_restart;

namespace VisAO
{

FilterWheelLocal::FilterWheelLocal( std::string name, const std::string &conffile) throw (AOException) : AOApp( name, conffile)
{
   setupVisAOApp();
}

FilterWheelLocal::FilterWheelLocal( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   setupVisAOApp();
}

void FilterWheelLocal::setupVisAOApp()
{
   need_restart = 0;

   std::string pathtmp;
   setup_fifo_list(2); //We're only going to use the local and script fifos.

   set_app_name(MyName());

   aoapp_name = MyFullName();

   int spos = aoapp_name.find("Local", 0);
   int epos = aoapp_name.find(".L", 0);

   if(spos < 0 || epos < 0)
   {
      std::cerr << "Can't create aoapp_name.\n";
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Can't create aoapp_name.");
      throw AOException("Fatal: cannot create name of filterwheel AOApp.");
   }
   
   aoapp_name.erase(spos, epos-spos);

   std::cerr << "aoapp_name set to: " << aoapp_name << "\n";
   readFilterConfig();

   //Set up the com_paths
   try
   {
      pathtmp = (std::string)((ConfigDictionary()))["com_path"];
   }
   catch(Config_File_Exception)
   {
      pathtmp = "fifos";
   }
   com_path = std::string(getenv("VISAO_ROOT")) + "/" + pathtmp + "/" + MyName();

   pause_time = 1.; //Hard coded because it corresponds to the adopt DoFSM loop.

   //Set up the VisAO status board
   try
   {
      statusboard_shmemkey = (ConfigDictionary())["statusboard_shmemkey"];
   }
   catch(Config_File_Exception)
   {
      statusboard_shmemkey = 0;
   }

   if(statusboard_shmemkey)
   {
      if(create_statusboard(sizeof(filterwheel_status_board)) != 0)
      {
         statusboard_shmemptr = 0;
         _logger->log(Logger::LOG_LEV_ERROR, "Could not create status board.");
      }
      else
      {
         VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;
         strncpy(bsb->appname, MyFullName().c_str(), 25);
         bsb->max_update_interval = pause_time;

         VisAO::filterwheel_status_board * fwsb = (VisAO::filterwheel_status_board *) statusboard_shmemptr;
         fwsb->pos = -99999;
         strncpy(fwsb->filter_name, "unknown", 50);
         fwsb->type = -1;
      }
   }

   setup_baseApp(0, 1, 1, 0, false); //Only make the local and script fifos.

   signal(SIGIO, SIG_IGN);
   
   
   
}

void FilterWheelLocal::readFilterConfig()
{
   Config_File*    adopt_cfg;
   int dotpos = aoapp_name.find(".L", 0);

   if(dotpos < 0)
   {
      throw AOException("Fatal: cannot create name of filterwheel config file.");
   }
   std::cerr << aoapp_name.substr(0, dotpos) << "\n";   
   try
   {
      adopt_cfg = new Config_File(Utils::getConffile(aoapp_name.substr(0, dotpos)));
   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
      throw AOException("Fatal: cannot find adopt filter wheel configuration.");
   }

   try
   {
      _abortPos = (double)(*adopt_cfg)["AbortPosition"];
      std::cout << "_abortPos " << _abortPos << "\n";
      _homingPos = (double)(*adopt_cfg)["HomingPosition"];
      std::cout << "_homingPos " << _homingPos << "\n";
      _startingPos = (double)(*adopt_cfg)["StartingPos"];
      std::cout << "_startingPos " << _startingPos << "\n";
      
   }
   catch(Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
      throw AOException("Fatal: cannot find required config values.");
   }

   // Read custom position list, if present.
   _customPos.clear();
   int numCustomPos=0;
   try
   {
      numCustomPos = (*adopt_cfg)["customPositionNum"];
   }
   catch (Config_File_Exception &e)
   {
      _logger->log( Logger::LOG_LEV_INFO, "No custom positions defined in cfg file.");
   }
  
   if (numCustomPos >0)
   {
     for (int i=0; i<numCustomPos; i++)
     {
         ostringstream namekey, poskey;
         namekey << "pos" << i << "_name";
         poskey << "pos" << i << "_pos";
         string name;
         float pos;
         try
         {
            name = (std::string) (*adopt_cfg)[namekey.str()];
            pos  = (float)(*adopt_cfg)[poskey.str()];
            _customPos[name] = pos;
         }
         catch (Config_File_Exception &e)
         {
            _logger->log( Logger::LOG_LEV_ERROR, "Custom position %d not found in cfg file.", i);
         }
     }
   }

   //Read custom types from list, if present.
   _customTypes.clear();
   int numCustomTypes=0;
   try
   {
      numCustomTypes = (*adopt_cfg)["customPositionNum"];
   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_INFO, "No custom positions defined in cfg file.");
   }
   
   if (numCustomTypes >0)
   {
      std::cout << "Loading " << numCustomTypes << " custom types.\n";

      for (int i=0; i<numCustomTypes; i++)
      {
         ostringstream typekey, poskey;
         typekey << "pos" << i << "_type";
         poskey << "pos" << i << "_pos";
         int type;
         float pos;
         try
         {
            type = (int) (*adopt_cfg)[typekey.str()];
            pos  = (float) (*adopt_cfg)[poskey.str()];
            _customTypes.insert(std::pair<int, float>(type, pos));
         }
         catch (Config_File_Exception &e)
         {
            Logger::get()->log( Logger::LOG_LEV_ERROR, "Custom type for %d not found in cfg file.", i);
         }
      }
   }

}
   
void FilterWheelLocal::SetupVars()
{


   //Add the control mode variables to RTDB.
   try
   {
      var_status = RTDBvar ( aoapp_name, "STATUS", NO_DIR, INT_VARIABLE, 1, 1);
      var_cmode_cur = RTDBvar( aoapp_name, "ConMode", CUR_VAR, INT_VARIABLE, 1,1);
      var_cmode_cur.Get(&control_mode);
      var_cmode_req = RTDBvar( aoapp_name, "ConMode", REQ_VAR, INT_VARIABLE, 1,1);
      var_pos_cur = RTDBvar( aoapp_name, "POS", CUR_VAR, REAL_VARIABLE, 1,1);
      var_pos_req = RTDBvar( aoapp_name, "POS", REQ_VAR, REAL_VARIABLE, 1);
      var_pos_local_req = RTDBvar( aoapp_name, "POS_LOCAL", REQ_VAR, REAL_VARIABLE, 1,1);
      var_pos_script_req = RTDBvar( aoapp_name, "POS_SCRIPT", REQ_VAR, REAL_VARIABLE, 1,1);
      
      
      
   }
   catch (AOVarException &e)
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }

   //Reset the RTDB notifier to our handler.
   this->Notify( var_status, StatusChanged);
   this->Notify(var_cmode_cur, CModeCurChanged);
   this->Notify(var_pos_cur, PosCurChanged);
   this->Notify(var_pos_req, PosReqChanged);
}

std::string FilterWheelLocal::GetPosName(double pos)
{
   double np, ul, ll;
   std::string name;

   pos = fmod(pos, _customPos.size());
   if(pos < 0) pos += _customPos.size();

   cpos_it = _customPos.begin();
   while(cpos_it != _customPos.end())
   {
      name = (*cpos_it).first;
      np = (*cpos_it).second;
      ll = fmod(np-.25, _customPos.size());
      if(ll < 0) ll += _customPos.size();
      ul = fmod(np+.25, _customPos.size());
      if(ul < 0) ul += _customPos.size();
      if((pos > ll && pos < ul) || (ul-ll < 0 && (pos < ul || pos > ll)))
      {
         return name;
         break;
      }

      cpos_it++;
   }

   return "intermediate";
}

int FilterWheelLocal::GetType(double pos)
{
   
   double np, ul, ll;
   int type;
   
   pos = fmod(pos, _customTypes.size());
   if(pos < 0) pos += _customTypes.size();
   
   ctype_it = _customTypes.begin();
   while(ctype_it != _customTypes.end())
   {
      type = (*ctype_it).first;
      np = (*ctype_it).second;
      ll = fmod(np-.25, _customTypes.size());
      if(ll < 0) ll += _customTypes.size();
      ul = fmod(np+.25, _customTypes.size());
      if(ul < 0) ul += _customTypes.size();
      if((pos > ll && pos < ul) || (ul-ll < 0 && (pos < ul || pos > ll)))
      {
         //std::cout << type << "\n"; 
         return type;
         break;
      }
      
      ctype_it++;
   }
   
   return 0;
}


void FilterWheelLocal::Run()
{

   //Startup the I/O signal handling thread
   if(start_signal_catcher(true) != 0)
   {
      _logger->log( Logger::LOG_LEV_FATAL, "Error starting signal thread.");
      SetTimeToDie(true);
      return;
   }

   sigset_t set;
   
   sigemptyset(&set);
   sigaddset(&set, SIGIO);
   sigaddset(&set, RTSIGIO);
   
   if(pthread_sigmask(SIG_BLOCK, &set, 0) != 0)
   {
      _logger->log( Logger::LOG_LEV_FATAL, "Error blocking SIGIO in main thread.");
      SetTimeToDie(true);
      return;
   }

   try
   {
      while(!TimeToDie()) DoFSM();
   }
   catch(...)
   {
      std::cerr << "Main thread: exception caught." << std::endl;
   }

   std::cerr << "Main thread exiting." << std::endl;
}

int FilterWheelLocal::DoFSM()
{

   sleep(1);
   //update_statusboard();

   return 0;
}

void FilterWheelLocal::updatePos(double pos)
{
   //std::cout << "cur_pos = " << pos << std::endl;
   cur_pos = pos;
   
}

void FilterWheelLocal::updateReq(double pos)
{
   //std::cout << "req_pos = " << pos << std::endl;
   req_pos = pos;

   
}

void FilterWheelLocal::set_control_mode(int cmode)
{
   control_mode = cmode;
}

void FilterWheelLocal::set_status(int stat)
{
   cur_state = stat;
}

void FilterWheelLocal::signal_catcher()
{
   double dt, t0;
   
   if(connect_fifo_list() == 0)
   {
      logss.clear();
      logss << "fifo_list connected.";
      log_msg(Logger::LOG_LEV_INFO, logss.str());
   }
   else
   {
      error_report(Logger::LOG_LEV_FATAL, "Error connecting the fifo list.");
      SetTimeToDie(true);
      return;
   }

   global_fifo_list = &fl;
   
   setup_RTsigio();


   while(!TimeToDie())
   {
      try
      {
         t0 = get_curr_time();
    
         while((dt = get_curr_time() - t0) < pause_time && !TimeToDie())
         {
            check_fifo_list_RTpending();
            usleep((int) 100);
         }
         check_fifo_list_RTpending();
         update_statusboard();
      }
      catch(AOVarException e)
      {
         need_restart = 1;
         SetTimeToDie(true);
         std::cerr << "Fifo thread: exception caught. " << __LINE__ << " " << e.what() << std::endl;
      }
      catch(...)
      {
         std::cerr << "Fifo thread: exception caught. " << __LINE__ << std::endl;
      }
   }
   std::cerr << "Fifo thread exiting." << std::endl;
}

void * __fwstart_signal_catcher(void * ptr)
{
   ((FilterWheelLocal *) ptr)->signal_catcher();
   return 0;
}

int FilterWheelLocal::start_signal_catcher(bool inherit_sched)
{
   struct sched_param schedpar;
   pthread_attr_t attr;
   
   pthread_attr_init(&attr);
   
   if(!inherit_sched)
   {
      //Start the signal catcher as a lower priority thread.
      
      schedpar.sched_priority = 0;

      pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
      pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
      pthread_attr_setschedparam(&attr, &schedpar);      
   }
   else
   {
      //Start with inherit sched, use the same priority as the main thread
      pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
   }
      
   //This is weird, but the doc for pthread_create says it returns an "error number" instead of -1
   if(pthread_create(&signal_thread, &attr, &__fwstart_signal_catcher, (void *) this) == 0) return 0;
   else return -1;

}

int FilterWheelLocal::request_control(int cmode)
{
   return request_control(cmode, 0);
}

int FilterWheelLocal::request_control(int cmode, bool override)
{
   std::cout << "Requesting control\n";

   try
   {
      if(override) cmode *= 10;
      var_cmode_req.Set(cmode, 0, FORCE_SEND);
      return cmode;
   }
   catch(AOVarException e)
   {
      std::cerr << "request_control: exception caught " << __LINE__ << " " << e.what() << std::endl;
      need_restart = 1;
      SetTimeToDie(true);
      return -1;
   }
  
}

int FilterWheelLocal::CModeCurChanged(void *pt, Variable *msgb)
{
   //std::cout << "In CModeReqChanged\n";
   FilterWheelLocal * vmc = (FilterWheelLocal *) pt;
   
   vmc->set_control_mode(msgb->Value.Lv[0]);

   return NO_ERROR;
}

int FilterWheelLocal::PosCurChanged( void *pt, Variable *var)
{
   //std::cout <<  "In PosCurChanged\n";
   
   FilterWheelLocal * vmc = (FilterWheelLocal *) pt;
   /***** Don't lock the mutex!  it will get locked by PosReqChanged *******/

   vmc->updatePos(var->Value.Dv[0]);
   
   return NO_ERROR;
}

int FilterWheelLocal::PosReqChanged( void *pt, Variable *var)
{
   //std::cout <<  "In PosCurChanged\n";
   
   FilterWheelLocal * vmc = (FilterWheelLocal *) pt;
   /***** Don't lock the mutex!  it will get locked by PosReqChanged *******/

   vmc->updateReq(var->Value.Dv[0]);
   
   return NO_ERROR;
}


int FilterWheelLocal::StatusChanged(void *pt, Variable *var)
{
   //std::cout <<  "In StatusCurChanged\n";
   
   FilterWheelLocal * vmc = (FilterWheelLocal *) pt;
   /***** Don't lock the mutex!  it will get locked by PosReqChanged *******/

   vmc->set_status(var->Value.Lv[0]);
   
   return NO_ERROR;
}

int FilterWheelLocal::ChangePos(double pos)
{

   std::cout << "Changing Pos\n";

   try
   {
   if(pos == getAbortPos())
   {
      var_pos_local_req.Set(pos, 0, FORCE_SEND);
      return 0;
   }

   if(control_mode == CMODE_LOCAL)
   {
      var_pos_local_req.Set(pos, 0, FORCE_SEND);
      updateReq(pos);
      return 0;
   }

   if(control_mode == CMODE_SCRIPT)
   {
      var_pos_script_req.Set(pos, 0, FORCE_SEND);
      updateReq(pos);
      return 0;
   }
   }
   catch(AOVarException e)
   {
      std::cerr << "ChangePos: exception caught " << " " << __LINE__ << " " << e.what() << std::endl;
      SetTimeToDie(true);
      need_restart = 1;
      
      return -1;
   }

   return -1;
}



std::string FilterWheelLocal::remote_command(std::string com)
{
   std::string rstr;

   _logger->log(Logger::LOG_LEV_TRACE, "Received remote command: %s.", com.c_str());

   rstr = common_command(com, CMODE_REMOTE);

   if(rstr == "") rstr = "UNKOWN COMMAND: " + com + "\n";

   _logger->log(Logger::LOG_LEV_TRACE, "Response to remote command: %s.", rstr.c_str());

   return rstr;
}

std::string FilterWheelLocal::local_command(std::string com)
{
   std::string rstr;

   _logger->log(Logger::LOG_LEV_TRACE, "Received local command: %s.", com.c_str());

   rstr = common_command(com, CMODE_LOCAL);

   if(rstr == "") rstr = "UNKOWN COMMAND: " + com + "\n";

   _logger->log(Logger::LOG_LEV_TRACE, "Response to local command: %s.", rstr.c_str());

   return rstr;
}

std::string FilterWheelLocal::script_command(std::string com)
{
   std::string rstr;

   _logger->log(Logger::LOG_LEV_TRACE, "Received script command: %s.", com.c_str());

   rstr = common_command(com, CMODE_SCRIPT);

   if(rstr == "") rstr = "UNKOWN COMMAND: " + com + "\n";

   _logger->log(Logger::LOG_LEV_TRACE, "Response to script command: %s.", rstr.c_str());

   return rstr;
}

std::string FilterWheelLocal::get_state_str()
{
   char statestr[100];

   int ismov = 0, ishom = 0;
   std::string filt;
   int status;

   status = cur_state;

   std::string str = control_mode_response();

//    if(status == STATE_READY || status == STATE_OPERATING || status == STATE_HOMING)
//    {
// 
//       //motor->var_pos_cur.Get(&cur_pos);
//       ismov = 0;motor->IsMoving();
//       //ishom = ((VisAOFilterWheel*)motor)->IsHoming();
//    }
//    else
//    {
//       ismov = 0;
//       ishom = 0;
//    }


   filt = GetPosName(cur_pos);

   snprintf(statestr, 100, "%c %2i %i %i %09.4f %s\n", str[0], status, ismov, ishom, cur_pos, filt.c_str());

   return statestr;
}

std::string FilterWheelLocal::common_command(std::string com, int calling_ctype)
{
   int rv;
   char rvstr[50];
   
   int pos = com.find_first_of("\n\r", 0);
   if(pos > -1) com.erase(pos, com.length()-pos);
   
   if(com == "state?")
   {
      return get_state_str();
   }
   if(com == "abort")
   {
      //Can always abort, from anywhere.
      rv = ChangePos(_abortPos);
      snprintf(rvstr, 5, "%i\n", rv);
      return rvstr;
   }
   if(com == "home")
   {
      if(control_mode == calling_ctype)
      {
         rv = ChangePos(_homingPos);
         snprintf(rvstr, 5, "%i\n", rv);
         return rvstr;
      }
      else
      {
         return control_mode_response();
      }
   }
   if(com == "pos?")
   {
      snprintf(rvstr, 50, "%0.3f\n", cur_pos );
      return rvstr;
   }

   if(com == "moving?")
   {
      int ismov = 0;
      if(cur_state == STATE_OPERATING || cur_state == STATE_HOMING)
      {
         
         ismov = 1;
         
      }
      snprintf(rvstr, 50, "%i\n", ismov);
      return rvstr;
   }

   if(com == "homing?")
   {
      int ishom = 0;
      if(cur_state == STATE_HOMING)
      {
         
         ishom = 1;
      }
      snprintf(rvstr, 50, "%i\n", ishom);
      return rvstr;
   }

   if(com == "filter?")
   {
      return GetPosName(cur_pos);
   }
   
   if(com.substr(0,3) == "pos")
   {
      std::cout << "pos " << control_mode << "\n";
      if(control_mode == calling_ctype)
      {
         double pos = strtod(com.substr(3, com.size()-3).c_str(), 0);
         std::cout << "Here we go\n";
         rv = ChangePos(pos);
         snprintf(rvstr, 5, "%i\n", rv);
         return rvstr;
      }
      else
      {
         return control_mode_response();
      }
   }

   if(com.substr(0,6) == "filter")
   {
      if(control_mode == calling_ctype)
      {
         int idx = com.find_first_not_of(' ', 6);
         std::string filt = com.substr(idx, com.size()-idx);
         double pos = _customPos[filt];
         //std::cout << "Moving to filter " << filt << " at " << pos << "\n";
         rv = ChangePos(pos);
         snprintf(rvstr, 5, "%i\n", rv);
         return rvstr;
      }
      else
      {
         return control_mode_response();
      }
   }

   return "";
}

int FilterWheelLocal::update_statusboard()
{
   
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();

      VisAO::filterwheel_status_board * fwsb = (VisAO::filterwheel_status_board *) statusboard_shmemptr;

      int status = cur_state;

      fwsb->state = status;
      if(status == STATE_READY || status == STATE_OPERATING || status == STATE_HOMING)
      {
            var_pos_cur.Update();
            var_pos_cur.Get(&cur_pos);
            
            fwsb->pos = cur_pos;
            
            //Update requested position 
            if(cur_state == STATE_HOMING)
            {
               std::cout << "STATE_HOMING\n";
               //If homing was requested, then the home position should be sent
               //Could also be homing without having a request (at startup)
               fwsb->req_pos = _startingPos;
            }
            else if(cur_state == STATE_OPERATING)
            {
               std::cout << "STATE_OPERATING\n";
               fwsb->req_pos = req_pos;
            }
            else
            {
               std::cout << "NOT MOVING\n";
               fwsb->req_pos = cur_pos;
            }
//             else if (req_pos == _abortPos)
//             {
//                //If aborted, then current position is the requested position.
//                fwsb->req_pos = cur_pos;
//             }
//             else
//             {
//                fwsb->req_pos = req_pos;
//             }

            strncpy(fwsb->filter_name, GetPosName(fwsb->pos).c_str(), 50);
            fwsb->type = GetType(fwsb->pos);
            fwsb->req_type = GetType(fwsb->req_pos);
            //std::cout << "rt= " << fwsb->req_type << std::endl;
      }
   }
   return 0;
}


} //namespace VisAO

