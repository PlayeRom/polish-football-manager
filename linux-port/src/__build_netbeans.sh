#!/bin/bash

# Since I lost the ability to run the project in NetBeans 8.2 I wrote this
# simple script to run the compilation configured in NetBeans. Here the Makefile
# and configuration in the nbproject directory is used.

# To switch the build mode from Release to Debug, edit the
# nbproject/Makefile-impl.mk file and change the value for DEFAULTCONF.

# Usage for build only:
# ./__build_netbeans.sh
#
# Usage for build and run:
# ./__build_netbeans.sh --run
# or
# ./__build_netbeans.sh -r

# Remove managerligipolskiej2002 file if exist
[ -f managerligipolskiej2002 ] && rm managerligipolskiej2002

make

if [ $? -eq 0 ]; then
    # make succeeded

    cp dist/Release/GNU-Linux/managerligipolskiej2002 ./managerligipolskiej2002

    if [ "$1" = "--run" -o "$1" == "-r" ]; then
        # Run game
        ./managerligipolskiej2002
    fi
fi
