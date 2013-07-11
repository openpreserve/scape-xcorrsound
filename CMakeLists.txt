cmake_minimum_required (VERSION 2.6) 
project (scape-xcorrsound)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules/")

find_package(Boost
  COMPONENTS program_options)

find_package(FFTW)

#include_directories(${FFTW_INCLUDES} ${Boost_INCLUDE_DIR} libs/core libs/sound_index tpie ${CMAKE_CURRENT_BINARY_DIR}/tpie)
include_directories(${FFTW_INCLUDES} ${Boost_INCLUDE_DIR} libs/core libs/sound_index)

set(CPACK_PACKAGE_NAME scape-xcorrsound)
set(CPACK_PACKAGE_VERSION 0.9.1)

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libboost-program-options-dev, libfftw3-dev")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jesper Sindahl Nielsen <jsn@statsbiblioteket.dk>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY 
"This is a package of the toolsuite scape-xcorrsound. It contains three tools
waveform-compare: Compare two wave files to determine if they represent the same waveforms
sound-match: find the location of a small wavefile inside a big wavefile
overlap-analysis: find the overlap between two wave files")

#add_subdirectory (tpie)
add_subdirectory (libs)
add_subdirectory (apps)
add_subdirectory (man)

#add_executable(tpietest tpietest.cc)
#target_link_libraries(tpietest tpie)

INCLUDE(CPack)
SET(CPACK_GENERATOR "DEB")
