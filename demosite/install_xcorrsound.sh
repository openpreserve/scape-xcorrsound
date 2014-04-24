#!/usr/bin/env bash

cd /vagrant
mkdir build
cd build
rm -r *
cmake ..
make
cpack -G DEB

# Install xcorrsound package
dpkg -i scape-xcorrsound*deb

