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
 * \file src/runtime/vm/profiler/vm.cc
 * \brief The Relay debug virtual machine.
 */

#include "vm.h"

#include <tvm/runtime/registry.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace tvm {
namespace runtime {
namespace vm {

PackedFunc VirtualMachineDebug::GetFunction(const std::string& name,
                                            const ObjectPtr<Object>& sptr_to_self) {
  if (name == "get_stat") {
    return PackedFunc(
        [sptr_to_self, this](TVMArgs args, TVMRetValue* rv) { *rv = prof_.Report(); });
  } else if (name == "reset") {
    return PackedFunc(
        [sptr_to_self, this](TVMArgs args, TVMRetValue* rv) { prof_ = profiling::Profiler(); });
  } else if (name == "invoke") {
    return PackedFunc([sptr_to_self, this](TVMArgs args, TVMRetValue* rv) {
      std::vector<TVMContext> ctxs;
      for (auto ctx : ctxs_) {
        if (ctx.device_type > 0) {
          ctxs.push_back(ctx);
        }
      }
      auto invoke = VirtualMachine::GetFunction("invoke", sptr_to_self);
      prof_.Start(ctxs);
      invoke.CallPacked(args, rv);
      prof_.Stop();
    });
  } else if (name == "profile") {
    return TypedPackedFunc<String(String)>([sptr_to_self, this](String arg_name) {
      std::vector<TVMContext> ctxs;
      for (auto ctx : ctxs_) {
        if (ctx.device_type > 0) {
          ctxs.push_back(ctx);
        }
      }

      auto invoke = VirtualMachine::GetFunction("invoke", sptr_to_self);
      // warmup
      for (int i = 0; i < 3; i++) {
        invoke(arg_name);
      }

      prof_ = profiling::Profiler();  // reset profiler
      prof_.Start(ctxs);
      invoke(arg_name);
      prof_.Stop();
      return prof_.Report();
    });
  } else {
    return VirtualMachine::GetFunction(name, sptr_to_self);
  }
}

void VirtualMachineDebug::LoadExecutable(const Executable* exec) {
  VirtualMachine::LoadExecutable(exec);
  ICHECK(exec_);
  for (auto kv : exec_->primitive_map) {
    packed_index_map_[kv.second] = kv.first;
  }
}

void VirtualMachineDebug::InvokePacked(Index packed_index, const PackedFunc& func, Index arg_count,
                                       Index output_size, const std::vector<ObjectRef>& args) {
  ICHECK(exec_);
  ICHECK(!ctxs_.empty()) << "Context has not been initialized yet.";
  // The device context of any input of the operator is used for
  // synchronization.
  ICHECK_GT(arg_count, 0U);
  ObjectRef arg = args[0];
  while (arg->IsInstance<ADTObj>()) {
    ADT adt = Downcast<ADT>(arg);
    arg = adt[0];
  }
  ICHECK(arg->IsInstance<NDArray::ContainerType>());
  auto nd_array = Downcast<NDArray>(arg);
  auto ctx = nd_array->ctx;

  // get argument sizes
  std::vector<NDArray> shapes;
  for (Index i = 0; i < arg_count; i++) {
    if (const auto* obj = args[i].as<ADTObj>()) {
      for (size_t fi = 0; fi < obj->size; ++fi) {
        auto o = (*obj)[fi];
        shapes.push_back(Downcast<NDArray>(o));
      }
    } else {
      shapes.push_back(Downcast<NDArray>(args[i]));
    }
  }

  std::unordered_map<std::string, ObjectRef> metrics;
  for (auto p : exec_->op_attrs.at(packed_index)) {
    if (std::string(p.first).find("layout") != std::string::npos) {
      metrics[p.first] = p.second;
    }
  }
  metrics["Hash"] = Downcast<String>(exec_->op_attrs.at(packed_index)["hash"]); // FIXME: if hash is not defined
  metrics["Argument Shapes"] = profiling::ShapeString(shapes);

  prof_.StartCall(packed_index_map_[packed_index], ctx, metrics);
  VirtualMachine::InvokePacked(packed_index, func, arg_count, output_size, args);
  prof_.StopCall();
}

runtime::Module CreateVirtualMachineDebug(const Executable* exec) {
  auto vm = make_object<VirtualMachineDebug>();
  vm->LoadExecutable(exec);
  return runtime::Module(vm);
}

TVM_REGISTER_GLOBAL("runtime._VirtualMachineDebug").set_body([](TVMArgs args, TVMRetValue* rv) {
  runtime::Module mod = args[0];
  const auto* exec = dynamic_cast<Executable*>(mod.operator->());
  ICHECK(exec) << "Virtual machine has not been defined yet."
               << "\n";
  *rv = CreateVirtualMachineDebug(exec);
});

}  // namespace vm
}  // namespace runtime
}  // namespace tvm
