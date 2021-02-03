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
 * \file src/runtime/vm/profiler/profiler.cc
 * \brief Passes and operators to support runtime profiling.
 */

#include <tvm/runtime/vm/bytecode.h>
#include <tvm/runtime/vm/profiler.h>
#include <tvm/runtime/vm/vm.h>

#include <chrono>
#include <iostream>
#include <numeric>
#include <iomanip>

#ifdef TVM_CUDA_ENABLED
#include "cuda.h"
#endif

#include "cpu.h"

namespace tvm {
namespace runtime {

static const std::map<DLDeviceType, std::string> DLDEVICETYPE_NAMES = {
    {kDLCPU, "CPU"},
    {kDLGPU, "CUDA"},
    {kDLROCM, "ROCM"},
};

Profiler::~Profiler() {}

TimeProfiler::TimeProfiler() {
#ifdef TVM_CUDA_ENABLED
  device_timer[DLDeviceType::kDLGPU] = CUDATimer::Start;
#endif
  device_timer[DLDeviceType::kDLCPU] = CPUTimer::Start;
  times[EventLocation::Initialization] = {};
  times[EventLocation::Runtime] = {};
  times[EventLocation::Summary] = {};
}

void TimeProfiler::RecordTime(EventLocation loc, std::string name, duration duration) {
  auto it = times[loc].find(name);
  if (it == times[loc].end()) {
    times[loc][name] = {duration};
  } else {
    times[loc][name].push_back(duration);
  }
}

void TimeProfiler::Start(const vm::VirtualMachine& vm) {
  for (auto& ctx : vm.ctxs_) {
    if (device_timer.find(ctx.device_type) != device_timer.end()) {
      global_timers.emplace_back(ctx.device_type, device_timer[ctx.device_type]());
    }
  }

  Module mod = vm.exec_->lib;
  for (auto& ctx : vm.ctxs_) {
    if (ctx.device_type == DLDeviceType::kDLGPU) {
      local_timer = device_timer.at(ctx.device_type)();
      auto func = mod.GetFunction(symbol::tvm_initialize, true);
      if (func != nullptr) {
        func(ctx.device_id);
      }
      RecordTime(EventLocation::Initialization, "CUDA Initialization", local_timer->Stop());
    }
  }
}

void TimeProfiler::End(const vm::VirtualMachine& vm) {
  for (auto& timer : global_timers) {
    RecordTime(EventLocation::Summary, "Total Time " + DLDEVICETYPE_NAMES.at(timer.first),
               timer.second->Stop());
  }
}

DLDeviceType GetDeviceType(const vm::VirtualMachine& vm, const vm::Instruction& instr) {
  if (instr.op == vm::Opcode::InvokePacked) {
    for (size_t i = 0; i < instr.arity; i++) {
      auto arg = vm.ReadRegister(instr.packed_args[i]);
      if (const auto* dt_cell = arg.as<ADTObj>()) {
        for (size_t fi = 0; fi < dt_cell->size; ++fi) {
          auto obj = (*dt_cell)[fi];
          auto nd_array = Downcast<NDArray>(obj);
          return nd_array->ctx.device_type;
        }
      } else {
        auto nd_array = Downcast<NDArray>(arg);
        return nd_array->ctx.device_type;
      }
    }
    LOG(FATAL) << "Could not determine device type from instruction";
    throw;
  } else {
    return kDLCPU;
  }
}

void TimeProfiler::PreCall(const vm::VirtualMachine& vm) {
  const vm::Instruction& next_instr = vm.code_[vm.pc_ + 1];
  DLDeviceType device_type = GetDeviceType(vm, next_instr);
  local_timer = device_timer[device_type]();
}

void TimeProfiler::PostCall(const vm::VirtualMachine& vm) {
  auto time = local_timer->Stop();
  const tvm::runtime::vm::Instruction& prev_instr = vm.code_[vm.pc_ - 1];
  std::string op_name;
  if (prev_instr.op == runtime::vm::Opcode::InvokePacked) {
    op_name =
        std::find_if(vm.exec_->primitive_map.begin(), vm.exec_->primitive_map.end(), [&](auto p) {
          return p.second == prev_instr.packed_index;
        })->first;
  } else {
    op_name = "Opcode " + OpName(prev_instr.op);
  }
  RecordTime(EventLocation::Runtime, op_name, time);
}

String TimeProfiler::Summarize() {
  duration total_time = times[EventLocation::Summary]["Total Time CPU"][0];
  auto microsecs = [](duration d) { return std::chrono::duration<double, std::micro>(d).count(); };
  std::stringstream s;
  s << std::fixed << std::setprecision(2);
  duration duration_sum = duration::zero();
  for (auto& loc :
       {EventLocation::Initialization, EventLocation::Runtime, EventLocation::Summary}) {
    for (auto ts : times[loc]) {
      s << ts.first << ":\t";
      auto total = std::accumulate(ts.second.begin(), ts.second.end(), duration::zero());
      s << microsecs(total) << "us\t" << microsecs(total)/microsecs(total_time)*100 << "%\t" << ts.second.size() << std::endl;
      if (loc != EventLocation::Summary) {
        duration_sum += total;
      }
      // for(auto t : ts.second) {
      //   s << microsecs(t) << " ";
      // }
      // s<<std::endl;
    }
  }
  s << "Time in initialization and operators:\t" << microsecs(duration_sum) << "us" << std::endl;
  s << "VM Overhead:\t" << microsecs(total_time - duration_sum) << "us "
    << microsecs(total_time - duration_sum) / microsecs(total_time) * 100 << "%" << std::endl;
  return s.str();
}

static std::vector<std::unique_ptr<Profiler>> profilers;

DLContext GetContext(const vm::VirtualMachine& vm, vm::Instruction instr) {
  ICHECK(instr.op == vm::Opcode::InvokePacked) << "Can only get context of InvokePacked";
  for (vm::Index i = 0; i < instr.arity; i++) {
    auto arg = vm.ReadRegister(instr.packed_args[i]);
    if (arg.as<ADTObj>() == nullptr) {
      auto ndarray = Downcast<NDArray>(arg);
      return ndarray->ctx;
    }
  }
  LOG(FATAL) << "Could not determine device context for instruction";
  throw;
}

void ProfilerStart(runtime::Module mod) {
  auto vm = mod.as<::tvm::runtime::vm::VirtualMachine>();
  profilers.clear();
  profilers.emplace_back(std::make_unique<TimeProfiler>());
  for (auto& profiler : profilers) {
    profiler->Start(*vm);
  }
}
TVM_REGISTER_GLOBAL("profiler_start").set_body_typed(ProfilerStart);

void ProfilerPreCall(runtime::Module mod) {
  auto vm = mod.as<::tvm::runtime::vm::VirtualMachine>();
  for (auto& profiler : profilers) {
    profiler->PreCall(*vm);
  }
}
TVM_REGISTER_GLOBAL("profiler_pre_call").set_body_typed(ProfilerPreCall);

void ProfilerPostCall(runtime::Module mod) {
  auto vm = mod.as<::tvm::runtime::vm::VirtualMachine>();
  for (auto& profiler : profilers) {
    profiler->PostCall(*vm);
  }
}
TVM_REGISTER_GLOBAL("profiler_post_call").set_body_typed(ProfilerPostCall);

void ProfilerEnd(runtime::Module mod) {
  auto vm = mod.as<::tvm::runtime::vm::VirtualMachine>();
  for (auto& profiler : profilers) {
    profiler->End(*vm);
  }
  for (auto& profiler : profilers) {
    std::cout << profiler->Summarize() << std::endl;
  }
}
TVM_REGISTER_GLOBAL("profiler_end").set_body_typed(ProfilerEnd);
}  // namespace runtime
}  // namespace tvm
