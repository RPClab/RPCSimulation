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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/DefaultFolders.hpp.in" "${PROJECT_BINARY_DIR}/include/RPCSim/DefaultFolders.hpp")
add_library(RPCSim STATIC
Exceptions.cpp
Medium.cpp
GasMixture.cpp
RPCGeometry.cpp
Setup.cpp
)
target_link_libraries(RPCSim PUBLIC Garfield::Garfield)
target_link_directories(RPCSim PUBLIC "${CMAKE_INSTALL_FULL_LIBDIR}")
target_include_directories(RPCSim PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include;${PROJECT_BINARY_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
set_target_properties(RPCSim PROPERTIES PUBLIC_HEADER 
"${PROJECT_BINARY_DIR}/include/RPCSim/DefaultFolders.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/GasMixture.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/Setup.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/Exceptions.hpp;${PROJECT_SOURCE_DIR}/include/RPCSim/RPCGeometry.hpp"
)
install(TARGETS RPCSim
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/RPCSim"
)
add_library(RPCSim::RPCSim ALIAS RPCSim)