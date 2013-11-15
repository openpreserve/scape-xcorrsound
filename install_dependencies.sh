#!/bin/bash
#####################################
### only run this script directly ###
### after exporting from github   ###
### and only run it once.         ###
#####################################

# This script installs dependencies

sudo apt-get install libc6
sudo apt-get install libboost-program-options-dev
sudo apt-get install libfftw3-dev

# On Fedora run
# sudo yum install glibc
# sudo yum install boost-devel
# sudo yum install fftw-devel

# ON Mac OS X run
# brew install boost
# brew install fftw
