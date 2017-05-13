/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreKernels.cu

Abstract:

    This module implements CUDA kernels for various trace store functions.

--*/

#ifdef __cplusplus
extern "C" {
#endif

__global__
void saxpy(int n, float a, float *x, float *y)
{
    for (int i = blockIdx.x * blockDim.x + threadIdx.x;
         i < n;
         i += blockDim.x * gridDim.x) {

        y[i] = a * x[i] + y[i];
    }
}

#ifdef __cplusplus
}
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
