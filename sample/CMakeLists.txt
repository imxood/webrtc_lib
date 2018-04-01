cmake_minimum_required(VERSION 3.3)
project(sample)

set(CMAKE_BUILD_TYPE Debug)

add_definitions(-DWEBRTC_BUILD_LIBEVENT)

find_package(LibWebRTC REQUIRED)
include(${LIBWEBRTC_USE_FILE})

set(SOURCE_FILES main.cpp)
add_executable(sample ${SOURCE_FILES})
target_link_libraries(sample ${LIBWEBRTC_LIBRARIES})

set(_STAMP_FILE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/check-sample-done)
add_custom_command(
    OUTPUT ${_STAMP_FILE}
    COMMENT "Run generated sample"
    COMMAND sample
    COMMAND ${CMAKE_COMMAND} -E touch ${_STAMP_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_custom_target(check-sample ALL DEPENDS ${_STAMP_FILE})
add_dependencies(check-sample sample)

set(TEST_FILE "log.txt")
add_custom_command(OUTPUT  ${TEST_FILE}
    COMMAND echo "Generating log.txt file..."
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_FILE} ${TEST_FILE}
    COMMENT  "This is a test"
)
add_custom_target(Test1 ALL DEPENDS ${TEST_FILE})

add_custom_command(TARGET Test1
    PRE_BUILD
    COMMAND echo "executing a fake command"
    COMMENT "This command will be executed before building target Test1"
)

add_executable(audio_sample audio_sample.cpp)
target_link_libraries(audio_sample ${LIBWEBRTC_LIBRARIES})

# add_executable(webrtc_voiceengine_demo webrtc_voiceengine_demo.cpp)
# target_link_libraries(webrtc_voiceengine_demo ${LIBWEBRTC_LIBRARIES})