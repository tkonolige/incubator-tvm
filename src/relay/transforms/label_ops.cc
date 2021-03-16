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
#include <tvm/relay/expr_functor.h>
#include <tvm/relay/transform.h>
#include <tvm/ir/attrs.h>
#include <tvm/runtime/container.h>

namespace tvm {
  namespace relay {
    namespace transform {

      namespace {
        struct CollectAttrs : public AttrVisitor {
          virtual void Visit(const char* key, std::string* value) final {
            if(std::string(key).find("layout") != std::string::npos) {
              attrs[key] = String(*value);
            }
          }
          virtual void Visit(const char* key, double* value) final {
          }
          virtual void Visit(const char* key, uint64_t* value) final {
          }
          virtual void Visit(const char* key, int* value) final {
          }
          virtual void Visit(const char* key, int64_t* value) final {
          }
          virtual void Visit(const char* key, bool* value) final {
          }
          virtual void Visit(const char* key, runtime::NDArray* value) final {
          }
          virtual void Visit(const char* key, ObjectRef* value) final {
            if(std::string(key).find("layout") != std::string::npos) {
              attrs[key] = *value;
            }
          }
          virtual void Visit(const char* key, DataType* value) final {
          }
          virtual void Visit(const char* key,void** value) final {
          }
          std::unordered_map<std::string, ObjectRef> attrs;
        };
      }

      class LabelOpsMutator :public MixedModeMutator {
        std::unordered_map<std::string, ObjectRef> body_attrs;
        private:
          Expr VisitExpr_(const FunctionNode* op) {
            body_attrs = {};
            auto updated = ExprMutator::VisitExpr_(op);
            size_t hash = StructuralHash()(updated);
            std::stringstream s;
            s << std::setfill('0') << std::setw(sizeof(size_t)*2) << std::hex << hash;
            Function f = WithAttr(Downcast<Function>(updated), "hash", String(s.str()));
            for(auto p : body_attrs) {
              f = WithAttr(f, p.first, p.second);
            }
            return f;
          }

          Expr Rewrite_(const CallNode* op, const Expr& post) {
            auto updated = MixedModeMutator::Rewrite_(op, post);
            if(op->attrs.defined()) {
              CollectAttrs collect;
              const_cast<BaseAttrsNode*>(op->attrs.get())->VisitAttrs(&collect);
              for(auto p : collect.attrs) {
                // TODO(tkonolige): will discard duplicate entries
                body_attrs[p.first] = p.second;
              }
            }
            return updated;
          }
      };

      Pass LabelOps() {
        runtime::TypedPackedFunc<Function(Function, IRModule, PassContext)> pass_func =
          [=](Function f, IRModule m, PassContext pc) {
            return Downcast<Function>(LabelOpsMutator().Mutate(f));
          };
        return CreateFunctionPass(pass_func, 1, "LabelOps", {});
      }

    }
  }
}
