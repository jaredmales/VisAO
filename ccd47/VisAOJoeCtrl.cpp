/************************************************************
*    VisAOJoeCtrl.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for CCD47 control from the AO Supervisor.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOJoeCtrl.cpp
  * \author Jared R. Males
  * \brief Definitions for CCD47 control from the AO Supervisor. 
  * 
  * Based on the Arcetri JoeCtrl class.
  * 
*/


#include "VisAOJoeCtrl.h"


extern int debug;
//debug=0;


namespace VisAO
{

#define JOE_ADDR_LEN            15                      // Length of idrive string address
#define ERRMSG_LEN              32                      // Length of an error message

int cur_xbin, cur_ybin, cur_speed;
int default_xbin, default_ybin, default_speed, default_black;

JoeCtrl::JoeCtrl( std::string name, const std::string &conffile) throw (AOException) : AOApp(name, conffile)
{
   Create();
}

JoeCtrl::JoeCtrl( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   std::cout << "Starting" << std::endl;
   Create();
}

void JoeCtrl::Create() throw (AOException)
{

   try 
   {
      _ccdName  = (std::string) ConfigDictionary()["ccdName"]; 
      //_ccdNetAddr = (std::string) ConfigDictionary()["ccdNetAddr"];
      //_ccdNetPort = ConfigDictionary()["ccdNetPort"];
      _ccdDx    = ConfigDictionary()["ccdXdim"];
      _ccdDy    = ConfigDictionary()["ccdYdim"];
      _ccdDefaultXbin = ConfigDictionary()["ccdDefaultXbin"];
      _ccdDefaultYbin = ConfigDictionary()["ccdDefaultYbin"];
      _ccdDefaultSpeed = ConfigDictionary()["ccdDefaultSpeed"];
      _ccdDefaultBlack = ConfigDictionary()["ccdDefaultBlack"];
      _ccdBlacksNum    = ConfigDictionary()["ccdBlacksNum"];
      _minRep          = ConfigDictionary()["minRep"];
      _maxRep          = ConfigDictionary()["maxRep"];
      _maxNumSpeeds    = ConfigDictionary()["maxNumSpeeds"];
      _maxNumBins      = ConfigDictionary()["maxNumBins"];
      _maxNumWins      = ConfigDictionary()["maxNumWins"];
      //_filePrefix      = (std::string) ConfigDictionary()["filePrefix"];
      _startProgramSet = ConfigDictionary()["startProgramSet"];
      _fanCtrlActive   = ConfigDictionary()["fanCtrlActive"];

      wait_to = ConfigDictionary()["wait_to"];
      if (_fanCtrlActive) 
      {
         _fanOnTemp       = ConfigDictionary()["fanOnTemp"];
         _fanOffTemp      = ConfigDictionary()["fanOffTemp"];
      }
   } 
   catch (Config_File_Exception &e) 
   {
      _logger->log( Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
      throw AOException("Fatal: Missing configuration data");
   }

   _ccdBlacks.resize( _ccdBlacksNum);

   //_tempsLogger = Logger::get( _ccdName+"Temperature", Logger::LOG_LEV_INFO, "TELEMETRY");
    
   // Create the specific LittleJoe arrays
   LoadJoeDiskFiles();
   EraseLocalMemory();

   setup_fifo_list(1);

   std::string ccd47_fifo_in = "/home/aosup/visao/fifos/ccd47ctrl_com_remote_in";
   std::string ccd47_fifo_out = "/home/aosup/visao/fifos/ccd47ctrl_com_remote_out";

   set_fifo_list_channel(&fl, 0, RWBUFF_SZ, ccd47_fifo_out.c_str(), ccd47_fifo_in.c_str(), 0, 0);

   signal(SIGIO, SIG_IGN);
   connect_fifo_list();

   global_fifo_list = &fl;

 
}


void JoeCtrl::PostInit()
{
   // ----------- Initial status
   setCurState(STATE_NOCONNECTION);
   var_errmsg.Set("Starting up");
}

void JoeCtrl::SetupVars()
{
   try 
   {
      var_name = RTDBvar( MyFullName(), "NAME", NO_DIR, CHAR_VARIABLE, _ccdName.size()+1);
      var_name.Set(_ccdName);


      var_enable_req = RTDBvar( MyFullName(), "ENABLE", REQ_VAR);
      var_enable_cur = RTDBvar( MyFullName(), "ENABLE", CUR_VAR);

      var_enable_cur.Set(0);

      var_errmsg = RTDBvar( MyFullName(), "ERRMSG", NO_DIR, CHAR_VARIABLE, ERRMSG_LEN);

      var_dx = RTDBvar( MyFullName(), "DX");
      var_dx_cur = RTDBvar( MyFullName(), "DX", CUR_VAR);
      var_dx_req = RTDBvar( MyFullName(), "DX", REQ_VAR);
      var_dy = RTDBvar( MyFullName(), "DY");
      var_dy_cur = RTDBvar( MyFullName(), "DY", CUR_VAR);
      var_dy_req = RTDBvar( MyFullName(), "DY", REQ_VAR);
      var_windowxs = RTDBvar( MyFullName(), "WINDOWXS", NO_DIR, INT_VARIABLE, _maxNumWins);
      var_windowys = RTDBvar( MyFullName(), "WINDOWYS", NO_DIR, INT_VARIABLE, _maxNumWins);

      var_xbins = RTDBvar( MyFullName(), "XBINS", NO_DIR, INT_VARIABLE, _maxNumBins);
      var_ybins = RTDBvar( MyFullName(), "YBINS", NO_DIR, INT_VARIABLE, _maxNumBins);
      var_speeds = RTDBvar( MyFullName(), "SPEEDS", NO_DIR, INT_VARIABLE, _maxNumSpeeds);

      var_xbin_req = RTDBvar( MyFullName(), "XBIN", REQ_VAR);
      var_ybin_req = RTDBvar( MyFullName(), "YBIN", REQ_VAR);
      var_xbin_cur = RTDBvar( MyFullName(), "XBIN", CUR_VAR);
      var_ybin_cur = RTDBvar( MyFullName(), "YBIN", CUR_VAR);

      var_speed_cur = RTDBvar( MyFullName(), "SPEED", CUR_VAR);
      var_speed_req = RTDBvar( MyFullName(), "SPEED", REQ_VAR);

      var_black_cur = RTDBvar( MyFullName(), "BLACK", CUR_VAR, INT_VARIABLE, _ccdBlacks.size());
      var_black_req = RTDBvar( MyFullName(), "BLACK", REQ_VAR, INT_VARIABLE, _ccdBlacks.size());

      var_framerate_cur = RTDBvar( MyFullName(), "FRMRT", CUR_VAR, REAL_VARIABLE);
      var_framerate_req = RTDBvar( MyFullName(), "FRMRT", REQ_VAR, REAL_VARIABLE);

      var_rep_cur = RTDBvar( MyFullName(), "REP", CUR_VAR);
      var_rep_req = RTDBvar( MyFullName(), "REP", REQ_VAR);

      var_gain_cur = RTDBvar( MyFullName(), "GAIN", CUR_VAR, INT_VARIABLE);
      var_gain_req = RTDBvar( MyFullName(), "GAIN", REQ_VAR, INT_VARIABLE);

      var_temps = RTDBvar( MyFullName(), "TEMPS", NO_DIR, INT_VARIABLE, 3);

      var_fanReq = RTDBvar( (std::string)ConfigDictionary()["fanReqVar"], INT_VARIABLE, 1, false);

      var_dx.Set( _ccdDx);
      var_dy.Set( _ccdDy);
      var_dx_cur.Set( _ccdDx);
      var_dy_cur.Set( _ccdDy);

      // Fill the XBINS and YBINS array up to the maximum length
      unsigned i;
      vector<int> binning;
      for ( i=0; i < _ccdXbins.size(); i++)
         if(_ccdXbins[i] > 0)  binning.push_back( _ccdXbins[i]);
      for ( i =  _ccdXbins.size(); i<  (unsigned) _maxNumBins; i++)
         binning.push_back(-1);
      var_xbins.Set( binning);

      binning.clear();
      for ( i=0; i < _ccdYbins.size(); i++)
         if(_ccdYbins[i] > 0) binning.push_back( _ccdYbins[i]);
      for ( i =  _ccdYbins.size(); i<  (unsigned)_maxNumBins; i++)
         binning.push_back(-1);

      var_ybins.Set( binning);

      Notify( var_enable_req, EnableReqChanged);
      Notify( var_xbin_req, XbinReqChanged);
      Notify( var_ybin_req, YbinReqChanged);
      Notify( var_speed_req, SpeedReqChanged);
      Notify( var_black_req, BlackReqChanged);
      Notify( var_framerate_req, FrameRateReqChanged);
      Notify( var_rep_req, RepReqChanged);
      Notify( var_gain_req, GainReqChanged);

      setCurState(STATE_NOCONNECTION);

      var_cmode_cur = RTDBvar( MyFullName(), "ConMode", CUR_VAR, INT_VARIABLE, 1,1);
      var_cmode_req = RTDBvar( MyFullName(), "ConMode", REQ_VAR, INT_VARIABLE, 1,1);

      Notify( var_cmode_req, CModeReqChanged);

      var_preset_req = RTDBvar( MyFullName(), "Preset", REQ_VAR, INT_VARIABLE, 1,1);
      Notify( var_preset_req, PresetReqChanged);
      
   }
   catch (AOVarException &e) 
   {
      _logger->log(Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }

   
}

void JoeCtrl::StateChange( int oldstate, int state)
{
   if ((state == STATE_NOCONNECTION) && (oldstate != STATE_NOCONNECTION))
         {
         var_enable_cur.Set( 0,0, FORCE_SEND);
         var_xbin_cur.Set( 0, 0, FORCE_SEND);
         var_ybin_cur.Set( 0, 0, FORCE_SEND);
         var_speed_cur.Set( 0, 0, FORCE_SEND);
         //var_framerate_cur.Set( 0, 0, FORCE_SEND);
         var_rep_cur.Set( 0, 0, FORCE_SEND);

         for (unsigned int i=0; i< _ccdBlacks.size(); i++)
            var_black_cur.Set( 0, i, NO_SEND);
         var_black_cur.Send();
         }
}

void JoeCtrl::Run()
{
   while(!TimeToDie())
   {
      try 
      {
         // Do iterative step of the controller

         DoFSM();

         // Read temperatures every now and then
         //if (iterations++ % 5 == 0)
           //       ReadJoeTemps();
      }
      catch (AOException &e) 
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception: %s", e.what().c_str());
      }
   }
   exit(0);
}

int JoeCtrl::DoFSM(void)
{
   int status;
   int stat = NO_ERROR;

   status = getCurState();

   if (TestCCD47CtrlLink() != NO_ERROR)
   {
      //setCurState(STATE_NOCONNECTION);
      EraseLocalMemory();
   }

   status = getCurState();

   switch(status)
   {
      // At first, try to start network
      case STATE_NOCONNECTION:
         msleep(1000);
         break;

       // After network start, configure LittleJoe
       // Configuration ensures at the end that LittleJoe is turned off
      // When ready to go, do nothing
      case STATE_CONNECTED:
      case STATE_OFF:
      case STATE_CONFIGURING:
      case STATE_READY:
      case STATE_OPERATING:
         msleep(1000);
         break;

      // Unknown states should not exist
      default:
         msleep(1000);
         break;
   }

   // Always set state again (for watchdogs)
   setCurState(getCurState());

   // Return the generated error code, if any
   return stat;
}




int JoeCtrl::TestCCD47CtrlLink()
{
   try
   {
      //std::cout << "Testing CCD47 Link\n";
      std::string resp = send_ccd47_command("state?");

      //std::cout << "Got: " << resp << "\n";

      if(resp == "")
      {
         setCurState(STATE_NOCONNECTION);
         //Ok, we timed out.  So flush the fifo channel
         sleep(1);
         read_fifo_channel(&fl.fifo_ch[0]);
         return -1;
      }

      int cmode;

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
            //We got garbage, try flushing the fifo channel.
            sleep(1);
            read_fifo_channel(&fl.fifo_ch[0]);
            break;
      }
      var_cmode_cur.Set(cmode, 0, CHECK_SEND);

      if(cmode == -1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }

      if(resp[2] == '3')
      {
         setCurState(STATE_CONFIGURING);
         return NO_ERROR;
      }

      if(resp[2] == '0')
      {
         setCurState(STATE_NOCONNECTION);
         return NO_ERROR;
      }

      if(resp[2] == '1')
      {
         setCurState(STATE_READY);
         var_enable_cur.Set(0, 0, CHECK_SEND);
      }

      if(resp[2] == '2')
      {
         setCurState(STATE_OPERATING);
         var_enable_cur.Set(1, 0, CHECK_SEND);
      }

      //If connected get the speed.
      write_fifo_channel(0, "set?\n", 5, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      int program_set = cur_ProgramSet;
      cur_ProgramSet = atoi(resp.c_str());
      if(program_set != cur_ProgramSet) UpdateJoeMemory(cur_ProgramSet);

      write_fifo_channel(0, "prog?\n", 7, &resp);

      if(resp == "" || resp.length() < 1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      cur_Program = atoi(resp.c_str());

      write_fifo_channel(0, "speed?\n", 8, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      int speed;
      speed = atoi(resp.c_str());
      var_speed_cur.Set(speed, 0, CHECK_SEND);

      write_fifo_channel(0, "xbin?\n", 7, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      int xbin = atoi(resp.c_str());
      var_xbin_cur.Set(xbin, 0, CHECK_SEND);

      write_fifo_channel(0, "ybin?\n", 7, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      int ybin = atoi(resp.c_str());
      var_ybin_cur.Set(ybin, 0, CHECK_SEND);

      write_fifo_channel(0, "windowx?\n", 9, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      int xwin = atoi(resp.c_str());
      var_dx.Set(xwin, 0, CHECK_SEND);
      var_dx_cur.Set(xwin, 0, CHECK_SEND);

      write_fifo_channel(0, "windowy?\n", 9, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      int ywin = atoi(resp.c_str());
      var_dy.Set(ywin,0,CHECK_SEND);
      var_dy_cur.Set(ywin, 0, CHECK_SEND);

      write_fifo_channel(0, "rep?\n", 6, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      int reps = atoi(resp.c_str());
      var_rep_cur.Set(reps, 0, CHECK_SEND);

      write_fifo_channel(0, "gain?\n", 7, &resp);
      if(resp == "" || resp.length() < 1)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      int gain = atoi(resp.c_str());
      var_gain_cur.Set(gain, 0, CHECK_SEND);

      ExposeSpeeds();
      ExposeWindows();
      ComputeFramerate();

      //If connected, get temperatures
      int temps[3];
      double t1, t2, t3;

      write_fifo_channel(0, "temps?\n", 8, &resp);
      if(resp == "" || resp.length() < 19)
      {
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      t1 = strtod(resp.substr(0,6).c_str(),0);
      t2 = strtod(resp.substr(7,6).c_str(),0);
      t3 = strtod(resp.substr(14,6).c_str(),0);

      temps[0] = (int) t1; 
      temps[1] = (int) t2; 
      temps[2] = (int) t3; 

      var_temps.Set(temps, CHECK_SEND);

      write_fifo_channel(0, "blacks?\n", 8, &resp);
      if(resp == "" || resp.length() < 9)
      {
         std::cout << resp << "\n";
         setCurState(STATE_NOCONNECTION);
         return -1;
      }
      int blacks[2];
      //std::cout << "resp:" << resp << "|\n";
      blacks[0] = atoi(resp.substr(0,4).c_str());
      //std::cout << "1\n";
      blacks[1] = atoi(resp.substr(5,4).c_str());
      var_black_cur.Set(blacks, CHECK_SEND);

      // Fan management
      if (_fanCtrlActive)
      {
         if (t1< _fanOffTemp)
         {
            var_fanReq.Set(0, 0, CHECK_SEND);
         }
         if (t1> _fanOnTemp)
         {
            var_fanReq.Set(1, 0, CHECK_SEND);
         }
      }
      return NO_ERROR;
   }
   catch(...)
   {
      setCurState(STATE_NOCONNECTION);
      return -1;
   }
}

int JoeCtrl::Stop()
{
   std::string resp;

   if(write_fifo_channel(0, "stop", 5, &resp) < 0)
   {
      ERROR_REPORT("Error sending stop command to CCD47Ctrl");
      return -1;
   }
   return 0;
   
}

int JoeCtrl::Start()
{
   std::string resp;

   if(write_fifo_channel(0, "start", 6, &resp) < 0)
   {
      ERROR_REPORT("Error sending start command to CCD47Ctrl");
      return -1;
   }
   return 0;
   
}

int JoeCtrl::EnableReqChanged( void *pt, Variable *var)
{
    int enable, enabled, stat;
    JoeCtrl *ctrl = (JoeCtrl *)pt;

    #ifdef _debug
      std::cerr << "EnableReqChanged" << std::endl;
    #endif

    stat = NO_ERROR;
    enable = var->Value.Lv[0];
    ctrl->var_enable_req.Set(enable, 0, NO_SEND);  // Mirror value locally
    ctrl->var_enable_cur.Get(&enabled);      // Current value

    // Ignore invalid values
    if ((enable != 0) && (enable != 1))
            return NO_ERROR;

    // Switch on if requested
    if ((enable == 1) && (enabled == 0))
            stat = ctrl->Start();

    // Switch off if requested
    if ((enable == 0) && (enabled == 1))
            stat = ctrl->Stop();

    //if (stat == NO_ERROR)
       //ctrl->var_enable_cur.Set(enable);
    return stat;
}

int JoeCtrl::XbinReqChanged( void *pt, Variable *var)
{
   #ifdef _debug
      std::cerr << "XbinReqChanged" << std::endl;
   #endif
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   ctrl->var_xbin_req.Set( var->Value.Lv[0], 0, NO_SEND);
   return ctrl->ReprogramJoe();
}

int JoeCtrl::YbinReqChanged( void *pt, Variable *var)
{
   #ifdef _debug
      std::cerr << "YbinReqChanged" << std::endl;
   #endif
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   ctrl->var_ybin_req.Set( var->Value.Lv[0], 0, NO_SEND);
   return ctrl->ReprogramJoe();
}

int JoeCtrl::SpeedReqChanged( void *pt, Variable *var)
{
   #ifdef _debug
      std::cerr << "SpeedReqChanged" << std::endl;
   #endif
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   ctrl->var_speed_req.Set( var->Value.Lv[0], 0, NO_SEND);
   return ctrl->ReprogramJoe();
}

        // --------------------------------------
        // Black levels (CCDnBLACKS variable)
        //

int JoeCtrl::BlackReqChanged( void *pt, Variable *var)
{
   #ifdef _debug
      std::cerr << "BlackReqChanged" << std::endl;
   #endif
   
   int black,cur_black,stat;
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   unsigned int i;
   char cmstr[50];
   // Set the new black levels, if different from the current ones
   for (i=0; i< ctrl->_ccdBlacks.size(); i++)
   {
         black = var->Value.Lv[i];
         ctrl->var_black_cur.Get(i, &cur_black);

         Logger::get()->log( Logger::LOG_LEV_DEBUG, "Setting black level %d to %d", i, black);
         if (black != cur_black)
         {
            snprintf(cmstr, 50, "black %d %d", i, black);
            std::cout << cmstr << "\n";
            ctrl->send_ccd47_command(cmstr);
            sleep(1);

         }
         //ctrl->var_black_cur.Set( black, (int)i,FORCE_SEND);
   }

   var = 0;
   return NO_ERROR;
}

int JoeCtrl::RepReqChanged( void *pt, Variable *var)
{

   #ifdef _debug
      std::cerr << "RepReqChanged" << std::endl;
   #endif

   int stat;
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   int rep = var->Value.Lv[0];

   if ((rep>= ctrl->_minRep) && (rep<= ctrl->_maxRep))
   {
      stat = 0;
      char cmstr[50];
      snprintf(cmstr, 50, "set %i %i %i %i", ctrl->cur_ProgramSet, ctrl->cur_Program, ctrl->cur_Gain, rep);
      std::cout << cmstr << "\n";
      ctrl->send_ccd47_command(cmstr);
      sleep(1);

      return stat;
   }
   else return JOE_OUT_OF_RANGE_ERROR;

}

int JoeCtrl::GainReqChanged( void *pt, Variable *var)
{

   #ifdef _debug
      std::cerr << "GainReqChanged" << std::endl;
   #endif

   int stat;
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   int gain = var->Value.Lv[0];

   if ((gain>= 0) && (gain<= 3))
   {
      stat = 0;
      char cmstr[50];
      snprintf(cmstr, 50, "set %i %i %i %i", ctrl->cur_ProgramSet, ctrl->cur_Program, gain, ctrl->cur_Reps);
      std::cout << cmstr << "\n";
      ctrl->send_ccd47_command(cmstr);
      sleep(1);

      return stat;
   }
   else return JOE_OUT_OF_RANGE_ERROR;

}


int JoeCtrl::CModeReqChanged(void *pt, Variable *msgb)
{
   std::string rstr;
   int stat, newstate;

   #ifdef _debug
      std::cerr << "CModeReqChanged" << std::endl;
   #endif
   JoeCtrl * jc = (JoeCtrl *) pt;
   
   /*** Don't lock threadMutex here, it is done in request_control ****/
   
   newstate = msgb->Value.Lv[0];
   //std::cout << newstate << "\n";

   if(newstate == 1) rstr = jc->send_ccd47_command("REMOTE");
   if(newstate == 10) rstr = jc->send_ccd47_command("XREMOTE");
   if(newstate == 0) rstr = jc->send_ccd47_command("~REMOTE");
   
   if(rstr == "") stat = -1;
   else stat = 0;

   return stat;
}

int JoeCtrl::PresetReqChanged(void *pt, Variable *msgb)
{
   std::string rstr;
   int stat, newstate;

#ifdef _debug
      std::cerr << "PresetReqChanged" << std::endl;
#endif
   JoeCtrl * jc = (JoeCtrl *) pt;
   
   /*** Don't lock threadMutex here, it is done in request_control ****/
   
   //newstate = msgb->Value.Lv[0];
   //std::cout << newstate << "\n";

   rstr = jc->send_ccd47_command("set 0 0 0 0");
   rstr = jc->send_ccd47_command("serve 1");
//   if(newstate == 10) rstr = jc->send_ccd47_command("XREMOTE");
//   if(newstate == 0) rstr = jc->send_ccd47_command("~REMOTE");
   
   if(rstr == "") stat = -1;
   else stat = 0;

   return stat;
}

std::string JoeCtrl::send_ccd47_command(std::string com)
{
   std::string resp;
   if(write_fifo_channel(0, com.c_str(), com.length()+1, &resp) < 0)
   {
      ERROR_REPORT("Error sending command to CCD47");
   }
   return resp;
}


int JoeCtrl::FrameRateReqChanged( void *pt, Variable *var)
{
   #ifdef _debug
      std::cerr << "FrameRateReqChanged" << std::endl;
   #endif

   JoeCtrl *ctrl = (JoeCtrl *)pt;
   return ctrl->ChangeFramerate( var->Value.Dv[0], 0);
}

//+Function: ChangeFramerate
//
// Changes the framerate selecting the most convenient readout speed
// and repetition number, without changing the binning.
// Updates the CURSPEED, CURREP e CURFRMRT variables.
// Can be called at any time.
//
// If bestspeed is set, it will search for the most convenient readout. Otherwise,
// the readout is not changed.
//-

int JoeCtrl::ChangeFramerate( double framerate, int bestspeed)
{
        int xbin, ybin, cur_speed, cur_winx, cur_winy;
        int found_speed=-1;
        int found_set=-1;
        int found_program=-1;
        unsigned int i,j;

        // Safety check (avoid division by zero and nonsensical framerates)
        if (framerate <=0)
                return VALUE_OUT_OF_RANGE_ERROR;

        var_xbin_cur.Get(&xbin);
        var_ybin_cur.Get(&ybin);
        var_speed_cur.Get(&cur_speed);
        var_dx_cur.Get(&cur_winx);
        var_dy_cur.Get(&cur_winy);

        _logger->log( Logger::LOG_LEV_DEBUG, "Requested frequency: %5.2f", framerate);

        for (i=0; i<ondisk.size(); i++)
                for (j=0; j< ondisk[i].programs.size(); j++)
                        {
                        // Skip programs with the wrong binning or window
                        if ((xbin != ondisk[i].programs[j].binx) ||
                            (ybin != ondisk[i].programs[j].biny) ||
                            (ondisk[i].programs[j].windowx != cur_winx) ||
                            (ondisk[i].programs[j].windowx != cur_winy))
                                continue;

                        // Delays are in microseconds
                        double mintime = ondisk[i].programs[j].delay_base;
                        double maxtime = mintime + ondisk[i].programs[j].delay_inc * 65535;

                        double max_framerate = 1e6/mintime;
                        double min_framerate = 1e6/maxtime;

                        _logger->log( Logger::LOG_LEV_DEBUG, "Program %d %d: max,min = %5.2f, %5.2f", i, j, max_framerate, min_framerate);


                        // Skip programs not able to reach the desired framerate
                        if (bestspeed)
                           if ((framerate < min_framerate) || (framerate > max_framerate))
                                continue;

                        int readout_speed = ondisk[i].programs[j].readout_speed;

                        // Skip programs with a different readout speed from our one (if bestspeed is not set)
                        if ((!bestspeed) && (readout_speed != cur_speed))
                           continue;

                        // Skip programs with a readout speed equal or bigger than the one already found
                        if (found_program>=0)
                                if ( readout_speed >= ondisk[found_set].programs[found_program].readout_speed)
                                        continue;

                        // Program passed all tests
                        found_set = i;
                        found_program = j;
                        found_speed = readout_speed;

                        _logger->log( Logger::LOG_LEV_DEBUG, "Found program: %d %d %d", found_set, found_program, found_speed);
                        }

        // See if we have found one
        if (found_program <0)
                return VALUE_OUT_OF_RANGE_ERROR;

        // Calculate new repetition number
        double base = ondisk[found_set].programs[found_program].delay_base;
        double inc = ondisk[found_set].programs[found_program].delay_inc;
        double goal = 1e6/framerate;

        goal = goal - base;
        goal = goal / inc;

        // May happen if the requested speed is not able to go so fast...
        if (goal<0)
           goal=0;

        // Apply new settings (will call the appropriate handlers in order...)
        //var_speed_req.Set(found_speed, 0, CHECK_SEND);
        //var_rep_req.Set((int)goal, 0, CHECK_SEND);

        _logger->log( Logger::LOG_LEV_DEBUG, "Base %5.2f, inc %5.2f, goal %5.2f", base, inc, goal);
        _logger->log( Logger::LOG_LEV_DEBUG, "Setting speed %d, repetitions %d", found_speed, (int)goal);

        char cmstr[50];
        snprintf(cmstr, 50, "set %i %i %i %i", found_set, found_program, cur_Gain, (int) goal);

         std::cout << cmstr << "\n";
         send_ccd47_command(cmstr);
         sleep(1);

        return NO_ERROR;
}


//+Function: ComputeFramerate
//
// Computes the current framerate from the current state of LittleJoe control variables.
// Refreshes the MsgD-RTDB CCDnCURFRMRT variable.
//-

int JoeCtrl::ComputeFramerate()
{
   std::string resp;

   write_fifo_channel(0, "framerate?\n", 12, &resp);

   if(resp == "" || resp.length() < 1)
   {
      setCurState(STATE_NOCONNECTION);
      return -1;
   }
   double framerate = strtod(resp.c_str(), 0);
   var_framerate_cur.Set(framerate, 0, CHECK_SEND);
   return NO_ERROR;
}

//+Entry: ExposeSpeeds
//
// Fills the CCDxxSPEEDS variable with each possible
// speed of the current binning, as available in the current programset

int JoeCtrl::ExposeSpeeds()
{
   int xbin, ybin;
   unsigned int i;

   var_xbin_cur.Get(&xbin);
   var_ybin_cur.Get(&ybin);

   int *speeds = new int[ _maxNumSpeeds];
   memset( speeds, 0, sizeof(int) * _maxNumSpeeds);

   int counter=0;
   for (i=0; i< memory.programs.size(); i++)
         {
         //_logger->log( Logger::LOG_LEV_DEBUG, "Checking program %dx%d, %d kpixel/sec", memory.programs[i].binx, memory.programs[i].biny,  memory.programs[i].readout_speed);
                        if ((memory.programs[i].binx == xbin) &&
                            (memory.programs[i].biny == ybin) &&
                            (memory.programs[i].windowx == 1024) &&
                            (memory.programs[i].windowy == 1024))
               speeds[ counter++ ] = memory.programs[i].readout_speed;
         }

   var_speeds.Set(speeds);

   delete speeds;
   return NO_ERROR;
}

 //+Entry: ExposeWindows
//
// Fills the CCDxxWINDOWS variable with each possible
// window of the current binning, as available in the current programset

int JoeCtrl::ExposeWindows()
{
   int xbin, ybin;
   unsigned int i;

   var_xbin_cur.Get(&xbin);
   var_ybin_cur.Get(&ybin);

   int *winxs = new int[ _maxNumWins];
   int *winys = new int[ _maxNumWins];
   memset( winxs, 0, sizeof(int) * _maxNumWins);
   memset( winys, 0, sizeof(int) * _maxNumWins);

   int counter=0;
   int found;
   for (i=0; i< memory.programs.size(); i++)
   {
      if ((memory.programs[i].binx == xbin) && (memory.programs[i].biny == ybin) )
      {
         found=0;
         for(int j=0; j< counter; j++)
         {
            
            if(winxs[j] == memory.programs[i].windowx && winys[j] == memory.programs[i].windowy) found = 1;
         }
         if(!found)
         {
            winxs[ counter ] = memory.programs[i].windowx;
            winys[ counter ] = memory.programs[i].windowy;
         }
         counter++;
       }
   }

   var_windowxs.Set(winxs);
   var_windowys.Set(winys);
   delete winxs;
   delete winys;

   return NO_ERROR;
}
//+Entry: ReprogramJoe
//
// This function switches between the different LittleJoe programs (waveforms)
//
// If the requested parameters are not valid, it fails silently
//
// Watch out: this routine calls itself recursively when an upload if needed. Always take this
// into consideration with changing something.
//
// LittleJoe can only be reprogrammed in the SWITCHEDOFF state. If a request is made when
// not in this state it will be refused, unless the <force> flag is nonzero. In this case
// the routine will attempt to reprogram the LittleJoe anyway (this is necessary, for example,
// for the first configuration)
//-

int JoeCtrl::ReprogramJoe( int force)
{
   int need_upload;
   int speed;
   int xbin; 
   int ybin;
   int reps;
   int pos;
   int cur_speed;
   int cur_xbin;
   int cur_ybin;

   //unsigned int i;
   char cmstr[50];

   var_speed_req.Get(&speed);
   var_xbin_req.Get(&xbin);
   var_ybin_req.Get(&ybin);
   var_rep_req.Get(&reps);

   var_speed_cur.Get(&cur_speed);
   var_xbin_cur.Get(&cur_xbin);
   var_ybin_cur.Get(&cur_ybin);

   var_rep_cur.Get(&cur_Reps);
   
   // Nothing to do?
   if ((speed == cur_speed) && (xbin == cur_xbin) && (ybin == cur_ybin) && (reps==cur_Reps))
   {
      _logger->log( Logger::LOG_LEV_INFO, "SetJoeProgram(): current settings are OK, nothing to do.");
      return NO_ERROR;
   }


/*        // Stop ccd integration
        stat = Stop();
        if (stat != NO_ERROR)
            return stat;
*/
   _logger->log( Logger::LOG_LEV_DEBUG, "Checking program %d, %dx%d", speed, xbin, ybin);
                                
   // See if we already have the program in memory, or if we have to upload it
   // For the VisAO version, this really just means updating memory from ondisk
   pos = GetProgramPos( speed, xbin, ybin, &need_upload);
   _logger->log( Logger::LOG_LEV_DEBUG, "Pos in memory=%d, need_upload = %d", pos, need_upload);

   
  
        //At this point should just be able to issue a "set x x x x" to CCD47Ctrl

   
   //but need to track current program_set
        
   // If not in current memory
   if (pos == -1)
   {
      // If not in disk files
      if (need_upload == -1)
      {
         _logger->log( Logger::LOG_LEV_WARNING, "Program %d, %dx%d is not valid", speed, xbin, ybin);
         return NO_ERROR;
      }

      // If in disk files, upload the new program set
      

      UpdateJoeMemory( need_upload);
      cur_ProgramSet = need_upload;
      
       // Call again this routine, now it will find the program inside the LittleJoe memory
      return ReprogramJoe( force);
   }
                
   // Program found in current memory
   _logger->log( Logger::LOG_LEV_INFO, "Program in memory at position %d", pos);

   cur_Program = pos;

   snprintf(cmstr, 50, "set %i %i %i %i", cur_ProgramSet, cur_Program, cur_Gain, cur_Reps);

   std::cout << "Sending command: " << cmstr << "\n";
   send_ccd47_command(cmstr);
   sleep(2);
   // Update status variables

   //in VisAO these will happen as part of event loop.

   //var_xbin_cur.Set(xbin, 0, FORCE_SEND);
   //var_ybin_cur.Set(ybin, 0, FORCE_SEND);
   //var_speed_cur.Set(speed, 0, FORCE_SEND);

   /*// Set default black levels, if specified
   littlejoe_program *program = GetProgram(speed, xbin, ybin);

   if (program)
           for (i=0; i< _ccdBlacks.size(); i++)
              if (program->black_levels[i] >=0)
                  if ((stat = SetJoeBlack(i, program->black_levels[i])) != NO_ERROR)
                     _logger->log( Logger::LOG_LEV_ERROR, "Error in SetJoeBlacklevels(): (%d) %s", stat, lao_strerror(stat));

        // Update black level variables
        for (i=0; i< _ccdBlacks.size(); i++)
           {
           int level = GetBlackLevel(i);
           if (level>=0)
               var_black_cur.Set( GetBlackLevel(i), (int)i, NO_SEND);
           else
               return level;
           }

        var_black_cur.Send();
   */
   // Since the program also stores run state, fix it if needed
   /*int enable;
   var_enable_req.Get(&enable);
   if (enable)
    stat = Start();
        else
           stat = Stop();
        if (stat != NO_ERROR)
            return stat;
*/
   // Show new available speeds
   ExposeSpeeds();
   // Show new available windows
   ExposeWindows();

   // Update shown framerate
   return ComputeFramerate();
}





// Adds $ADOPT_ROOT/filePrefix/ to the name
/*std::string JoeCtrl::getCompletePath( std::string filename)
{
   std::string path;

   path = getenv("ADOPT_ROOT");
   path += "/" + _filePrefix;
   path += filename;

   return path;
}*/



//+Function GetProgram
//
// Finds the program with the specified characteristics and returns a pointer to it.
// Returns NULL if the program is not found.
//-

littlejoe_program *JoeCtrl::GetProgram( int speed, int xbin, int ybin)
{
        unsigned int i,j;

        for (i=0; i<ondisk.size(); i++)
                for (j=0; j< ondisk[i].programs.size(); j++)
                        if ((ondisk[i].programs[j].binx == xbin) &&
                            (ondisk[i].programs[j].biny == ybin) &&
                            (ondisk[i].programs[j].readout_speed == speed))
                                return &(ondisk[i].programs[j]);
                
        return NULL;
}



//+Function
//
// GetProgramPos
//
// Returns the position into the LittleJoe controller memory of the specified program.
// If not found, returns -1 and puts into need_upload the file number to upload.
// If a file with the specified program is not found, need_upload will be -1 as well.
//-


int JoeCtrl::GetProgramPos( int speed, int xbin, int ybin, int *need_upload)
{
        unsigned int i,j;
        i=0;

        // Search in the current LittleJoe memory
        vector<littlejoe_program>::iterator iter;
        for (iter= memory.programs.begin(); iter != memory.programs.end(); iter++)
                {
                if ((iter->binx == xbin) &&
                    (iter->biny == ybin) &&
                    (iter->readout_speed == speed))
                        {
                        if (need_upload)
                                *need_upload=0;
                        return i;
                   }
                i++;
           }    

        _logger->log( Logger::LOG_LEV_DEBUG, "Not in memory");

        // Search in the disk files
        if (need_upload)
                for (i=0; i<ondisk.size(); i++)
                        for (j=0; j< ondisk[i].programs.size(); j++)
                                if ((ondisk[i].programs[j].binx == xbin) &&
                                    (ondisk[i].programs[j].biny == ybin) &&
                                    (ondisk[i].programs[j].readout_speed == speed))
                                        {
                                        *need_upload = i;
                                        return -1;
                                        }
        _logger->log( Logger::LOG_LEV_DEBUG, "Not on disk");        
        if (need_upload)
                *need_upload=-1;
        return -1;
}

//+Function
//
// UpdateJoeMemory
//
// To be called after a program has been successfully downloaded to the LittleJoe controller
//-

int JoeCtrl::UpdateJoeMemory( unsigned int uploaded_file)
{
        if (uploaded_file >= ondisk.size())
                return JOE_OUT_OF_RANGE_ERROR;

   memory = ondisk[uploaded_file];

        return NO_ERROR;
}


//+Function: DumpProgramset
//
// Dumps a programset on screen
//-

int DumpProgramset( littlejoe_programset programset)
{
   int i=0;

        printf("Programset: %s\n", programset.name.c_str());
        printf("Control filename: %s\n", programset.control_filename.c_str());
        printf("Pattern filename: %s\n", programset.pattern_filename.c_str());
        printf("Number of programs: %lu\n", (unsigned long) (programset.programs.size()));

    vector<littlejoe_program>::iterator iter;
        for (iter= programset.programs.begin(); iter != programset.programs.end(); iter++)
                {
                printf("Program %d, name:%s\n", i++, iter->name.c_str());
                printf("Bin, speed: %dx%d, %d\n", iter->binx, iter->biny, iter->readout_speed);
                printf("Delay base, inc: %f,%f\n", iter->delay_base, iter->delay_inc);
                }
        
        return NO_ERROR;
}


//@Function: EraseLocalMemory
//
// Erases the local copy of LittleJoe's internal memory
//@

int JoeCtrl::EraseLocalMemory()
{
    memory.programs.clear();

    // If requested, start assuming a certain program set is available
    if (_startProgramSet >= 0)
       SetLocalMemory( _startProgramSet);

        return NO_ERROR;
}

//@Function: SetLocalMemory
//
// Synch the local memory information to a certain programset
//@

int JoeCtrl::SetLocalMemory( unsigned int programset_num)
{
    memory = ondisk[programset_num];
    return NO_ERROR;
}
        

//@Function: LoadJoeDiskFiles
//
// Load the configuration files specifying the different
// program sets that can be uploaded to LittleJoe
//
//@

int JoeCtrl::LoadJoeDiskFiles()
{
    int num, i;
    std::string prefix;

    num = ConfigDictionary()["num_programsets"];

    ondisk.resize(num);

    for (i=0; i<num; i++)
         {
         char param[32];

         sprintf( param, "programset%d", i);
         Config_File *subtree = ConfigDictionary().extract(param);
         ondisk[i] = ReadProgramSet( *subtree);
         delete subtree;
         }

    if (debug)
        for (i=0; i<num; i++)
                DumpProgramset(ondisk[i]);

    _logger->log( Logger::LOG_LEV_INFO, "%d programsets loaded.", ondisk.size());
    return 0;
}

// Reads a configuration file with the parameters of an entire LittleJoe program set

littlejoe_programset JoeCtrl::ReadProgramSet( Config_File &cfg)
{
        int size;
        unsigned int i;

        littlejoe_programset programset;

        programset.name = (std::string) cfg["name"];
        programset.control_filename = (std::string) cfg["control_filename"];
        programset.pattern_filename = (std::string) cfg["pattern_filename"];
        size = cfg["num_programs"];

        if ((size<=0) || (size > 1000))
                {
                _logger->log( Logger::LOG_LEV_WARNING, "Skipping programset %s because it has %d programs", programset.name.c_str(), size);
                return programset;
                }

        programset.programs.resize( size);

        for (i=0; i< programset.programs.size(); i++)
                {
                char par_name[32];
                sprintf( par_name, "program%d", i);

                _logger->log( Logger::LOG_LEV_DEBUG, "Reading program %d of %d", i, programset.programs.size());
                Config_File *subtree = cfg.extract(par_name);
                programset.programs[i] = ReadProgram( *subtree);
                delete subtree;
                }

        return programset;
}

// Read a configuration file with the parameters of a single LittleJoe program

littlejoe_program JoeCtrl::ReadProgram( Config_File &cfg)
{
        littlejoe_program program;

        program.name = (std::string) cfg["name"];
        program.readout_speed = cfg["readout_speed"];
        program.binx = cfg["binx"];
        program.biny = cfg["biny"];
        program.windowx = cfg["windowx"];
        program.windowy = cfg["windowy"];
        program.delay_base = cfg["delay_base"];
        program.delay_inc = cfg["delay_inc"];

        // Read default black levels if they exist, otherwise set to -1
        for (int i=0; i<4; i++) {
           try {
              char str[10];
              sprintf( str, "black%d", i+1);
              program.black_levels[i] = cfg[str];
           } catch (Config_File_Exception &e) {
              program.black_levels[i] = -1;
           }
        }

        _logger->log( Logger::LOG_LEV_DEBUG, "Found: speed %d, binx %d, biny %d, base %f, inc %f", program.readout_speed, program.binx, program.biny, program.delay_base, program.delay_inc);

        // Build the arrays listing each possible value
        insert_value( _ccdXbins, program.binx);
        insert_value( _ccdYbins, program.biny);
//        insert_value( ccd_speeds, program.readout_speed);

        return program;
}

int JoeCtrl::insert_value( vector<int> &array, int value)
{
    vector<int>::iterator iter;
    for ( iter = array.begin(); iter != array.end(); iter++)
        if (*iter == value)
            return NO_ERROR;

    array.push_back(value);
    return NO_ERROR;
}


} //namespace VisAO                                                           
                    
