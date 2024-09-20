set(GARFIELDPP_UPSTREAM FALSE) # Use the gitlab Garfield++
set(GARFIELDPP_URL "https://gitlab.cern.ch/garfield/garfieldpp.git")
set(GARFIELDPP_TAG "5.0") #maybe use master

function(FetchGarfield)
  include(FetchContent)
  message(STATUS "Fetching ${GARFIELDPP_URL} with tag ${GARFIELDPP_TAG}")
  set(ENV{GARFIELD_INSTALL} "${CMAKE_INSTALL_PREFIX}")
  FetchContent_Declare(Garfield GIT_REPOSITORY "${GARFIELDPP_URL}" GIT_TAG "${GARFIELDPP_TAG}" GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(Garfield)
endfunction()

if(GARFIELDPP_UPSTREAM)
  FetchGarfield()
else()
  find_package(Garfield)
  if(NOT Garfield_FOUND)
    FetchGarfield()
  endif()
endif()

include(FetchContent)
FetchContent_Declare(cli11 GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git GIT_TAG v2.4.2 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cli11)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.14.1 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(EcoMug GIT_REPOSITORY https://github.com/dr4kan/EcoMug.git GIT_TAG v2.1 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(EcoMug)
add_library(EcoMug INTERFACE)
target_include_directories(EcoMug INTERFACE "${ecomug_SOURCE_DIR}")


configure_file("${CMAKE_CURRENT_SOURCE_DIR}/DefaultFolders.hpp.in" "${PROJECT_BINARY_DIR}/include/RPCSim/DefaultFolders.hpp")
add_library(RPCSim STATIC
new/GasMixture.cpp
new/WebServer.cpp
#Exceptions.cpp
#Medium.cpp
#GasMixture.cpp
#PCBGeometry.cpp
#RPCGeometry.cpp
#Setup.cpp
#SignalPlotter.cpp
#Webserver.cpp
#Websocket.cpp
)
target_link_libraries(RPCSim PUBLIC Garfield::Garfield ROOT::RHTTP spdlog::spdlog)
target_link_directories(RPCSim PUBLIC "${CMAKE_INSTALL_FULL_LIBDIR}")
target_include_directories(RPCSim PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include;${PROJECT_BINARY_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
set_target_properties(RPCSim PROPERTIES PUBLIC_HEADER 
"${PROJECT_BINARY_DIR}/include/RPCSim/DefaultFolders.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/Webserver.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/SignalPlotter.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/GasMixture.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/Setup.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/Exceptions.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/Dimensions.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/RPCGeometry.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/PCBGeometry.hpp"
)
install(TARGETS RPCSim
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/RPCSim"
)
add_library(RPCSim::RPCSim ALIAS RPCSim)