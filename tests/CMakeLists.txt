cmake_minimum_required(VERSION 3.14)
project(tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory(googletest)

set(QUERY_BUILDER_LIB_DIRECTORY ../)
find_package(query-builder)

if(TARGET query-builder::query-builder)
    message(STATUS "Using query-builder as INTERFACE library")
else()
    add_library(query-builder INTERFACE)
    target_include_directories(query-builder INTERFACE ${QUERY_BUILDER_LIB_DIRECTORY}/include)
endif()

set(TESTS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TESTS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

file(GLOB TEST_SOURCES "${TESTS_SRC_DIR}/*.cpp")

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    target_include_directories(${TEST_NAME} PRIVATE
        ${TESTS_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include
    )
    
    target_link_libraries(${TEST_NAME}
        gtest
        gtest_main
        gmock
        query-builder::query-builder 
    )
    
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} WORKING_DIRECTORY ${BUILD_DIR})
endforeach()