set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -rdynamic")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")
set (CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set (BUILD_SHARED_LIBS OFF)
set (CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -Wl,--unresolved-symbols=ignore-all") # ignore references for selinux

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

add_library(srcfs STATIC ${src_files})
target_link_libraries(${PROJECT_NAME} srcfs)

# package dependencies
link_directories(${CMAKE_SOURCE_DIR}/include/lib)

find_package( aravis REQUIRED)
include_directories(${aravis_INCLUDE_DIR})

string(REGEX REPLACE "[.]so$" ".a" aravis_lib ${aravis_LIBRARIES}) 
target_link_libraries(${PROJECT_NAME} ${aravis_lib} -static)
message("ARAVIS-LIB : " ${aravis_lib})

find_package( GLIB2 REQUIRED)
include_directories(${GLIB2_INCLUDE_DIR})
link_directories(${GLIB2_LIBRARY_DIRS})
string(REGEX REPLACE "[.]so$" ".a" glib_libs ${GLIB2_LIBRARIES}) 
target_link_libraries(${PROJECT_NAME} ${glib_libs})

message ("GLIB2_INCLUDE_DIR  : " ${GLIB2_INCLUDE_DIR})
message ("GLIB2_LIBRARIES : " ${glib_libs})

#find_package(udev REQUIRED)
#include_directories(${UDEV_INCLUDE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME} gobject-2.0 ${glib_libs} gio-2.0 gthread-2.0 gmodule-2.0 gio-2.0 gobject-2.0 ${glib_libs} gio-2.0 gthread-2.0 gmodule-2.0 gio-2.0 ffi pcre mount blkid uuid
	icui18n icuuc z m lzma swresample xml2 z m lzma icuuc icui18n icudata dl resolv -static)
