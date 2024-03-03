cmake_minimum_required(VERSION 3.10)
project(MyClangTooling)

find_package(Clang REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(ParFun ParseFunction.cpp)

target_link_libraries(ParFun
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  )
