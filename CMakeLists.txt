cmake_minimum_required (VERSION 3.8)
project(tetris)
Set (CMAKE_CXX_STANDARD 14)

find_package(SDL2 CONFIG REQUIRED)
include_directories(tetris ${SDL2_INCLUDE_DIRS})

add_executable (tetris "ui.cpp" "tetrisboard.cpp" "tetrisboard.h" "config.h" "ui.h")
target_link_libraries(tetris ${SDL2_LIBRARIES})