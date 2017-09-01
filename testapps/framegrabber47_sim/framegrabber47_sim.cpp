/************************************************************
*    framegrabber47_sim.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for a class to manage a simulated framegrabber  of CCD47 data.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber47_sim.cpp
  * \author Jared R. Males
  * \brief Definitions for a class to manage a simulated framegrabber of CCD47 data.
  * 
  *
*/

#include "framegrabber47_sim.h"

namespace VisAO
{
   
framegrabber47_sim::framegrabber47_sim(int argc, char **argv) throw (AOException) : framegrabber<short>(argc, argv)
{
   init_framegrabber47_sim();
}

framegrabber47_sim::framegrabber47_sim(std::string name, const std::string &conffile) throw (AOException) : framegrabber<short>(name, conffile)
{
   init_framegrabber47_sim();
}

void framegrabber47_sim::init_framegrabber47_sim()
{
   
   //Read the mode
   try
   {
      sim_mode = (int)(ConfigDictionary())["sim_mode"];
   }
   catch(Config_File_Exception)
   {
      sim_mode = 0;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the simulation mode: %i", sim_mode);
   
   //Read the frame rate
   try
   {
      sim_rate = (double)(ConfigDictionary())["sim_rate"];
   }
   catch(Config_File_Exception)
   {
      sim_rate = 0;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the simulated framerate: %f", sim_rate);
   
   
   if(sim_mode ==0 || sim_mode == 2)
   {
      fitsfile * fptr1 = 0, *fptr2 =0, *fptr3=0, *fptr4=0;
      long fpixel[2] = {1,1};;
      long lpixel[2] = {1024,1024};
      
      im1024 = new short*[4];
   
      for(int i=0;i<4;i++) im1024[i] = new short[1024*1024];     
     
      get_fits_im(im1024[0], 1, fpixel, lpixel, &fptr1, "data/1024x1024/im1.fits");
      get_fits_im(im1024[1], 1, fpixel, lpixel, &fptr2, "data/1024x1024/im2.fits");
      get_fits_im(im1024[2], 1, fpixel, lpixel, &fptr3, "data/1024x1024/im3.fits");
      get_fits_im(im1024[3], 1, fpixel, lpixel, &fptr4, "data/1024x1024/im4.fits");
      
      im1024[0][1024*512+512] = 18000;
      im1024[1][1024*512+512] = 18000;
   }
   if(sim_mode == 1 || sim_mode == 2)
   {
      im128 = new short*[30];
      load_128x128(im128);
   }
   
   if(sim_mode == 3)
   {
      int fstatus;
      fitsfile * fptr1 = 0, *fptr2 =0, *fptr3=0, *fptr4=0;
      long fpixel[2] = {1,1};;
      long lpixel[2] = {1024,1024};
      
      im1024 = new short*[72];
   
      for(int i=0;i<72;i++) im1024[i] = new short[1024*1024];

      std::ifstream fin;
      fin.open("data/1024x1024_tt_ip_unsat/fits.list");
      std::string fname;

      for(int i=0;i<72;i++)
      {
         fin >> fname;   
         fptr1 = 0;
         get_fits_im(im1024[i], 1, fpixel, lpixel, &fptr1, fname.c_str());
         fits_close_file(fptr1, &fstatus);
      }
   }

   if(sim_mode == 4)
   {
      int fstatus;
      fitsfile * fptr1 = 0, *fptr2 =0, *fptr3=0, *fptr4=0;
      long fpixel[2] = {1,1};;
      long lpixel[2] = {1024,1024};
      
      im1024 = new short*[63];
   
      for(int i=0;i<63;i++) im1024[i] = new short[1024*1024];

      std::ifstream fin;
      fin.open("data/1024x1024_tt_ip_unsat/darks/fits.list");
      std::string fname;

      for(int i=0;i<63;i++)
      {
         fin >> fname;   
         fptr1 = 0;
         get_fits_im(im1024[i], 1, fpixel, lpixel, &fptr1, fname.c_str());
         fits_close_file(fptr1, &fstatus);
      }
   }

   request_control(CMODE_LOCAL);
   STOP_FRAMEGRABBER = 0;
   
   signalth_sleeptime = 99999.;

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

int framegrabber47_sim::start_framegrabber()
{
   double t0, dt, frametime;
   int imsz, nims;
   short **ims;
   struct timespec sleeptime;
   int called = 0;
   
   std::cout << "Starting . . .\n";
   
   RUNNING = 1;
   if(sim_mode == 0 || sim_mode == 2)
   {
      ims = im1024;
      imsz = 1024;
      nims=4;
   }
   else if(sim_mode == 1)
   {
      ims = im128;
      imsz = 128;
      nims = 30;
   }
   else if(sim_mode == 3)
   {
      ims = im1024;
      imsz = 1024;
      nims = 72;
   }
   else if(sim_mode == 4)
   {
      ims = im1024;
      imsz = 1024;
      nims = 63;
   }

   frametime = 1./sim_rate;
   
   while(!STOP_FRAMEGRABBER && !TimeToDie)
   {
      for(int k=0; k<nims;k++)
      {
         width = imsz;
         height = imsz;
         depth = 14;

         gettimeofday(&tv, 0);
         postGetImage();
      
         if(imsz == 1024) copyto_sharedim_short_rotated((u_char *) ims[k]);

         if(imsz == 128)  copyto_sharedim_short((u_char *) ims[k]);
         
         send_ping();
         
         if(!called)
         {
            t0 = get_curr_time();
            called = 1;
         }
         
         dt = get_curr_time() - t0;
         while(dt < .75*frametime && !TimeToDie)
         {
            sleeptime.tv_sec = (int)dt;
            sleeptime.tv_nsec = .25*(dt - sleeptime.tv_sec) * 1e9;
            nanosleep(&sleeptime, 0);
            dt = get_curr_time() - t0;
         }
         
         while(dt < frametime && !TimeToDie)
         {
            dt = get_curr_time() - t0;
         }
         
         t0 = get_curr_time();
         if(STOP_FRAMEGRABBER || TimeToDie) break;
      }
      if(sim_mode == 2)
      {
         if(imsz == 1024)
         {
            ims = im128;
            imsz = 128;
            nims = 30;
         }
         else
         {
            ims = im1024;
            imsz = 1024;
            nims=4;
         }
      }
      if(STOP_FRAMEGRABBER || TimeToDie) break;
   }
   return stop_framegrabber();
}

int framegrabber47_sim::stop_framegrabber()
{
   RUNNING = 0;
   
   logss.str("");
   logss << "framegrabber stopped.";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   std::cerr << logss.str() << " (logged) \n";
   
   return 0;
}

int get_fits_im(short *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname)
{
   int fstatus = 0;
   long inc[2] = {1,1};
   //std::cout << fname << "\n" << hduno << "\n";
   if(*fptr == 0 && fname > 0)
   {
      fits_open_file(fptr, fname, READONLY, &fstatus);
      if (fstatus) 
      {
         fprintf(stderr, "Error in get_fits_data.\n");
         fits_report_error(stderr, fstatus); /* print any error message */
         return -1;
      }
   }
   fits_movabs_hdu(*fptr, hduno, NULL, &fstatus);
   if (fstatus) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); /* print any error message */
      return -1;
   }
   
   
   fits_read_subset(*fptr, TSHORT, fpix, lpix, inc, 0, im, 0, &fstatus);
   if (fstatus && fstatus != 107) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); /* print any error message */
      return -1;
   }
   
