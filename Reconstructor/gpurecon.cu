
#include "gpurecon.h"

#ifdef __cplusplus
extern "C"
{
#endif
   
///Pointer to the reconstructor on the GPU, allocated and populated by calling init_gpurecon
float * rec_gpu;

///Pointer to memory for slopes on the GPU, allocated by calling init_gpurecon, updated with each call to gpurecon
float * slopes_gpu;

///Pointer to the amplitude vector on the GPU, allocated by calling init_gpurecon, filled with result of sgemv
float * amps_gpu;

///Number of modes in reconstructor (matrix rows, length or amp vector)
int n_modes;

///Number of slopes in reconstructor (matrix columns, length of slopes vector)
int n_slopes;

///Used bye cublas library
cublasHandle_t handle;

///Get the current time as a double.
double get_curr_t()
{
   struct timespec tsp;
   clock_gettime(CLOCK_REALTIME, &tsp);
   
   return ((double)tsp.tv_sec) + ((double)tsp.tv_nsec)/1e9;
}


int init_gpurecon(int nm, int ns, float *rec_host)
{

   cudaError_t cudaStat;
   cublasStatus_t stat;

   //Initialize the cublas library
   stat = cublasCreate(&handle);
   if ( stat != CUBLAS_STATUS_SUCCESS )
   {
      fprintf (stderr, "CUBLAS initialization failed\n" ) ;
      return EXIT_FAILURE;
   }
   
   n_modes = nm;
   n_slopes = ns;

   //Convert to column major storage.
   float *colmaj = (float *) malloc(nm*ns*sizeof(float));

   if(colmaj == 0)
   {
      fprintf(stderr, "Allocation of main memory for reconstructor col-major failed.\n");
      return EXIT_FAILURE;
   }
   
   for(int i=0;i<nm;i++)
   {
      for(int j=0;j<ns;j++)
      {
         colmaj[j*nm +i] = rec_host[i*ns + j];
      }
   }
   
   cudaStat = cudaMalloc((void **)&rec_gpu, n_modes*n_slopes*sizeof(float));
   if( cudaStat != cudaSuccess )
   {
      fprintf(stderr, "GPU memory allocation failed\n") ;
      return EXIT_FAILURE ;
   }
   
   cudaStat = cudaMalloc((void **)&slopes_gpu, n_slopes * sizeof(float));
   if( cudaStat != cudaSuccess )
   {
      fprintf(stderr, "GPU memory allocation failed\n") ;
      return EXIT_FAILURE ;
   }
   
   cudaStat = cudaMalloc((void **) &amps_gpu, n_modes*sizeof(float));
   if( cudaStat != cudaSuccess )
   {
      fprintf(stderr, "GPU memory allocation failed\n") ;
      return EXIT_FAILURE ;
   }   
   //stat = cublasSetMatrix ( M , N , s i z e o f (* a ) , a , M , devPtrA , M ) ;
   
   stat = cublasSetMatrix(n_modes, n_slopes, sizeof(float), colmaj, n_modes, rec_gpu, n_modes);
   
   if ( stat != CUBLAS_STATUS_SUCCESS)
   {
      fprintf(stderr, "Error loading reconstructor onto GPU\n") ;
      return EXIT_FAILURE ;
   }

   free(colmaj);
   
   return EXIT_SUCCESS;

}

int free_gpurecon()
{
   cudaFree(rec_gpu);
   cudaFree(slopes_gpu);
   cudaFree(amps_gpu);

   cublasDestroy(handle);

   return 0;
}


int gpurecon(float *slopes_host, float *amps_host)
{
   cublasStatus_t stat;
   
   float alpha = 1.0f, beta = 0.0f;
   
   //double t0, t1, t2;
   
   //t0 = get_curr_t();
   
   stat = cublasSetVector(n_slopes, sizeof(float), slopes_host, 1, slopes_gpu, 1);
   if(stat != CUBLAS_STATUS_SUCCESS)
   {
      fprintf(stderr, "Error sending slopes vector to GPU.\n");
      return EXIT_FAILURE;
   }
   //t1 = get_curr_t();
   
   stat = cublasSgemv(handle, CUBLAS_OP_N, n_modes, n_slopes, &alpha, rec_gpu, n_modes, slopes_gpu, 1, &beta, amps_gpu, 1);
   if(stat != CUBLAS_STATUS_SUCCESS)
   {
      fprintf(stderr, "Error during matrix-vector multiply.\n");
      return EXIT_FAILURE;
   }
   //t2 = get_curr_t();

   stat = cublasGetVector(n_modes, sizeof(float), amps_gpu, 1, amps_host, 1);
   if(stat != CUBLAS_STATUS_SUCCESS)
   {
      fprintf(stderr, "Error getting amplitudes vector from GPU.\n");
      return EXIT_FAILURE;
   }
   /*if(tw)
   {
      *tr = get_curr_t() - t2;
      *tmm = t2 - t1;
      *tw = t1 - t0;
   }*/
   
   return EXIT_SUCCESS;

   
}

#ifdef __cplusplus
}//extern "C"
#endif

