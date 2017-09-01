/************************************************************
 *    recmat.cpp
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Definitions for a class to manage a reconstructor matrix.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file recmat.cpp
 * \author Jared R. Males
 * \brief Definitions for a class to manage a reconstructor matrix.
 *
 *
 */

#include "recmat.h"

recmat::recmat()
{
   n_modes = 0;
   n_slopes = 0;
   R = 0;
   amp = 0;

   reflection_gain = 2.; //double pass in the tower = 4
   unit_conversion = 1e9; //meters to nm
   
   rec_tech = REC_ATLAS;
   gpu_inited = 0;
}

recmat::recmat(std::string fname)
{
   n_modes = 0;
   n_slopes = 0;
   R = 0;
   amp = 0;

   reflection_gain = 2.; //double pass in the tower
   unit_conversion = 1e9; //meters to nm
   
   rec_tech = REC_ATLAS;
   gpu_inited = 0;
   
   load_recmat_LBT(fname);
}

recmat::~recmat()
{
   if(R) gsl_matrix_float_free(R);

   if(amp) delete amp;

   #ifdef REC_USE_GPU
   if(gpu_inited) free_gpurecon();
   #endif
}


int recmat::load_recmat_LBT(std::string fname)
{
   fitsfile *fptr = 0;
   int fstatus;
   int naxis;
   long naxes[2], fpix[2]={1,1}, lpix[2];
   int f_nmodes, f_nslopes;

   std::cout << fname << std::endl;

   fstatus = 0;
   fits_open_file(&fptr, fname.c_str(), READONLY, &fstatus);
   
   if (fstatus)
   {
      fprintf(stderr, "Error in get_open_file.\n");
      fits_report_error(stderr, fstatus); /* print any error message */
      return -1;
   }

   //If successful, then store the path name.
   filePath = fname;
   
   fits_get_img_dim(fptr, &naxis, &fstatus);
   
   if (fstatus)
   {
      fprintf(stderr, "Error in get_img_dim.\n");
      fits_report_error(stderr, fstatus); /* print any error message */
      return -1;
   }
   //Check naxis == 2 here.
   fits_get_img_size(fptr, naxis, naxes, &fstatus);
   if (fstatus)
   {
      fprintf(stderr, "Error in get_img_size.\n");
      fits_report_error(stderr, fstatus); /* print any error message */
         
      return -1;
   }
   
   f_nmodes = naxes[1];
   f_nslopes = naxes[0];
   
   std::cerr << "Fits image size: " << f_nmodes << " X " << f_nslopes << std::endl;
   
   float *fR = new float[f_nmodes*f_nslopes*2];
   
   double nm;
   fits_read_key(fptr, TDOUBLE, "IM_MODES", &nm, 0, &fstatus);
   
   std::cerr << "Actual number of modes: " << nm << "\n";
   
   n_modes = (int) nm;
   
   //n_deforms = n_modes;
   
   if(amp) delete amp;
   amp = new float[n_modes];
   
   lpix[0] = f_nslopes;
   lpix[1] = f_nmodes;
   
   
   get_fits_im(fR, 1, fpix, lpix, &fptr, 0);

   n_slopes = f_nslopes;
   for(int k = f_nslopes - 1; k > -1; k--)
   {
      if(fR[k] !=0 || fR[1*f_nslopes + k] !=0 || fR[2*f_nslopes + k] !=0)
      {
         n_slopes = k+1;
         break;
      }
   }
   
   std::cerr << "Actual # slopes " << n_slopes << std::endl;
     
   if(R)
   {
      gsl_matrix_float_free(R);
      R = 0;
   }
   R = gsl_matrix_float_alloc(n_modes, n_slopes);
   
   for(int i=0;i<n_modes;i++)
   {
      for(int j=0;j<n_slopes;j++)
      {
         gsl_matrix_float_set(R, i, j, fR[i*f_nslopes + j]);
      }
   }

   #ifdef REC_USE_GPU
   //fR is column major, but is the full size of the fits image, and R->data is the correct size.
   //So we pass R->data, which is in row-major format.
   
   if(gpu_inited) free_gpurecon();
   
   init_gpurecon(n_modes, n_slopes, R->data);
   gpu_inited = 1;
   #endif


   fitting_error_sq = fitting_A * pow(n_modes, fitting_B)*pow(tel_diam, 5./3.)/(4.*3.14159*3.14159)*pow(median_r0_lam*1e3/median_r0, 2.);
   
   std::cerr << "fitting error squared = " << fitting_error_sq << std::endl;
   
   delete fR;
  
   fits_close_file(fptr, &fstatus);
   
   return 0;
}

int recmat::reconstruct(float * slopes)
{
   if(rec_tech == REC_ATLAS)
   {
      //gsl_blas_sgemv(CblasNoTrans, 1.0, rmat.R, &gslSlopes.vector, 0.0, &gslAmp.vector);
      cblas_sgemv(CblasRowMajor, CblasNoTrans, n_modes, n_slopes, 1., R->data, n_slopes, slopes, 1, 0., amp, 1);
   }
   else if(rec_tech == REC_GPU)
   {
      #ifdef REC_USE_GPU
      gpurecon(slopes, amp);
      #else
      std::cerr << "No GPU available.\n";
      #endif
   }

   return 0;
}

int recmat::calc_sumvar(float *sumvar, int m0, int m1, bool fiterr)
{
   *sumvar = 0.0;
   float fact = reflection_gain*reflection_gain*unit_conversion*unit_conversion;
   
   if(m1 < 0) m1 = n_modes;
   
   for(int j=m0;j<m1; ++j)
   {
      *sumvar +=  fact*amp[j]*amp[j];
   }

   if(fiterr) *sumvar += fitting_error_sq;
   
   return 0;
}
   
/*
int main()
{
   recmat rm("bdata/SRTS_wdisturb/Rec_20111125_165625.fits");
}*/

