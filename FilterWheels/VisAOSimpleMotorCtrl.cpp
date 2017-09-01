/************************************************************
*    VisAOSimpleMotorCtrl.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the VisAOSimpleMotorCtrl class, which adapts
* the SimpleMotorCtrl class for use as a VisAO App.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOSimpleMotorCtrl.cpp
  * \author Jared R. Males
  * \brief Declarations for the VisAOSimpleMotorCtrl class.
  *
*/

#include "VisAOSimpleMotorCtrl.h"

/// Mutex to lock out other threads during complex operations.
/** Defined in SimpleMotorCtrl.cpp, we use it here for \ref VisAOSimpleMotorCtrl::CModeReqChanged.
 */
extern pthread_mutex_t threadMutex;

namespace VisAO
{

//Static fields
int VisAOSimpleMotorCtrl::control_mode = VisAOSimpleMotorCtrl::CMODE_REMOTE;
   
int VisAOSimpleMotorCtrl::default_control_mode = -1;
   
VisAOSimpleMotorCtrl::VisAOSimpleMotorCtrl( std::string name, const std::string &conffile) throw (AOException) : SimpleMotorCtrl( name, conffile)
{
   control_mode = CMODE_REMOTE;
   return;
}

VisAOSimpleMotorCtrl::VisAOSimpleMotorCtrl( int argc, char **argv) throw (AOException) : SimpleMotorCtrl( argc, argv)
{
   control_mode = CMODE_REMOTE;
   return;
}

void VisAOSimpleMotorCtrl::SetupVars()
{
   //Add the normal motor variables to RTDB
   SimpleMotorCtrl::SetupVars();

   //Add the control mode variables to RTDB.
   try
   {
      var_cmode_cur = RTDBvar( MyFullName(), "ConMode", CUR_VAR, INT_VARIABLE, 1,1);
      var_cmode_cur.Set(control_mode, 0);
      var_cmode_req = RTDBvar( MyFullName(), "ConMode", REQ_VAR, INT_VARIABLE, 1,1);
   }
   catch (AOVarException &e)
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }

   //Reset the RTDB notifier to our handler.
   this->Notify( var_cmode_req, ((VisAOSimpleMotorCtrl *)this)->CModeReqChanged);

   VisAOSimpleMotorCtrl::request_control(CMODE_REMOTE, 1);
}

SimpleMotor * VisAOSimpleMotorCtrl::CreateMotor()
{
   SimpleMotor *motor = NULL;

   if ((string)ConfigDictionary()["MotorType"] == string("VisAOfilterwheel"))
   {
      //std::cout << "Creating VisAOfilterwheel\n";
      _logger->log( Logger::LOG_LEV_DEBUG, "This is a VisAO filterwheel");
      motor = new VisAOFilterWheel( this, ConfigDictionary());

      //After motor is created, re-Notify so the VisAO version of PosReqChange gets called
      Notify( *(((VisAOFilterWheel *)motor)->get_var_pos_req()), VisAO_PosReqChanged);
      Notify( ((VisAOFilterWheel *)motor)->var_pos_local_req, VisAO_PosLocalReqChanged);
      Notify( ((VisAOFilterWheel *)motor)->var_pos_script_req, VisAO_PosScriptReqChanged);
      
   }
   else motor = SimpleMotorCtrl::CreateMotor();

   return motor;
}


void VisAOSimpleMotorCtrl::updatePos( bool force)
{
   double pos = motor->GetPosition( force);     // This will update the POS.CUR value too.

   std::string customPos = GetPosName(pos);

   motor->SetPosName(customPos);
}

std::string VisAOSimpleMotorCtrl::GetPosName(double pos)
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
      }
      
      cpos_it++;
   }
   
   return "intermediate";
}

int VisAOSimpleMotorCtrl::GetType(double pos)
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

int VisAOSimpleMotorCtrl::request_control(int cmode)
{
   return request_control(cmode, 0);
}

int VisAOSimpleMotorCtrl::request_control(int cmode, bool override)
{
   //std::cout << "Requesting control\n";

      
   if(cmode >= control_mode || override || cmode == CMODE_NONE)
   {
      control_mode = cmode;

      var_cmode_cur.Set(control_mode, 0);
      return control_mode;
   }
   else return -1;
}



