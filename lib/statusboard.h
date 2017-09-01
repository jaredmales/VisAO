/************************************************************
*    statusboard.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the VisAO status information structures
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file statusboard.h
  * \author Jared R. Males
  * \brief Declarations for the VisAO status information structures.
  *
*/


#ifndef __statusboard_h__
#define __statusboard_h__

#include <iostream>

/*Status board shared memory ID:*/

///Shared memory key for ccd47 data
#define DATA_framegrabber47 5000
///Shared memory key for BCU39 data
#define DATA_framegrabber39 5001

#define DATA_47darks 5002

#define DATA_39darks 5004

///Shared memory key for reconstructed strehl data
#define DATA_strehls        5003


///Shared memory key for the focus motor controller status board
#define STATUS_focusstage     6000
///Shared memory key for the ccd47 controller status board
#define STATUS_ccd47  7000
///Shared memory key for the ccd47 framegrabber status board
#define STATUS_framegrabber47  7050
///Shared memory key for the ccd47 framewriter status board
#define STATUS_framewriter47   7100
///Shared memory key for the ccd47 frameserver status board
#define STATUS_frameserver47   7150
///Shared memory key for the reconstructor status board
#define STATUS_reconstructor   7500
///Shared memory key for the BCU39 framegrabber status board
#define STATUS_framegrabber39  7550
#define STATUS_framewriter39   7600

#define STATUS_shutterctrl   8000
#define STATUS_filterwheel2  9000
#define STATUS_filterwheel3  9500
#define STATUS_sysmonD      10000
#define STATUS_wollaston    11000
#define STATUS_aosystem     12000
#define DL_aosystem         12001

#define STATUS_power        13000

///Shared memory key for the Gimbal mirror status board
#define STATUS_gimbal   14000

///Shared memory key for the Coronagraph Guider status board
#define STATUS_coronguide 14100

///Shared memory key for the dioserver status board
#define STATUS_dioserver   15000

///Shared memory key for the frame selector status board
#define STATUS_frameselector 16000

///Shared memory key for imviewer status board
#define STATUS_imviewer  17000

///Shared memory key for Zaber stage status board
#define STATUS_zaber 18000

///Shared memor key for HWP PI stage status board
#define STATUS_hwp  19000

///Shared memory key for shutter tester status board
#define STATUS_shuttertester 22000


namespace VisAO
{

struct basic_status_board
{
   char appname[25];
   int control_mode;
   
   struct timespec last_update;
   double max_update_interval;
};


struct focusstage_status_board : basic_status_board
{
   int power_state; //0 off, 1 on
   int is_moving;
   int cur_dir;
   int cur_enabled;
   int homing;
   double cur_pos; //position in microns
   int is_focused;
   
};


struct ccd47_status_board : basic_status_board
{
   int status;
   int speed;
   int xbin;
   int ybin;
   int windowx;
   int windowy;
   int repetitions;
   double framerate;
   int gain;
   double joe_temp;
   double head_temp1;
   double head_temp2;
   int black0;
   int black1;

   int saving;
   int skipping;
   int remaining;

   int imtype; ///<0=science, 1=acquisition, 2=dark, 3=sky, 4=flat
};
   
struct framegrabber39_status_board : basic_status_board
{
   int running;
   int reconstructing;
   int saving;
   double fps;
};

struct reconstructor_status_board : basic_status_board
{
   double avgwfe_1_sec; ///<1 second average WFE
   double stdwfe_1_sec; ///<1 second std dev of WFE
   double inst_wfe; ///<Instantaneous WFE
};


struct shutter_status_board : basic_status_board
{
   int in_auto;
   int power_state;
   int sw_state;
   int hw_state;
   int sync_enabled;
};

struct shutter_tester_status_board : basic_status_board
{
   int RunTest; //1 = test running, 0 = no test running
   int testmode;
   char testfile[255];
   double threshold;
   double freq;
   double dutycyc;
   double duration;
};

struct filterwheel_status_board : basic_status_board
{
   int state;
   double pos;
   double req_pos;
   char filter_name[256];
   int type;
   int req_type;
};

struct system_status_board : basic_status_board
{
   double core_temps[SYS_N_CORES];
   double core_max[SYS_N_CORES];
   double core_idle[SYS_N_VCORES];

   double core_temp_warn;
   double core_temp_limit;
   
   
   
   
   char raid_stat[SYS_N_LOGDRV];

