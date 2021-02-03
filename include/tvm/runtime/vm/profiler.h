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
 * \file tvm/runtime/vm/profiler.h
 * \brief Profilers for the runtime VM.
 */
#ifndef TVM_RUNTIME_VM_PROFILER_H_
#define TVM_RUNTIME_VM_PROFILER_H_

#include <cuda_runtime.h>
#include <dlpack/dlpack.h>
#include <tvm/runtime/container.h>
#include <tvm/runtime/module.h>
#include <tvm/runtime/vm/vm.h>

#include <chrono>
#include <map>
#include <vector>

namespace tvm {
namespace runtime {

struct Profiler {
  virtual void PreCall(const vm::VirtualMachine& vm){};
  virtual void PostCall(const vm::VirtualMachine& vm){};
  virtual void Start(const vm::VirtualMachine& vm){};
  virtual void End(const vm::VirtualMachine& vm){};
  // virtual Object Collect() {};
  virtual String Summarize() { return ""; };
  virtual ~Profiler() = 0;
};
struct Timer {
  using duration = std::chrono::duration<int64_t, std::nano>;
  virtual duration Stop() = 0;
  virtual ~Timer(){};
};

enum class EventLocation {
  Initialization,
  Runtime,
  Summary,
};

class TimeProfiler : public Profiler {
 public:
  TimeProfiler();
  using duration = Timer::duration;
  virtual void PreCall(const vm::VirtualMachine& vm);
  virtual void Start(const vm::VirtualMachine& vm);
  virtual void End(const vm::VirtualMachine& vm);
  virtual void PostCall(const vm::VirtualMachine& vm);
  // virtual Object Collect() ;
  virtual String Summarize();
  virtual ~TimeProfiler(){};

 private:
  void RecordTime(EventLocation loc, std::string name, duration duration);
  std::map<DLDeviceType, std::function<std::unique_ptr<Timer>()>> device_timer;
  std::vector<std::pair<DLDeviceType, std::unique_ptr<Timer>>> global_timers;
  std::unique_ptr<Timer> local_timer;
  std::map<EventLocation, std::unordered_map<std::string, std::vector<duration>>> times;
};

}  // namespace runtime
}  // namespace tvm

#endif
