
add_executable(bilibili_player main.cpp bilibiliplayer.hpp bilibiliplayer.cpp bilibilires.cpp)

set_target_properties(bilibili_player PROPERTIES AUTOMOC ON)

target_link_libraries(bilibili_player Qt5::Widgets Qt5::Network Qt5::Xml Qt5::Multimedia Qt5::MultimediaWidgets ${CMAKE_THREAD_LIBS_INIT})

