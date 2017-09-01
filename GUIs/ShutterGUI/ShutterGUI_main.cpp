/************************************************************
*    ShutterGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the Shutter GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the Shutter GUI.  
  * 
*/

#include "shutterform.h"

#define VISAO_APP_TYPE VisAO::ShutterForm
#define VISAO_APP_NAME "shutterGUI"

#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}

