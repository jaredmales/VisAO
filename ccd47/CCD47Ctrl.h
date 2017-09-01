/************************************************************
*    CCD47Ctrl.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for the VisAO CCD47 controller.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file CCD47Ctrl.h
  * \author Jared R. Males
  * \brief Declarations for the VisAO CCD47 controller.
  *
  * VisAO specific CCD47 controller.  Instead of trying to calculate the correct program (as the adopt version does)
  * we expect this to be explicitly set by the user, along with gain and repititions.  We'll eventually have to adapt this
  * for arbitrary integrations (using the TTL input on the Little Joe).
  * 
  * Is a standalone VisAOApp, doesn't depend on MsgD.
*/


#ifndef __CCD47Ctrl_h__
#define __CCD47Ctrl_h__

#include <vector>
#include <string>

#include "VisAOApp_standalone.h"
#include "AOStates.h"

extern "C" 
{

#include <stdio.h>
#include <string.h>             // strncmp()
#include <stdlib.h>             // atoi()
#include <stdarg.h>
#include <time.h>               // time(), localtime()
//#include <unistd.h>             // sleep()
#include <pthread.h>

// Joe related libs
#include "hwlib/netseriallib.h"
#include "hwlib/joelib.h"

// General Supervisor libs
#include "base/thrdlib.h"
}//extern "C" 

#include "EDTutils.h"

//#define _debug
   
namespace VisAO
{
  
///Holds the details of one little joe program
/** A few differences from the Arcetri version.
  */ 
typedef struct
{
    std::string name; ///< Program name
    int readout_speed; ///< Program readout speed, pixel rate in khz
    int binx; ///< x binning
    int biny; ///< y binning
    int windowx; ///< x window size
    int windowy; ///< y window size
    float32 delay_base; 
    float32 delay_inc;
    /// Array of default black levels.
    /** This should be long as the highest no. of black level possible.
      */
    int black_levels[4];
    
    std::string EDT_cfg_fname; ///<Name of the EDT config file for this program
    
} littlejoe_program;

///Holds the details of a little joe program set (i.e. a pattern/con file pair)
/** A few differences from the Arcetri version.
  */ 
typedef struct
{
   std::string name; ///< Name of the program set
   std::string control_filename; ///< Name of the control file
   std::string pattern_filename; ///< Name of the pattern file
   std::vector<littlejoe_program> programs; ///< Details of the individual programs
   double load_time; ///< Typical time for this programs set to upload to the little joe.
} littlejoe_programset;


///The controller for the VisAO CCD47 via its Little Joe
/** Mostly based on the Arcetri JoeCtrl class.
  * This is the VisAO specific CCD47 controller.  Instead of trying to calculate the correct program (as the adopt version does)
  * we expect this to be explicitly set by the user, along with gain and repititions.  We'll eventually have to adapt this
  * for arbitrary integrations.
  *
  * See \ref VisAOApp_standalone for command line arguments.  There are no additional command line arguments for CCD47Ctrl.
  *
  * This is a standalone VisAOApp, so it doesn't depend on MsgD.  In addition to the optional standard config options inherited from \ref VisAOApp_standalone this class REQUIRES:
  *    - <b>ccd47_name</b> <tt>string</tt> - name of the ccd, usually ccd47, used to find the adopt config file, e.g. ccd47.conf
  *    - <b>EDT_cfgdir</b> <tt>string</tt> - directory where the EDT configuration files are stored.
  *
  * The config file referenced by ccd47_name must have the following
  *      values in it:
  *    - <b>ccdName</b> <tt>string</tt> - Name of the ccd, in this case always ccd47, used for conf file lookup.
  *    - <b>ccdNetAddr</b> <tt>string</tt> - IP address, or host name, of the port server
  *    - <b>ccdNetPort</b> <tt>int</tt> - Port number of the ccd on the port server
  *    - <b>ccdXdim</b> <tt>int</tt>
  *    - <b>ccdYdim</b> <tt>int</tt>
  *    - <b>ccdDefaultXbin</b> <tt>int</tt>
  *    - <b>ccdDefaultYbin</b> <tt>int</tt>
  *    - <b>ccdDefaultSpeed</b> <tt>int</tt>
  *    - <b>ccdDefaultBlack</b> <tt>int</tt>
  *    - <b>ccdBlacksNum</b> <tt>int</tt>
  *    - <b>minRep</b> <tt>int</tt>
  *    - <b>maxRep</b> <tt>int</tt>
  *    - <b>maxNumSpeeds</b> <tt>int</tt>
  *    - <b>maxNumBins</b> <tt>int</tt>
  *    - <b>startProgramSet</b> <tt>int</tt>
  *    - <b>startProgram</b> <tt>int</tt>
  *    - <b>startGain</b> <tt>int</tt>
  *    - <b>startReps</b> <tt>int</tt>
  *
  */

class CCD47Ctrl: public VisAOApp_standalone 
{
   public:
      /// Name and config file constructor.
      CCD47Ctrl( std::string name, const std::string &conffile) throw (AOException);

