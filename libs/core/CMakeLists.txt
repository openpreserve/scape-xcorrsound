add_library (core
  AudioFile.cpp
  AudioFile.h
  AudioStream.h
  cross_correlation.h
  logstream.cpp
  logstream.h
  my_utils.cpp
  my_utils.h
  wavStdinReader.cc
  wavStdinReader.hh
)

target_link_libraries(core ${FFTW_LIBRARIES})
#set_target_properties(core PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../..)