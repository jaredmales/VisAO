/************************************************************
*    framegrabber.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Declarations and definitions for a template class to manage a generic framegrabber.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber.h
  * \author Jared R. Males
  * \brief Declarations and definitions for a template class to manage a generic framegrabber.
  * 
  *
*/

#ifndef __framegrabber_h__
#define __framegrabber_h__

#include "VisAOApp_standalone.h"
#include "visaoimutils.h"
#include "../lib/sharedim_stack.h"

namespace VisAO
{
   
///The basic VisAO framegrabber.
/** This class provides a framework for a framegrabber which writes to a sharedim_stack.
  * You need to provide, at a minimum overloaded members start_framegrabber and stop_framegrabber.
  * Has local, script, and auto fifos.  Is intended to be normally controlled in auto mode by the camera controller.
  * When a frame is ready, it (optionally) issues a ping to a frame writer process and a frame server process.
  * These pings are frame-ready notifications.  The difference between the writer and the server is that the writer
  * also manages saving (number and skipping), whereas the server only receives pings.
  *
  * In addition to the \ref VisAOApp_standalone config file options, has:
  *  - <b>num_sharedim</b> <tt>int</tt> [200] - the number of shared images to build into the stack
  *  - <b>max_imsize</b> <tt>int</tt> [1024x1024] -  the maximum possible size of an image
  *  - <b>shmem_key</b> <tt>int</tt> [DATA_framegrabber47] - the shared memory key of the image data circular buffer
  *  - <b>dark_shmem_key</b> <tt>int</tt> [0] - the shared memory key of the dark image data circular buffer
  *  - <b>writer_ping_path</b> <tt>string</tt> - the path to the frame writer ping fifo, relative to VISAO_ROOT
  *  - <b>writer_ping_name</b> <tt>string</tt> - the name of the frame writer ping.  If not set then writer ping is
  *    disabled
  *  - <b>server_ping_path</b> <tt>string</tt>- the path to the frame server ping fifo, relative to VISAO_ROOT
  *  - <b>server_ping_name</b> <tt>string</tt>- the name of the frame server ping.  If not set then server ping is
  *    disabled
  *  - <b>init_save_interval</b> <tt>int</tt> [200] - how often, in frames, to save the frame number to the init
  *    file
  * 
  * See \ref VisAOApp_standalone for command line arguments.  There are no additional command line arguments for
  * a basic framegrabber, though derived clases may add them.
  * 
  * Commands:
  *   - <b>start</b> starts the framegrabber, returns 1
  *   - <b>stop</b>  stops the framegrabber, returns 0
  *   - <b>running?</b>  returns 1 if running, 0 if not
  *   - <b>save  n</b> enable pings for n images
  *   - <b>save -1</b>  enable pings indefinitely
  *   - <b>save  0</b>  disable pings for frame writer
  *   - <b>savedark  n</b>  saves n dark images, and also updates the realtime dark
  *   - <b>skip  n</b>  when pings enabled, skip n frames between pings
  *   - <b>save?</b>   status of pings for frame writer (returns value of save)
  *   - <b>skip?</b>  returns the number of images being skipped
  *   - <b>dark n</b> calculates a new realtime dark image (average) from the next n frames
  *   - <b>serve 1</b> enable pings for the frame server
  *   - <b>serve 0</b> disable pings for the frame server
  *   - <b>serve?</b>  status of pings for frame server
  *   - <b>state?</b> returns a string of the form "A,B,C,D,E,F" where the fields indicate:
  *      - A is the one letter control mode (N,R,L,S,or A)
  *      - B is the running state, 1 or 0
  *      - C is the status of saving, 0, n, or -1
  *      - D is the status of skipping, 0..n
  *      - E is the number of saves remaining
  *      - F is the status of the server ping
  *
  * 
  */
template <class dataT> class framegrabber : public VisAOApp_standalone
{
   public:
      framegrabber(int argc, char **argv) throw (AOException);
      framegrabber(std::string, const std::string &conffile) throw (AOException);

      ~framegrabber();
      
   protected:
      ///Processes the config file
      void Create();
                  
      u_char *image_p;  ///< Pointer to the current image data.
      
      int     width;  ///<Image width
      int     bias_width; ///<Width of bias stripe (if any)
      int     height; ///<Image height 
      int     bias_height; ///<Height of bias stripe (if any)
      int     depth;  ///<Image bit depth
     
      int frameNo; ///<For use as a running tally of images captured
    
      sharedim_stack<dataT> sis; ///< The shared memory ring buffer for image storage
      sharedim<dataT> * sim; ///< Pointer to a shared memory image
      
      int shmem_key; ///< The key for the shared memory 
      int dark_shmem_key; ///< The key for the darks shared memory 
      int num_sharedim; ///< The number of shared images available in the buffer
      size_t max_imsize; ///<The maximum size (x*y) of images that will be put in this buffer.
      
      int hasWriterPing;         ///<If true, open fifo for pinging the framewriter
      int writerPingEnabled;     ///<Send pings on frame ready when true
      int writerPingChan;        ///<The fifo channel for the writer pings
      int saves_remaining;  ///<Number pings left to issue
      int num_skip;         ///<The number of frames to skip between pings
      int skips_remaining;  ///<Skips remaining

      int hasServerPing;         ///<If true, open fifo for pinging the frame server
      int serverPingEnabled;     ///<Send pings on frame ready when true
      int serverPingChan;        ///<The fifo channel for the writer pings

      int STOP_FRAMEGRABBER;///<If true, framegrabber stops at top of next loop.
      int RUNNING; ///<Status of framegrabber 
      
      int sh_i; ///<Convenience variable for loops
      int sh_j; ///<Convenience variable for loops
      timeval tv; ///<Convenience variable for filling in the frame time in the image header
      timeval tv_dma; ///<Convenience variable for filling in the dma timestamp
      
      //Calculating darks
      sharedim_stack<dataT> dark_sis; ///< The shared memory ring buffer for image storage
      sharedim<dataT> * dark_sim; ///< Pointer to a shared memory image
      std::vector<dataT> dark_temp; ///<Temporary data storage for dark calculations
      
      int calc_dark;
      
      int addto_dark();
      
      int no_darks_added;
      

   public:
      virtual int Run(); ///<Main loop, normally won't need to be overridden

      virtual int start_framegrabber(); ///<Override this: it is where your framegrabber should do all its work.  Check for !STOP_FRAMEGRABBER and !TimeToDie
      virtual int stop_framegrabber(); ///<Override this: framegrabber clean up.

      int postGetImage(); ///< Call this immediately after getting each frame.  updates frameNo and saves the init file
      
      ///Sends a ping to the waiting process (usuallly a framewriter).
      /**Contains the logic for number of saves and skipping frames.
        *Should be called after one of the copyto functions completes.
        *\retval 0 on success
        *\retval -1 on failure
        */
      int send_ping();

      ///Copies the image data to shared memory, as shorts
      int copyto_sharedim_short(unsigned char * im); 

      ///Copies the image data to shared memory, as shorts, rotated by 90 degrees
      int copyto_sharedim_short_rotated(unsigned char * im);

      ///Copies the image to data to shared memory, as shorts, rotated by 90 degrees, with DALSA 2 channel de-interleaving
      int copyto_sharedim_short_deintlv_rotated(unsigned char * im); 
         
      std::string local_command(std::string com);
      std::string script_command(std::string com);
      std::string auto_command(std::string com, char *seqmsg);
      std::string common_command(std::string com, int cmode);
      
   protected:
      ///Save the initialization data to disk
      int save_init();
      
      int init_save_interval;
      
      ///Delete the initialization file
      int delete_init();
};


template <class dataT> framegrabber<dataT>::framegrabber(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   Create();
}

template <class dataT> framegrabber<dataT>::framegrabber(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   Create();
}

template <class dataT> framegrabber<dataT>::~framegrabber()
{
   save_init();
}

template <class dataT> void framegrabber<dataT>::Create()
{
   //Set the number of images in the shared image buffer
   try
   {
      num_sharedim = (int)(ConfigDictionary())["num_sharedim"];
   }
   catch(Config_File_Exception)
   {
      num_sharedim = 200;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set number of images in the shared image buffer (num_sharedim): %i", num_sharedim);
   
   try
   {
      max_imsize = (int)(ConfigDictionary())["max_imsize"];
   }
   catch(Config_File_Exception)
   {
      max_imsize = 1024*1024;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the maximum image size to (max_imsize): %i", max_imsize);
   
   try
   {
      shmem_key = (int)(ConfigDictionary())["shmem_key"];
   }
   catch(Config_File_Exception)
   {
      shmem_key = DATA_framegrabber47;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the shared memory key (shmem_key): %i", shmem_key);
   
   try
   {
      dark_shmem_key = (int)(ConfigDictionary())["dark_shmem_key"];
   }
   catch(Config_File_Exception)
   {
      dark_shmem_key = 0;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the shared memory key (dark_shmem_key): %i", shmem_key);
   
   
   std::string wPingPath, wPingName;
   //Set the writer ping fifo base path
   try
   {
      wPingPath = (std::string)(ConfigDictionary())["writer_ping_path"];
   }
   catch(Config_File_Exception)
   {
      wPingPath = "fifos";
   }
   
   try
   {
      wPingName = (std::string)(ConfigDictionary())["writer_ping_name"];
      hasWriterPing = 1;
   }
   catch(Config_File_Exception)
   {
      hasWriterPing = 0;
   }
   
   std::string sPingPath, sPingName;
   //Set the server ping fifo base path
   try
   {
      sPingPath = (std::string)(ConfigDictionary())["server_ping_path"];
      hasServerPing = 1;
   }
   catch(Config_File_Exception)
   {
      hasServerPing = 0;
   }

   try
   {
      sPingName = (std::string)(ConfigDictionary())["server_ping_name"];
      hasServerPing = 1;
   }
   catch(Config_File_Exception)
   {
      hasServerPing = 0;
   }
   
   //set up the fifos
   setup_fifo_list(3+hasWriterPing + hasServerPing);
   setup_baseApp(0, 1, 1, 1, false);
   
   std::string tmppath;
   std::string visao_root = getenv("VISAO_ROOT");

   int nPings = 0;
   if(hasWriterPing)
   {
      tmppath = visao_root + "/" + wPingPath + "/" + wPingName;
      
      writerPingChan = nPings;
      
      //get the pingfifoXX
      set_fifo_list_channel(&fl, writerPingChan, RWBUFF_SZ, std::string(tmppath+"_out").c_str(),std::string(tmppath+"_in").c_str(), 0, 0);
      _logger->log(Logger::LOG_LEV_INFO, "Set writer ping: %s", tmppath.c_str());
      nPings++;
   }
   
   serverPingEnabled = 0;
   if(hasServerPing)
   {
      tmppath = visao_root + "/" + sPingPath + "/" + sPingName;
      
      serverPingChan = nPings;
      
      //get the pingfifoXX
      set_fifo_list_channel(&fl, serverPingChan, RWBUFF_SZ, std::string(tmppath+"_out").c_str(),std::string(tmppath+"_in").c_str(), 0, 0);
      _logger->log(Logger::LOG_LEV_INFO, "Set server ping", tmppath.c_str());
      
      nPings++;
      serverPingEnabled = 1;
   }
   
   try
   {
      init_save_interval = (int)(ConfigDictionary())["init_save_interval"];
   }
   catch(Config_File_Exception)
   {
      init_save_interval = 100;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set interval to save init info to disk (init_save_interval): %i", init_save_interval);
   
   
   if(init_vars)
   {
      try
      {
         frameNo = (*init_vars)["frameNo"];
      }
      catch(Config_File_Exception)
      {
         frameNo = 0;
      }
   }
   
   //Init darks shared memory buffer
   int num_darks = 1;
   
   if(dark_shmem_key)
   {
      if(dark_sis.create_shm(dark_shmem_key, num_sharedim, sizeof(sharedim_stack_header) + num_darks*sizeof(intptr_t) + (num_darks)*(sizeof(sharedim<dataT>) + max_imsize*sizeof(dataT))) != 0)
      {
         ERROR_REPORT("Error attaching to shared memory for dark frame.");
      
         exit(0);
      }
      dark_sis.header->save_sequence = 0;
      dark_sim = dark_sis.set_next_image(1024, 1024);
      dark_temp.resize(1024*1024);
      dark_sis.enable_next_image();
   
      for(sh_i=0; sh_i<1024; sh_i++)
      {
         for(sh_j=0; sh_j<1024; sh_j++)
         {
            dark_sim->imdata[sh_i*1024+sh_j] = 0.;
         }
      }
   }//if(dark_shmem_key)
   
   writerPingEnabled = 0;
   
   num_skip = 0;

   STOP_FRAMEGRABBER = 1;
   RUNNING = 0;
   
   bias_width = 0;
   bias_height = 0;
   
}

template <class dataT> int framegrabber<dataT>::Run()
{
   if(sis.create_shm(shmem_key, num_sharedim, sizeof(sharedim_stack_header) + num_sharedim*sizeof(intptr_t) + (num_sharedim)*(sizeof(sharedim<dataT>) + max_imsize*sizeof(dataT))) != 0)
   {
      ERROR_REPORT("Error attaching to shared memory.");
      return -1;
   }
   
   sis.header->save_sequence = 0;
   
   //Install the main thread handler
   if(install_sig_mainthread_catcher() != 0)
   {
      ERROR_REPORT("Error installing main thread catcher.");
      return -1;
   }
   
   signal(SIGIO, SIG_IGN);
   
   //Startup the I/O signal handling thread
   if(start_signal_catcher() != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }
   
   //Now Block all I/O signals in this thread.
   if(block_sigio() != 0)
   {
      ERROR_REPORT("Error starting signal catching thread.");
      return -1;
   }
   
   LOG_INFO("starting up . . .");
   
   
   while(!TimeToDie)
   {
      pthread_mutex_lock(&signal_mutex);
      pthread_cond_wait(&signal_cond, &signal_mutex);
      pthread_mutex_unlock(&signal_mutex);
      
      if(!STOP_FRAMEGRABBER) start_framegrabber();
   }
   
   pthread_join(signal_thread, 0);
   
   return 0;
   
}

template <class dataT> int framegrabber<dataT>::start_framegrabber()
{
   return 0;
}

template <class dataT> int framegrabber<dataT>::stop_framegrabber()
{
   return 0;
}

template <class dataT> int framegrabber<dataT>::postGetImage()
{
   frameNo++;
   
   if((frameNo % init_save_interval) == 0) save_init();
   
   return 0;
}

template <class dataT> int framegrabber<dataT>::send_ping()
{
   //issue pings to notify waiting processes that the data is ready.
   if(writerPingEnabled)
   {
      if(skips_remaining <= 0)
      {
         skips_remaining = num_skip;
         write_fifo_channel(writerPingChan, "1", 2, 0);
         if(saves_remaining > 0) saves_remaining--;
         if(saves_remaining == 0)
         {
            writerPingEnabled = 0;
            std::cout << '\a' << std::endl; //Beep to say saving done.
         }
      }
      else
      {
         if(skips_remaining == 1)
         {
            sis.header->save_sequence++;
            write_fifo_channel(writerPingChan, "1", 2, 0); //On first ping after new sequence, no image is saved.
         }
         skips_remaining--;
      }
   }
   
   if(serverPingEnabled)
   {
      write_fifo_channel(serverPingChan, "1", 2, 0);
   }
   
   return 0;
}

template <class dataT> int framegrabber<dataT>::copyto_sharedim_short(unsigned char * im)
{
   sim = sis.set_next_image(width, height);
   sim->depth = depth;
   sim->frameNo = frameNo;
   sim->saved = 0;
   
   if(sim > 0)
   {
      for(sh_i=0; sh_i<width; sh_i++)
      {
         for(sh_j=0; sh_j<height; sh_j++)
         {
            sim->imdata[sh_i*width+sh_j] = (short) ((short *)im)[sh_i*width+sh_j];
         }
      }
      sim->frame_time = tv;
      sim->frame_time_dma = tv_dma;
      sis.enable_next_image();
   }
   
   return 0;
}

template <class dataT> int framegrabber<dataT>::copyto_sharedim_short_rotated(unsigned char * im)
{
   sim = sis.set_next_image(width, height);
   sim->depth = depth;
   sim->frameNo = frameNo;
   sim->saved = 0;
   
   if(sim > 0)
   {
      for(sh_i=0; sh_i<width; sh_i++)
      {
         for(sh_j=0; sh_j<height; sh_j++)
         {
            sim->imdata[sh_i*width+sh_j] = (short) ((short *)im)[sh_j*width+sh_i];
         }
      }
      sim->frame_time = tv;
      sim->frame_time_dma = tv_dma;
      sis.enable_next_image();
   }
   
   return 0;
}

template <class dataT> int framegrabber<dataT>::copyto_sharedim_short_deintlv_rotated(unsigned char * im)
{
   sim = sis.set_next_image(height, width);
   sim->depth = depth;
   sim->frameNo = frameNo;
   sim->saved = 0;
   
   if(sim > 0)
   {
      int sh_k = 0;
      
      for(sh_i=0; sh_i<(height-bias_height); sh_i+=2, sh_k++)
      {
         /*for(sh_j=0; sh_j<(height-bias_height); sh_j+=1)
          *         {
          *            sim->imdata[sh_k*(width-bias_width)+sh_j] = (short) ((short *)im)[sh_j*(width-bias_width)+sh_i];
          }*/
         for(sh_j=(width-bias_width)-1; sh_j>=0; sh_j-=1)
         {
            sim->imdata[sh_k*(height-bias_height)+sh_j] = (short) ((short *)im)[sh_j*(height-bias_height)+sh_i];
         }
      }
          
      sh_k = (height-bias_height)-1;
      for(sh_i=1; sh_i<(height-bias_height); sh_i+=2, sh_k--)
      {
             /*for(sh_j=0; sh_j< (height-bias_height); sh_j+=1)
              *         {
              *            sim->imdata[sh_k*(width-bias_width)+sh_j] = (short) ((short *)im)[sh_j*(width-bias_width)+sh_i];
              }*/
          for(sh_j=(width-bias_width)-1; sh_j>= 0; sh_j-=1)
          {
             sim->imdata[sh_k*(height-bias_height)+sh_j] = (short) ((short *)im)[sh_j*(height-bias_height)+sh_i];
          }
       }
       sim->frame_time = tv;
       sim->frame_time_dma = tv_dma;
       sis.enable_next_image();
   }
   return 0;
}

template <class dataT> int framegrabber<dataT>::addto_dark()
{
   if(!dark_shmem_key) return 0;
   
   if(no_darks_added == 0)
   {
      dark_sim->depth = depth;
      dark_sim->nx = width;
      dark_sim->ny = height;
      for(sh_i=0; sh_i<width; sh_i++)
      {
         for(sh_j=0; sh_j<height; sh_j++)
         {
            dark_temp[sh_i*width+sh_j] =0;
         }
      }
      std::cout << dark_sim->imdata << std::endl;
   }
   
   if(no_darks_added < calc_dark)
   {
      if(sim > 0)
      {
         for(sh_i=0; sh_i<width; sh_i++)
         {
            for(sh_j=0; sh_j<height; sh_j++)
            {
               dark_temp[sh_i*width+sh_j] += sim->imdata[sh_i*width+sh_j];
            }
         }
      }
      no_darks_added++;
   }
   else
   {
      for(sh_i=0; sh_i<width; sh_i++)
      {
         for(sh_j=0; sh_j<height; sh_j++)
         {
            dark_sim->imdata[sh_i*width+sh_j] = (int) ((double)dark_temp[sh_i*width+sh_j])/((double) no_darks_added);
            //std::cout << dark_sim->imdata[sh_i*width+sh_j] << " ";
         }
      }
      //std::cout << "\n";
      calc_dark = 0;
      no_darks_added = 0;
   }
}

template <class dataT> std::string framegrabber<dataT>::local_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received local command: %s.", com.c_str());
   resp = common_command(com, CMODE_LOCAL);
   if(resp == "") resp = "UNKOWN COMMAND\n";
   _logger->log(Logger::LOG_LEV_TRACE, "Response to local command: %s.", resp.c_str());
   return resp;
}

template <class dataT> std::string framegrabber<dataT>::script_command(std::string com)
{
   std::string resp;
   std::cout << "In script command" << std::endl;
   _logger->log(Logger::LOG_LEV_TRACE, "Received script command: %s.", com.c_str());
   resp = common_command(com, CMODE_SCRIPT);
   if(resp == "") resp = "UNKOWN COMMAND\n";
   _logger->log(Logger::LOG_LEV_TRACE, "Response to script command: %s.", resp.c_str());
   return resp;
}

template <class dataT> std::string framegrabber<dataT>::auto_command(std::string com, char *seqmsg __attribute__((unused)))
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received auto command: %s.", com.c_str());
   resp = common_command(com, CMODE_AUTO);

   if(resp == "") resp = post_auto_command(com);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to auto command: %s.", resp.c_str());
   return resp;
}

template <class dataT> std::string framegrabber<dataT>::common_command(std::string com, int cmode)
{
   char rstr[50];
   
   cmode = cmode;
   
   if(com == "start")
   {
      if(cmode == control_mode)
      {
         STOP_FRAMEGRABBER = 0;
         
         return "0\n";
      }
      else return control_mode_response();
   }
   
   if(com == "stop")
   {
      if(cmode == control_mode)
      {
         STOP_FRAMEGRABBER = 1;
         pthread_kill(main_thread, SIG_MAINTHREAD);
         writerPingEnabled = 0;
         saves_remaining = 0;
      }
      else return control_mode_response();
      
      return "0\n";
   }
   
   if(com == "running?")
   {
      if(RUNNING) return "1\n";
      else return "0\n";
   }
   
   if(com == "save?")
   {
      if(writerPingEnabled ) return "1\n";
      else return "0\n";
   }
   
   if(com == "serve?")
   {
      if(serverPingEnabled ) return "1\n";
      else return "0\n";
   }
   
   if(com == "remaining?")
   {
      snprintf(rstr, 50, "%i\n", saves_remaining);
      return rstr;
   }
   
   if(com == "skip?")
   {
      snprintf(rstr, 50, "%i\n", num_skip);
      return rstr;
   }
   
   if(com.substr(0,4) == "skip")
   {
      if(cmode == control_mode)
      {
         int nskip = atoi(com.substr(5, com.length()-5).c_str());
         
         if(nskip > 0) num_skip = nskip;
         else num_skip = 0;
         return "0\n";
      }
      else return control_mode_response();
   }
   
   if(com.substr(0,8) == "savedark")
   {
      if(cmode == control_mode)
      {
         int nsav = atoi(com.substr(9, com.length()-9).c_str());
         std::cout << nsav << "\n";
         if(nsav != 0 && !writerPingEnabled)
         {
            writerPingEnabled = 1;
            sis.header->save_sequence++;
            saves_remaining = nsav; //don't know why yet, but framewriter skips first ping.
            skips_remaining = 0; //save first one, then start skipping.
            
            calc_dark = nsav;
            no_darks_added = 0;
            
         }
         
         if(nsav == 0)
         {
            calc_dark = 0;
            writerPingEnabled = 0;
            saves_remaining = 0;
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
         
         if(nsav != 0 && !writerPingEnabled)
         {
            writerPingEnabled = 1;
            sis.header->save_sequence++;
            if(nsav > 0) saves_remaining = nsav; //don't know why yet, but framewriter skips first ping.
            else saves_remaining = -1;
            skips_remaining = 0; //save first one, then start skipping.
         }
         
         if(nsav == 0)
         {
            writerPingEnabled = 0;
            saves_remaining = 0;
         }
         
         return "0\n";
      }
      else return control_mode_response();
   }
   
   
   
   if(com.substr(0,4) == "dark")
   {
      if(cmode == control_mode)
      {
         int ndrk = atoi(com.substr(5, com.length()-5).c_str());
         
         if(ndrk > 0) 
         {
            calc_dark = ndrk;
            no_darks_added = 0;
         }
         else calc_dark = 0;
         return "0\n";
      }
      else return control_mode_response();
   }
   
   if(com.substr(0,5) == "serve")
   {
      if(cmode == control_mode)
      {
         int nsav = atoi(com.substr(5, com.length()-5).c_str());
         
         if(nsav != 0)
         {
            serverPingEnabled = 1;
            if(writerPingEnabled == 0) sis.header->save_sequence++; //don't scram the writer
         }
         
         if(nsav == 0)
         {
            serverPingEnabled = 0;
         }
         
         return "0\n";
      }
      else return control_mode_response();
   }

   if(com == "state?")
   {
      snprintf(rstr, 50, "%c,%i,%i,%i,%i,%i\n", control_mode_response()[0], RUNNING, writerPingEnabled, num_skip, saves_remaining-1, serverPingEnabled);
      return rstr;
   }
   
   return "";
};


template <class dataT> int framegrabber<dataT>::save_init()
{
   std::ofstream of;
   
   of.open((std::string(getenv("VISAO_ROOT")) + "/" + init_file).c_str());
   
   of.precision(10);
   of << "frameNo    int      " << frameNo << "\n";
   
   of.close();
   
   return 0;
}

template <class dataT> int framegrabber<dataT>::delete_init()
{
   remove((std::string(getenv("VISAO_ROOT")) + "/" + init_file).c_str());
   
   return 0;
}

      
} //namespace VisAO


#endif //__framegrabber_h__
