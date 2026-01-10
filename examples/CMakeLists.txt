# Usage: rtaco_add_example(<target> <sources...>)
function(rtaco_add_example target)
  if (NOT ARGN)
    message(FATAL_ERROR "rtaco_add_example: missing sources for ${target}")
  endif()

  add_executable(${target} ${ARGN})

  target_compile_features(${target} PRIVATE cxx_std_23)
  target_link_libraries(${target} PRIVATE llmx::rtaco)

  install(TARGETS ${target}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endfunction()

rtaco_add_example(nl_control_example nl_control_example.cxx)
rtaco_add_example(nl_listener_example nl_listener_example.cxx)
