/************************************************************
*    framegrabber47_sim_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the simulated frame grabber of CCD47 data.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber47_sim_main.cpp
  * \author Jared R. Males
  * \brief The main program for the simulated frame grabber of CCD47 data.
  *
*/


#include "framegrabber47_sim.h"

#define VISAO_APP_TYPE VisAO::framegrabber47_sim
#define VISAO_APP_NAME "framegrabber47_sim"
//#define VISAO_APP_CONFFILE "conf/sims/framegrabber_sim.conf"
#include "VisAO_main.h"


int main(int argc, char **argv)
{
   return VisAO_main(argc, argv);
}


