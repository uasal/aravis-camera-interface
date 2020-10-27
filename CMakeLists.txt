cmake_minimum_required(VERSION 3.10.2)
project(cam)

add_subdirectory(src)

# Uncomment for tests

include(CTest)
enable_testing()

if(BUILD_TESTING)
    add_subdirectory (tst)
endif()