/************************************************************
*    dioserver.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for the dioserver.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file dioserver.cpp
  * \author Jared R. Males
  * \brief Definitions for the dioserver.
  *
*/

#include "dioserver.h"

fifo_list * global_fifo_list;

namespace VisAO
{
   
dioserver::dioserver(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   init_dioserver();
}

dioserver::dioserver(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   init_dioserver();
}

int dioserver::init_dioserver()
{
   int i;
   char chcfg[11];
   
   //Initialize the channel structures
   for(i=0; i < DIO_CHANNELS; i++)
   {
      diosch[i].swchannel = i;
      diosch[i].hwchannel = 0;
      diosch[i].direction = 0;
      diosch[i].enabled = 0;
      diosch[i].dios = this;

   }
   n_enabled = 0;
   
   std::string pathtmp;
   std::string visao_root = getenv("VISAO_ROOT");
   
   //Set up the fifo_path
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["fifo_path"];
   }
   catch(Config_File_Exception)
   {
      pathtmp = "fifos";
   }
   fifo_path = visao_root + "/" + pathtmp + "/diofifo";
   

   _logger->log(Logger::LOG_LEV_INFO, "Set dioserver fifo_path: %s", fifo_path.c_str());
   
   diocard_info = 0;
   diocard_init = 0;
   diocard_sd = 0;
   diocard_write = 0;
   diocard_read = 0;

   //Read the hardware channel config
   int hwcfg, dircfg, encfg;
   std::vector<std::string> chvec;
   
   for(i=0; i < DIO_CHANNELS; i++)
   {
      snprintf(chcfg, 11, "DIOCHAN_%02i", i);
      try
      {
         chvec = (ConfigDictionary())[chcfg];
         hwcfg = atoi(chvec[0].c_str());
         dircfg = atoi(chvec[1].c_str());
         encfg = atoi(chvec[2].c_str());
         set_dioserver_channel(i, hwcfg, dircfg, encfg);
      }
      catch(Config_File_Exception)
      {
         //Don't do anything - just means the channel wasn't config-ed
      }
      
   }

   //Init the status board
   statusboard_shmemkey = STATUS_dioserver;
   if(create_statusboard(sizeof(basic_status_board)) != 0)
   {
      statusboard_shmemptr = 0;
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create status board.");
   }
   else
   {
      VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;
      strncpy(bsb->appname, MyFullName().c_str(), 25);
      bsb->max_update_interval = pause_time;
   }
   
   return 0;
}//int dioserver::init_dioserver()

int dioserver::set_dioserver_channel(int chnum, int hwchan, int dir, int enab)
{
   diosch[chnum].hwchannel = hwchan;
   diosch[chnum].direction = (dir != 0);
   diosch[chnum].enabled = (enab != 0);
   if(diosch[chnum].enabled) n_enabled++;
   _logger->log(Logger::LOG_LEV_INFO, "Set dioserver channel %i: hw channel=%i dir=%i enab=%i", chnum, hwchan, dir, enab);
   return 0;
}

int dioserver::setup_dioserver()
{
   int i, j;
   char rdfname[MAX_FNAME_SZ],wrfname[MAX_FNAME_SZ];
   
   //Setup the fifo_list
   setup_fifo_list(n_enabled+2);
   //Install the command fifos 
   setup_baseApp(1, 1,0,0, false);
   
   //Install the DIO fifos.
   j = 0;
   for(i = 0; i<DIO_CHANNELS; i++)
   {
      if(diosch[i].enabled)
      {
         snprintf(rdfname, MAX_FNAME_SZ, "%s_in_%02i",  fifo_path.c_str(), i);
         snprintf(wrfname, MAX_FNAME_SZ, "%s_out_%02i", fifo_path.c_str(), i);
         
         set_fifo_list_channel(&fl, j, RWBUFF_SZ, rdfname, wrfname, &read_diofifo, (void *)&(diosch[i]));
         j++;
      }
   }
   
   
   return 0;
}//int dioserver::setup_dioserver()


