cmake_minimum_required(VERSION 3.6)
project(CImgProject)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp octree.h structs.h)
add_executable(CImgProject ${SOURCE_FILES})

# You can alter these according to your needs, e.g if you don't need to display images - set(YOU_NEED_X11 0)
set(YOU_NEED_X11 1)
set(YOU_NEED_PNG 1)
set(YOU_NEED_JPG 1)
set(YOU_NEED_THREADS 1)

SET(CMAKE_CXX_FLAGS "-fopenmp")


if(${YOU_NEED_X11} EQUAL 1)
    message(STATUS "Looking for X11...")
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
    target_link_libraries(CImgProject ${X11_LIBRARIES})
else()
    target_compile_definitions(CImgProject PRIVATE cimg_display=0)
endif()

if(${YOU_NEED_JPG} EQUAL 1)
    message(STATUS "Looking for libjpg...")
    find_package(JPEG REQUIRED)
    include_directories(${JPEG_INCLUDE_DIR})
    target_link_libraries (CImgProject ${JPEG_LIBRARY})
    target_compile_definitions(CImgProject PRIVATE cimg_use_jpeg=1)
endif()

if(${YOU_NEED_PNG} EQUAL 1)
    message(STATUS "Looking for libpng...")
    find_package(PNG REQUIRED)
    include_directories(${PNG_INCLUDE_DIR})
    target_link_libraries (CImgProject ${PNG_LIBRARY})
    target_compile_definitions(CImgProject PRIVATE cimg_use_png=1)
endif()

if(${YOU_NEED_THREADS} EQUAL 1)
    message(STATUS "Looking for threads...")
    find_package(Threads REQUIRED)
    target_link_libraries(CImgProject ${CMAKE_THREAD_LIBS_INIT})
endif()