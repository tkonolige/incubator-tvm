cmake_minimum_required(VERSION 3.8)
project(tvm C CXX)

# Utility functions
include(cmake/util/Util.cmake)
include(cmake/util/FindCUDA.cmake)
include(cmake/util/FindOpenCL.cmake)
include(cmake/util/FindVulkan.cmake)
include(cmake/util/FindLLVM.cmake)
include(cmake/util/FindROCM.cmake)
include(cmake/util/FindEthosN.cmake)

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/config.cmake)
  include(${CMAKE_CURRENT_BINARY_DIR}/config.cmake)
else()
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake)
  endif()
endif()

# NOTE: do not modify this file to change option values.
# You can create a config.cmake at build folder
# and add set(OPTION VALUE) to override these build options.
# Alernatively, use cmake -DOPTION=VALUE through command-line.
tvm_option(USE_CUDA "Build with CUDA" OFF)
tvm_option(USE_OPENCL "Build with OpenCL" OFF)
tvm_option(USE_VULKAN "Build with Vulkan" OFF)
tvm_option(USE_METAL "Build with Metal" OFF)
tvm_option(USE_ROCM "Build with ROCM" OFF)
tvm_option(ROCM_PATH "The path to rocm" /opt/rocm)
tvm_option(USE_HEXAGON_DEVICE "Build with Hexagon device support in TVM runtime" OFF)
tvm_option(USE_HEXAGON_SDK "Path to the Hexagon SDK root (required for Hexagon support in TVM runtime or for building TVM runtime for Hexagon)" /path/to/sdk)
tvm_option(USE_RPC "Build with RPC" ON)
tvm_option(USE_THREADS "Build with thread support" ON)
tvm_option(USE_LLVM "Build with LLVM, can be set to specific llvm-config path" OFF)
tvm_option(USE_STACKVM_RUNTIME "Include stackvm into the runtime" OFF)
tvm_option(USE_GRAPH_RUNTIME "Build with tiny graph runtime" ON)
tvm_option(USE_GRAPH_RUNTIME_DEBUG "Build with tiny graph runtime debug mode" OFF)
tvm_option(USE_OPENMP "Build with OpenMP thread pool implementation" OFF)
tvm_option(USE_RELAY_DEBUG "Building Relay in debug mode..." OFF)
tvm_option(USE_RTTI "Build with RTTI" ON)
tvm_option(USE_MSVC_MT "Build with MT" OFF)
tvm_option(USE_MICRO "Build with Micro TVM support" OFF)
tvm_option(INSTALL_DEV "Install compiler infrastructure" OFF)
tvm_option(HIDE_PRIVATE_SYMBOLS "Compile with -fvisibility=hidden." OFF)
tvm_option(USE_TF_TVMDSOOP "Build with TensorFlow TVMDSOOp" OFF)
tvm_option(USE_FALLBACK_STL_MAP "Use TVM's POD compatible Map" OFF)
tvm_option(USE_ETHOSN "Build with Arm Ethos-N" OFF)
tvm_option(INDEX_DEFAULT_I64 "Defaults the index datatype to int64" ON)

# 3rdparty libraries
tvm_option(DLPACK_PATH "Path to DLPACK" "3rdparty/dlpack/include")
tvm_option(DMLC_PATH "Path to DMLC" "3rdparty/dmlc-core/include")
tvm_option(RANG_PATH "Path to RANG" "3rdparty/rang/include")
tvm_option(COMPILER_RT_PATH "Path to COMPILER-RT" "3rdparty/compiler-rt")
tvm_option(PICOJSON_PATH "Path to PicoJSON" "3rdparty/picojson")

