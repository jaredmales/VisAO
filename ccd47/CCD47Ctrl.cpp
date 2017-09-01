/************************************************************
*    CCD47Ctrl.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for the VisAO CCD47 controller.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file CCD47Ctrl.cpp
  * \author Jared R. Males
  * \brief Definitions for the VisAO CCD47 controller.
  *
*/

#include "CCD47Ctrl.h"

#define JOE_ADDR_LEN            15                      // Length of idrive string address
#define ERRMSG_LEN              32                      // Length of an error message

//#define NOPDV
//#define _debug

#define FG_FIFO 0
#define FW_FIFO 1

namespace VisAO
{
   

CCD47Ctrl::CCD47Ctrl( std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   Create();
}

CCD47Ctrl::CCD47Ctrl( int argc, char **argv) throw (AOException) : VisAOApp_standalone( argc, argv)
{
   Create();
}

void CCD47Ctrl::Create() throw (AOException)
{
   try
   {
      adopt_cfg_file = (std::string)ConfigDictionary()["ccd47_name"];
      adopt_cfg = new Config_File(Utils::getConffile(adopt_cfg_file));
   }
   catch (Config_File_Exception &e) 
   {
      _logger->log( Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
      throw AOException("Fatal: cannot find adopt ccd47 configuration");
   } 

   //adopt_cfg = &ConfigDictionary();
   
   try
   {
      //_ccdNum   = (*adopt_cfg)["ID"];
      _ccdName  = (std::string) (*adopt_cfg)["ccdName"]; 
      _ccdNetAddr = (std::string) (*adopt_cfg)["ccdNetAddr"];
      _ccdNetPort = (*adopt_cfg)["ccdNetPort"];
      _ccdDx    = (*adopt_cfg)["ccdXdim"];
      _ccdDy    = (*adopt_cfg)["ccdYdim"];
      _ccdDefaultXbin = (*adopt_cfg)["ccdDefaultXbin"];
      _ccdDefaultYbin = (*adopt_cfg)["ccdDefaultYbin"];
      _ccdDefaultSpeed = (*adopt_cfg)["ccdDefaultSpeed"];
      _ccdDefaultBlack = (*adopt_cfg)["ccdDefaultBlack"];
      _ccdBlacksNum    = (*adopt_cfg)["ccdBlacksNum"];
      _minRep          = (*adopt_cfg)["minRep"];
      _maxRep          = (*adopt_cfg)["maxRep"];
      _maxNumSpeeds    = (*adopt_cfg)["maxNumSpeeds"];
      _maxNumBins      = (*adopt_cfg)["maxNumBins"];
      //_filePrefix      = (std::string) (*adopt_cfg)["filePrefix"];
      _startProgramSet = (*adopt_cfg)["startProgramSet"];
      _startProgram = (*adopt_cfg)["startProgram"];
      _startGain = (*adopt_cfg)["startGain"];
      _startReps = (*adopt_cfg)["startReps"];

      EDT_cfgdir = (std::string)ConfigDictionary()["EDT_cfgdir"];

      init_VisAOApp();
     
   } 
   catch (Config_File_Exception &e) 
   {
      _logger->log( Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
      throw AOException("Fatal: Missing configuration data");
   }


   
   _tempsLogger = Logger::get( _ccdName+"Temperature", Logger::LOG_LEV_INFO, "TELEMETRY");
    
   // Create the specific LittleJoe arrays
   LoadJoeDiskFiles();

   cur_State = STATE_UNDEFINED;
   
   cur_ProgramSet = _startProgramSet;
   cur_Program = _startProgram;
   cur_Gain = _startGain;
   cur_Reps = _startReps;
   
   cur_speed = ondisk[cur_ProgramSet].programs[cur_Program].readout_speed;
   cur_xbin = ondisk[cur_ProgramSet].programs[cur_Program].binx;
   cur_ybin = ondisk[cur_ProgramSet].programs[cur_Program].biny;
   cur_windowx = ondisk[cur_ProgramSet].programs[cur_Program].windowx;
   cur_windowy = ondisk[cur_ProgramSet].programs[cur_Program].windowy;
   

   // Update shown framerate
   ComputeFramerate();

   FGsaving = 0;
   FGrunning = 0;
   
   imtype = 0; //Default imtype is science
   
   //Init the status board
   statusboard_shmemkey = STATUS_ccd47;
   if(create_statusboard(sizeof(ccd47_status_board)) != 0)
   {
      statusboard_shmemptr = 0;
      _logger->log(Logger::LOG_LEV_ERROR, "Could not create status board.");
   }
   else
   {
      VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;
      strncpy(bsb->appname, MyFullName().c_str(), 25);
      bsb->max_update_interval = pause_time;
   }

   size_t sz;
   fw47sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_framewriter47, 0);
}



void CCD47Ctrl::init_VisAOApp()
{
   std::string fifo_path, visao_root, fgin, fgout;

   try 
   {
      fifo_path   = (std::string) ConfigDictionary()["fifo_path"];
   }
   catch (Config_File_Exception &e) 
   {
      fifo_path = "fifos";
   }

   try 
   {
      FrameGrabberName = (std::string)ConfigDictionary()["FrameGrabberName"];
   }
   catch (Config_File_Exception &e) 
   {
      FrameGrabberName = "framegrabber47";
   }

   try
   {
      FrameWriterName = (std::string)ConfigDictionary()["FrameWriterName"];
   }
   catch (Config_File_Exception &e) 
   {
      FrameWriterName = "framewriter47";
   }

   visao_root = getenv("VISAO_ROOT");




   setup_fifo_list(5);
   setup_baseApp(1,1,1,0,false);


   fgin = visao_root +  "/" + fifo_path + "/" + FrameGrabberName + "_com_auto_in";
   fgout = visao_root + "/" + fifo_path + "/" + FrameGrabberName + "_com_auto_out";
   set_fifo_list_channel(&fl, FG_FIFO, RWBUFF_SZ, fgout.c_str(),fgin.c_str(), 0, 0);

   fgin = visao_root +  "/" + fifo_path + "/" + FrameWriterName + "_com_auto_in";
   fgout = visao_root + "/" + fifo_path + "/" + FrameWriterName + "_com_auto_out";
   set_fifo_list_channel(&fl, FW_FIFO, RWBUFF_SZ, fgout.c_str(),fgin.c_str(), 0, 0);

   //::global_fifo_list = &fl;
   
   pthread_mutex_init(&reprogMutex, 0);
}

int CCD47Ctrl::LoadJoeDiskFiles()
{
   int num, i;
   std::string prefix;

   num = (*adopt_cfg)["num_programsets"];

   ondisk.resize(num);

   for (i=0; i<num; i++)
   {
      char param[32];
      sprintf( param, "programset%d", i);
      Config_File *subtree = (*adopt_cfg).extract(param);
      ondisk[i] = ReadProgramSet( *subtree);
      delete subtree;
   }

   //if (debug) for (i=0; i<num; i++) DumpProgramset(ondisk[i]);

   _logger->log( Logger::LOG_LEV_INFO, "%d programsets loaded.", ondisk.size());
   return 0;
}//int CCD47Ctrl::LoadJoeDiskFiles()

littlejoe_programset CCD47Ctrl::ReadProgramSet( Config_File &cfg)
{
   int size;
   unsigned int i;

   littlejoe_programset programset;

   programset.name = (std::string) cfg["name"];
   programset.control_filename = (std::string) cfg["control_filename"];
   programset.pattern_filename = (std::string) cfg["pattern_filename"];
   size = cfg["num_programs"];

   if ((size<=0) || (size > 1000))
   {
      _logger->log( Logger::LOG_LEV_WARNING, "Skipping programset %s because it has %d programs", programset.name.c_str(), size);
      return programset;
   }

   programset.programs.resize( size);

   for (i=0; i< programset.programs.size(); i++)
   {
      char par_name[32];
      sprintf( par_name, "program%d", i);

      _logger->log( Logger::LOG_LEV_DEBUG, "Reading program %d of %d", i, programset.programs.size());
      Config_File *subtree = cfg.extract(par_name);
      programset.programs[i] = ReadProgram( *subtree);
      delete subtree;
   }

   return programset;
}//littlejoe_programset CCD47Ctrl::ReadProgramSet( Config_File &cfg)


littlejoe_program CCD47Ctrl::ReadProgram( Config_File &cfg)
{
   littlejoe_program program;

   program.name = (std::string) cfg["name"];
   program.readout_speed = cfg["readout_speed"];
   program.binx = cfg["binx"];
   program.biny = cfg["biny"];
   program.windowx = cfg["windowx"];
   program.windowy = cfg["windowy"];
   program.delay_base = cfg["delay_base"];
   program.delay_inc = cfg["delay_inc"];

   // Read default black levels if they exist, otherwise set to -1
   for (int i=0; i<4; i++) 
   {
      try 
      {
         char str[10];
         sprintf( str, "black%d", i+1);
         program.black_levels[i] = cfg[str];
      } 
      catch (Config_File_Exception &e) 
      {
         program.black_levels[i] = -1;
      }
   }

   program.EDT_cfg_fname = (std::string) cfg["EDT_cfg_fname"];
   
   _logger->log( Logger::LOG_LEV_DEBUG, "Found: speed %d, binx %d, biny %d, windowx %d, windowy %d, base %f, inc %f, EDT config %s", program.readout_speed, program.binx, program.biny, program.windowx, program.windowy, program.delay_base, program.delay_inc, program.EDT_cfg_fname.c_str());

   return program;
}//littlejoe_program CCD47Ctrl::ReadProgram( Config_File &cfg)

int CCD47Ctrl::Stop()
{
   int stat;
   std::string resp;

   _logger->log( Logger::LOG_LEV_DEBUG, "Stopping CCD");
        
   //Stop framegrabber here.

   write_fifo_channel(FG_FIFO, "AUTO", 4, &resp);
   write_fifo_channel(FG_FIFO, "stop", 4, &resp);
   

   stat = StopJoe();
        
   if (stat == NO_ERROR)
   {
      if(cur_State != STATE_CONFIGURING)
      {
         cur_State = STATE_READY;
      }
   }
                
   return stat;
}

int CCD47Ctrl::GetFramegrabberStatus()
{
   std::string resp;
   
   write_fifo_channel(FG_FIFO, "save?", 6, &resp);
   
   if(resp[0] == '1')
   {
      FGsaving = 1;
      write_fifo_channel(FG_FIFO, "remaining?", 10, &resp);
      FGremaining = atoi(resp.c_str());
   }
   else
   {
      FGsaving = 0;
      FGremaining = 0;
   }

   write_fifo_channel(FG_FIFO, "skip?", 6, &resp);
   
   FGskipping = atoi(resp.c_str());

   write_fifo_channel(FG_FIFO, "running?", 9, &resp);
   if(resp[0] == '1') FGrunning = 1;
   else FGrunning = 0;
   
   write_fifo_channel(FW_FIFO, "subdir?", 8, &resp);
   FWsubdir = resp;
   
   
   return 0;
}
   
int CCD47Ctrl::Start()
{

   int stat;
   std::string resp;
   _logger->log( Logger::LOG_LEV_DEBUG, "Starting CCD");
   #ifdef _debug
      std::cerr << "Starting CCD\n";
   #endif
   stat = StartJoe();
   
   if (stat == NO_ERROR)
   {
      _logger->log( Logger::LOG_LEV_DEBUG, "CCD Started");
      #ifdef _debug
         std::cerr << "CCD Started\n";
      #endif
      cur_State = STATE_OPERATING;
      
      //Start up the framegrabber
      write_fifo_channel(FG_FIFO, "AUTO", 5, &resp);

      write_fifo_channel(FG_FIFO, "start", 6, &resp);

      GetFramegrabberStatus();
   }
   else
   {
      _logger->log( Logger::LOG_LEV_ERROR, "Error starting CCD");
      #ifdef _debug
         std::cerr << "Not Started\n";
      #endif
   }

   return stat;

}

int CCD47Ctrl::FirstJoeConfig()
{
   
   //This should query 
   
   cur_ProgramSet = _startProgramSet;
   cur_Program = _startProgram;
   cur_Gain = _startGain;
   cur_Reps = _startReps;
   
   //int stat;
   //unsigned int i;

   //StopJoe();

   //return ReprogramJoe(_startProgramSet, 0, 0, 0, 1);
   
/*   cur_ProgramSet = _startProgramSet;
   cur_Program = 0;
   cur_Gain = 0;
   
   cur_xbin = ondisk[cur_ProgramSet].programs[cur_Program].binx;
   cur_ybin = ondisk[cur_ProgramSet].programs[cur_Program].biny;
   cur_windowx = ondisk[cur_ProgramSet].programs[cur_Program].windowx;
   cur_windowy = ondisk[cur_ProgramSet].programs[cur_Program].windowy;
   cur_rep = 0;

   // Update shown framerate
   ComputeFramerate();

   
   load_EDT_config(ondisk[cur_ProgramSet].programs[cur_Program].EDT_cfg_fname);*/

/*   for (i=0; i< _ccdBlacks.size(); i++)
   { 
      if ((stat=SetJoeBlack( i, default_black)) != NO_ERROR)
      {
         return stat;
      }
   } */        

   return NO_ERROR;

} //int CCD47Ctrl::FirstJoeConfig()

int CCD47Ctrl::set_imtype(int it)
{
   if(it <0 || it > 4) it = 0;
   
   imtype = it;
   
   if(statusboard_shmemptr)
   {
 
      VisAO::ccd47_status_board * csb = (VisAO::ccd47_status_board *) statusboard_shmemptr;
      
      csb->imtype = imtype;
   }
   return 0;
}


int CCD47Ctrl::ComputeFramerate()
{

   int xbin, ybin, speed, rep;
 
   xbin = cur_xbin;
   ybin = cur_ybin;
   speed = cur_speed;
   rep = cur_Reps;
   
   double framerate;

   littlejoe_program *program = &(ondisk[cur_ProgramSet].programs[cur_Program]);
   
   if (!program)
   {
      return VALUE_OUT_OF_RANGE_ERROR;
   }
      
   framerate = 1.0e6/ (program->delay_base + program->delay_inc * rep);
   
   _logger->log( Logger::LOG_LEV_DEBUG, "Delay base: %f - Delay_inc: %f - Rep: %d - framerate: %f", program->delay_base, program->delay_inc, rep, framerate);

   cur_framerate = framerate;
   
   return NO_ERROR;
}//int CCD47Ctrl::ComputeFramerate()

int CCD47Ctrl::ComputeRepsFrameRate(double fr)
{
   int xbin, ybin, speed, rep;
 
   xbin = cur_xbin;
   ybin = cur_ybin;
   speed = cur_speed;

   littlejoe_program *program = &(ondisk[cur_ProgramSet].programs[cur_Program]);
   
   if (!program)
   {
      return VALUE_OUT_OF_RANGE_ERROR;
   }
      
   
   rep = ((1.0e6/fr) - program->delay_base)/program->delay_inc;
      
   return rep;
}

int CCD47Ctrl::ComputeRepsExpTime(double et)
{
   return ComputeRepsFrameRate(1./et);
}


   
//Structure to hold instructions to pass to the reprogram thread.
struct ReprogramData
{
   CCD47Ctrl * ctrl;
   int program_set;
   int program;
   int gain;
   int rep;
   int force;
};

ReprogramData rpd;


//This is the thread start function for reprogramming the little joe
// First blocks SIGIO so uploads aren't interrupted
// Then calls __ReprogramJoe
void * ReprogramJoeThreadWorker(void * adata)
{
   static int rv;
   sigset_t set;
   sigemptyset(&set);
   sigaddset(&set, SIGIO);
   sigaddset(&set, RTSIGIO);
   pthread_sigmask(SIG_BLOCK, &set, 0);

   ReprogramData * rpd = (ReprogramData *) adata;

   rv = rpd->ctrl->__ReprogramJoe(rpd->program_set, rpd->program, rpd->gain, rpd->rep, rpd->force);

   return (void *) (&rv);
   
}//void * ReprogramJoeThreadWorker(void * adata)


int CCD47Ctrl::ReprogramJoe(int program_set, int program, int gain, int rep, int force)
{

   pthread_mutex_lock(&reprogMutex);
   
   rpd.ctrl = this;
   rpd.program_set = program_set;
   rpd.program = program;
   rpd.gain = gain;
   rpd.rep = rep;
   rpd.force = force;

   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);

   joe_loading = 1;
   
   pthread_create(&load_thread, &attr, &ReprogramJoeThreadWorker, (void *) &rpd);

   return 0;

}//int CCD47Ctrl::ReprogramJoe(int program_set, int program, int gain, int rep, int force)
   
int CCD47Ctrl::__ReprogramJoe(int program_set, int program, int gain, int rep, int force)
{
   int stat;

   int AstronomerWantsItRunning;

   if(cur_State == STATE_OPERATING) AstronomerWantsItRunning = 1;
   else AstronomerWantsItRunning = 0;

   // Stop ccd integration
   stat = Stop();
   cur_State = STATE_CONFIGURING;

   if (stat != NO_ERROR)
   {
      pthread_mutex_unlock(&reprogMutex);
      return stat;
   }
   
   _logger->log( Logger::LOG_LEV_DEBUG, "Checking program set: %d  program %d", program_set, program);
                                
   // If not in current memory
   if (program_set != cur_ProgramSet || force)
   {
      // If not in disk files
      if ((unsigned) program_set >= ondisk.size())
      {
         _logger->log( Logger::LOG_LEV_WARNING, "Program set %d is not valid", program_set);
         pthread_mutex_unlock(&reprogMutex);
         return NO_ERROR;
      }

      // If in disk files, upload the new program set
      _logger->log( Logger::LOG_LEV_INFO, "Program selection requires upload of program set %d", program_set);

      _logger->log( Logger::LOG_LEV_INFO, "Starting control file upload (%s)", getCompletePath(ondisk[program_set].control_filename).c_str());
      
      if ((stat = SendXmodemFile( "XMC 1", (char *) getCompletePath( ondisk[program_set].control_filename).c_str())) != NO_ERROR)
      {
         pthread_mutex_unlock(&reprogMutex);
         _logger->log( Logger::LOG_LEV_ERROR, "Xmodem upload returned error");
         std::cerr << "Xmodem upload returned error at " << __LINE__ << "\n";
         return stat;
      }
                
      _logger->log( Logger::LOG_LEV_INFO, "Starting pattern file upload (%s)", getCompletePath(ondisk[program_set].pattern_filename).c_str());
                
      if ((stat = SendXmodemFile( "XMP 1", (char *) getCompletePath( ondisk[program_set].pattern_filename).c_str())) != NO_ERROR)
      {
         pthread_mutex_unlock(&reprogMutex);
         _logger->log( Logger::LOG_LEV_ERROR, "Xmodem upload returned error");
         std::cerr << "Xmodem upload returned error at " << __LINE__ << "\n";
         return stat;
      }
                
      cur_ProgramSet = program_set;
      
      _logger->log( Logger::LOG_LEV_INFO, "Upload complete");

   }
                
   if(program < 0 || program > 7 || gain < 0 || gain > 3)
   {
      _logger->log( Logger::LOG_LEV_ERROR, "Program %d, gain %d is not valid", program, gain);
      pthread_mutex_unlock(&reprogMutex);
      return NO_ERROR;
   }
   
   #ifdef _debug
      std::cout << "Setting program " << program + gain*8 << std::endl;
   #endif
      
   if ((stat = SetJoeRCL(program + gain*8)) != NO_ERROR)
   {
      _logger->log( Logger::LOG_LEV_ERROR, "Error in SetJoeRCL(): %d", stat);
      pthread_mutex_unlock(&reprogMutex);
      return stat;
   }
   
   //rep = 0;/*
   char buffer[25];
   if(rep < 0) rep = 0;
   if(rep > 65535) rep = 65535;
   
   #ifdef _debug
      std::cout << "Setting repetitions" << std::endl;
   #endif

   sprintf( buffer, "@REP %d", rep);
   if ((SendJoeCommand( buffer, NULL, 25, 0)) != NO_ERROR)
   {
      std::cerr << "Error setting repetitions\n";
   }
   /*if((stat == SetJoeRepetitions(rep)) != NO_ERROR)
   {
      _logger->log( Logger::LOG_LEV_ERROR, "Error in SetJoeRepititions(): %d", stat);
      return stat;
   }*/

   cur_Program = program;
   cur_Gain = gain;
   
   cur_speed = ondisk[cur_ProgramSet].programs[cur_Program].readout_speed;

   cur_xbin = ondisk[cur_ProgramSet].programs[cur_Program].binx;
   cur_ybin = ondisk[cur_ProgramSet].programs[cur_Program].biny;
   cur_windowx = ondisk[cur_ProgramSet].programs[cur_Program].windowx;
   cur_windowy = ondisk[cur_ProgramSet].programs[cur_Program].windowy;
   cur_Reps = rep;

   #ifdef _debug
      std::cout << "Loading EDT conf at " << __LINE__ <<  std::endl;
   #endif

   //Stop() seems to be required here to get everything synched up.  Not sure why . . .
   #ifdef _debug
      std::cout << "Stopping at " << __LINE__ <<  std::endl;
   #endif
   Stop();

//Now load the new config file
   load_EDT_config(ondisk[cur_ProgramSet].programs[cur_Program].EDT_cfg_fname);
   
   // Update shown framerate
   ComputeFramerate();


   if(AstronomerWantsItRunning) 
   {
      
      //For whatever reason have to sleep for a bit before restarting.
      //double st = get_curr_time();
      /*while(get_curr_time() - st < 1.)
      {
         sleep(1);
      }*/

      #ifdef _debug
         std::cout << "Starting at " << __LINE__ <<  std::endl;
      #endif
      Start();
      /*st = get_curr_time();
      while(get_curr_time() - st < 1.)
      {
         sleep(1);
      }*/
   }
   else cur_State = STATE_READY;
 
   pthread_mutex_unlock(&reprogMutex);
   #ifdef _debug
   std::cout << "Leaving _ReprogramJoe()\n";
   #endif
   return 0;   
   
}//int CCD47Ctrl::__ReprogramJoe(int program_set, int program, int gain, int rep, int force)

int CCD47Ctrl::swonly_reprogram(int program_set, int program, int gain, int rep)
{
   
   #ifdef _debug
      std::cout << "s/w only reprogram" << std::endl;
   #endif
      
   cur_ProgramSet = program_set;
   cur_Program = program;
   cur_Gain = gain;
   
   cur_speed = ondisk[cur_ProgramSet].programs[cur_Program].readout_speed;

   cur_xbin = ondisk[cur_ProgramSet].programs[cur_Program].binx;
   cur_ybin = ondisk[cur_ProgramSet].programs[cur_Program].biny;
   cur_windowx = ondisk[cur_ProgramSet].programs[cur_Program].windowx;
   cur_windowy = ondisk[cur_ProgramSet].programs[cur_Program].windowy;
   cur_Reps = rep;
   
   // Update shown framerate
   ComputeFramerate();

   return 0;   
   
}//int CCD47Ctrl::swonly_reprogram(int program_set, int program, int gain, int rep)

void * __SetupNetwork(void * ctrl)
{
   //First block the async I/O signals
   sigset_t set;
   sigemptyset(&set);
   sigaddset(&set, SIGIO);
   sigaddset(&set, RTSIGIO);
   
   pthread_sigmask(SIG_BLOCK, &set, 0);
   
   int * result = new int;
   
   CCD47Ctrl * ctrl47 = (CCD47Ctrl *) ctrl;

   *result = InitJoeLink( (char *)ctrl47->get_ccdNetAddr().c_str(), ctrl47->get_ccdNetPort());

   return (void *) result;
}//void * __SetupNetwork(void * ctrl)
   
int CCD47Ctrl::SetupNetwork()
{
   
   int result, stat;
   
   // Close the previous connection, if any
   SerialClose();
   msleep(1000);

   _logger->log( Logger::LOG_LEV_INFO, "Connecting to %s:%d", (char *)_ccdNetAddr.c_str(), _ccdNetPort);

   // Setup serial/network interface
   if (( result = InitJoeLink( (char *)_ccdNetAddr.c_str(), _ccdNetPort)) != NO_ERROR)
   {
      _logger->log( Logger::LOG_LEV_ERROR, "Error configuring network: (%d) %s", result, lao_strerror(result));
      return NETWORK_ERROR;
   }
   _logger->log( Logger::LOG_LEV_INFO, "Network reconfigured OK");    

  
   // Now check our friend Joe
   stat = PLAIN_ERROR(CheckJoe());
   if (stat)
   {
      _logger->log( Logger::LOG_LEV_ERROR, "Error during LittleJoe testing: (%d) %s", stat, (char *)lao_strerror( stat));
      return stat;
   }

   return NO_ERROR;
}//int CCD47Ctrl::SetupNetwork()

int CCD47Ctrl::ReadJoeStatus()
{
   int stat;

   int state;
   state = cur_State;

   if (state == STATE_NOCONNECTION) 
   {
      return NO_ERROR;
   }

   #ifdef _debug
      std::cout << "Getting joe status" << std::endl;
   #endif
   stat = GetJoeStatus();
   #ifdef _debug
      std::cout << "Done getting joe status" << std::endl;
   #endif
      
   if(stat < 0)
   {
      return stat;
   }

   blackLevel[0] = GetBlackLevel(0);
   blackLevel[1] = GetBlackLevel(1);

   GetFramegrabberStatus();
         
   if(stat == 0 && cur_State == STATE_OPERATING)
   {
    //We think we're running, but we're not, so restart.
      #ifdef _debug
         std::cout << "Not running, restarting.\n";
      #endif
      //load_EDT_config(ondisk[cur_ProgramSet].programs[cur_Program].EDT_cfg_fname);
      //sleep(1);
      //return Start();
   }
   
   if(stat == 1 && cur_State == STATE_READY)
   {
      //We think we've stopped, but we haven't, so restop.
      #ifdef _debug
         std::cout << "Running, restopping.\n";
      #endif
      return Stop();
   }
   
   GetFramegrabberStatus();

   //If we're just starting up, don't change state but do setup the framegrabber
   if(stat == 0 && cur_State == STATE_CONNECTED)
   {
      #ifdef _debug
         std::cout << "STATE_CONNECTED, Stopping FG.\n";
      #endif
      cur_State = STATE_READY;
      if(FGrunning) return Stop(); //Frame grabber was running.
      return 0;
   }

   if(stat == 1 && cur_State == STATE_CONNECTED)
   {
      cur_State = STATE_OPERATING;

      if(!FGrunning)
      {
         #ifdef _debug
            std::cout << "STATE_CONNECTED, Starting FG.\n";
         #endif
         //Now load the new config file
         load_EDT_config(ondisk[cur_ProgramSet].programs[cur_Program].EDT_cfg_fname);
         return Start();
      }
      return 0;
   }

   
   return NO_ERROR;

}//int CCD47Ctrl::ReadJoeStatus()

int CCD47Ctrl::ReadJoeTemps()
{
   float t1, t2, t3;
 
   int stat;

   int state;
   state = cur_State;

   if (state == STATE_NOCONNECTION) 
   {
      temps[0] = -1;
      temps[1] = -1;
      temps[2] = -1;
      return NO_ERROR;
   }
   
   #ifdef _debug
      std::cout << "Getting temperatures" << std::endl;
   #endif
   stat = GetJoeTemperature( &t1, &t2, &t3);
   #ifdef _debug
      std::cout << "Done getting temperatures" << std::endl;
   #endif
      
   if (stat != NO_ERROR)
   {
      return stat;
   }
        
   temps[0] = (double)t1;
   temps[1] = (double)t2;
   temps[2] = (double)t3;

   _tempsLogger->log(Logger::LOG_LEV_INFO, "%06.2f %06.2f %06.2f", temps[0], temps[1], temps[2]);

   return NO_ERROR;

} //int CCD47Ctrl::ReadJoeTemps()

std::string CCD47Ctrl::getCompletePath( std::string filename)
{
   std::string path;

   path = Utils::getConffile(filename);

   int i = path.find(".conf");

   path.erase(i, path.length()-i);

   return path;
}//std::string CCD47Ctrl::getCompletePath( std::string filename)

int CCD47Ctrl::DoFSM(void)
{
   std::string resp;
   int status;
   int stat = NO_ERROR;

   status = cur_State; 

   #ifdef _debug
      std::cout << "DoFSM"  << std::endl;
   #endif
      
   // Always check if we can reach the LittleJoe
   // If not, reset everything and start again
   if (status != STATE_NOCONNECTION)
   {
      #ifdef _debug
         std::cout << "Testing Joe Link"  << std::endl;
      #endif

      if ((stat=TestJoeLink()) != NO_ERROR)
      {
         #ifdef _debug
            std::cerr << "TestJoeLink Failed " << stat  << std::endl;
         #endif
         if(cur_State == STATE_OPERATING)
         {   
            write_fifo_channel(FG_FIFO, "AUTO", 5, &resp);
            write_fifo_channel(FG_FIFO, "stop", 5, &resp);
         }
         cur_State = STATE_NOCONNECTION;
      }
   }
      status = cur_State;

      switch(status)
      {
         // At first, try to start network
         case STATE_NOCONNECTION:
            #ifdef _debug
               std::cout << "Setting up network"  << std::endl;
            #endif
            stat = SetupNetwork();
            
            if (stat == NO_ERROR) cur_State = STATE_CONNECTED;

            #ifdef _debug
               std::cout << "Network is setup"  << std::endl;
               if(cur_State == STATE_CONNECTED) std::cout << "STATE_CONNECTED" << std::endl;
               else std::cout << "STATE_NOCONNECTION" << std::endl;
            #endif

            break;

         // After network start, discover configuration of LittleJoe
         
         case STATE_CONNECTED:
            #ifdef _debug
               //std::cerr << "First configing" << std::endl;
            #endif
            //stat = FirstJoeConfig(); //First try to figure out what state we're in.
            
            #ifdef _debug
               std::cout << "reading joe status" << std::endl;
            #endif
            stat = ReadJoeStatus(); //Find out what we're doing.
            break;

         case STATE_CONFIGURING:
            break;
         // When ready to go, do nothing
         case STATE_READY:
         case STATE_OPERATING:
            #ifdef _debug
               std::cout << "reading joe status" << std::endl;
            #endif

            ReadJoeStatus();

            #ifdef _debug
               std::cout << "reading joe temps" << std::endl;
            #endif   
            stat = ReadJoeTemps();
            
            break;

         // Unknown states should not exist
         default:
            break;
      }
   
      #ifdef _debug
         std::cout << "done with DoFSM" << std::endl;
      #endif
      // Return the generated error code, if any
      return stat;
}//int CCD47Ctrl::DoFSM(void)


int CCD47Ctrl::Run()
{
   double t0 = 0, dt;
   
   signal(SIGIO, SIG_IGN);

   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }
   
