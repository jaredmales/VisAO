/************************************************************
*    ShutterControl_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for shutter control.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterControl_main.cpp
  * \author Jared R. Males
  * \brief Main program for shutter control.  
  * 
*/

#include "ShutterControlDioclient.h"


#define VISAO_APP_TYPE VisAO::ShutterControlDioclient
#define VISAO_APP_NAME "shuttercontrol"

#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}

