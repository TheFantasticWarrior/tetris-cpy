cmake_minimum_required (VERSION 3.8)
project(tetris)
Set (CMAKE_CXX_STANDARD 14)

find_package(SDL2 REQUIRED)
include_directories(tetris ${SDL2_INCLUDE_DIRS})

add_executable (tetris "ui.cpp" "tetrisboard.cpp" "tetrisboard.h" "config.h"  "inireader.h")
add_library (tetrislib "ui.cpp" "tetrisboard.cpp" "tetrisboard.h")
set_target_properties(tetrislib PROPERTIES PUBLIC_HEADER include/tetrislib.h)
target_link_libraries(tetris ${SDL2_LIBRARIES})