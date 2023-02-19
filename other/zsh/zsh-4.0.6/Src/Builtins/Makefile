# Generated automatically from Makefile.in by configure.
##### Makefile.in generated automatically by mkmakemod.sh
##### DO NOT EDIT!

##### ===== DEFINITIONS ===== #####

makefile = Makefile
dir_top = ../..
subdir = Src/Builtins

MODOBJS = modobjs.rlimits modobjs.sched
MODULES =
MDDS    = rlimits.mdd sched.mdd
MDHS    = rlimits.mdh sched.mdh
PROTOS  = proto.rlimits proto.sched
SUBDIRS =

ENTRYOBJ = $(dir_src)/modentry..o
NNTRYOBJ =
ENTRYOPT = -emodentry
NNTRYOPT =

##### ===== INCLUDING Makemod.in.in ===== #####

#
# Makemod.in.in
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

# ========== OVERRIDABLE VARIABLES ==========

# subdir is done by mkmakemod.sh
# dir_top is done by mkmakemod.sh
# SUBDIRS is done by mkmakemod.sh


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
sdir_top        = ../..
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
CFLAGS          =  -Wall -Wmissing-prototypes -O2
LDFLAGS         = 
EXTRA_LDFLAGS   = 
DLCFLAGS        = 
DLLDFLAGS       = 
LIBLDFLAGS      =  -s
EXELDFLAGS      =  -s
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

sdir_src      = $(sdir_top)/Src
dir_src       = $(dir_top)/Src

# ========== COMPILATION RULES ==========

DNCFLAGS =

COMPILE     = $(CC) -c -I. $(CPPFLAGS) $(DEFS) $(CFLAGS) $(DNCFLAGS)
DLCOMPILE   = $(CC) -c -I. $(CPPFLAGS) $(DEFS) -DMODULE $(CFLAGS) $(DLCFLAGS)
LINK        = $(CC) $(LDFLAGS) $(EXELDFLAGS) $(EXTRA_LDFLAGS) -o $@
DLLINK      = $(DLLD) $(LDFLAGS) $(LIBLDFLAGS) $(DLLDFLAGS) -o $@

KNR_OBJ=.o
KNROBJ=._foo_

ANSIOBJ=.o
ANSI_OBJ=._foo_

.SUFFIXES: .c .$(DL_EXT) ..o .._foo_ .o ._foo_ .syms .pro .epro

.c$(ANSIOBJ):
	$(COMPILE) -o $@ $<
	@rm -f $(dir_src)/stamp-modobjs

.c$(KNROBJ):
	: ansi2knr $< > $@.c
	$(COMPILE) -o $@ $@.c
	rm -f $@.c
	@rm -f $(dir_src)/stamp-modobjs

.c.$(ANSIOBJ):
	$(DLCOMPILE) -o $@ $<

.c.$(KNROBJ):
	: ansi2knr $< > $@.c
	$(DLCOMPILE) -o $@ $@.c
	rm -f $@.c

.c.syms:
	$(AWK) -f $(sdir_src)/makepro.awk $< $(subdir) > $@

.syms.epro:
	(echo '/* Generated automatically */'; sed -n '/^E/{s/^E//;p;}' < $<) \
		> $@
	(echo '/* Generated automatically */'; sed -n '/^L/{s/^L//;p;}' < $<) \
		> `echo $@ | sed 's/\.epro$$/.pro/'`

PROTODEPS = $(sdir_src)/makepro.awk

# ========== DEPENDENCIES FOR BUILDING ==========

all: modobjs modules
.PHONY: all

modobjs: $(MODOBJS)
modules: $(MODULES)
headers: $(MDHS)
proto: $(PROTOS)
.PHONY: modobjs modules headers proto

