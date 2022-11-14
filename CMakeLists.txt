cmake_minimum_required (VERSION 3.8)
project(tetris)
Set (CMAKE_CXX_STANDARD 14)

find_package(SDL2 REQUIRED)
find_package(Python3 3.8 EXACT REQUIRED COMPONENTS Interpreter NumPy)
include_directories(tetris ${SDL2_INCLUDE_DIRS})
include_directories(tetris ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})

add_executable (tetris "ui.cpp" "tetrisboard.cpp" "tetrisboard.h" "config.h"  "inireader.h")
Python3_add_library(tetrislib "tetrisboard.cpp"  "ui.cpp")
set_target_properties(tetrislib PROPERTIES PUBLIC_HEADER tetrisboard.h)

target_link_libraries(tetris ${SDL2_LIBRARIES})
target_link_libraries(tetrislib PUBLIC ${SDL2_LIBRARIES} ${Python3_LIBRARIES})

install(TARGETS tetrislib
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/tetris
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})