   //Startup the I/O signal handling thread
   if(start_signal_catcher(true) != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }
   
   //Now Block all I/O signals in this thread.
   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error blocking SIGIO in main thread.");
      return -1;
   }
   
   LOG_INFO("starting up . . .");
   
   joe_loading = 0;
   
   while(!TimeToDie)
   {
      
      if(joe_loading)
      {
         void * thrdretval;
         
         if(pthread_tryjoin_np(load_thread, &thrdretval) == 0)
         {
            joe_loading = 0;
            if(*((int *) thrdretval) != 0)
            {
               error_report(Logger::LOG_LEV_ERROR, "Program load thread returned error.");
            }
         }
        
      }

      if(!joe_loading)
      {
         if(DoFSM() != 0 && cur_State != STATE_NOCONNECTION) //don't report if we just aren't connected.
         {
            error_report(Logger::LOG_LEV_ERROR, "Error in DoFSM.");
         }
      }


      t0 = get_curr_time();
  
      while((dt = get_curr_time() - t0) < pause_time && !TimeToDie)
      {
         //check_fifo_list_RTpending();
         if(pause_time - dt >= 1) sleep((int)(pause_time - dt));
         else usleep((int)((pause_time-dt)*1e6 * .99));
        
      }
      //update_statusboard();
   }
   return 0;   
}//int CCD47Ctrl::Run()

