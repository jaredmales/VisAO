/************************************************************
*    coronguide.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for a class to auto guide for the coronagraph
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file coronguide.h
  * \author Jared R. Males
  * \brief Declarations for a class to auto guide for the coronagraph
  * 
  *
*/

#ifndef __coronguide_h__
#define __coronguide_h__

#include "VisAOApp_standalone.h"

#include "sharedim_stack.h"

#include "statusboard.h"

#include <cmath>
#include <algorithm>

#include "improcessing.h"

namespace VisAO
{
   
#define MEDSUB  0
#define UNSHARP 1

#define NOAVG -10000

class coronguide : public VisAOApp_standalone
{
   
public:
   coronguide(int argc, char **argv) throw (AOException);
   coronguide(std::string name, const std::string &conffile) throw (AOException);
   
   int Create();
   
protected:
      
   sharedim_stack<short> * sis; ///< Manages a VisAO shared memory image stack.
   sharedim_stack<short> * dark_sis;
   
   int shmem_key; ///<shared memory key for 
   
   bool attached_to_shmem;
   
   sharedim<short> sim; ///<The sharedim structure retreived from the stack
   sharedim<short> dark_sim;
   
   imviewer_status_board *isb;
   aosystem_status_board *aosb;
   shutter_status_board * ssb;
   ccd47_status_board * ccdsb;
   int sbconn;
   
   int attach_status_boards();
   int write_frame();
   int move_gimbal(double gx, double gy);
   
   size_t last_x0;
   size_t last_x1;
   size_t last_y0;
   size_t last_y1;
   
   double xcen;
   double ycen;
   double xcen_avg;
   double ycen_avg;
   
   //double min;
   //double max;
   //double mean;
   
   int bgAlgorithm;
   
   double * tempIm;
   double * backgroundIm;
   double * kernel;
   
   size_t kdim;
   
   double kfwhm;
   int ksize; ///Size of the kernel, in units of kfwhm.
   
   
   double tgtx;
   double tgty;
   
   int loopState; //0==open, 1==closed, 2==paused, 3==auto pause, 4==auto pause recovery
   
   int close_loop();
   int pause_loop();
   int nod_loop();
   int open_loop();
   
   double gimbscale;
   
   double ap_recov_time;
   
   double loop_gain;
   int set_loop_gain(double lg);
   
   int set_bgalgo(int bga);
   int set_kfwhm(double kfw);
   int set_ksize(int ks);
   
   int avgLen;
   int avgidx;
   int inAvg;
   
   std::vector<double> xcens;
   std::vector<double> ycens;
   
   bool useAvg;
   bool minMaxReject;
   int setUseAvg(bool ua);
   int setMinMaxReject(bool mmr);
   
   int setAvgLen(int na);
   
   int gimbCounter;
   
   
   pthread_mutex_t memmutex; 
   
public:
    
   int set_shmem_key(int sk);
   int get_shmem_key(){return shmem_key;}
   int connect_shmem();
   
   int set_sim(sharedim<short> s);
   sharedim<short> get_sim(){return sim;}
      
   virtual int Run();
   
   std::string get_state_str();
   
   int  update_statusboard();

   ///Write coronguide status to the data log
   virtual void dataLogger(timeval tv);
   
protected:
   
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
     */
   std::string common_command(std::string com, int cmode);
};



} //namespace VisAO

#endif //__coronguide_h__

