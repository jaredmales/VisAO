/************************************************************
*    VisAOJoeCtrl_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for CCD47 control from the AO Supervisor.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOJoeCtrl_main.cpp
  * \author Jared R. Males
  * \brief Main program for CCD47 control from the AO Supervisor. 
  * 
  * 
*/

#include "VisAOJoeCtrl.h"

#define VISAO_APP_TYPE VisAO::JoeCtrl
#define VISAO_APP_NAME "ccd47"
//#define VISAO_APP_CONFFILE "conf/left/ccd47/ccd47.conf"
#include "VisAO_main.h"


int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}

