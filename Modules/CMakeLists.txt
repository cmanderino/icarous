cmake_minimum_required(VERSION 2.6.4)
project(ICAROUS_MODULES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_subdirectory(ACCoRD)
add_subdirectory(Core/GeofenceMonitor)
add_subdirectory(Core/TrafficMonitor)
add_subdirectory(Core/TrajectoryPlanners)
add_subdirectory(Core/Utils)
add_subdirectory(Core/Guidance)
add_subdirectory(Core/Merger)
add_subdirectory(Core/Cognition)
add_subdirectory(Core/QuadCopterSim)