int VisAOSimpleMotorCtrl::CModeReqChanged(void *pt, Variable *msgb)
{

   int newstate;
   
   //std::cout << "In CModeReqChanged\n";
   VisAOSimpleMotorCtrl * vmc = (VisAOSimpleMotorCtrl *) pt;
   
   /*** Don't lock threadMutex here, it is done in request_control ****/
   
   newstate = msgb->Value.Lv[0];

   int cmode;
   bool orride = false;
   switch(newstate)
   {
      case 1:
         cmode = CMODE_REMOTE;
         break;
      case 10:
         cmode = CMODE_REMOTE;
         orride = true;
         break;
      case 2:
         cmode = CMODE_LOCAL;
         break;
      case 20:
         cmode = CMODE_LOCAL;
         orride = true;
         break;
      case 3:
         cmode = CMODE_SCRIPT;
         break;
      case 30:
         cmode = CMODE_SCRIPT;
         orride = true;
         break;
      default:
         cmode = CMODE_NONE;
         break;
   }

   return vmc->request_control(cmode, orride);

  

}

int VisAOSimpleMotorCtrl::VisAO_PosReqChanged( void *pt, Variable *var)
{
   int stat;
   double pos;

   //std::cout << "In Remote version of PosReqChanged\n";
   _logger->log(Logger::LOG_LEV_TRACE, "In Remote version of PosReqChanged");
   
   /***** Don't lock the mutex!  it will get locked by PosReqChanged *******/

   pos = var->Value.Dv[0];
   
   //First check if control type supports, or if it is an abort
   if(control_mode == CMODE_REMOTE || pos == ((VisAOSimpleMotorCtrl*)pt)->getAbortPos())
   {
      try
      {
         stat = SimpleMotorCtrl::PosReqChanged(pt, var);
      }
      catch(AOVarException &e)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "Exception in %s %i: %s", __FILE__, __LINE__, e.what().c_str());
         pthread_mutex_unlock(&threadMutex);
         stat = -1;
      }

   }
   else
   {
      stat = NO_ERROR; //Otherwise do nothing, and that's ok.
   }
   
   return stat;
}

int VisAOSimpleMotorCtrl::VisAO_PosLocalReqChanged( void *pt, Variable *var)
{
   int stat;
   double pos;
   //std::cout << "In Local version of PosReqChanged\n";

   _logger->log(Logger::LOG_LEV_TRACE, "In Local version of PosReqChanged");
   
   /***** Don't lock the mutex!  it will get locked by PosReqChanged *******/
   
   pos = var->Value.Dv[0];
   
   //First check if control type supports, or if it is an abort
   if(control_mode == CMODE_LOCAL || pos == ((VisAOSimpleMotorCtrl*)pt)->getAbortPos())
   {
      try
      {
         stat = SimpleMotorCtrl::PosReqChanged(pt, var);
      }
      catch(AOVarException &e)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "Exception in %s %i: %s", __FILE__, __LINE__, e.what().c_str());
         pthread_mutex_unlock(&threadMutex);
         stat = -1;
      }
      
   }
   else
   {
      stat = NO_ERROR; //Otherwise do nothing, and that's ok.
   }
   
   return stat;
}

int VisAOSimpleMotorCtrl::VisAO_PosScriptReqChanged( void *pt, Variable *var)
{
   int stat;
   double pos;

   //std::cout << "In Script version of PosReqChanged\n";
   _logger->log(Logger::LOG_LEV_TRACE, "In Script version of PosReqChanged");
   
   /***** Don't lock the mutex!  it will get locked by PosReqChanged *******/
   
   pos = var->Value.Dv[0];
   
   //First check if control type supports, or if it is an abort
   if(control_mode == CMODE_SCRIPT || pos == ((VisAOSimpleMotorCtrl*)pt)->getAbortPos())
   {
      try
      {
         stat = SimpleMotorCtrl::PosReqChanged(pt, var);
      }
      catch(AOVarException &e)
      {
         _logger->log(Logger::LOG_LEV_ERROR, "Exception in %s %i: %s", __FILE__, __LINE__, e.what().c_str());
         pthread_mutex_unlock(&threadMutex);
         stat = -1;
      }
      
   }
   else
   {
      stat = NO_ERROR; //Otherwise do nothing, and that's ok.
   }
   
   return stat;
}

} //namespace VisAO

