#!/bin/bash

set -o errexit
echo "Running succes test 1"

datadir=$1

command="./migrationQA $datadir/P1_1800_2000_031001_001.mp3.ffmpeg.wav $datadir/P1_1800_2000_031001_001.mp3.mpg321.wav --verbose"

echo "Execution \"$command\" on $(hostname)"

$command

echo "Test passed"

