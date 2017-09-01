/************************************************************
*    WollastonStatusGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the Wollaston Status GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file WollastonStatusGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the Wollaston Status GUI.
  * 
*/

#include "basicwollastonliftform.h"

#define VISAO_APP_TYPE VisAO::BasicWollastonLiftForm
#define VISAO_APP_NAME "wollastonstatusGUI"
//#define VISAO_APP_CONFFILE "conf/WollastonStatusGUI.conf"
#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}




