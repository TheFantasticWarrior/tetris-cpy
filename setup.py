from setuptools import setup, Extension
import numpy as np

module1 = Extension('tetris',
                    include_dirs=[#'/usr/include/SDL2', # adjust if needed
                                  np.get_include()],
                    libraries=['SDL2'],
                    #library_dirs=['/usr/lib'], # adjust if needed
                    sources = ['tetrisboard.cpp', 'env.cpp'],
                    include_package_data=True)

setup (name = 'tetris_c',
       version = '0.1',
       description = 'Modern tetris python library, implemented in C++ for speed. Requires SDL2',
       author='TFW',
       author_email='tfwplssub@gmail.com',
       url='https://github.com/TheFantasticWarrior/tetris-cpy',
       ext_modules = [module1])