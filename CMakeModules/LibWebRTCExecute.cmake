if (LIBWEBRTC_EXECUTE_INCLUDED)
  return()
endif (LIBWEBRTC_EXECUTE_INCLUDED)
set(LIBWEBRTC_EXECUTE_INCLUDED true)

include(CMakeParseArguments)
include(Environment)

function (libwebrtc_execute)
    set(ONE_VALUE_ARGS OUTPUT_VARIABLE WORKING_DIRECTORY STAMPFILE STATUS ERROR)
    set(MULTI_VALUE_ARGS COMMAND)
    cmake_parse_arguments(COMMAND "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    set(CMF_DIR ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY})

    if (COMMAND_STAMPFILE)
        set(STAMP_FILE "${CMF_DIR}/${COMMAND_STAMPFILE}")

        if (EXISTS ${STAMP_FILE})
            if (COMMAND_OUTPUT_VARIABLE)
                file(READ ${STAMP_FILE} _OUTPUT)

                if (_OUTPUT)
                    set(${COMMAND_OUTPUT_VARIABLE} ${_OUTPUT} PARENT_SCOPE)
                endif ()
            endif ()

            return()
        endif ()
    endif ()

    if (COMMAND_STATUS)
        message(STATUS ${COMMAND_STATUS})
    endif ()

    execute_process(COMMAND ${COMMAND_COMMAND}
                    WORKING_DIRECTORY ${COMMAND_WORKING_DIRECTORY}
                    OUTPUT_VARIABLE _OUTPUT
                    RESULT_VARIABLE _RESULT)

    if (NOT _RESULT EQUAL 0)
        message(FATAL_ERROR "-- " ${COMMAND_ERROR})
    endif ()

    if (COMMAND_STAMPFILE)
        file(WRITE ${STAMP_FILE} ${_OUTPUT})
    endif ()

    if (COMMAND_OUTPUT_VARIABLE)
        set(${COMMAND_OUTPUT_VARIABLE} ${_OUTPUT} PARENT_SCOPE)
    endif ()
endfunction ()
