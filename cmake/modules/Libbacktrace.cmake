include(ExternalProject)

ExternalProject_Add(project_libbacktrace
  PREFIX libbacktrace
  GIT_REPOSITORY https://github.com/ianlancetaylor/libbacktrace.git
  GIT_TAG 1da441c1b020bef75d040dd300814b1a49e0e529
  CONFIGURE_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace/src/project_libbacktrace/configure" "--prefix=${CMAKE_CURRENT_BINARY_DIR}/libbacktrace"
  INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace"
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace/lib/libbacktrace.a" "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace/include/backtrace.h"
  )

# rebuild libbacktrace when its source files change
file(GLOB libbacktrace_srcs 3rdparty/libbacktrace/*)
ExternalProject_Add_Step(project_libbacktrace sources DEPENDERS configure DEPENDS "${libbacktrace_srcs}")

ExternalProject_Get_Property(project_libbacktrace install_dir)
add_library(libbacktrace STATIC IMPORTED)
add_dependencies(libbacktrace project_libbacktrace)
set_property(TARGET libbacktrace PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libbacktrace.a)
file(MAKE_DIRECTORY ${install_dir}/include) # create include directory so cmake doesn't complain
target_include_directories(libbacktrace INTERFACE $<BUILD_INTERFACE:${install_dir}/include>)
