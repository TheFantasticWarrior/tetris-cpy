# Tetris with instant soft drop delay
Following tetris guidelines, instant softdrop after specified time\
You can edit the [settings](config.h)
# Install

## Windows:
install sdl: `vcpkg install sdl:x64-windows`\
install [cmake](https://cmake.org/download/)
```
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/user/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```
note: mingw seems to fail, use visual studio if error:
`Could not find a package configuration file provided by "SDL2" with any of the following names:`
## mac/linux(not tested):
Install [brew](brew.sh)
```
brew install sdl2
brew install cmake
mkdir build
cd build
cmake ..
make
```



