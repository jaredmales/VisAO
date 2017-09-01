#include "ESPMotorCtrl.h"


namespace VisAO
{

#define _debug

ESPMotorCtrl::ESPMotorCtrl()
{
   numAxes = 0;
   default_timeout = 1000;
}

ESPMotorCtrl::ESPMotorCtrl(int no)
{
   numAxes = 0; //must initialize or setNumAxes will fail.
   setNumAxes(no);
   default_timeout = 1000;
}

ESPMotorCtrl::~ESPMotorCtrl()
{
   return;
}

void ESPMotorCtrl::getStageName(int axis, std::string &sname)
{
   std::string com, resp;
   char errc;

   errStr.str("");
   
   if(makeCom(com, axis, "ZX?") < 0)
   {
      errStr << "Error making command ZX?" <<  __FILE__ << " " <<  __LINE__ << '\n';
      return;
   }
   
   sendCommand(com, resp, default_timeout);
   

   errc = getError(axis);
   if(errc != '@')
   {
      //std::cerr << "An Error occurred\n";
      errStr << "An error occurred (" << errc << ") " <<  __FILE__ << " " <<  __LINE__ << '\n';
      sname = "";
      return;
   }
   
  
   if(axis < 10)
   {
      sname = resp.substr(3, resp.length()-3);
   }
   else
   {
      sname = resp.substr(4, resp.length()-4);
   }
   
}

int ESPMotorCtrl::setNumAxes(int no)
{
   errStr.str("");
   if(numAxes != 0)
   {
      errStr << "Already initialized.  Can't change number of axes " << __FILE__ <<  " " <<  __LINE__ << '\n';
      return -1;
   }

   numAxes = no;

   axisAddress.resize(numAxes);
   for(int i=0;i<numAxes;i++) axisAddress[i] = i+1;
   
   stageName.resize(numAxes);

   return 0;
}

int ESPMotorCtrl::powerOnInit()
{
   std::string com, resp, err;

   errStr.str("");
   
   if(numAxes == 0)
   {
      errStr  << "Not initialized.  Must set number of axes. "<< __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }

   #ifdef _debug
      std::cout << "Power on initialization\n";
   #endif

   for(int i=0;i<numAxes;i++)
   {
      getStageName(i+1, stageName[i]);
      if(stageName[i] == "" && errStr.str() != "")
      {
         errStr << "Error getting stagename " << i << " " <<  __FILE__ << " " << __LINE__ << '\n';
         return -1;
      }
   }

   #ifdef _debug
      std::cout << "Got stage names\n";
   #endif

   std::string state;
   for(int i=0;i<numAxes;i++)
   {
      if(getCtrlState(i+1, state) != 0)
      {
         errStr << "Error getting controller " << i+1 << " state." << __FILE__ << " " << __LINE__ << "\n";
         return -1;
      }

      /*if(state[0] != '0' || state[1] != '0' || (state[2] != '0' && state[2] != '4') || (state[3] != '0' && state[3] != '1' && state[3] != '2'))
      {
         errStr << "Unacceptable positioner " << i+1 << " error.  State: " << state << " " << __FILE__ << " " <<  __LINE__ << "\n";
         return -1;
      }*/

      if(state[0] == '0' || (state[0] == '1' && state[1] == '1'))
      {
         if(home(i+1) < 0)
         {
            errStr << "Power on homing error " << __FILE__ << " " << __LINE__ << "\n";
            return -1;
         }
      }
      /*else if (state[4] == '1' && (state[5] == 'E' || state[5] == 'F')) curState = STATE_HOMING;
      else if (state[4] == '2') curState = STATE_OPERATING;
      else if (state[4] == '3' && isnum(state[5])) curState = STATE_READY;
      else curState = STATE_INVALID;*/
   }
   
   #ifdef _debug
      std::cout << "Power on init complete\n";
   #endif

   return 0;
}

char ESPMotorCtrl::getError(int axis)
{
   std::string com, resp;

   errStr.str("");
   if(axis < 1 || axis > numAxes)
   {
      errStr << "Bad axis " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }

   if(makeCom(com, axis, "TE") < 0)
   {
      errStr << "Error making command TE" << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   sendCommand(com, resp, default_timeout);

   if(axis < 10)
   {
      return resp[3];
   }
   else
   {
      return resp[4];
   }

}
   
std::string ESPMotorCtrl::getStageName(int axis)
{
   if(axis < 1 || axis > numAxes)
   {
      errStr << "Bad Axis " << __FILE__ << " " << __LINE__ << '\n';
      return "";
   }
   
   return stageName[axis-1];
}

int ESPMotorCtrl::getCtrlState(int axis, std::string &state)
{
   std::string com, resp;
   errStr.str("");
   
   if(makeCom(com, axis, "TS") < 0)
   {
      errStr << "Error making command TS" << __FILE__ << " " << __LINE__ << '\n';
      return 0;
   }
   
   sendCommand(com, resp, default_timeout);

   int raxis;
   std::string rcom, rval;
   
   splitResponse(raxis, rcom, rval, resp);
   
   if(rcom == "")
   {
      errStr << "An Error occurred " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
      
   if(raxis != axis)
   {
      errStr << "Wrong axis returned " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }

   if(rcom != "TS")
   {
      errStr << "Wrong command returned " <<  __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   if(rval.length() != 6)
   {
      errStr << "Incorrect response length "<< __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }

   state = rval.substr(4, 2);

   return 0;
}
   

double ESPMotorCtrl::getCurPos(int axis)
{
   std::string com, resp;

   if(makeCom(com, axis, "TP") < 0)
   {
      errStr << "Error making command TP" << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   sendCommand(com, resp, default_timeout);

   int raxis;
   std::string rcom, rval;
   splitResponse(raxis, rcom, rval, resp);
   
   if(rcom == "")
   {
      errStr << "An Error occurred " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   if(raxis != axis)
   {
      errStr <<  "Wrong axis returned " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   if(rcom != "TP")
   {
      errStr << "Wrong command returned " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }

   return strtod(rval.c_str(),0);
   
}

int ESPMotorCtrl::home(int axis)
{
   std::string com, resp;
   
   if(makeCom(com, axis, "OR") < 0)
   {
      errStr << "Error making command OR" << __FILE__ << " " << __LINE__ << '\n';
      return 0;
   }
   
   moveStart();
   
   if(sendCommand(com) < 0)
   {
      errStr << "Error sending command OR" << __FILE__ << " " << __LINE__ << '\n';
      return 0;
   }
   
   char errc = getError(axis);
   if(errc != '@')
   {
      errStr << "An error occurred in homing (" << errc << ") " <<  __FILE__ << " " <<  __LINE__ << '\n';
      return -1;
   }
   
   return 0;
   
}

int ESPMotorCtrl::stop(int axis)
{
   std::string com;

   if(axis == 0)
   {
      com = "0ST";
   }
   else
   {
      if(makeCom(com, axis, "ST") < 0)
      {
         errStr << "Error making command ST" << __FILE__ << " " << __LINE__ << '\n';
         errStr << "Sending 0ST" << __FILE__ << " " << __LINE__ << '\n';
         com = "0ST";
      }
   }
   
   if(sendCommand(com) < 0)
   {
      errStr << "Error sending command ST" << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   char errc = getError(axis);
   if(errc != '@')
   {
      errStr << "An error occurred in stopping (" << errc << ") " <<  __FILE__ << " " <<  __LINE__ << '\n';
      return -1;
   }
   
   return 0;
}

int ESPMotorCtrl::gotoAbsPos(int axis, double apos)
{
   std::string com;
   
   if(makeCom(com, axis, "PA", apos) < 0)
   {
      errStr << "Error making command PA " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   moveStart();
   
   if(sendCommand(com) < 0)
   {
      errStr << "Error sending command PA " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   char errc = getError(axis);
   if(errc != '@')
   {
      errStr << "An error occurred in goto abs. pos. (" << errc << ") " <<  __FILE__ << " " <<  __LINE__ << '\n';
      return -1;
   }
   
   return 0;
}

int ESPMotorCtrl::gotoRelPos(int axis, double apos)
{
   std::string com;
   errStr.str("");

   //Don't do a relative move less than the minimum resolution
   if(fabs(apos) < MOVE_RESOLUTION) return 0;
   
   if(makeCom(com, axis, "PR", apos) < 0)
   {
      errStr << "Error making command PR " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   moveStart();
   
   if(sendCommand(com) < 0)
   {
      errStr << "Error sending command PR" << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   char errc = getError(axis);
   if(errc != '@')
   {
      errStr << "An error occurred in goto rel. pos. (" << errc << ") " <<  __FILE__ << " " <<  __LINE__ << '\n';
      return -1;
   }
   
   return 0;
}


int ESPMotorCtrl::makeCom(std::string &str, int axis, const char *com)
{
   char tmp[10];

   if(axis < 1 || axis > numAxes)
   {
      errStr << "Bad axis " << __FILE__ << " " << __LINE__ << '\n';
      return -1;
   }
   
   snprintf(tmp, 10, "%i", axisAddress[axis-1]);
   
   str = tmp;
   
   str += com;

   return 0;
}

int ESPMotorCtrl::makeCom(std::string &str, int axis, const char *com, int val)
{
   char tmp[10];

   errStr.str("");
   if(axis < 1 || axis > numAxes)
   {
      errStr << "Bad axis " << __FILE__ <<  " " << __LINE__ << '\n';
      return -1;
   }
   
   snprintf(tmp, 10, "%i", axisAddress[axis-1]);

   str = tmp;

   str += com;

   snprintf(tmp, 10, "%i", val);

   str += tmp;

   return 0;
}

int ESPMotorCtrl::makeCom(std::string &str, int axis, const char *com, double val)
{
   char tmp[10];

   errStr.str("");
   if(axis < 1 || axis > numAxes)
   {
      errStr << "Bad axis " << __FILE__ <<  " " << __LINE__ << '\n';
      return -1;
   }
   
   snprintf(tmp, 10, "%i", axisAddress[axis-1]);
   
   str = tmp;
   
   str += com;
   
   snprintf(tmp, 10, "%f", val);
   
   str += tmp;

   return 0;
}

int ESPMotorCtrl::makeCom(std::string &str, int axis, const char *com, std::string &val)
{
   char tmp[10];

   errStr.str("");
   if(axis < 1 || axis > numAxes)
   {
      errStr << "Bad axis " << __FILE__ <<  " " << __LINE__ << '\n';
      return -1;
   }
   
   snprintf(tmp, 10, "%i", axisAddress[axis-1]);
   
   str = tmp;
   
   str += com;
      
   str += val;

   return 0;
}

int ESPMotorCtrl::splitResponse(int &axis, std::string &com, std::string &val, std::string &resp)
{
   if(resp.length() < 3)
   {
      errStr << "Invalid response " << __FILE__ << " " << __LINE__;
      return -1;
   }
   
   if(isalpha(resp[0]))
   {
      errStr << "Invalid response " << __FILE__ << " " << __LINE__;
      axis = 0;
      com = "";
      val = resp;
      return 0;
   }

   if(isalpha(resp[1]))
   {
      axis = resp[0] - '0';
   }
   else
   {
      axis = atoi(resp.substr(0,2).c_str());
   }
   
   if(axis < 10)
   {
      
      com = resp.substr(1,2);
      if(resp.length() < 4 ) val = "";
      else val = resp.substr(3, resp.length()-3);
   }
   else
   {
      if(resp.length() < 4)
      {
         errStr << "Invalid response " << __FILE__ << " " << __LINE__;
         com = "";
         val = "";
         return -1;
      }
      com = resp.substr(2,2);
      if(resp.length() < 5) val = "";
      else val = resp.substr(4, resp.length()-4);
   }

   return 0;
}


int ESPMotorCtrl::sendCommand(std::string &com, std::string &resp, int timeout __attribute__((unused)))
{
   std::cout << com << "\n";
   std::cin >> resp;
   return 0;
}

int ESPMotorCtrl::sendCommand(std::string &com)
{
   std::cout << com << "\n";
   return 0;
}

void ESPMotorCtrl::moveStart()
{
   std::cout << "ESP moveStart\n";
   return;
}

}//namespace VisAO

