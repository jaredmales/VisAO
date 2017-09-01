/************************************************************
*    VisAOSimpleMotorCtrl_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Contains the main program for the VisAO Simple Motor Controller.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file FilterWheelLocal_main.cpp
  * \author Jared R. Males
  * \brief Contains the main program for the filter wheel controller.
  * 
*/

#include "FilterWheelLocal.h"


#define VISAO_APP_TYPE VisAO::FilterWheelLocal
#define VISAO_APP_NAME "filterwheel2Local"

#include "VisAO_main.h"

int main(int argc, char **argv)
{
   extern int need_restart;

   need_restart = 1;
   while(need_restart)
   {
      std::cout << "\n\nStarting main thread\n\n";
      VisAO_main(argc, argv);
   }
}

