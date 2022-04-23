cmake_minimum_required(VERSION 3.20)
project(compiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS "-lclang")

find_package(Clang REQUIRED)

# Add path to LLVM modules
set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${LLVM_CMAKE_DIR}"
  )

# import LLVM CMake functions
include(AddLLVM)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(sandbox)

add_definitions(${LLVM_DEFINITIONS})
add_definitions(${CLANG_DEFINITIONS})

file(GLOB SANDBOX
        sandbox/*.h)

add_llvm_executable(compiler ${SANDBOX} clang_ast.h main.cpp)
set_property(TARGET compiler PROPERTY CXX_STANDARD 20)
target_link_libraries(compiler PRIVATE clangTooling)