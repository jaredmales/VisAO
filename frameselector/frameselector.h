/************************************************************
*    frameselector.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the real time frame selector
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file frameselector.h
  * \author Jared R. Males
  * \brief Declarations for the real time frame selector
  * 
  *
*/

#ifndef __frameselector_h__
#define __frameselector_h__

#include "VisAOApp_standalone.h"

namespace VisAO
{

class frameselector : public VisAOApp_standalone
{
public:
   frameselector(int argc, char **argv) throw (AOException);
   frameselector(std::string name, const std::string &conffile) throw (AOException);
   
   int Create();
   
protected:
   std::string ping_fifo_path;
   
   sharedim_stack<float> * sis; ///< Manages a VisAO shared memory image stack.
   
   int shmem_key;
   
   bool attached_to_shmem;
   
   sharedim<float> sim; ///<The sharedim structure retreived from the stack
   
   double thresh;

   bool frame_select;

   pthread_mutex_t strehl_ready_mutex;
   pthread_mutex_t select_mutex;
   
public:
   pthread_cond_t strehl_ready_cond;

public:
   int set_ping_fifo_path(std::string &);
   std::string get_ping_fifo_path(){return ping_fifo_path;}
   
   int set_sharedim_stack(sharedim_stack<float> *);
   sharedim_stack<float> * get_sharedim_stack(){return sis;}
   
   int set_shmem_key(int sk);
   int get_shmem_key(){return shmem_key;}
   int connect_shmem();
   
   int set_sim(sharedim<float> s);
   sharedim<float> get_sim(){return sim;}
   
   virtual int Run();

   int selector();
   
   virtual int kill_me();

   virtual int update_statusboard();
   
protected:
   
   int attach_status_boards();

   /// Overridden from VisAOApp_base::remote_command, here just calls common_command.
   virtual std::string remote_command(std::string com);
   /// Overridden from VisAOApp_base::local_command, here just calls common_command.
   virtual std::string local_command(std::string com);
   /// Overridden from VisAOApp_base::script_command, here just calls common_command.
   virtual std::string script_command(std::string com);  
   /// Overridden from VisAOApp_base::auto_command, here just calls common_command.
   std::string auto_command(std::string com, char *seqmsg);

   /// The common command processor for commands received by fifo.
   /** The return value depends on the command received.  Recognized commands are:
    * - 
    * - For any other inputs returns "UNKNOWN COMMAND: (str)\n"
    */
   std::string common_command(std::string com, int cmode);
   
   std::string get_state_str();
};

//read one of the dio channel fifos
//int strehl_ready(fifo_channel *fc);
void strehl_ready(int signum, siginfo_t *siginf, void *ucont);


} //namespace VisAO

#endif //__framewriter_h__

