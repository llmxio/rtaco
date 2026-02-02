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
  test_socket.cpp
  test_nl_common.cpp
)

target_link_libraries(test_rtaco PRIVATE llmx_rtaco GTest::gtest_main)

enable_testing()

include(GoogleTest)
gtest_discover_tests(test_rtaco)