int read_diofifo(fifo_channel *fc)
{
   static int ncalled = 0;
   
   int totbytes;
   int xput, chnum;
   char *rbuffer, wbuffer[RWBUFF_SZ];
   struct timespec ts;
   dioserver_ch * diosch;
   
   diosch = (dioserver_ch *) fc->auxdata;
   chnum = diosch->swchannel;
   
   
   fc->server_response[0] = '\0';
   totbytes = read_fifo_channel(fc);
   rbuffer = fc->server_response;
   
   if(totbytes == 0 || rbuffer[0] == '\0')
   {
      return 0;
   }
   
   if(fc->seqmsg[0] && diosch->dios->get_use_profiler())
   {
      clock_gettime(CLOCK_REALTIME, &ts);
      diosch->dios->get_profile()->logseqmsg(fc->seqmsg, "dio1", &ts);
   }
   
   if(diosch->direction == 0)
   {
      xput = (rbuffer[0] != '0');

      diosch->dios->diocard_write(diosch->dios->diocard_info, chnum, xput);
		
      snprintf(wbuffer, RWBUFF_SZ, "%i", xput);
   }
   else
   {
      //read input here
      xput = diosch->dios->diocard_read(diosch->dios->diocard_info, chnum);
      
      snprintf(wbuffer, RWBUFF_SZ, "%i", xput);
   }
   
   if(fc->seqmsg[0] && diosch->dios->get_use_profiler())
   {
      clock_gettime(CLOCK_REALTIME, &ts);
      diosch->dios->get_profile()->logseqmsg(fc->seqmsg, "dio2", &ts);
   }
   
   write_fifo_channel(fc, wbuffer, strlen(wbuffer)+1);

   return 0;  
}

std::string dioserver::remote_command(std::string com)
{
   if(com ==  "ping") 
   {
      return "1";
   }
   
   return "UNKOWN COMMAND: " + com + "\n";
}

std::string dioserver::local_command(std::string com)
{
   if(com ==  "ping") 
   {
      return "1";
   }
   
   return "UNKOWN COMMAND: " + com + "\n";
}

int dioserver::Run()
{
   
   //Setup the dioserver
   if(setup_dioserver() == 0)
   {
      _logger->log(Logger::LOG_LEV_INFO, "dioserver setup complete.");
   }
   else
   {
      _logger->log(Logger::LOG_LEV_ERROR, "error during dioserver setup.");
      return -1;
   }
   
   //Initialize the diocard
   if(diocard_init(diocard_info) == 0)
   {
      _logger->log(Logger::LOG_LEV_INFO, "Initialized dio card.");
   }
   else
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Dio card initialization failed.");
      #ifdef NODIOCARD
      _logger->log(Logger::LOG_LEV_WARNING, "DIO card initialization error checking disabled.");
      std::cerr << "DIO card initialization error checking disabled." << "\n";
      #else
      return -1;
      #endif
   }
   

   global_fifo_list = &fl;
   signal(SIGIO, SIG_IGN);
   if(connect_fifo_list_nolock() == 0)
   {
      _logger->log(Logger::LOG_LEV_INFO, "fifo_list connected.");
   }
   else
   {
      _logger->log(Logger::LOG_LEV_ERROR, "Error connecting the fifo list.");
      return -1;
   }
   start_profiler();
   
   setup_RTsigio();

   _logger->log(Logger::LOG_LEV_INFO, "dioserver starting up . . .");

   //diocard_write(diocard_info, 9, 0);
  
   while(!TimeToDie) 
   {
      if(fl.RTSIGIO_overflow)
      {
         //Do something here!
      }
      while(fl.tot_pending_reads > 0 && !TimeToDie) 
      {
         fifo_list_do_pending_read(&fl);
         pthread_cond_broadcast(&profile->thcond);
      }

      //While there are no more pending reads, update the status board
      update_statusboard();
      
      //sleep instead of pausing to make sure we keep the profile thread awake.
      #ifdef VISAO_SIMDEV
      sleep(1);
      #else
      usleep(signalth_sleeptime); //But not too long, so process doesn't get into too deep of a sleep.
      //sleep(1); //This makes things sluggish.
      #endif
      pthread_cond_broadcast(&profile->thcond);

   }

   return 0;
}


} //namespace VisAO



