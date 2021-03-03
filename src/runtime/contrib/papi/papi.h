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
#pragma once

#include <papi.h>

namespace tvm {
namespace runtime {

#define PAPI_CALL(func)                                \
  {                                                    \
    int e = (func);                                    \
    if (e < 0) {                                \
      LOG(FATAL) << "PAPIError: " << e << " " << std::string(PAPI_strerror(e)); \
    }                                                  \
  }

inline void* papi_start_call(TVMContext ctx) {
  PAPI_CALL(PAPI_library_init(PAPI_VER_CURRENT));
  int* event_set = new int(PAPI_NULL);
  PAPI_CALL(PAPI_create_eventset(event_set));
  // PAPI_CALL(PAPI_add_event(*event_set, PAPI_TOT_INS));
  int cidx;
  cidx = PAPI_get_component_index("cuda");
  LOG(INFO) << cidx << " " << PAPI_OK;
  int event_code;
  PAPI_CALL(PAPI_event_name_to_code("cuda:::metric:dram_utilization", &event_code));
  PAPI_CALL(PAPI_add_event(*event_set, event_code));
  PAPI_CALL(PAPI_start(*event_set));
  return event_set;
}

inline std::unordered_map<std::string, ObjectRef> papi_stop_call(void* data) {
  long_long values[1];
  int* event_set = static_cast<int*>(data);
  PAPI_CALL(PAPI_stop(*event_set, values));
  // PAPI_CALL(PAPI_read(*event_set, values));
  LOG(INFO) << values[0];
  // PAPI_CALL(PAPI_destroy_eventset(event_set));
  delete event_set;
  return {};
}

}  // namespace runtime
}  // namespace tvm
