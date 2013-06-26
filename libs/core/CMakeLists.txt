add_library (core
#FFT.cpp
#FFT.h
AudioFile.cpp
AudioFile.h
AudioStream.h
cross_correlation.h
logstream.cpp
logstream.h
my_utils.cpp
my_utils.h
)

target_link_libraries(core ${FFTW_LIBRARIES})
#set_target_properties(core PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../..)