cmake_minimum_required (VERSION 3.19)
project(tetris)
Set (CMAKE_CXX_STANDARD 14)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

find_package(SDL2 REQUIRED)
find_package(Python3 3.9 EXACT REQUIRED COMPONENTS Interpreter Development NumPy)
include_directories(tetris ${SDL2_INCLUDE_DIRS})

add_executable (play "ui.cpp" "tetrisboard.cpp" "tetrisboard.h" "config.h"  "inireader.h")
python3_add_library(env MODULE "tetrisboard.cpp"  "render.cpp")
target_link_libraries(play ${SDL2_LIBRARIES})


target_include_directories(env PRIVATE ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})
target_link_libraries(env PRIVATE ${SDL2_LIBRARIES} ${Python3_LIBRARIES})

install(TARGETS env OPTIONAL
    DESTINATION tetris)