/************************************************************
*    frameselector_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the frame selector.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file frameselector_main.cpp
  * \author Jared R. Males
  * \brief The main program for the frame selector.
  *
*/

#include "frameselector.h"

#define VISAO_APP_TYPE VisAO::frameselector
#define VISAO_APP_NAME "frameselector"


#include "VisAO_main.h"

int main(int argc, char **argv)
{
   std::cout.precision(15);
   return VisAO_main(argc, argv);
}

