#!/bin/bash

set -o errexit

datadir=$1

#sudo apt-get install libboost-dev
#sudo apt-get install libboost-program-options-dev

make clean
var=$(pwd)
if [ ! -d "$var/fftw" ]; then 
        echo "Setting up fftw.."
        cd fftw-3.2.2
        ./configure --prefix=$var/fftw
        make
        make install
        cd ..
fi


make migrationQA

make xcorrSound

make soundMatch


for file in *Test.sh; do
        ./$file $datadir
done

