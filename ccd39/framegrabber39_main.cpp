/************************************************************
*    framegrabber39_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the CCD39 frame grabber.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber39_main.cpp
  * \author Jared R. Males
  * \brief The main program for the CCD39 frame grabber.
  *
*/


#include "framegrabber39.h"

#define VISAO_APP_TYPE VisAO::framegrabber39
#define VISAO_APP_NAME "framegrabber39"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}



