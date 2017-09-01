/************************************************************
 *    visaoimutils.h
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Declarations for various image utility functions.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file visaoimutils.h
 * \author Jared R. Males
 * \brief Declarations for various image utility functions.  
 * 
 */

#ifndef __VISAOIMUTILS_H__
#define __VISAOIMUTILS_H__

#include <stdio.h>
#include <fitsio.h>
#include <sys/time.h>
#include <time.h>

#include "libvisao.h"
#include "sharedim_stack.h"

#include "statusboard.h"
#include "AOStates.h"

#define INT_2BYTES short

typedef struct
{
   std::string origin;
   std::string telescop;
   std::string instrume;
   
   timeval dateobs;
   timeval timestamp_dma;
   
   long frameNo;
   
   VisAO::focusstage_status_board * fsb;
   VisAO::ccd47_status_board * csb;
   VisAO::shutter_status_board * ssb;
   VisAO::filterwheel_status_board *fw2sb;
   VisAO::filterwheel_status_board *fw3sb;
   VisAO::wollaston_status_board *wsb;
   VisAO::aosystem_status_board *aosb;
   VisAO::gimbal_status_board *gsb;
   VisAO::system_status_board *vssb;
   VisAO::reconstructor_status_board *rsb;
   
   VisAO::shutter_tester_status_board *stsb;
   
   VisAO::zaber_status_board *zsb;
   VisAO::hwp_status_board * hwpsb;
   
} visao_imheader;

void init_visao_imheader(visao_imheader * imhead);


///Writes the standard visao unique filename for a timeval to the buffer.  
/** YYYYMODDHHMMSSUUUUUU where
  * YYYY = the 4 digit year
  * MO = the 2 digit month
  * DD = the 2 digit day
  * HH = the 2 digit hour
  * MM = the 2 digit minut
  * SS = the 2 digit second
  * UUUUUU = the 6 digit microsecond
  * \param buffer pointer to be written to, must be allocated to at least 21
  * \param tv is the time to use to construct the filename.
  * \retval 0 on success.
  * \retval -1 on failure in gmtime_r.
  */ 
int get_visao_filename(char * buffer, struct timeval *tv);

///Writes the standard visao unique filename for a timespec to the buffer.  
/** YYYYMODDHHMMSSUUUUUU where
  * YYYY = the 4 digit year
  * MO = the 2 digit month
  * DD = the 2 digit day
  * HH = the 2 digit hour
  * MM = the 2 digit minut
  * SS = the 2 digit second
  * UUUUUU = the 6 digit microsecond
  * \param buffer pointer to be written to, must be allocated to at least 21
  * \param ts is the time to use to construct the filename.
  * \retval 0 on success.
  * \retval -1 on failure in gmtime_r.
  */ 
int get_visao_filename(char * buffer, struct timespec *ts);


template <class dataT> int visao_fits_create_img( fitsfile *fptr, int naxis, long *naxes, int *status);
template <class dataT> int visao_fits_write_subset(fitsfile *fptr, long *fpixel, long *lpixel,
                                                   dataT *array,  int *status);

///Write a fits file.
template <class dataT> int write_visao_fits(const char *foutname, int nx, int ny, dataT *im, visao_imheader *head, bool aohead, bool visaohead);

///Write a fits file, using the sharedim structure.
template <class dataT> int write_visao_fits(const char * name_base, sharedim<dataT> *sim, visao_imheader * head, bool aohead=true, bool visaohead=true);

int write_visao_raw(const char * name_base, sharedimS *sim, visao_imheader * head, bool aohead=true, bool visaohead=true);


int write_visao_fits_aosys_header(fitsfile * outfptr, visao_imheader * head);
int write_visao_fits_visao_header(fitsfile * outfptr, visao_imheader * head);


template <class dataT> int write_visao_fits(const char *foutname, int nx, int ny, dataT *im, visao_imheader * head, bool aohead, bool visaohead)
{
   fitsfile *outfptr;   /* FITS file pointers defined in fitsio.h */
   int status = 0, i;       /* status must always be initialized = 0  */
   long naxes[2], lpix[2],fpix[2];//, imlong[1024*1024];
   char hfld[81], tmp[30], datestr[80];
   struct tm *dateobs_tm;
   int timeref;
   
   //fits_get_system_date(&day, &month, &year, &status );
   fits_get_system_time(datestr, &timeref, &status);
   
   //im = (INT_2BYTES *) malloc(nx*ny*2);
   
   /* Create the output file */
   if ( !fits_create_file(&outfptr, foutname, &status) )
   {
      naxes[0] = nx;
      naxes[1] = ny;
      
      if(!visao_fits_create_img<dataT>(outfptr,2, naxes, &status))
      {
         fpix[0] = 1;
         fpix[1] = 1;
         lpix[0] = nx;
         lpix[1] = ny;
         visao_fits_write_subset<dataT>(outfptr,fpix, lpix, im, &status);
         if(!status)
         {
            for(i=0;i<81;i++) hfld[i] = ' ';
            fits_update_key(outfptr, TSTRING, "DATE", (void *)datestr,"Date this file was written YYYY-mm-dd",  &status);
            if(head)
            {
               fits_update_key(outfptr, TSTRING, "ORIGIN", (void *)head->origin.c_str(),0,  &status);
               fits_update_key(outfptr, TSTRING, "TELESCOP", (void *)head->telescop.c_str(),0,  &status);
               fits_update_key(outfptr, TSTRING, "INSTRUME", (void *)head->instrume.c_str(),0,  &status);
               
               dateobs_tm = gmtime(&head->dateobs.tv_sec);
               strftime(tmp, 30, "%Y-%m-%dT%H:%M:%S", dateobs_tm);
               sprintf(datestr, "%s.%06u", tmp, (unsigned) head->dateobs.tv_usec);
               //yyyy-mm-ddTHH:MM:SS[.sss]
               fits_update_key(outfptr, TSTRING, "DATE-OBS", (void *)datestr, "Date of obs. YYYY-mm-ddTHH:MM:SS",  &status);

               if(head->frameNo > -1) fits_update_key(outfptr, TLONG, "FRAMENO", (void *)&head->frameNo, "Camera frame counter",  &status);
               
               if(aohead) status = write_visao_fits_aosys_header(outfptr, head);
               if(visaohead) status = write_visao_fits_visao_header(outfptr, head);
            }
         }
      }
      
      fits_close_file(outfptr,  &status);
   }

   /* if error occured, print out error message */
   if (status) fits_report_error(stderr, status);
   return(status);
   
}

template <class dataT> int write_visao_fits(const char * name_base, sharedim<dataT> *sim, visao_imheader * head, bool aohead, bool visaohead)
{
   std::string fname;
   char fext[21];
   
   fname = name_base;
   get_visao_filename(fext, &sim->frame_time);
   
   fname += fext;
   fname += ".fits";
   
   if(head)
   {
      head->dateobs.tv_sec = sim->frame_time.tv_sec;
      head->dateobs.tv_usec = sim->frame_time.tv_usec;
      head->timestamp_dma.tv_sec = sim->frame_time_dma.tv_sec;
      head->timestamp_dma.tv_usec = sim->frame_time_dma.tv_usec;
      
      head->frameNo = sim->frameNo;
   }
   
   return write_visao_fits<dataT>(fname.c_str(), sim->nx, sim->ny, sim->imdata, head, aohead, visaohead);
   
}

int get_fits_im(unsigned char *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname);

int get_fits_im(float *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname);

int get_fits_im(double *im, long hduno, long *fpix, long *lpix, fitsfile **fptr, const char *fname);

#endif //__VISAOIMUTILS_H__

