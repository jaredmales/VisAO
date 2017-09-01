/************************************************************
 *    fir_filter.cpp
 *
 * Author: Jared R. Males (jrmales@as.arizona.edu)
 *
 * FIR digital filter class definitions.
 *
 ************************************************************/

/** \file fir_filter.cpp
 * \author Jared R. Males
 * \brief FIR digital filter class definitions.
 *
 */

#include "fir_filter.h"

namespace VisAO
{
   
fir_filter::fir_filter()
{
   order = 0;
   coef = 0;
   gain = 0;
}

fir_filter::~fir_filter()
{
   if(coef) delete[] coef;
}

void fir_filter::set_coef(int ord, float *nc, float gn)
{
   if(coef) delete[] coef;

   coef = new float[ord];

   order = ord;

   for(int i=0;i<order;i++) coef[i] = nc[i];

   gain = gn;

}

int fir_filter::read_coef_file(std::string fname)
{
   std::ifstream fin;

   fin.open(fname.c_str());

   fin >> order;

   if(coef) delete[] coef;

   coef = new float[order];

   for(int i=0;i<order;i++) fin >> coef[i];

   fin >> gain;

   fin.close();

   return 0;
}

void fir_filter::print_filter()
{
   std::cout << "N: " << order << "\n";
   for(int i=0;i<order;i++) std::cout << coef[i] << " ";
   std::cout << "\nG: " << gain << "\n";
}

float fir_filter::apply_filter(float *data, int len)
{
   float sum = 0.0;

   if(order == 0) return data[len-1];
   
   for(int i = len-1, j=0; i >= 0 && j < order; i--, j++)
   {
      sum += coef[j]*data[i];
   }

   return sum;
}

} //namespace VisAO

