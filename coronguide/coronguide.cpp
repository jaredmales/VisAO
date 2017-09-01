/************************************************************
*    coronguide.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for a class to auto guide for the coronagraph
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file coronguide.h
  * \author Jared R. Males
  * \brief Definitions for a class to auto guide for the coronagraph
  * 
  *
*/

#include "coronguide.h"

namespace VisAO
{

coronguide::coronguide(int argc, char **argv) throw (AOException) : VisAOApp_standalone(argc, argv)
{
   Create();
}

coronguide::coronguide(std::string name, const std::string &conffile) throw (AOException) : VisAOApp_standalone(name, conffile)
{
   Create();
}

int coronguide::Create()
{
  
   std::string visao_root = getenv("VISAO_ROOT");
   
     
   setup_fifo_list(4);
   setup_baseApp(0, 1, 1, 1, false);
   //setup_baseApp();
    
   signal(SIGIO, SIG_IGN);
  
   try
   {
      gimbscale = (int)(ConfigDictionary())["gimbscale"];
   }
   catch(Config_File_Exception)
   {
      _logger->log(Logger::LOG_LEV_FATAL, "Gimbal scale (gimbscale) is a required config parameter.");
      throw;
   }
   
   try
   {
      ap_recov_time = (double)(ConfigDictionary())["ap_recov_time"];
   }
   catch(Config_File_Exception)
   {
      ap_recov_time = 1.5;
      _logger->log(Logger::LOG_LEV_INFO, "Auto pause recover time set to default %f secs.", ap_recov_time);
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
   attached_to_shmem  = false;
   
   
   std::string gimbal_fifopath;

   try
   {
      gimbal_fifopath = (std::string)(ConfigDictionary())["gimbal_fifopath"];
   }
   catch(...)
   {
      gimbal_fifopath = "fifos";
   }
   
   std::string fpathin = visao_root + "/" + gimbal_fifopath + "/gimbal_com_script_in";
   std::string fpathout = visao_root + "/" + gimbal_fifopath + "/gimbal_com_script_out";
   set_fifo_list_channel(&fl, 0, 100, fpathout.c_str(), fpathin.c_str(), 0, 0);
   
  
   //Init the status board for known process names
   statusboard_shmemkey = 0;
   if(MyFullName() == "coronguide.L")
   {
      //std::cout << "Initing for coronguide.L\n";
      statusboard_shmemkey = STATUS_coronguide;
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

   isb = 0;
   aosb = 0;
   sis = 0;
   dark_sis  = 0;
   ssb = 0;
   ccdsb = 0;
   sbconn = 0;
   
   loop_gain = 0.01;
   
   wait_to = .5;
   
   last_x0 = 0;
   last_x1 = 0;
   last_y0 = 0;
   last_y1 = 0;
   
   tempIm = 0;
   backgroundIm = 0;
   kernel = 0;
   
   bgAlgorithm = MEDSUB;
   kfwhm = 10.;
   ksize = 4.;
   
   avgLen = 10;
   xcens.resize(10);
   ycens.resize(10);
   inAvg = 0;
   avgidx = 0;
   
   useAvg = false;
   minMaxReject = true;
   
   gimbCounter = 0;
   pthread_mutex_init(&memmutex, NULL);
   
   return 0;
}

int coronguide::open_loop()
{
   loopState = 0;
   
   struct timeval tv;
   gettimeofday(&tv,0);
   dataLogger(tv);
   
   
   
   
   return 0;
}

int coronguide::close_loop()
{
   
   if(useAvg && (xcen_avg == NOAVG || ycen_avg == NOAVG)) return -1;
   
   if(loopState == 0)
   {
      
      if(useAvg)
      {
         tgtx = xcen_avg;
         tgty = ycen_avg;
      }
      else
      {
         tgtx = xcen;
         tgty = ycen;
      }
      
      _logger->log(Logger::LOG_LEV_INFO, "Loop closed on target x: %f y: %f", tgtx, tgty);
   }
   
   loopState = 1;
   
   struct timeval tv;
   gettimeofday(&tv,0);
   dataLogger(tv);
   
   return 0;
}

int coronguide::pause_loop()
{     
   if(loopState == 0) return -1;
   
   loopState = 2;
   
   struct timeval tv;
   gettimeofday(&tv,0);
   dataLogger(tv);
   
   return 0;
}

int coronguide::set_loop_gain(double lg)
{
   if(lg < 0 || lg >=1) 
   {
      _logger->log(Logger::LOG_LEV_INFO, "Crazy loop gain requested: %f", lg);
      
      lg = .01;
      
   }
   
   loop_gain = lg;
   
   _logger->log(Logger::LOG_LEV_INFO, "Loop gain set to: %f", loop_gain);
   
   struct timeval tv;
   gettimeofday(&tv,0);
   dataLogger(tv);
   
   return 0;
}

int coronguide::set_bgalgo(int bga)
{
   pthread_mutex_lock(&memmutex);
   
   switch(bga)
   {
      case MEDSUB:
         bgAlgorithm = MEDSUB;
         break;
      case UNSHARP:
         bgAlgorithm = UNSHARP;
         break;
      default:
         bgAlgorithm = MEDSUB;
   }
   
   if(backgroundIm)
   {
      delete backgroundIm;
      backgroundIm = 0;
   }
   
   pthread_mutex_unlock(&memmutex);
   return 0;
}


int coronguide::set_kfwhm(double kfw)
{
   pthread_mutex_lock(&memmutex);
   kfwhm = kfw;
   
   if(kernel)
   {
      delete kernel;
      kernel = 0;
   }
   pthread_mutex_unlock(&memmutex);
   return 0;
   
}

int coronguide::set_ksize(int ks)
{
   pthread_mutex_lock(&memmutex);
   
   ksize = ks;
   
   if(kernel)
   {
      delete kernel;
      kernel = 0;
   }
   
   pthread_mutex_unlock(&memmutex);
   return 0;
}
   
int coronguide::setUseAvg(bool ua)
{
   useAvg = ua;
   //std::cerr << "useAvg = " << useAvg << "\n";
}

int coronguide::setMinMaxReject(bool mmr)
{
   minMaxReject = mmr;
   //std::cerr << "minMaxReject = " << minMaxReject << "\n";
   
}
   
int coronguide::setAvgLen(int na)
{
   if(na != avgLen && na > 0)
   {
      pthread_mutex_lock(&memmutex);
      
      avgLen = na;
      xcens.resize(na);
      ycens.resize(na);
      inAvg = 0;
      avgidx = 0;
      
      xcen_avg = NOAVG;
      ycen_avg = NOAVG;
   
      //std::cerr << "avgLen = " << avgLen << "\n";
   
      pthread_mutex_unlock(&memmutex);
   }
}
   
int coronguide::set_shmem_key(int sk)
{
   shmem_key = sk;
   return 0;
}

int coronguide::connect_shmem()
{
   sis = new sharedim_stack<short>;
   if(sis->attach_shm(shmem_key) != 0)
   {
      ERROR_REPORT("Error attaching to shared memory.");
      attached_to_shmem = false;
      delete sis;
      sis = 0;
      dark_sis  = 0;
      return -1;
   }
   
   dark_sis = new sharedim_stackS;
   if(dark_sis->attach_shm(5002) != 0)
   {
      ERROR_REPORT("Error attaching to shared memory for darks.");
      attached_to_shmem = false;
      delete sis;
      delete dark_sis;
      sis = 0;
      dark_sis  = 0;
      return -1;
   }
   
   
   attached_to_shmem = true;
   return 0;
}



int coronguide::Run()
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
    
   attach_status_boards();
   LOG_INFO("starting up . . .");
   
   while((!sbconn || !attached_to_shmem) && !TimeToDie)
   {
      sleep(1);
      
      if(!isb || !aosb)  attach_status_boards();
      
      if(attached_to_shmem == false)
      {
         connect_shmem();
      }
   }
   
   while(!TimeToDie)
   {
      //usleep(50000);
      sleep(1);
      write_frame();
      
   }
   
   return 0;
}

int coronguide::write_frame()
{
   int curr_image;
   
   static int last_image = -1;
   
   static double autopause_time = 0;
      
   double background = 0.;
   double imv;
    
   
   size_t xdim;
   size_t ydim;
   
   curr_image = sis->get_last_image();

   if(curr_image == last_image || curr_image < 0) return -1;

   sim = sis->get_image(curr_image);
   last_image = curr_image;
   
   int darkcurrim = dark_sis->get_last_image();
   dark_sim = dark_sis->get_image(darkcurrim);
      
   double totl = 0;
   double tmp_xcen = 0;
   double tmp_ycen = 0;
   
   size_t x0 = isb->guidebox_x0;
   size_t x1 = isb->guidebox_x1;
   size_t y0 = isb->guidebox_y0;
   size_t y1 = isb->guidebox_y1;
   size_t idx;
   size_t nx = sim.nx;
   
   pthread_mutex_lock(&memmutex);
   
   //If different, need to reallocate
   if(x0 != last_x0 || x1 != last_x1 || y0 != last_y0 || y1 != last_y1)
   {
      if(tempIm) delete tempIm;
      if(backgroundIm) delete backgroundIm;
      
      tempIm = 0;
      backgroundIm = 0;
      
      open_loop();
      
      inAvg = 0;
      avgidx = 0;
   }
   
   last_x0 = x0;
   last_x1 = x1;
   last_y0 = y0;
   last_y1 = y1;
   
   
   //If box inverted, fix.
   if(x1 < x0)
   {
      double dtmp = x1;
      x1 = x0;
      x0 = dtmp;
   }
   
   if(y1 < y0)
   {
      double dtmp = y1;
      y1 = y0;
      y0 = dtmp;
   }

   if(x0 >= nx or y0 >= nx or x1 >= nx or y1 >= nx or x0 == x1 or y0 == y1) 
   {
      pthread_mutex_unlock(&memmutex);
      return 0;
   }
   
   xdim = x1 - x0;
   ydim = y1 - y0;
   
   //If necessary, allocate tempIm
   if(!tempIm) tempIm = new double[xdim*ydim];
   
   //If necessary, allocate and initialize backgroundIm
   if(!backgroundIm)
   {
      backgroundIm = new double[xdim*ydim];
      
      for(size_t i=0;i<xdim; i++)
      {
         for(size_t j=0;j< ydim; j++)
         {
            backgroundIm[j*xdim + i] = 0.;
         }
      }
   }

   
   /******************/
   //copy to tempIm
   /*****************/
   for(size_t i=x0;i<x1;i++)
   {
      for(size_t j=y0;j<y1;j++)
      {
         idx = j*nx + i;
         tempIm[(j-y0)*xdim+(i-x0)] = sim.imdata[idx];
      }
   }
      
      
   /******************/
   //Median subtraction
   /*****************/
   if(bgAlgorithm == MEDSUB)
   {
      size_t cnt = 0;
      std::vector<double> medvect((y1-y0)*(x1-x0));
   
      for(size_t i=0;i<xdim;i++)
      {
         for(size_t j=0;j<ydim;j++)
         {                
            imv = tempIm[j*xdim + i]; //sim.imdata[idx]; //-dark_sim.imdata[idx];
            
            if(isnan(imv) || imv < 0 || imv > 16383) continue;
            medvect[cnt] = imv;
         
            cnt++;
         }
      }
   
      std::sort(medvect.begin(), medvect.end());
      int meddx = 0.5*cnt;
      background = medvect[meddx];
   }
   
   /******************/
   //Unsharp mask
   /*****************/
   if(bgAlgorithm == UNSHARP)
   {
      if(!kernel)
      {
         kdim = ksize*kfwhm;
         kernel = new double[kdim*kdim];
         gaussKernel(kernel, kdim, kdim, kfwhm/GSIG2FW);
      }

      //std::cerr << "usm-ing\n";
      applyKernel(backgroundIm, tempIm, xdim, ydim, kernel, kdim, kdim);
      background = 0;
   }
      
      
   totl = 0;
   for(size_t i=0;i<xdim;i++)
   {
      for(size_t j=0;j<ydim;j++)
      {
         idx = j*xdim+i;
         imv = tempIm[idx] - backgroundIm[idx] - background; //dark_sim.imdata[idx] - med;
         
         if(bgAlgorithm == UNSHARP)
         {
            if(imv < 0) imv = 0;
         }
         //if(isnan(imv) || imv < 0 || imv > 16383) continue;
         if(isnan(imv)) continue;
         
         totl += imv;
         
         tmp_xcen += imv*i;
         tmp_ycen += imv*j;
      
      }
   }
   
   pthread_mutex_unlock(&memmutex);
   
   xcen = tmp_xcen/totl;
   ycen = tmp_ycen/totl;
   
   //oooh, don't add to avg if paused, autopause, etc., only closed or open.
   
   if(loopState==0 or loopState==1)
   {
      xcens[avgidx] = tmp_xcen/totl;
      ycens[avgidx] = tmp_ycen/totl;
      avgidx++;
      inAvg++;
      gimbCounter++;
      
      if(avgidx >= avgLen) avgidx = 0;
      if(inAvg > avgLen) inAvg = avgLen;
   
      if(inAvg >= avgLen)
      {
         double totx = 0, minx = xcens[0], maxx = xcens[0];
         double toty = 0, miny = ycens[0], inAvg = ycens[0];
      
         for(int q=0; q < avgLen; q++)
         {
            totx += xcens[q];
            if(xcens[q] < minx) minx = xcens[q];
            if(xcens[q] > maxx) maxx = xcens[q];
            
            toty += ycens[q];
            if(ycens[q] < miny) miny = ycens[q];
            if(ycens[q] > inAvg) inAvg = ycens[q];
         }
      
         if(minMaxReject && avgLen > 2)
         {
            xcen_avg = (totx - minx - maxx)/(avgLen-2);
            ycen_avg = (toty - miny - inAvg)/(avgLen-2);
         }
         else
         {
            xcen_avg = totx/avgLen;
            ycen_avg = toty/avgLen;
         }
      }
      else
      {
         xcen_avg = NOAVG;
         ycen_avg = NOAVG;
      }
   }
   
   std::cout << xcen << " " << ycen << " " << xcen_avg << " " << ycen_avg <<  std::endl;
   
   /*if(bgAlgorithm == UNSHARP)
   {
      for(size_t i=0;i<xdim;i++)
      {
         for(size_t j=0;j<ydim;j++)
         {
            idx = j*xdim+i;
            std::cout << tempIm[idx] <<  " " << backgroundIm[idx] << "\n";     
         }
      }
}*/
   
   struct timeval tv;
   
   gettimeofday(&tv,0);
   
   if(loopState > 0)
   {    
    
      if(aosb->loop_on != 1 || aosb->nodInProgress || ssb->sw_state != 1)
      {
         if(loopState == 1) 
         {
            loopState = 3;
            dataLogger(tv);
         }
      }
      else if(loopState == 3)
      {
         if(!aosb->nodInProgress && ssb->sw_state == 1 && aosb->loop_on == 1) 
         {
            autopause_time = get_curr_time();
            loopState = 4;
            dataLogger(tv);
         }
      }
      else if(loopState == 4)
      {
         if(get_curr_time() - autopause_time > 1.5 + 1./ccdsb->framerate)
         {
            dataLogger(tv);
            loopState = 1;
         }
      }
      else if(loopState == 1)
      {  
         double gx, gy;
      
         if(useAvg)
         {    
            gx = -1.*(xcen_avg-tgtx)/gimbscale*loop_gain;
            gy = (ycen_avg-tgty)/gimbscale*loop_gain;
            if(gimbCounter > avgLen) move_gimbal(gx, gy);
         }
         else
         {
            gx = -1.*(xcen-tgtx)/gimbscale*loop_gain;
            gy = (ycen-tgty)/gimbscale*loop_gain;
            move_gimbal(gx, gy);
         }
      
         
         dataLogger(tv);
      }
   }

   return 0;
}

int coronguide::move_gimbal(double gx, double gy)
{
   char str[256];
   std::string resp;
   
   //std::cout << "Gimb dx: " << gx << " Gimb dy: " << gy << "\n";
   
   //std::cout << "Taking Script control" << std::endl;
   
   std::cout << "moving gimbal: " << gx << " " << gy << std::endl;
   write_fifo_channel(0, "SCRIPT", 7, &resp);
   
   //std::cout << "Applying x diff" << std::endl;
   
   snprintf(str, 256, "xrel %f", gx);
   write_fifo_channel(0, str, strlen(str)+1, &resp);
   
   //std::cout << "Applying y diff" << std::endl;
   
   snprintf(str, 256, "yrel %f", gy);
   write_fifo_channel(0, str, strlen(str)+1, &resp);
   
   gimbCounter = 0;
   
   return 0;
}


int coronguide::attach_status_boards()
{
   size_t sz;
   
   if(!isb)
   {
      isb =  (VisAO::imviewer_status_board*) attach_shm(&sz, STATUS_imviewer, 0);              
   }
   
   if(!aosb)
   {
      aosb =  (VisAO::aosystem_status_board*) attach_shm(&sz, STATUS_aosystem, 0);              
   }
   
   if(!ssb)
   {
      ssb =  (VisAO::shutter_status_board*) attach_shm(&sz, STATUS_shutterctrl, 0);              
   }
   
   if(!ccdsb)
   {
      ccdsb =  (VisAO::ccd47_status_board*) attach_shm(&sz, STATUS_ccd47, 0);              
   }
   
   if(isb && aosb && ssb && ccdsb) sbconn = 1;
   
   return 0;
}

std::string  coronguide::remote_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received remote command: %s.", com.c_str());
   resp = common_command(com, CMODE_REMOTE);
   if(resp == "") resp = (std::string("UNKOWN COMMAND: ") + com + "\n");
   _logger->log(Logger::LOG_LEV_TRACE, "Response to remote command: %s.", resp.c_str());
   return resp;
}

std::string coronguide::local_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received local command: %s.", com.c_str());
   resp = common_command(com, CMODE_LOCAL);
   if(resp == "") resp = (std::string("UNKOWN COMMAND: ") + com + "\n");
   _logger->log(Logger::LOG_LEV_TRACE, "Response to local command: %s.", resp.c_str());
   return resp;
}

