set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")
set (CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set (BUILD_SHARED_LIBS OFF)
set (CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

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

############### package dependencies
#link_directories(${CMAKE_SOURCE_DIR}/include/lib)
# for static import of glib
#target_link_libraries(${PROJECT_NAME} c)
target_link_libraries(${PROJECT_NAME} ffi)

find_package( GObject REQUIRED)
find_package( GLIB2 REQUIRED)
find_package( GIO REQUIRED)
find_package( aravis REQUIRED)
find_package( PNG REQUIRED)


include_directories(${GOBJECT_INCLUDE_DIR})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${aravis_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})
include_directories(${GIO_INCLUDE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

add_library(srcfs STATIC ${src_files})

target_link_libraries(srcfs 
	gobject-2.0 glib-2.0 gio-2.0 gthread-2.0 glib-2.0 gobject-2.0 aravis-0.6 png Threads::Threads ffi pcre gmodule-2.0 gobject-2.0 glib-2.0 gio-2.0 gthread-2.0 glib-2.0 gobject-2.0 mount blkid uuid gmodule-2.0 gobject-2.0 glib-2.0 gio-2.0 gthread-2.0 glib-2.0 gobject-2.0
	icui18n icuuc z m lzma swresample xml2 z m lzma icuuc icui18n dl 
)
target_link_libraries(${PROJECT_NAME} 
	gobject-2.0 glib-2.0 gio-2.0 gthread-2.0 glib-2.0 gobject-2.0 aravis-0.6 png Threads::Threads ffi pcre gmodule-2.0 gobject-2.0 glib-2.0 gio-2.0 gthread-2.0 glib-2.0 gobject-2.0 mount blkid uuid
	icui18n icuuc z m lzma swresample xml2 z m lzma icuuc icui18n dl 
)

message("DL : " ${CMAKE_DL_LIBS})
target_link_libraries(${PROJECT_NAME} srcfs)