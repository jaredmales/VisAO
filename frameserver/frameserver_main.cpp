/************************************************************
*    frameserver_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the frame server.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file frameserver_main.cpp
  * \author Jared R. Males
  * \brief The main program for the frame server.
  *
*/

#include "frameserver.h"


#define VISAO_APP_TYPE VisAO::frameserver
#define VISAO_APP_NAME "frameserver47"

#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}


