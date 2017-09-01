/************************************************************
*    CCD47GUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the basic CCD47 GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file CCD47GUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the basic CCD47 GUI.  
  * 
*/

#include "basicccd47form.h"

#define VISAO_APP_TYPE VisAO::BasicCCD47CtrlForm
#define VISAO_APP_NAME "ccd47ctrlGUI"
//#define VISAO_APP_CONFFILE "conf/CCD47GUI.conf"
#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}

