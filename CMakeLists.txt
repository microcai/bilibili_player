cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)

project(bilibili)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


add_definitions(-DMODULE_STRING="bilibili" -DMODULE_NAME=bilibili)

add_definitions(-D_FILE_OFFSET_BITS=64 -D_REENTRANT -D_THREAD_SAFE)

find_package(OpenSSL REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Threads REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Boost 1.55 COMPONENTS regex thread REQUIRED)

find_package(KF5WindowSystem)

include_directories(${OPENSSL_INCLUDE_DIR})

link_libraries(${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(src/)

