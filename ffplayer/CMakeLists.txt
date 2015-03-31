
add_library(ffplayer STATIC src/ffplayer.cpp src/qio2avio.cpp)

target_link_libraries(ffplayer Qt5::Core)


target_include_directories(ffplayer PUBLIC include/)
