#!/bin/bash
#####################################
### only run this script directly ###
### after exporting from svn      ###
### and only run it once.         ###
#####################################

var=$(pwd)
echo "setting up fftw.."
cd fftw-3.2.2
echo "cd " $(pwd)
./configure --prefix=$var/fftw
make
make install
cd ..
echo "cd " $(pwd)
echo "compiling xcorrSound.."
make xcorrSound
echo "creating tarball.."
tar -cf xcorrSound.tar xcorrSound
