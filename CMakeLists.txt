cmake_minimum_required(VERSION 3.22)

project(llmx-rtaco VERSION 0.0.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include(cmake/build-options.cmake)
include(cmake/colored-output.cmake)

find_package(Threads REQUIRED)
find_package(Boost 1.83.0 REQUIRED CONFIG COMPONENTS system REQUIRED)

set(RTACO_SOURCES
  src/nl_control.cxx
  src/nl_address_dump_task.cxx
  src/nl_link_event.cxx
  src/nl_route_event.cxx
  src/nl_address_event.cxx
  src/nl_neighbor_event.cxx
  src/nl_listener.cxx
  src/nl_neighbor_dump_task.cxx
  src/nl_neighbor_flush_task.cxx
  src/nl_neighbor_get_task.cxx
  src/nl_neighbor_probe_task.cxx
  src/nl_route_dump_task.cxx
  src/nl_socket_guard.cxx
  src/nl_socket.cxx
)

add_library(llmx_rtaco ${RTACO_SOURCES})

target_include_directories(llmx_rtaco PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(llmx_rtaco PUBLIC cxx_std_23)

target_link_libraries(llmx_rtaco PUBLIC
  Threads::Threads
  Boost::system
)

set_target_properties(llmx_rtaco PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
  POSITION_INDEPENDENT_CODE ON
)

# Provide a namespaced alias usable by consumers as llmx::rtaco
add_library(llmx::rtaco ALIAS llmx_rtaco)

# Installation
install(TARGETS llmx_rtaco
  EXPORT llmx-rtacoTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT llmx-rtacoTargets
  FILE llmx-rtacoTargets.cmake
  NAMESPACE llmx::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/llmx-rtaco
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/llmx-rtaco-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/llmx-rtaco-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/llmx-rtaco-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/llmx-rtaco
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/llmx-rtaco-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/llmx-rtaco-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/llmx-rtaco
)

# Export for local usage (useful for `cmake --install` plus `export(PACKAGE ...)`)
export(EXPORT llmx-rtacoTargets NAMESPACE llmx:: FILE ${CMAKE_CURRENT_BINARY_DIR}/llmx-rtacoTargets.cmake)
export(PACKAGE llmx-rtaco)

option(RTACO_BUILD_EXAMPLES "Build examples" OFF)

if (RTACO_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

message(STATUS "Config: llmx-rtaco project configured")
