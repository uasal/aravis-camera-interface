set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
"${CMAKE_SOURCE_DIR}/modules")

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
	../src/downlink/packetinterface.hpp
)

add_library(srcdeps ${src_files})
target_link_libraries(${PROJECT_NAME} srcdeps)

# package dependencies

find_package( aravis REQUIRED)
include_directories(${aravis_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${aravis_LIBRARIES})

find_package( GLIB2 REQUIRED)
message ("GLIB2_INCLUDE_DIR  : " ${GLIB2_INCLUDE_DIR})
message ("GLIB2_LIBRARIES : " ${GLIB2_LIBRARIES})
include_directories(${GLIB2_INCLUDE_DIR})
link_directories(${GLIB2_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLIB2_LIBRARIES})


find_package( GObject REQUIRED)
message ("GOBJECT_INCLUDE_DIR : " ${GOBJECT_INCLUDE_DIR})
message ("GOBJECT_LIBRARIES : " ${GOBJECT_LIBRARIES})
include_directories(${GOBJECT_INCLUDE_DIR})
#link_directories(${GOBJECT_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${GOBJECT_LIBRARIES})


# libpng may be removed later
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

find_package(Catch2 REQUIRED)
target_link_libraries(${PROJECT_NAME} Catch2::Catch2)

include(Catch)
include(CTest)
catch_discover_tests(${PROJECT_NAME})
