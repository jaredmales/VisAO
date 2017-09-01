/************************************************************
*    sysmonD.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for the sysmonD system monitor
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file sysmonD.cpp
  * \author Jared R. Males
  * \brief Definitions for the sysmonD system monitor
  *
*/

#include "sysmonD.h"
#include <setjmp.h>

//extern int qtitimeout;

namespace VisAO
{

sysmonD::sysmonD(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   init_sysmonD();
}
      
sysmonD::sysmonD(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   init_sysmonD();
}

void sysmonD::init_sysmonD()
{
   //Get the QTI Air Temp Probe serial number
   try
   {
      qtiAirSerialNo = (std::string)(ConfigDictionary())["airTemp_serial_no"];
      haveQTIAir = true;
      _logger->log(Logger::LOG_LEV_INFO, "Set air temp probe ser. no. to: %s", qtiAirSerialNo.c_str());
   }
   catch(Config_File_Exception)
   {
      haveQTIAir = false;
      _logger->log(Logger::LOG_LEV_INFO, "Air temp probe ser. no. not found");
   }
   
   //Get the QTI Joe 47 Temp Probe serial number
   try
   {
      qtiJoe47SerialNo = (std::string)(ConfigDictionary())["joe47Temp_serial_no"];
      haveQTIJoe47 = true;
      _logger->log(Logger::LOG_LEV_INFO, "Set joe 47 temp probe ser. no. to: %s", qtiJoe47SerialNo.c_str());
   }
   catch(Config_File_Exception)
   {
      haveQTIJoe47 = false;
      _logger->log(Logger::LOG_LEV_INFO, "Joe 47 temp probe ser. no. not found");
   }

   //Get the QTI Ambient Temp Probe serial number
   try
   {
      qtiAmbSerialNo = (std::string)(ConfigDictionary())["ambTemp_serial_no"];
      haveQTIAmb = true;
      _logger->log(Logger::LOG_LEV_INFO, "Set ambient temp probe ser. no. to: %s", qtiAmbSerialNo.c_str());
   }
   catch(Config_File_Exception)
   {
      haveQTIAmb = false;
      _logger->log(Logger::LOG_LEV_INFO, "Ambient temp probe ser. no. not found");
   }

   nprobes = 3;

   //Init the status board
   statusboard_shmemkey = STATUS_sysmonD;
   if(create_statusboard(sizeof(VisAO::system_status_board)) != 0)
   {
      statusboard_shmemptr = 0;
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create status board.");
   }
   else
   {
      VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;
      strncpy(bsb->appname, MyFullName().c_str(), 25);
      bsb->max_update_interval = 5.*pause_time;
   }
   
   _tempsLogger = Logger::get("VisAO Temperature", Logger::LOG_LEV_INFO, "TELEMETRY");

   //Read config data.
   try
   {
      core_temp_warn = (double)(ConfigDictionary())["core_temp_warn"];
      core_temp_limit = (double)(ConfigDictionary())["core_temp_limit"];
      hdd_used_warn = (double)(ConfigDictionary())["hdd_used_warn"];
      hdd_used_limit = (double)(ConfigDictionary())["hdd_used_limit"];
      hdd_temp_warn = (double)(ConfigDictionary())["hdd_temp_warn"];
      hdd_temp_limit = (double)(ConfigDictionary())["hdd_temp_limit"];
      gpu_temp_warn = (double)(ConfigDictionary())["gpu_temp_warn"];
      gpu_temp_limit = (double)(ConfigDictionary())["gpu_temp_limit"];
      air_temp_warn = (double)(ConfigDictionary())["air_temp_warn"];
      air_temp_limit = (double)(ConfigDictionary())["air_temp_limit"];
      joe_temp_warn = (double)(ConfigDictionary())["joe_temp_warn"];
      joe_temp_limit = (double)(ConfigDictionary())["joe_temp_limit"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "Missing warning and limit data");
      throw;
   }
         
   
}

int sysmonD::get_sysstat()
{
   double cpun, user, nice, sys, iowait, irq, soft, steal, intrs;

   std::string com;
   com = "sh ";
   com += getenv("VISAO_ROOT");
   com += "/bin/get_sysstat.sh";
   //system("sh ./get_sysstat.sh");
   //std::cout << com << "\n";

   system(com.c_str());

   fin.open("sysstat.txt");

   for(int i=0; i<SYS_N_CORES; i++)
   {
      fin >> core_temps[i];
      fin >> core_max[i];
   }

   fin >> mem_tot;
   fin >> mem_used;
   fin >> mem_free;
   fin >> mem_shared;
   fin >> mem_buff;
   fin >> mem_cached;
   fin >> mem_buff;
   fin >> mem_free;
   std::cout << "mem_free: " << mem_free << "\n";
   
   fin >> swap_tot;
   fin >> swap_used;
   fin >> swap_free;

   fin >> dfroot_size;
   fin >> dfroot_used;
   fin >> dfroot_avail;

   for(int i=0; i<SYS_N_VCORES; i++)
   {
      fin >> cpun;
      fin >> user;
      fin >> nice;
      fin >> sys;
      fin >> iowait;
      fin >> irq;
      fin >> soft;
      fin >> steal;
      fin >> core_idle[i];
      fin >> intrs;
   }

   fin.close();

   visao_mdstat(raid_stat);

   return 0;
}

int sysmonD::get_GPUstat()
{
   std::string tmp;
   system("nvidia-smi | grep Default > GPUstat.txt");

   fin.open("GPUstat.txt");

   fin >> tmp;

   fin >> tmp;

   fin >> GPUTemp;

   for(int i=0;i<6; i++)   fin >> tmp;

   fin >> GPUMemUsage;

   fin.close();

   return 0;

}

int sysmonD::get_HDDstat()
{
   std::string tmp;

   system("/usr/sbin/smartctl /dev/sda --all | grep Temperature_Celsius > HDDstat.txt");

   fin.open("HDDstat.txt");

   for(int i=0; i < 10;i++) fin >> tmp;

   fin.close();


   HDDTemp_a = atoi(tmp.c_str());
   std::cout << HDDTemp_a << "\n";

   system("/usr/sbin/smartctl /dev/sdb --all | grep Temperature_Celsius > HDDstat.txt");
   fin.open("HDDstat.txt");
   for(int i=0; i < 10;i++) fin >> tmp;
   fin.close();

   HDDTemp_b = atoi(tmp.c_str());

   return 0;
}

sysmonD * global_sysmonD;

static void timeout_handler(int sig, siginfo_t *si, void *uc)
{
   try{
   global_sysmonD->qtiTimeout();
   }
   catch(...)
   {
      std::cout << "caught in  handler\n";
   }
   std::cout << "Sighandler returning" << std::endl;
}



int sysmonD::setupQTI()
{
   timer_t timerid;
   struct sigevent sev;
   struct itimerspec its, itsdisarm;
   long long freq_nanosecs;
   
   struct sigaction sa;

   global_sysmonD = this;

   sa.sa_flags = SA_SIGINFO;
   sa.sa_sigaction = timeout_handler;
   sigemptyset(&sa.sa_mask);
   sigaction(RTSIGTIMEOUT, &sa, NULL);
   
   
   sev.sigev_notify = SIGEV_SIGNAL;
   sev.sigev_signo = RTSIGTIMEOUT;
   sev.sigev_value.sival_ptr = &timerid;

   
   freq_nanosecs = (long long) 2.*1e9;
   its.it_value.tv_sec = freq_nanosecs/1000000000;
   its.it_value.tv_nsec = freq_nanosecs % 1000000000;
   its.it_interval.tv_sec = its.it_value.tv_sec;
   its.it_interval.tv_nsec = its.it_value.tv_nsec;

   itsdisarm.it_value.tv_sec = 0;
   itsdisarm.it_value.tv_nsec = 0;
   itsdisarm.it_interval.tv_sec = 0;
   itsdisarm.it_interval.tv_nsec = 0;

   

   QTITempProbe qti;

   qtiProbes = new QTITempProbe[nprobes];

   char devpath[25];

   set_euid_called();

   timer_create(CLOCK_REALTIME, &sev, &timerid);

   for(int i=0; i < nprobes; i++)
   {
      snprintf(devpath,25, "/dev/ttyACM%i", i);
      std::cout << "Reading: " << devpath << "\n";
      qtiProbes[i].setDevPath(devpath);
      //std::cout << "1" << std::endl;

   
      timer_settime(timerid, 0, &its, 0);
      qtiProbes[i].readSerialNumber();
      timer_settime(timerid, 0, &itsdisarm, 0);
      //timer_delete(timerid);
      //std::cout << "timer deleted\n";

//       if(qtitimeout) 
//       {
//          std::cout << "returning\n";
//          return -1;
// 
//          //sigaction(RTSIGTIMEOUT, &sa, NULL);
//          //longjmp(env, 1);
//       }

      std::cout << "Got: " << qtiProbes[i].getSerialNumber() << "\n";
      

   }

   set_euid_real();

   if(haveQTIAir)
   {
      qtiAirIndex = -1;
      for(int i=0; i<nprobes; i++)
      {
         if(qtiProbes[i].getSerialNumber() == qtiAirSerialNo)
         {
            _logger->log(Logger::LOG_LEV_INFO, "Found Air Temp. Probe.");
            qtiAirIndex = i;
            break;
         }
      }
      if(qtiAirIndex == -1)
      {
         _logger->log(Logger::LOG_LEV_INFO, "Could not find Air Temp. Probe.");
      }
   }

   if(haveQTIJoe47)
   {
      qtiJoe47Index = -1;
      for(int i=0; i<nprobes; i++)
      {
         if(qtiProbes[i].getSerialNumber() == qtiJoe47SerialNo)
         {
            _logger->log(Logger::LOG_LEV_INFO, "Found Joe 47 Temp. Probe.");
            qtiJoe47Index = i;
            break;
         }
      }
      if(qtiJoe47Index == -1)
      {
         _logger->log(Logger::LOG_LEV_INFO, "Could not find Joe47 Temp. Probe.");
      }
   }

   if(haveQTIAmb)
   {
      qtiAmbIndex = -1;
      for(int i=0; i<nprobes; i++)
      {
         if(qtiProbes[i].getSerialNumber() == qtiAmbSerialNo)
         {
            _logger->log(Logger::LOG_LEV_INFO, "Found Ambient Temp. Probe.");
            qtiAmbIndex = i;
            break;
         }
      }
      if(qtiAmbIndex == -1) 
      {
         _logger->log(Logger::LOG_LEV_INFO, "Could not find Ambient Temp. Probe.");
      }
   }

   std::cout << "Done initing QTI devices" << std::endl;

   return 0;
}

jmp_buf env;

void sysmonD::qtiTimeout()
{
   
   ERROR_REPORT("Timed out looking for QTI teperature devices.");
   longjmp(env,1);

}




int sysmonD::get_QTITemps()
{
   int rv;

   rv = set_euid_called();

   if(haveQTIAir && qtiAirIndex > -1)
   {
      airTemp = qtiProbes[qtiAirIndex].getTemperature();
      if(airTemp < -100 || airTemp > 500) airTemp = 0.0;
   }
   else airTemp = -100.;
   if(airTemp == -1000)
   {
      std::cout << "Timed out\n";
   }
   if(haveQTIJoe47 && qtiJoe47Index > -1)
   {
      joe47Temp = qtiProbes[qtiJoe47Index].getTemperature();
      if(joe47Temp < -100 || joe47Temp > 500) joe47Temp = 0.0;
   }
   else joe47Temp = -100.;
   if(joe47Temp == -1000)
   {
      std::cout << "Timed out\n";
   }

   if(haveQTIAmb && qtiAmbIndex > -1)
   {
      ambTemp = qtiProbes[qtiAmbIndex].getTemperature();
      if(ambTemp < -100 || ambTemp > 500) ambTemp = 0.0;
      std::cout << ambTemp << "\n";
   }
   else ambTemp = -100.;

   rv = set_euid_real();
   return 0;
}


int sysmonD::Run()
{
   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }

