/************************************************************
*    WollastonStatus.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for the VisAO wollaston lift status maintainer.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file WollastonStatus.cpp
  * \author Jared R. Males
  * \brief Definitions for the VisAO wollaston lift status maintainer.
  *
*/

#include "WollastonStatus.h"

namespace VisAO
{

WollastonStatus::WollastonStatus( std::string name, const std::string& conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   initApp();
}

WollastonStatus::WollastonStatus( int argc, char**argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   initApp();
}

void WollastonStatus::initApp()
{
   
   std::string pathtmp;
   std::string visao_root = getenv("VISAO_ROOT");
   
   setup_fifo_list(2);
   setup_baseApp(1, 1, 0, 0, false);
   
   //Init the status board
   statusboard_shmemkey = 11000;
   if(create_statusboard(sizeof(focusstage_status_board)) != 0)
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

   size_t sz;
   fw3sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  9500, 0);

   
}//void WollastonStatus::initapp()


int WollastonStatus::WollastonStatus::set_cur_pos(int cp)
{
   //Avoid prompting until logic in main loop.
   prompt = 0;

   if(cp > 0) cur_pos = 1;
   if(cp == 0) cur_pos = 0;
   if(cp < 0) cur_pos = -1;

   return 0;
}



int WollastonStatus::Run()
{
   double t0, dt;
   
   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }

   request_control(CMODE_LOCAL);

   signal(SIGIO, SIG_IGN);
   if(connect_fifo_list() == 0)
   {
      LOG_INFO("fifo_list connected.");
   }
   else
   {
      ERROR_REPORT("Error connecting the fifo list.");
      TimeToDie = 1;
      return -1;
   }
   
   global_fifo_list = &fl;
   
   setup_RTsigio();
   
   LOG_INFO("starting up . . .");
   
   while(!TimeToDie)
   {
      t0 = get_curr_time();
      
      while((dt = get_curr_time() - t0) < pause_time && !TimeToDie)
      {
         check_fifo_list_RTpending();
         
         if(pause_time - dt >= 1) sleep((int)(pause_time - dt));
         else usleep((int)((pause_time-dt)*1e6 * .99));
      }

      if(!fw3sb)
      {
         size_t sz;
         fw3sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  9500, 0);
      }
      
      prompt = 0;

      if(fw3sb)
      {
         if(fw3sb->state != STATE_OPERATING && fw3sb->state != STATE_HOMING && fw3sb->type != last_type)
         {
            ignored = 0;
         }
      
         if(fw3sb->state != STATE_OPERATING && fw3sb->state != STATE_HOMING && fw3sb->type == FTYPE_SDI && cur_pos != 1 && !ignored)
         {
            prompt = PROMPT_UP;
         }

         if(fw3sb->state != STATE_OPERATING && fw3sb->state != STATE_HOMING && fw3sb->type != FTYPE_SDI && cur_pos != -1 && !ignored)
         {
            prompt = PROMPT_DOWN;
         }

         last_type = fw3sb->type;
      }
      
      if(cur_pos == 0 && !ignored) prompt = PROMPT_INT;

      check_fifo_list_RTpending();
      update_statusboard();
   }

   return 0;
}

std::string  WollastonStatus::remote_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received remote command: %s.", com.c_str());
   resp = common_command(com, CMODE_REMOTE);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to remote command: %s.", resp.c_str());
   return resp;
}

std::string  WollastonStatus::local_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received local command: %s.", com.c_str());
   resp = common_command(com, CMODE_LOCAL);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to local command: %s.", resp.c_str());
   return resp;
}

std::string WollastonStatus::common_command(std::string com, int cmode)
{
   char rstr[50];
   
   if(com == "pos?")
   {
      snprintf(rstr, 50, "%i\n", get_cur_pos());
      return rstr;
   }
   
   if(com == "state?")
   {
      return get_state_str();
   }
   
   if(com == "up")
   {
      if(control_mode == cmode)
      {
         set_cur_pos(1);
         return "0\n";
      }
      else return control_mode_response();
   }
   
   if(com == "down")
   {
      if(control_mode == cmode)
      {
         set_cur_pos(-1);
         return "0\n";
      }
      else return control_mode_response();
   }

   if(com == "ignore")
   {
      if(control_mode == cmode)
      {
         ignored = 1;
         return "0\n";
      }
      else return control_mode_response();
   }
   
   return (std::string("UNKOWN COMMAND: ") + com + "\n");
}

std::string WollastonStatus::get_state_str()
{
   char statestr[50];
   std::string str = control_mode_response();
   
   snprintf(statestr, 50, "%c %2i %i\n", str[0], cur_pos, prompt);
   
   return statestr;
}

int WollastonStatus::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      VisAO::wollaston_status_board * wsb = (VisAO::wollaston_status_board *) statusboard_shmemptr;
      
      wsb->cur_pos = cur_pos;
      
   }
   return 0;
}

} //namespace VisAO

