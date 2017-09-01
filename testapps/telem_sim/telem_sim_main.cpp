/************************************************************
*    telem_sim_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for simulating telemetry.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file telem_sim_main.cpp
  * \author Jared R. Males
  * \brief The main program for the simulated telemetry.
  *
*/


//#include "telem_sim.h"

#include "VisAOApp_standalone.h"

#define VISAO_APP_TYPE VisAO::framegrabber47_sim
#define VISAO_APP_NAME "framegrabber47_sim"
//#define VISAO_APP_CONFFILE "conf/sims/framegrabber_sim.conf"
//#include "VisAO_main.h"

//Globals
int TimeToDie;
int debug;
//std::string global_app_name;

fifo_list * global_fifo_list;

int main(int argc, char **argv)
{
   //return VisAO_main(argc, argv);
   
   int rv;
   size_t sizecheck;
   
   void * statusboard_shmemptr; ///<The pointer to the shared memory block for the statusboard
   key_t statusboard_shmemkey; ///<The key used to lookup the shared memory
   int   statusboard_shmemid; ///<The ID of the shared memory block.
   
   VisAO::aosystem_status_board *aosb;
   
   statusboard_shmemkey = STATUS_aosystem;
   
   rv = create_shmem(&statusboard_shmemid, statusboard_shmemkey, sizeof(VisAO::aosystem_status_board));
   
   if(!rv)
   {
     statusboard_shmemptr = attach_shm(&sizecheck,  statusboard_shmemkey, statusboard_shmemid);
     if(statusboard_shmemptr) rv = 0;
     else 
     {   rv = -1;
         std::cerr << "Didn't work\n";
         exit(0);
     }
   }
     
   
   
   aosb = (VisAO::aosystem_status_board *) statusboard_shmemptr;
   
   std::cerr << "Worked\n";
   
   aosb->pa = 0.;
   while(1)
   {
      aosb->pa += 1.;
      if(aosb->pa > 360.) aosb->pa = 0;
      usleep(250000);
   }
}