   size_t mem_tot;
   size_t mem_used;
   size_t mem_free;
   size_t mem_shared;
   size_t mem_buff;
   size_t mem_cached;

   size_t swap_tot;
   size_t swap_used;
   size_t swap_free;

   size_t dfroot_size;
   size_t dfroot_used;
   size_t dfroot_avail;

   double hdd_used_warn;
   double hdd_used_limit;
   
   int GPUTemp;
   int GPUMemUsage;

   double gpu_temp_warn;
   double gpu_temp_limit;
   
   int HDDTemp_a;
   int HDDTemp_b;

   double hdd_temp_warn;
   double hdd_temp_limit;
   
   double AirTemp;
   double Joe47Temp; 
   
   double air_temp_warn;
   double air_temp_limit;
   
   double joe_temp_warn;
   double joe_temp_limit;
   
};

struct wollaston_status_board : basic_status_board
{
   double cur_pos; //1 up, 0 intermediate, -1 down
   
};

struct aosystem_status_board : basic_status_board
{
   //char tgtname[256];

   char dateobs[256];
   int ut;
   double epoch;
   double ra;
   double dec;
   double az;
   double el;
   double am;
   double pa;
   double ha;
   double zd;
   int st;
   
   int istracking;
   int isguiding;
   int isslewing;
   int guider_ismoving;

   double catra;
   double catdec;
   double catep;
   double catro;
   char catrm[256];
   char catobj[256];
   char obsinst[256];
   char obsname[256];
         
   double rotang;
   double rotoffset;
   int rotfollowing;
   
   double wxtemp;
   double wxpres;
   double wxhumid;
   double wxwind;
   double wxwdir;
   double wxdewpoint;
   double wxpwvest;
   double ttruss;
   double tcell;
   double tseccell;
   double tambient;
   double dimmfwhm;
   int dimmtime;
   double mag1fwhm;
   int mag1time;
   double mag2fwhm;
   int mag2time;
   
   double filter1_pos;
   double filter1_reqpos;
   char filter1_name[256];

   double baysidex;
   int baysidex_enabled;
   double baysidey;
   int baysidey_enabled;
   double baysidez;
   int baysidez_enabled;
   
   int correctedmodes;
   int loop_on;
   int loop_open_counter;
 
   char modal_basis[256];
   char reconstructor[256];
   int n_modes;
   int homiddle;
   char loop_gains[256];
   double loop_gain_tt;
   double loop_gain_ho1;
   double loop_gain_ho2;

   double tt_amp[3];
   double tt_freq[3];
   double tt_offset[3];
   double ccd39_freq;
   int ccd39_bin;
   int wfs_counts;

   int nodInProgress;
   
   int orient_useel;
   int orient_usepa;
   
   //camera lens position, rirot ang
};

struct power_status_board : basic_status_board
{
   int outlet1_state;
   char outlet1_name[50];

   int outlet2_state;
   char outlet2_name[50];

   int outlet3_state;
   char outlet3_name[50];

   int outlet4_state;
   char outlet4_name[50];

   int outlet5_state;
   char outlet5_name[50];

   int outlet6_state;
   char outlet6_name[50];

   int outlet7_state;
   char outlet7_name[50];

   int outlet8_state;
   char outlet8_name[50];
};

struct gimbal_status_board : basic_status_board
{
   double xpos;
   double ypos;
   int power;
};

struct imviewer_status_board : basic_status_board
{
   //int guidebox_vis;
   int guidebox_x0;
   int guidebox_x1;
   int guidebox_y0;
   int guidebox_y1;
   
   double min;
   double max;
   double mean;
   
   double fit_FWHM;
   double fit_FWHM_min;
   double fit_FWHM_max;

   double fit_Ellipt;

   double fit_Peak;
   double fit_X;
   double fit_Y;
   
   
};

struct coronguide_status_board :  basic_status_board
{
   int loopState;
   double loop_gain;
   double tgtx;
   double tgty;
   double xcen;
   double ycen;
};

struct frameselector_status_board :  basic_status_board
{
   int frame_select;
   double thresh;
   
};

struct zaber_status_board : basic_status_board
{
   int state;
   double pos; 
   
};

struct hwp_status_board : basic_status_board
{
   int state;
   double pos; 
   
};

}//namespace VisAO

#endif //__statusboard_h__


