/************************************************************
 *    recmat.h
 *
 * Author: Jared R. Males (jrmales@email.arizona.edu)
 *
 * Declarations for a class to manage a reconstructor matrix.
 *
 * Developed as part of the Magellan Adaptive Optics system.
 ************************************************************/

/** \file recmat.h
 * \author Jared R. Males
 * \brief Declarations for a class to manage a reconstructor matrix.
 *
 *
 */

#ifndef __recmat_h__
#define __recmat_h__

#include <string>
#include <iostream>

#include <math.h>

#include "svbksb.h"
#include "gsl/gsl_linalg.h"
#include "gsl/gsl_blas.h"

#include "fitsio.h"
#include "visaoimutils.h"

#ifdef REC_USE_GPU
#include "gpurecon.h"
#endif

#define REC_ATLAS 0
#define REC_GPU   1

class recmat
{
   public:
      recmat();
      recmat(std::string fname);

      ~recmat();

   //protected:
   public:
      std::string filePath; ///<The full path to the reconstructor matrix
      std::string fileName; ///<file name of the reconstructor matrix
      
      int gpu_inited; ///<Flag for whether the gpu globals are initialized.

      int n_modes; ///<The number of modes, and the rows in the reconstructor matrix.
      //int n_deforms;
      int n_slopes; ///<The number of slopes, and the columns in the reconstructor matrix

     
      
      float reflection_gain; ///< Factor to apply to reconstructed amplitudes to account for mirror reflection
      float unit_conversion; ///< Factor to convert variances to nanometers.
      
      int rec_tech; ///<Which reconstructor technique to use, either REC_ATLAS or REC_CPU
      
      gsl_matrix_float *R;
      
      float *amp; ///<The reconstructed amplitudes, a vector of length n_modes

      double tel_diam;
      double median_r0;     
      double median_r0_lam;  
      double fitting_A;      
      double fitting_B;     
       
      double fitting_error_sq;
            
   public:
      std::string get_filePath(){return filePath;}
      
      int get_n_modes(){return n_modes;}
      int get_n_slopes(){return n_slopes;}

      int get_rec_tech(){return rec_tech;}
      
      int load_recmat_LBT(std::string fname);

      ///Given a new slopes vector, reconstruct the amplitudes
      /** Upon exit the amp vector will be populated with the reconstructed modal amplitudes.
        */
      int reconstruct(float *slopes);

      ///Calculate the sum of the variances, e.g. the dot product of the amplitude vector.
      /** Does not include the first 2 modes, since they are tip and tilt.  This corresponds to the instantaneous Strehl ratio.
       */
      int calc_sumvar(float *sumvar, int m0 = 2, int m1 = -1, bool fiterr=true);
      
      
};


#endif //__recmat_h__

