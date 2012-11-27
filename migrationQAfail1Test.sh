#!/bin/bash

set -o errexit

datadir=$1

./migrationQA "$datadir/P1_1800_2000_040712_001.mp3.ffmpeg.short.wav" "$datadir/P1_1800_2000_040712_001.mp3.mpeg321.short.wav" --verbose
