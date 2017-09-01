/************************************************************
*    frameserver.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for a class for serving frames over UDP.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file frameserver.h
  * \author Jared R. Males
  * \brief Declarations for a class for serving frames over UDP.
  * 
  *
*/

#ifndef __frameserver_h__
#define __frameserver_h__


#include "VisAOApp_standalone.h"

#include "UdpConnection.h"

#include "BcuLib/BcuCommon.h"

#define FRAME_READY_FIFO_CH 0

//#define _debug


namespace VisAO
{

///a class for serving frames over UDP.
/** \todo manage clearing the input ping channel better on frameserver
  */
class frameserver : public VisAOApp_standalone
{
public:
   frameserver(int argc, char **argv) throw (AOException);
   frameserver(std::string name, const std::string &conffile) throw (AOException);
   
   int Create();
   
protected:
   std::string pingFifoPath;
   
   std::string remoteIP;
   
   int localPort;
   
   int remotePort;
   
   //unsigned fileno;
   //unsigned ndigits;
   
   UdpConnection* conn;
   
   sharedim_stackS * sis; ///< Manages a VisAO shared memory image stack.
   
   int shmem_key;
   
   bool attached_to_shmem;
   
   sharedimS sim; ///<The sharedim structure retreived from the stack
   
   int behind; ///<The number of frames currently behind
   int total_skipped; ///<The total number of frames skipped
   
   int frameSizeDw; ///<Size of the frame in DWORD
   int packetsPerFrame; ///<No of UDP messages per frame
   
   BYTE sendBuff[Constants::MAX_ETH_PACKET_SIZE];
   
   ///Calculate frameSizeDW and packetsPerFrame
   int calc_packetsPerFrame(int frameSizeBytes, int bitsPerPix);

   int _MAX_TDP_PACKET_SIZE;
   int _MAX_ETH_PACKET_SIZE;

public:
   int setPingFifoPath(std::string &);
   std::string getPingFifoPath(){return pingFifoPath;}
   
   int setRemoteIp(std::string &);
   std::string getRemoteIp(){return remoteIP;}
   
   int setLocalPort(int);
   int getLocalPort(){return localPort;}
   
   int setRemotePort(int);
   int getRemotePort(){return remotePort;}
   
   
   
   int set_sharedim_stack(sharedim_stackS *);
   sharedim_stackS * get_sharedim_stack(){return sis;}
   
   int set_shmem_key(int sk);
   int get_shmem_key(){return shmem_key;}
   int connect_shmem();
   
   int set_sim(sharedimS s);
   sharedimS get_sim(){return sim;}
   
   int get_total_skipped(){return total_skipped;}
   
   virtual int Run();
   
   ///Send the frame, packet by packet, as if we're a BCU 47
   /** \todo modify to allow for the case where the last packet has less than 16 bytes, so the footer starts in 2nd to last packet.
    */
   int send_frame();
   
   
};

void frame_ready_handler(int signum, siginfo_t *siginf, void *ucont);

//read one of the dio channel fifos
int frame_ready(fifo_channel *fc);

} //namespace VisAO

#endif //__frameserver_h__

