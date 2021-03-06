cmake_minimum_required(VERSION 2.6)
project(PathPlanner)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
add_subdirectory(RRT)
add_subdirectory(Astar)

IF(SPLINES)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPLINES")
    message(STATUS "Using BSplines planner")
    add_subdirectory(BSplines)
ENDIF(SPLINES)

set(SOURCE_FILES PathPlanner.cpp AstarPlan.cpp GridAstarPlan.cpp RRTPlan.cpp Dynamics.cpp)

IF(SPLINES)
    set(SOURCE_FILES ${SOURCE_FILES} SplinesPlan.cpp)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/BSplines)
ENDIF(SPLINES)


set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../ACCoRD/inc
                    ${CMAKE_CURRENT_SOURCE_DIR}/../Utils
                    ${CMAKE_CURRENT_SOURCE_DIR}/RRT
                    ${CMAKE_CURRENT_SOURCE_DIR}/Astar)

link_directories(${LIBRARY_OUTPUT_PATH})

add_library(PathPlanner SHARED ${SOURCE_FILES})

target_link_libraries(PathPlanner RRT ASTAR ACCoRD Utils)

IF(SPLINES)
    target_link_libraries(PathPlanner BSPLINES)
ENDIF(SPLINES)

add_executable(PathTest Test/main.cpp)
target_link_libraries(PathTest PathPlanner)
