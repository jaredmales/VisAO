/************************************************************
*    fifoutils.h
*
* Author: Jared R. Males (jrmales@as.arizona.edu)
*
* Linux fifo utilities, declarations
*
************************************************************/

/** \file fifoutils.h
  * \author Jared R. Males
  * \brief Declarations for linux fifo utilities.
  * 
*/

#ifndef __FIFOUTILS_H__
#define __FIFOUTILS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>

#include "profileutils.h"

#ifndef MAX_FNAME_SZ
///The maximum allowed filename length
#define MAX_FNAME_SZ 256
#endif


#ifndef RWBUFF_SZ
///The size of the i/o buffer
#define RWBUFF_SZ 256
#endif

#ifndef ERRMSG_SZ
///The size of the errmsg buffers
#define ERRMSG_SZ 256
#endif

#ifndef ERRHAND
///Error reporting macro. If not using the function __error_message(msg) then use stderr.
#define ERRMSG(msg) fprintf(stderr, "%s (%s line %i)\n", msg,__FILE__,__LINE__)
#else
extern void fifo_error_message(const char *, const char *, int);
#define ERRMSG(msg) fifo_error_message(msg, __FILE__, __LINE__)
#endif


#ifndef RTSIGIO
#define RTSIGIO (SIGRTMIN+4)
#define RTSIGIGN (SIGRTMIN+5)
#define RTSIGPING (SIGRTMIN+6)
#endif

/****************** The basic fifo_channel.  *******************************/

typedef struct __fifo_channel fifo_channel;

/// The basic fifo_channel structure.
/** Contains the meta data for a single (both ways) fifo i/o channel.
 *  Holds the file descriptors, the file names, and pointer to the input handler.*/
struct __fifo_channel
{
   int fd_out; ///<output file descriptor
   int fd_in;  ///<input file descriptor

   char * outfname; ///<output file name
   char * infname; ///<input file name
   
   int (*input_handler)(fifo_channel *); ///<the handler for input
   char * server_response; ///<buffer for the server response after output
   char * seqmsg; ///< Pointer to a possible sequence message, normally just the \0 in server_response.
   
   int sr_sz; ///<size of the server_response

   void *auxdata; ///<for passing other data to the handler.  note that no memory management is done.
   
   int timeout; ///<true if last read was a timeout.

   time_t last_atime_out; ///<last access time for the output channel, used for clearing timeouts
   time_t last_atime_in;///<last access time for the input channel, used for clearing timeouts
};

///Initializes the \ref __fifo_channel "fifo_channel" structure
/** All members are set to zero by this init.
  * Should be called every time a \ref __fifo_channel "fifo_channel" is created.
  * \param fc is a pointer the fifo_channel to initialize
  * \retval 0 on success (always)
  */
int init_fifo_channel(fifo_channel * fc);

///Allocate the server_response buffer.
/** Checks if buffer is non-zero and free's it if it is - so you must call \ref init_fifo_channel before setup.
 * \param fc is a pointer the fifo_channel to initialize
 * \param buffsz is the size of the server_response buffer to allocate
 * \retval 0 on success
 * \retval -1 on failure
 */
int setup_fifo_channel(fifo_channel *fc, size_t buffsz);

///Set the details of the fifo_channel.
/** Allocates and assigns the file name strings, you must call \ref init_fifo_channel before using this function.
  * Also installs the handler pointer.
  * This function calls \ref create_fifos, thus creating them if they don't already exist.
  * This function does not actually open the fifos.
  * \param fc is a pointer the fifo_channel to initialize 
  * \param fin is the file name of the input fifo
  * \param fout is the file name of the output fifo
  * \param inp_hand is a pointer to the input handler function for this channel
  * \param adata a pointer to auxiliary data.
  * \retval 0 on success
  * \retval -1 on failure
  */
int set_fifo_channel(fifo_channel *fc, const char *fin, const char *fout, int (*inp_hand)(fifo_channel *), void *adata);

///Creates the fifos if they don't already exist
/** \param fc is a pointer the fifo_channel to initialize
  * \retval 0 on success
  * \retval -1 on failure (check errno)
  */
int create_fifos(fifo_channel *fc);

