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
 * \file src/relay/op/profiler/profiler.cc
 * \brief Passes and operators to support runtime profiling.
 */

// TODO(tkonolige): this file should be in runtime

#include <tvm/relay/attrs/profiler.h>
#include <tvm/relay/expr_functor.h>
#include <tvm/relay/op.h>
#include <tvm/relay/transform.h>
#include <tvm/runtime/vm/bytecode.h>
#include <tvm/runtime/vm/vm.h>
#include <tvm/topi/elemwise.h>

#include "../op/op_common.h"

namespace tvm {
namespace relay {

TVM_REGISTER_NODE_TYPE(InvokeRegistryPackedAttrs);

bool InvokeRegistryPackedRel(const Array<Type>& types, int num_inputs, const Attrs& attrs,
                             const TypeReporter& reporter) {
  ICHECK_EQ(types.size(), 2u);
  reporter->Assign(types[1], types[0]);
  return true;
}

RELAY_REGISTER_OP("profiler.call_packed")
    .describe(R"code(Call arbitrary packed function)code" TVM_ADD_FILELINE)
    .set_num_inputs(1)
    .set_support_level(10)
    .add_type_rel("InvokeRegistryPacked", InvokeRegistryPackedRel)
    .set_attr<TOpPattern>("TOpPattern", kOpaque)
    .set_attr<TOpIsStateful>("TOpIsStateful", false)
    .set_attr<TNonComputational>("TNonComputational", true)
    .set_attr<FInferCorrectLayout>("FInferCorrectLayout", ElemwiseArbitraryLayout)
    .set_attr<FTVMCompute>("FTVMCompute",
                           [](const Attrs& attrs, const Array<te::Tensor>& inputs,
                              const Type& out_dtype) -> Array<te::Tensor> {
                             return {topi::identity(inputs[0])};
                           });

TVM_REGISTER_GLOBAL("relay.op.profiler.call_packed").set_body_typed([](String name) {
  static const Op& op = Op::Get("profiler.call_packed");
  auto attrs = make_object<InvokeRegistryPackedAttrs>();
  return Call(op, {}, Attrs(attrs), {});
});

class ProfilerCallInserter : public ExprMutator {
 public:
  Expr VisitExpr_(const CallNode* call_node) final {
    // c = f(a, b)
    // c = profiler.call_packed((f(a, b))
    auto ret = ExprMutator::VisitExpr_(call_node);
    static const Op& op = Op::Get("profiler.call_packed");
    auto attrs = make_object<InvokeRegistryPackedAttrs>();
    attrs->name = "test";
    return Call(op, {ret}, Attrs(attrs), {});
  }
};



// DLContext InstructionContext(const Instruction& instr) {
//   return;
// }

/*
 Profiler API

RegisterProfiler: takes 3 functions
  - Profiler Init: () -> Profiler
  - Profiler Precall: Profiler -> VM -> ()
  - Profiler Postcall: Profiler -> VM -> ()
  - Generate summary: Profiler -> A
  - summary print: A -> string

Alternatively to pre and post call
  - Profiler call: Profiler -> (call location as string or enum) -> VM -> ()



  rly_vm = tvm.runtime.vm.VirtualMachine(exe, _ctx, profilers=["time", "instructions"])
  rly_vm = tvm.runtime.vm.VirtualMachine(exe, _ctx, profilers=profilers.all())


  Object ProfilerInit();
  void ProfilerPreCall(Object, VM);
  void ProfilerPostCall(Object, VM);

  class Profiler {
    static Profiler Init();
    void ProfilerPreCall(VM);
    void ProfilerPostCall(VM);
  }
 */

namespace transform {

Pass AddProfiling() {
  return tvm::transform::CreateModulePass(
      [=](IRModule mod, const PassContext& pass_ctx) {
        for (auto func : mod->functions) {
          auto rewriter = ProfilerCallInserter();
          auto updated_func = Downcast<Function>(rewriter.Mutate(func.second));
          mod->Update(func.first, updated_func);
        }
        return mod;
      },
      0, "AddProfiling", {});
}

TVM_REGISTER_GLOBAL("relay.profiler.add_profiling").set_body_typed(AddProfiling);

}  // namespace transform
}  // namespace relay
}  // namespace tvm
