
pkg_check_modules(LIBVA libva-glx libva)

pkg_check_modules(GST gstreamer-1.0 gstreamer-video-1.0 gstreamer-app-1.0 gstreamer-gl-1.0)

qt5_wrap_cpp(MOC_FILE include/gstplayer.hpp)

file(GLOB SRCS src/*.cpp src/*.h*)

add_library(gstplayer STATIC ${MOC_FILE} ${SRCS})

target_link_libraries(gstplayer Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets Qt5::X11Extras)

target_include_directories(gstplayer PUBLIC include/)

target_link_libraries(gstplayer ${GST_LIBRARIES})

set_target_properties(gstplayer PROPERTIES AUTOMOC ON)

target_include_directories(gstplayer PRIVATE ${GST_INCLUDE_DIRS})
