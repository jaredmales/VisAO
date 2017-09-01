/************************************************************
*    VisAOSimpleMotorCtrl_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Contains the main program for the VisAO Simple Motor Controller.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOSimpleMotorCtrl_main.cpp
  * \author Jared R. Males
  * \brief Contains the main program for the filter wheel controller.
  * 
*/

#include "VisAOSimpleMotorCtrl.h"


#define VISAO_APP_TYPE VisAO::VisAOSimpleMotorCtrl
#define VISAO_APP_NAME "filterwheel2"

#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}