std::string  coronguide::script_command(std::string com)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received script command: %s.", com.c_str());
   resp = common_command(com, CMODE_SCRIPT);
   if(resp == "") resp = (std::string("UNKOWN COMMAND: ") + com + "\n");
   _logger->log(Logger::LOG_LEV_TRACE, "Response to script command: %s.", resp.c_str());
   return resp;
}

std::string coronguide::auto_command(std::string com, char *seqmsg)
{
   std::string resp;
   _logger->log(Logger::LOG_LEV_TRACE, "Received auto command: %s.", com.c_str());
   resp = common_command(com, CMODE_AUTO);
   seqmsg = 0; //just to avoid the warning
   if(resp == "") resp = post_auto_command(com);
   _logger->log(Logger::LOG_LEV_TRACE, "Response to auto command: %s.", resp.c_str());
   return resp;
}

std::string coronguide::common_command(std::string com, int cmode)
{  
   char tmpstr[256];
   
   if(com == "state?")
   {
      return get_state_str();
   }
   
   if(com == "loopstate?")
   {
      snprintf(tmpstr, 256, "%i\n", loopState);
      return tmpstr;
   }
      
   if(com == "loopgain?")
   {
      snprintf(tmpstr, 256, "%0.4f\n", loop_gain);
      return tmpstr;
   }
   
   if(com == "tgtx?")
   {
      snprintf(tmpstr, 256, "%0.4f\n", tgtx);
      return tmpstr;
   }
   
   if(com == "tgty?")
   {
      snprintf(tmpstr, 256, "%0.4f\n", tgty);
      return tmpstr;
   }
   
   if(com == "xcen?")
   {
      snprintf(tmpstr, 256, "%0.4f\n", xcen);
      return tmpstr;
   }
   
   if(com == "ycen?")
   {
      snprintf(tmpstr, 256, "%0.4f\n", ycen);
      return tmpstr;
   }
   
   if(com == "open")
   {
      if(cmode == control_mode)
      {
         //std::cout << "Opening the loop\n";
         open_loop();
         return "0\n";
      }
      else
      {
         return control_mode_response();
      }
   }
   
   if(com == "close" && cmode == control_mode)
   {
      if(cmode == control_mode)
      {
         //std::cout << "Closing the loop\n";
         close_loop();
         return "0\n";
      }
      else
      {
         return control_mode_response();
      }
   }
   
   if(com == "pause" && cmode == control_mode)
   {
      if(cmode == control_mode)
      {
         //std::cout << "Pausing the loop\n";
         pause_loop();
         return "0\n";
      }
      else
      {
         return control_mode_response();
      }
   }
   
   if(com.length() > 4)
   {
      if(com.substr(0, 4)== "gain")
      {
         if(cmode == control_mode)
         {
            std::string newgain = com.substr(5, com.length()-5);
            set_loop_gain(strtod(newgain.c_str(),0));
            return "0\n";
         }
         else
         {
            return control_mode_response();
         }
      }
   }
   
   if(com.length() > 6)
   {
      if(com.substr(0, 6)== "bgalgo")
      {
         if(cmode == control_mode)
         {
            std::string newalgo = com.substr(7, com.length()-7);
            set_bgalgo(atoi(newalgo.c_str()));
            return "0\n";
         }
         else
         {
            return control_mode_response();
         }
      }
   }
   
   if(com.length() > 5)
   {
      if(com.substr(0, 5)== "kfwhm")
      {
         if(cmode == control_mode)
         {
            std::string newkfwhm = com.substr(6, com.length()-6);
            set_kfwhm(strtod(newkfwhm.c_str(), 0));
            return "0\n";
         }
         else
         {
            return control_mode_response();
         }
      }
   }
   
   if(com.length() > 5)
   {
      if(com.substr(0, 5)== "ksize")
      {
         if(cmode == control_mode)
         {
            std::string newksize = com.substr(6, com.length()-6);
            set_ksize(atoi(newksize.c_str()));
            return "0\n";
         }
         else
         {
            return control_mode_response();
         }
      }
   }
   
   if(com.length() > 6)
   {
      if(com.substr(0, 6)== "useavg")
      {
         if(cmode == control_mode)
         {
            std::string newua = com.substr(7, com.length()-7);
            setUseAvg(atoi(newua.c_str()));
            return "0\n";
         }
         else
         {
            return control_mode_response();
         }
      }
   }
   
   if(com.length() > 5)
   {
      if(com.substr(0, 5)== "mmrej")
      {
         if(cmode == control_mode)
         {
            std::string mmrej = com.substr(6, com.length()-6);
            setMinMaxReject(atoi(mmrej.c_str()));
            return "0\n";
         }
         else
         {
            return control_mode_response();
         }
      }
   }
   
   if(com.length() > 6)
   {
      if(com.substr(0, 6)== "avglen")
      {
         if(cmode == control_mode)
         {
            std::string avglen = com.substr(7, com.length()-7);
            setAvgLen(atoi(avglen.c_str()));
            return "0\n";
         }
         else
         {
            return control_mode_response();
         }
      }
   }
   
   return "";
}

