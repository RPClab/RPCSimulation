cmake_minimum_required(VERSION 3.29 FATAL_ERROR) # 3.17 is the version available on the server

project(RPCSimulation VERSION "0.1.0" DESCRIPTION "Simulation of RPCs" HOMEPAGE_URL "https://github.com/RPClab/RPCSimulation" LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE PATH "Default installation folder" FORCE)
  message(STATUS "RPCSim will be installed in ${CMAKE_INSTALL_PREFIX}")
endif()

include(GNUInstallDirs)

option(RPC_SIMULATION_TESTS "Build tests" ON)
option(RPC_SIMULATION_EXAMPLES "Build examples" ON)
option(RPC_SIMULATION_DOCS "Build documentations" ON)

set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")


# Where to install data
set(DATA_FOLDER "${CMAKE_INSTALL_FULL_DATADIR}/data")

add_subdirectory(src)

add_subdirectory(bin)

if(RPC_SIMULATION_TESTS)
  add_subdirectory(tests)
endif()

if(RPC_SIMULATION_EXAMPLES)
  add_subdirectory(examples)
endif()

if(RPC_SIMULATION_DOCS)
  add_subdirectory(docs)
endif()
