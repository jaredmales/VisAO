//Solves A·X = B for a vector X, where A is specified by the arrays u[1..m][1..n], w[1..n],
//v[1..n][1..n] as returned by svdcmp. m and n are the dimensions of a, and will be equal for
//square matrices. b[1..m] is the input right-hand side. x[1..n] is the output solution vector.
//No input quantities are destroyed, so the routine may be called sequentially with different b’s.

#include <stdlib.h>

void preconditionU(double **u, double *w, int m, int n);
void preconditionU_F(float **u, float *w, int m, int n);

void backsub_precond(double **u, double **v, int m, int n, double *b, double *x, double *_tmp);
void backsub_precond_F(float **u, float **v, int m, int n, float *b, float *x, float *_tmp);

void backsub_precond_F_threads(float **u, float **v, int m, int n, float *b, float *x, float *tmp, int nthreads);
