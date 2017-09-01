/************************************************************
*    FocusMotorCtrl_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the focus stage controller.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file FocusMotorCtrl_main.cpp
  * \author Jared R. Males
  * \brief The main program for the focus stage controller.
  *
  */

#include "FocusMotorCtrl.h"

#define VISAO_APP_TYPE VisAO::FocusMotorCtrl
#define VISAO_APP_NAME "focusmotor"
//#define VISAO_APP_CONFFILE "conf/FocusMotorCtrl.conf"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}
