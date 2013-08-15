find_package (LibAVCodec 53.35.0 REQUIRED)
find_package (LibAVFormat 53.21.1 REQUIRED)
find_package (LibAVUtil 51.22.1 REQUIRED)
find_package (LibSWScale 2.1.0 REQUIRED)

include_directories (${LIBAVCODEC_INCLUDE_DIRS})
include_directories (${LIBAVFORMAT_INCLUDE_DIRS})
include_directories (${LIBAVUTIL_INCLUDE_DIRS})
include_directories (${LIBSWSCALE_INCLUDE_DIRS})

link_directories (${LIBAVCODEC_LIBRARY_DIRS})
link_directories (${LIBAVFORMAT_LIBRARY_DIRS})
link_directories (${LIBAVUTIL_LIBRARY_DIRS})
link_directories (${LIBSWSCALE_LIBRARY_DIRS})

# __STDC_CONSTANT_MACROS is necessary for libav on Linux
add_definitions (-D__STDC_CONSTANT_MACROS)

list (APPEND bluecherry_client_LIBRARIES
    ${LIBAVCODEC_LIBRARIES}
    ${LIBAVFORMAT_LIBRARIES}
    ${LIBAVUTIL_LIBRARIES}
    ${LIBSWSCALE_LIBRARIES}
)

get_filename_component (LIBAVCODEC_RPATH ${LIBAVCODEC_LIBRARY} PATH)
get_filename_component (LIBAVFORMAT_RPATH ${LIBAVFORMAT_LIBRARY} PATH)
get_filename_component (LIBAVUTIL_RPATH ${LIBAVUTIL_LIBRARY} PATH)
get_filename_component (LIBSWSCALE_RPATH ${LIBSWSCALE_LIBRARY} PATH)

if (LINUX)
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBAVCODEC_RPATH}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBAVCODEC_LIBAVFORMAT}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBAVUTIL_RPATH}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBSWSCALE_RPATH}")
endif (LINUX)
