#!/bin/bash

# Usage for build only:
# ./__build_cmake.sh
#
# Usage for build and run:
# ./__build_cmake.sh --run
# or
# ./__build_cmake.sh -r

# Remove managerligipolskiej2002 file if exist
[ -f managerligipolskiej2002 ] && rm managerligipolskiej2002

cmake -S . -B build/cmake/ -DCMAKE_BUILD_TYPE=Release
cd build/cmake
make

if [ $? -eq 0 ]; then
    # make succeeded

    cd ../../

    if [ "$1" = "--run" -o "$1" == "-r" ]; then
        # Run game
        ./managerligipolskiej2002
    fi
else
    # make failed
    cd ../../
fi
