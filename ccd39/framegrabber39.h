/************************************************************
*    framegrabber39.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for a class to capture frames from the BCU39, using the ebtables ulog.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber39.h
  * \author Jared R. Males
  * \brief Declarations for a class to capture frames from the BCU39, using the ebtables ulog.
  * 
  *
*/

#ifndef __framegrabber39_h__
#define __framegrabber39_h__

#include "framegrabber.h"

#include <asm/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include "BcuLib/Commons.h"
#include "bcu_diag.h"


extern "C"
{
   #include "ebtables_u.h"
   #include "ethernetdb.h"

}
#include <linux/netfilter_bridge/ebt_ulog.h>

#include <algorithm>

#define DEBUG_QUEUE 0
#define BUFLEN 65536


/*struct DiagnosticUdpHeader
{
   uint32 tot_len;
   uint32 saddr;
   uint16 packetId;   // Restarts from zero for each frameId
   uint16 frameId;
};*/

namespace VisAO
{

///The framegrabber specialization for the CCD39.
/** This class uses the bcu ethernet bridge to transparently grab BCU39 frames, and writes them a sharedim_stack.
 * Does not interface with the BCU 39 in anyway.
 *
 * See \ref framegrabber for a list of config file options.
 *
 * See \ref VisAOApp_standalone for command line arguments.  There are no additional command line arguments for
 * a basic framegrabber, though derived clases may add them.
 *
 * See \ref framegrabber for the commands accepted by this class.
 *
 */
class framegrabber39 : public framegrabber<unsigned char>
{
   public:
      framegrabber39(int argc, char **argv) throw (AOException);
      framegrabber39(std::string name, const std::string &conffile) throw (AOException);

   protected:
      void init_framegrabber39();

      int   frameSizeDw;
      int   _frameSizeBytes;
      static const int   _FRAME_HEADER_SIZE_DW = 4;      
      int     _packetsPerFrame;     // Number of packet for each frame: computed from frameSize
      
      size_t curr_len;

      double t0; ///<Starting time of framegrabber loop.

      int pktcnt;
      
      int rcvbufsize;

      char buf[BUFLEN];
      
      int sfd;
      
      ebt_ulog_packet_msg_t *msg;
      
      struct ethhdr *ehdr;

      struct ethertypeent *etype;

      struct protoent *prototype;

      struct iphdr *iph;

      ebt_ulog_packet_msg_t *ulog_get_packet();

      std::vector<int> bcuLUT;
   public:
      virtual int start_framegrabber();
      virtual int stop_framegrabber();


      /// Update the status board.
      /** Overridden from VisAOApp_base
        * \retval 0 on success
        * \retval -1 on failure
        */
      virtual int update_statusboard();

};

} //namespace VisAO


#endif //__framegrabber47_h__
