# Generated automatically from Makefile.in by configure.
#
# Makefile for top level of zsh distribution
#
# Copyright (c) 1995-1997 Richard Coleman
# All rights reserved.
#
# Permission is hereby granted, without written agreement and without
# license or royalty fees, to use, copy, modify, and distribute this
# software and to distribute modified versions of this software for any
# purpose, provided that the above copyright notice and the following
# two paragraphs appear in all copies of this software.
#
# In no event shall Richard Coleman or the Zsh Development Group be liable
# to any party for direct, indirect, special, incidental, or consequential
# damages arising out of the use of this software and its documentation,
# even if Richard Coleman and the Zsh Development Group have been advised of
# the possibility of such damage.
#
# Richard Coleman and the Zsh Development Group specifically disclaim any
# warranties, including, but not limited to, the implied warranties of
# merchantability and fitness for a particular purpose.  The software
# provided hereunder is on an "as is" basis, and Richard Coleman and the
# Zsh Development Group have no obligation to provide maintenance,
# support, updates, enhancements, or modifications.
#

subdir = .
dir_top = .
SUBDIRS = Doc Etc Src Test


#
# Makefile fragment for version numbers
#
# Copyright (c) 1995-1997 Richard Coleman
# All rights reserved.
#
# Permission is hereby granted, without written agreement and without
# license or royalty fees, to use, copy, modify, and distribute this
# software and to distribute modified versions of this software for any
# purpose, provided that the above copyright notice and the following
# two paragraphs appear in all copies of this software.
#
# In no event shall Richard Coleman or the Zsh Development Group be liable
# to any party for direct, indirect, special, incidental, or consequential
# damages arising out of the use of this software and its documentation,
# even if Richard Coleman and the Zsh Development Group have been advised of
# the possibility of such damage.
#
# Richard Coleman and the Zsh Development Group specifically disclaim any
# warranties, including, but not limited to, the implied warranties of
# merchantability and fitness for a particular purpose.  The software
# provided hereunder is on an "as is" basis, and Richard Coleman and the
# Zsh Development Group have no obligation to provide maintenance,
# support, updates, enhancements, or modifications.
#

# This must also serve as a shell script, so do not add spaces around the
# `=' signs.

VERSION=4.0.6
VERSION_DATE='August 14, 2002'

# source/build directories
sdir            = .
sdir_top        = .
INSTALL         = /usr/bin/install -c


#
# Basic Makefile definitions
#
# Copyright (c) 1995-1997 Richard Coleman
# All rights reserved.
#
# Permission is hereby granted, without written agreement and without
# license or royalty fees, to use, copy, modify, and distribute this
# software and to distribute modified versions of this software for any
# purpose, provided that the above copyright notice and the following
# two paragraphs appear in all copies of this software.
#
# In no event shall Richard Coleman or the Zsh Development Group be liable
# to any party for direct, indirect, special, incidental, or consequential
# damages arising out of the use of this software and its documentation,
# even if Richard Coleman and the Zsh Development Group have been advised of
# the possibility of such damage.
#
# Richard Coleman and the Zsh Development Group specifically disclaim any
# warranties, including, but not limited to, the implied warranties of
# merchantability and fitness for a particular purpose.  The software
# provided hereunder is on an "as is" basis, and Richard Coleman and the
# Zsh Development Group have no obligation to provide maintenance,
# support, updates, enhancements, or modifications.
#

# fundamentals
SHELL = /bin/sh

EXEEXT = 

# install basename
tzsh            = zsh

# installation directories
prefix          = /tmp/zsh_build
exec_prefix     = ${prefix}
bindir          = ${exec_prefix}/bin
libdir          = ${exec_prefix}/lib
MODDIR          = $(libdir)/$(tzsh)/$(VERSION)
infodir         = ${prefix}/info
mandir          = ${prefix}/man
datadir         = ${prefix}/share
fndir           = ${prefix}/share/zsh/${VERSION}/functions
sitefndir       = ${prefix}/share/zsh/site-functions
htmldir         = $(datadir)/$(tzsh)/htmldoc

# compilation
CC              = gcc
CPP             = gcc -E -traditional-cpp
CPPFLAGS        = 
DEFS            = -DHAVE_CONFIG_H
CFLAGS          =  -Wall -Wmissing-prototypes -O2 -arch i386 -arch m68k -arch hppa -arch sparc
LDFLAGS         = -arch i386 -arch m68k -arch hppa -arch sparc
EXTRA_LDFLAGS   = 
DLCFLAGS        = 
DLLDFLAGS       = 
LIBLDFLAGS      = 
EXELDFLAGS      = 
LIBS            = -ltermcap -lm 
DL_EXT          = 
DLLD            = 
EXPOPT          = 
IMPOPT          = 

# utilities
AWK             = gawk
ANSI2KNR        = : ansi2knr
YODL            = : yodl
YODL2TXT        = $(YODL)2txt
YODL2HTML       = $(YODL)2html

# install utility
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA    = ${INSTALL} -m 644

