# Set output to same directory as the other TVM libs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(tvm_vm main.cc)
set_property(TARGET tvm_rpc PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

target_include_directories(
  tvm_vm
  PUBLIC "../../include"
  PUBLIC DLPACK_PATH
  PUBLIC DMLC_PATH
)

target_link_libraries(tvm_vm tvm_runtime)
