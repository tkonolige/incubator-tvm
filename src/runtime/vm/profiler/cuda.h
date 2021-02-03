/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file src/runtime/vm/profiler/cuda.h
 * \brief CUDA specific profiling.
 */

#ifndef TVM_RUNTIME_VM_PROFILER_CUDA_H_
#define TVM_RUNTIME_VM_PROFILER_CUDA_H_

#include <cuda_runtime.h>
#include <tvm/runtime/vm/profiler.h>

#include <chrono>

namespace tvm {
namespace runtime {
struct CUDATimer : public Timer {
  cudaEvent_t start;
  cudaEvent_t end;
  CUDATimer() {
    cudaEventCreate(&start);
    cudaEventCreate(&end);
  }

  static std::unique_ptr<Timer> Start() {
    std::unique_ptr<Timer> timer(new CUDATimer());
    cudaEventRecord(static_cast<CUDATimer*>(timer.get())->start);
    return timer;
  }

  virtual duration Stop() {
    cudaEventRecord(end);
    cudaEventSynchronize(end);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, end);
    return std::chrono::duration_cast<duration>(
        std::chrono::duration<double, std::milli>(milliseconds));
  }

  virtual ~CUDATimer() {
    cudaEventDestroy(start);
    cudaEventDestroy(end);
  }
};
}  // namespace runtime
}  // namespace tvm

#endif