      /// Command line constructor.
      CCD47Ctrl( int argc, char **argv) throw (AOException);

   protected:

      /// Common creation tasks, called by constructors.
      void Create(void) throw (AOException);

      /// Initialize the details of the VisAOApp.
      void init_VisAOApp();

      
      /// Name of the adopt config file (this is specified in the main config file)
      std::string     adopt_cfg_file;
      /// The dictionary of config details from the adopt config file.
      Config_File*    adopt_cfg;
      
      /// CCD name
      std::string _ccdName;

      /// CCD network address
      std::string _ccdNetAddr;

      ///The TCP/IP port of the CCD at its network address.
      int         _ccdNetPort;

      int          _ccdDx; ///< CCD x dimension
      int          _ccdDy; ///< CCD y dimension
      unsigned int _ccdNumSpeeds; ///< Number of speeds
      int          _maxNumSpeeds; ///< Maximum number of speeds
      int          _maxNumBins; ///< Maximum number of bins
      
      int          _ccdBlacksNum; ///< Number of blacks
      int          _minRep; ///< Minimum repetitions
      int          _maxRep; ///< Maximum repetitions.

      ///EDT configuration files directory
      std::string EDT_cfgdir; ///<EDT configuration file directory
      
      std::string FrameGrabberName; ///<Process name of the relevant framegrabber
      std::string FrameWriterName; ///<Process name of the relevant framewriter

      int _ccdDefaultXbin; ///< Default x binning
      int _ccdDefaultYbin; ///< Default y binning
      int _ccdDefaultSpeed; ///< Default speed
      int _ccdDefaultBlack; ///< Default black levels

      //Current values:
      int cur_State; ///< Current state

      /// Whether or not the framegrabber is running.
      int FGrunning;

      /// Whether or not the framegrabber is saving, and how many to save.
      /** If -1 then saving indefinately.
       */
      int FGsaving;

      /// How many saves are remaining.
      int FGremaining;
      
      /// Whether or not the framegrabber is skipping, and how many frames to skip.
      int FGskipping;

      /// The current subdirectory of the framewriter
      std::string FWsubdir;
      
      /// Image type, 0=science (default), 1=acquisition, 2=dark, 3=sky
      int imtype;
      
      ///The Framewriter47 status board is used to check if framewriter is running before saving.
      basic_status_board *fw47sb;
   
      int cur_ProgramSet; ///< The currently loaded program set
      int cur_Program; ///< The currently running program
      int cur_Gain; ///< The currently loaded gain
      int cur_Reps; ///< The currently loaded repetitions.
      int cur_xbin;  ///< Current x binning
      int cur_ybin; ///< Current y binning
      int cur_windowx; ///< Current x window size
      int cur_windowy; ///< Current y window size
      int cur_speed; ///< Current speed
      
      double cur_framerate; ///< Current frame rate
      double temps[3]; ///< Current temperatures

      int blackLevel[2]; ///< Current blacklevels

      /// Struct array representing files that can be uploaded to LittleJoe.
      std::vector<littlejoe_programset> ondisk;
      int _startProgramSet;
      int _startProgram;
      int _startGain;
      int _startReps;

      /// Load the configuration files specifying the different program sets that can be uploaded to LittleJoe
      int LoadJoeDiskFiles(void);

      /// Read a configuration file with the parameters of a little Joe Program SET
      littlejoe_programset ReadProgramSet( Config_File &cfg);
      
      /// Read a configuration file with the parameters of a single LittleJoe program
      littlejoe_program ReadProgram( Config_File &cfg);

      ///Start sequencing and start the framegrabber
      int Start(void);

      ///Stop sequencing and stop the framegrabber
      int Stop(void);

      ///Sets the LittleJoe controller in a known state
      /** \retval NO_ERROR on success
        * \todo Make this do something usefule.
        */
      int FirstJoeConfig(void);

