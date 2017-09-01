/************************************************************
*    coronguide_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the coronagraph guider.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file coronguide_main.cpp
  * \author Jared R. Males
  * \brief The main program for the Coronagraph Guider
  *
*/

#include "coronguide.h"

#define VISAO_APP_TYPE VisAO::coronguide
#define VISAO_APP_NAME "coronguide"
//#define VISAO_APP_CONFFILE "conf/framewriter47.conf"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}


