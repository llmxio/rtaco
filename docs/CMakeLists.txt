find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
  message(FATAL_ERROR "Doxygen is required to build docs. Install Doxygen or set RTACO_BUILD_DOCS=OFF.")
endif()

set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

add_custom_target(docs
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating API documentation with Doxygen"
  VERBATIM
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        OPTIONAL
)
