/************************************************************
*    ShutterRemoteControl_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for shutter control from the AO Supervisor.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterRemoteControl_main.cpp
  * \author Jared R. Males
  * \brief Main program for shutter control from the AO Supervisor.  
  * 
*/

#include "ShutterRemoteControl.h"


#define VISAO_APP_TYPE VisAO::ShutterRemoteControl
#define VISAO_APP_NAME "shutterremote"
//#define VISAO_APP_CONFFILE "conf/left/ShutterRemoteControl.conf"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}

