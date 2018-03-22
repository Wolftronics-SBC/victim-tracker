cmake_minimum_required(VERSION 2.8)
project(EMILYVictimTracker)
set(CMAKE_CXX_STANDARD 11)
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})
file(GLOB SOURCES
    *.h
    *.cpp
)
add_executable(EMILYVictimTracker ${SOURCES})
target_link_libraries(EMILYVictimTracker ${OpenCV_LIBS})