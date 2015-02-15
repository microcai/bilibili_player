cmake_minimum_required(VERSION 2.6)
project(bilibili)

add_executable(bilibili main.cpp)

install(TARGETS bilibili RUNTIME DESTINATION bin)