///Opens the input fifo channel for async reading with an exculsive lock.
/** Clears the fifo, ignoring any pending input, so open before listening for SIGIO
  * \param fc  is a pointer the fifo_channel to initialize
  * \retval 0 on success
  * \retval -1 on failure
  */
int open_fifo_channel_in(fifo_channel *fc);

///Opens the input fifo channel for async reading without an exculsive lock.
/** Clears the fifo, ignoring any pending input, so open before listening for SIGIO
  * \param fc  is a pointer the fifo_channel to initialize
  * \retval 0 on success
  * \retval -1 on failure
  */
int open_fifo_channel_in_nolock(fifo_channel *fc);

///Opens the output fifo channel for non-blocking writing with an exclusive lock
/** \param fc is a pointer the fifo_channel to initialize
  * \retval 0 on success
  * \retval -1 on failure
  */
int open_fifo_channel_out(fifo_channel *fc);

///Opens the output fifo channel for non-blocking writing without an exclusive lock
/** \param fc is a pointer the fifo_channel to initialize
  * \retval 0 on success
  * \retval -1 on failure
  */
int open_fifo_channel_out_nolock(fifo_channel *fc);

///Write data to the output channel
/** \param fc is a pointer the fifo_channel to initialize
  * \param com the buffer to write
  * \param comlen length of the buffer
  * \retval nbytes the number of bytes written, on success
  * \retval -1 on failure 
  */
int write_fifo_channel(fifo_channel *fc, const char * com, int comlen);

///Write data to the output channel, including a sequence message.
/** \param fc is a pointer the fifo_channel to initialize
  * \param com the buffer to write
  * \param comlen length of the buffer
  * \param seqmsg the 6 byte sequence message.
  * \retval nbytes the number of bytes written, on success
  * \retval -1 on failure 
  */
int write_fifo_channel_seqmsg(fifo_channel *fc, const char * com, int comlen, char * seqmsg);

///Read data from the input fifo channel
/** \param fc is a pointer the fifo_channel to initialize
  * \retval 0 on success
  * \retval 1 if read not complete before filling \ref __fifo_channel::server_response.
  */
int read_fifo_channel(fifo_channel * fc);

///Checks if the output fifo of the channel is locked
/** Normally a process obtains an exclusive lock on the input of its fifo channels.  That is
  * it wants to be the only process which can receive commands on that channel.  This function
  * checks for the presence of such a lock on the output fifo of a channel, as a way to test if
  * anybody is listening.  This can be used to prevent writing to a fifo that isn't being read.
  * See \ref VisApp_base::write_fifo_channel for an example.
  * \param fc the relevant fifo_channel
  * \retval 0 if there is not lock
  * \retval 1 if there is a lock
  */ 
int check_if_in_locked(fifo_channel *fc);


///A structure to hold a list of \ref __fifo_channel "fifo_channels"
typedef struct
{
   int nchan; ///<The number of channels.
   fifo_channel * fifo_ch; ///<An array of \ref __fifo_channel "fifo_channels"

   fd_set fds; ///< the fd_set for the async SIGIO handler
   int maxfile; ///< the maximum file descriptor for the SIGIO handler to worry about.
   
   //version 2
   int * fd_to_fifo_ch_index; ///< Array mapping a file descriptor to an index in the fifo_ch array.
   
   int tot_pending_reads; ///< The total number of pending reads.
   int * pending_reads; ///< An array, with same index as fifo_ch, which tracks number of pending reads on each channel
   int * read_queue; ///< Circular buffer which contains the fifo_ch index to read next.
   int rq_sz;///< Size of the read_queue
   
   int read_queue_pos; ///< The current position in the read_queue
   int read_queue_nextpos;
   
   int RTSIGIO_overflow; ///< flag indicating that the signal queue has overflown.
   //Todo:
   /* X populate fd_to_fifo_ch_index at end of connect_fifo_list
    * X initialize the pending reads and queue.
    * X SIGIO to real time
    * X Signal handler just updates tot_pending_reads, pending_reads, read_queue_pos, and read_queue[read_queue_pos] using sig_fd from the realtime signal
    * X App main loop is responsible for checking for tot_pending_reads > 0.
    * X Write fifo_list_do_pending_reads, which handles the pending reads undle tot_pending_reads == 0
    */
} fifo_list;


