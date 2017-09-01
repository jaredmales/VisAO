/************************************************************
*    ShutterTesterGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the Shutter Tester GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterTesterGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the Shutter Tester GUI.  
  * 
*/

#include "shuttertesterform.h"

#define VISAO_APP_TYPE VisAO::ShutterTesterForm
#define VISAO_APP_NAME "shuttertesterGUI"
//#define VISAO_APP_CONFFILE "conf/sims/ShutterTesterGUI.conf"
#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}