# variables used in determining what to install
FUNCTIONS_SUBDIRS = no

# flags passed to recursive makes in subdirectories
MAKEDEFS = \
prefix='$(prefix)' exec_prefix='$(exec_prefix)' bindir='$(bindir)' \
libdir='$(libdir)' MODDIR='$(MODDIR)' infodir='$(infodir)' mandir='$(mandir)' \
datadir='$(datadir)' fndir='$(fndir)' htmldir='$(htmldir)' \
CC='$(CC)' CPPFLAGS='$(CPPFLAGS)' DEFS='$(DEFS)' CFLAGS='$(CFLAGS)' \
LDFLAGS='$(LDFLAGS)' EXTRA_LDFLAGS='$(EXTRA_LDFLAGS)' \
DLCFLAGS='$(DLCFLAGS)' DLLDFLAGS='$(DLLDFLAGS)' \
LIBLDFLAGS='$(LIBLDFLAGS)' EXELDFLAGS='$(EXELDFLAGS)' \
LIBS='$(LIBS)' DL_EXT='$(DL_EXT)' DLLD='$(DLLD)' \
AWK='$(AWK)' ANSI2KNR='$(ANSI2KNR)' \
YODL='$(YODL)' YODL2TXT='$(YODL2TXT)' YODL2HTML='$(YODL2HTML)' \
FUNCTIONS_INSTALL='$(FUNCTIONS_INSTALL)' tzsh='$(tzsh)'

# override built-in suffix list
.SUFFIXES:

# ========== DEPENDENCIES FOR BUILDING ==========

# default target
all: config.h
	@for subdir in Src Doc; do \
	  (cd $$subdir && $(MAKE) $(MAKEDEFS) $@) || exit 1; \
	done

# prepare module configuration
prep:
	@cd Src && $(MAKE) $(MAKEDEFS) $@

META-FAQ: FORCE
	@cd Doc && $(MAKE) $(MAKEDEFS) ../META-FAQ

dvi ps html info:
	@cd Doc && $(MAKE) $(MAKEDEFS) $@

# ========== DEPENDENCIES FOR INSTALLING ==========

# install stripped
install-strip:
	$(MAKE) install STRIPFLAGS="-s"

# install/uninstall most things
install: install.bin install.modules install.fns install.man
uninstall: uninstall.bin uninstall.modules uninstall.fns uninstall.man

# install/uninstall just the binary
install.bin uninstall.bin:
	@cd Src && $(MAKE) $(MAKEDEFS) $@

# install/uninstall just the modules
install.modules uninstall.modules:
	@cd Src && $(MAKE) $(MAKEDEFS) $@

# install/uninstall just the man pages
install.man uninstall.man:
	@cd Doc && $(MAKE) $(MAKEDEFS) $@

