configure_file (
  "cmake_config.h.in"
  "cmake_config.h"
  )
include_directories ("${CMAKE_CURRENT_BINARY_DIR}")


add_executable(overlap-analysis overlap-analysis.cpp)
add_executable(sound-match sound_match.cpp sound_match.h)
add_executable(waveform-compare waveform-compare.cpp)
#add_executable(ismir_impl ismir_impl.cc)
add_executable(ismir_query ismir_query.cc)
add_executable(ismir_build_index ismir_build_index.cc)
add_executable(ismir_merge ismir_merge.cc)

#add_executable(revindex revindex.cc)

#add_executable(mything mything.cc)

#add_executable(test test.cc)

#target_link_libraries(test ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVUTIL_LIBRARIES} chromaprint)
target_link_libraries(overlap-analysis core ${Boost_LIBRARIES})
target_link_libraries(sound-match core ${Boost_LIBRARIES})
target_link_libraries(waveform-compare core ${Boost_LIBRARIES})
#target_link_libraries(ismir_impl core sound_index ${Boost_LIBRARIES})
target_link_libraries(ismir_query core sound_index ${Boost_LIBRARIES})
target_link_libraries(ismir_build_index core sound_index ${Boost_LIBRARIES})
target_link_libraries(ismir_merge core sound_index ${Boost_LIBRARIES})

#target_link_libraries(revindex tpie)

#target_link_libraries(mything tpie)

install(TARGETS overlap-analysis sound-match waveform-compare RUNTIME DESTINATION bin)
