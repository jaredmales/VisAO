/************************************************************
*    frameserver.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for a class for serving frames over UDP.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file frameserver.cpp
  * \author Jared R. Males
  * \brief Definitions for a class for serving frames over UDP.
  * 
  *
*/

#include "frameserver.h"


#ifndef ERROR_REPORT
#define ERROR_REPORT(er) if(global_error_report) (*global_error_report)(er,__FILE__,__LINE__);
#endif

#ifndef LOG_INFO
#define LOG_INFO(li) if(global_log_info) (*global_log_info)(li);
#endif

namespace VisAO
{


frameserver::frameserver(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   Create();
}

frameserver::frameserver(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   Create();
}

int frameserver::Create()
{
   std::string pathtmp;
   
   std::string visao_root = getenv("VISAO_ROOT");
   
   //Set up the ping_fifo_path
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["pingFifoPath"];
   }
   catch(Config_File_Exception)
   {
      pathtmp = "fifos";
   }
   pingFifoPath = visao_root + "/" + pathtmp +"/";
   _logger->log(Logger::LOG_LEV_INFO, "Set pingFifoPath: %s", pingFifoPath.c_str());
   
   
   try
   {
      remoteIP = (std::string)(ConfigDictionary())["remoteIP"];
   }
   catch(Config_File_Exception)
   {
      remoteIP = "127.0.0.1";
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set remoteIP: %s", remoteIP.c_str());
   
   try
   {
      localPort =(ConfigDictionary())["localPort"];
   }
   catch(Config_File_Exception)
   {
      localPort = 2000;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set localPort: %i", localPort);
   
   try
   {
      remotePort = (ConfigDictionary())["remotePort"];
   }
   catch(Config_File_Exception)
   {
      remotePort = 2001;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set remotePort: %i", remotePort);
   
   
   setup_fifo_list(3);
   setup_baseApp(1, 1, 0, 0, false);
   //setup_baseApp();
   
   //set_fifo_list_channel(&fl, 0, RWBUFF_SZ, (char *)std::string(pingFifoPath + "frameserver47_ping_in").c_str(), (char *)std::string(pingFifoPath + "frameserver47_ping_out").c_str(), &frame_ready, (void *)this);

   //No fifo handler, rather we assign it its own signal handler
   set_fifo_list_channel(&fl, FRAME_READY_FIFO_CH, RWBUFF_SZ, (char *)std::string(pingFifoPath + "frameserver47_ping_in").c_str(), (char *)std::string(pingFifoPath + "frameserver47_ping_out").c_str(), 0, (void *)this);
   
   shmem_key = 5000;
   attached_to_shmem     = false;
   
   total_skipped = 0;
   behind = 0;

   //Init the status board
   statusboard_shmemkey = STATUS_frameserver47;
   if(create_statusboard(sizeof(basic_status_board)) != 0)
   {
      statusboard_shmemptr = 0;
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create status board.");
   }
   else
   {
      VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;
      strncpy(bsb->appname, MyFullName().c_str(), 25);
      bsb->max_update_interval = 1;
   }

   //_MAX_TDP_PACKET_SIZE = 6500; //Constants::MAX_TDP_PACKET_SIZE
   //_MAX_ETH_PACKET_SIZE = 6500; //Constants::MAX_ETH_PACKET_SIZE

   _MAX_TDP_PACKET_SIZE = Constants::MAX_TDP_PACKET_SIZE;
   _MAX_ETH_PACKET_SIZE = Constants::MAX_ETH_PACKET_SIZE;

   return 0;
}

int frameserver::setPingFifoPath(std::string &pfp)
{
   pingFifoPath = pfp;
   return 0;
}

int frameserver::setRemoteIp(std::string &ip)
{
   remoteIP = ip;
   return 0;
}

int frameserver::setLocalPort(int lp)
{
   localPort = lp;
   return 0;
}

int frameserver::setRemotePort(int rp)
{
   remotePort = rp;
   return 0;
}

int frameserver::set_sharedim_stack(sharedim_stackS * s)
{
   sis = s;
   return 0;
}

int frameserver::set_shmem_key(int sk)
{
   shmem_key = sk;
   return 0;
}

int frameserver::connect_shmem()
{
   sis = new sharedim_stackS;
   if(sis->attach_shm(shmem_key) != 0)
   {
      ERROR_REPORT("Error attaching to shared memory.");
      attached_to_shmem = false;
      delete sis;
      return -1;
   }
   attached_to_shmem = true;
   return 0;
}

int frameserver::set_sim(sharedimS s)
{
   sim = s;
   return 0;
}

int frameserver::Run()
{
   /*struct sigaction act;
   sigset_t sset;
      
   connect_shmem();
   
   global_fifo_list = &fl;
   
   signal(SIGIO, SIG_IGN);
   if(connect_fifo_list() == 0)
   {
      LOG_INFO("fifo_list connected.");
   }
   else
   {
      ERROR_REPORT("Error connecting the fifo list.");
      return -1;
   }
   
   act.sa_handler = &catch_fifo_response_list;
   act.sa_flags = 0;
   sigemptyset(&sset);
   act.sa_mask = sset;
   
   sigaction(SIGIO, &act, 0);*/

   signal(SIGIO, SIG_IGN);
   signal(RTSIGIO, SIG_IGN);
   
   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }
   
   //Startup the I/O signal handling thread, without INHERIT_SCHED
   if(start_signal_catcher() != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }
   
   sleep(1); //let signal thread get started
   
   //Now Block all I/O signals in this thread.
   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error blicking SIGIO.");
      return -1;
   }
   
   //Setup to catch the ping in this thread
   //The low signal catcher has already blocked it.
   
   fcntl(fl.fifo_ch[FRAME_READY_FIFO_CH].fd_in, F_SETOWN, getpid());
   
   int rv = fcntl(fl.fifo_ch[FRAME_READY_FIFO_CH].fd_in, F_SETSIG, RTSIGPING);
   if(rv < 0)
   {
      logss.str("");
      logss << "Error changing signal: " << strerror(errno);
      ERROR_REPORT(logss.str().c_str());
      return -1;
   }
   
   struct sigaction act;
   sigset_t sset;
   
   act.sa_sigaction = &frame_ready_handler;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&sset);
   act.sa_mask = sset;
   
