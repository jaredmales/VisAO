/************************************************************
*    GimbalCtrl_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the gimbal controller.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file GimbalCtrl_main.cpp
  * \author Jared R. Males
  * \brief The main program for the gimbal controller.
  *
  */

#include "GimbalMotorCtrl.h"

#define VISAO_APP_TYPE VisAO::GimbalMotorCtrl
#define VISAO_APP_NAME "gimbal"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}