      ///Check the LittleJoe CCD camera status
      /** Gets the CCD status from the serial interface and checks
       * the status of the frame grabber process.
       * \retval NO_ERROR on success
       */
      int ReadJoeStatus();
      
      ///Read the LittleJoe CCD camera temperatures
      /** Reads the CCD temperature from the serial interface and logs
        * them to a file. 
        * \retval NO_ERROR on success
        */
      int ReadJoeTemps(void);

      ///Checks whether the framegrabber process is running, and what its current status is.
      int GetFramegrabberStatus();

      ///Setup network connection with LittleJoe
      /** This function sets up the network connection with the LittleJoe CCD Camera.
        * After the network is up, the communication is tested with the TestJoe() function.
        * \retval 0 on success
        * \retval <0 otherwise (errordb.h code)
        */
      int SetupNetwork(void);

      ///Reprogram the little joe, including file uploads as necessary
      /** Actually launches a thread, which does the work using __ReprogramJoe
        * which is necessary to prevent uploads from being interrupted by signals.
        */
      int ReprogramJoe( int program_set, int program, int gain, int rep, int force=0);

      ///Only change the settings in this software, do not reprogram the little joe
      /** This is useful if a software restart is necessary, but the Little Joe is still running.
        */
      int swonly_reprogram( int program_set, int program, int gain, int rep);

      pthread_t load_thread; ///<Identifier for the __ReprogramJoe thread

      ///Mutex for the __ReprogramJoe thread
      /** Prevents multiple simultaneous reprograms.
        */
      pthread_mutex_t reprogMutex;

      ///Tells the main thread that the __ReprogramJoe thread is working
      /** is true until the main thread can try_join on load_thread.
        */
      int joe_loading;

      
   public:
      ///Return the network address of the CCD
      std::string get_ccdNetAddr(){return _ccdNetAddr;}
   
      ///Returns the TCP/IP port of the CCD at its network address
      int get_ccdNetPort(){return _ccdNetPort;}
   
      ///This function does the actual reprogramming, based on the adopt version.
      int __ReprogramJoe( int program_set, int program, int gain, int rep, int force=0);
      
      ///Set the image type
      int set_imtype(int it);
      
   protected:
      /// Computes the current framerate from the current state of LittleJoe control variables.
      /** \retval NO_ERROR on success
       */
      int ComputeFramerate();

      /// Computes the reps required to match a requested framerate
      int ComputeRepsFrameRate(double fr);
            
      /// Computes the reps required to match a requested exptime
      int ComputeRepsExpTime(double et);
            
      /// Get the complete path of a program/pattern file
      std::string getCompletePath( std::string filename);

      /// Performs controller functions and manages states
      /** Switches the controller from one state to another. States can be changed asynchronously
        * from this thread or the listening thread, and this function will properly react to a new state.
        * Some states include a sleep() function to slow down the thread when immediate action is not necessary
        */
      int DoFSM(void);

      ///Temperature logger
      Logger *_tempsLogger;

      ///Loads the EDT configuration file.
      /** \param fname is the path to the config file
        * \retval 0 on success
        */
      int load_EDT_config(std::string);
      
   public:
      ///Overriden virtual function, called by Exec().
      int Run();
      
      /// Called by \ref __remote_command after control state logic.
      /** This should be overriden by derived classes.
        * \param com is the command string to be processed.
        * \returns the application response.
        */
      virtual std::string remote_command(std::string com);
            
      /// Called by \ref __local_command after control state logic.
      /** This should be overriden by derived classes.
        * \param com is the command string to be processed.
        * \returns the application response.
        */
      virtual std::string local_command(std::string com);
            
      /// Called by \ref __script_command after control state logic.
      /** This should be overriden by derived classes.
        * \param com is the command string to be processed.
        * \returns the application response.
        */
      virtual std::string script_command(std::string com);
      
      ///Called by all the command handlers, to perform common command processing.
      std::string common_command(std::string com, int cmode);
      
   public:
      ///Update the CCD47 Status Board shared memory.
      virtual int update_statusboard();
};

///Set the recall "RCL" parameter on the little joe (pixel readout, gain & related settings)
/** \param rcl program number 0-31 inclusive
  * \retval 0 on success
  * \retval <0 otherwise (errordb code)
  */
int SetJoeRCL( int rcl);

///This is the thread start function for reprogramming the little joe
/** First blocks SIGIO so uploads aren't interrupted
  * Then calls __ReprogramJoe
  */
void * ReprogramJoeThreadWorker(void * adata);

} //namespace VisAO

#endif //__CCD47Ctrl_h__
