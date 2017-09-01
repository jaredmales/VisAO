/************************************************************
*    FilterWheelGUI_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the filter wheel GUI.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file FilterWheelGUI_main.cpp
  * \author Jared R. Males
  * \brief Main program for the filter wheel GUI.
  * 
*/

#include "basicfilterwheelform.h"

#define VISAO_APP_TYPE VisAO::BasicFilterWheelForm
#define VISAO_APP_NAME "filterwheel2GUI"
//#define VISAO_APP_CONFFILE "conf/FilterWheel2GUI.conf"
#include "VisAOGUI_main.h"

int main(int argc, char **argv)
{
   return VisAOGUI_main(argc, argv);
}




