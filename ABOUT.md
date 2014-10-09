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
All the implemented algorithms rely on the [Cross Correlation](http://en.wikipedia.org/wiki/Cross-correlation) procedure. 

### Waveform compare

The input is two wav files of the same length (n), sample-rate, bit-rate
and so on. The output is a real value between 0 and 1 indicating how
similar the two files are (content-wise) where 0 indicates no
similarity and 1 indicates they are identical.

The algorithm splits the two files f and g into blocks f_1, f_2, ...,
f_{n/B} and g_1, g_2, ..., g_{n/B} all with the same length B. That
is, the first block consists of the first B samples, the second block
of the following B samples from the respective files, and so on. Then
cross correlation is applied on all corresponding blocks, f_i and
g_i. The peak value of the cross correlation tells how much to shift
one block in time to achieve the best match value -- we call this the
offset of the block. If there is a block where the offset is more than
500 samples away from the offset of the first block, then an error is
reported and f and g are deemed different. Otherwise a the minimum
match value among the blocks is reported as well as the offset in that
block.

This algorithm has a low memory use that is proportional to the size
of the blocks.

### Overlap analysis

 The input is two wav files such that the last part (unknown how much)
of the first appears as the first part (also unknown how much) of the
second file -- content-wise.  The input is two wav files of the same
length (n), sample-rate, bit-rate and so on. The output is a length
and a real value between 0 and 1 indicating how good the match is.

The algorithm does one cross correlation computation of the two input
files and outputs the peak position and value. This means the memory
usage is proportional to the size of the input files which is quite
memory intensive compared to the input. The use case for this tool is
to find small overlaps e.g. a few minutes.


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