# Contrib library options
tvm_option(USE_BLAS "The blas library to be linked" none)
tvm_option(USE_MKL "MKL root path when use MKL blas" OFF)
tvm_option(USE_MKLDNN "Build with MKLDNN" OFF)
tvm_option(USE_DNNL_CODEGEN "Enable MKLDNN (DNNL) codegen" OFF)
tvm_option(USE_CUDNN "Build with cuDNN" OFF)
tvm_option(USE_CUBLAS "Build with cuBLAS" OFF)
tvm_option(USE_THRUST "Build with Thrust" OFF)
tvm_option(USE_MIOPEN "Build with ROCM:MIOpen" OFF)
tvm_option(USE_ROCBLAS "Build with ROCM:RoCBLAS" OFF)
tvm_option(USE_SORT "Build with sort support" OFF)
tvm_option(USE_NNPACK "Build with nnpack support" OFF)
tvm_option(USE_RANDOM "Build with random support" OFF)
tvm_option(USE_MICRO_STANDALONE_RUNTIME "Build with micro.standalone_runtime support" OFF)
tvm_option(USE_CPP_RPC "Build CPP RPC" OFF)
tvm_option(USE_TFLITE "Build with tflite support" OFF)
tvm_option(USE_TENSORFLOW_PATH "TensorFlow root path when use TFLite" none)
tvm_option(USE_COREML "Build with coreml support" OFF)
tvm_option(USE_TARGET_ONNX "Build with ONNX Codegen support" OFF)
tvm_option(USE_ARM_COMPUTE_LIB "Build with Arm Compute Library" OFF)
tvm_option(USE_ARM_COMPUTE_LIB_GRAPH_RUNTIME "Build with Arm Compute Library graph runtime" OFF)

# add source group
FILE(GLOB_RECURSE GROUP_SOURCE "src/*.cc")
FILE(GLOB_RECURSE GROUP_INCLUDE "src/*.h" "include/*.h")
assign_source_group("Source" ${GROUP_SOURCE})
assign_source_group("Include" ${GROUP_INCLUDE})

