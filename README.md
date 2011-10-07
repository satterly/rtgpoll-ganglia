RTG Poll for Ganglia
====================

Introduction
------------

This is a modified version of Router Traffic Grapher (RTG) SNMP poller that is available on sourceforge at [SourceForge](http://rtg.sourceforge.net/)

Important Differences
---------------------

This version has stripped out any reference to using MySQL as a metric data store and instead the rtgpoller submits collected SNMP metrics to Ganglia via a gmond agent running on the same host as the RTG poller. A Ganglia server can then be used for data retention via RRDs and graphs.

Enhancements to Original Version
--------------------------------

* constant value SNMP variables are supported ie. gmetric type of 'uint32' and slope 'zero'
* strings in SNMP variables are supported ie. gmetric type of 'string' and slope 'zero'
* opaque float SNMP variables are supported ie. gmetric type of 'float' and slope 'both'
* forces the use of short hostnames (yes, this is an enhancement)
* converts TimeTicks from 100ths of a second to seconds (simply by dividing result by 100)
* removed 'multi-instance' flag and replaced it with user-definable pid file location

Build Steps
-----------
1. make sure you have the "ganglia-develop" RPM package installed on the system you are going to build rtgpoll-ganglia
2. `./configure`

or just...

1. `rpmbuild -bb rtgpoll-ganglia-<version>.spec`
2. `rpm -Uvh rtgpoll-ganglia`

To Do
-----

* basic installation instructions
* sample Ganglia gmond.conf configuration
* sample Ganglia graphs of SNMP data
* refactor code as it's now a mess
