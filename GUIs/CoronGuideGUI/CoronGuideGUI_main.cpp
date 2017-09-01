/************************************************************
*    CoronGuideGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the Coronagraph Auto Guider
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file CoronGuideGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the Coronagraph Auto Guider  
  * 
*/

#include "coronguideform.h"

#define VISAO_APP_TYPE VisAO::CoronGuideForm
#define VISAO_APP_NAME "coronguideGUI"

#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}

