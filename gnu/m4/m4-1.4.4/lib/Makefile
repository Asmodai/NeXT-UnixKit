## Makefile.in -- Makefile for GNU m4 library.
##
## Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; See the file COPYING.  if not, write to
## the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
## Boston, MA 02110-1301, USA.

PACKAGE = m4
VERSION = 1.4.4

SHELL = /bin/sh
srcdir = .


AR = ar
CC = gcc
CFLAGS = -O3 -arch m68k -arch i386 -arch hppa -arch sparc  -posix
CPPFLAGS = 
DEFS = -DHAVE_CONFIG_H
RANLIB = ranlib

COMPILE = $(CC) -c $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

.SUFFIXES:
.SUFFIXES: .c .o
.c.o:
	$(COMPILE) $<

INCLUDES = -I.. -I$(srcdir)

HEADERS = getopt.h obstack.h regex.h
SOURCES = regex.c getopt.c getopt1.c error.c obstack.c xmalloc.c \
xstrdup.c alloca.c strtol.c
OBJECTS = regex.o getopt.o getopt1.o error.o obstack.o xmalloc.o \
xstrdup.o  

DISTFILES = COPYING.LIB Makefile.in $(HEADERS) $(SOURCES) \
TAGS

all: libm4.a

libm4.a: $(OBJECTS)
	rm -f libm4.a
	$(AR) cru libm4.a $(OBJECTS)
	$(RANLIB) libm4.a

$(OBJECTS): ../config.h

install: all

uninstall:

tags: TAGS

TAGS: $(HEADERS) $(SOURCES)
	cd $(srcdir) && etags $(HEADERS) $(SOURCES)

mostlyclean:
	rm -f *.o

clean: mostlyclean
	rm -f libm4.a

distclean: clean
	rm -f Makefile

realclean: distclean
	rm -f TAGS

dist: $(DISTFILES)
	@echo "Copying distribution files"
	@for file in $(DISTFILES); do \
	  ln $(srcdir)/$$file ../$(PACKAGE)-$(VERSION)/lib 2> /dev/null \
	    || cp -p $(srcdir)/$$file ../$(PACKAGE)-$(VERSION)/lib; \
	done

Makefile: Makefile.in ../config.status
	cd .. && CONFIG_FILES=lib/$@ CONFIG_HEADERS= ./config.status

# Tell versions [3.59,3.63) of GNU make not to export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