int CCD47Ctrl::load_EDT_config(std::string fname)
{
#ifndef NOPDV
   std::string visao_root;
   std::string edt_dir;
   std::string cfgtmp, bittmp;
   
   //visao_root = getenv("VISAO_ROOT");
   edt_dir = getenv("EDTDIR");

   cfgtmp = Utils::getConffile(EDT_cfgdir);
   int idx = cfgtmp.find(".conf");
   if(idx > -1) cfgtmp.erase(idx, cfgtmp.length()-idx);

   cfgtmp = cfgtmp + fname; //visao_root + "/" + EDT_cfgdir + "/" + fname;
   bittmp = edt_dir + "/camera_config";

   #ifdef _debug
      std::cout << "Loading EDT Config\n" << cfgtmp << "\n" << bittmp << "\n";
   #endif

   return initcam(cfgtmp.c_str(), bittmp.c_str());
#endif
   
   return 0;
}//int CCD47Ctrl::load_EDT_config(std::string fname)
   
std::string CCD47Ctrl::remote_command(std::string com)
{
   return common_command(com, CMODE_REMOTE);
}

std::string CCD47Ctrl::local_command(std::string com)
{
   return common_command(com, CMODE_LOCAL);
}
            
std::string CCD47Ctrl::script_command(std::string com)
{
   return common_command(com, CMODE_SCRIPT);
}