   if (setjmp(env))   
   {
      //std::cout << "we're back \n";
      haveQTIAir = 0;
      haveQTIJoe47 = 0;
      haveQTIAmb = 0;
      set_euid_real();
      //delete[] qtiProbes;
   }
   else setupQTI();

   while(!TimeToDie) 
   {
      //std::cout << 1 << std::endl;
      get_sysstat();
      //std::cout << 2 << std::endl;
      get_GPUstat();
      //std::cout << 3 << std::endl;
      get_HDDstat();
      //std::cout << 4 << std::endl;
      get_QTITemps();
      //std::cout << 5 << std::endl;
      update_statusboard();
      
      if(pause_time >= 1) sleep(pause_time);
      else usleep((int)(pause_time * 1e6));
   }   

   return 0;
}

int sysmonD::update_statusboard()
{
   std::string logs;
   char logstr[256];

   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      VisAO::system_status_board * ssb = (VisAO::system_status_board *) statusboard_shmemptr;
   
      for(int i=0;i<SYS_N_CORES; i++)
      {
         ssb->core_temps[i] = core_temps[i];
         ssb->core_max[i] = core_max[i];

         //std::cout << ssb->core_temps[i] << "/" << ssb->core_max[i] << "\n";
      }

      for(int i=0;i<SYS_N_VCORES; i++)
      {
         ssb->core_idle[i] = core_idle[i];
      }

      for(int i =0; i < SYS_N_LOGDRV; i++)
      {
         ssb->raid_stat[i] = raid_stat[i];
      }


      ssb->mem_tot = mem_tot;
      ssb->mem_used = mem_used;
      ssb->mem_free = mem_free;
      ssb->mem_shared = mem_shared;
      ssb->mem_buff = mem_buff;
      ssb->mem_cached = mem_cached;

      ssb->swap_tot = swap_tot;
      ssb->swap_used = swap_used;
      ssb->swap_free = swap_free;

      ssb->dfroot_size = dfroot_size;
      ssb->dfroot_used = dfroot_used;
      ssb->dfroot_avail = dfroot_avail;

      ssb->GPUTemp = GPUTemp;
      ssb->GPUMemUsage = GPUMemUsage;
      ssb->HDDTemp_a = HDDTemp_a;
      ssb->HDDTemp_b = HDDTemp_b;

      ssb->AirTemp = airTemp;
      ssb->Joe47Temp = joe47Temp;

      logs = "";
      for(int i=0;i<SYS_N_CORES; i++)
      {
         snprintf(logstr, 256, "%s %i", logstr, (int) core_temps[i]);
         logs += logstr;
         //std::cout << ssb->core_temps[i] << "/" << ssb->core_max[i] << "\n";
      }

      ssb->core_temp_warn  =  core_temp_warn; 
      ssb->core_temp_limit =  core_temp_limit;
      ssb->hdd_used_warn   =  hdd_used_warn;  
      ssb->hdd_used_limit  =  hdd_used_limit; 
      ssb->hdd_temp_warn   =  hdd_temp_warn; 
      ssb->hdd_temp_limit  =  hdd_temp_limit; 
      ssb->gpu_temp_warn   =  gpu_temp_warn;  
      ssb->gpu_temp_limit  =  gpu_temp_limit; 
      ssb->air_temp_warn   =  air_temp_warn;  
      ssb->air_temp_limit  =  air_temp_limit; 
      ssb->joe_temp_warn   =  joe_temp_warn;  
      ssb->joe_temp_limit  =  joe_temp_limit; 
      
      _tempsLogger->log(Logger::LOG_LEV_INFO, "%s %i %i %i %0.2f %0.2f", logs.c_str(), (int)GPUTemp, (int)HDDTemp_a, (int)HDDTemp_b, airTemp, joe47Temp);

      /*std::cout << ssb->mem_tot << "\n";
      std::cout << ssb->mem_used << "\n";
      std::cout << ssb->mem_free << "\n";
      std::cout << ssb->mem_shared << "\n";
      std::cout << ssb->mem_buff << "\n";
      std::cout << ssb->mem_cached << "\n";
      std::cout << ssb->swap_tot << "\n";
      std::cout << ssb->swap_used << "\n";
      std::cout << ssb->swap_free << "\n";
      std::cout << ssb->dfroot_size << "\n";
      std::cout << ssb->dfroot_used << "\n";
      std::cout << ssb->dfroot_avail << "\n";*/
   }
   return 0;
}

}//namespace VisAO
