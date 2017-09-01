/************************************************************
*    CCD47Ctrl_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the VisAO CCD47 controller.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file CCD47Ctrl_main.cpp
  * \author Jared R. Males
  * \brief The main program for the VisAO CCD47 controller.
  *
*/


#include "CCD47Ctrl.h"

#define VISAO_APP_TYPE VisAO::CCD47Ctrl
#define VISAO_APP_NAME "ccd47ctrl"
//#define VISAO_APP_CONFFILE "conf/CCD47Ctrl.conf"
#include "VisAO_main.h"


int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}
