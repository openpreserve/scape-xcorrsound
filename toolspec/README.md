xcorrSound SCAPE Toolspecs
=============================
*Automatic Component Creation for Discovery*

## Description

The tool specification (.xml) and component specification (.component) in this directory
can be used by the [scape-toolwrapper](http://openplanets.github.io/scape-toolwrapper/) to
create bash scripts, [Taverna](http://www.taverna.org.uk/) Workflows, components, and
Debian packages for the tools in the xcorrSound project.

### How to

See the [scape-toolwrapper](http://openplanets.github.io/scape-toolwrapper/) README.

### What for

ToolWrapper is used within the SCAPE project to allow easy finding of existing
preservation components, easily using and combining them in preservation workflows
using [Taverna](http://www.taverna.org.uk/) and easily deploying them in execution platforms
using Debian packages.

### Catch

There is however already a debian package for xcorrSound as part of the tool releases
[https://github.com/openplanets/scape-xcorrsound/releases](https://github.com/openplanets/scape-xcorrsound/releases).
The automatically generated scape components however use the automatically generated scape debian package tool.

## Components on myExperiment

The 2.0.2 version of the tools already have components on myExperiment, see
[Pack: Audio Video Quality Assurance](http://www.myexperiment.org/packs/597.html).
