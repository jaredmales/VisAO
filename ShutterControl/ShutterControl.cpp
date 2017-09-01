/************************************************************
*    ShutterControl.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Generic shutter controller.  Used for simulations or as base class for real shutter.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterControl.cpp
  * \author Jared R. Males (jrmales@email.arizona.edu)
  * \brief Definitions for a generic shutter controller.  
  * 
  * Declarations in \ref ShutterControl.h
  * 
*/

#include "ShutterControl.h"


ShutterControl::ShutterControl()
{
   initialize_ShutterControl();
}

ShutterControl::ShutterControl(double dt, int st)
{
   initialize_ShutterControl();

   dead_time = dt;

   sw_state = st;
   
   #ifdef SIMSHUTTER
   hw_state = st;
   #else
   hw_state = 0;
   #endif
   
}

int ShutterControl::initialize_ShutterControl()
{
   dead_time = 0;
   
   sw_state = 0;
   hw_state = 0;
   ignore_hw_state = false;
   
   curr_t = 0;
   last_t = 0;
   
   #ifdef SIMSHUTTER
   delta_t = 0.001;
   #endif

   return 0;
}

int ShutterControl::set_last_t(double lt)
{
   last_t = lt;
   return 0;
}

int ShutterControl::open_shutter(void *adata)
{
   int state;
   double ct;
   ct = get_curr_t();

   state = get_state();

   if(state == 1) return sw_state; //already open

   //if(state == 0 && hw_state == 1) return 0; //probably already open.
   if(ct - last_t >= dead_time) //not open, check dead time
   {
      if(do_shutter_open(adata) == 0)
      {
         sw_state = 1; //update state

         hw_state = get_hw_state();
         last_t = ct; //update time

         return 1;
      }
      else
      {
         last_t = ct; //update time
         std::cerr << "Error in do_shutter_open()\n";
         return 0;
      }
   }

   return -1; //still shut
}

int ShutterControl::close_shutter(void *adata)
{
   int state;
   double ct;
   ct = get_curr_t();

   state = get_state();

   if(state == -1) return sw_state; //already shut

   //if(state == 0 && hw_state == -1) return 0; //probably already shut

   if(ct - last_t >= dead_time) // not shut, check dead time
   {
      if(do_shutter_close(adata) == 0)
      {
         sw_state = -1; //update state
         hw_state = get_hw_state();
         last_t = ct; //update time
         return -1;
      }
      else
      {
         last_t = ct; //update time
         std::cerr << "Error in do_shutter_close()\n";
         return 0;
      }
   }

   return 1; //still open
}

int ShutterControl::set_state(int st)
{
   
   if(st > 0) return open_shutter();
   else return close_shutter();
   
}

int ShutterControl::get_state()
{
   
   sw_state = get_sw_state();
   
   hw_state = get_hw_state();
   
   if(sw_state == hw_state) 
   {
      return sw_state;
   }
   return 0;
}

int ShutterControl::get_sw_state()
{
   return sw_state;
}

int ShutterControl::get_hw_state()
{
   return sw_state;
}

double ShutterControl::get_curr_t()
{
   #ifdef SIMSHUTTER
   curr_t+=delta_t; //for simulations
   #else
   gettimeofday(&tp, 0);
   curr_t = tp.tv_sec + tp.tv_usec/1e6;
   #endif
   
   return curr_t;
}

int ShutterControl::do_shutter_open(void *adata)
{
   return 0;
}

int ShutterControl::do_shutter_close(void *adata)
{
   return 0;
}

int ShutterControl::start_ShutterControl()
{
	return 0;
}

int ShutterControl::shutdown_ShutterControl()
{
   return 0;
}


