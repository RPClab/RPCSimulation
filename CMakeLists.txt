cmake_minimum_required(VERSION 3.17 FATAL_ERROR) # 3.17 is the version available on the server

project(RPCSimulation VERSION "0.1.0" DESCRIPTION "Simulation of RPCs" HOMEPAGE_URL "https://github.com/RPClab/RPCSimulation" LANGUAGES CXX)

option(RPC_SIMULATION_TESTS "Build tests" ON)
option(RPC_SIMULATION_EXAMPLES "Build examples" ON)
option(RPC_SIMULATION_DOCS "Build documentations" ON)

if(RPC_SIMULATION_TESTS)
  add_subdirectory(tests)
endif()

if(RPC_SIMULATION_EXAMPLES)
  add_subdirectory(examples)
endif()

if(RPC_SIMULATION_DOCS)
  add_subdirectory(docs)
endif()

add_subdirectory(src)

add_subdirectory(bin)