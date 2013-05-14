#!/bin/bash


echo "Running fail test 1"
datadir=$1

command="./waveform-compare $datadir/P1_1800_2000_040712_001.mp3.ffmpeg.short.wav $datadir/P1_1800_2000_040712_001.mp3.mpeg321.short.wav --verbose"

echo "Execution \"$command\" on $(hostname)"

$command
returncode=$?
if [ $returncode -eq 1 ]; then
        echo "Test passed, Failure expected result"
        exit 0;
else
        echo "Test failed, files should not be alike"
        exit $returncode;
fi


