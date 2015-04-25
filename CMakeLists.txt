cmake_minimum_required(VERSION 2.8.12)

project(bilibili)

include(CheckCXXCompilerFlag)

if (NOT MSVC)
CHECK_CXX_COMPILER_FLAG(-std=c++11 LIBCXX_HAS_STDCXX11_FLAG)
if(LIBCXX_HAS_STDCXX11_FLAG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
	message(FATAL "need at least gcc 4.8 or clang 3.4")
endif()
endif(NOT MSVC)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_definitions(-DMODULE_STRING="bilibili" -DMODULE_NAME=bilibili)

add_definitions(-D_FILE_OFFSET_BITS=64 -D_REENTRANT -D_THREAD_SAFE)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

find_package(Qt5 5.4 COMPONENTS Core Xml Svg Widgets Network Multimedia MultimediaWidgets OPTIONAL_COMPONENTS DBus X11Extras)

if (Qt5_FOUND)
	message(STATUS "qt5 found!")
else()
	message(FATAL_ERROR "required qt5 components not found")
endif()

find_package(OpenSSL REQUIRED)
find_package(Boost 1.55 COMPONENTS regex thread system timer REQUIRED)

find_package(Bullet REQUIRED)

set(BOX2D_INSTALL OFF)
set(BOX2D_INSTALL_DOC OFF)
set(BOX2D_BUILD_SHARED  OFF)
set(BOX2D_BUILD_STATIC ON)
set(BOX2D_BUILD_EXAMPLES OFF)

# add_subdirectory(third_party/Box2D/Box2D EXCLUDE_FROM_ALL)

pkg_check_modules(LIBASS REQUIRED libass)

#pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavfilter libavutil libavresample libavdevice)


find_package(PulseAudio)

find_package(KF5WindowSystem)

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${BULLET_INCLUDE_DIRS})

add_definitions     ( ${BULLET_DEFINITIONS} )
link_directories    ( ${BULLET_LIBRARY_DIRS} )

link_libraries(${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${LIBASS_LDFLAGS} ${LIBASS_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(gstplayer/)

add_subdirectory(src/)