prep:
	@case $(sdir_top) in \
	    /*) top_srcdir=$(sdir_top) ;; \
	    *) top_srcdir=$(subdir)/$(sdir_top) ;; \
	esac; \
	export top_srcdir; \
	cd $(dir_top) || exit 1; \
	subdirs='$(SUBDIRS)'; \
	for subdir in $$subdirs; do \
	    dir=$(subdir)/$$subdir; \
	    test -d $$dir || mkdir $$dir; \
	    $(SHELL) $$top_srcdir/Src/mkmakemod.sh $$dir Makefile || exit 1; \
	    ( cd $$dir && $(MAKE) $(MAKEDEFS) $@ ) || exit 1; \
	done
.PHONY: prep

headers prep: $(dir_src)/modules.stamp
$(dir_src)/modules.stamp: $(MDDS)
	echo 'timestamp for *.mdd files' > $@
.PHONY: headers

FORCE:
.PHONY: FORCE

# ========== DEPENDENCIES FOR INSTALLING ==========

install: install.bin install.modules
uninstall: uninstall.bin uninstall.modules
.PHONY: install uninstall

install.bin: install.bin-here
uninstall.bin: uninstall.bin-here
install.modules: install.modules-here
uninstall.modules: uninstall.modules-here
.PHONY: install.bin uninstall.bin install.modules uninstall.modules

install.bin-here uninstall.bin-here:
install.modules-here uninstall.modules-here:
.PHONY: install.bin-here install.modules-here

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

mostlyclean-here:
	rm -f *.o *.export *.$(DL_EXT)
.PHONY: mostlyclean-here

clean-here:
	rm -f *.o.c *.syms *.pro *.epro *.mdh *.mdhi *.mdhs *.mdh.tmp
.PHONY: clean-here

distclean-here:
	rm -f $(makefile) $(makefile).in
.PHONY: distclean-here

# ========== RECURSIVE MAKES ==========

install.bin uninstall.bin install.modules uninstall.modules \
modobjs modules headers proto:
	@subdirs='$(SUBDIRS)'; for subdir in $$subdirs; do \
	  ( cd $$subdir && $(MAKE) $(MAKEDEFS) $@ ) || exit 1; \
	done

# ========== DEPENDENCIES FOR MAINTENANCE ==========

$(makefile): $(makefile).in $(dir_top)/config.status
	@case $(sdir_top) in \
	    /*) top_srcdir=$(sdir_top) ;; \
	    *) top_srcdir=$(subdir)/$(sdir_top) ;; \
	esac; \
	export top_srcdir; \
	echo 'cd $(dir_top) && $(SHELL)' \
	    '$$top_srcdir/Src/mkmakemod.sh -m $(subdir) $(makefile)'; \
	cd $(dir_top) && \
	    $(SHELL) $$top_srcdir/Src/mkmakemod.sh -m $(subdir) $(makefile)

$(makefile).in: $(sdir_src)/mkmakemod.sh $(sdir_src)/Makemod.in.in $(MDDS) \
	$(dir_top)/config.modules
	@case $(sdir_top) in \
	    /*) top_srcdir=$(sdir_top) ;; \
	    *) top_srcdir=$(subdir)/$(sdir_top) ;; \
	esac; \
	export top_srcdir; \
	echo 'cd $(dir_top) && $(SHELL)' \
	    '$$top_srcdir/Src/mkmakemod.sh -i $(subdir) $(makefile)'; \
	cd $(dir_top) && \
	    $(SHELL) $$top_srcdir/Src/mkmakemod.sh -i $(subdir) $(makefile)


##### ===== DEPENDENCIES GENERATED FROM rlimits.mdd ===== #####

MODOBJS_rlimits = rlimits.o
MODDOBJS_rlimits = rlimits..o  $(NNTRYOBJ)
SYMS_rlimits = rlimits.syms 
EPRO_rlimits = rlimits.epro
INCS_rlimits = $(EPRO_rlimits) 
EXPIMP_rlimits =  $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)rlimits.export
NXPIMP_rlimits =
LINKMODS_rlimits =  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_rlimits = 

proto.rlimits: $(EPRO_rlimits)
$(SYMS_rlimits): $(PROTODEPS)

rlimits.export: $(SYMS_rlimits)
	( echo '#!'; cat $(SYMS_rlimits) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.rlimits: $(MODOBJS_rlimits)
	echo '' $(MODOBJS_rlimits) | sed 's" " Builtins/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.rlimits: rlimits.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) rlimits.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/rlimits.$(DL_EXT)

uninstall.modules.rlimits:
	rm -f $(DESTDIR)$(MODDIR)/zsh/rlimits.$(DL_EXT)

rlimits.$(DL_EXT): $(MODDOBJS_rlimits) rlimits.export  $(dir_top)/Src/zsh.export $(NOLINKMODS_rlimits)
	rm -f $@
	$(DLLINK) $(NXPIMP_rlimits) $(NNTRYOPT) $(MODDOBJS_rlimits) $(NOLINKMODS_rlimits) $(LIBS) 

rlimits.mdhi: rlimits.mdhs $(INCS_rlimits)
	@test -f $@ || echo 'do not delete this file' > $@

rlimits.mdhs: rlimits.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) rlimits.mdh.tmp
	@if cmp -s rlimits.mdh rlimits.mdh.tmp; then \
	    rm -f rlimits.mdh.tmp; \
	    echo "\`rlimits.mdh' is up to date."; \
	else \
	    mv -f rlimits.mdh.tmp rlimits.mdh; \
	    echo "Updated \`rlimits.mdh'."; \
	fi
	echo 'timestamp for rlimits.mdh against rlimits.mdd' > $@

rlimits.mdh:  $(dir_top)/Src/zsh.mdh   rlimits.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) rlimits.mdh.tmp
	@mv -f rlimits.mdh.tmp rlimits.mdh
	@echo "Updated \`rlimits.mdh'."

rlimits.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQsrlimits_module'; \
	    echo '#define have_zshQsrlimits_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQsrlimits'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQsrlimits'; \
	    echo '#   define cleanup_ cleanup_zshQsrlimits'; \
	    echo '#   define setup_ setup_zshQsrlimits'; \
	    echo '#   define finish_ finish_zshQsrlimits'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQsrlimits */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQsrlimits) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQsrlimits && MODULE */'; \
	    for epro in $(EPRO_rlimits); do \
		echo '# include "'$$epro'"'; \
	    done; \
	    echo '# undef mod_import_variable'; \
	    echo '# define mod_import_variable'; \
	    echo '# undef mod_import_variable'; \
	    echo '# define mod_import_variable'; \
	    echo '# ifndef mod_export'; \
	    echo '#  define mod_export '; \
	    echo '# endif /* mod_export */'; \
	    echo; \
	    echo '#endif /* !have_zshQsrlimits_module */'; \
	) > $@

