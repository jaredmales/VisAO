/************************************************************
*    sysmonD.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the sysmonD system monitor
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file sysmonD.h
  * \author Jared R. Males
  * \brief Declarations for the sysmonD system monitor
   *
  * The VisAO sysmonD monitors system parameters (temperatures, memory usage, raid status).
  * 
  * This is a standalone VisAOApp, doesn't depend on MsgD.
*/


#ifndef __sysmonD_h__
#define __sysmonD_h__


#include "VisAOApp_standalone.h"
#include "libvisao.h"

#include <fstream>

#include "QTITempProbe.h"

namespace VisAO
{

///The VisAO system monitor
/** Periodically queries various system temperatures and other parameters.  Makes use of some shell scripts to parse 
  * various /cat/proc and utility program outputs.  Temperatures are written to a TELEMETRY log.
  *
  * See \ref VisAOApp_standalone for command line arguments.  There are no additional command line arguments for sysmonD.
  *
  * This is a standalone VisAOApp, so it doesn't depend on MsgD.  In addition to the optional standard config options inherited from \ref VisAOApp_standalone this class has the following also optional config parameters:
  *    - <b>airTemp_serial_no</b> <tt>string</tt> - the serial number of the QTI temperature probe monitoring VisAO box air temp.
  *    - <b>joe47Temp_serial_no</b> <tt>string</tt> - the serial number of the QTI temperature probe monitoring little joe 47 
  *                                                   exhaust temp.
  *    - <b>qtiAmbSerialNo</b> <tt>string</tt> - the serial number of the QTI temperature probe monitoring ambient temp.
  *
  *
  *
  */

class sysmonD : public VisAOApp_standalone
{
   public:
      /// Command line constructor.
      sysmonD(int argc, char **argv) throw (AOException);
      
      /// Config file constructor.
      sysmonD(std::string name, const std::string &conffile) throw (AOException);  

      void init_sysmonD();
      
   protected:
      double core_temps[SYS_N_CORES];
      double core_max[SYS_N_CORES];
      double core_idle[SYS_N_VCORES];

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

      int GPUTemp;
      int GPUMemUsage;

      ifstream fin;
      int get_sysstat();
      int get_GPUstat();
      int get_HDDstat();

      int HDDTemp_a;
      int HDDTemp_b;

      QTITempProbe * qtiProbes;
      int nprobes;

      bool haveQTIAir;
      int qtiAirIndex;
      std::string qtiAirSerialNo;

      bool haveQTIJoe47;
      int qtiJoe47Index;
      std::string qtiJoe47SerialNo;

      bool haveQTIAmb;
      int qtiAmbIndex;
      std::string qtiAmbSerialNo;

      double airTemp;
      double joe47Temp;
      double ambTemp;

      int setupQTI();

      ///Temperature logger
      Logger *_tempsLogger;
      
      double core_temp_warn;
      double core_temp_limit;
      double hdd_used_warn;
      double hdd_used_limit;
      double hdd_temp_warn;
      double hdd_temp_limit;
      double gpu_temp_warn;
      double gpu_temp_limit;
      double air_temp_warn;
      double air_temp_limit;
      double joe_temp_warn;
      double joe_temp_limit;
      

public:
      void qtiTimeout();
protected:

      int get_QTITemps();

      /// The main loop.
      /** Just installs the kill handler and starts pausing and updating.
        */
      virtual int Run();
      
   public:
      virtual int update_statusboard();

};//class sysmonD

}//namespace VisAO

#endif //__sysmonD_h__
