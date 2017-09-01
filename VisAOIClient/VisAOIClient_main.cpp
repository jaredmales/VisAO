/************************************************************
*    VisAOIClient_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for VisAOI system MagAOIClient.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOIClient_main.cpp
  * \author Jared R. Males
  * \brief Main program for VisAOI system MagAOIClient.
  * 
  * 
*/

#include "VisAOIClient.h"

#define VISAO_APP_TYPE VisAO::VisAOIClient
#define VISAO_APP_NAME "visaoiclient"
//#define VISAO_APP_CONFFILE "conf/left/ccd47/ccd47.conf"
#include "VisAO_main.h"


int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}

