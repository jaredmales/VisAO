/************************************************************
*    fieldstopGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the fieldstop Telemetry GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file fieldstopGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the fieldstop Telemetry GUI.
  * 
*/

#include "basicfieldstopform.h"

#define VISAO_APP_TYPE VisAO::BasicFieldStopForm
#define VISAO_APP_NAME "fieldstopGUI"
#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}




