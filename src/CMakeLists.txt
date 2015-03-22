
qt5_add_resources(RC_SRCS ../bilibili.qrc)

add_executable(bilibili_player
	${RC_SRCS}
	main.cpp
	bplayer.hpp
	bplayer.cpp
	bilibilires.cpp
	acfunres.cpp
	danmumanager.cpp
	danmumanager.hpp
	screensaverinhibitor.cpp
	videoitem.hpp
	videoitem.cpp
	)

set_target_properties(bilibili_player PROPERTIES AUTOMOC ON)

target_link_libraries(bilibili_player ${BULLET_LIBRARIES} Qt5::Svg Qt5::DBus Qt5::Widgets Qt5::Network Qt5::Xml Qt5::Multimedia Qt5::MultimediaWidgets ${CMAKE_THREAD_LIBS_INIT})

if(KF5WindowSystem_FOUND)
target_link_libraries(bilibili_player KF5::WindowSystem)
target_compile_definitions(bilibili_player PRIVATE -DHAVE_KF5_WINDOWSYSTEM)
endif()

install(TARGETS bilibili_player RUNTIME DESTINATION bin)


