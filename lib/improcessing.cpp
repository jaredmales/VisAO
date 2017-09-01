
#include "improcessing.h"

double gaussian2D(double dx, double dy, double sigx, double sigy)
{   
   return exp(-0.5*(pow(dx/sigx,2) + pow(dy/sigy,2)));
}


int gaussKernel(double * kernel, size_t dim1, size_t dim2, double sig)
{
   double xcen = ((int) (0.5*(((double)dim1) - 1.) ));
   double ycen = ((int) (0.5*(((double)dim2) - 1.) ));

   
   size_t i, j;
   
   double total = 0;
   for(i=0;i<dim1;i++)
   {
      for(j=0;j<dim2;j++)
      {
         kernel[j*dim1+i] = gaussian2D((double)i-xcen, (double)j-ycen, sig, sig);
         total += kernel[j*dim1+i];
      }
   }
   
   for(i=0;i<dim1;i++)
   {
      for(j=0;j<dim2;j++)
      {
         kernel[j*dim1+i] /= total;
      }
   }
   
}

int applyKernel(double *smim, double *im, size_t dim1, size_t dim2, double *kern, size_t kdim1, size_t kdim2)
{
   double xcen = ((int) (0.5*(((double)kdim1) - 1.)));
   double ycen = ((int) (0.5*(((double)kdim2) - 1.)));
   
   double sum;
   double ktot;
   
   size_t i, j, k, l;
   
   long ix, jx;
   
   for(i =0; i< dim1; i++)
   {
      for(j=0; j<dim2; j++)
      {
         sum = 0;
         ktot = 0;
         
         for(k=0;k<kdim1;k++)
         {
            for(l=0;l<kdim2;l++)
            {
               ix = i - (xcen-k);
               jx = j - (ycen-l);
               
               if( ix >= 0 and (size_t) ix < dim1 and  jx >=0 and (size_t) jx < dim2)
               {
                  sum += kern[l*kdim1+k] * im[jx*dim1 + ix];
                  ktot += kern[l*kdim1+k];
               }
            }
         }
         
         smim[j*dim1+i] = sum/ktot;
      }
   }
   
   return 0;
}     
    
/*    
#include <iostream>
int main()
{
   size_t kdim = 40., dim1 = 37., dim2=52;
   
   double *im, *smim, *kern;
   
   im = new double[dim1*dim2];
   smim = new double[dim1*dim2];
   
   kern = new double[kdim*kdim];
   
   gaussKernel(im, dim1, dim2, 3./GSIG2FW);
   
   gaussKernel(kern, kdim, kdim, 10./GSIG2FW);
   
   applyKernel(smim, im, dim1, dim2, kern, kdim, kdim);
   
   
   double imv, totl = 0;
   int idx;
   double xcen = 0, ycen = 0;
   for(size_t i=0;i<dim1;i++)
   {
      for(size_t j=0;j<dim2;j++)
      {
         idx = j*dim1+i;
         imv = im[idx] - smim[idx]; 
         
         if(isnan(imv)) continue;
         
         totl += imv;
         
         xcen += imv*i;
         ycen += imv*j;
      
      }
   }
   
   xcen /= totl;
   ycen /= totl;
   
   std::cerr << xcen << " " << ycen << "\n";
   
   for(int i=0;i<dim1;i++)
   {
      for(int j=0;j<dim2;j++)
      {
         std::cout << im[j*dim1 + i] << " " << smim[j*dim1 + i] << "\n";
      }
   }
}   
*/

