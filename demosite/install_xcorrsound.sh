#!/usr/bin/env bash

cd /vagrant
mkdir -p build
cd build
touch test && rm -r *
cmake ..
make
cpack -G DEB

# Install xcorrsound package
dpkg -i scape-xcorrsound*deb