   errno = 0;
   
   if(sigaction(RTSIGPING, &act, 0) < 0)
   {
      logss.str("");
      logss << "Error setting signal handler for RTSIGPING. Errno says: " << strerror(errno) << ".";
      error_report(Logger::LOG_LEV_ERROR, logss.str());
      return -1;
   }
   
   conn = new UdpConnection(localPort, remoteIP, remotePort, 0);
   
   LOG_INFO("starting up . . .");
   
   while(!TimeToDie)
   {
      sleep(1);
   }
   
   return 0;
}


int frameserver::calc_packetsPerFrame(int frameSizePixels, int bitPerPix)
{
   frameSizeDw = (frameSizePixels * bitPerPix) / (Constants::DWORD_SIZE*8) + 8; // 4 DW for header and 4 for footer
   
   int frameSizeBytes = frameSizeDw * Constants::DWORD_SIZE;
   
   packetsPerFrame = frameSizeBytes / _MAX_TDP_PACKET_SIZE;
   
   if(frameSizeBytes % _MAX_TDP_PACKET_SIZE) 
   {
      packetsPerFrame++;
   }
   
   return 0;
}

   
int frameserver::send_frame()
{
   static int last_image_abs = -1, last_image = -1, last_save_sequence = -1;
   int skipped;
   //timeval tv0, tv1;
   //double dt;
   static int frames = 0;
   struct timespec ts;
   ts.tv_sec = 0;
   //send_skipped = 0;
   
   if(attached_to_shmem == false)
   {
      if(connect_shmem() != 0) return -1;
   }
   
   //Detect a framegrabber restart
   if(sis->get_last_image_abs() < last_image_abs || last_save_sequence != sis->header->save_sequence)
   {
      last_image_abs = sis->get_last_image_abs();
      last_image = sis->get_last_image();
      last_save_sequence = sis->header->save_sequence;
      
      //If we restarted, log it:
      if(sis->get_last_image_abs() < last_image_abs) LOG_INFO("Detected framegrabber restart, resetting");
   }
   
   if(last_image_abs < 0)
   {
      last_image = sis->get_last_image();
      last_save_sequence = sis->header->save_sequence;
      last_image_abs = sis->get_last_image_abs()-1;
   }
   
   DiagnosticUdpHeader* header = (DiagnosticUdpHeader*) &((TDPHeader*)sendBuff)->header;
   
   
   while(last_image_abs < sis->get_last_image_abs() && !TimeToDie)
   {
      last_image_abs = sis->get_last_image_abs();
      
      sim = sis->get_image(last_image);
      int tosend, maxpcksz, pcktsz, bytesent;
      if(sim.nx)
      {
         calc_packetsPerFrame(sim.nx*sim.ny, 16);//sim.depth);
         
         tosend = frameSizeDw*Constants::DWORD_SIZE;
         
         maxpcksz = Constants::MAX_ETH_PACKET_SIZE;
         
         bytesent = 0;
         
         header->tot_len = tosend;
         header->saddr = 0;
         header->packetId = 0;
         header->frameId = sim.frameNo;
         
         #ifdef _debug
         std::cout << "frameSizeDw = " << frameSizeDw << "\n";
         std::cout << "packetsPerFrame = " << packetsPerFrame << "\n";
         std::cout << "bytes tosend = " << tosend << "\n";
         std::cout << "frameId = " << header->frameId << std::endl;
         #endif
         
         //Disaster waiting to happen: this assumes that there will be at least 16 bytes in the last frame.
         int i;
         int sleeps = 0;
         for(i=0; i< packetsPerFrame; i++)
         {

            if(tosend - bytesent + Constants::TDP_PACKET_HEADER_SIZE > maxpcksz) pcktsz = maxpcksz;
            else pcktsz = tosend - bytesent+Constants::TDP_PACKET_HEADER_SIZE;

            if(i == 0) //First frame needs a header
            {
               for(int j= 0; j < 16; j++)
               {
                  sendBuff[Constants::TDP_PACKET_HEADER_SIZE+j] = 0;
               }
               for(int j = 16; j < pcktsz - Constants::TDP_PACKET_HEADER_SIZE; j++)
               {
                  sendBuff[Constants::TDP_PACKET_HEADER_SIZE + j] = ((BYTE *)sim.imdata)[bytesent + j-16];
               }
            }
            else if(i == packetsPerFrame -1) //Last needs a footer
            {
               int j;
               for(j = 0; j < pcktsz - Constants::TDP_PACKET_HEADER_SIZE - 16; j++)
               {
                  sendBuff[Constants::TDP_PACKET_HEADER_SIZE + j] = ((BYTE *)sim.imdata)[bytesent + j-16];
               }
               for(; j < pcktsz - Constants::TDP_PACKET_HEADER_SIZE; j++)
               {
                  sendBuff[Constants::TDP_PACKET_HEADER_SIZE+j] = 0;
               }
            }
            else
            {
               for(int j = 0; j < pcktsz - Constants::TDP_PACKET_HEADER_SIZE; j++)
               {
                  sendBuff[Constants::TDP_PACKET_HEADER_SIZE + j] = ((BYTE *)sim.imdata)[bytesent + j-16];
               }
            }

            int old_packet = header->packetId;

            while(header->packetId == old_packet && !TimeToDie)
            {
               try
               {
                  conn->send(sendBuff, pcktsz);
                  if(sleeps > 0)
                  {

                     ts.tv_nsec = 100;
                     nanosleep(&ts, 0);
                     sleeps = 0;
                  }
                  else sleeps++;

                  header->packetId++;
                  bytesent += pcktsz-Constants::TDP_PACKET_HEADER_SIZE;
               }
               catch (UdpFatalException u)
               {
                  //Wait for resource to become available.  Re-throw otherwise.
                  if(errno != EAGAIN)
                  {
                     std::cout << u.what() << "\n";
                     std::cout << frames << " " << bytesent << "\n";
                     ERROR_REPORT("Exception thrown sending data via udp");
                     throw(u);
                  }
               }
            }

         }

         #ifdef _debug
            std::cout << "bytes sent = " << bytesent << "\n";
            std::cout << "packets sent = " << i << "\n" << std::endl;
          #endif

         frames++;
         last_image_abs++;
         //last_image++;
         last_image = sis->get_last_image();
         if(last_image >= sis->get_max_n_images()) last_image = 0;
         
         
         //This is here to avoid errors.  Should be handled in separate thread, etc.  Also should macro-ize the channel number.
         while(read_fifo_channel(&fl.fifo_ch[0]) > 0);
      }
      else
      {
         #ifdef _debug
         std::cout << "last_image " << last_image << "\n";
         std::cout << "last_image_abs " << last_image_abs << "\n";
         std::cout << "max_n_images " << sis->get_max_n_images() << "\n";
         #endif
         ERROR_REPORT("Error getting image in send_frame().");
         exit(0);
       }
       behind = sis->get_last_image_abs() - last_image_abs;
       
       //if(behind) std::cout << "Behind by: " << sis->get_last_image_abs() - last_image_abs << "\n";
       
       
       if(behind >= sis->get_max_n_images())
       {
          skipped = behind - (int) .5*sis->get_max_n_images();
          total_skipped += skipped;
          _logger->log(Logger::LOG_LEV_ERROR,"Behind %i frames, skipping %i frames.  Total skipped: %i", behind, skipped, total_skipped);
          
          last_image_abs += skipped;
          last_image += skipped;
          if(last_image >= sis->get_max_n_images()) last_image = last_image - sis->get_max_n_images();
          
       }
       
   }
   
   return 0;
}

void frame_ready_handler(int signum __attribute__((unused)), siginfo_t *siginf, void *ucont __attribute__((unused)))
{
   fifo_channel * fc;
   frameserver *fw;
   
   if(siginf->si_code == POLL_IN)
   {  
      fc = &global_fifo_list->fifo_ch[FRAME_READY_FIFO_CH];
   
      fw = (frameserver *)fc->auxdata;

      fw->send_frame();
      
      while(read_fifo_channel(fc) > 0 && !TimeToDie); //We don't do anything with this, just clear it out.
   }
}


int frame_ready(fifo_channel *fc)
{
   frameserver *fw;
   
   fw = (frameserver *)fc->auxdata;
   
   if(!TimeToDie)
   {
      fw->send_frame();
      
      while(read_fifo_channel(fc) > 0); //We don't do anything with this, just clear it out.
   }
   return 0;
}

} //namespace VisAO
	
