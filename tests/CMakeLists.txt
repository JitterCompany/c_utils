cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

project(tests)

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../c_utils/src)

include(${CMAKE_SOURCE_DIR}/CPM_setup.cmake)

#------------------------------------------------------------------------------
# Build Settings
#------------------------------------------------------------------------------

# optimize level
set(OPT 0)

# compile flags
set(C_FLAGS_WARN "-Wall -Wextra -Wno-unused-parameter                   \
    -Wshadow -Wpointer-arith -Winit-self -Wstrict-overflow=5")

set(C_FLAGS "${C_FLAGS_WARN} -O${OPT} -g3 -c -fmessage-length=80        \
    -fno-builtin -ffunction-sections -fdata-sections -std=gnu99")

add_definitions("${C_FLAGS}")

# dir where the normal project sources can be found
set(TEST_NORMAL_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/")

# dir where the test-related sources can be found
set(TEST_TESTS_SOURCE_DIR    "${CMAKE_CURRENT_SOURCE_DIR}")

# set specific sources: for each test <testname>,
# the sources specified by test_<testname>_src are linked in.
# Note: these are relative to TEST_NORMAL_SOURCE_DIR.
set(test_align_src align.c)


# all 'shared' c files: these are linked against every test.
# files that also occur in TEST_MAIN_SOURCES are automatically removed
file(GLOB TEST_SHARED_SOURCES
    RELATIVE ${TEST_TESTS_SOURCE_DIR}
    "*.c"
)

# all test 'main' files: each of these should have its own main() function.
# they are compiled and run when calling 'make tests'
file(GLOB TEST_MAIN_SOURCES
    RELATIVE ${TEST_TESTS_SOURCE_DIR}
    "*.test.c"
)

include_directories("${TEST_NORMAL_SOURCE_DIR}")
include_directories("${TEST_NORMAL_SOURCE_DIR}/..")

message(STATUS "globbed:${TEST_SHARED_SOURCES}")

CPM_AddModule("cpm_unity"
    GIT_REPOSITORY "https://github.com/JitterCompany/cpm_unity.git"
    GIT_TAG "2.3.2-8")


CPM_Finish()

