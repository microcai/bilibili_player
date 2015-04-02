
qt5_wrap_cpp(MOC_FILE include/ffplayer.hpp)

add_library(ffplayer STATIC ${MOC_FILE} src/ffplayer.cpp src/qio2avio.cpp)

target_link_libraries(ffplayer Qt5::Core Qt5::Widgets Qt5::Multimedia ${FFMPEG_LIBRARIES})


target_include_directories(ffplayer PUBLIC include/)
