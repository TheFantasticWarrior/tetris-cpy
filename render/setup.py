from setuptools import setup, Extension
import numpy as np

module1 = Extension('tetris',
                    define_macros = [('RENDER', '1')],
                    include_dirs=[#'/usr/include/SDL2', # adjust if needed, or use env var
                                  "../src",np.get_include()],
                    libraries=['SDL2'],
                    #library_dirs=['/usr/lib'], # adjust if needed
                    sources = ['src/board.cpp', 'src/env.cpp'],
                    )

setup (name = 'tetris_c',
       version = '0.1',
       description = 'Modern tetris python library, implemented in C++ for speed. Requires SDL2 to render, if you dont want rendering choose the tetris_c_nore version',
       author='TFW',
       author_email='tfwplssub@gmail.com',
       url='https://github.com/TheFantasticWarrior/tetris-cpy',
       ext_modules = [module1])

