# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) StackBlender LLC
cmake_minimum_required(VERSION 3.16)
project(snes_st010 CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(st010 STATIC src/st010/St010.cpp)
target_include_directories(st010 PUBLIC src)

add_executable(st010_check tools/st010_check.cpp)
target_link_libraries(st010_check PRIVATE st010)

enable_testing()
add_executable(st010_test tests/st010_test.cpp)
target_link_libraries(st010_test PRIVATE st010)
add_test(NAME st010 COMMAND st010_test)