std::string CCD47Ctrl::common_command(std::string com, int cmode)
{
   std::string resp;
   char rstr[50];
   int rv;

   if(com == "state?") 
   {
      int cstate = 0;
      if(cur_State == STATE_READY) cstate = 1;
      if(cur_State == STATE_OPERATING) cstate = 2;
      if(cur_State == STATE_CONFIGURING) cstate = 3;
      snprintf(rstr, 50, "%c %i\n", control_mode_response()[0], cstate);
      return rstr;
   }
   
   if(com == "set?") 
   {
      snprintf(rstr, 50, "%i\n", cur_ProgramSet);
      return rstr;
   }
   
   if(com == "prog?") 
   {
      snprintf(rstr, 50, "%i\n", cur_Program);
      return rstr;
   }
   
   if(com == "speed?") 
   {
      snprintf(rstr, 50, "%i\n", cur_speed);
      return rstr;
   }
   
   if(com == "gain?") 
   {
      snprintf(rstr, 50, "%i\n", cur_Gain);
      return rstr;
   }
   
   if(com == "xbin?") 
   {
      snprintf(rstr, 50, "%i\n", cur_xbin);
      return rstr;
   }
   
   if(com == "ybin?") 
   {
      snprintf(rstr, 50, "%i\n", cur_ybin);
      return rstr;
   }
   
   if(com == "windowx?") 
   {
      snprintf(rstr, 50, "%i\n", cur_windowx);
      return rstr;
   }
   
   if(com == "windowy?") 
   {
      snprintf(rstr, 50, "%i\n", cur_windowy);
      return rstr;
   }
   
   if(com == "framerate?") 
   {
      snprintf(rstr, 50, "%f\n", cur_framerate);
      return rstr;
   }

   if(com == "rep?") 
   {
      snprintf(rstr, 50, "%i\n", cur_Reps);
      return rstr;
   }

   if(com == "temps?")
   {
      snprintf(rstr, 50, "%06.2f %06.2f %06.2f\n", temps[0], temps[1], temps[2]);
      return rstr;
   }

   if(com == "blacks?")
   {
      snprintf(rstr, 50, "%04i %04i\n", blackLevel[0], blackLevel[1]);
      return rstr;
   }

   if(com == "start")
   {
      if(control_mode == cmode)
      {
         rv = Start();
         snprintf(rstr, 50, "%i\n", rv);
         return rstr;
      }
      else return control_mode_response();
   }

   if(com == "stop")
   {
      if(control_mode == cmode)
      {
         rv = Stop();
         snprintf(rstr, 50, "%i\n", rv);
         return rstr;
      }
      else return control_mode_response();
   }

   if(com == "save?")
   {
      if(FGsaving) return "1\n";
      else return "0\n";
   }

   if(com == "remaining?")
   {
      snprintf(rstr, 50, "%i\n", FGremaining);
      return rstr;
   }

   if(com == "skip?")
   {
      snprintf(rstr, 50, "%i\n", FGskipping);
      return rstr;
   }

   if(com == "imtype?")
   {
      snprintf(rstr, 50, "%i\n", imtype);
      return rstr;
   }
   
   if(com == "subdir?")
   {
      return FWsubdir;
   }
   
   if(com.substr(0,4) == "skip")
   {
      if(cmode == control_mode)
      {
         int nsk = atoi(com.substr(5, com.length()-5).c_str());
         if(nsk < 0) nsk = 0;
         snprintf(rstr, 50, "skip  %i", nsk);
         write_fifo_channel(FG_FIFO, "AUTO", 5, &resp);
         write_fifo_channel(FG_FIFO, rstr, strlen(rstr)+1, &resp);
         FGskipping = nsk;
         return resp;
      }
      else return control_mode_response();   
   }

   if(com.substr(0,8) == "savedark")
   {
      if(cmode == control_mode)
      {
         int nsav = atoi(com.substr(9, com.length()-9).c_str());

         if(!fw47sb)
         {
            size_t sz;
            fw47sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  7100, 0);
            _logger->log( Logger::LOG_LEV_ERROR, "framewriter47 is not running.  no images saved.");
            if(!fw47sb) return "-2\n";
         }

         if(get_curr_time() - ts_to_curr_time(&fw47sb->last_update) > 3.*fw47sb->max_update_interval)
         {
            _logger->log( Logger::LOG_LEV_ERROR, "framewriter47 appears to not be running.  no images saved.");
            return "-2\n";
         }
         if(nsav != 0)
         {
            FGremaining = nsav;
            set_imtype(2);
            snprintf(rstr, 50, "savedark  %i", nsav);
            write_fifo_channel(FG_FIFO, "AUTO", 5, &resp);
            write_fifo_channel(FG_FIFO, rstr, strlen(rstr)+1, &resp);
            FGsaving = 1;
            return resp;
         }
         
         if(nsav == 0)
         {
            write_fifo_channel(FG_FIFO, "AUTO", 5, &resp);
            write_fifo_channel(FG_FIFO, "savedark 0", 11, &resp);
            FGsaving = 0;
            return resp;
         }
         
         return "0\n";
      }
      else return control_mode_response();   
   }
   
   if(com.substr(0,4) == "save")
   {
      if(cmode == control_mode)
      {
         int nsav = atoi(com.substr(5, com.length()-5).c_str());

         if(!fw47sb)
         {
            size_t sz;
            fw47sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  7100, 0);
            _logger->log( Logger::LOG_LEV_ERROR, "framewriter47 is not running.  no images saved.");
            if(!fw47sb) return "-2\n";
         }

         if(get_curr_time() - ts_to_curr_time(&fw47sb->last_update) > 3.*fw47sb->max_update_interval)
         {
            _logger->log( Logger::LOG_LEV_ERROR, "framewriter47 appears to not be running.  no images saved.");
            return "-2\n";
         }
         if(nsav != 0)
         {
            FGremaining = nsav;
            snprintf(rstr, 50, "save  %i", nsav);
            write_fifo_channel(FG_FIFO, "AUTO", 5, &resp);
            write_fifo_channel(FG_FIFO, rstr, strlen(rstr)+1, &resp);
            FGsaving = 1;
            return resp;
         }
         
         if(nsav == 0)
         {
            write_fifo_channel(FG_FIFO, "AUTO", 5, &resp);
            write_fifo_channel(FG_FIFO, "save 0", 7, &resp);
            FGsaving = 0;
            return resp;
         }
         
         return "0\n";
      }
      else return control_mode_response();   
   }

   if(com.substr(0,5) == "serve")
   {
      if(cmode == control_mode)
      {
         int nsrv = atoi(com.substr(6, com.length()-6).c_str());

//          if(!fw47sb)
//          {
//             size_t sz;
//             fw47sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  7100, 0);
//             _logger->log( Logger::LOG_LEV_ERROR, "framewriter47 is not running.  no images saved.");
//             if(!fw47sb) return "-2\n";
//          }

//          if(get_curr_time() - ts_to_curr_time(&fw47sb->last_update) > 3.*fw47sb->max_update_interval)
//          {
//             _logger->log( Logger::LOG_LEV_ERROR, "framewriter47 appears to not be running.  no images saved.");
//             return "-2\n";
//          }
         if(nsrv != 0)
         {
            snprintf(rstr, 50, "serve  %i", nsrv);
            write_fifo_channel(FG_FIFO, "AUTO", 5, &resp);
            write_fifo_channel(FG_FIFO, rstr, strlen(rstr)+1, &resp);
            FGsaving = 1;
            return resp;
         }
         
         if(nsrv == 0)
         {
            write_fifo_channel(FG_FIFO, "AUTO", 5, &resp);
            write_fifo_channel(FG_FIFO, "serve 0", 7, &resp);
            FGsaving = 0;
            return resp;
         }
         
         return "0\n";
      }
      else return control_mode_response();   
   }
   
   if(com.substr(0,3) == "set")
   {
      if(control_mode == cmode)
      {
         if(com.length() < 11)
         {
            return "not enough arguments in set";
         }
         
         int ns, np, ng, nrep;
         
         ns = atoi(com.substr(4,1).c_str());
         np = atoi(com.substr(6,1).c_str());
         ng = atoi(com.substr(8,1).c_str());
         nrep = atoi(com.substr(10, com.length() - 10).c_str());

        #ifdef _debug
         std::cout << "Reprogram: " << ns << " " << np << " " << ng << " " << nrep << "\n";
        #endif
         ReprogramJoe(ns, np, ng, nrep, 0);
         
         return "1\n";
      }
      else return control_mode_response();
   }

   if(com.substr(0,4) == "reps")
   {
      if(control_mode == cmode)
      {
         if(com.length() < 5)
         {
            return "not enough arguments in reps";
         }
         
         int ns, np, ng, nrep;
         
         ns = cur_ProgramSet;
         np = cur_Program;
         ng = cur_Gain;
         nrep = atoi(com.substr(4, com.length() - 4).c_str());

         #ifdef _debug
         std::cout << "Reprogram: " << ns << " " << np << " " << ng << " " << nrep << "\n";
         #endif
         ReprogramJoe(ns, np, ng, nrep, 0);
         
         return "1\n";
      }
      else return control_mode_response();
   }
   
   
   if(com.substr(0,9) == "framerate")
   {
      if(control_mode == cmode)
      {
         if(com.length() < 10)
         {
            return "not enough arguments in framerate";
         }
         
         int ns, np, ng, nrep;
         
         ns = cur_ProgramSet;
         np = cur_Program;
         ng = cur_Gain;
         nrep = ComputeRepsFrameRate(strtod(com.substr(9, com.length() - 9).c_str(),0));

         #ifdef _debug
         std::cout << "Reprogram: " << ns << " " << np << " " << ng << " " << nrep << "\n";
         #endif
         ReprogramJoe(ns, np, ng, nrep, 0);
         
         return "1\n";
      }
      else return control_mode_response();
   }
   
   if(com.substr(0,7) == "exptime")
   {
      if(control_mode == cmode)
      {
         if(com.length() < 8)
         {
            return "not enough arguments in exptime";
         }
         
         int ns, np, ng, nrep;
         
         ns = cur_ProgramSet;
         np = cur_Program;
         ng = cur_Gain;
         nrep = ComputeRepsExpTime(strtod(com.substr(8, com.length() - 8).c_str(),0));

         #ifdef _debug
         std::cout << "Reprogram: " << ns << " " << np << " " << ng << " " << nrep << "\n";
         #endif
         ReprogramJoe(ns, np, ng, nrep, 0);
         
         return "1\n";
      }
      else return control_mode_response();
   }
   
   if(com.substr(0,4) == "fset")
   {
      if(control_mode == cmode)
      {
         if(com.length() < 12)
         {
            return "not enough arguments in fset";
         }
         
         int ns, np, ng, nrep;
         
         ns = atoi(com.substr(5,1).c_str());
         np = atoi(com.substr(7,1).c_str());
         ng = atoi(com.substr(9,1).c_str());
         nrep = atoi(com.substr(11, com.length() - 10).c_str());
         #ifdef _debug
         std::cout << "Force Reprogram: " << ns << " " << np << " " << ng << " " << nrep << "\n";
         #endif
         ReprogramJoe(ns, np, ng, nrep, 1);
         
         return "1\n";
      }
      else return control_mode_response();
   }

   if(com.substr(0,5) == "swset")
   {
      if(control_mode == cmode)
      {
         if(com.length() < 13)
         {
            return "not enough arguments in swset";
         }
         
         int ns, np, ng, nrep;
         
         ns = atoi(com.substr(6,1).c_str());
         np = atoi(com.substr(8,1).c_str());
         ng = atoi(com.substr(10,1).c_str());
         nrep = atoi(com.substr(12, com.length() - 10).c_str());
         #ifdef _debug
         std::cout << "SW only Reprogram: " << ns << " " << np << " " << ng << " " << nrep << "\n";
         #endif 
         swonly_reprogram(ns, np, ng, nrep);
         
         return "1\n";
      }
      else return control_mode_response();
   }

   if(com.substr(0,5) == "black")
   {
      if(control_mode == cmode)
      {
         if(com.length() < 9)
         {
            return "not enough arguments to set black\n";
         }
         
         int bchan, blev;
         
         bchan = atoi(com.substr(6,1).c_str());

         if(bchan <0 || bchan > 1) return "-1\n";

         blev  = atoi(com.substr(8,4).c_str());

         if(blev < 0) blev = 0;
         if(blev > 1023) blev = 1023;

         if(pthread_mutex_trylock(&reprogMutex) == 0)
         {
            #ifdef _debug
            std::cout << "Setting black: " << bchan << " " << blev << "\n";
            #endif
            if(blev != blackLevel[bchan]) SetJoeBlack(bchan, blev);
            pthread_mutex_unlock(&reprogMutex);
            return "0\n";
         }
         return "-1\n";
      }
      else return control_mode_response();
   }

   if(com.substr(0,6) == "subdir")
   {
      if(control_mode == cmode)
      {
         write_fifo_channel(FW_FIFO, "AUTO", 4, &resp);
         write_fifo_channel(FW_FIFO, com.c_str(), com.length(), &resp);
         return "0\n";
      }
      else return control_mode_response();
   }

   if(com.substr(0,4) == "dark")
   {
      if(control_mode == cmode)
      {
         write_fifo_channel(FG_FIFO, "AUTO", 4, &resp);
         write_fifo_channel(FG_FIFO, com.c_str(), com.length(), &resp);
         return "0\n";
      }
      else return control_mode_response();
   }
   
   if(com.substr(0,6) == "imtype")
   {
      if(com.length() >= 7)
      {
         if(control_mode == cmode)
         {
            int nsav = atoi(com.substr(7, com.length()-7).c_str());
            set_imtype(nsav);
            return "0\n";
         }
         else return control_mode_response();
      }
   }
   
   resp = "Unkown Command: " + com ;
   return  resp;
}//std::string CCD47Ctrl::common_command(std::string com, int cmode)


