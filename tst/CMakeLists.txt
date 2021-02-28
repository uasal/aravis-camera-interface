set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
"${CMAKE_SOURCE_DIR}/modules")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

project("testmain")

set(
	tst_files
	testmain.cpp
	membuffersimulatortest.cpp
	test_camera_feature_io.cpp
	test_packet_downlink.cpp
)

add_executable(${PROJECT_NAME} ${tst_files})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

target_include_directories (
	${PROJECT_NAME}
	PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

message("current source dir : " ${CMAKE_CURRENT_SOURCE_DIR})
set(
	src_files
	../src/hdcamera.cpp
	../src/hdcamera.h
	../src/config.h
	../src/simulation/membuffersimulator.cpp
	../src/simulation/membuffersimulator.h
	../src/downlink/packetinterface.h
	../src/downlink/packetinterface.cpp
	../src/downlink/packetmanager.cpp
	../src/downlink/packetmanager.h
)

add_library(srcdeps ${src_files})
target_link_libraries(${PROJECT_NAME} srcdeps)

# package dependencies
link_directories(${CMAKE_SOURCE_DIR}/include/lib)

find_package( aravis REQUIRED)
include_directories(${aravis_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} aravis-0.6)

find_package( GLIB2 REQUIRED)
include_directories(${GLIB2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} glib-2.0)


find_package( GObject REQUIRED)
include_directories(${GOBJECT_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} gobject-2.0)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# libpng may be removed later
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} png)

find_package(Catch2 REQUIRED)
target_link_libraries(${PROJECT_NAME} Catch2::Catch2)

include(Catch)
include(CTest)
catch_discover_tests(${PROJECT_NAME})
