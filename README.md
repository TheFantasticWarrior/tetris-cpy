# Tetris with instant soft drop delay(branch python library)
Following tetris guidelines, with instant softdrop after specified time and short softdrop like DAS

This branch currently breaks everything else other than target sim(python library)
# Python Usage
```python
import sim
import numpy as np

x=sim.Container() # game object
y=x.copy() # deep copy, doesn't survive passing through processes
game_state=x.get_state() # shape (1476,) numpy array
# for reconstructing game object through processes or if you're interested in next pieces
hidden_queue=x.get_hidden() 

atk=x.get_atk() # ((attack is to player 1),attacks)

# reconstruct
new_x.Container((
np.concatenate([game_state[:222],game_state[738:960]]),
*hidden_queue, sum(atk[1])*(-1 if atk[0] else 1), atk[1]
))

"""
to train your AI/whatever
game_state[:12] #info
# board:
game_state[12:642].reshape(3,10,21)
# shape of hold and queue
game_state[642:].reshape(-1, 6, 4, 4)
"""
"""render
Renderer(mode,size)
or
Renderer().create_window(mode,size)
mode 1: pixel size=size
mode 2: screen width=size
mode 3: screen height=size
"""

r=sim.Renderer(1,30) 
r.render(x)

# close window
r.close()
```
# Install:
After Python3 add 3.x EXACT in CMakeLists.txt if you have more than 1 python versions installed
## Windows:
install sdl:

Visual Studio: `vcpkg install sdl:x64-windows`\
replace `cmake ..` with `cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/user/vcpkg/scripts/buildsystems/vcpkg.cmake` later when building

MinGW: [Link](https://github.com/libsdl-org/SDL/releases/latest)

install [cmake](https://cmake.org/download/)
## Mac:
On Mac install [brew](https://brew.sh)(package manager for mac)

Install cmake and sdl2(sometimes libsdl2-dev)

# Cmake Install
```cmake
mkdir build
cd build
cmake ..
sudo cmake --build . --target sim --target install --config Release
```


