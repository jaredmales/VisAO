/************************************************************
*    libvisao.h
*
* Author: Jared R. Males (jrmales@as.arizona.edu)
*
* VisAO software utilitites, declarations
*
************************************************************/

/** \file libvisao.h
  * \author Jared R. Males
  * \brief VisAO software utilitites, declarations
  * 
*/

#ifndef __libvisao_h__
#define __libvisao_h__

#include "time.h"
#include "sys/time.h"

#include <sys/shm.h>

#include "fifoutils.h"

/// The real-time scheduling policy used  by the VisAO system.
#define VISAO_SCHED_POLICY  SCHED_FIFO

#define PROMPT_UP    1
#define PROMPT_DOWN  2
#define PROMPT_INT   3

#define FTYPE_NORMAL 0
#define FTYPE_SDI    1
#define FTYPE_CORON  2

#define RTSIGTIMEOUT (RTSIGPING + 1)

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

///Number of physical processors in the system
#define SYS_N_CORES 6

///Number of virtual processors in the system
#define SYS_N_VCORES 12

///Number of physical drives in the system
#define SYS_N_DRV  2

///Number of logical drives in the system
#define SYS_N_LOGDRV  6


///A global error reporting function, arguments are the report, file, and line.
/** This is used by various library functions and classes that aren't derived from one of the Apps.
  */
extern void (*global_error_report)(const char*, const char*, int);

///Macro for declaring and setting the error reporting global.
#define set_global_error_report(er) void (*global_error_report)(const char*, const char*, int) = er

///A global info logging function.
/** This is used by various library functions and classes that aren't derived from one of the Apps.
  */
extern void (*global_log_info)(const char*);

///Macro for declaring and setting the info logging global.
#define set_global_log_info(li) void (*global_log_info)(const char*) = li


/// The fifoutils error reporting function.
/** This passes the ERRMSG macro in fifoutils out to the global_error_report.
  */
void fifo_error_message(const char *, const char *, int);

/// Fills in the properly formatted dioserver channel fifo names
int get_dio_fnames(char *fout, char *fin, char *fbase, int ch);

/// Convert a timespec structure to double seconds.
double ts_to_curr_time(struct timespec *tsp);

/// Convert a timeval structure to double seconds.
double tv_to_curr_time(struct timeval *tvp);

/// Gets the current CLOCK_REALTIME time, returns it in seconds to double precision.
double get_curr_time(void);


/// Create a shared memory buffer
int create_shmem(int * shmemid, key_t mkey, size_t sz);

/// Attach to a shared  memory buffer and get its size.
void * attach_shm(size_t *sz,  key_t mkey, int shmemid);

/// Calculate frame rate given Little Joe program parameters
double ComputeFramerate(double delay_base, double delay_inc, int rep);

int ComputeRepsFrameRate(double delay_base, double delay_inc, double fr);

int ComputeRepsExpTime(double delay_base, double delay_inc, double et);

      
/// Signal proof sleep function.
/** Sleeps for at least secs seconds, even if interrupted by a handled signal.
  * May sleep for longer if signal handling takes a long time.
  * Monitors the integer pointed by dienow and exits if it becomes 1
  * set dienow to 0 to ignore.
  */
int sigproof_sleep(double secs, int *dienow);

#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus


#ifdef __cplusplus

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>

/// Read /proc/mdstat and parse for status of the 2 RAID 1 arrays.
int visao_mdstat(char dstat[SYS_N_LOGDRV]);

///Save a preset file for, e.g., the focus or gimbal
/** \param calname is the name of the directory in calib/visao where the presets directory for this device is
 * \param fw1pos is the current position of F/W 1 position, which will be rounded to the nearest integer
 * \param wollstat is the current status of the wollaston.
 * \param fw2pos is the current position of F/W 2, which will be rounded to the nearest integer
 * \param fw3pos is the current position of F/W 3, which will be rounded to the nearest integer
 * \param vec is a pointer to a vector containing the presets.  The number of presets read from the file is determined by the size of vec.
 * \retval -1 if the file can't be created or saved
 * \retval 0 on success.
 */
int save_preset(std::string calname, double fw1pos, int wollstat, double fw2pos, double fw3pos, std::vector<double> *vec);

///Read the focus calibration
/** \param fcal [output] is the read focus position
  * \retval -1 if the file doesn't exist or can't be opened
  * \retval 0 on success.
  */ 
int get_focuscal(double * fcal);
      
///Read a preset file for, e.g., the focus or gimbal
/** \param calname is the name of the directory in calib/visao where the presets directory for this device is
  * \param fw1pos is the current position of F/W 1 position, which will be rounded to the nearest integer
  * \param wollstat is the current status of the wollaston.
  * \param fw2pos is the current position of F/W 2, which will be rounded to the nearest integer
  * \param fw3pos is the current position of F/W 3, which will be rounded to the nearest integer
  * \param vec is a pointer to an allocated vector to return the presets in.  The number of presets read from the file is determined by the size of vec.
  * \param presetf is a string that will be filled with the preset file name
  * \retval -1 if the file doesn't exist or can't be opened
  * \retval 0 on success.
  */
int get_preset(std::string calname, double fw1pos, int wollstat, double fw2pos, double fw3pos, std::vector<double> *vec, std::string &presetf);



#endif //__cplusplus

           
#endif //__libvisao_h__
