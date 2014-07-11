xcorrSound
==========
*Improve Your Digital Audio Recordings*

What is xcorrSound?
-------------------
xcorrSound consists of four tools:

* overlap-analysis detects overlaps in two audio files
* sound-match finds occurrences of smaller WAV within a larger WAV
* waveform-compare splits two audio files into equal sized blocks and outputs the correlation for each block
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
The State and University Library in Denmark holds a large collection of digitised audio recordings, originally recorded on two-hour tapes, with overlaps from tape to tape. To enhance the user experience, the library wanted to eliminate the overlaps and make the broadcast a continuous stream. This was done by using xcorrSound, a tool package for audio materials.

In xcorrSound, algorithms use cross correlation to compare the sound waves. With this an automated overlap analysis of the audio recordings was conducted. This enabled the library to cut and put together the resulting trimmed files in 24 hour blocks which enabled improvement of the end users' listening experience. 

Publications
------------

### Conference paper

* Bolette Ammitzbøll Jurik and Jesper Sindahl Nielsen: [Audio Quality Assurance: An Application of Cross Correlation](http://www.scape-project.eu/publication/audio-quality-assurance). In: iPRES 2012 Proceedings of the 9th International Conference on Preservation of Digital Objects. Toronto 2012, 144-149. ISBN 978-0-9917997-0-1

### Blog posts

* [http://openplanetsfoundation.com/blogs/2012-07-09-xcorrsound-waveform-compare-new-audio-quality-assurance-tool](http://openplanetsfoundation.com/blogs/2012-07-09-xcorrsound-waveform-compare-new-audio-quality-assurance-tool)
* [http://openplanetsfoundation.com/blogs/2013-05-21-sound-challenge-and-easter-egg-goes](http://openplanetsfoundation.com/blogs/2013-05-21-sound-challenge-and-easter-egg-goes)
* [http://openplanetsfoundation.org/blogs/2014-01-21-developing-audio-qa-workflow-using-hadoop-part-i](http://openplanetsfoundation.org/blogs/2014-01-21-developing-audio-qa-workflow-using-hadoop-part-i)
* [http://openplanetsfoundation.org/blogs/2014-02-03-developing-audio-qa-workflow-using-hadoop-part-ii](http://openplanetsfoundation.org/blogs/2014-02-03-developing-audio-qa-workflow-using-hadoop-part-ii)


Credits
-------

* This work was partially supported by the [SCAPE project](http://scape-project.eu). The SCAPE project is co-funded 
by the European Union under FP7 ICT-2009.4.1 (Grant Agreement number 270137)
* XCORRSOUND is copyright 2012 State and University Library, Denmark released under GPLv2, see ./COPYING or [http://www.gnu.org/licenses/gpl-2.0.html](http://www.gnu.org/licenses/gpl-2.0.html)
