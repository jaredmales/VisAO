/************************************************************
*    dioserver.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the dioserver.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file dioserver.h
  * \author Jared R. Males
  * \brief Declarations for the dioserver.
  * 
  *  Do we need more?
  * 
  *
*/

#ifndef __dioserver_h__
#define __dioserver_h__

#define NODIOCARD

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>

#include <sys/stat.h>

#define DIO_CHANNELS 64

#include "VisAOApp_standalone.h"
#include "libvisao.h"

namespace VisAO
{

class dioserver; //Forward declaration

///Holds the information for one digital I/O channel.
typedef struct
{
   int swchannel; /// The software channel number - used to access the channel via the dio fifos.
   int hwchannel; /// The  hardware channel number - used to access the diocard itself.
   int direction; ///The direction, 0 for output, 1 for input
   int enabled; /// Whether the channel is enabled or not.  Disabled channels (enabled = 0) are ignored.
   dioserver * dios; /// Pointer to the dioserver managing this channel, for handlers.
} dioserver_ch;

/// Class to manage access to a digital input/output device
/** Allows us to map "software channels" to "hardware channels", so that if we have
  * to rewire we only have to change the config file.
  *
  * Uses a generic DIO interface, consisting of a void * and various function pointers which
  * take said void * as an argument.  To adapt to a new card, just provide the relevant functions
  * (diocard_init, diocard_sd, diocard_write, and diocard_read).  dioserver does the rest.
  * 
  * Provides only remote and local command fifos.  Most interaction with dioserver will be via the 
  * dio channel fifos.
  * 
  * See \ref VisAOApp_standalone for command line arguments.  There are no additional command line arguments for dioserver.
  *
  * This is a standalone VisAOApp, so it doesn't depend on MsgD.  In addition to the optional standard config options inherited from \ref VisAOApp_standalone this class REQUIRES:
  *    - <b>DIOCHAN_XX</b> <tt>array of int</tt> - XX is the software channel number (how fifos are labeled).  The array consists of [hw,dir,en] where hw is hardware channel, dir is the direction (0=out, 1=in), and en is the enable status.
  * 
  * The following config setting is optional:
  *    - <b>fifo_path</b> <tt>string</tt> - the base bath, relative to VISAO_ROOT, where the dio fifos are created.
 */

class dioserver : public VisAOApp_standalone
{
public:
   /// Command line constructor.
   dioserver(int argc, char **argv) throw (AOException);
   
   /// Config file constructor.
   dioserver(std::string name, const std::string &conffile) throw (AOException);
   
   dioserver_ch diosch[DIO_CHANNELS]; ///< The channels being managed.
   int n_enabled; ///< Number of channel enabled.
   
   /// The base path names for the fifos
   std::string fifo_path;
   
   ///The generic DIO interface. This pointer is passed unaltered to the diocard_* functions.
   void * diocard_info;
   int (* diocard_init)(void *); ///< Initialize the card.
   int (* diocard_sd)(void *); ///< Shutdown the card.
   int (* diocard_write)(void * card, int ch, int bit); ///< Write a bit to a channel
   int (* diocard_read)(void * card, int ch); ///< Read a bit from the channel.
   
   /// Initialize the dioserver structure
   int init_dioserver();
   
   /// Set a single channel of the server
   /** Called for each channel in the configuration file.  Update n_enabled.
    * \param chnum the software channel number
    * \param hwchan the hardware channel number
    * \param dir the direction (0 = output, 1 = input)
    * \param enab whether or not the channel is enabled.
    * \retval 0 on success
    * \retval -1 on failure
    */
   int set_dioserver_channel(int chnum, int hwchan, int dir, int enab);
   
   ///Setup the dioserver
   /** Sets up the fifo_list, and installs the channels*/
   int setup_dioserver();
   
   /// Take all the actions needed to start the server.
   virtual int Run();
   
   /// Overridden from VisAOApp_base::remote_command
   std::string remote_command(std::string);
   /// Overridden from VisAOApp_base::local_command
   std::string local_command(std::string);
   
};

/// Handler to read one of the dio channel fifos
int read_diofifo(fifo_channel *fc);

} //namespace VisAO

#endif //__dioserver_h__
