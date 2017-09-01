/************************************************************
*    framegrabber47.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for a class to manage the EDT PCI framegrabber for the CCD47.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber47.cpp
  * \author Jared R. Males
  * \brief Definitions for a class to manage the EDT PCI framegrabber for the CCD47.
  * 
  *
*/

#include "framegrabber47.h"


namespace VisAO
{
   
framegrabber47::framegrabber47(int argc, char **argv) throw (AOException) : framegrabber<short>(argc, argv)
{
   init_framegrabber47();
}

framegrabber47::framegrabber47(std::string name, const std::string &conffile) throw (AOException) : framegrabber<short>(name, conffile)
{
   init_framegrabber47();
}

void framegrabber47::init_framegrabber47()
{
   edt_devname[128] = '\0';
   channel = 0;
   unit  = 0;

   overruns = 0;

   last_timeouts = 0;

   recovering_timeout = FALSE;
   
   calc_dark = 0;
   
   //Set the number of buffers
   try
   {
      num_bufs = (int)(ConfigDictionary())["num_bufs"];
   }
   catch(Config_File_Exception)
   {
      num_bufs = 4;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set number of DMA buffers used by EDT driver (num_bufs): %i", num_bufs);

   
   
   //Init the status board
   statusboard_shmemkey = STATUS_framegrabber47;
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


int framegrabber47::start_framegrabber()
{
   /*
    * open the interface
    * 
    * EDT_INTERFACE is defined in edtdef.h (included via edtinc.h)
    *
    * edt_parse_unit_channel and pdv_open_channel) are equivalent to
    * edt_parse_unit and pdv_open except for the extra channel arg and
    * would normally be 0 unless there's another camera (or simulator)
    * on the second channel (camera link) or daisy-chained RCI (PCI FOI)
    */
   if (edt_devname[0])
   {
      unit = edt_parse_unit_channel(edt_devname, edt_devname, EDT_INTERFACE, &channel);
   }
   else
   {
      strcpy(edt_devname, EDT_INTERFACE);
   }
   //printf("%s %i % i\n", edt_devname, unit, channel);

   if ((pdv_p = pdv_open_channel(edt_devname, unit, channel)) == NULL)
   {
      logss.str("");
      logss << "pdv_open_channel(" << edt_devname << unit << channel << ") failed.";
      log_msg(Logger::LOG_LEV_ERROR, logss.str());
      std::cerr << logss.str() << " (logged) \n";
      return -1;
   }

   /*
    * get image size and name for display, save, printfs, etc.
    */
   width = pdv_get_width(pdv_p);
   if(width > 1024) bias_width = width-1024;
   else bias_width = 0;
   
   height = pdv_get_height(pdv_p);
   if(height > 1024) bias_height = height-1024;
   else bias_height = 0;
   
   depth = pdv_get_depth(pdv_p);
   cameratype = pdv_get_cameratype(pdv_p);

   u_int edt_timestamp[2];
   /*
    * allocate buffers for optimal pdv ring buffer pipeline (reduce if
    * memory is at a premium)
    */
   pdv_multibuf(pdv_p, num_bufs);

   logss.str("");
   logss << "reading images from '" << cameratype << "' width " << width << " height " << height << " depth " << depth << ".";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   std::cerr << logss.str() << " (logged) \n";

   /*
    * prestart the first image or images outside the loop to get the
    * pipeline going. Start multiple images unless force_single set in
    * config file, since some cameras (e.g. ones that need a gap between
    * images or that take a serial command to start every image) don't
    * tolerate queueing of multiple images
    */
//    if (pdv_p->dd_p->force_single)
//    {
//       pdv_start_image(pdv_p);
//       RUNNING = 1;
//    }
//    else
//   {
      pdv_start_images(pdv_p, 0);// starts freerun (instead of using num_bufs);
      RUNNING = num_bufs;
//   }
   

   logss.str("");
   logss << "framegrabber running.";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   std::cerr << logss.str() << " (logged) \n";

   u_char **image_buff = pdv_buffer_addresses(pdv_p);
   //int buffi = 0;
   while(!STOP_FRAMEGRABBER && !TimeToDie)
   {

      /*
       * get the image and immediately start the next one (if not the last
       * time through the loop). Processing (saving to a file in this case)
       * can then occur in parallel with the next acquisition
       */
             
/*      int loop_open_counter = 0;
      
      if(aosb) loop_open_counter == aosb->loop_open_counter;*/
      
      image_p = pdv_wait_image_timed(pdv_p, edt_timestamp);
      //frameNo = edt_done_count(pdv_p);
      gettimeofday(&tv, 0);      
      tv_dma.tv_sec = edt_timestamp[0];
      tv_dma.tv_usec = edt_timestamp[1];
      
      //if(tv.tv_usec == last_usec) tv.tv_usec++;
      //last_usec = tv.tv_usec;
//       if(aosb && close_loop_save) 
//       {
//          //Loop changed state during exposure - do not save.
//          if(loop_open_counter != aosb->loop_open_counter || aosb->loop_on != 1) continue;
//       }
      
      postGetImage();
      
      if ((overrun = (edt_reg_read(pdv_p, PDV_STAT) & PDV_OVERRUN))) ++overruns;

      if (!STOP_FRAMEGRABBER)
      {
         //pdv_start_image(pdv_p);
      
         timeouts = pdv_timeouts(pdv_p);
      }
      /*
       * check for timeouts or data overruns -- timeouts occur when data
       * is lost, camera isn't hooked up, etc, and application programs
       * should always check for them. data overruns usually occur as a
       * result of a timeout but should be checked for separately since
       * ROI can sometimes mask timeouts
       */
      if (timeouts > last_timeouts)
      {
         /*
         * pdv_timeout_cleanup helps recover gracefully after a timeout,
         * particularly if multiple buffers were prestarted
         */
         pdv_timeout_restart(pdv_p, TRUE);
         last_timeouts = timeouts;
         recovering_timeout = TRUE;
         logss.str("");
         logss << "timeout.";
         log_msg(Logger::LOG_LEV_ERROR, logss.str());
         std::cerr << logss.str() << " (logged) \n";
      }
      else if (recovering_timeout)
      {
         pdv_timeout_restart(pdv_p, TRUE);
         recovering_timeout = FALSE;
         logss.str("");
         logss << "....restarted....";
         log_msg(Logger::LOG_LEV_ERROR, logss.str());
         std::cerr << logss.str() << " (logged) \n";
      }

      copyto_sharedim_short_deintlv_rotated(image_p);
      
      if(calc_dark)
      {
         addto_dark();
      }
      send_ping();
    }


   logss.str("");
   logss << frameNo << " images " << last_timeouts << " timeouts " << overruns << " overruns.";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   std::cerr << logss.str() << " (logged) \n";

   return stop_framegrabber();
}

int framegrabber47::stop_framegrabber()
{
   if(pdv_p) pdv_close(pdv_p);
   pdv_p = 0;
   
   edt_devname[128] = '\0';
   channel = 0;
   unit  = 0;

   overruns = 0;

   last_timeouts = 0;
      
   recovering_timeout = FALSE;

   RUNNING = 0;
   
   logss.str("");
   logss << "framegrabber stopped.";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   std::cerr << logss.str() << " (logged) \n";
   
   return 0;
}

} //namespace VisAO
      