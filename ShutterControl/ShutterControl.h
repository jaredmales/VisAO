/************************************************************
*    ShutterControl.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Generic shutter controller.  Used for simulations or as base class for real shutter.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterControl.h
  * \author Jared R. Males
  * \brief Declarations for a generic shutter controller.  
  * 
  * Used for simulations or as base class for real shutter.
  * To use as a simulator, define SIMSHUTTER prior to compile and include.  Then time is incremented with
  * each call to open_shutter() or close_shutter().
  *
*/

#ifndef __ShutterControl_h__
#define __ShutterControl_h__

#include <sys/time.h>
#include <iostream>

class ShutterControl
{
   public:
      ShutterControl(); ///< Default constructor
      ShutterControl(double dt, int st); ///< Constructor to set dead_time and sw_state

   protected:
      double dead_time; ///<Time to wait between commanded changes in state
      
      int sw_state; ///<1 is open, -1 is shut, 0 is unknown
      int hw_state; ///<1 is open, -1 is shut, 0 is unknown
      bool ignore_hw_state; ///< For when hw_state is unavailable.
      
      double curr_t; ///<the current time
      double last_t; ///<the time of the last commanded change in state
      
      timeval tp; ///<for use in getting system time
      
      #ifdef SIMSHUTTER
      double delta_t; ///<In simulations, the time steps between calls to open and close shutter.
      #endif
      
	   int initialize_ShutterControl(); ///<Sets the basic parameters to default values
		
   public:
      int set_last_t(double lt);
      
      int open_shutter(void *adata = 0);
      int close_shutter(void *adata = 0);

      virtual int set_state(int st);
      
      virtual int get_state();
      virtual int get_sw_state();
      virtual int get_hw_state();
      
      double get_curr_t();
      
      //Virtual functions, to be overridden by derived classes specific to shutter and controller
		virtual int start_ShutterControl();
      virtual int shutdown_ShutterControl();
      
      #ifdef SIMSHUTTER
      int set_delta_t(dt);
      int get_delta_t();
      #endif
      
   protected:
      virtual int do_shutter_open(void *adata);
      virtual int do_shutter_close(void *adata);
};


#endif //__ShutterControl_h__
      

