/************************************************************
*    PwrMon.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the PwrMon class, which monitors the VisAOPwrCtrl
* application.  VisAO power control is managed from the supervisor computer
* in the interest of reliability, but the VisAO system needs access to it.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file PwrMon.h
  * \author Jared R. Males
  * \brief Declarations for the VisAOPwrMon class
  *
  * VisAOPwrMon monitors the VisAOPwrCtrl
  * application.  VisAO power control is managed from the supervisor computer
  * in the interest of reliability, but the VisAO system needs access to it.
  *
*/

//#define _debug

#include "PwrMon.h"



namespace VisAO
{

PwrMon::PwrMon( std::string name, const std::string &conffile) throw (AOException) : AOApp(name, conffile)
{
   setupVisAOApp();
}

PwrMon::PwrMon( int argc, char **argv) throw (AOException) : AOApp(argc, argv)
{
   setupVisAOApp();
}

void PwrMon::setupVisAOApp()
{
   //Init the status board
   statusboard_shmemkey = 13000;
   if(create_statusboard(sizeof(power_status_board)) != 0)
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
   
}

   
void PwrMon::SetupVars()
{
   Config_File *cfg = &ConfigDictionary();
   std::string pwrctrl_name, pwrconf_name, pwr_full_name;
   Config_File * pwr_conf;
   
   try
   {
      pwrctrl_name  = (std::string)(*cfg)["pwrCtrlName"];
   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, e.what().c_str());
      throw(e);
   }

   pwr_full_name = pwrctrl_name + ".L";
   
   pwrconf_name = Utils::getConffile(pwrctrl_name);

   pwr_conf = new Config_File(pwrconf_name);

   std::vector<std::string> item_codes(8);
   std::vector<std::string> item_names(8);
   char item_code[32];
   char item_name[32];
   
   for(int i=0; i< 8; i++)
   {
      
      snprintf( item_code, 32, "item%d.code", i);
      snprintf( item_name, 32, "item%d.name", i);
      
      try
      {
         item_codes[i]  = (std::string)(*pwr_conf)[item_code];
      }
      catch (Config_File_Exception &e)
      {
         snprintf( item_code, 32, "OUTLET%d", i+1);
         item_codes[i]  = item_code;
      }
      
      try
      {
         item_names[i]  = (std::string)(*pwr_conf)[item_name];
      }
      catch (Config_File_Exception &e)
      {
         snprintf( item_name, 32, "OUTLET%d", i+1);
         item_names[i]  = item_name;
      }
   }

