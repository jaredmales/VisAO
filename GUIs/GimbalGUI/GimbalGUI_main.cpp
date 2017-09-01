/************************************************************
*    GimbalGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the Gimbal GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file GimbalGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the Gimbal GUI.
  * 
*/

#include "basicgimbalform.h"

#define VISAO_APP_TYPE VisAO::BasicGimbalForm
#define VISAO_APP_NAME "gimbalGUI"
#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}




