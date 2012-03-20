#!/bin/bash
#####################################
### only run this script directly ###
### after exporting from github   ###
### and only run it once.         ###
#####################################

var=$(pwd)
echo "Setting up fftw.."
cd fftw-3.2.2
echo "cd " $(pwd)
./configure --prefix=$var/fftw
make
make install
cd ..
echo "cd " $(pwd)
