/************************************************************
*    FocusMotorGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the focus stage motor GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file FocusMotorGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the focus stage motor GUI.  
  * 
*/

#include "basicfocusstageform.h"

#define VISAO_APP_TYPE VisAO::BasicFocusStageForm
#define VISAO_APP_NAME "focusmotorGUI"
//#define VISAO_APP_CONFFILE "conf/FocusMotorGUI.conf"
#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}

