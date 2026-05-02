cmake_minimum_required(VERSION 3.22)

function(add_llmx_test target)
    cmake_parse_arguments(
        LLMX_TEST
        ""
        "TEST_NAME"
        ""
        ${ARGN}
    )

    if(LLMX_TEST_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_llmx_test does not accept source file arguments")
    endif()

    add_executable(${target} ${target}.cxx)
    target_link_libraries(${target}
        PRIVATE
            GTest::gtest
            ${PROJECT_NAME}
    )

    if(LLMX_TEST_TEST_NAME)
        set(test_name ${LLMX_TEST_TEST_NAME})
    else()
        set(test_name ${target})
    endif()

    add_test(NAME ${test_name} COMMAND ${target})
endfunction()


include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)

add_executable(test_rtaco
  test_signal.cpp
  test_requesttask_compile.cpp
  test_requesttask.cpp
  test_socket.cpp
  test_control.cpp
  test_nl_common.cpp
  test_neighbor_tasks.cpp
)

set(rtaco_test_sources)
foreach(source IN LISTS RTACO_SOURCES)
    list(APPEND rtaco_test_sources "${PROJECT_SOURCE_DIR}/${source}")
endforeach()

add_library(llmx_rtaco_test_instrumented STATIC ${rtaco_test_sources})
target_compile_features(llmx_rtaco_test_instrumented PUBLIC cxx_std_23)
target_link_libraries(llmx_rtaco_test_instrumented PUBLIC Threads::Threads Boost::system)
target_include_directories(llmx_rtaco_test_instrumented
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)
target_compile_definitions(llmx_rtaco_test_instrumented PRIVATE RTACO_ENABLE_TEST_HOOKS)

get_target_property(rtaco_compile_defs llmx_rtaco COMPILE_DEFINITIONS)
if(rtaco_compile_defs)
    list(FIND rtaco_compile_defs RTACO_ENABLE_TEST_HOOKS rtaco_hooks_idx)
    if(NOT rtaco_hooks_idx EQUAL -1)
        message(FATAL_ERROR "RTACO_ENABLE_TEST_HOOKS leaked into llmx_rtaco")
    endif()
endif()

target_link_libraries(test_rtaco PRIVATE llmx_rtaco_test_instrumented GTest::gtest_main)
target_include_directories(test_rtaco PRIVATE ${PROJECT_SOURCE_DIR})

enable_testing()

include(GoogleTest)
gtest_discover_tests(test_rtaco)
