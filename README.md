xcorrSound Installation & Use
=============================
*Improve Your Digital Audio Recordings*

## Installation Guide

### Requirements

To install you need:

* FFTW and
* Boost

### Download

You can download the debian package from https://bintray.com/openplanets/opf-debian/xcorrsound_amd64

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

## License

XCORRSOUND is copyright 2012 State and University Library, Denmark
released under GPLv2, see [COPYING](https://github.com/openplanets/scape-xcorrsound/blob/master/COPYING) or http://www.gnu.org/licenses/gpl-2.0.html

## Features

### Version 2.0.0

* _waveform-block-similarity_ in the output
* Micro site

