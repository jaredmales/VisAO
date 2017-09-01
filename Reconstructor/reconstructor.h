/************************************************************
*    reconstructor.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for a class to perform real-time reconstruction of wavefronts.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file reconstructor.h
  * \author Jared R. Males
  * \brief Declarations for a class to perform real-time reconstruction of wavefronts.
  *
  *
*/

#ifndef __reconstructor_h__
#define __reconstructor_h__

#include "VisAOApp_standalone.h"
#include "../lib/sharedim_stack.h"
#include <math.h>

#include "recmat.h"
#include "fir_filter.h"
#include "rmsAccumulator.h"

#define PING_IN_FIFO 0
#define FS_PING_FIFO 1

///The size of each telemetry vector
#define REC_TELEM_N  19
/*
0 - WFE 
1 - filtered WFE
2 - Tip
3 - Tilt 
4 - raw strehl
5 - calibrated strehl
6 - filtered strehl
7 - filtered calibrated strehl
8 - fitered tip
9 - filtered tilt
10 - ho1 WFE
11 - filtered ho1 wfe 
12 - ho2 WFE
13 - filtered ho2 WFE
14 - WFE temporal variance of WFE (165 frame avg)
15 - Tip temporal variance (165 frame avg)
16 - Tilt temporal variance (165 frame avg)
17 - HO1 temporal variance (165 frame avg)
18 - HO2 temporal variance (165 frame avg)
*/
namespace VisAO
{

///The VisAO reconstructor
/** This class provides a framework for a reconstructing wavefronts from slopes computed elsewhere, nominally
  * by the BCU39 and provided by the framegrabber39 process.  Receives pings from the framegrabber39 process when a new frame arrives.
  * Sends pings to the frameselector process. 
  *
  * See \ref VisAOApp_standalone for command line arguments.  There are no additional command line arguments for reconstructor.
  *
  * This is a standalone VisAOApp, so it doesn't depend on MsgD.  
  *
  * The following configuration values are optional:
  *    - <b>ping_fifo_path</b> <tt>string</tt> [fifos] - path, relative to VISAO_ROOT, specifying where to create the input and output pings
  *    - <b>bcu39_shmem_key</b> <tt>int</tt> [DATA_framegrabber39] - shared memory key for the framegrabber 39 frames circular buffer 
  *    - <b>strehls_shmem_key</b> <tt>int</tt> [DATA_strehls] - shared memory key for the Strehls circular buffer
  *    - <b>num_strehls</b> <tt>int</tt> [2500] - number of Strehls to store in the Strehls circular buffer
  *    - <b>rec_tech</b> <tt>int</tt> [REC_ATLAS] - if 0 (REC_ATLAS), then the CPU is used for reconstruction. if 1 (REC_GPU) then the GPU is used for reconstruction.
  *    - <b>tel_diam</b> <tt>double</tt> [6.5] - primary diamter
  *    - <b>median_r0</b> <tt>double</tt> [8.0] - median Fried parameter
  *    - <b>median_r0_lam</b> <tt>double</tt> [0.55] - wavelength at which median Fried parameter
  *    - <b>fitting_A</b> <tt>double</tt> [0.232555] - fitting error A coefficient (Males et al, SPIE, 2012)
  *    - <b>fitting_B</b> <tt>double</tt> [-0.840466] - fitting error B coefficient (Males et al, SPIE, 2012)
  *    - <b>reflection_gain</b> <tt>double</tt> [2.] - gain to go from surface to phase WFE. (normally 2, but it's 4 on the CRO due to double pass)
  * 
  */
   
class reconstructor : public VisAOApp_standalone
{
   public:
      reconstructor(int argc, char **argv) throw (AOException);
      reconstructor(std::string, const std::string &conffile) throw (AOException);

      //~reconstructor();

   protected:
      ///Processes the config file
      void Create();

      recmat rmat;
      int valid_recmat;
      
      std::string recFileName;
   
      int n_modes;
      int ho_middle;
      
      int slopes_shmem_key; ///< The key for the shared memory for slopes
      sharedim_stack<unsigned char> slopes_sis; ///< The shared memory ring buffer for slopes storage
      sharedim<unsigned char> slopes_sim; ///< Shared memory image of slopes (a vector actually)
      

      sharedim_stack<float> strehl_sis; ///<Shared memory ring buffer for strehl data
      sharedim<float> * strehl_sim; ///<Shared memory image, from ring buffer, for strehl data
      sharedim<float>  strehl_simavg; ///<Shared memory image, from ring buffer, for strehl data
      
      float lambda;
      float lamgain;

      std::string filter_name;
      fir_filter fir;

      float cal_a;
      float cal_b;
      
      float sumv;
      float rawstrehl;
      float filtstrehl;
      
      float filt_tip;
      float tilt_tilt;
      float tt_wfe;
      float ho1_wfe;
      float ho2_wfe;
      float filt_tt_wfe;
      float filt_ho1_wfe;
      float filt_ho2_wfe;
      float rms_tt_wfe;
      float rms_ho1_wfe;
      float rms_ho2_wfe;
      
      rmsAccumulator<float> rmsAccum;
      
      float * strehldata;
      float * wfedata;
      
      float *slopes; ///Pointer to the slopes vector.
      
      int strehls_shmem_key;
      int num_strehls;

      std::string ping_fifo_path;

      pthread_mutex_t reconMutex;
      
      VisAO::reconstructor_status_board * rsb;
      VisAO::aosystem_status_board * aosb;
      
      bool FSPing_enabled;
      
   public:

      int set_matint(std::string matintfile);

      int set_lambda(float lam);
      float get_lambda(){ return lambda;}

      int set_filter(std::string fname);
      std::string get_filter(){ return filter_name;}

      int set_cal_a(float a);
      float get_cal_a(){return cal_a;}
      int set_cal_b(float b);
      float get_cal_b(){return cal_b;}

      int reconstruct();

      virtual int Run();

      std::string local_command(std::string com);
      std::string script_command(std::string com);
      
      std::string common_command(std::string com, int cmode);
      
   public:
      ///Update the reconstructor Status Board shared memory.
      virtual int update_statusboard();
};

//read the ping fifo channel
void frame_ready(int signum, siginfo_t *siginf, void *ucont);

} //namespace VisAO

#endif //__reconstructor_h__
