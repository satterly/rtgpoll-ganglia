## Makefile.am -- Process this file with automake to produce Makefile.in

GANG_LIB_DIR       = /usr/lib64
INCLUDES           = -I$(GANG_LIB_DIR)

RTG_LIBS = $(GANG_LIB_DIR/libganglia.a

rtgpoll_SOURCES = rtgsnmp.c rtgpoll.c rtgutil.c rtghash.c

include_HEADERS = rtg.h common.h

bin_PROGRAMS = rtgpoll
