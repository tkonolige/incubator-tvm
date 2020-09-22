# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# ROCM Module

if(USE_ROCM)
  find_rocm(${USE_ROCM})
  if(NOT ROCM_FOUND)
    message(FATAL_ERROR "Cannot find ROCM, USE_ROCM=" ${USE_ROCM})
  endif()
  message(STATUS "Build with ROCM support")

  target_include_directories(tvm_deps INTERFACE ${ROCM_INCLUDE_DIRS})
  target_compile_definitions(tvm_deps INTERFACE __HIP_PLATFORM_HCC__=1)

  file(GLOB RUNTIME_ROCM_SRCS src/runtime/rocm/*.cc)
  target_sources(tvm_runtime_objs PRIVATE ${RUNTIME_ROCM_SRCS})
  target_link_libraries(tvm_runtime_objs PRIVATE ${ROCM_HIPHCC_LIBRARY})

  if(USE_MIOPEN)
    message(STATUS "Build with MIOpen support")
    file(GLOB MIOPEN_CONTRIB_SRCS src/runtime/contrib/miopen/*.cc)
    target_sources(tvm_runtime_objs PRIVATE ${MIOPEN_CONTRIB_SRCS})
    target_link_libraries(tvm_runtime_objs PRIVATE ${ROCM_MIOPEN_LIBRARY})
  endif(USE_MIOPEN)

  if(USE_ROCBLAS)
    message(STATUS "Build with RocBLAS support")
    file(GLOB ROCBLAS_CONTRIB_SRCS src/runtime/contrib/rocblas/*.cc)
    target_sources(tvm_runtime_objs PRIVATE ${ROCBLAS_CONTRIB_SRCS})
    target_link_libraries(tvm_runtime_objs PRIVATE ${ROCM_ROCBLAS_LIBRARY})
  endif(USE_ROCBLAS)
else(USE_ROCM)
  target_sources(tvm_objs PRIVATE src/target/opt/build_rocm_off.cc)
endif(USE_ROCM)
