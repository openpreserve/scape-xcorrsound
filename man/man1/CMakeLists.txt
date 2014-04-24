set(ASCIIDOC_MANS_SRC waveform-compare.1 overlap-analysis.1 sound-match.1 xcorrsound.1)

foreach(manpage ${ASCIIDOC_MANS_SRC})
      add_custom_command(OUTPUT ${manpage}
                         COMMAND ${RONN_TOOL}
                         ARGS -roff --pipe ${CMAKE_CURRENT_SOURCE_DIR}/${manpage}.ronn > ${CMAKE_CURRENT_BINARY_DIR}/${manpage}
                         DEPENDS ${manpage}.ronn
                         COMMENT "Building ${manpage}")

      add_custom_command(OUTPUT ${manpage}.gz
                         COMMAND ${GZIP_TOOL} --best -c ${CMAKE_CURRENT_BINARY_DIR}/${manpage} > ${CMAKE_CURRENT_BINARY_DIR}/${manpage}.gz
                         DEPENDS ${manpage}
                         COMMENT "Building ${manpage}.gz")
      add_custom_target(manpage_${manpage} ALL DEPENDS ${manpage}.gz )
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${manpage}.gz DESTINATION share/man/man1)
endforeach()





