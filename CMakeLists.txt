cmake_minimum_required(VERSION 3.2)
PROJECT(indi_rotctl CXX C)

include(GNUInstallDirs)

SET(CMAKE_CXX_STANDARD 11)

if (NOT INDI_SOURCE_DIRECTORY)
    message(FATAL_ERROR, "Variable INDI_SOURCE_DIRECTORY is not set. Rerun with cmake -D INDI_SOURCE_DIRECTORY=[path to e.g. git clone of INDI source code directory (needed for CMake modules)]")
endif()
LIST(APPEND CMAKE_MODULE_PATH "${INDI_SOURCE_DIRECTORY}/cmake_modules/") #needed for finding INDI

find_package(INDI REQUIRED)
include(CMakeCommon)
include_directories(${INDI_INCLUDE_DIR}) #needed for internal includes within INDI headers

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/indi_rotctl.xml.in ${CMAKE_CURRENT_BINARY_DIR}/indi_rotctl.xml)

add_executable(indi_rotctl indi_rotctl.cpp rotctld_communication.cpp coordinate_conversion.cpp)
target_link_libraries(indi_rotctl ${INDI_LIBRARIES} nova)

install(TARGETS indi_rotctl RUNTIME DESTINATION bin)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/indi_rotctl.xml DESTINATION ${INDI_DATA_DIR})
