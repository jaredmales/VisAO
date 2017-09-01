/************************************************************
*    bcu39GUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the BCU39 Telemetry GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file bcu39GUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the BCU39 Telemetry GUI.
  * 
*/

#include "basicbcu39form.h"

#define VISAO_APP_TYPE VisAO::BasicBCU39Form
#define VISAO_APP_NAME "bcu39GUI"
#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}




