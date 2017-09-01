/************************************************************
*    ShutterGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the Frame Selector GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file FrameSelectorGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the Frame Selector GUI.  
  * 
*/

#include "frameselectorform.h"

#define VISAO_APP_TYPE VisAO::FrameSelectorForm
#define VISAO_APP_NAME "frameselectorGUI"

#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}

