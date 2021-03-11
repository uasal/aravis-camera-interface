set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")
project("hdcam")

add_executable(${PROJECT_NAME} main.cpp)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

target_include_directories (
	${PROJECT_NAME}
	PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(
	src_files
	hdcamera.cpp
	hdcamera.h
	config.h
	simulation/membuffersimulator.cpp
	simulation/membuffersimulator.h
	downlink/packetinterface.h
	downlink/packetinterface.cpp
	downlink/packetmanager.cpp
	downlink/packetmanager.h
)

add_library(srcfs ${src_files})
target_link_libraries(${PROJECT_NAME} srcfs)

# package dependencies

find_package( aravis REQUIRED)
include_directories(${aravis_INCLUDE_DIR})
string(REGEX REPLACE "[.]so$" ".a" aravis_lib ${aravis_LIBRARIES}) 
target_link_libraries(${PROJECT_NAME} ${aravis_lib} -static)

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

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# libpng may be removed later
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})


