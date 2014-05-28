#!/usr/bin/env bash

cd /vagrant
mkdir -p build
cd build
touch test && rm -r *
cmake ..
make VERBOSE=1
cpack -G DEB

sudo apt-get -y remove scape-xcorrsound
# Install xcorrsound package
dpkg -i scape-xcorrsound*deb