# install/uninstall just the shell functions
install.fns:
	if test x$(fndir) != x && test x$(fndir) != xno; then \
	  test x$(sitefndir) != xno && \
	    $(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(sitefndir); \
	  sdir_top="$(sdir_top)" fndir="$(fndir)" dir_top="$(dir_top)" \
	  FUNCTIONS_SUBDIRS="$(FUNCTIONS_SUBDIRS)" \
	  INSTALL_DATA="$(INSTALL_DATA)" \
	  DESTDIR="$(DESTDIR)" VERSION="$(VERSION)" \
	  $(SHELL) $(sdir_top)/Config/installfns.sh || exit 1; \
	fi; \
	exit 0

uninstall.fns:
	if test x$(fndir) != x && test x$(fndir) != xno; then \
	  fndir="$(fndir)" dir_top="$(dir_top)" \
	  FUNCTIONS_SUBDIRS="$(FUNCTIONS_SUBDIRS)" \
	  DESTDIR="$(DESTDIR)" VERSION="$(VERSION)" \
	  $(SHELL) $(sdir_top)/Config/uninstallfns.sh || exit 1; \
	fi; \
	exit 0

# install/uninstall just the info pages
install.info uninstall.info:
	@cd Doc && $(MAKE) $(MAKEDEFS) $@

# install/uninstall just the HTML manual
install.html uninstall.html:
	@cd Doc && $(MAKE) $(MAKEDEFS) $@

# ========== DEPENDENCIES FOR TESTING ==========
check test:
	cd Test ; $(MAKE) check

# ========== DEPENDENCIES FOR CLEANUP ==========


#
# Makefile fragment for cleanup
#
# Copyright (c) 1995-1997 Richard Coleman
# All rights reserved.
#
# Permission is hereby granted, without written agreement and without
# license or royalty fees, to use, copy, modify, and distribute this
# software and to distribute modified versions of this software for any
# purpose, provided that the above copyright notice and the following
# two paragraphs appear in all copies of this software.
#
# In no event shall Richard Coleman or the Zsh Development Group be liable
# to any party for direct, indirect, special, incidental, or consequential
# damages arising out of the use of this software and its documentation,
# even if Richard Coleman and the Zsh Development Group have been advised of
# the possibility of such damage.
#
# Richard Coleman and the Zsh Development Group specifically disclaim any
# warranties, including, but not limited to, the implied warranties of
# merchantability and fitness for a particular purpose.  The software
# provided hereunder is on an "as is" basis, and Richard Coleman and the
# Zsh Development Group have no obligation to provide maintenance,
# support, updates, enhancements, or modifications.
#

mostlyclean: mostlyclean-recursive mostlyclean-here
clean:       clean-recursive       clean-here
distclean:   distclean-recursive   distclean-here
realclean:   realclean-recursive   realclean-here

mostlyclean-here:
clean-here: mostlyclean-here
distclean-here: clean-here
realclean-here: distclean-here

mostlyclean-recursive clean-recursive distclean-recursive realclean-recursive:
	@subdirs='$(SUBDIRS)'; if test -n "$$subdirs"; then \
	  target=`echo $@ | sed s/-recursive//`; \
	  for subdir in $$subdirs; do \
	    (cd $$subdir && $(MAKE) $(MAKEDEFS) $$target) || exit 1; \
	  done; \
	fi

distclean-here:
	rm -f Makefile config.h config.status config.log config.cache config.modules stamp-h Config/defs.mk

realclean-here:
	cd $(sdir) && rm -f config.h.in stamp-h.in configure

# ========== DEPENDENCIES FOR MAINTENANCE ==========


#
# Makefile fragment for building Makefiles
#
# Copyright (c) 1995-1997 Richard Coleman
# All rights reserved.
#
# Permission is hereby granted, without written agreement and without
# license or royalty fees, to use, copy, modify, and distribute this
# software and to distribute modified versions of this software for any
# purpose, provided that the above copyright notice and the following
# two paragraphs appear in all copies of this software.
#
# In no event shall Richard Coleman or the Zsh Development Group be liable
# to any party for direct, indirect, special, incidental, or consequential
# damages arising out of the use of this software and its documentation,
# even if Richard Coleman and the Zsh Development Group have been advised of
# the possibility of such damage.
#
# Richard Coleman and the Zsh Development Group specifically disclaim any
# warranties, including, but not limited to, the implied warranties of
# merchantability and fitness for a particular purpose.  The software
# provided hereunder is on an "as is" basis, and Richard Coleman and the
# Zsh Development Group have no obligation to provide maintenance,
# support, updates, enhancements, or modifications.
#

config: Makefile
	@subdirs='$(SUBDIRS)'; for subdir in $$subdirs; do \
	  (cd $$subdir && $(MAKE) $(MAKEDEFS) $@) || exit 1; \
	done

CONFIG_INCS = \
$(dir_top)/Config/clean.mk $(dir_top)/Config/config.mk \
$(dir_top)/Config/defs.mk $(dir_top)/Config/version.mk

Makefile: Makefile.in $(dir_top)/config.status $(CONFIG_INCS)
	cd $(dir_top) && \
	  CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status

$(dir_top)/Config/defs.mk: $(sdir_top)/Config/defs.mk.in $(dir_top)/config.status
	cd $(dir_top) && \
	  CONFIG_FILES=Config/defs.mk CONFIG_HEADERS= $(SHELL) ./config.status

config: config.h

config.status: $(sdir)/configure
	$(SHELL) ./config.status --recheck

$(sdir)/configure: $(sdir)/zshconfig.ac $(sdir)/aclocal.m4 $(sdir)/aczsh.m4 $(sdir)/configure.in $(sdir)/configure.ac
	cd $(sdir) && autoconf

config.h: stamp-h
stamp-h: $(sdir)/config.h.in config.status
	cd $(dir_top) && \
	  CONFIG_FILES= CONFIG_HEADERS=$(subdir)/config.h $(SHELL) ./config.status

$(sdir)/config.h.in: $(sdir)/stamp-h.in
$(sdir)/stamp-h.in: $(sdir)/zshconfig.ac $(sdir)/acconfig.h \
		$(sdir)/aclocal.m4 $(sdir)/aczsh.m4
	cd $(sdir) && autoheader
	echo > $(sdir)/stamp-h.in

# ========== DEPENDENCIES FOR DISTRIBUTION ==========

DISTNAME = zsh-$(VERSION)

targz-src: $(DISTNAME).tar.gz
$(DISTNAME).tar.gz: FORCE
	@$(sdir_top)/Util/mkdisttree.sh $(DISTNAME) $(sdir_top) $(dir_top) SRC \
	    $(MAKE) $(MAKEDEFS)
	tar cf - $(DISTNAME) | gzip -9 > $@
	rm -rf $(DISTNAME)

targz-doc: $(DISTNAME)-doc.tar.gz
$(DISTNAME)-doc.tar.gz: FORCE
	@$(sdir_top)/Util/mkdisttree.sh $(DISTNAME) $(sdir_top) $(dir_top) DOC \
	    $(MAKE) $(MAKEDEFS)
	tar cf - $(DISTNAME) | gzip -9 > $@
	rm -rf $(DISTNAME)

FORCE:
