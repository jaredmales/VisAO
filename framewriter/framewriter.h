/************************************************************
*    framewriter.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations for a class for writing image frames from shared memory to disk.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framewriter.h
  * \author Jared R. Males
  * \brief Declarations for a class for writing image frames from shared memory to disk.
  * 
  *
*/

#ifndef __framewriter_h__
#define __framewriter_h__

#include "VisAOApp_standalone.h"
#include "visaoimutils.h"

#include "statusboard.h"

namespace VisAO
{
   
///A class for writing image frames from shared memory to disk.
/** Begins writing to disk when a frame ready notification is received via the ping fifo.
 *
 * In addition to the \ref VisAOApp_standalone config file options, has:
 *
 * Required:
 *  - <b>name_base</b> <tt>string</tt>  - the prefix of the data files, e.g. V47_ for ccd47 data
 *  - <b>shmem_key</b> <tt>int</tt>  - the share memory key to write from.
 * 
 * Optional:
 *  - <b>ping_fifo_path</b> <tt>string</tt> [fifos] - the directory containing the ping fifo, relative
 *     to VISAO_ROOT
 *  - <b>save_path</b> <tt>string</tt> [data] -  the directory, relative to VISAO_ROOT, where to write the
 *     data
 *  - <b>write_aosys_head</b> <tt>int</tt> [1] - whether or not to write AO system header data
 *  - <b>write_visao_head</b> <tt>int</tt> [1] - whether or not to write VisAO camera header data
 *  - <b>origin</b> <tt>string</tt> [Magellan AO System]- the value of the FITS ORIGIN keyword for this data
 *  - <b>telescope</b> <tt>string</tt> [Magellan Clay]- the value of the FITS TELESCOP keyword for this data
 *  - <b>instrument</b> <tt>string</tt> [Magellan VisAO] - the value of the FITS INSTRUME keyword for this data
 *
 * See \ref VisAOApp_standalone for command line arguments.  There are no additional command line arguments for
 * a basic framegrabber, though derived clases may add them.
 *
 * Commands:
 *   - <b>"subdir s"</b> changes the directory where data is saved to s, which is relative to the data directory
 *     specified in the configuration by save_path.  "subdir ." resets to save_path.  The subdirectory is
 *     created if it doesn't exist.
 *   - <b>"subdir?"</b>  returns the current subdirectory
 */
template <class dataT> class framewriter : public VisAOApp_standalone
{
public:
   framewriter(int argc, char **argv) throw (AOException);
   framewriter(std::string name, const std::string &conffile) throw (AOException);
   
   int Create();
   
protected:
   std::string ping_fifo_path;
   
   std::string save_path;
   
   std::string name_base;
   
   std::string path_name;

   std::string subdir;

   int change_subdir(std::string sd);
   //unsigned fileno;
   //unsigned ndigits;
   
   sharedim_stack<dataT> * sis; ///< Manages a VisAO shared memory image stack.
   
   int shmem_key;
   
   bool attached_to_shmem;
   
   sharedim<dataT> sim; ///<The sharedim structure retreived from the stack
   
   int behind; ///<The number of frames currently behind
   int total_skipped; ///<The total number of frames skipped
   
   visao_imheader imhead;

   bool write_aosys_head;
   bool write_visao_head;
   
   pthread_mutex_t frame_ready_mutex;
public:
   pthread_cond_t frame_ready_cond;

public:
   int set_ping_fifo_path(std::string &);
   std::string get_ping_fifo_path(){return ping_fifo_path;}
   
   int set_save_path(std::string &);
   std::string get_save_path(){return save_path;}
   
   int set_name_base(std::string &);
   std::string get_name_base(){return name_base;}
   
   int set_sharedim_stack(sharedim_stack<dataT> *);
   sharedim_stack<dataT> * get_sharedim_stack(){return sis;}
   
   int set_shmem_key(int sk);
   int get_shmem_key(){return shmem_key;}
   int connect_shmem();
   
   int set_sim(sharedim<dataT> s);
   sharedim<dataT> get_sim(){return sim;}
   
   int get_total_skipped(){return total_skipped;}
   
   virtual int Run();
   
   int write_frame();
   virtual int kill_me();

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
    * - subdir?  the return value is the current subdirectory"
    * - subdir xxxxx  sets the subdirectory to xxxxx
    * - For any other inputs returns "UNKNOWN COMMAND: (str)\n"
    */
   std::string common_command(std::string com, int cmode);
};

//read one of the dio channel fifos
template <class dataT> int frame_ready(fifo_channel *fc);


template <class dataT> framewriter<dataT>::framewriter(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   Create();
}

template <class dataT> framewriter<dataT>::framewriter(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   Create();
}

template <class dataT> int framewriter<dataT>::Create()
{
   std::string pathtmp;
   
   std::string visao_root = getenv("VISAO_ROOT");
   
   //Set up the ping_fifo_path
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["ping_fifo_path"];
   }
   catch(Config_File_Exception)
   {
      pathtmp = "fifos";
   }
   ping_fifo_path = visao_root + "/" + pathtmp +"/";
   _logger->log(Logger::LOG_LEV_INFO, "Set ping_fifo_path: %s", ping_fifo_path.c_str());
   
   //Set up the save_path
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["save_path"];
   }
   catch(Config_File_Exception)
   {
      pathtmp = "data";
   }
   save_path = visao_root + "/" + pathtmp + "/";
   set_save_path(save_path); //takes care of other business
   
   _logger->log(Logger::LOG_LEV_INFO, "Set save_path: %s", save_path.c_str());
   
   //Set up the name_base
   try
   {
      pathtmp = (std::string)(ConfigDictionary())["name_base"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "name_base is a required config parameter.");
      throw;
   }
   
   name_base = pathtmp;
   _logger->log(Logger::LOG_LEV_INFO, "Set name_base: %s", name_base.c_str());
   
   path_name = save_path+name_base;
   _logger->log(Logger::LOG_LEV_INFO, "Generated path_name: %s", path_name.c_str());
   
   setup_fifo_list(4);
   setup_baseApp(0, 1, 1, 1, false);
   //setup_baseApp();

   set_fifo_list_channel(&fl, 0, RWBUFF_SZ, (char *)std::string(ping_fifo_path + MyName()+"_ping_in").c_str(), (char *)std::string(ping_fifo_path + MyName()+"_ping_out").c_str(), &frame_ready<dataT>, (void *)this);
      
   //Set up the fits ORIGIN header
   try
   {
      imhead.origin = (std::string)(ConfigDictionary())["origin"];
   }
   catch(Config_File_Exception)
   {
      imhead.origin = "Magellan AO System";
   }
   
   //Set up the fits TELESCOP header
   try
   {
      imhead.telescop = (std::string)(ConfigDictionary())["telescope"];
   }
   catch(Config_File_Exception)
   {
      imhead.telescop = "Magellan Clay";
   }
   
   //Set up the fits INSTRUME header
   try
   {
      imhead.instrume = (std::string)(ConfigDictionary())["instrument"];
   }
   catch(Config_File_Exception)
   {
      imhead.instrume = "Magellan VisAO";
   }
   
   //Decide whether we write the ao system header
   try
   {
      write_aosys_head = (int)(ConfigDictionary())["write_aosys_head"];
   }
   catch(Config_File_Exception)
   {
      write_aosys_head = 1;
   }
   
   //Decide whether we write the visao header
   try
   {
      write_visao_head = (int)(ConfigDictionary())["write_visao_head"];
   }
   catch(Config_File_Exception)
   {
      write_visao_head = 1;
   }

   try
   {
      shmem_key = (int)(ConfigDictionary())["shmem_key"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "shmem_key is a required config parameter.");
      throw;
   }
   attached_to_shmem     = false;
   
   total_skipped = 0;
   behind = 0;

   init_visao_imheader(&imhead);
   
   //Init the status board for known process names
   statusboard_shmemkey = 0;
   if(MyFullName() == "framewriter47.L")
   {
      std::cout << "Initing for framewriter47.L\n";
      statusboard_shmemkey = STATUS_framewriter47;
   }
   if(MyFullName() == "framewriter39.L")
   {
      std::cout << "Initing for framewriter39.L\n";
      statusboard_shmemkey = STATUS_framewriter39;
   }

   if(statusboard_shmemkey)
   { 
      if(create_statusboard(sizeof(basic_status_board)) != 0)
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
   }

   pthread_cond_init(&frame_ready_cond, NULL);
   pthread_mutex_init(&frame_ready_mutex, NULL);

   return 0;
}

