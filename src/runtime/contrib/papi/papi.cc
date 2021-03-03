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
 * \brief Performance counters for profiling via the PAPI library.
 * \file papi.h
 */
#ifndef TVM_RUNTIME_CONTRIB_PAPI_PAPI_H_
#define TVM_RUNTIME_CONTRIB_PAPI_PAPI_H_

#include <papi.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace tvm {
namespace runtime {
namespace profiling {

#define PAPI_CALL(func)                                                         \
  {                                                                             \
    int e = (func);                                                             \
    if (e < 0) {                                                                \
      LOG(FATAL) << "PAPIError: " << e << " " << std::string(PAPI_strerror(e)); \
    }                                                                           \
  }

// const static std::vector<std::string> metrics = {"cuda:::metric:dram_write_throughput:device=0"};
const static std::unordered_map<std::string, std::vector<std::string>> metrics = {
    //"perf::INSTRUCTIONS",
    "perf::CYCLES",
    // "perf::CACHE-MISSES",
    // "perf::CACHE-REFERENCES",
    // "perf::BRANCH-MISSES",
    // "MISALIGNED_LOADS",
    "perf::STALLED-CYCLES-FRONTEND",
    "perf::STALLED-CYCLES-BACKEND",
    // "perf::L1-ICACHE-LOAD-MISSES",
    // "perf::ITLB-LOAD-MISSES",
    // "RETIRED_BRANCH_INSTRUCTIONS_MISPREDICTED",
    // "32_BYTE_INSTRUCTION_CACHE_MISSES",
    "perf::L1-DCACHE-LOAD-MISSES",
    "perf::L1-DCACHE-LOADS",
    // "perf::L1-DCACHE-STORES:cpu=0",
    // "perf::L1-DCACHE-STORE-MISSES",
    // "perf::LLC-LOAD-MISSES",
    // "perf::PERF_COUNT_HW_CACHE_LL",
    // "perf::LLC-STORE-MISSES",
    // "perf::DTLB-LOAD-MISSES",
    // "CYCLES_WITH_FILL_PENDING_FROM_L2"
};

inline int papi_start(TVMContext ctx) {
  if (!PAPI_is_initialized()) {
    // PAPI_CALL(PAPI_set_debug(PAPI_VERB_ECONT));
    PAPI_CALL(PAPI_library_init(PAPI_VER_CURRENT));
    // PAPI_CALL(PAPI_thread_init(pthread_self));
    // PAPI_CALL(PAPI_register_thread());
    // PAPI_CALL(PAPI_multiplex_init());
    // PAPI_CALL(PAPI_set_granularity(PAPI_GRN_PROC));
  }
  int event_set = PAPI_NULL;
  PAPI_CALL(PAPI_create_eventset(&event_set));
  // TODO: set this correctly
  PAPI_CALL(PAPI_assign_eventset_component(event_set, 0));
  PAPI_option_t opt;
  memset(&opt, 0x0, sizeof(PAPI_option_t));
  opt.inherit.inherit = PAPI_INHERIT_ALL;
  opt.inherit.eventset = event_set;
  PAPI_CALL(PAPI_set_opt(PAPI_INHERIT, &opt));
  // TODO: check if multiplexing is needed
  // PAPI_CALL(PAPI_set_multiplex(*event_set));
  for (auto metric : metrics) {
    int e = PAPI_add_named_event(event_set, metric.c_str());
    if (e < 0) {
      LOG(FATAL) << "PAPIError: " << e << " " << std::string(PAPI_strerror(e)) << ": " << metric << ". If running on the cpu, try setting `sudo sh -c 'echo 1 >/proc/sys/kernel/perf_event_paranoid'`";
    }
  }
  PAPI_CALL(PAPI_start(event_set));
  return event_set;
}

inline std::vector<long_long> papi_start_call(int event_set) {
  std::vector<long_long> values(metrics.size());
  PAPI_CALL(PAPI_read(event_set, values.data()));
  return values;
}

inline std::unordered_map<std::string, ObjectRef> papi_stop_call(
    const std::vector<long_long>& starting_values, int event_set) {
  std::vector<long_long> values(metrics.size());
  PAPI_CALL(PAPI_read(event_set, values.data()));
  std::unordered_map<std::string, ObjectRef> out;
  for (size_t i = 0; i < metrics.size(); i++) {
    out[metrics[i]] = ObjectRef(make_object<CountNode>(values[i] - starting_values[i]));
  }
  return out;
}

inline std::unordered_map<std::string, ObjectRef> papi_stop(int event_set) {
  std::vector<long_long> values(metrics.size());
  PAPI_CALL(PAPI_stop(event_set, values.data()));
  // PAPI_CALL(PAPI_read(*event_set, values));
  PAPI_CALL(PAPI_cleanup_eventset(event_set));
  PAPI_CALL(PAPI_destroy_eventset(&event_set));
  std::unordered_map<std::string, ObjectRef> out;
  for (size_t i = 0; i < metrics.size(); i++) {
    out[metrics[i]] = ObjectRef(make_object<CountNode>(values[i]));
  }
  return out;
}

}  // namespace profiling
}  // namespace runtime
}  // namespace tvm
#endif  // TVM_RUNTIME_CONTRIB_PAPI_PAPI_H_
