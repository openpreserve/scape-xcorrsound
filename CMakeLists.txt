cmake_minimum_required (VERSION 2.6) 
project (scape-xcorrsound)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules/")

SET(CPACK_STRIP_FILES "apps/")

find_package(Boost
  COMPONENTS program_options)

find_package(FFTW)

FIND_PROGRAM(GZIP_TOOL
             NAMES gzip
             PATHS /bin
                   /usr/bin
                   /usr/local/bin)


IF(NOT GZIP_TOOL)
  MESSAGE(FATAL_ERROR "Unable to find 'gzip' program") 
ENDIF(NOT GZIP_TOOL)


#find_package (LibAVCodec 53.35.0 REQUIRED)
#find_package (LibAVFormat 53.21.1 REQUIRED)
#find_package (LibAVUtil 51.22.1 REQUIRED)

#include_directories(${FFTW_INCLUDES} ${Boost_INCLUDE_DIR} libs/core libs/sound_index tpie ${CMAKE_CURRENT_BINARY_DIR}/tpie)
include_directories(${FFTW_INCLUDES} ${Boost_INCLUDE_DIR} libs/core libs/sound_index)

#include_directories (${LIBAVCODEC_INCLUDE_DIRS})
#include_directories (${LIBAVFORMAT_INCLUDE_DIRS})
#include_directories (${LIBAVUTIL_INCLUDE_DIRS})

## testing
#include_directories (/home/jsn/chromaprint-0.7/build/src/Headers)

#link_directories (${LIBAVCODEC_LIBRARY_DIRS})
#link_directories (${LIBAVFORMAT_LIBRARY_DIRS})
#link_directories (${LIBAVUTIL_LIBRARY_DIRS})


#link_directories (/home/jsn/chromaprint-0.7/build/src)
#list (APPEND CHROMA_LIBS chromaprint)
#add_definitions (-D__STDC_CONSTANT_MACROS)

#get_filename_component (LIBAVCODEC_RPATH ${LIBAVCODEC_LIBRARY} PATH)
#get_filename_component (LIBAVFORMAT_RPATH ${LIBAVFORMAT_LIBRARY} PATH)
#get_filename_component (LIBAVUTIL_RPATH ${LIBAVUTIL_LIBRARY} PATH)
## testing

#if (LINUX)
#    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBAVCODEC_RPATH}")
#    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBAVCODEC_LIBAVFORMAT}")
#    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBAVUTIL_RPATH}")
#    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBSWSCALE_RPATH}")
#endif (LINUX)

set(CPACK_PACKAGE_NAME scape-xcorrsound)
set(CPACK_PACKAGE_VERSION 2.0.0)
set(COPYRIGHT_HOLDER "State and University Library")

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


#add_subdirectory (tpie)
add_subdirectory (libs)
add_subdirectory (apps)
add_subdirectory (man)

#add_executable(tpietest tpietest.cc)
#target_link_libraries(tpietest tpie)

INCLUDE(CPack)
SET(CPACK_GENERATOR "DEB")
