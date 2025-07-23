#!/bin/bash

#-----------------------------------------------------------------------#
# OpenRGB E1.31 Receiver Plugin Build Script                            #
#-----------------------------------------------------------------------#

#set -x
#set -e

#-----------------------------------------------------------------------#
# Setup environment                                                     #
#-----------------------------------------------------------------------#
if [ "$1" = "qt6" ]; then
    export QT_SELECT=qt6
else
    export QT_SELECT=qt5
fi

if [ "$QT_SELECT" = "qt6" ]; then
    export QMAKE=qmake6
else
    export QMAKE=qmake
fi

#-----------------------------------------------------------------------#
# Configure build files with qmake                                      #
# we need to explicitly set the install prefix, as qmake's default is   #
# /usr/local for some reason...                                         #
#-----------------------------------------------------------------------#
$QMAKE .

#-----------------------------------------------------------------------#
# Build project and install files into AppDir                           #
#-----------------------------------------------------------------------#
make -j$(nproc)