std::string coronguide::get_state_str()
{
   std::string cmstr;
   char str[256];
   
   cmstr= control_mode_response();
   
   snprintf(str, 256, "%c %i %0.4f %0.4f %0.4f %0.4f %0.4f %i %0.4f %i %i %i %i\n", cmstr[0], loopState, loop_gain, tgtx, tgty, xcen, ycen, bgAlgorithm, kfwhm, ksize, useAvg, minMaxReject, avgLen);
   
   return str;
}

   
   
int coronguide::update_statusboard()
{
   if(statusboard_shmemptr)
   {
      VisAOApp_base::update_statusboard();
     
      coronguide_status_board * cgsb = (coronguide_status_board *) statusboard_shmemptr;
      
      cgsb->loopState = loopState;
      cgsb->loop_gain = loop_gain;
      cgsb->tgtx = tgtx;
      cgsb->tgty = tgty;
      cgsb->xcen = xcen;
      cgsb->ycen = ycen;
      
   }
   
   return 0;

}

void coronguide::dataLogger(timeval tv)
{
   checkDataFileOpen();

   
   dataof << tv.tv_sec << " " << tv.tv_usec << " " << loopState << " " << loop_gain << " " << tgtx << " " << tgty << " " << xcen << " " << ycen << std::endl;

   if(!dataof.good())
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "Error in coronguide data file.  Data may not be logged correctly");
   }

}

} //namespace VisAO

