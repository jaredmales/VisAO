
#ifndef __improcessing_h__
#define __improcessing_h__

#include <cstdlib>
#include <cmath>

#define GSIG2FW  2.3548200450309493

double gaussian2D(double dx, double dy, double sigx, double sigy);

int gaussKernel(double * kernel, size_t dim1, size_t dim2, double sig);

int applyKernel(double *smim, double *im, size_t dim1, size_t dim2, double *kern, size_t kdim1, size_t kdim2);

      


#endif //__improcessing_h__
