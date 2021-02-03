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
 * \brief CPU specific profiling.
 */

#ifndef TVM_RUNTIME_VM_PROFILER_CPU_H_
#define TVM_RUNTIME_VM_PROFILER_CPU_H_

#include <tvm/runtime/vm/profiler.h>

#include <chrono>

namespace tvm {
namespace runtime {
struct CPUTimer : public Timer {
  std::chrono::steady_clock::time_point start;

  static std::unique_ptr<Timer> Start() {
    std::unique_ptr<Timer> timer(new CPUTimer());
    static_cast<CPUTimer*>(timer.get())->start = std::chrono::steady_clock::now();
    return timer;
  }

  virtual duration Stop() {
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<duration>(stop - start);
  }

  virtual ~CPUTimer() {}
};
}  // namespace runtime
}  // namespace tvm

#endif
