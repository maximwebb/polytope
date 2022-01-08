cmake_minimum_required(VERSION 3.16)
project(polytope-pass)
#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 20)
find_package(Clang REQUIRED)

set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${LLVM_CMAKE_DIR}"
  )
include(AddLLVM)

include_directories(${CMAKE_SOURCE_DIR})

if( NOT LLVM_REQUIRES_RTTI )
  if( NOT LLVM_REQUIRES_EH )
    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Polytope.exports)
  endif()
endif()

set(CMAKE_CXX_FLAGS "-fPIE")
set(FOO "BAR")

add_library(polytope-pass SHARED Polytope.cpp)
target_include_directories(
  polytope-pass
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/"
)