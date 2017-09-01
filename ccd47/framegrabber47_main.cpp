/************************************************************
*    framegrabber47_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the CCD47 frame grabber.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber47_main.cpp
  * \author Jared R. Males
  * \brief The main program for the CCD47 frame grabber.
  *
*/


#include "framegrabber47.h"

#define VISAO_APP_TYPE VisAO::framegrabber47
#define VISAO_APP_NAME "framegrabber47"
//#define VISAO_APP_CONFFILE "conf/framegrabber47.conf"
#include "VisAO_main.h"

int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}



