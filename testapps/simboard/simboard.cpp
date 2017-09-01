/************************************************************
 *    simboard.cpp
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Definitions for a class to simulate the VisAO Status Boards..
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file simboard.cpp
 * \author Jared R. Males
 * \brief Definitions for a class to simulate the VisAO Status Boards.
 *
 *
 */

#include "simboard.h"


namespace VisAO
{


simboard::simboard(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   Create();
}

simboard::simboard(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   Create();
}

void simboard::Create()
{
   int rv;
   size_t sizecheck;
   void * statusboard_shmemptr;
   int   statusboard_shmemid; 
   
   //Init the focus stage status board
   rv = create_shmem(&statusboard_shmemid, STATUS_focusstage, sizeof(focusstage_status_board));
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck, STATUS_focusstage, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else rv = -1;
   }
   
   if(rv != 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create focus stage status board.");
   }
   else
   {
      fsb = (VisAO::focusstage_status_board *) statusboard_shmemptr;
      strncpy(fsb->appname, "focusmotor.L", 25);
      fsb->max_update_interval = pause_time;
   }
   
   //Set up the focus status board
   try
   {
      fsb->power_state = (int)(ConfigDictionary())["focus_power_state"];
      fsb->is_moving = (int)(ConfigDictionary())["focus_is_moving"];
      fsb->cur_dir = (int)(ConfigDictionary())["focus_cur_dir"];
      fsb->cur_enabled = (int)(ConfigDictionary())["focus_cur_enabled"];
      fsb->homing = (int)(ConfigDictionary())["focus_homing"];
      fsb->cur_pos = (double)(ConfigDictionary())["focus_cur_pos"]; 
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Missing focus stage config data");
      throw;
   }

   
   //Init the CCD47 status board
   rv = create_shmem(&statusboard_shmemid, STATUS_ccd47, sizeof(ccd47_status_board));
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck, STATUS_ccd47, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else rv = -1;
   }
   
   if(rv != 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create ccd47 status board.");
   }
   else
   {
      ccd47sb = (VisAO::ccd47_status_board *) statusboard_shmemptr;
      strncpy(ccd47sb->appname, "ccd47ctrl.L", 25);
      ccd47sb->max_update_interval = pause_time;
   }
   
   //Set up the ccd47 status board
   try
   {
      ccd47sb->status = (int)(ConfigDictionary())["ccd47_status"];
      ccd47sb->speed = (int)(ConfigDictionary())["ccd47_speed"];
      ccd47sb->xbin = (int)(ConfigDictionary())["ccd47_xbin"];
      ccd47sb->ybin = (int)(ConfigDictionary())["ccd47_ybin"];
      ccd47sb->windowx = (int)(ConfigDictionary())["ccd47_windowx"];
      ccd47sb->windowy = (int)(ConfigDictionary())["ccd47_windowy"];
      ccd47sb->repetitions = (int)(ConfigDictionary())["ccd47_repetitions"];
      ccd47sb->framerate = (double)(ConfigDictionary())["ccd47_framerate"];
      ccd47sb->gain = (int)(ConfigDictionary())["ccd47_gain"];
      ccd47sb->joe_temp = (double)(ConfigDictionary())["ccd47_joe_temp"];
      ccd47sb->head_temp1 = (double)(ConfigDictionary())["ccd47_head_temp1"];
      ccd47sb->head_temp2 = (double)(ConfigDictionary())["ccd47_head_temp2"];
      ccd47sb->black0 = (int)(ConfigDictionary())["ccd47_black0"];
      ccd47sb->black1 = (int)(ConfigDictionary())["ccd47_black1"];

      ccd47sb->saving = (int)(ConfigDictionary())["ccd47_saving"];
      ccd47sb->skipping = (int)(ConfigDictionary())["ccd47_skipping"];
      ccd47sb->remaining = (int)(ConfigDictionary())["ccd47_remaining"];

      ccd47sb->imtype = (int)(ConfigDictionary())["ccd47_imtype"];

   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Missing CCD47 config data");
      throw;
   }
   
   //Init the shutter status board
   rv = create_shmem(&statusboard_shmemid, STATUS_shutterctrl, sizeof(shutter_status_board));
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck, STATUS_shutterctrl, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else rv = -1;
   }
   
   if(rv != 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create shutter status board.");
   }
   else
   {
      shsb = (VisAO::shutter_status_board *) statusboard_shmemptr;
      strncpy(shsb->appname, "shuttercontrol.L", 25);
      shsb->max_update_interval = pause_time;
   }
   
   //Set up the shutter status board
   try
   {
      shsb->in_auto = (int)(ConfigDictionary())["shutter_in_auto"];
      shsb->power_state = (int)(ConfigDictionary())["shutter_power_state"];
      shsb->sw_state = (int)(ConfigDictionary())["shutter_sw_state"];
      shsb->hw_state = (int)(ConfigDictionary())["shutter_hw_state"];
      shsb->sync_enabled = (int)(ConfigDictionary())["shutter_sync_enabled"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Missing shutter config data");
      throw;
   }
   
   //Init the f/w 2 status board
   rv = create_shmem(&statusboard_shmemid, STATUS_filterwheel2, sizeof(filterwheel_status_board));
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck, STATUS_filterwheel2, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else rv = -1;
   }
   
   if(rv != 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create f/w 2 status board.");
   }
   else
   {
      fw2sb = (VisAO::filterwheel_status_board *) statusboard_shmemptr;
      strncpy(fw2sb->appname, "filterwheel2.L", 25);
      fw2sb->max_update_interval = pause_time;
   }
   
   //Set up the f/w 2 status board
   try
   {
      fw2sb->state = (int)(ConfigDictionary())["fw2_state"];
      fw2sb->pos = (double)(ConfigDictionary())["fw2_pos"];
      fw2sb->req_pos = (double)(ConfigDictionary())["fw2_req_pos"];
      std::string name = (std::string)(ConfigDictionary())["fw2_name"];
      strncpy(fw2sb->filter_name, name.c_str(), 256);
      fw2sb->type = (int)(ConfigDictionary())["fw2_type"];
      fw2sb->req_type = (int)(ConfigDictionary())["fw2_req_type"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Missing f/w 2 config data");
      throw;
   }
   
   
   //Init the f/w 3 status board
   rv = create_shmem(&statusboard_shmemid, STATUS_filterwheel3, sizeof(filterwheel_status_board));
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck, STATUS_filterwheel3, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else rv = -1;
   }
   
   if(rv != 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create f/w 3 status board.");
   }
   else
   {
      fw3sb = (VisAO::filterwheel_status_board *) statusboard_shmemptr;
      strncpy(fw3sb->appname, "filterwheel3.L", 25);
      fw3sb->max_update_interval = pause_time;
   }
   
   //Set up the f/w 3 status board
   try
   {
      fw3sb->state = (int)(ConfigDictionary())["fw3_state"];
      fw3sb->pos = (double)(ConfigDictionary())["fw3_pos"];
      fw3sb->req_pos = (double)(ConfigDictionary())["fw3_req_pos"];
      std::string name = (std::string)(ConfigDictionary())["fw3_name"];
      strncpy(fw3sb->filter_name, name.c_str(), 256);
      fw3sb->type = (int)(ConfigDictionary())["fw3_type"];
      fw3sb->req_type = (int)(ConfigDictionary())["fw3_req_type"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Missing f/w 3 config data");
      throw;
   }
   
   
   
   
   
   //Init the system status board
   rv = create_shmem(&statusboard_shmemid, STATUS_sysmonD, sizeof(system_status_board));
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck, STATUS_sysmonD, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else rv = -1;
   }
   
   if(rv != 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create system status board.");
   }
   else
   {
      syssb = (VisAO::system_status_board *) statusboard_shmemptr;
      strncpy(syssb->appname, "sysmonD.L", 25);
      syssb->max_update_interval = pause_time;
   }
   
   //Set up the system status board
   try
   {
      syssb->Joe47Temp = (double)(ConfigDictionary())["sys_Joe47Temp"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Missing sysmonD config data");
      throw;
   }
   
   
   //Init the wollaston status board
   rv = create_shmem(&statusboard_shmemid, STATUS_wollaston, sizeof(wollaston_status_board));
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck, STATUS_wollaston, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else rv = -1;
   }
   
   if(rv != 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create wollaston status board.");
   }
   else
   {
      wsb = (VisAO::wollaston_status_board *) statusboard_shmemptr;
      strncpy(wsb->appname, "wollaston.L", 25);
      wsb->max_update_interval = pause_time;
   }
   
   //Set up the wollaston status board
   try
   {
      wsb->cur_pos = (double)(ConfigDictionary())["wollaston_cur_pos"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Missing wollaston config data");
      throw;
   }
   
   
    //Init the gimbal status board
   rv = create_shmem(&statusboard_shmemid, STATUS_gimbal, sizeof(gimbal_status_board));
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck, STATUS_gimbal, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else rv = -1;
   }
   
   if(rv != 0)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create gimbal status board.");
   }
   else
   {
      gsb = (VisAO::gimbal_status_board *) statusboard_shmemptr;
      strncpy(gsb->appname, "gimbal.L", 25);
      gsb->max_update_interval = pause_time;
   }
   
   //Set up the gimbal status board
   try
   {
      gsb->xpos = (double)(ConfigDictionary())["gimbal_x_pos"];
      gsb->ypos = (double)(ConfigDictionary())["gimbal_y_pos"];
      gsb->power = (int)(ConfigDictionary())["gimbal_power"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Missing gimbal config data");
      throw;
   }
   
}




int simboard::Run()
{

   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }
   
   
   LOG_INFO("starting up . . .");


      
   while(!TimeToDie)
   {
      sleep(1);
   }

   
   return 0;
   
}//int simboard::Run()


} //namespace VisAO
