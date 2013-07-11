#!/bin/bash

SCRIPT_PATH=$(pwd)

set -o errexit

fileServer="https://sbforge.org/downloads/scape/scape_xcorrsound_jenkins_files"

mkdir -p "$SCRIPT_PATH/data"
cd "$SCRIPT_PATH/data"
wget -r -np -nd -N "$fileServer/"
cd ..

datadir="$SCRIPT_PATH/data"

for file in $SCRIPT_PATH/*Test.sh; do 
   echo 
   $file "$datadir"
done;
      
        
