/************************************************************
*    framegrabber39_sim.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for a class to manage a simulated framegrabber of CCD39 data.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file framegrabber_sim.cpp
  * \author Jared R. Males
  * \brief Definitions for a class to manage a simulated framegrabber of CCD39 data.
  * 
  *
*/

#include "framegrabber39_sim.h"

namespace VisAO
{
   
framegrabber39_sim::framegrabber39_sim(int argc, char **argv) throw (AOException) : framegrabber<unsigned char>(argc, argv)
{
   init_framegrabber39_sim();
}

framegrabber39_sim::framegrabber39_sim(std::string name, const std::string &conffile) throw (AOException) : framegrabber<unsigned char>(name, conffile)
{
   init_framegrabber39_sim();
}

framegrabber39_sim::~framegrabber39_sim()
{
   if(impyr)
   {
      for(int i=0;i<2500;i++)
      {
         delete impyr[i];
         delete slopes[i];
      }
      delete impyr;
      delete slopes;
   }

   if(im39)
   {
      for(int i=0;i<2500; i++) delete im39[i];
      delete im39;
   }
   

}

void framegrabber39_sim::init_framegrabber39_sim()
{
   //Read the mode
   try
   {
      sim_mode = (int)(ConfigDictionary())["sim_mode"];
   }
   catch(Config_File_Exception)
   {
      sim_mode = 1;
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

   impyr = 0;
   slopes = 0;
   im39 = 0;

   if(sim_mode == 0) load_caos_data();
   if(sim_mode == 1) load_bcu39_data();
   request_control(CMODE_LOCAL);
   STOP_FRAMEGRABBER = 1;
   
   signalth_sleeptime = 99999.;

   //Init the status board
   statusboard_shmemkey = STATUS_framegrabber39;
   if(create_statusboard(sizeof(framegrabber39_status_board)) != 0)
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

void framegrabber39_sim::load_caos_data()
{
   fitsfile * fptr1 = 0;
   long fpixel[3] = {1,1,51};;
   long lpixel[3] = {80,80,51};
   
   impyr = new short*[2500];
   
   for(int i=0;i<2500;i++) impyr[i] = new short[80*80];
   
   double *tmpim = new double[80*80];
   std::string fname = "data/sim39/pyrout_ex.fits";
   get_fits_im(tmpim, 1, fpixel, lpixel, &fptr1, fname.c_str());
   for(int j=0;j<80*80; j++) impyr[0][j] = (short)tmpim[j];
   
   for(int i=1; i< 2500; i++)
   {
      fpixel[2] = i+51;
      lpixel[2] = i+51;
      get_fits_im(tmpim, 1, fpixel, lpixel, &fptr1, 0);
      for(int j=0;j<80*80; j++) impyr[i][j] = (short)tmpim[j];
   }
   delete tmpim;
   
   slopes = new float*[2500];
   for(int i=0;i<2500;i++) slopes[i] = new float[1096];
   
   std::ifstream fin;
   fin.open("data/sim39/slopes_from_pyr.txt");
   double tmpd;
   for(int i=0;i<50;i++) for(int j=0;j<1096;j++) fin >> tmpd;
   
   for(int i=50;i<2550;i++) for(int j=0;j<1096;j++) fin >> slopes[i-50][j];
   
   fin.close();
}

void framegrabber39_sim::load_bcu39_data()
{
   std::ifstream fin;
   std::string fname, dir;

   fitsfile * fptr1 = 0;
   long fpixel[2] = {1,1};;
   long lpixel[2] = {19248,1};

   int N = 2500;
   im39 = new unsigned char*[N];
   
   for(int i=0;i<N;i++) im39[i] = new unsigned char[sizeof(slopecomp_diagframe_pixels_slopes)];

   unsigned char *tmpim = new unsigned char[sizeof(slopecomp_diagframe_pixels_slopes)];
   
   fin.open("data/bcu39/bcu39.list");
   dir = "data/bcu39/";
   int status;
   for(int i=0;i<N;i++)
   {
      fin >> fname;
      get_fits_im(tmpim, 1, fpixel, lpixel, &fptr1, (dir+fname).c_str());
      for(int j=0;j<sizeof(slopecomp_diagframe_pixels_slopes); j++) im39[i][j] = tmpim[j];
      fits_close_file(fptr1, &status);
      fptr1 = 0;
   }

   delete tmpim;
   fin.close();
}


/*int framegrabber39_sim::Run()
{
   return framegrabber<unsigned char>::Run();
}*/

int framegrabber39_sim::start_framegrabber()
{
   double t0=0, dt, frametime;
   int imsz, nims;
   //short **ims;
   struct timespec sleeptime;
   int called = 0;
   
   std::cout << "Starting . . .\n";
   
   RUNNING = 1;

   //ims = impyr;
   imsz = 80;

   if(sim_mode == 0) nims=2500;
   else nims=2500;

   std::cout << nims << "\n";
   
   frametime = 1./sim_rate;
   double startt, endt;
   frameCnt = 0;
   loopt0 = get_curr_time();

   float *sl;
   while(!STOP_FRAMEGRABBER && !TimeToDie)
   {
      startt = get_curr_time();
      for(int k=0; k<nims;k++)
      {
         width = 19248;
         height = 1;
         depth = 8;

         gettimeofday(&tv, 0);
         postGetImage();
      
         sim = sis.set_next_image(19248, 1);
         sim->depth = 8;
         sim->frameNo = frameNo;
         
         if(sim > 0)
         {
            if(sim_mode == 0)
            {
               for(sh_i=0; sh_i<12800; sh_i++)
               {
                  sim->imdata[16 + sh_i] = ((unsigned char *)impyr[k])[sh_i];
               }

               for(sh_i=0; sh_i<4384; sh_i++)
               {
                  sim->imdata[sh_i+12800+32] = ((unsigned char *)slopes[k])[sh_i];
               }
            }
            else if(sim_mode == 1)
            {
               for(sh_i = 0; sh_i < sizeof(slopecomp_diagframe_pixels_slopes); sh_i++)
               {
                  sim->imdata[sh_i] = im39[k][sh_i];
               }

            }
            sim->frame_time = tv;
            sis.enable_next_image();
            frameCnt++;
         }
            
         send_ping();
         
         if(!called)
         {
            t0 = get_curr_time();
            called = 1;
         }
         
         dt = get_curr_time() - t0;
         while(dt < .75*frametime)
         {
            sleeptime.tv_sec = (int)dt;
            sleeptime.tv_nsec = .1*(dt - sleeptime.tv_sec) * 1e9;
            nanosleep(&sleeptime, 0);
            dt = get_curr_time() - t0;
         }
         
         while(dt < frametime-.000015)
         {
            dt = get_curr_time() - t0;
         }
         
         t0 = get_curr_time();
         if(STOP_FRAMEGRABBER || TimeToDie) break;
      }
      endt = get_curr_time();
      std::cout << ((double)nims)/(endt-startt) << "\n";
      
      //if(STOP_FRAMEGRABBER || TimeToDie) break;
   }
   return stop_framegrabber();
}

int framegrabber39_sim::stop_framegrabber()
{
   RUNNING = 0;
   
   logss.str("");
   logss << "framegrabber stopped.";
   log_msg(Logger::LOG_LEV_INFO, logss.str());
   std::cerr << logss.str() << " (logged) \n";
   
   return 0;
}

int framegrabber39_sim::update_statusboard()
{
   static int last_frameCnt = 0;
   static double last_loopt0 = loopt0;
   
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
      
      VisAO::framegrabber39_status_board * fsb = (VisAO::framegrabber39_status_board *) statusboard_shmemptr;
      
      fsb->running = RUNNING;
      fsb->reconstructing = serverPingEnabled;
      fsb->saving = writerPingEnabled;
      
      if(RUNNING)
      {
         double t1 = get_curr_time();
         fsb->fps = ((double)(frameCnt-last_frameCnt))/(t1-last_loopt0);
         last_frameCnt = frameCnt;
         last_loopt0 = t1;
      }
      else fsb->fps = 0.;
   }
   return 0;
}//int framegrabber39_sim::update_statusboard()

/*
int get_fits_im(unsigned char *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname)
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
         fits_report_error(stderr, fstatus); // print any error message 
         return -1;
      }
   }
   fits_movabs_hdu(*fptr, hduno, NULL, &fstatus);
   if (fstatus)
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message 
      return -1;
   }
   
   
   fits_read_subset(*fptr, TBYTE, fpix, lpix, inc, 0, im, 0, &fstatus);
   if (fstatus && fstatus != 107)
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message 
      return -1;
   }
   
   return fstatus;
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
         fits_report_error(stderr, fstatus); // print any error message
         return -1;
      }
   }
   fits_movabs_hdu(*fptr, hduno, NULL, &fstatus);
   if (fstatus) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message 
      return -1;
   }
   
   
   fits_read_subset(*fptr, TDOUBLE, fpix, lpix, inc, 0, im, 0, &fstatus);
   if (fstatus && fstatus != 107) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message
      return -1;
   }
   
   return fstatus;
}

int get_fits_im(double *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname)
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
         fits_report_error(stderr, fstatus); // print any error message 
         return -1;
      }
   }

//    int naxis;
//    fits_get_img_dim(*fptr, &naxis, &fstatus);
// 
//    long * naxes = new long[naxis];
//    
//    fits_get_img_size(*fptr, naxis, naxes, &fstatus);
// 
//    for(int q=0;q<naxis;q++) std::cout << naxes[q] << " ";
//    std::cout << "\n";
//    exit(0);




   
   fits_movabs_hdu(*fptr, hduno, NULL, &fstatus);
   if (fstatus) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message 
      return -1;
   }
   
   
   fits_read_subset(*fptr, TDOUBLE, fpix, lpix, inc, 0, im, 0, &fstatus);
   if (fstatus && fstatus != 107) 
   {
      fprintf(stderr, "Error in get_fits_data.\n");
      fits_report_error(stderr, fstatus); // print any error message 
      return -1;
   }
   
   return fstatus;
}*/

int load_128x128(short ** imd128)
{
   double tmpim[128*128];
   char fname[20];
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
      
