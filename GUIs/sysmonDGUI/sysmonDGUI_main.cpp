/************************************************************
*    sysmonDGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the system monitor GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file sysmonDGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the system monitor GUI.  
  * 
*/

#include "BasicsysmonDForm.h"


#define VISAO_APP_TYPE VisAO::BasicsysmonDForm
#define VISAO_APP_NAME "sysmonDGUI"
#define VISAO_APP_CONFFILE "conf/sysmonDGUI.conf"
#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}