///Initialize the fifo_list.
/** All members set to 0.
  * \param fl is a pointer to the fifo_list to initialize
  * \retval 0 on success (always)*/
int init_fifo_list(fifo_list * fl);

///Setup a fifo_list.
/** Allocates the fifo_ch array, then initializes each fifo_channel
  * \param fl is a pointer to the initialized fifo_list to setup
  * \param nch is the number of channels in the list
  * \retval 0 on success
  * \retval -1 on failure*/ 
int setup_fifo_list(fifo_list * fl, int nch);

///Set the details of one channel in the list.
/** First calls \ref setup_fifo_channel
  * Then calls \ref set_fifo_channel
  * \param fl  is a pointer to the initialized fifo_list to setup
  * \param nch is the channel number to set
  * \param buffsz is the size of the server_reponse buvver for this channel
  * \param fin is the file name of the input fifo
  * \param fout is the file name of the output fifo
  * \param inp_hand is a pointer to the input handler function for this channel
  * \param adata a pointer to auxiliary data.
  * \retval 0 on success
  * \retval -1 on failure*/
int set_fifo_list_channel(fifo_list *fl, int nch, int buffsz, const char *fin, const char *fout, int (*inp_hand)(fifo_channel *), void * adata);

///Open each fifo in the list, with exclusive lock on the input fifo.
/** The input channel is locked so that no other process will intercept communications
  * intended for the calling process.  Output fifos are not locked, as multiple processes
  * could conceivable write to the same channel (e.g. a ping or watchdog).
  * Both channels can't be locked, as then nobody could establish a connection.  
  * \param fl is a pointer to the initialized fifo_list to setup
  * \retval 0 on success
  * \retval -1 on failure*/ 
int connect_fifo_list(fifo_list *fl);

///Open each fifo in the list, without exclusive locks.
/** \param fl is a pointer to the initialized fifo_list to setup
  * \retval 0 on success
  * \retval -1 on failure*/ 
int connect_fifo_list_nolock(fifo_list *fl);

///Populates the fd_to_fifo_ch_index array
int init_fifo_list_pending_reads(fifo_list *fl);

///The global fifo_list, for signal handling
extern fifo_list * global_fifo_list;

///A SIGIO handler, which uses \ref global_fifo_list to call the appropriate handler.
/** This means you must define global_fifo_list and point it at your \ref fifo_list
  * or this will segfault.
  */ 
void catch_fifo_response_list(int signum);


///Change from SIGIO to real time signals
int set_fifo_list_rtsig(fifo_list * fl);

///A handler for a R/T SIGIO, which just updates the pending reads meta data but does no actual signal handling.
void catch_fifo_pending_reads(int signum, siginfo_t *siginf, void *ucont);

///A handler for normal SIGIO when we should have gotten the R/T SIGIO, implying signal queue overflow
void catch_fifo_standard_sigio(int signum);

///Runs through the pending reads, dispatching each handler in turn.
int fifo_list_do_pending_read(fifo_list * fl);


///Open a file for non-blocking asynchronous reads.
/** The nolock parameter controls whether or not to attempt an exclusive lock.   
  * \param fname is the name of the file to open
  * \param nolock if 0 attempt to get an exclusive lock (WRLCK) and close file if rejected.  if 1 do not lock.
  * \retval >0 (the file descriptor) on success
  * \retval 0 on failure
  * \retval -1 on failure to lock
  */  
int ropen_async(char * fname, int nolock);

///Open a file for non-blocking writes.
/** The nolock parameter controls whether or not to attempt an exclusive lock.  
  * \param fname is the name of the file to open
  * \param nolock if 0 attempt to get an exclusive lock (WRLCK) and close file if rejected.  if 1 do not lock.
  * \retval >0 (the file descriptor) on success
  * \retval 0 on failure
  * \retval -1 on failure to lock
  */  
int wopen_nonblock(char * fname, int nolock);

///Returns time since the Unix epoch in double precision seconds
/** Uses clock_gettime, CLOCK_REALTIME, and the attendant timespec, so it has precision of nanoseconds.
  */
double get_currt();

#ifdef __cplusplus
} //extern "C"
#endif

#endif //__FIFOUTILS_H__