# Source file lists
file(GLOB_RECURSE COMPILER_SRCS
    src/auto_scheduler/*.cc
    src/node/*.cc
    src/ir/*.cc
    src/arith/*.cc
    src/te/*.cc
    src/autotvm/*.cc
    src/tir/*.cc
    src/topi/*.cc
    src/driver/*.cc
    src/parser/*.cc
    src/printer/*.cc
    src/support/*.cc
    )

file(GLOB CODEGEN_SRCS
  src/target/*.cc
  src/target/source/*.cc
    )

list(APPEND COMPILER_SRCS ${CODEGEN_SRCS})

file(GLOB_RECURSE RELAY_OP_SRCS
    src/relay/op/*.cc
    )
file(GLOB_RECURSE RELAY_PASS_SRCS
    src/relay/analysis/*.cc
    src/relay/transforms/*.cc
    src/relay/quantize/*.cc
    )
file(GLOB RELAY_BACKEND_SRCS
    src/relay/backend/*.cc
    src/relay/backend/vm/*.cc
    )
file(GLOB_RECURSE RELAY_IR_SRCS
    src/relay/ir/*.cc
    )
file(GLOB_RECURSE RELAY_QNN_SRCS
    src/relay/qnn/*.cc
)
list(APPEND COMPILER_SRCS ${RELAY_OP_SRCS})
list(APPEND COMPILER_SRCS ${RELAY_PASS_SRCS})
list(APPEND COMPILER_SRCS ${RELAY_BACKEND_SRCS})
list(APPEND COMPILER_SRCS ${RELAY_IR_SRCS})
list(APPEND COMPILER_SRCS ${RELAY_QNN_SRCS})


if(USE_VM_PROFILER)
  message(STATUS "Build compiler with Relay VM profiler support...")
  file(GLOB BACKEND_VM_PROFILER_SRCS src/relay/backend/vm/profiler/*.cc)
  list(APPEND COMPILER_SRCS ${BACKEND_VM_PROFILER_SRCS})
endif(USE_VM_PROFILER)

file(GLOB DATATYPE_SRCS src/target/datatype/*.cc)
list(APPEND COMPILER_SRCS ${DATATYPE_SRCS})

file(GLOB RUNTIME_SRCS
  src/runtime/*.cc
  src/runtime/vm/*.cc
)

list(APPEND RUNTIME_SRCS 3rdparty/bfloat16/bfloat16.cc)

if(USE_RPC)
  message(STATUS "Build with RPC support...")
  file(GLOB RUNTIME_RPC_SRCS src/runtime/rpc/*.cc)
  list(APPEND RUNTIME_SRCS ${RUNTIME_RPC_SRCS})
endif(USE_RPC)

file(GLOB STACKVM_RUNTIME_SRCS src/runtime/stackvm/*.cc)
file(GLOB STACKVM_CODEGEN_SRCS src/target/stackvm/*.cc)
list(APPEND COMPILER_SRCS ${STACKVM_CODEGEN_SRCS})
if(USE_STACKVM_RUNTIME)
  message(STATUS "Build with stackvm support in runtime...")
  list(APPEND RUNTIME_SRCS ${STACKVM_RUNTIME_SRCS})
else()
  list(APPEND COMPILER_SRCS ${STACKVM_RUNTIME_SRCS})
endif(USE_STACKVM_RUNTIME)

if(USE_GRAPH_RUNTIME)
  message(STATUS "Build with Graph runtime support...")
  file(GLOB RUNTIME_GRAPH_SRCS src/runtime/graph/*.cc)
  list(APPEND RUNTIME_SRCS ${RUNTIME_GRAPH_SRCS})

  if(USE_GRAPH_RUNTIME_DEBUG)
    message(STATUS "Build with Graph runtime debug support...")
    file(GLOB RUNTIME_GRAPH_DEBUG_SRCS src/runtime/graph/debug/*.cc)
    list(APPEND RUNTIME_SRCS ${RUNTIME_GRAPH_DEBUG_SRCS})
    set_source_files_properties(${RUNTIME_GRAPH_SRCS}
      PROPERTIES COMPILE_DEFINITIONS "TVM_GRAPH_RUNTIME_DEBUG")
  endif(USE_GRAPH_RUNTIME_DEBUG)
endif(USE_GRAPH_RUNTIME)

if(USE_VM_PROFILER)
  message(STATUS "Build with Relay VM profiler support...")
  file(GLOB RUNTIME_VM_PROFILER_SRCS src/runtime/vm/profiler/*.cc)
  list(APPEND RUNTIME_SRCS ${RUNTIME_VM_PROFILER_SRCS})
endif(USE_VM_PROFILER)

add_library(tvm_objs OBJECT ${COMPILER_SRCS})
add_library(tvm_runtime_objs OBJECT ${RUNTIME_SRCS})

# with cmake 3.12 we can just link to the object libraries themselves
add_library(tvm SHARED $<TARGET_OBJECTS:tvm_objs> $<TARGET_OBJECTS:tvm_runtime_objs>)
set_property(TARGET tvm APPEND PROPERTY LINK_OPTIONS "${TVM_VISIBILITY_FLAGS}")
add_library(tvm_runtime SHARED $<TARGET_OBJECTS:tvm_runtime_objs>)
set_property(TARGET tvm_runtime APPEND PROPERTY LINK_OPTIONS "${TVM_VISIBILITY_FLAGS}")


# with cmake 3.12 these are automatically propogated if we use target_link_libraries
# TODO: need to append, use set_property
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  INTERFACE_COMPILE_FEATURES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_COMPILE_FEATURES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_FEATURES>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  INTERFACE_COMPILE_OPTIONS
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_COMPILE_OPTIONS>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_OPTIONS>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  INTERFACE_COMPILE_DEFINITIONS
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_COMPILE_DEFINITIONS>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_DEFINITIONS>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  INTERFACE_SOURCES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_SOURCES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_SOURCES>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  INTERFACE_POSITION_INDEPENDENT_CODE
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_POSITION_INDEPENDENT_CODE>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_POSITION_INDEPENDENT_CODE>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  INTERFACE_LINK_LIBRARIES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_LINK_LIBRARIES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_LINK_LIBRARIES>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  COMPILE_FEATURES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_COMPILE_FEATURES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_FEATURES>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  COMPILE_OPTIONS
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_COMPILE_OPTIONS>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_OPTIONS>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  COMPILE_DEFINITIONS
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_COMPILE_DEFINITIONS>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_DEFINITIONS>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  SOURCES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_SOURCES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_SOURCES>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  SYSTEM_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  POSITION_INDEPENDENT_CODE
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_POSITION_INDEPENDENT_CODE>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_POSITION_INDEPENDENT_CODE>
    )
set_property(TARGET tvm tvm_runtime APPEND PROPERTY
  LINK_LIBRARIES
    $<TARGET_PROPERTY:tvm_objs,INTERFACE_LINK_LIBRARIES>
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_LINK_LIBRARIES>
    )

set_property(TARGET tvm_runtime APPEND PROPERTY
  INTERFACE_COMPILE_FEATURES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_FEATURES>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  INTERFACE_COMPILE_OPTIONS
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_OPTIONS>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  INTERFACE_COMPILE_DEFINITIONS
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_DEFINITIONS>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  INTERFACE_SOURCES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_SOURCES>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  INTERFACE_POSITION_INDEPENDENT_CODE
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_POSITION_INDEPENDENT_CODE>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  INTERFACE_LINK_LIBRARIES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_LINK_LIBRARIES>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  COMPILE_FEATURES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_FEATURES>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  COMPILE_OPTIONS
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_OPTIONS>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  COMPILE_DEFINITIONS
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_COMPILE_DEFINITIONS>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  SOURCES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_SOURCES>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  SYSTEM_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  POSITION_INDEPENDENT_CODE
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_POSITION_INDEPENDENT_CODE>
    )
set_property(TARGET tvm_runtime APPEND PROPERTY
  LINK_LIBRARIES
    $<TARGET_PROPERTY:tvm_runtime_objs,INTERFACE_LINK_LIBRARIES>
    )



add_library(tvm_deps INTERFACE) # dummy library to hold dependencies
# TODO(tkonolige): test against early version of cmake
# TODO(tkonolige): split public and private dependencies
target_link_libraries(tvm PUBLIC tvm_deps)
target_link_libraries(tvm_runtime PUBLIC tvm_deps)
# in cmake 3.13 we can just us link_libraries
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  INTERFACE_COMPILE_FEATURES
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_COMPILE_FEATURES>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  INTERFACE_COMPILE_OPTIONS
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_COMPILE_OPTIONS>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  INTERFACE_COMPILE_DEFINITIONS
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_COMPILE_DEFINITIONS>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  INTERFACE_SOURCES
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_SOURCES>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  INTERFACE_POSITION_INDEPENDENT_CODE
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_POSITION_INDEPENDENT_CODE>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  COMPILE_FEATURES
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_COMPILE_FEATURES>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  COMPILE_OPTIONS
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_COMPILE_OPTIONS>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  COMPILE_DEFINITIONS
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_COMPILE_DEFINITIONS>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  SOURCES
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_SOURCES>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  SYSTEM_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
    )
set_property(TARGET tvm_runtime_objs tvm_objs APPEND PROPERTY
  POSITION_INDEPENDENT_CODE
    $<TARGET_PROPERTY:tvm_deps,INTERFACE_POSITION_INDEPENDENT_CODE>
    )

# include directories
target_include_directories(tvm_deps INTERFACE "${CMAKE_INCLUDE_PATH}")
target_include_directories(tvm_deps INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                          $<INSTALL_INTERFACE:include>)
target_include_directories(tvm_deps SYSTEM INTERFACE "${DLPACK_PATH}")
target_include_directories(tvm_deps SYSTEM INTERFACE "${DMLC_PATH}")
target_include_directories(tvm_deps SYSTEM INTERFACE "${RANG_PATH}")
target_include_directories(tvm_deps SYSTEM INTERFACE "${COMPILER_RT_PATH}")
target_include_directories(tvm_deps SYSTEM INTERFACE "${PICOJSON_PATH}")

# Module rules
include(cmake/modules/VTA.cmake)
include(cmake/modules/StandaloneCrt.cmake)
include(cmake/modules/CUDA.cmake)
include(cmake/modules/Hexagon.cmake)
include(cmake/modules/OpenCL.cmake)
include(cmake/modules/OpenMP.cmake)
include(cmake/modules/Vulkan.cmake)
include(cmake/modules/Metal.cmake)
include(cmake/modules/ROCM.cmake)
include(cmake/modules/LLVM.cmake)
include(cmake/modules/Micro.cmake)
include(cmake/modules/contrib/EthosN.cmake)
include(cmake/modules/contrib/BLAS.cmake)
include(cmake/modules/contrib/CODEGENC.cmake)
include(cmake/modules/contrib/DNNL.cmake)
include(cmake/modules/contrib/Random.cmake)
include(cmake/modules/contrib/MicroStandaloneRuntime.cmake)
include(cmake/modules/contrib/Sort.cmake)
include(cmake/modules/contrib/NNPack.cmake)
include(cmake/modules/contrib/HybridDump.cmake)
include(cmake/modules/contrib/TFLite.cmake)
include(cmake/modules/contrib/TF_TVMDSOOP.cmake)
include(cmake/modules/contrib/CoreML.cmake)
include(cmake/modules/contrib/ONNX.cmake)
include(cmake/modules/contrib/ArmComputeLib.cmake)
include(cmake/modules/Git.cmake)
include(cmake/modules/LibInfo.cmake)

add_lib_info(${CMAKE_CURRENT_LIST_DIR}/src/support/libinfo.cc)

# Generic compilation options
if(MSVC)
  target_compile_definitions(tvm_deps INTERFACE WIN32_LEAN_AND_MEAN)
  target_compile_definitions(tvm_deps INTERFACE _CRT_SECURE_NO_WARNINGS)
  target_compile_definitions(tvm_deps INTERFACE _SCL_SECURE_NO_WARNINGS)
  target_compile_definitions(tvm_deps INTERFACE _ENABLE_EXTENDED_ALIGNED_STORAGE)
  target_compile_options(tvm_deps INTERFACE "/EHsc /MP /bigobj")
  # TODO(tkonolige): this should not touch globals
  if(USE_MSVC_MT)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
  endif()
  # Disable common MSVC warnings
  # Integer conversion warnings(e.g. int64 to int)
  target_compile_options(tvm_deps INTERFACE /wd4244)
  target_compile_options(tvm_deps INTERFACE /wd4267)
  # Signed unsigned constant comparison
  target_compile_options(tvm_deps INTERFACE /wd4018)
  # Aligned alloc may not met(need c++17)
  target_compile_options(tvm_deps INTERFACE /wd4316)
  # unreferenced local variables(usually in exception catch)
  target_compile_options(tvm_deps INTERFACE /wd4101)
  # always inline keyword not necessary
  target_compile_options(tvm_deps INTERFACE /wd4180)
  # DLL interface warning in c++
  target_compile_options(tvm_deps INTERFACE /wd4251)
else(MSVC)
  target_compile_options(tvm_deps INTERFACE "-Wall")
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(STATUS "Build in Debug mode")
  else()
    set(TVM_VISIBILITY_FLAG "")
    if (HIDE_PRIVATE_SYMBOLS)
      message(STATUS "Hide private symbols...")
      set(TVM_VISIBILITY_FLAG "-fvisibility=hidden")
    endif(HIDE_PRIVATE_SYMBOLS)
  endif ()
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND
      CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
    target_compile_options(tvm_deps "-faligned-new")
  endif()
  include(cmake/modules/ClangFlags.cmake)

  # Detect if we're compiling for Hexagon.
  set(TEST_FOR_HEXAGON_CXX
      "#ifndef __hexagon__"
      "#error"
      "#endif"
      "int main() {}"
      # Define _start_main to avoid linking errors with -fPIC.
      "extern \"C\" void _start_main() {}")
  set(TEST_FOR_HEXAGON_DIR
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
  set(TEST_FOR_HEXAGON_FILE "${TEST_FOR_HEXAGON_DIR}/test_for_hexagon.cc")
  string(REPLACE ";" "\n" TEST_FOR_HEXAGON_CXX_TEXT "${TEST_FOR_HEXAGON_CXX}")
  file(WRITE "${TEST_FOR_HEXAGON_FILE}" "${TEST_FOR_HEXAGON_CXX_TEXT}")
  try_compile(BUILD_FOR_HEXAGON "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}"
              "${TEST_FOR_HEXAGON_FILE}")
  file(REMOVE "${TEST_FOR_HEXAGON_FILE}")
  if(BUILD_FOR_HEXAGON)
    message(STATUS "Building for Hexagon")
  endif()

  # Detect if we're compiling for Android.
  set(TEST_FOR_ANDROID_CXX
      "#ifndef __ANDROID__"
      "#error"
      "#endif"
      "int main() {}")
  set(TEST_FOR_ANDROID_DIR
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
  set(TEST_FOR_ANDROID_FILE "${TEST_FOR_ANDROID_DIR}/test_for_android.cc")
  string(REPLACE ";" "\n" TEST_FOR_ANDROID_CXX_TEXT "${TEST_FOR_ANDROID_CXX}")
  file(WRITE "${TEST_FOR_ANDROID_FILE}" "${TEST_FOR_ANDROID_CXX_TEXT}")
  try_compile(BUILD_FOR_ANDROID "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}"
              "${TEST_FOR_ANDROID_FILE}")
  file(REMOVE "${TEST_FOR_ANDROID_FILE}")
  if(BUILD_FOR_ANDROID)
    message(STATUS "Building for Android")
  endif()
endif(MSVC)

if(BUILD_FOR_HEXAGON)
  # Add file implementing posix_memalign.
  target_sources(tvm_runtime PRIVATE src/runtime/hexagon/hexagon_posix.cc)

  target_compile_definitions(tvm_deps INTERFACE _MACH_I32=int)
endif()

# Package runtime rules
if(NOT USE_RTTI)
  target_compile_definitions(tvm_deps INTERFACE DMLC_ENABLE_RTTI=0)
endif()

if (INDEX_DEFAULT_I64)
  target_compile_definitions(tvm_deps INTERFACE TVM_INDEX_DEFAULT_I64=1)
endif()

if(USE_CUDA)
  enable_language(CUDA)
  target_compile_options(tvm_deps INTERFACE cuda_std_14)
endif()

# C++14 support
target_compile_features(tvm_deps INTERFACE cxx_std_14)

if(USE_MICRO)
  # NOTE: cmake doesn't track dependencies at the file level across subdirectories. For the
  # Unix Makefiles generator, need to add these explicit target-level dependency)
  add_dependencies(tvm host_standalone_crt)
  add_dependencies(tvm_runtime host_standalone_crt)
endif()

if(USE_CPP_RPC)
  add_subdirectory("apps/cpp_rpc")
endif()

if(USE_RELAY_DEBUG)
  message(STATUS "Building Relay in debug mode...")
  target_compile_definitions(tvm_objs PRIVATE "USE_RELAY_DEBUG")
  target_compile_definitions(tvm_objs PRIVATE "DMLC_LOG_DEBUG")
else()
  target_compile_definitions(tvm_objs PRIVATE "NDEBUG")
endif(USE_RELAY_DEBUG)

if(USE_FALLBACK_STL_MAP)
  message(STATUS "Building with STL Map...")
  target_compile_definitions(tvm_objs PRIVATE "USE_FALLBACK_STL_MAP=1")
else()
  message(STATUS "Building with TVM Map...")
  target_compile_definitions(tvm_objs PRIVATE "USE_FALLBACK_STL_MAP=0")
endif(USE_FALLBACK_STL_MAP)

if(BUILD_FOR_HEXAGON)
  # Wrap pthread_create to allow setting custom stack size.
  set_target_properties(tvm_runtime PROPERTIES LINK_FLAGS
                        "-Wl,--wrap=pthread_create")

  target_include_directories(tvm_runtime
    PUBLIC "${USE_HEXAGON_SDK}/libs/common/qurt/ADSPv62MP/include/posix"
    PUBLIC "${USE_HEXAGON_SDK}/libs/common/qurt/ADSPv62MP/include/qurt"
    PUBLIC "${USE_HEXAGON_SDK}/incs"
    PUBLIC "${USE_HEXAGON_SDK}/incs/stddef")
endif()

if(USE_THREADS AND NOT BUILD_FOR_HEXAGON)
  message(STATUS "Build with thread support...")
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(tvm PUBLIC Threads::Threads)
  target_link_libraries(tvm_runtime PUBLIC Threads::Threads)
endif()

# Hexagon has dlopen built into QuRT (no need for static library).
if(NOT BUILD_FOR_HEXAGON)
  target_link_libraries(tvm_runtime PRIVATE ${CMAKE_BL_LIBS})
endif()

if(BUILD_FOR_ANDROID)
  # EmuTLS on Android is in libgcc. Without it linked in, libtvm_runtime.so
  # won't load on Android due to missing __emutls_XXX symbols.
  target_link_libraries(tvm_runtime PRIVATE "gcc")
endif()

# Related headers
target_include_directories(
  tvm
  PUBLIC "topi/include")
target_include_directories(
  tvm_objs
  PUBLIC "topi/include")
set(CRC16_INCLUDE_PATH "3rdparty/libcrc/include")
target_include_directories(
  tvm_objs
  PRIVATE "${CRC16_INCLUDE_PATH}")
target_include_directorieS(
  tvm_runtime_objs
  PRIVATE "${CRC16_INCLUDE_PATH}")

set(TVM_TEST_LIBRARY_NAME tvm)
if (HIDE_PRIVATE_SYMBOLS AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_library(tvm_allvisible SHARED $<TARGET_OBJECTS:tvm_objs>)
  target_include_directories(tvm_allvisible PUBLIC "$<TARGET_PROPERTY:tvm,INCLUDE_DIRECTORIES>")
  target_link_libraries(tvm_allvisible PRIVATE "$<TARGET_PROPERTY:tvm,LINK_LIBRARIES>")
  set(TVM_TEST_LIBRARY_NAME tvm_allvisible)

  set(HIDE_SYMBOLS_LINKER_FLAGS "-Wl,--exclude-libs,ALL")
  # Note: 'target_link_options' with 'PRIVATE' keyword would be cleaner
  # but it's not available until CMake 3.13. Switch to 'target_link_options'
  # once minimum CMake version is bumped up to 3.13 or above.
  set_target_properties(tvm_runtime PROPERTIES LINK_FLAGS ${HIDE_SYMBOLS_LINKER_FLAGS})
  set_target_properties(tvm PROPERTIES LINK_FLAGS ${HIDE_SYMBOLS_LINKER_FLAGS})
endif()

# Tests
set(TEST_EXECS "")
file(GLOB TEST_SRCS tests/cpp/*.cc)
find_path(GTEST_INCLUDE_DIR gtest/gtest.h)
find_library(GTEST_LIB gtest "$ENV{GTEST_LIB}")

# Create the `cpptest` target if we can find GTest.  If not, we create dummy
# targets that give the user an informative error message.
if(GTEST_INCLUDE_DIR AND GTEST_LIB)
  foreach(__srcpath ${TEST_SRCS})
    get_filename_component(__srcname ${__srcpath} NAME)
    string(REPLACE ".cc" "" __execname ${__srcname})
    add_executable(${__execname} ${__srcpath})
    list(APPEND TEST_EXECS ${__execname})
    target_include_directories(${__execname} PUBLIC ${GTEST_INCLUDE_DIR})
    target_link_libraries(${__execname} ${TVM_TEST_LIBRARY_NAME} ${GTEST_LIB} pthread dl)
    set_target_properties(${__execname} PROPERTIES EXCLUDE_FROM_ALL 1)
    set_target_properties(${__execname} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)
  endforeach()
  add_custom_target(cpptest DEPENDS ${TEST_EXECS})
elseif(NOT GTEST_INCLUDE_DIR)
  add_custom_target(cpptest
      COMMAND echo "Missing Google Test headers in include path"
      COMMAND exit 1)
elseif(NOT GTEST_LIB)
  add_custom_target(cpptest
      COMMAND echo "Missing Google Test library"
      COMMAND exit 1)
endif()

# Custom targets
add_custom_target(runtime DEPENDS tvm_runtime)

# Installation rules
install(TARGETS tvm DESTINATION lib${LIB_SUFFIX})
install(TARGETS tvm_runtime DESTINATION lib${LIB_SUFFIX})

if (INSTALL_DEV)
  install(
    DIRECTORY "include/." DESTINATION "include"
    FILES_MATCHING
    PATTERN "*.h"
  )
  install(
    DIRECTORY "3rdparty/dlpack/include/." DESTINATION "include"
    FILES_MATCHING
    PATTERN "*.h"
    )
  install(
    DIRECTORY "3rdparty/dmlc-core/include/." DESTINATION "include"
    FILES_MATCHING
    PATTERN "*.h"
    )
else(INSTALL_DEV)
  install(
    DIRECTORY "include/tvm/runtime/." DESTINATION "include/tvm/runtime"
    FILES_MATCHING
    PATTERN "*.h"
    )
endif(INSTALL_DEV)

# More target definitions
# TODO(tkonolige): TVM_EXPORTS doesn't seem to be defined anywhere
if(MSVC)
  target_compile_definitions(tvm_objs PRIVATE TVM_EXPORTS)
  target_compile_definitions(tvm_runtime_objs PRIVATE TVM_EXPORTS)
endif()
