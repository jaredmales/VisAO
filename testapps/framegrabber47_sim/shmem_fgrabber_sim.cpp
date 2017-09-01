
#include <fitsio.h>

#include "sharedim_stack.h"

#include "fifoutils.h"

int get_fits_im(short *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, char *fname)
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

fifo_list * global_fifo_list = 0;

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
	
void error_report(const char* er, const char* file, int lno) 
{
	//Logger::get()->log(Logger::LOG_LEV_ERROR, "%s.  File: %s  Line: %i", er, file, lno);
	std::cerr << er << " File: " << file << " Line: " << lno << std::endl;
}


void log_info(const char* li) 
{
	//Logger::get()->log(Logger::LOG_LEV_INFO, "%s.", li);
	std::cout << li << "\n";
}


set_global_error_report(&error_report);
set_global_log_info(&log_info);
	
	
	
int main(int argc, char *argv[])
{
	static int called = 0;
	fitsfile * fptr1 = 0, *fptr2 =0, *fptr3=0, *fptr4=0;
	timeval tv, tv0, tv1;
	long fpixel[2] = {1,1};;
   long lpixel[2] = {1024,1024};
	short **im1024;
	short **im128;
	
   int ping_enabled;
   
	signal(SIGIO, SIG_IGN);
	
	fifo_list fl;
	
	im1024 = new short*[4];
	for(int i=0;i<4;i++) im1024[i] = new short[1024*1024];	  
	  
	get_fits_im(im1024[0], 1, fpixel, lpixel, &fptr1, "data/1024x1024/im1.fits");
	get_fits_im(im1024[1], 1, fpixel, lpixel, &fptr2, "data/1024x1024/im2.fits");
	get_fits_im(im1024[2], 1, fpixel, lpixel, &fptr3, "data/1024x1024/im3.fits");
	get_fits_im(im1024[3], 1, fpixel, lpixel, &fptr4, "data/1024x1024/im4.fits");
	
	im128 = new short*[30];
	load_128x128(im128);
	
	sharedim_stack sis;
	sharedim * sim;
	
	
	sis.create_shm(5000, 250, 300*(sizeof(sharedim) + 1024*1024*sizeof(short)));
   struct timespec sleeptime;
	sleeptime.tv_sec = 0;
	
	init_fifo_list(&fl);
	setup_fifo_list(&fl, 1);
	//set_fifo_list_channel(&fl, 0, RWBUFF_SZ, "/home/aosup/visao/fifos/framewriter47_ping_out", "/home/aosup/visao/fifos/framewriter47_ping_in", 0, 0);
	set_fifo_list_channel(&fl, 0, RWBUFF_SZ, "/home/jaredmales/Source/Magellan/visao_base/fifos/framewriter47_ping_out", "/home/jaredmales/Source/Magellan/visao_base/fifos/framewriter47_ping_in", 0, 0);
	connect_fifo_list_nolock(&fl);
   if(fl.fifo_ch[0].fd_out > 0) ping_enabled = 1;
   else ping_enabled = 0;
   
	int k = 0, kmax=4;
	
	int imsz, nims;
	short **ims;
	

   for(int i =15;i<29;i++) im128[i][128*64+64] = 17000;
	
/*	ims = im128;
	imsz = 128;
	nims = 30;/**/
	ims = im1024;
	imsz = 1024;
	nims=4;/**/
	double frametime = 0.99e9;
	//while(1)
   for(int qq=0;qq<100000;qq++)
	//for(int qq=0;qq<2; qq++)
	{
      //std::cout << "1\n";
		for(int k=0; k<nims;k++)
		{
			//std::cout << sis.get_last_image() << "\n";
			sim = sis.set_next_image(imsz,imsz);
			//std::cout << sis.get_last_image() << "\n";
			sim->depth = 14;
			for(int i=0; i<imsz; i++)
			{
				for(int j=0; j<imsz; j++)
				{
					sim->imdata[i*imsz+j] = (short)(ims[k][j*imsz+i]);
				}
			}
			gettimeofday(&tv, 0);
			sim->frame_time = tv;
			sis.enable_next_image();
			std::cout << sis.get_last_image() << "\n";
	 		if(ping_enabled) write_fifo_channel(&fl.fifo_ch[0], "1", 1);
				
			if(!called) 
			{
				gettimeofday(&tv0,0);
				called = 1;
			}
			//else called++;
			//if(called > 20) pause();
			gettimeofday(&tv1,0);
			double dt = ((double)tv1.tv_sec + ((double)tv1.tv_usec)/1e6)-((double)tv0.tv_sec + ((double)tv0.tv_usec)/1e6);
			//std::cout << tv1.tv_sec + tv1.tv_usec << " " << tv0.tv_sec << " " << tv0.tv_usec << " " << dt << "\n";
			sleeptime.tv_nsec = frametime-dt*1e9;
			if(sleeptime.tv_nsec < 0) 
			{
				sleeptime.tv_nsec = 0;
				std::cerr << "Too fast!\n";
			}
			nanosleep(&sleeptime, 0);
			gettimeofday(&tv0,0);
			//tv0 = tv1;
		}
		if(imsz == 128)//1024)
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
	
	
}