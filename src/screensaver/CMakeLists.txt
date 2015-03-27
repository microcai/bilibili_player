
add_library(bilibili_screensaver STATIC screensaverinhibitor.cpp screensaverinhibitor.hpp)

target_link_libraries(bilibili_screensaver Qt5::DBus)
