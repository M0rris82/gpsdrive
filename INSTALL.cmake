Installation Instructions for CMake
====================================

You can go to the build directory in the gpsdrive source tree and run the
build_make.sh file or build it manually.

This is not the CMake documentation so take a look at http://www.cmake.org/ for
more details. You need at least version 2.4.3 of CMake.

Short instructions
-------------------

cmake -DCMAKE_INSTALL_PREFIX=/usr /path/to/gpsdrive/source
make
make install

