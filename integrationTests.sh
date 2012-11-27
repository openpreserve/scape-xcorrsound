#!/bin/bash

set -o errexit

datadir=$1

host=scape@iapetus

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

tar -cvzf scape_xcorrsound.tgz migrationQA xcorrSound sound_match *Test.sh >/dev/null

ssh $host "rm -rf working/scape_xcorrsound_jenkins && mkdir working/scape_xcorrsound_jenkins"
scp scape_xcorrsound.tgz $host:~/working/scape_xcorrsound_jenkins/
ssh $host "\
        set -o errexit
        cd working/scape_xcorrsound_jenkins/; \
        tar -xvzf scape_xcorrsound.tgz > /dev/null; \
        for file in *Test.sh; do \
                ./\$file $datadir; \
        done;"
        
