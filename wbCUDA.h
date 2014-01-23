
#ifndef __WB_CUDA_H__
#define __WB_CUDA_H__

#ifdef WB_USE_CUDA
#include "cuda.h"
#include "cuda_runtime.h"
#include "time.h"

typedef struct st_wbCUDAMemory_t {
  void *mem;
  size_t sz;
} wbCUDAMemory_t;

#define _cudaMemoryListSize 1024

extern size_t _cudaMallocSize;
extern wbCUDAMemory_t _cudaMemoryList[_cudaMemoryListSize];
extern int _cudaMemoryListIdx;

char * wbRandom_list(size_t sz);

static inline cudaError_t wbCUDAMalloc(void **devPtr, size_t sz) {
  int idx = _cudaMemoryListIdx;
  
  cudaError_t err = cudaMalloc(devPtr, sz);
  
  if (idx == 0) {
    srand(time(NULL));
    memset(_cudaMemoryList, 0, sizeof(wbCUDAMemory_t) * _cudaMemoryListSize);
  }

  if (err == cudaSuccess) {
#if 1
    char * rands = wbRandom_list(sz);
    // can use curand here, but do not want to invoke a kernel
    err = cudaMemcpy(*devPtr, rands, sz, cudaMemcpyHostToDevice);
    wbFree(rands);
#else
    err = cudaMemset(*devPtr, 0, sz);
#endif
  }

  _cudaMallocSize += sz;
  _cudaMemoryList[idx].mem = *devPtr;
  _cudaMemoryList[idx].sz = sz;
  _cudaMemoryListIdx++;
  return err;
}

static inline cudaError_t wbCUDAFree(void *mem) {
  int idx = _cudaMemoryListIdx;
  if (idx == 0) {
    memset(_cudaMemoryList, 0, sizeof(wbCUDAMemory_t) * _cudaMemoryListSize);
  }
  for (int ii = 0; ii < idx; ii++) {
    if (_cudaMemoryList[ii].mem != NULL && _cudaMemoryList[ii].mem == mem) {
      cudaError_t err = cudaFree(mem);
      _cudaMallocSize -= _cudaMemoryList[ii].sz;
      _cudaMemoryList[idx].mem = NULL;
      return err;
    }
  }
  return cudaErrorMemoryAllocation;
}

#define cudaMalloc wbCUDAMalloc
#define cudaFree wbCUDAFree

#endif /* WB_USE_CUDA */

#endif /* __WB_CUDA_H__ */
