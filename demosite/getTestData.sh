#!/bin/bash

SCRIPT_PATH=$(dirname $(readlink -f $0 ) )

echo "$SCRIPT_PATH"
set -o errexit

fileServer="https://sbforge.org/downloads/scape/scape_xcorrsound_jenkins_files"

echo "Getting demo site sample content"
mkdir -p "$SCRIPT_PATH/data"
cd "$SCRIPT_PATH/data"
wget -r -np -nd -N -q "$fileServer/"
cd ..

datadir="$SCRIPT_PATH/data"

cd "$datadir" && find -type f -not -name '*.wav' | xargs rm

git clone https://github.com/statsbiblioteket/xcorrsound-test-files.git