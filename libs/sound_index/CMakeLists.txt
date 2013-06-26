add_library (sound_index
  computeFFT.h
  hamming.h
  #ismir_impl.cc
  tmp.cc
)

target_link_libraries(sound_index ${FFTW_LIBRARIES} core)
#set_target_properties(sound_index PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../..)
