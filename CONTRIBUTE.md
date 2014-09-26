Contribute To xcorrSound
========================
*Improve Your Digital Audio Recordings*

## Develop

* https://github.com/openplanets/scape-xcorrsound
* https://travis-ci.org/openplanets/scape-xcorrsound

### Requirements

To build you require:

* C++
* cmake

The tools depend on FFTW3 and Boost program options. These can be found as debian packages.
And as rpm packages. On Mac OS X the dependencies can be installed using Homebrew (http://brew.sh).
To build you also need ronn / ruby-ronn / rubygem-ronn

Preparations Debian/Ubuntu:

    sudo apt-get install libfftw3-dev libboost-all-dev ruby-ronn

Preparations Centos/Fedora:

    sudo yum install gcc gcc-c++ glibc boost-devel fftw-devel rubygem-ronn

Preparations Mac OS X run

    brew install boost
    brew install fftw
    brew install ronn ??? NOT TESTED

### Build

To build the xcorrSound tool suite follow these steps:
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

### Create Taverna workflows and Components

See toolspecs and README in [toolspec/](https://github.com/openplanets/scape-xcorrsound/toolspec/) directory.

## Contribute

1. [Fork the GitHub project](https://help.github.com/articles/fork-a-repo)
2. Change the code and push into the forked project
3. [Submit a pull request](https://help.github.com/articles/using-pull-requests)

To increase the chances of your changes being accepted and merged into the official source here's a checklist of things to go over before submitting a contribution. For example:

* Has unit tests (that covers at least 80% of the code)
* Has documentation (at least 80% of public API)
* Agrees to contributor license agreement, certifying that any contributed code is original work and that the copyright is turned over to the project

## Roadmap

* Demo site updates
* User stories on Micro site
