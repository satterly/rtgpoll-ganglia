# Makefile.in generated automatically by automake 1.5 from Makefile.am.

# Copyright 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001
# Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.



#
# Makefile to install RTG support files.
#

SHELL = /bin/bash

srcdir = .
top_srcdir = ..

prefix = /usr/local/rtg
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/share/info
mandir = ${prefix}/share/man
includedir = ${prefix}/include
oldincludedir = /usr/include
pkgdatadir = $(datadir)/rtg
pkglibdir = $(libdir)/rtg
pkgincludedir = $(includedir)/rtg

ACLOCAL = ${SHELL} /home/satterly/git/rtgpoll-ganglia-0.8/config/missing --run aclocal
AUTOCONF = ${SHELL} /home/satterly/git/rtgpoll-ganglia-0.8/config/missing --run autoconf
AUTOMAKE = ${SHELL} /home/satterly/git/rtgpoll-ganglia-0.8/config/missing --run automake
AUTOHEADER = ${SHELL} /home/satterly/git/rtgpoll-ganglia-0.8/config/missing --run autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}
INSTALL_HEADER = $(INSTALL_DATA)
transform = s,x,x,
NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = 
host_triplet = x86_64-unknown-linux-gnu
AMTAR = ${SHELL} /home/satterly/git/rtgpoll-ganglia-0.8/config/missing --run tar
AWK = gawk
CC = gcc
CGI_LIB_DIR = cgilib-0.4
CPP = gcc -E
DEPDIR = .deps
EXEEXT = 
GD_LIB_DIR = gd-1.8.4
INSTALL_STRIP_PROGRAM = ${SHELL} $(install_sh) -c -s
LN_S = ln -s
MYSQL_DIR = 
OBJEXT = o
PACKAGE = rtg
PNG_LIB_DIR = libpng-1.2.1
RANLIB = ranlib
RTG_HOME = /usr/local/rtg
VERSION = 0.7.4
ZLIB_LIB_DIR = zlib-1.1.4
ac_aux_dir = config
am__include = include
am__quote = 
install_sh = /home/satterly/git/rtgpoll-ganglia-0.8/config/install-sh

top_builddir = ..
etcdir = /etc
webdir = /web
reportsdir = /bin

ETC = rtg.conf rtgtargmkr.pl routers createdb BER.pm \
               SNMP_Session.pm SNMP_util.pm

WEB = rtg.php 95.php view.php common.php rtg.png rtgback.png
REPORTS = report.pl 95.pl

EXTRA_DIST = rtgtargmkr.pl.in report.pl 95.pl createdb.in \
               BER.pm SNMP_Session.pm SNMP_util.pm rtg.conf \
	       routers rtg.php 95.php view.php common.php.in rtg.png rtgback.png

subdir = etc
mkinstalldirs = $(SHELL) $(top_srcdir)/config/mkinstalldirs
CONFIG_HEADER = $(top_builddir)/config/config.h
CONFIG_CLEAN_FILES = createdb rtgtargmkr.pl common.php
DIST_SOURCES =
DIST_COMMON = Makefile.am Makefile.in common.php.in createdb.in \
	rtgtargmkr.pl.in
all: all-am

.SUFFIXES:
$(srcdir)/Makefile.in:  Makefile.am  $(top_srcdir)/configure.in $(ACLOCAL_M4)
	cd $(top_srcdir) && \
	  $(AUTOMAKE) --foreign  etc/Makefile
Makefile:  $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) && \
	  CONFIG_HEADERS= CONFIG_LINKS= \
	  CONFIG_FILES=$(subdir)/$@ $(SHELL) ./config.status
createdb: $(top_builddir)/config.status createdb.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= CONFIG_LINKS= $(SHELL) ./config.status
rtgtargmkr.pl: $(top_builddir)/config.status rtgtargmkr.pl.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= CONFIG_LINKS= $(SHELL) ./config.status
common.php: $(top_builddir)/config.status common.php.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= CONFIG_LINKS= $(SHELL) ./config.status
uninstall-info-am:
tags: TAGS
TAGS:


DISTFILES = $(DIST_COMMON) $(DIST_SOURCES) $(TEXINFOS) $(EXTRA_DIST)

top_distdir = ..
distdir = $(top_distdir)/$(PACKAGE)-$(VERSION)

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  if test -f $$file; then d=.; else d=$(srcdir); fi; \
	  dir=`echo "$$file" | sed -e 's,/[^/]*$$,,'`; \
	  if test "$$dir" != "$$file" && test "$$dir" != "."; then \
	    $(mkinstalldirs) "$(distdir)/$$dir"; \
	  fi; \
	  if test -d $$d/$$file; then \
	    cp -pR $$d/$$file $(distdir) \
	    || exit 1; \
	  else \
	    test -f $(distdir)/$$file \
	    || cp -p $$d/$$file $(distdir)/$$file \
	    || exit 1; \
	  fi; \
	done
check-am: all-am
check: check-am
all-am: Makefile
install-exec: install-exec-am
install-data: install-data-am
uninstall: uninstall-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am

installcheck: installcheck-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) INSTALL_PROGRAM="$(INSTALL_STRIP_PROGRAM)" \
	  `test -z '$(STRIP)' || \
	    echo "INSTALL_PROGRAM_ENV=STRIPPROG='$(STRIP)'"` install
mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES) stamp-h stamp-h[0-9]*

maintainer-clean-generic:
	@echo "This command is intended for maintainers to use"
	@echo "it deletes files that may require special tools to rebuild."
clean: clean-am

clean-am: clean-generic mostlyclean-am

distclean: distclean-am

distclean-am: clean-am distclean-generic

dvi: dvi-am

dvi-am:

info: info-am

info-am:

install-data-am:

install-exec-am:

install-info: install-info-am

install-man:

installcheck-am:

maintainer-clean: maintainer-clean-am

maintainer-clean-am: distclean-am maintainer-clean-generic

mostlyclean: mostlyclean-am

mostlyclean-am: mostlyclean-generic

uninstall-am: uninstall-info-am

.PHONY: all all-am check check-am clean clean-generic distclean \
	distclean-generic distdir dvi dvi-am info info-am install \
	install-am install-data install-data-am install-exec \
	install-exec-am install-info install-info-am install-man \
	install-strip installcheck installcheck-am installdirs \
	maintainer-clean maintainer-clean-generic mostlyclean \
	mostlyclean-generic uninstall uninstall-am uninstall-info-am


all:

install: installdirs
	@for i in $(ETC) ; do $(INSTALL) $(srcdir)/$$i $(prefix)$(etcdir) ; echo "install: installed $$i in $(prefix)$(etcdir)" ; done
	@for i in $(WEB) ; do $(INSTALL) $(srcdir)/$$i $(prefix)$(webdir) ; echo "install: installed $$i in $(prefix)$(webdir)" ; done
	@for i in $(REPORTS) ; do $(INSTALL) $(srcdir)/$$i $(prefix)$(reportsdir) ; echo "install: installed $$i in $(prefix)$(reportsdir)" ; done

installdirs:
	$(mkinstalldirs) $(prefix)$(etcdir)
	$(mkinstalldirs) $(prefix)$(webdir)
	$(mkinstalldirs) $(prefix)$(reportsdir)
# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
