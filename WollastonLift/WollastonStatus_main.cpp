/************************************************************
*    WollastonStatus_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Main program for the VisAO wollaston lift status maintainer.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file WollastonStatus_main.cpp
  * \author Jared R. Males
  * \brief Main program for the VisAO wollaston lift status maintainer.
  *
*/

#include "WollastonStatus.h"


#define VISAO_APP_TYPE VisAO::WollastonStatus
#define VISAO_APP_NAME "wollastonstatus"
//#define VISAO_APP_CONFFILE "conf/WollastonStatus.conf"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}
