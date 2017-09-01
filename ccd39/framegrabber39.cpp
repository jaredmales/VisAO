/************************************************************
*    framegrabber39.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for a class to capture frames from the BCU39, using the ebtables ulog.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber39.cpp
  * \author Jared R. Males
  * \brief Definitions for a class to capture frames from the BCU39, using the ebtables ulog.
  * 
  *
*/

#include "framegrabber39.h"

#include "display"

static struct sockaddr_nl sa_local;

static struct sockaddr_nl sa_kernel;

   
namespace VisAO
{

framegrabber39::framegrabber39(int argc, char **argv) throw (AOException) : framegrabber<unsigned char>(argc, argv)
{
   init_framegrabber39();
}

framegrabber39::framegrabber39(std::string name, const std::string &conffile) throw (AOException) : framegrabber<unsigned char>(name, conffile)
{
   init_framegrabber39();
}

struct lutentry
{
   int order;
   int value;
};

bool complutentries(lutentry a, lutentry b)
{
   return a.value < b.value;
}

void framegrabber39::init_framegrabber39()
{
   sa_local.nl_family = AF_NETLINK;
   sa_local.nl_groups = 1;

   sa_kernel.nl_family = AF_NETLINK;
   sa_kernel.nl_pid = 0;
   sa_kernel.nl_groups = 1;
   
   pktcnt = 0;
   
   rcvbufsize = BUFLEN;

   frameSizeDw = 4812;

   std::vector<lutentry> le(6400);
   for(int i=0;i<6400;i++)
   {
      le[i].order = i;
      le[i].value = displayLUT[i];
   }

   std::sort(le.begin(), le.end(), complutentries);

   bcuLUT.resize(6400);
   for(int i=0;i<6400;i++) bcuLUT[i] = le[i].order;


   //Init the status board
   statusboard_shmemkey = STATUS_framegrabber39;
   if(create_statusboard(sizeof(framegrabber39_status_board)) != 0)
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
   
   
   STOP_FRAMEGRABBER = 0; //Always start running!
   

}

/* Get the next ebt_ulog packet, talk to the kernel if necessary */
ebt_ulog_packet_msg_t *  framegrabber39::ulog_get_packet()
{
   static struct nlmsghdr *nlh = NULL;
   static int len, remain_len;
   static int pkts_per_msg = 0;
   
   ebt_ulog_packet_msg_t *msg;
   
   socklen_t addrlen = sizeof(sa_kernel);
   
   if (!nlh)
   {
      //printf("1.0\n");
      if (pkts_per_msg && DEBUG_QUEUE)
      {
         logss.str("");
         logss << "PACKETS IN LAST MSG: " << pkts_per_msg;
         log_msg(Logger::LOG_LEV_ERROR, logss.str());
         //std::cerr << logss.str() << " (logged) \n";
      }
      pkts_per_msg = 0;
      //printf("1.1\n");
      len = recvfrom(sfd, buf, BUFLEN, 0, (struct sockaddr *)&sa_kernel, &addrlen);
      //printf("1.2\n");
      if (errno == EINTR)
         return 0;
      if (addrlen != sizeof(sa_kernel))
      {
         logss.str("");
         logss << "addrlen" << addrlen << " != " << sizeof(sa_kernel);
         log_msg(Logger::LOG_LEV_ERROR, logss.str());
         //std::cerr << logss.str() << " (logged) \n";
         return 0;
      }
      if (len == -1)
      {
         logss.str("");
         logss << "recvmsg: " << strerror(errno);
         log_msg(Logger::LOG_LEV_ERROR, logss.str());
         //std::cerr << logss.str() << " (logged) \n";
         return 0;
      }
      nlh = (struct nlmsghdr *)buf;
      if (nlh->nlmsg_flags & MSG_TRUNC || len > BUFLEN)
      {
         logss.str("");
         logss << "packet truncated";
         log_msg(Logger::LOG_LEV_ERROR, logss.str());
         //std::cerr << logss.str() << " (logged) \n";
         //printf("Packet truncated");
         return 0;
      }
      if (!NLMSG_OK(nlh, BUFLEN))
      {
         logss.str("");
         logss << "Netlink message parse error: " << strerror(errno);
         log_msg(Logger::LOG_LEV_ERROR, logss.str());
         //std::cerr << logss.str() << " (logged) \n";
         return NULL;
      }
   }
   
   msg = (ebt_ulog_packet_msg_t *) NLMSG_DATA(nlh);
   
   remain_len = (len - ((char *)nlh - buf));
   if (nlh->nlmsg_flags & NLM_F_MULTI && nlh->nlmsg_type != NLMSG_DONE)
      nlh = NLMSG_NEXT(nlh, remain_len);
   else
      nlh = NULL;
   
   pkts_per_msg++;
   return msg;
}

/* details of the BCU39 frame are contained in adopt/lib/bcu_diag.h*/
int framegrabber39::start_framegrabber()
{
   set_euid_called();
   
   sa_local.nl_pid = getpid();

   sa_local.nl_groups = sa_kernel.nl_groups = 1 << (30 - 1);

   sfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_NFLOG);
   
   if (!sfd)
   {
      logss.str("");
      logss << "socket: " << strerror(errno);
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //std::cerr << logss.str() << " (logged) \n";
      return stop_framegrabber();
   }
   
   if (bind(sfd, (struct sockaddr *)(&sa_local), sizeof(sa_local)) == -1)
   {
      logss.str("");
      logss << "bind: " << strerror(errno);
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //std::cerr << logss.str() << " (logged) \n";
      return stop_framegrabber();
   }
  
