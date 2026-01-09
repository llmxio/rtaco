#----------------------------------------------------------------
# Generated CMake target import file for configuration "RELEASE".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "llmx::llmx_rtaco" for configuration "RELEASE"
set_property(TARGET llmx::llmx_rtaco APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(llmx::llmx_rtaco PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libllmx_rtaco.a"
  )

list(APPEND _cmake_import_check_targets llmx::llmx_rtaco )
list(APPEND _cmake_import_check_files_for_llmx::llmx_rtaco "${_IMPORT_PREFIX}/lib64/libllmx_rtaco.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
