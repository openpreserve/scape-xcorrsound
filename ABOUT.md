xcorrSound
==========
*Improve Your Digital Audio Recordings*

What is xcorrSound?
-------------------
xcorrSound consists of four tools:

* overlap-analysis detects overlap in two audio files
* waveform-compare compares two audio files and outputs the similarity
* sound-match detects occurrences of a smaller audio file (e.g. a jingle) within a larger audio file or an index of audio files
* sound-index builds an index for sound-match to work within

### xcorrSound Demo Site

[http://scape.opf-labs.org/xcorrsound/index.html](http://scape.opf-labs.org/xcorrsound/index.html)

What Can xcorrSound Do For Me?
------------------------------
The automisation of manual processes offers an important performance improvement.
xcorrSound brings the following benefits:

* precision in the overlap analysis
* automated processes
* resource efficiency
* open source: freely available
* easy to install and integrate into a workflow (command line tool)
* leads to an improved and optimised end user experience

xcorrSound Can Be Used By
-------------------------

* Institutions disseminating audio content
* Institutions preserving audio collections

Examples
--------
The State and University Library in Denmark holds a large collection of digitised audio recordings, originally recorded
on two-hour tapes, with overlaps from tape to tape. To enhance the user experience, the library wanted to eliminate the
overlaps and make the broadcast a continuous stream. This was done by using xcorrSound overlap-analysis.

In xcorrSound overlap-analysis, algorithms use cross correlation to compare the sound waves. With this an automated overlap analysis of
the audio recordings was conducted. This enabled the library to cut and put together the resulting trimmed files in 24 hour blocks
which enabled improvement of the end users' listening experience.

Algorithms
----------
TODO

Publications
------------

### Leaflet

* [xcorrSound – Improve Your Digital Audio Recordings](www.scape-project.eu/leaflets/xcorrsound-improve-your-digital-audio-recordings)

### Conference paper

* Bolette Ammitzbøll Jurik and Jesper Sindahl Nielsen: [Audio Quality Assurance: An Application of Cross Correlation](http://www.scape-project.eu/publication/audio-quality-assurance). In: iPRES 2012 Proceedings of the 9th International Conference on Preservation of Digital Objects. Toronto 2012, 144-149. ISBN 978-0-9917997-0-1

### Blog posts

* [xcorrSound: waveform-compare New Audio Quality Assurance Tool](http://openplanetsfoundation.com/blogs/2012-07-09-xcorrsound-waveform-compare-new-audio-quality-assurance-tool)
* [Sound Challenge: And the Easter Egg goes to ...](http://openplanetsfoundation.com/blogs/2013-05-21-sound-challenge-and-easter-egg-goes)
* [Developing an Audio QA workflow using Hadoop: Part I](http://openplanetsfoundation.org/blogs/2014-01-21-developing-audio-qa-workflow-using-hadoop-part-i)
* [Developing an Audio QA workflow using Hadoop: Part II](http://openplanetsfoundation.org/blogs/2014-02-03-developing-audio-qa-workflow-using-hadoop-part-ii)
* [Scape Demonstration: Migration of audio using xcorrSound](http://openplanetsfoundation.org/blogs/2014-09-30-scape-demonstration-migration-audio-using-xcorrsound)

### SlideShare

* [Audio Quality Assurance. An application of cross correlation](http://www.slideshare.net/SCAPEproject/audio-quality-assurance-an-application-of-cross-correlation)
* [Migration of audio files using Hadoop - and Taverna - and xcorrSound waveform-compare](www.slideshare.net/SCAPEproject/scape-information-day-sb-migration-xcorr-sound)

### Vimeo

* [Scape Demonstration: Migration of audio using xcorrSound by Bolette Ammitzbøll Jurik](https://vimeo.com/107573486)

### Components

* [Pack: Audio Video Quality Assurance](http://www.myexperiment.org/packs/597.html)

Credits
-------

* This work was partially supported by the [SCAPE project](http://scape-project.eu). The SCAPE project is co-funded 
by the European Union under FP7 ICT-2009.4.1 (Grant Agreement number 270137)
* XCORRSOUND is copyright 2012 State and University Library, Denmark released under GPLv2, see ./COPYING or [http://www.gnu.org/licenses/gpl-2.0.html](http://www.gnu.org/licenses/gpl-2.0.html)