   if(setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &rcvbufsize, sizeof(rcvbufsize)) < 0)
   {
      logss.str("");
      logss << "setsockopt: " << strerror(errno);
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      //std::cerr << logss.str() << " (logged) \n";
      return stop_framegrabber();
   }
      

   logss.str("");
   logss << "framegrabber39 running.";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   //std::cerr << logss.str() << " (logged) \n";

   t0 = get_curr_time();
   pktcnt = 0;
   RUNNING = 1;

   DiagnosticUdpHeader * bcuhead;

   int curr_frameId = -1;
   int last_packetId=-1;

   int curr_pixel = 0;
   int curr_pixel_offset = 4*2;
   char * bcudata;
   uint8* cursor;
   
   _frameSizeBytes = frameSizeDw * _FRAME_HEADER_SIZE_DW;
   _packetsPerFrame = _frameSizeBytes / Constants::MAX_TDP_PACKET_SIZE;
   if(_frameSizeBytes % Constants::MAX_TDP_PACKET_SIZE)
   {
      _packetsPerFrame++;
   }

   //std::cout << "Packets Per Frame: " << _packetsPerFrame << "\n";
   while(!STOP_FRAMEGRABBER && !TimeToDie)
   {
      if (!(msg = ulog_get_packet()))
      {
         //printf("returned 0\n");
         continue;
      }
      
      if (msg->version != EBT_ULOG_VERSION)
      {
         printf("WRONG VERSION: %d INSTEAD OF %d\n",
                msg->version, EBT_ULOG_VERSION);
         continue;
      }
      //printf("PACKET NR: %d\n", ++pktcnt);
      ++pktcnt;
      iph = NULL;
      curr_len = ETH_HLEN;
      
      if (msg->data_len < curr_len)
      {
         goto letscontinue;
      }
      
   
      ehdr = (struct ethhdr *)msg->data;
      etype = getethertypebynumber(ntohs(ehdr->h_proto));
      
      if (ehdr->h_proto == htons(ETH_P_IP))
         iph = (struct iphdr *)(((char *)ehdr) + curr_len);
      if (!iph)
         goto letscontinue;
      
      curr_len += sizeof(struct iphdr);
      if (msg->data_len < curr_len || msg->data_len < (curr_len += iph->ihl * 4 - sizeof(struct iphdr)))
      {
         goto letscontinue;
      }

      //length of ip header *4bytes per word, +8 bytes for UDP, +2bytes to align the BCU packet.
      bcuhead = (DiagnosticUdpHeader *)((char *) iph + iph->ihl*4 + 10);
      //length of the diagnostic header is 12 bytes
      bcudata = (char *) ((char *)bcuhead + 12);
      
      if(bcuhead->frameId == curr_frameId  && bcuhead->packetId == last_packetId + 1)
      {
         //std::cout << bcuhead->frameId << " " << bcuhead->packetId << " " << bcuhead->tot_len <<  "\n";
         last_packetId = bcuhead->packetId;

         if(last_packetId == 0)
         {
            sim = sis.set_next_image(sizeof(slopecomp_diagframe_pixels_slopes), 1);
            sim->depth = 8;
            //sim->frameNo = frameNo;
            gettimeofday(&tv, 0);
            sim->frame_time = tv;
         }

         cursor = ((uint8 *) bcudata + curr_pixel_offset);
         /*while(curr_pixel < 6400 && ((char *) cursor - (char *)ehdr) < msg->data_len)
         {
            sim->imdata[bcuLUT[curr_pixel]] = *((short*) cursor);
            cursor++;
            curr_pixel++;
         }*/

         while(((char *) cursor - (char *)ehdr) < msg->data_len)
         {
            sim->imdata[curr_pixel] = *((unsigned char*) cursor);
            cursor++;
            curr_pixel++;
         }
         
         //curr_pixel_offset = 0;

         if(last_packetId == _packetsPerFrame-1)
         {
            last_packetId = -1;
            curr_frameId++;
            curr_pixel = 0;
            curr_pixel_offset = 0;//4*2;
            
            sim->frameNo = *( (uint32*) sim->imdata);
            
            sis.enable_next_image();
            send_ping();
         }
      }
      else
      {
         last_packetId = -1;
         curr_frameId = bcuhead->frameId + 1;
      }

   
      letscontinue:
      ;
   }

   double t1 = get_curr_time();

   close(sfd);
   
   set_euid_real(); //Go back to regular privileges
   logss.str("");
   logss << pktcnt << " frames.";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   //std::cerr << logss.str() << " (logged) \n";

   //std::cout << ((double)pktcnt)/(t1-t0) << " packets/sec" << std::endl;
   //std::cout << ((double)pktcnt)/(t1-t0)/_packetsPerFrame << " fps";

   return stop_framegrabber();
}

int framegrabber39::stop_framegrabber()
{
   RUNNING = 0;
   
   logss.str("");
   logss << "framegrabber39 stopped.";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   //std::cerr << logss.str() << " (logged) \n";
   
   return 0;
}

int framegrabber39::update_statusboard()
{
   static int last_pktcnt = 0;
   static double last_t0 = t0;
   double t1;
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      VisAO::framegrabber39_status_board * fsb = (VisAO::framegrabber39_status_board *) statusboard_shmemptr;

      fsb->running = RUNNING;
      fsb->reconstructing = serverPingEnabled;
      fsb->saving = writerPingEnabled;

      if(RUNNING)
      {
         t1 = get_curr_time();
         fsb->fps = ((double)(pktcnt-last_pktcnt))/(t1-last_t0)/_packetsPerFrame;
         last_pktcnt = pktcnt;
         last_t0 = t1;
      }
      else fsb->fps = 0.;
   }
   return 0;
}//int framegrabber39::update_statusboard()

} //namespace VisAO
      
