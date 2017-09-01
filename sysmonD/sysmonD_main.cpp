/************************************************************
*    sysmonD_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the VisAO system monitor.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file sysmonD_main.cpp
  * \author Jared R. Males
  * \brief The main program for the VisAO system monitor.
  *
*/

#include "sysmonD.h"


#define VISAO_APP_TYPE VisAO::sysmonD
#define VISAO_APP_NAME "sysmonD"
//#define VISAO_APP_CONFFILE "conf/sysmonD.conf"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}
