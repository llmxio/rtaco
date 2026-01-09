option(LLMX_FORCE_UNORDERED_LOOKUP "Force unordered containers heterogenous lookup for GCC version prio to 12." ON)

function(enable_cxx23 TARGET_ARG)
if(NOT TARGET_ARG)
  message(FATAL_ERROR "The target name is not defined")
else()
  message(STATUS "Enabled CXX 23 standard support for ${TARGET_ARG}")
  target_compile_features(${TARGET_ARG} PUBLIC cxx_std_23)
endif()
endfunction()

# Enable interprocedural optimization (LTO/IPO) for a target if supported by the
# current toolchain. Usage: llmx_enable_ipo(<target>)
function(llmx_enable_ipo target)
    if(NOT target)
        message(FATAL_ERROR "llmx_enable_ipo: target name is required")
    endif()
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _ipo_supported)
    if(_ipo_supported)
        set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
        message(STATUS "Enabled interprocedural optimization for ${target}")
    else()
        message(STATUS "Interprocedural optimization not supported for ${target}")
    endif()
endfunction()

function(enable_unordered_lookup TARGET_ARG)
if(NOT TARGET_ARG)
  message(FATAL_ERROR "The target name is not defined")
endif()
if(LLMX_FORCE_UNORDERED_LOOKUP)
  target_compile_definitions(${TARGET_ARG} PUBLIC LLMX_FORCE_UNORDERED_LOOKUP)
  message(STATUS "Enabled unordered containers heterogenous lookup for ${TARGET_ARG}")
else()
  message(STATUS "Unordered containers heterogenous lookup is not enabled for ${TARGET_ARG}")
endif()
endfunction()

# Helper to set POSITION_INDEPENDENT_CODE on a target.
# Usage: llmx_set_pic(<target> [<value>])
# If <value> is not provided, it defaults to ${BUILD_SHARED_LIBS} so
# behavior matches the previous inline call in CMakeLists.txt.
function(llmx_set_pic target value)
    if(NOT DEFINED value OR "${value}" STREQUAL "")
        set(_pic_val ${BUILD_SHARED_LIBS})
    else()
        set(_pic_val ${value})
    endif()
    set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ${_pic_val})
endfunction()

function(find_static_libs OUT_VAR LIBS_PATH)
    # Usage:
    #   find_static_libs(<OUT_VAR> [libs_path] <lib1> <lib2> ...)
    # The second parameter is optional. If it looks like a path (contains '/'),
    # it will be used as HINTS for find_library. For backward compatibility,
    # if the second parameter does not look like a path, it will be treated as
    # the first library name.

    set(_tmp_found_libs)
    set(_libs ${ARGN})

    if(LIBS_PATH)
        string(FIND "${LIBS_PATH}" "/" _slash_pos)
        if(_slash_pos EQUAL -1)
            list(INSERT _libs 0 ${LIBS_PATH})
            set(LIBS_PATH "")
        endif()
    endif()

    foreach(LIB ${_libs})
        if(NOT DEFINED FOUND_LIB_${LIB} OR "${FOUND_LIB_${LIB}}" STREQUAL "")
            if(LIBS_PATH)
                find_library(FOUND_LIB_${LIB}
                    NAMES lib${LIB}.a
                    HINTS ${LIBS_PATH}
                    REQUIRED
                )
            else()
                find_library(FOUND_LIB_${LIB}
                    NAMES lib${LIB}.a
                    REQUIRED
                )
            endif()
            message(STATUS "Found ${LIB}: ${FOUND_LIB_${LIB}}")
            set(FOUND_LIB_${LIB} ${FOUND_LIB_${LIB}} PARENT_SCOPE)
        endif()

        list(APPEND _tmp_found_libs ${FOUND_LIB_${LIB}})
    endforeach()

    set(_existing)
    if(DEFINED ${OUT_VAR})
        set(_existing "${${OUT_VAR}}")
    endif()
    if(_existing)
        list(APPEND _existing ${_tmp_found_libs})
        set(${OUT_VAR} ${_existing} PARENT_SCOPE)
    else()
        set(${OUT_VAR} ${_tmp_found_libs} PARENT_SCOPE)
    endif()
endfunction()

# Reorder DOCA_LIBS so that libdoca_flow archive appears first. This helps
# static linking where left-to-right archive order matters.
function(llmx_reorder_doca_libs)
    if(NOT DEFINED DOCA_LIBS)
        message(FATAL_ERROR "llmx_reorder_doca_libs: DOCA_LIBS is not defined, skipping")
        return()
    endif()

    set(_flow_lib)
    foreach(_l ${DOCA_LIBS})
        if(_l MATCHES "libdoca_flow")
            set(_flow_lib ${_l})
            break()
        endif()
    endforeach()
    if(_flow_lib)
        list(REMOVE_ITEM DOCA_LIBS ${_flow_lib})
        list(INSERT DOCA_LIBS 0 ${_flow_lib})
    endif()
endfunction()

function(llmx_disable_rtti target)
    target_compile_options(${target} PUBLIC -fno-rtti)
    target_compile_definitions(${target} PUBLIC ASIO_NO_TYPEID)
endfunction()
