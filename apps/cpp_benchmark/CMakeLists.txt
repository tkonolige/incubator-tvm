cmake_policy(SET CMP0069 NEW) # suppress cmake warning about IPO


# Set output to same directory as the other TVM libs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(tvm_benchmark main.cc)

# target_include_directories(
#   tvm_rpc
#   PUBLIC "../../include"
#   PUBLIC DLPACK_PATH
#   PUBLIC DMLC_PATH
# )

target_link_libraries(tvm_benchmark PUBLIC tvm_runtime)