template <class dataT> int framewriter<dataT>::change_subdir(std::string sd)
{
   
   int idx = sd.find_first_of(" \t\r\n", 0);
   if(idx > -1) subdir = sd.substr(0, idx);
   else subdir = sd;
   
   path_name = save_path+ subdir+ "/" + name_base;
   _logger->log(Logger::LOG_LEV_INFO, "Generated path_name: %s", path_name.c_str());
   
   if(subdir != "." && subdir != "")
   {
      std::string com = "mkdir -p ";
      com += save_path+ subdir;
      
      system(com.c_str());
   }
   
   return 0;
}

template <class dataT> int framewriter<dataT>::set_ping_fifo_path(std::string &pfp)
{
   ping_fifo_path = pfp;
   return 0;
}

template <class dataT> int framewriter<dataT>::set_save_path(std::string &sp)
{
   save_path = sp;
   path_name = save_path+name_base;

   //Make the directory
   std::string com = "mkdir -p ";
   com += save_path;
      
   system(com.c_str());

   return 0;
}

template <class dataT> int framewriter<dataT>::set_name_base(std::string &nb)
{
   name_base = nb;
   path_name = save_path+name_base;
   return 0;
}

template <class dataT> int framewriter<dataT>::set_sharedim_stack(sharedim_stack<dataT> * s)
{
   sis = s;
   return 0;
}

template <class dataT> int framewriter<dataT>::set_shmem_key(int sk)
{
   shmem_key = sk;
   return 0;
}

template <class dataT> int framewriter<dataT>::connect_shmem()
{
   sis = new sharedim_stack<dataT>;
   if(sis->attach_shm(shmem_key) != 0)
   {
      ERROR_REPORT("Error attaching to shared memory.");
      attached_to_shmem = false;
      delete sis;
      return -1;
   }
   attached_to_shmem = true;
   return 0;
}

template <class dataT> int framewriter<dataT>::set_sim(sharedim<dataT> s)
{
   sim = s;
   return 0;
}

template <class dataT> int framewriter<dataT>::Run()
{
   connect_shmem();
   
   //Install the main thread handler
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
   
   
   /*global_fifo_list = &fl;
    * 
    *   signal(SIGIO, SIG_IGN);
    *   if(connect_fifo_list() == 0)
    *   {
    *      LOG_INFO("fifo_list connected.");
    }
    else
    {
       ERROR_REPORT("Error connecting the fifo list.");
       return -1;
    }
    
    
    
    act.sa_handler = &catch_fifo_response_list;
    act.sa_flags = 0;
    sigemptyset(&sset);
    act.sa_mask = sset;
    
    sigaction(SIGIO, &act, 0);
    */
   
   attach_status_boards();
   LOG_INFO("starting up . . .");
   
   while(!TimeToDie)
   {
      //sleep(pause_time);
      //update_statusboard();
      
      pthread_mutex_lock(&frame_ready_mutex);
      pthread_cond_wait(&frame_ready_cond, &frame_ready_mutex);
      pthread_mutex_unlock(&frame_ready_mutex);
      write_frame();
      
   }
   
   return 0;
}

