

#include "ui_BasictempmonForm.h"

#include "BasictempmonForm.h"

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

static   QString goodStyle = "";
static   QString warnStyle = "background-color : yellow; color : black; ";
static   QString limitStyle = "background-color : red; color : black; ";

BasictempmonForm::BasictempmonForm(QWidget * Parent, Qt::WindowFlags f): QWidget(Parent, f)
{
}
   
BasictempmonForm::BasictempmonForm(int argc, char **argv, QWidget * Parent, Qt::WindowFlags f): QWidget(Parent, f), VisAO::VisAOApp_standalone(argc, argv)
{
   Create();
}

BasictempmonForm::BasictempmonForm(std::string name, const std::string& conffile, QWidget * Parent, Qt::WindowFlags f): QWidget(Parent, f), VisAO::VisAOApp_standalone(name, conffile)
{
   Create();
}
      
void BasictempmonForm::Create()
{
   ui.setupUi(this); 
   
   //Init the status board
   size_t sz;
   statusboard_shmemkey = 10000;
   statusboard_shmemptr = attach_shm(&sz,  statusboard_shmemkey , 0);
   
   ccd47sb = (ccd47_status_board*) attach_shm(&sz,  STATUS_ccd47, 0);

   statustimeout = 1000;
   connect(&statustimer, SIGNAL(timeout()), this, SLOT(update_status()));

      

   statustimer.start(statustimeout);

   

}
            
		     
void BasictempmonForm::update_status()
{
   char str[25];
   size_t sz;


   if(!statusboard_shmemptr) statusboard_shmemptr = attach_shm(&sz,  statusboard_shmemkey , 0);

   if(statusboard_shmemptr)
   {
      VisAO::system_status_board * ssb = (VisAO::system_status_board *) statusboard_shmemptr;
      
      core_temp_warn  =   ssb->core_temp_warn;
      core_temp_limit =   ssb->core_temp_limit;

      gpu_temp_warn   =   ssb->gpu_temp_warn;  
      gpu_temp_limit  =   ssb->gpu_temp_limit; 

      hdd_temp_warn   =   ssb->hdd_temp_warn; 
      hdd_temp_limit  =   ssb->hdd_temp_limit; 
      
      air_temp_warn   =   ssb->air_temp_warn; 
      air_temp_limit  =   ssb->air_temp_limit; 
      
      joe_temp_warn  = ssb->joe_temp_warn;
      joe_temp_limit = ssb->joe_temp_limit;
      
      if((get_curr_time() - ts_to_curr_time(&ssb->last_update)) < ssb->max_update_interval)
      {
         core_avg = 0;   
         for(int i=0;i<SYS_N_CORES; i++)
         {
            core_temps[i] = ssb->core_temps[i];
            core_avg += core_temps[i];
         }
         core_avg /= SYS_N_CORES;

         GPUTemp = ssb->GPUTemp;
         HDDTemp_a = ssb->HDDTemp_a;
         HDDTemp_b = ssb->HDDTemp_b;
         AirTemp = ssb->AirTemp;
         ExhTemp = ssb->Joe47Temp;

         ui.tempBar_1->setValue((int)(core_temps[0] + .5));
         ui.tempBar_2->setValue((int)(core_temps[1] + .5));
         ui.tempBar_3->setValue((int)(core_temps[2] + .5));
         ui.tempBar_4->setValue((int)(core_temps[3] + .5));
         ui.tempBar_5->setValue((int)(core_temps[4] + .5));
         ui.tempBar_6->setValue((int)(core_temps[5] + .5));
         ui.tempBar_Avg->setValue((int)(core_avg + .5));

         snprintf(str, 25, "%i", (int)(core_temps[0]+.5));
         ui.tempCore_1->setText(str);
         if(core_temps[0] >= core_temp_limit) ui.tempCore_1->setStyleSheet(limitStyle);
         else if(core_temps[0] >= core_temp_warn) ui.tempCore_1->setStyleSheet(warnStyle);
         else ui.tempCore_1->setStyleSheet(goodStyle);

         snprintf(str, 25, "%i", (int)(core_temps[1]+.5));
         ui.tempCore_2->setText(str);
         if(core_temps[1] >= core_temp_limit) ui.tempCore_2->setStyleSheet(limitStyle);
         else if(core_temps[1] >= core_temp_warn) ui.tempCore_2->setStyleSheet(warnStyle);
         else ui.tempCore_2->setStyleSheet(goodStyle);

         snprintf(str, 25, "%i", (int)(core_temps[2]+.5));
         ui.tempCore_3->setText(str);
         if(core_temps[2] >= core_temp_limit) ui.tempCore_3->setStyleSheet(limitStyle);
         else if(core_temps[2] >= core_temp_warn) ui.tempCore_3->setStyleSheet(warnStyle);
         else ui.tempCore_3->setStyleSheet(goodStyle);

         snprintf(str, 25, "%i", (int)(core_temps[3]+.5));
         ui.tempCore_4->setText(str);
         if(core_temps[3] >= core_temp_limit) ui.tempCore_4->setStyleSheet(limitStyle);
         else if(core_temps[3] >= core_temp_warn) ui.tempCore_4->setStyleSheet(warnStyle);
         else ui.tempCore_4->setStyleSheet(goodStyle);

         snprintf(str, 25, "%i", (int)(core_temps[4]+.5));
         ui.tempCore_5->setText(str);
         if(core_temps[4] >= core_temp_limit) ui.tempCore_5->setStyleSheet(limitStyle);
         else if(core_temps[4] >= core_temp_warn) ui.tempCore_5->setStyleSheet(warnStyle);
         else ui.tempCore_5->setStyleSheet(goodStyle);

         snprintf(str, 25, "%i", (int)(core_temps[5]+.5));
         ui.tempCore_6->setText(str);
         if(core_temps[5] >= core_temp_limit) ui.tempCore_6->setStyleSheet(limitStyle);
         else if(core_temps[5] >= core_temp_warn) ui.tempCore_6->setStyleSheet(warnStyle);
         else ui.tempCore_6->setStyleSheet(goodStyle);

         snprintf(str, 25, "%i", (int)(core_avg+.5));
         ui.tempCore_Avg->setText(str);
         if(core_avg >= core_temp_limit) ui.tempCore_Avg->setStyleSheet(limitStyle);
         else if(core_avg >= core_temp_warn) ui.tempCore_Avg->setStyleSheet(warnStyle);
         else ui.tempCore_Avg->setStyleSheet(goodStyle);

         ui.tempBar_GPU->setValue((int)(GPUTemp + .5));
         snprintf(str, 25, "%i", (int)(GPUTemp+.5));
         ui.tempGPU->setText(str);
         if(GPUTemp >= gpu_temp_limit) ui.tempGPU->setStyleSheet(limitStyle);
         else if(GPUTemp >= gpu_temp_warn) ui.tempGPU->setStyleSheet(warnStyle);
         else ui.tempGPU->setStyleSheet(goodStyle);

         ui.tempBar_HDD_a->setValue((int)(HDDTemp_a + .5));
         snprintf(str, 25, "%i", (int)(HDDTemp_a+.5));
         ui.tempHDD_a->setText(str);
         if(HDDTemp_a >= hdd_temp_limit) ui.tempHDD_a->setStyleSheet(limitStyle);
         else if(HDDTemp_a >= hdd_temp_warn) ui.tempHDD_a->setStyleSheet(warnStyle);
         else ui.tempHDD_a->setStyleSheet(goodStyle);

         ui.tempBar_HDD_b->setValue((int)(HDDTemp_b + .5));
         snprintf(str, 25, "%i", (int)(HDDTemp_b+.5));
         ui.tempHDD_b->setText(str);
         if(HDDTemp_b >= hdd_temp_limit) ui.tempHDD_b->setStyleSheet(limitStyle);
         else if(HDDTemp_b >= hdd_temp_warn) ui.tempHDD_b->setStyleSheet(warnStyle);
         else ui.tempHDD_b->setStyleSheet(goodStyle);

         if(AirTemp > -100)
         {
            ui.tempBar_Air->setValue((int)(AirTemp + .5));
            snprintf(str, 25, "%0.1f", AirTemp);
            ui.tempAir->setText(str);
            if(AirTemp >= air_temp_limit) ui.tempAir->setStyleSheet(limitStyle);
            else if(AirTemp >= air_temp_warn) ui.tempAir->setStyleSheet(warnStyle);
            else ui.tempAir->setStyleSheet(goodStyle);
         }
         else 
         {
            ui.tempBar_Air->setValue(0);
            ui.tempAir->setText("?");
         }

         if(ExhTemp > -100)
         {
            ui.tempBar_JoeExh->setValue((int)(ExhTemp + .5));
            snprintf(str, 25, "%0.1f", ExhTemp);
            ui.tempJoeExh->setText(str);
         }
         else 
         {
            ui.tempBar_JoeExh->setValue(0);
            ui.tempJoeExh->setText("?");
         }
      }
      else
      {
         ui.tempBar_1->setValue(0);
         ui.tempBar_2->setValue(0);
         ui.tempBar_3->setValue(0);
         ui.tempBar_4->setValue(0);
         ui.tempBar_5->setValue(0);
         ui.tempBar_6->setValue(0);
         ui.tempBar_Avg->setValue(0);
         ui.tempCore_1->setText("?");
         ui.tempCore_2->setText("?");
         ui.tempCore_3->setText("?");
         ui.tempCore_4->setText("?");
         ui.tempCore_5->setText("?");
         ui.tempCore_6->setText("?");
         ui.tempCore_Avg->setText("?");
         
         ui.tempBar_GPU->setValue(0);
         ui.tempGPU->setText("?");

         ui.tempBar_HDD_a->setValue(0);
         ui.tempHDD_a->setText("?");

         ui.tempBar_HDD_b->setValue(0);
         ui.tempHDD_b->setText("?");

         ui.tempBar_Air->setValue(0);
         ui.tempAir->setText("?");

         ui.tempBar_JoeExh->setValue(0);
         ui.tempJoeExh->setText("?"); 
      }    
   }
   else
   {
      ui.tempBar_1->setValue(0);
      ui.tempBar_2->setValue(0);
      ui.tempBar_3->setValue(0);
      ui.tempBar_4->setValue(0);
      ui.tempBar_5->setValue(0);
      ui.tempBar_6->setValue(0);
      ui.tempBar_Avg->setValue(0);
      ui.tempCore_1->setText("?");
      ui.tempCore_2->setText("?");
      ui.tempCore_3->setText("?");
      ui.tempCore_4->setText("?");
      ui.tempCore_5->setText("?");
      ui.tempCore_6->setText("?");
      ui.tempCore_Avg->setText("?");
      
      ui.tempBar_GPU->setValue(0);
      ui.tempGPU->setText("?");

      ui.tempBar_HDD_a->setValue(0);
      ui.tempHDD_a->setText("?");

      ui.tempBar_HDD_b->setValue(0);
      ui.tempHDD_b->setText("?");

      ui.tempBar_Air->setValue(0);
      ui.tempAir->setText("?");

      ui.tempBar_JoeExh->setValue(0);
      ui.tempJoeExh->setText("?");
   }


   if(!ccd47sb) ccd47sb = (ccd47_status_board*) attach_shm(&sz,  STATUS_ccd47, 0);

   if(ccd47sb)
   {
      if((get_curr_time() - ts_to_curr_time(&ccd47sb->last_update)) < ccd47sb->max_update_interval)
      {      
         
         snprintf(str, 25, "%0.2f", ccd47sb->joe_temp);
         ui.tempJoeInt->setText(str);
         
         if(ccd47sb->joe_temp >= joe_temp_limit) ui.tempJoeInt->setStyleSheet(limitStyle);
         else if(ccd47sb->joe_temp >= joe_temp_warn) ui.tempJoeInt->setStyleSheet(warnStyle);
         else ui.tempJoeInt->setStyleSheet(goodStyle);
         
         ui.tempBar_JoeInt->setValue((int)(ccd47sb->joe_temp + .5));
         
         snprintf(str, 25, "%0.2f", ccd47sb->head_temp1);
         ui.temp471->setText(str);
         ui.tempBar_471->setValue((int)( (ccd47sb->head_temp1/-50.) *100.));

         snprintf(str, 25, "%0.2f", ccd47sb->head_temp2);
         ui.temp472->setText(str);
         ui.tempBar_472->setValue((int)((ccd47sb->head_temp2/-50.)*100.));
      }
      else
      {
         ui.tempJoeInt->setText("?");
         ui.tempBar_JoeInt->setValue(0);
         ui.temp471->setText("?");
         ui.tempBar_471->setValue(0);
         ui.temp472->setText("?");
         ui.tempBar_472->setValue(0);
      }   
   }
   else
   {
      ui.tempJoeInt->setText("?");
      ui.tempBar_JoeInt->setValue(0);
      ui.temp471->setText("?");
      ui.tempBar_471->setValue(0);
      ui.temp472->setText("?");
      ui.tempBar_472->setValue(0);
   }




   return;
}

		
}//namespace VisAO
