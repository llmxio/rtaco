# version and build date headers autogen
string(TIMESTAMP PROJECT_BUILD_DATE "%a %d %b %Y %H:%M:%S UTC" UTC)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)
