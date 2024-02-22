from setuptools import setup, Extension
import numpy as np


nore = Extension('tetris',
                    include_dirs=[np.get_include()],
                    sources = ['src/board.cpp', 'src/env.cpp'],
                    )

setup (name = 'tetris_c_nore',
       version = '0.1',
       description = 'Modern tetris python library, implemented in C++ for speed. No rendering in this version. Use the version without nore for render, rendering might require SDL2 if no binary is built on your platform',
       author='TFW',
       author_email='tfwplssub@gmail.com',
       url='https://github.com/TheFantasticWarrior/tetris-cpy',
       ext_modules = [nore])