int CCD47Ctrl::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      VisAO::ccd47_status_board * csb = (VisAO::ccd47_status_board *) statusboard_shmemptr;
   
      csb->status = cur_State;
      csb->speed = cur_speed;
      csb->xbin = cur_xbin;
      csb->ybin = cur_ybin;
      csb->windowx = cur_windowx;
      csb->windowy = cur_windowy;
      csb->repetitions = cur_Reps;
      csb->framerate = cur_framerate;
      csb->gain = cur_Gain;

      csb->joe_temp = temps[0];
      csb->head_temp1 = temps[1];
      csb->head_temp2 = temps[2];

      csb->black0 = blackLevel[0];
      csb->black1 = blackLevel[1];

      csb->saving = FGsaving;
      csb->skipping = FGskipping;
      csb->remaining = FGremaining;
      
      csb->imtype = imtype;

   }
   return 0;
}//int CCD47Ctrl::update_statusboard()

int SetJoeRCL( int rcl)
{
   char buffer[32];
   int cur_rcl, error;

   if(rcl < 0 || rcl > 31)
   {
      return -1;
   }
   
   error = SendJoeCommand("@RCL?", buffer, 31, 1);
   if (error != NO_ERROR)
   {
      return error;
   }
   cur_rcl = atoi(buffer+6);

   // If the selected program is already active, stop here
   if (cur_rcl == rcl)
   {
      return NO_ERROR;
   }
   
   sprintf( buffer, "@RCL %1d", rcl);
   if ((error = SendJoeCommand( buffer, NULL, 0, 0)) != NO_ERROR)
   {
      return error;
   }
   
   return error;
}


                    
} //namespace VisAO
