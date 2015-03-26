
find_package(X11)

if (NOT WIN32)

	pkg_check_modules(XRANDR REQUIRED xrandr)

	add_library(bilibili_xrandr STATIC xrandr.cpp xrandr.hpp)

	target_link_libraries(bilibili_xrandr Qt5::X11Extras ${XRANDR_LIBRARIES} ${X11_LIBRARIES})

endif()
