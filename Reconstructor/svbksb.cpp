#include "svbksb.h"
#include <pthread.h>

void preconditionU(double **u, double *w, int m, int n)
{
   for(int j =0; j<n; j++)
   {
      for(int i=0;i<m;i++)
      {
         if(w[j]) u[i][j]/= w[j];
         else u[i][j] = 0;
      }
   }

}

void preconditionU_F(float **u, float *w, int m, int n)
{
   for(int j =0; j<n; j++)
   {
      for(int i=0;i<m;i++)
      {
         if(w[j]) u[i][j]/= w[j];
         else u[i][j] = 0;
      }
   }
   
}

void backsub_precond(double **u, double **v, int m, int n, double *b, double *x, double *tmp)
{
   int jj,j,i;
   
   double *vrow, *urow,  _b;

   urow = u[0];
   _b = b[0];

   for(j=0;j<n; j++)
   {
      tmp[j] = urow[j]*_b;
   }

   for(i=1;i<m; i++)
   {
      urow = u[i];
      _b = b[i];

      for(j=0;j<n; j++)
      {
         tmp[j] += urow[j]*_b;
      }
   }

   vrow = v[0];
   for (j=0;j<n;j++)
   {
      x[j] = vrow[0]*tmp[0];
   }
   
   for (j=0;j<n;j++)
   {
      vrow = v[j];
      for (jj=1;jj<n;jj++) x[j] += vrow[jj]*tmp[jj];
   }
}

void backsub_precond_F(float **u, float **v, int m, int n, float *b, float *x, float *tmp)
{
   int jj,j,i;
   
   float *vrow, *urow,  _b;
   
   urow = u[0];
   _b = b[0];
   
   for(j=0;j<n; j++)
   {
      tmp[j] = urow[j]*_b;
   }
   
   for(i=1;i<m; i++)
   {
      urow = u[i];
      _b = b[i];
      
      for(j=0;j<n; j++)
      {
         tmp[j] += urow[j]*_b;
      }
   }

/*   for(j=0;j<n;j++)
   {
      for(i=1;i<m;i++)
      {
         tmp[j] += u[i][j]*b[i];
      }
   }*/
   
   vrow = v[0];
   for (j=0;j<n;j++)
   {
      x[j] = vrow[0]*tmp[0];
   }
   
   for (j=0;j<n;j++)
   {
      vrow = v[j];
      for (jj=1;jj<n;jj++) x[j] += vrow[jj]*tmp[jj];
   }
}

struct thread_arg
{
   float **u;
   float **v;
   int m;
   int n;
   float *b;
   float *x;
   float *tmp;
   int jmin;
   int jmax;
   int done;
};

void * threadsub(void * ptr)
{
   int i, j;
   thread_arg * ta = (thread_arg *) ptr;

   float *urow,  _b;
   
   for(i=1;i<ta->m; i++)
   {
      urow = ta->u[i];
      _b = ta->b[i];
      
      for(j=ta->jmin;j<ta->jmax; j++)
      {
         ta->tmp[j] += urow[j]*_b;
      }
   }

   ta->done = 1;
   return 0;
}

void backsub_precond_F_threads(float **u, float **v, int m, int n, float *b, float *x, float *tmp, int nthreads)
{
   struct sched_param schedpar;
   schedpar.sched_priority = 99;
   
   pthread_t th;
   pthread_attr_t attr;
   pthread_attr_init(&attr);

   pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
   pthread_attr_setschedparam(&attr, &schedpar);
   
   int jj,j,i;
   
   float *vrow, *urow,  _b;
   
   urow = u[0];
   _b = b[0];
   
   for(j=0;j<n; j++)
   {
      tmp[j] = urow[j]*_b;
   }

   pthread_t th1;
   pthread_t th2;
   pthread_t th3;
   pthread_t th4;
   
   thread_arg ta, ta2, ta3, ta4;
   ta.u = u;
   ta.v = v;
   ta.m = m;
   ta.n = n;
   ta.b = b;
   ta.x = x;
   ta.tmp = tmp;
   ta.jmin = 0;
   ta.jmax = n/nthreads;
   ta.done = 0;
   //call thread1
   pthread_create(&th1, 0, &threadsub, (void *) &ta);

   ta4 = ta;
   ta4.jmin = ta.jmax;
   ta4.jmax = n;
   
   if(nthreads == 3)
   {
      ta2 = ta;
      ta2.jmin = ta.jmax;
      ta2.jmax = 2.*ta.jmax;
      pthread_create(&th2, 0, &threadsub, (void *) &ta2);
      ta4.jmin = ta2.jmax;
   }

   threadsub(&ta4);
   
   //pthread_join(th1, 0);
   if(nthreads == 2)
   {
      pthread_join(th1, 0);
   }
   else
   {
      if(!ta.done) pthread_join(th1, 0);
      if(!ta2.done) pthread_join(th2, 0);
   }
   
   vrow = v[0];
   for (j=0;j<n;j++)
   {
      x[j] = vrow[0]*tmp[0];
   }
   
   for (j=0;j<n;j++)
   {
      vrow = v[j];
      for (jj=1;jj<n;jj++) x[j] += vrow[jj]*tmp[jj];
   }
}
