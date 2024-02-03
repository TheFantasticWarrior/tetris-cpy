cmake_minimum_required (VERSION 3.19)
project(tetris)
Set (CMAKE_CXX_STANDARD 14)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
find_package(SDL2 REQUIRED)
find_package(Python3 3.10 EXACT REQUIRED COMPONENTS Interpreter Development NumPy)
include_directories(tetris ${SDL2_INCLUDE_DIRS})

add_executable (play "ui.cpp" "tetrisboard.cpp" "tetrisboard.h" "config.h"  "inireader.h")
python3_add_library(env MODULE "tetrisboard.cpp"  "render.cpp")
python3_add_library(env2p MODULE "tetrisboard.cpp"  "2p.cpp")
python3_add_library(sim MODULE "tetrisboard.cpp"  "sim.cpp")
target_link_libraries(play ${SDL2_LIBRARIES})

target_include_directories(sim PRIVATE ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})
target_link_libraries(sim PRIVATE ${SDL2_LIBRARIES} ${Python3_LIBRARIES})
target_compile_options(sim PRIVATE -g)
install(TARGETS sim CONFIGURATIONS DEBUG LIBRARY DESTINATION tetris)

target_include_directories(env PRIVATE ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})
target_link_libraries(env PRIVATE ${SDL2_LIBRARIES} ${Python3_LIBRARIES})

install(TARGETS env CONFIGURATIONS Release LIBRARY DESTINATION tetris)


target_include_directories(env2p PRIVATE ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})
target_link_libraries(env2p PRIVATE ${SDL2_LIBRARIES} ${Python3_LIBRARIES})

install(TARGETS env2p CONFIGURATIONS Release LIBRARY DESTINATION tetris)

