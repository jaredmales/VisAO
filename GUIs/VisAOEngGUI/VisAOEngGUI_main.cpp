/************************************************************
*    VisAOEngGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the VisAO Engineering GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file FocusMotorGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the VisAO Engineering GUI.
  * 
*/

#include "visaoengguiform.h"

#define VISAO_APP_TYPE VisAO::VisAOEngGUIForm
#define VISAO_APP_NAME "visaoengGUI"
//#define VISAO_APP_CONFFILE "conf/VisAOEngGUI.conf"
#include "../VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}




