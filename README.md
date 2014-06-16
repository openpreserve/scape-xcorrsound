xcorrSound Installation & Use
=============================
*Improve Your Digital Audio Recordings*

## Installation Guide

### Requirements

To install you need:

* FFTW and
* Boost

To build you also need

* ronn / ruby-ronn / rubygem-ronn

### Download

You can download the debian package from [https://github.com/openplanets/scape-xcorrsound/releases](https://github.com/openplanets/scape-xcorrsound/releases)

### Installing xcorrSound

On Debian/Ubuntu you can install the debian package using this command:

    sudo dpkg -i scape-xcorrsound*deb

## Using xcorrSound

Example: overlap-analysis

Syntax:

    overlap-analysis <file1.wav> <file2.wav> [v<log level>]

Output example:

    The best match was below the threshold of 0.2
    Best at sample number: 11520000
    Best at second: 240
    Value of match was: 0.132016

    In the log various information relating to the wav files can be found,
    such as samplerate, number of channels and so on.

    ----------------------- wav header start -----------------------
    ChunkID: RIFF$ËnWAVE
    ChunkSize: 57600036
    Format: WAVE
    ----------------------- wav header end -----------------------
    ----------------------- wav subchunk start -----------------------
    Subchunk1ID: fmt 
    Subchunk1Size: 16
    AudioFormat: 1
    NumChannels: 2
    SampleRate: 48000
    ByteRate: 192000
    BlockAlign: 4
    BitsPerSample: 16
    Subchunk2ID: data
    Subchunk2Size: 57600000
    ----------------------- wav subchunk end -----------------------
    ----------------------- wav header start -----------------------
    ChunkID: RIFF$ËnWAVE
    ChunkSize: 57600036
    Format: WAVE
    ----------------------- wav header end -----------------------
    ----------------------- wav subchunk start -----------------------
    Subchunk1ID: fmt 
    Subchunk1Size: 16
    AudioFormat: 1
    NumChannels: 2
    SampleRate: 48000
    ByteRate: 192000
    BlockAlign: 4
    BitsPerSample: 16
    Subchunk2ID: data
    Subchunk2Size: 57600000
    ----------------------- wav subchunk end -----------------------

## Building xcorrSound

To install follow these steps:

The tools depend on FFTW3, Boost and Ronn program options. These can be found as debian packages.
And as rpm packages. On Mac OS X the dependencies can be installed using
Homebrew (http://brew.sh). See install_dependencies.sh

Download the repository, change directory to the repository and run:

    mkdir build
    cd build
    cmake ..
    make

You will now find all the tools in build/apps/

### Building xcorrSound Debian Package

Inside the build directory run:

    cpack -G DEB

Now the .deb package file is in the build directory.
To install

    sudo dpkg -i scape-xcorrsound*deb


## Using Vagrant

The project includes a Vagrant config file, which uses Virtual Box to start an Ubuntu 12.04 instance from which you can build and use the tool. This should work on any environment that supports Vagrant and Virtual Box. When you have Vagrant and Virtual Box installed, the following should work:

    vagrant up
    vagrant ssh
    sudo /vagrant/demosite/install_xcorrsound.sh

Now you should have a working xcorrSound installation and test files. Try

    waveform-compare /vagrant/demosite/sample/P1_1800_2000_040712_001.mp3.mpeg321.short.wav /vagrant/demosite/sample/P1_1800_2000_040712_001.mp3.ffmpeg.short.wav

## License

XCORRSOUND is copyright 2012 State and University Library, Denmark
released under GPLv2, see [COPYING](https://github.com/openplanets/scape-xcorrsound/blob/master/COPYING) or http://www.gnu.org/licenses/gpl-2.0.html

## Features

### Version 2.0.2

* CMAKE flags for release, ie. optimized heavily

### Version 2.0.1

* bugfix with silence detection

### Version 2.0.0

* _waveform-block-similarity_ in the output
* Micro site

