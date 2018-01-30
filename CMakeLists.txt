cmake_minimum_required(VERSION 2.6)

project(sensor-drivers)
set(CMAKE_BUILD_TYPE Debug)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")



add_executable(
    sensors_unittest
    ms4525do.c
    ./tests/test_ms4525do.cpp
    ./tests/mock/hal.cpp
    ./tests/runner.cpp
    )

target_include_directories(sensors_unittest PUBLIC tests/mock)

target_link_libraries(
    sensors_unittest
    m
    CppUTest
    CppUTestExt
    )

enable_testing()
add_test(sensors_unittest sensors_unittest;-c)

list(APPEND ALL_UNIT_TESTS ${CMAKE_CURRENT_BINARY_DIR}/sensors_unittest)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    FOREACH(t ${ALL_UNIT_TESTS})
        LIST(APPEND ALL_UNIT_TESTS_AS_COMMAND COMMAND;echo;${t})
        LIST(APPEND ALL_UNIT_TESTS_AS_COMMAND COMMAND;${t};-c)
    ENDFOREACH()
    add_custom_target(check ${ALL_UNIT_TESTS_AS_COMMAND} DEPENDS ${ALL_UNIT_TESTS})
else()
    set(ALL_UNIT_TESTS ${ALL_UNIT_TESTS} PARENT_SCOPE)
endif()

