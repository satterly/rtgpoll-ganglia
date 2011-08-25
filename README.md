RTG Poll for Ganglia
====================

Introduction
------------

This is a modified version of Router Traffic Grapher (RTG) SNMP poller that is available on sourceforge at [SourceForge](http://rtg.sourceforge.net/)

Important Differences
---------------------

This version has stripped out any reference to using MySQL as a metric data store and instead the rtgpoller submits collected SNMP metrics to Ganglia via a gmond agent. A Ganglia server can then be used for data retention via RRD's and graphing.

Build Steps
-----------
1. make sure you have the "ganglia-develop" RPM package installed on the system you are going to build rtgpoll-ganglia
2. `./configure`

To Do
-----

* basic installation instructions
* provide an RPM spec file
* Ganglia gmond.conf configuration
* sample Ganglia graphs of SNMP data