   return fstatus;
}

int get_fits_im(double *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, char *fname)
{
   int fstatus = 0;
   long inc[2] = {1,1};
   //std::cout << fname << "\n" << hduno << "\n";
   if(*fptr == 0 && fname > 0)
   {
      fits_open_file(fptr, fname, READONLY, &fstatus);
      if (fstatus) 
      {
         fprintf(stderr, "Error in get_fits_data.\n");
         fits_report_error(stderr, fstatus); /* print any error message */
         return -1;
      }
   }
   fits_movabs_hdu(*fptr, hduno, NULL, &fstatus);
   if (fstatus) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); /* print any error message */
      return -1;
   }
   
   
   fits_read_subset(*fptr, TDOUBLE, fpix, lpix, inc, 0, im, 0, &fstatus);
   if (fstatus && fstatus != 107) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); /* print any error message */
      return -1;
   }
   
   return fstatus;
}

int load_128x128(short ** imd128)
{
   double tmpim[128*128];
   char fname[200];
   long fpixel[2] = {1,1};;
   long lpixel[2] = {128,128};
   int fstatus = 0;
   fitsfile * fptr;
   
   for(int i=0;i<30;i++) 
   {
   
      imd128[i] = new short[128*128];
      sprintf(fname, "data/128x128/%04i.fits", i);
      fptr = 0;   
      get_fits_im(tmpim, 1, fpixel, lpixel, &fptr, fname);
      fits_close_file(fptr, &fstatus);
      for(int j=0;j<128*128;j++) imd128[i][j] = 2000.*tmpim[j];
   }

   return 0;
}

} //namespace VisAO
      
