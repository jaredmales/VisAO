/************************************************************
 *    PwrMon_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Contains the main program for the power monitor.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file PwrMon_main.cpp
  * \author Jared R. Males
  * \brief Contains the main program for the power monitor.
  * 
*/

#include "PwrMon.h"


#define VISAO_APP_TYPE VisAO::PwrMon
#define VISAO_APP_NAME "visaopwrmon"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}




