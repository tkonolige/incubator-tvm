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

# Be compatible with older version of CMake
find_vulkan(${USE_VULKAN})

# Extra Vulkan runtime options, exposed for advanced users.
tvm_option(USE_VULKAN_IMMEDIATE_MODE "Use Vulkan Immediate mode
(KHR_push_descriptor extension)" ON IF USE_VULKAN)
tvm_option(USE_VULKAN_DEDICATED_ALLOCATION "Use Vulkan dedicated allocations" ON
IF USE_VULKAN)
tvm_option(USE_VULKAN_VALIDATION "Enable Vulkan API validation layers" OFF
  IF USE_VULKAN)

if(Vulkan_FOUND)
  # always set the includedir
  # avoid global retrigger of cmake
  include_directories(${Vulkan_INCLUDE_DIRS})
endif(Vulkan_FOUND)

if(USE_VULKAN)
  if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Cannot find Vulkan, USE_VULKAN=" ${USE_VULKAN})
  endif()
  message(STATUS "Build with Vulkan support")
  file(GLOB RUNTIME_VULKAN_SRCS src/runtime/vulkan/vulkan.cc)
  file(GLOB COMPILER_VULKAN_SRCS src/target/spirv/*.cc)
  target_sources(tvm_runtime_objs PRIVATE ${RUNTIME_VULKAN_SRCS})
  target_sources(tvm_objs PRIVATE ${COMPILER_VULKAN_SRCS})
  target_link_libraries(tvm_deps INTERFACE ${Vulkan_SPIRV_TOOLS_LIBRARY})

  if(USE_VULKAN_IMMEDIATE_MODE)
    message(STATUS "Build with Vulkan immediate mode")
    target_compile_definitions(tvm_deps INTERFACE USE_VULKAN_IMMEDIATE_MODE=1)
  endif()
  if(USE_VULKAN_DEDICATED_ALLOCATION)
    message(STATUS "Build with Vulkan dedicated allocation")
    target_compile_definitions(tvm_deps INTERFACE USE_VULKAN_DEDICATED_ALLOCATION=1)
  endif()
  if(USE_VULKAN_VALIDATION)
    message(STATUS "Build with Vulkan API validation")
    target_compile_definitions(tvm_deps INTERFACE USE_VULKAN_VALIDATION=1)
  endif()
endif(USE_VULKAN)
