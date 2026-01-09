# build-docs.cmake

find_package(Doxygen REQUIRED)
if(DOXYGEN_FOUND)
  # Path to the per-component Doxygen config we ship with the repo
  set(DOXYFILE_DOCA "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.doca")

  # Output directory inside the source tree (docs/html)
  set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs/html")

  # Ensure the output directory exists in source tree so the generated
  # files are visible to e.g. GitHub Pages or local browsing.
  file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

  # Target that generates DOCA API docs into docs/html
  add_custom_target(docs-doca
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_DOCA}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating DOCA API documentation with Doxygen -> ${DOXYGEN_OUTPUT_DIR}"
    VERBATIM
  )

  # Backwards-compatible umbrella target: generate the DOCA docs
  add_custom_target(docs DEPENDS docs-doca)

  message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")
  message(STATUS "DOCA documentation will be generated in: ${DOXYGEN_OUTPUT_DIR}")
  # Install target: copy generated HTML into the install prefix
  # Use standard CMake variable for documentation dir if available
  if(NOT DEFINED CMAKE_INSTALL_DOCDIR)
    set(CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}")
  endif()
  message(STATUS "Documentation install dir: ${CMAKE_INSTALL_DOCDIR}/html")

  install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}/
    DESTINATION "${CMAKE_INSTALL_DOCDIR}/html"
    COMPONENT doc
    OPTIONAL
  )
else()
  message(WARNING "Doxygen not found - documentation will not be generated")
endif()
