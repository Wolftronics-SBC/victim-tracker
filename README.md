# emily-tracker
Track a victim in the video feed to get its center and size in the image frame.

## Graphical User Interface

Select victim by holding CTRL key and making selectiong using left mouse button.

Zoom with mouse wheel.

Drag with left mouse button.

Show backprojection mode by pressing b key. Switch back by pressing b again.

Press escape key to exit.

## OpenCV Installation on macOS

1. Install Homebrew

    /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

2. Install OpenCV with ffmpeg support

    brew tap homebrew/science

    brew install opencv3 --with-contrib --with-ffmpeg --with-tbb --with-qt5

3. Link OpenCV

    brew link --force --override opencv3

4. Install CMake from cmake.org

5. Delete the following files from the root directory if they exist (those are system specific files that you will have to generate again on your system):

    cmake_install.cmake

    CMakeCache.txt

    CMakeFiles

    EMILYVictimTracker

    Makefile

6. In terminal, change directory into the root directory of the project and run the following command to generate makefile:

    cmake CMakeLists.txt

7. Compile the project using C++ compiler.
