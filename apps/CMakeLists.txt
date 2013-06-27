add_executable(overlap-analysis overlap-analysis.cpp)
add_executable(sound-match sound_match.cpp sound_match.h)
add_executable(waveform-compare waveform-compare.cpp)
add_executable(ismir_impl ismir_impl.cc)

target_link_libraries(overlap-analysis core ${Boost_LIBRARIES})
target_link_libraries(sound-match core ${Boost_LIBRARIES})
target_link_libraries(waveform-compare core ${Boost_LIBRARIES})
target_link_libraries(ismir_impl core ${Boost_LIBRARIES})

install(TARGETS overlap-analysis sound-match waveform-compare RUNTIME DESTINATION bin)
