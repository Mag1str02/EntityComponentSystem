cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Examples)

add_executable(Test test/Test.cpp)
target_include_directories(Test PUBLIC include)