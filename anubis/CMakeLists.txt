cmake_minimum_required(VERSION 3.10)
project(MyCTestProject C CXX)

set(SOURCE_DIR "src/")
set(TEST_DIR "gtests/")

file(GLOB MY_C_SOURCES "${SOURCE_DIR}*.c")
file(GLOB MY_TEST_SOURCES "${TEST_DIR}*.cpp")
file(GLOB MAIN_SOURCE "${SOURCE_DIR}anubis.c")
# list(REMOVE_ITEM MY_C_SOURCES
#      "/home/dai/git_repos/comp3300-2023-assignment1/anubis/src/anubis.c"
#      # Add other files to exclude here if needed
# )
foreach(EXCLUDE_FILE ${MAIN_SOURCE})
    list(REMOVE_ITEM MY_C_SOURCES ${EXCLUDE_FILE})
endforeach()

message(STATUS "MY_C_SOURCES after exclusion: ${MY_C_SOURCES}")


find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(runTests ${MY_TEST_SOURCES} ${MY_C_SOURCES})
target_link_libraries(runTests ${GTEST_BOTH_LIBRARIES} pthread)
