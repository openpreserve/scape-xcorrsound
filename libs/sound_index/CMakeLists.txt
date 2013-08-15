add_library (sound_index
  #fingerprint_stream.cc
  #fingerprint_stream.hh
  #ismir_impl.cc
  computeFFT.h
  fingerprint_db.cc
  fingerprint_db.hh
  fingerprint_strategy.hh
#  fingerprint_strategy_chroma.cc
  fingerprint_strategy_ismir.cc
  hamming.h
)

#target_link_libraries(sound_index ${FFTW_LIBRARIES} core ${LIBAVFORMAT_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVUTIL_LIBRARIES} chromaprint)
target_link_libraries(sound_index ${FFTW_LIBRARIES} core)
#set_target_properties(sound_index PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../..)
