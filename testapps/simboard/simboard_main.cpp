/************************************************************
*    simboard_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for simulating the status boards.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file simboard_main.cpp
  * \author Jared R. Males
  * \brief The main program for the simulating the status boards.
  *
*/

#include "simboard.h"



#define VISAO_APP_TYPE VisAO::simboard
#define VISAO_APP_NAME "simboard"



#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}