template <class dataT> int framewriter<dataT>::write_frame()
{
   static int last_image_abs = -1, last_image = -1, last_save_sequence = -1;
   int skipped;
   int fitsStatus;
   char fitsError[30];
   std::stringstream ss;
   //timeval tv0, tv1;
   //double dt;
   
   if(attached_to_shmem == false)
   {
      if(connect_shmem() != 0) return -1;
   }
   
   //Detect a framegrabber restart
   if(sis->get_last_image_abs() < last_image_abs || last_save_sequence != sis->header->save_sequence)
   {
      last_image_abs = sis->get_last_image_abs()-1; //Subtracting 1 here causes the current frame to be saved
                                                    //Not subtracting one causes us to wait 1 frame.
      last_image = sis->get_last_image();
      last_save_sequence = sis->header->save_sequence;
      attach_status_boards();
      //If we restarted, log it:
      if(sis->get_last_image_abs() < last_image_abs) LOG_INFO("Detected framegrabber restart, resetting");
      std::cout << "Detected framegrabber restart, resetting" << "\n";
   }
   
   if(last_image_abs < 0)
   {
      last_image = sis->get_last_image();
      last_save_sequence = sis->header->save_sequence;
      last_image_abs = sis->get_last_image_abs()-1;
   }
   
   while(last_image_abs < sis->get_last_image_abs() && !TimeToDie)
   {
      sim = sis->get_image(last_image);
      
      if(sim.nx)
      {
         //gettimeofday(&tv0, 0);
         
         fitsStatus = write_visao_fits<dataT>(path_name.c_str(), &sim, &imhead, write_aosys_head, write_visao_head);
         if(fitsStatus != 0)
         {
            ss.str("");
            fits_get_errstatus(fitsStatus, fitsError);
            ss << "write_visao_fits returned status: [" << fitsStatus << "] " << fitsError << " - Error writing image";
            ERROR_REPORT(ss.str().c_str());
            //ERROR_REPORT(fits_report_error(stderr, fitsStatus));
            return -1;
         }
         sis->set_saved(last_image, 1);
         
         //gettimeofday(&tv1,0);
         //dt = ((double)tv1.tv_sec + ((double)tv1.tv_usec)/1e6)-((double)tv0.tv_sec + ((double)tv0.tv_usec)/1e6);
         //std::cout  << dt << "\n";
         last_image_abs++;
         last_image++;
         if(last_image >= sis->get_max_n_images()) last_image = 0;
      }
      else
      {
         std::cout << "last_image " << last_image << "\n";
         std::cout << "last_image_abs " << last_image_abs << "\n";
         std::cout << "max_n_images " << sis->get_max_n_images() << "\n";
         ERROR_REPORT("Error getting image in write_frame().");
         exit(0);
      }
      behind = sis->get_last_image_abs() - last_image_abs;
      
      //if(behind) std::cout << "Behind by: " << sis->get_last_image_abs() - last_image_abs << "\n";
      
      
      if(behind >= sis->get_max_n_images())
      {
         skipped = (int)(behind - .5*sis->get_max_n_images());
         total_skipped += skipped;
         _logger->log(Logger::LOG_LEV_ERROR,"Behind %i frames, skipping %i frames.  Total skipped: %i", behind, skipped, total_skipped);
         
         last_image_abs += skipped;
         last_image += skipped;
         if(last_image >= sis->get_max_n_images()) last_image = last_image - sis->get_max_n_images();
         
      }
      update_statusboard();
      
   }
   
   
   
   return 0;
}

