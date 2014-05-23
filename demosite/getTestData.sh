#!/bin/bash

SCRIPT_PATH=$(dirname $(readlink -f $0 ) )

echo "$SCRIPT_PATH"
set -o errexit

datadir="$SCRIPT_PATH/sample"
fileServer="https://sbforge.org/downloads/scape/scape_xcorrsound_jenkins_files"

echo "Getting demo site sample content from $fileServer"
mkdir -p "$datadir"
cd "$datadir"
wget -r -np -nd -N -q "$fileServer/"
cd ..

cd "$datadir" && find -type f -not -name '*.wav' | xargs rm

ln -s "$datadir/P1_1800_2000_040712_001.mp3.ffmpeg.short.wav" "$datadir/before.wav"
ln -s "$datadir/P1_1800_2000_040712_001.mp3.mpeg321.short.wav" "$datadir/after.wav"


#This takes forever, so skip it
#gitServer=https://github.com/statsbiblioteket/xcorrsound-test-files.git
#echo "Getting demo site sample content from $gitServer"
#rm -rf xcorrsound-test-files/
#git clone $gitServer