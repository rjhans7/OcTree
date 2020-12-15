cmake_minimum_required(VERSION 3.17)
project(Octree)

set(CMAKE_CXX_STANDARD 17)

find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})

find_package( Threads )
add_executable(main main.cpp octree.h ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
