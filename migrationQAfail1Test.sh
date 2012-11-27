#!/bin/bash

datadir=$1

command="./migrationQA \"$datadir/P1_1800_2000_040712_001.mp3.ffmpeg.short.wav\" \"$datadir/P1_1800_2000_040712_001.mp3.mpeg321.short.wav\" --verbose"

echo "Execution \"$command\" on $(hostname)"

$command 2>&1
returncode=$?
if [ $returncode -eq 1 ]; then
        exit 0;
else
        exit $returncode;
fi


