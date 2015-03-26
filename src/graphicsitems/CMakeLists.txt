
file(GLOB HDRS include/*.h*)
file(GLOB SRCS src/*.cpp src/*.h*)
file(GLOB UIS src/*.ui)

qt5_wrap_cpp(MOC_FILE ${HDRS})

add_library(graphicsitems STATIC ${SRCS} ${MOC_FILE})

target_link_libraries(graphicsitems ${LIBASS_LIBRARIES} Qt5::Svg Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets)

target_include_directories(graphicsitems PUBLIC include)

target_include_directories(graphicsitems PRIVATE ${LIBASS_INCLUDE_DIRS})