$(MODOBJS_rlimits) $(MODDOBJS_rlimits): rlimits.mdh

rlimits.o rlimits..o: rlimits.h

# this file will not be made if limits are unavailable
rlimits.h: rlimits.awk 
	$(AWK) -f $(sdir)/rlimits.awk  /dev/null > rlimits.h
	@if grep ZLIMTYPE_UNKNOWN rlimits.h >/dev/null; then \
	    echo >&2 WARNING: unknown limits: mail rlimits.h to developers; \
	else :; fi

clean-here: clean.rlimits
clean.rlimits:
	rm -f rlimits.h

##### ===== DEPENDENCIES GENERATED FROM sched.mdd ===== #####

MODOBJS_sched = sched.o
MODDOBJS_sched = sched..o  $(NNTRYOBJ)
SYMS_sched = sched.syms 
EPRO_sched = sched.epro
INCS_sched = $(EPRO_sched) 
EXPIMP_sched =  $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)sched.export
NXPIMP_sched =
LINKMODS_sched =  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_sched = 

proto.sched: $(EPRO_sched)
$(SYMS_sched): $(PROTODEPS)

sched.export: $(SYMS_sched)
	( echo '#!'; cat $(SYMS_sched) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.sched: $(MODOBJS_sched)
	echo '' $(MODOBJS_sched) | sed 's" " Builtins/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.sched: sched.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) sched.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/sched.$(DL_EXT)

uninstall.modules.sched:
	rm -f $(DESTDIR)$(MODDIR)/zsh/sched.$(DL_EXT)

sched.$(DL_EXT): $(MODDOBJS_sched) sched.export  $(dir_top)/Src/zsh.export $(NOLINKMODS_sched)
	rm -f $@
	$(DLLINK) $(NXPIMP_sched) $(NNTRYOPT) $(MODDOBJS_sched) $(NOLINKMODS_sched) $(LIBS) 

sched.mdhi: sched.mdhs $(INCS_sched)
	@test -f $@ || echo 'do not delete this file' > $@

sched.mdhs: sched.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) sched.mdh.tmp
	@if cmp -s sched.mdh sched.mdh.tmp; then \
	    rm -f sched.mdh.tmp; \
	    echo "\`sched.mdh' is up to date."; \
	else \
	    mv -f sched.mdh.tmp sched.mdh; \
	    echo "Updated \`sched.mdh'."; \
	fi
	echo 'timestamp for sched.mdh against sched.mdd' > $@

sched.mdh:  $(dir_top)/Src/zsh.mdh   sched.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) sched.mdh.tmp
	@mv -f sched.mdh.tmp sched.mdh
	@echo "Updated \`sched.mdh'."

sched.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQssched_module'; \
	    echo '#define have_zshQssched_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQssched'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQssched'; \
	    echo '#   define cleanup_ cleanup_zshQssched'; \
	    echo '#   define setup_ setup_zshQssched'; \
	    echo '#   define finish_ finish_zshQssched'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQssched */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQssched) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQssched && MODULE */'; \
	    for epro in $(EPRO_sched); do \
		echo '# include "'$$epro'"'; \
	    done; \
	    echo '# undef mod_import_variable'; \
	    echo '# define mod_import_variable'; \
	    echo '# undef mod_import_variable'; \
	    echo '# define mod_import_variable'; \
	    echo '# ifndef mod_export'; \
	    echo '#  define mod_export '; \
	    echo '# endif /* mod_export */'; \
	    echo; \
	    echo '#endif /* !have_zshQssched_module */'; \
	) > $@

$(MODOBJS_sched) $(MODDOBJS_sched): sched.mdh

##### ===== DEPENDENCIES FOR REMOTE MODULES ===== #####

$(dir_top)/Src/zsh.mdh:
	false # should only happen with make -n

$(dir_top)/Src/zsh.export:
	false # should only happen with make -n

$(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) :
	false # should only happen with make -n

##### End of Makefile.in
