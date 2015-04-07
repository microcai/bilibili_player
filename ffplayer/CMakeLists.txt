
qt5_wrap_cpp(MOC_FILE include/ffplayer.hpp)

file(GLOB SRCS src/*.cpp src/*.h*)

add_library(ffplayer STATIC ${MOC_FILE} ${SRCS})

target_link_libraries(ffplayer Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets ${FFMPEG_LIBRARIES} ${LIBVA_LIBRARIES} Qt5::X11Extras)

target_include_directories(ffplayer PUBLIC include/)

set_target_properties(ffplayer PROPERTIES AUTOMOC ON)

target_link_libraries(ffplayer ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY})
