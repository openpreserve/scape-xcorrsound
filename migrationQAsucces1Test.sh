#!/bin/bash

set -o errexit

datadir=$1

command="./migrationQA $datadir/P1_1800_2000_031001_001.mp3.ffmpeg.wav $datadir/P1_1800_2000_031001_001.mp3.mpg321.wav --verbose"

echo "Execution \"$command\" on $(hostname)"

$command 2>&1


