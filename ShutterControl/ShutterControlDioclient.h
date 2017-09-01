/************************************************************
*    ShutterControlDioclient.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Generic shutter controller as a client of the dioserver
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file ShutterControlDioclient.h
  * \author Jared R. Males (jrmales@email.arizona.edu)
  * \brief Declarations for the shutter controller as a client of the dioserver.
  *
*/

#ifndef __ShutterControlDioclient_h__
#define __ShutterControlDioclient_h__

#include "ShutterControl.h"
#include "libvisao.h"
#include "VisAOApp_standalone.h"

#include "profileutils.h"

#include <math.h>
#include <time.h>
#include <sys/time.h>

namespace VisAO
{

class ShutterControlDioclient : public ShutterControl, public VisAOApp_standalone
{
public:
   ShutterControlDioclient(int argc, char **argv) throw (AOException);
   ShutterControlDioclient(std::string name, const std::string &conffile) throw (AOException);

   ~ShutterControlDioclient();
   
protected:
   int dio_ch_set;  ///<The dioserver channel used for setting shutter state (open or shut)
   int dio_ch_get; ///<The dioserver channel used for getting shutter state (open or shut)
   std::string diofifo_path; ///<The base path of the dioserver fifos
   
   int initialize_ShutterControl() throw (AOException);
   
   int DO_OPENSHUT;
   char * glob_seqmsg;

   int power_outlet; ///<Configuration variable, setting which power outlet to monitor
   int *powerOutlet; ///<The power strip outlet controlling shutter power.
   
   power_status_board * psb;

   int getPowerStatus();

   sharedim_stack<char> sis; ///< The shared memory ring buffer for timestamp storage
   sharedim<char> * sim; ///< Pointer to a shared memory "image"
   
   int shmem_key; ///< The key for the ring buffer shared memory
   int num_timestamps; ///< The number of shared "images" available in the ring buffer

   int initSaveInterval;
   uint64 cycleNo;

   //Data logger
   pthread_t logger_th; ///<Thread which periodically logs data in the sharedim_stack data buffer
   double logger_pause; ///<Pause time of the logger thread, seconds
   double data_log_time_length; ///<Time duration of a shutter log file, seconds

   std::string save_path; ///<Path, relative to VISAO_ROOT, where to save data.

   ///Generate a unique data file name, using the standard VisAO timestamp.
   std::string getDataFileName();
   
public:
   virtual int Run();
   //Virtual functions, to be overridden by derived classes specific to shutter and controller
   virtual int start_ShutterControl();
   virtual int shutdown_ShutterControl();
   
   virtual int get_hw_state();
   std::string get_state_str();

protected:
   ///Send shutter open signal for initialization, but do not log or profile this.
   /** The shutter will almost always be open (it fails open, etc.), this is just a check.
     * so we don't want to count the cycle.
     */
   int init_shutter_open();
   virtual int do_shutter_open(void *adata);
   virtual int do_shutter_close(void *adata);

   ///Save the initialization data to disk
   int saveInit();

   ///Launches the data logger thread
   /** Sets to priority 0.
     * \retval x is the return value of pthread_create 
     */
   int launchDataLogger();
   
public:
   ///Log data at intervals
   void dataLogger();

   
   std::string remote_command(std::string com, char * seqmsg = 0);
   std::string local_command(std::string com, char *seqmsg = 0);
   std::string script_command(std::string com, char * seqmsg = 0);
   std::string auto_command(std::string com, char * seqmsg);
   std::string common_command(std::string com, int calling_ctype);

   virtual int update_statusboard();

   
};

///Launches the data logger
/** \param Sctrl is a ShutterControlDioclient
  */
void * __launch_data_logger(void * Sctrl);

void shutter_auto_handler(int signum, siginfo_t *siginf, void *ucont);

} //namespace VisAO

#endif // __ShutterControlDioclient_h__
