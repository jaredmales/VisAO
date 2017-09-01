

#include "ui_basicsysmonDForm.h"

#include "BasicsysmonDForm.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include <QWidget>
#include <QLabel>
#include <QTimer>

#include "VisAOApp_standalone.h"
#include "libvisao.h"
#include "statusboard.h"

namespace VisAO
{

BasicsysmonDForm::BasicsysmonDForm(QWidget * Parent, Qt::WindowFlags f): QWidget(Parent, f)
{
}
   
BasicsysmonDForm::BasicsysmonDForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f): QWidget(Parent, f), VisAO::VisAOApp_standalone(argc, argv)
{
   Create();
}

BasicsysmonDForm::BasicsysmonDForm(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f): QWidget(Parent, f), VisAO::VisAOApp_standalone(name, conffile)
{
   Create();
}
      
void BasicsysmonDForm::Create()
{
   ui.setupUi(this); 
   
   //Init the status board
   size_t sz;
   statusboard_shmemkey = 10000;
   statusboard_shmemptr = attach_shm(&sz,  statusboard_shmemkey , 0);
   
   statustimeout = 1000;
   connect(&statustimer, SIGNAL(timeout()), this, SLOT(update_status()));
   

   statustimer.start(statustimeout);
}
            
		     
void BasicsysmonDForm::update_status()
{
   char str[25];
   if(statusboard_shmemptr)
   {
      VisAO::system_status_board * ssb = (VisAO::system_status_board *) statusboard_shmemptr;
   
      for(int i=0;i<SYS_N_CORES; i++)
      {
         core_temps[i] = ssb->core_temps[i];
         core_max[i] = ssb->core_max[i];
      }
      

      for(int i=0;i<SYS_N_VCORES; i++)
      {
         core_idle[i] = ssb->core_idle[i];
      }
      ui.loadBar_1->setValue((int)(100.0 - core_idle[0] + .5));
      ui.loadBar_2->setValue((int)(100.0 - core_idle[1] + .5));
      ui.loadBar_3->setValue((int)(100.0 - core_idle[2] + .5));
      ui.loadBar_4->setValue((int)(100.0 - core_idle[3] + .5));
      ui.loadBar_5->setValue((int)(100.0 - core_idle[4] + .5));
      ui.loadBar_6->setValue((int)(100.0 - core_idle[5] + .5));
      ui.loadBar_7->setValue((int)(100.0 - core_idle[6] + .5));
      ui.loadBar_8->setValue((int)(100.0 - core_idle[7] + .5));
      ui.loadBar_9->setValue((int)(100.0 - core_idle[8] + .5));
      ui.loadBar_10->setValue((int)(100.0 - core_idle[9] + .5));
      ui.loadBar_11->setValue((int)(100.0 - core_idle[10] + .5));
      ui.loadBar_12->setValue((int)(100.0 - core_idle[11] + .5));

      mem_tot = ssb->mem_tot;
      mem_used = ssb->mem_used;
      mem_free = ssb->mem_free;
      mem_shared = ssb->mem_shared;
      mem_buff = ssb->mem_buff;
      mem_cached = ssb->mem_cached;

      ui.mainMemBar->setValue((int)((100.* ((double) (mem_tot - mem_free)))/((double)mem_tot) + .5));
      snprintf(str, 25, "%0.1f", ((double)mem_free)/((double)1048576));
      ui.mainMemFree->setText(str);

      swap_tot = ssb->swap_tot;
      swap_used = ssb->swap_used;
      swap_free = ssb->swap_free;

      ui.swapMemBar->setValue((int)((100.*(double) swap_used)/((double)swap_tot) + .5));
      snprintf(str, 25, "%0.1f", ((double)swap_free)/((double)1048576));
      ui.swapMemFree->setText(str);
      dfroot_size = ssb->dfroot_size;
      dfroot_used = ssb->dfroot_used;
      dfroot_avail = ssb->dfroot_avail;
      
      ui.diskBar->setValue((int)((100.*(double) dfroot_used)/((double)dfroot_size) + .5));
      snprintf(str, 25, "%0.1f", ((double)dfroot_avail)/((double)1048576));
      ui.diskSpaceFree->setText(str);

      snprintf(str, 25, "[%c %c] [%c %c]", ssb->raid_stat[0], ssb->raid_stat[1], ssb->raid_stat[4], ssb->raid_stat[5]);
      ui.raidStatus->setText(str);
   }
   else
   {
      size_t sz;
      statusboard_shmemptr = attach_shm(&sz,  statusboard_shmemkey , 0);
   }
   return;
}

		
}//namespace VisAO
