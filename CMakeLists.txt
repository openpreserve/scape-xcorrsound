cmake_minimum_required (VERSION 2.6) 
project (scape-xcorrsound)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules/")

set(CPACK_STRIP_FILES "apps/")

find_package(Boost
  COMPONENTS program_options)

find_package(FFTW)

include_directories(${FFTW_INCLUDES} ${Boost_INCLUDE_DIR} libs/core libs/sound_index)

find_program(GZIP_TOOL
             NAMES gzip
             PATHS /bin
                   /usr/bin
                   /usr/local/bin)
if(NOT GZIP_TOOL)
  message(FATAL_ERROR "Unable to find 'gzip' program")
endif(NOT GZIP_TOOL)

find_program(RONN_TOOL
             NAMES ronn
             PATHS /bin
                   /usr/bin
                   /usr/local/bin)
if(NOT RONN_TOOL)
  message(FATAL_ERROR "Unable to find 'ronn' program")
endif(NOT RONN_TOOL)






set(CPACK_PACKAGE_NAME scape-xcorrsound)
set(CPACK_PACKAGE_VERSION 2.0.2)
set(COPYRIGHT_HOLDER "State and University Library")

# Debian package stuff
set(CPACK_DEBIAN_PACKAGE_NAME        ${CPACK_PACKAGE_NAME})
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libboost-all-dev, libfftw3-dev")
set(CPACK_PACKAGE_CONTACT "Jesper Sindahl Nielsen <jsn@statsbiblioteket.dk>")
set(CPACK_DEBIAN_PACKAGE_PRIORITY    "optional")
set(CPACK_DEBIAN_PACKAGE_SECTION     "devel")
#file(READ ${CMAKE_CURRENT_SOURCE_DIR}/README.md README)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY 
"This is a package of the toolsuite scape-xcorrsound.
 It contains three tools
  * waveform-compare:
 Compare two wave files to determine if they represent the same waveforms
  * sound-match:
 find the location of a small wavefile inside a big wavefile
  * overlap-analysis:
 find the overlap between two wave files")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
        DESTINATION "share/doc/${CPACK_DEBIAN_PACKAGE_NAME}"
        RENAME copyright)


#The changelog
add_custom_command(OUTPUT changelog.gz
                     COMMAND ${GZIP_TOOL} --best -c ${CMAKE_CURRENT_SOURCE_DIR}/changelog > ${CMAKE_CURRENT_BINARY_DIR}/changelog.gz
                     DEPENDS changelog
                     COMMENT "Building changelog.gz")
add_custom_target(changelog ALL DEPENDS changelog.gz)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/changelog.gz DESTINATION share/doc/${CPACK_DEBIAN_PACKAGE_NAME})


add_subdirectory (libs)
add_subdirectory (apps)
add_subdirectory (man)

INCLUDE(CPack)
SET(CPACK_GENERATOR "DEB")