template <class dataT> int framewriter<dataT>::attach_status_boards()
{
   size_t sz;
   
   if(!imhead.fsb)
      imhead.fsb = (VisAO::focusstage_status_board*) attach_shm(&sz, STATUS_focusstage, 0);
   
   if(!imhead.csb)
      imhead.csb = (VisAO::ccd47_status_board*) attach_shm(&sz,  STATUS_ccd47, 0);
   
   if(!imhead.ssb)
      imhead.ssb = (VisAO::shutter_status_board*) attach_shm(&sz,  STATUS_shutterctrl, 0);
   
   if(!imhead.fw2sb)
      imhead.fw2sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel2, 0);
   
   if(!imhead.fw3sb)
      imhead.fw3sb = (VisAO::filterwheel_status_board*) attach_shm(&sz,  STATUS_filterwheel3, 0);
   
   if(!imhead.wsb)
      imhead.wsb = (VisAO::wollaston_status_board*) attach_shm(&sz,  STATUS_wollaston, 0);
   
   if(!imhead.aosb)
      imhead.aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);
       
   if(!imhead.gsb)
      imhead.gsb = (VisAO::gimbal_status_board*) attach_shm(&sz,  STATUS_gimbal, 0);

   if(!imhead.vssb)
      imhead.vssb = (VisAO::system_status_board*) attach_shm(&sz,  STATUS_sysmonD, 0);

   if(!imhead.rsb)
      imhead.rsb = (VisAO::reconstructor_status_board*) attach_shm(&sz,  STATUS_reconstructor, 0);
   
   if(!imhead.stsb)
      imhead.stsb = (VisAO::shutter_tester_status_board*) attach_shm(&sz, STATUS_shuttertester, 0);
   
   if(!imhead.zsb)
      imhead.zsb = (VisAO::zaber_status_board*) attach_shm(&sz, STATUS_zaber, 0);
   
   if(!imhead.hwpsb)
      imhead.hwpsb = (VisAO::hwp_status_board*) attach_shm(&sz, STATUS_hwp, 0);
   
   return 0;
}

template <class dataT> std::string  framewriter<dataT>::remote_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received remote command: %s.", com.c_str());
   resp = common_command(com, CMODE_REMOTE);
   if(resp == "") resp = (std::string("UNKOWN COMMAND: ") + com + "\n");
   _logger->log(Logger::LOG_LEV_TRACE, "Response to remote command: %s.", resp.c_str());
   return resp;
}

template <class dataT> std::string framewriter<dataT>::local_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received local command: %s.", com.c_str());
   resp = common_command(com, CMODE_LOCAL);
   if(resp == "") resp = (std::string("UNKOWN COMMAND: ") + com + "\n");
   _logger->log(Logger::LOG_LEV_TRACE, "Response to local command: %s.", resp.c_str());
   return resp;
}

template <class dataT> std::string  framewriter<dataT>::script_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received script command: %s.", com.c_str());
   resp = common_command(com, CMODE_SCRIPT);
   if(resp == "") resp = (std::string("UNKOWN COMMAND: ") + com + "\n");
   _logger->log(Logger::LOG_LEV_TRACE, "Response to script command: %s.", resp.c_str());
   return resp;
}

template <class dataT> std::string framewriter<dataT>::auto_command(std::string com, char *seqmsg)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received auto command: %s.", com.c_str());
   resp = common_command(com, CMODE_AUTO);
   seqmsg = 0; //just to avoid the warning
   if(resp == "") resp = post_auto_command(com);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to auto command: %s.", resp.c_str());
   return resp;
}

template <class dataT> std::string framewriter<dataT>::common_command(std::string com, int cmode)
{
   if(com == "subdir?")
   {
      return subdir + "\n";
   }
   
   if(com.substr(0,6) == "subdir")
   {
      if(change_subdir(com.substr(7, com.length()-7)) == 0) return "0\n";
      else return "-1\n";
   }
   
   return "";
}

template <class dataT> int framewriter<dataT>::kill_me()
{
   pthread_cond_broadcast(&frame_ready_cond);
   return 0;
}

template <class dataT> int frame_ready(fifo_channel *fc)
{
   framewriter<dataT> *fw;
   
   fw = (framewriter<dataT> *)fc->auxdata;
   
   if(!TimeToDie)
   {
      //fw->write_frame();
      pthread_cond_broadcast(&fw->frame_ready_cond);
      while(read_fifo_channel(fc) > 0); //We don't do anything with this, just clear it out.
   }
   return 0;
}




} //namespace VisAO

#endif //__framewriter_h__

