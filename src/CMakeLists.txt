
qt5_add_resources(RC_SRCS ../bilibili.qrc)

add_executable(bilibili_player ${RC_SRCS} main.cpp bplayer.hpp bplayer.cpp bilibilires.cpp acfunres.cpp ../bilibili.qrc)

set_target_properties(bilibili_player PROPERTIES AUTOMOC ON)

target_link_libraries(bilibili_player Qt5::Svg Qt5::DBus Qt5::Widgets Qt5::Network Qt5::Xml Qt5::Multimedia Qt5::MultimediaWidgets ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS bilibili_player RUNTIME DESTINATION bin)


