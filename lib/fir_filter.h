/************************************************************
 *    fir_filter.h
 *
 * Author: Jared R. Males (jrmales@as.arizona.edu)
 *
 * FIR digital filter class declarations.
 *
 ************************************************************/

/** \file fir_filter.h
 * \author Jared R. Males
 * \brief FIR digital filter class declarations.
 *
 */

#ifndef __fir_filter_h__
#define __fir_filter_h__

#include <string>
#include <iostream>
#include <fstream>

namespace VisAO
{

/** \todo fir_filter: need error checking in read_coef_file
  */
class fir_filter
{
   public:
      fir_filter();
      ~fir_filter();
      
   protected:

      int order;
      float *coef;
      float gain;

   public:

      void set_coef(int ord, float *nc, float gain);

      int read_coef_file(std::string fname);

      int get_order(){ return order;}
      
      void print_filter();
      
      ///Apply the filter to the data fector of length len.
      /** The newest point should be data[len-1].  Applies at most order coefficients.  
       */
      float apply_filter(float *data, int len);
};

} //namespace VisAO

#endif

