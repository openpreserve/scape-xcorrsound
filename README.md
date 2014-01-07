# XCORRSOUND

Compare Sound Waves using Cross Correlation.

### What does XCORRSOUND do?

The xcorrSound package contains several tools.

* overlap-analysis is a tool to find the overlap between two audio files.
* waveform-compare is a tool that splits two audio files into equal sized blocks and outputs the correlation for each block (a_i,b_i), if a and b was the input.
* sound-match is a tool to find the location of a small wavefile inside a big wavefile

The tools all make use of cross correlation, which can be computed through the Fourier transform.

Note that all the tools will need write access to a file called <tool>.log where various information about a run will be stored.

### What are the benefits for end user?
The automisation of manual processes offers a big performance improvement.

* you can find overlaps between audio files automatically
* you can perform quality assurance of all the files automatically after a mass audio file migration

### Who is intended audience?
XCORRSOUND is for:

* Institutions who disseminate audio content
* Institutions that preserve audio collection

## Features and roadmap

### Version 2.0.0

* Feature 1
* Feature 2

### Roadmap

* Feature 3
* Feature 4

## How to install and use

### Requirements

To install you need:

* FFTW and
* Boost

### Download

You can download the debian package from https://bintray.com/openplanets/opf-debian/xcorrsound_amd64

### Install instructions

On Debian/Ubuntu you can install the debian package using this command:

    sudo dpkg -i scape-xcorrsound*deb

### Use

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


## More information

### Publications

#### Conference paper
* Bolette Ammitzbøll Jurik and Jesper Sindahl Nielsen:
  Audio Quality Assurance: An Application of Cross Correlation.
  In: iPRES 2012 – Proceedings of the 9th International Conference on Preservation of Digital Objects. Toronto 2012, 144-149.
  ISBN 978-0-9917997-0-1 (http://www.scape-project.eu/publication/audio-quality-assurance)

#### Blog posts
* http://openplanetsfoundation.com/blogs/2012-07-09-xcorrsound-waveform-compare-new-audio-quality-assurance-tool
* http://openplanetsfoundation.com/blogs/2013-05-21-sound-challenge-and-easter-egg-goes

### Licence

XCORRSOUND is copyright 2012 State and University Library, Denmark
released under GPLv2, see ./COPYING or http://www.gnu.org/licenses/gpl-2.0.html

### Acknowledgements

Part of this work was supported by the European Union in the 7th Framework Program, IST, through the SCAPE project, Contract 270137.

## Develop

* https://github.com/openplanets/scape-xcorrsound
* https://travis-ci.org/openplanets/scape-xcorrsound

### Requirements

To build you require:

* C++
* cmake

And the tools depend on FFTW3 and Boost program options. These can be found as debian packages.
And as rpm packages. On Mac OS X the depdencies can be installed using Homebrew (http://brew.sh).
See install_dependencies.sh

Preparations Debian/Ubuntu:

    sudo apt-get install libfftw3-dev libboost-all-dev

Preparations Centos/Fedora:

    sudo yum install gcc gcc-c++ glibc boost-devel fftw-devel

Preparations Mac OS X run

    brew install boost
    brew install fftw

### Build

To buil the xcorrSound tool suite follow these steps:
Download the repository, change directory to the repository and run:

    mkdir build
    cd build
    cmake ..
    make

You will now find all the tools in build/apps/

### Build Debian Package

Inside the build directory run:

    cpack -G DEB

Now the .deb package file is in the build directory.
To install

    sudo dpkg -i scape-xcorrsound*deb

### Contribute

1. [Fork the GitHub project](https://help.github.com/articles/fork-a-repo)
2. Change the code and push into the forked project
3. [Submit a pull request](https://help.github.com/articles/using-pull-requests)

To increase the changes of you code being accepted and merged into the official source here's a checklist of things to go over before submitting a contribution. For example:

* Has unit tests (that covers at least 80% of the code)
* Has documentation (at least 80% of public API)
* Agrees to contributor license agreement, certifying that any contributed code is original work and that the copyright is turned over to the project
