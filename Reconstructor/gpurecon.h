#include <stdio.h>
#include <stdlib.h>
#include "cublas_v2.h"

#ifdef __cplusplus
extern "C"
{
#endif

///Initialize the GPU reconstructor.
/** Sets the globals n_modes and n_slopes, and allocates rec_gpu, slopes_gpu, and amps_gpu.
 * \param nm sets n_modes
 * \param ns sets n_slopes
 * \param rec_host is the reconstructor matrix, in row major format.  Converted to column major for passing to GPU.
 * \retval EXIT_FAILURE if an error occurs.
 * \retval EXIT_SUCCESS if it applies.
 */
int init_gpurecon(int nm, int ns, float *rec_host);

///Free GPU memory and shutdown the cublas library.
/** \retval 0 always
 */
int free_gpurecon();

///Performs the matrix-vector multiply on the GPU.
/**Uses the Cuda BLAS routine sgemv (see the blas standard for description).
 * \param slopes_host pointer to the slopes vector of length n_slopes (as set in init_gpurecon)
 * \param amps_host pointer to the amplitudes vector of length n_modes (as set in init_gpurecon)
 * \retval EXIT_FAILURE if something goes wrong
 * \retval EXIT_SUCCESS if everything works.
 */
int gpurecon(float *slopes_host, float *amps_host);

#ifdef __cplusplus
}//extern "C"
#endif
