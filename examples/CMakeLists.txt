# Examples CMake

add_executable(nl_control_example
  src/nl_control_example.cxx
)

add_executable(nl_dump_addresses_example
  src/nl_dump_addresses_example.cxx
)

add_executable(nl_dump_routes_example
  src/nl_dump_routes_example.cxx
)

target_compile_features(nl_control_example PRIVATE cxx_std_23)
target_compile_features(nl_dump_addresses_example PRIVATE cxx_std_23)
target_compile_features(nl_dump_routes_example PRIVATE cxx_std_23)

target_link_libraries(nl_control_example PRIVATE llmx::rtaco)
target_link_libraries(nl_dump_addresses_example PRIVATE llmx::rtaco)
target_link_libraries(nl_dump_routes_example PRIVATE llmx::rtaco)

install(TARGETS nl_control_example nl_dump_addresses_example nl_dump_routes_example
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