   try
   {
      
      var_outlet1_cur = RTDBvar( pwr_full_name, item_codes[0], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet1_req = RTDBvar( pwr_full_name, item_codes[0], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet1_name = RTDBvar( pwr_full_name, item_codes[0]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[0].size());
      Notify(var_outlet1_cur, outlet1CurChanged);

      var_outlet2_cur = RTDBvar( pwr_full_name, item_codes[1], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet2_req = RTDBvar( pwr_full_name, item_codes[1], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet2_name = RTDBvar( pwr_full_name, item_codes[1]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[1].size());
      Notify(var_outlet2_cur, outlet2CurChanged);

      var_outlet3_cur = RTDBvar( pwr_full_name, item_codes[2], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet3_req = RTDBvar( pwr_full_name, item_codes[2], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet3_name = RTDBvar( pwr_full_name, item_codes[2]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[2].size());
      Notify(var_outlet3_cur, outlet3CurChanged);
      
      var_outlet4_cur = RTDBvar( pwr_full_name, item_codes[3], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet4_req = RTDBvar( pwr_full_name, item_codes[3], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet4_name = RTDBvar( pwr_full_name, item_codes[3]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[3].size());
      Notify(var_outlet4_cur, outlet4CurChanged);
   
      var_outlet5_cur = RTDBvar( pwr_full_name, item_codes[4], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet5_req = RTDBvar( pwr_full_name, item_codes[4], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet5_name = RTDBvar( pwr_full_name, item_codes[4]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[4].size());
      Notify(var_outlet5_cur, outlet5CurChanged);
      
      var_outlet6_cur = RTDBvar( pwr_full_name, item_codes[5], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet6_req = RTDBvar( pwr_full_name, item_codes[5], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet6_name = RTDBvar( pwr_full_name, item_codes[5]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[5].size());
      Notify(var_outlet6_cur, outlet6CurChanged);

      var_outlet7_cur = RTDBvar( pwr_full_name, item_codes[6], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet7_req = RTDBvar( pwr_full_name, item_codes[6], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet7_name = RTDBvar( pwr_full_name, item_codes[6]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[6].size());
      Notify(var_outlet7_cur, outlet7CurChanged);
      
      var_outlet8_cur = RTDBvar( pwr_full_name, item_codes[7], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet8_req = RTDBvar( pwr_full_name, item_codes[7], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet8_name = RTDBvar( pwr_full_name, item_codes[7]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[7].size());
      Notify(var_outlet8_cur, outlet8CurChanged);

   }
   catch (AOVarException &e)
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
}

int PwrMon::outlet1CurChanged(void *pt, Variable *msgb)
{
   PwrMon * pm = (PwrMon *) pt;

   pm->var_outlet1_cur.Update();

   pm->update_statusboard();

   return 0;
}

int PwrMon::outlet2CurChanged(void *pt, Variable *msgb)
{
   PwrMon * pm = (PwrMon *) pt;

   pm->var_outlet2_cur.Update();

   pm->update_statusboard();

   return 0;
}

int PwrMon::outlet3CurChanged(void *pt, Variable *msgb)
{
   PwrMon * pm = (PwrMon *) pt;

   pm->var_outlet3_cur.Update();

   pm->update_statusboard();

   return 0;
}

int PwrMon::outlet4CurChanged(void *pt, Variable *msgb)
{
   PwrMon * pm = (PwrMon *) pt;

   pm->var_outlet4_cur.Update();

   pm->update_statusboard();

   return 0;
}

int PwrMon::outlet5CurChanged(void *pt, Variable *msgb)
{
   PwrMon * pm = (PwrMon *) pt;

   pm->var_outlet5_cur.Update();

   pm->update_statusboard();

   return 0;

}

int PwrMon::outlet6CurChanged(void *pt, Variable *msgb)
{
   PwrMon * pm = (PwrMon *) pt;

   pm->var_outlet6_cur.Update();

   pm->update_statusboard();

   return 0;
}

int PwrMon::outlet7CurChanged(void *pt, Variable *msgb)
{
   PwrMon * pm = (PwrMon *) pt;

   pm->var_outlet7_cur.Update();

   pm->update_statusboard();

   return 0;
}

int PwrMon::outlet8CurChanged(void *pt, Variable *msgb)
{
   PwrMon * pm = (PwrMon *) pt;

   pm->var_outlet8_cur.Update();

   pm->update_statusboard();

   return 0;
}


void PwrMon::Run()
{
   init_statusboard();

   _logger->log( Logger::LOG_LEV_INFO, "Running...");
      
   while(!TimeToDie())
   {
      try
      {
         usleep((int) pause_time*1000000.);
         update_statusboard();
      }
      catch (AOException &e)
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());      
      }
   }
}
   

int PwrMon::init_statusboard()
{
   std::string tmpstr;
   if(statusboard_shmemptr)
   {
      VisAO::power_status_board * psb = (VisAO::power_status_board *) statusboard_shmemptr;
      
      tmpstr = var_outlet1_name.Get();
      strncpy(psb->outlet1_name, tmpstr.c_str(), 50);

      tmpstr = var_outlet2_name.Get();
      strncpy(psb->outlet2_name, tmpstr.c_str(), 50);

      tmpstr = var_outlet3_name.Get();
      strncpy(psb->outlet3_name, tmpstr.c_str(), 50);

      tmpstr = var_outlet4_name.Get();
      strncpy(psb->outlet4_name, tmpstr.c_str(), 50);

      tmpstr = var_outlet5_name.Get();
      strncpy(psb->outlet5_name, tmpstr.c_str(), 50);

      tmpstr = var_outlet6_name.Get();
      strncpy(psb->outlet6_name, tmpstr.c_str(), 50);

      tmpstr = var_outlet7_name.Get();
      strncpy(psb->outlet7_name, tmpstr.c_str(), 50);

      tmpstr = var_outlet8_name.Get();
      strncpy(psb->outlet8_name, tmpstr.c_str(), 50);
   }

   return 0;
}

int PwrMon::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      VisAO::power_status_board * psb = (VisAO::power_status_board *) statusboard_shmemptr;

/*      var_outlet1_cur.Update();
      var_outlet2_cur.Update();
      var_outlet3_cur.Update();
      var_outlet4_cur.Update();
      var_outlet5_cur.Update();
      var_outlet6_cur.Update();
      var_outlet7_cur.Update();
      var_outlet8_cur.Update();*/

      var_outlet1_cur.Get(&psb->outlet1_state);
      var_outlet2_cur.Get(&psb->outlet2_state);
      var_outlet3_cur.Get(&psb->outlet3_state);
      var_outlet4_cur.Get(&psb->outlet4_state);
      var_outlet5_cur.Get(&psb->outlet5_state);
      var_outlet6_cur.Get(&psb->outlet6_state);
      var_outlet7_cur.Get(&psb->outlet7_state);
      var_outlet8_cur.Get(&psb->outlet8_state);

      #ifdef _debug
      std::cout << psb->outlet1_name << " " << psb->outlet1_state << "\n";
      std::cout << psb->outlet2_name << " " << psb->outlet2_state << "\n";
      std::cout << psb->outlet3_name << " " << psb->outlet3_state << "\n";
      std::cout << psb->outlet4_name << " " << psb->outlet4_state << "\n";
      std::cout << psb->outlet5_name << " " << psb->outlet5_state << "\n";
      std::cout << psb->outlet6_name << " " << psb->outlet6_state << "\n";
      std::cout << psb->outlet7_name << " " << psb->outlet7_state << "\n";
      std::cout << psb->outlet8_name << " " << psb->outlet8_state << "\n";
      std::cout << "\n";
      #endif
      
   }
   return 0;
}//int PwrMon::update_statusboard()

} //namespace VisAO


