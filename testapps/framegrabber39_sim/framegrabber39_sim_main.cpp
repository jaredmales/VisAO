/************************************************************
*    framegrabber39_sim_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the simulated frame grabber of CCD39 data.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber39_sim_main.cpp
  * \author Jared R. Males
  * \brief The main program for the simulated frame grabber of CCD39 data.
  *
*/


#include "framegrabber39_sim.h"

#define VISAO_APP_TYPE VisAO::framegrabber39_sim
#define VISAO_APP_NAME "framegrabber39_sim"
//#define VISAO_APP_CONFFILE "conf/sims/framegrabber_sim.conf"
#include "VisAO_main.h"


int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}


