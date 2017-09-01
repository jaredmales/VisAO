/************************************************************
*    shutter_tester_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the shutter tester
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file shutter_tester_main.cpp
  * \author Jared R. Males
  * \brief The main program for the shutter tester
  *
*/

#include "shutter_tester.h"


#define VISAO_APP_TYPE VisAO::shutter_tester
#define VISAO_APP_NAME "ShutterTester"
#define VISAO_APP_CONFFILE "conf/sims/ShutterTester.conf"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}

