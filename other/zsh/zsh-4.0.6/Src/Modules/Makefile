# Generated automatically from Makefile.in by configure.
##### Makefile.in generated automatically by mkmakemod.sh
##### DO NOT EDIT!

##### ===== DEFINITIONS ===== #####

makefile = Makefile
dir_top = ../..
subdir = Src/Modules

MODOBJS = modobjs.parameter modobjs.termcap modobjs.terminfo modobjs.zutil
MODULES =
MDDS    = parameter.mdd termcap.mdd terminfo.mdd zutil.mdd
MDHS    = parameter.mdh termcap.mdh terminfo.mdh zutil.mdh
PROTOS  = proto.parameter proto.termcap proto.terminfo proto.zutil
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


##### ===== DEPENDENCIES GENERATED FROM parameter.mdd ===== #####

MODOBJS_parameter = parameter.o
MODDOBJS_parameter = parameter..o  $(NNTRYOBJ)
SYMS_parameter = parameter.syms 
EPRO_parameter = parameter.epro
INCS_parameter = $(EPRO_parameter) 
EXPIMP_parameter =  $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)parameter.export
NXPIMP_parameter =
LINKMODS_parameter =  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_parameter = 

proto.parameter: $(EPRO_parameter)
$(SYMS_parameter): $(PROTODEPS)

parameter.export: $(SYMS_parameter)
	( echo '#!'; cat $(SYMS_parameter) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.parameter: $(MODOBJS_parameter)
	echo '' $(MODOBJS_parameter) | sed 's" " Modules/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.parameter: parameter.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) parameter.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/parameter.$(DL_EXT)

uninstall.modules.parameter:
	rm -f $(DESTDIR)$(MODDIR)/zsh/parameter.$(DL_EXT)

parameter.$(DL_EXT): $(MODDOBJS_parameter) parameter.export  $(dir_top)/Src/zsh.export $(NOLINKMODS_parameter)
	rm -f $@
	$(DLLINK) $(NXPIMP_parameter) $(NNTRYOPT) $(MODDOBJS_parameter) $(NOLINKMODS_parameter) $(LIBS) 

parameter.mdhi: parameter.mdhs $(INCS_parameter)
	@test -f $@ || echo 'do not delete this file' > $@

parameter.mdhs: parameter.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) parameter.mdh.tmp
	@if cmp -s parameter.mdh parameter.mdh.tmp; then \
	    rm -f parameter.mdh.tmp; \
	    echo "\`parameter.mdh' is up to date."; \
	else \
	    mv -f parameter.mdh.tmp parameter.mdh; \
	    echo "Updated \`parameter.mdh'."; \
	fi
	echo 'timestamp for parameter.mdh against parameter.mdd' > $@

parameter.mdh:  $(dir_top)/Src/zsh.mdh   parameter.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) parameter.mdh.tmp
	@mv -f parameter.mdh.tmp parameter.mdh
	@echo "Updated \`parameter.mdh'."

parameter.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQsparameter_module'; \
	    echo '#define have_zshQsparameter_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQsparameter'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQsparameter'; \
	    echo '#   define cleanup_ cleanup_zshQsparameter'; \
	    echo '#   define setup_ setup_zshQsparameter'; \
	    echo '#   define finish_ finish_zshQsparameter'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQsparameter */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQsparameter) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQsparameter && MODULE */'; \
	    for epro in $(EPRO_parameter); do \
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
	    echo '#endif /* !have_zshQsparameter_module */'; \
	) > $@

$(MODOBJS_parameter) $(MODDOBJS_parameter): parameter.mdh

##### ===== DEPENDENCIES GENERATED FROM termcap.mdd ===== #####

MODOBJS_termcap = termcap.o
MODDOBJS_termcap = termcap..o  $(NNTRYOBJ)
SYMS_termcap = termcap.syms 
EPRO_termcap = termcap.epro
INCS_termcap = $(EPRO_termcap) 
EXPIMP_termcap =  $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)termcap.export
NXPIMP_termcap =
LINKMODS_termcap =  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_termcap = 

proto.termcap: $(EPRO_termcap)
$(SYMS_termcap): $(PROTODEPS)

termcap.export: $(SYMS_termcap)
	( echo '#!'; cat $(SYMS_termcap) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.termcap: $(MODOBJS_termcap)
	echo '' $(MODOBJS_termcap) | sed 's" " Modules/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.termcap: termcap.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) termcap.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/termcap.$(DL_EXT)

uninstall.modules.termcap:
	rm -f $(DESTDIR)$(MODDIR)/zsh/termcap.$(DL_EXT)

termcap.$(DL_EXT): $(MODDOBJS_termcap) termcap.export  $(dir_top)/Src/zsh.export $(NOLINKMODS_termcap)
	rm -f $@
	$(DLLINK) $(NXPIMP_termcap) $(NNTRYOPT) $(MODDOBJS_termcap) $(NOLINKMODS_termcap) $(LIBS) 

termcap.mdhi: termcap.mdhs $(INCS_termcap)
	@test -f $@ || echo 'do not delete this file' > $@

termcap.mdhs: termcap.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) termcap.mdh.tmp
	@if cmp -s termcap.mdh termcap.mdh.tmp; then \
	    rm -f termcap.mdh.tmp; \
	    echo "\`termcap.mdh' is up to date."; \
	else \
	    mv -f termcap.mdh.tmp termcap.mdh; \
	    echo "Updated \`termcap.mdh'."; \
	fi
	echo 'timestamp for termcap.mdh against termcap.mdd' > $@

termcap.mdh:  $(dir_top)/Src/zsh.mdh   termcap.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) termcap.mdh.tmp
	@mv -f termcap.mdh.tmp termcap.mdh
	@echo "Updated \`termcap.mdh'."

termcap.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQstermcap_module'; \
	    echo '#define have_zshQstermcap_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQstermcap'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQstermcap'; \
	    echo '#   define cleanup_ cleanup_zshQstermcap'; \
	    echo '#   define setup_ setup_zshQstermcap'; \
	    echo '#   define finish_ finish_zshQstermcap'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQstermcap */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQstermcap) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQstermcap && MODULE */'; \
	    for epro in $(EPRO_termcap); do \
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
	    echo '#endif /* !have_zshQstermcap_module */'; \
	) > $@

$(MODOBJS_termcap) $(MODDOBJS_termcap): termcap.mdh

##### ===== DEPENDENCIES GENERATED FROM terminfo.mdd ===== #####

MODOBJS_terminfo = terminfo.o
MODDOBJS_terminfo = terminfo..o  $(NNTRYOBJ)
SYMS_terminfo = terminfo.syms 
EPRO_terminfo = terminfo.epro
INCS_terminfo = $(EPRO_terminfo) 
EXPIMP_terminfo =  $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)terminfo.export
NXPIMP_terminfo =
LINKMODS_terminfo =  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_terminfo = 

proto.terminfo: $(EPRO_terminfo)
$(SYMS_terminfo): $(PROTODEPS)

terminfo.export: $(SYMS_terminfo)
	( echo '#!'; cat $(SYMS_terminfo) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.terminfo: $(MODOBJS_terminfo)
	echo '' $(MODOBJS_terminfo) | sed 's" " Modules/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.terminfo: terminfo.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) terminfo.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/terminfo.$(DL_EXT)

uninstall.modules.terminfo:
	rm -f $(DESTDIR)$(MODDIR)/zsh/terminfo.$(DL_EXT)

terminfo.$(DL_EXT): $(MODDOBJS_terminfo) terminfo.export  $(dir_top)/Src/zsh.export $(NOLINKMODS_terminfo)
	rm -f $@
	$(DLLINK) $(NXPIMP_terminfo) $(NNTRYOPT) $(MODDOBJS_terminfo) $(NOLINKMODS_terminfo) $(LIBS) 

terminfo.mdhi: terminfo.mdhs $(INCS_terminfo)
	@test -f $@ || echo 'do not delete this file' > $@

terminfo.mdhs: terminfo.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) terminfo.mdh.tmp
	@if cmp -s terminfo.mdh terminfo.mdh.tmp; then \
	    rm -f terminfo.mdh.tmp; \
	    echo "\`terminfo.mdh' is up to date."; \
	else \
	    mv -f terminfo.mdh.tmp terminfo.mdh; \
	    echo "Updated \`terminfo.mdh'."; \
	fi
	echo 'timestamp for terminfo.mdh against terminfo.mdd' > $@

terminfo.mdh:  $(dir_top)/Src/zsh.mdh   terminfo.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) terminfo.mdh.tmp
	@mv -f terminfo.mdh.tmp terminfo.mdh
	@echo "Updated \`terminfo.mdh'."

terminfo.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQsterminfo_module'; \
	    echo '#define have_zshQsterminfo_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQsterminfo'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQsterminfo'; \
	    echo '#   define cleanup_ cleanup_zshQsterminfo'; \
	    echo '#   define setup_ setup_zshQsterminfo'; \
	    echo '#   define finish_ finish_zshQsterminfo'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQsterminfo */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQsterminfo) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQsterminfo && MODULE */'; \
	    for epro in $(EPRO_terminfo); do \
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
	    echo '#endif /* !have_zshQsterminfo_module */'; \
	) > $@

$(MODOBJS_terminfo) $(MODDOBJS_terminfo): terminfo.mdh

##### ===== DEPENDENCIES GENERATED FROM zutil.mdd ===== #####

MODOBJS_zutil = zutil.o
MODDOBJS_zutil = zutil..o  $(NNTRYOBJ)
SYMS_zutil = zutil.syms 
EPRO_zutil = zutil.epro
INCS_zutil = $(EPRO_zutil) 
EXPIMP_zutil =  $(IMPOPT)$(dir_top)/Src/Zle/complete.export $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)zutil.export
NXPIMP_zutil =
LINKMODS_zutil =  $(dir_top)/Src/Zle/complete.$(DL_EXT)  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_zutil = 

proto.zutil: $(EPRO_zutil)
$(SYMS_zutil): $(PROTODEPS)

zutil.export: $(SYMS_zutil)
	( echo '#!'; cat $(SYMS_zutil) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.zutil: $(MODOBJS_zutil)
	echo '' $(MODOBJS_zutil) | sed 's" " Modules/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.zutil: zutil.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) zutil.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/zutil.$(DL_EXT)

uninstall.modules.zutil:
	rm -f $(DESTDIR)$(MODDIR)/zsh/zutil.$(DL_EXT)

zutil.$(DL_EXT): $(MODDOBJS_zutil) zutil.export  $(dir_top)/Src/Zle/complete.export $(dir_top)/Src/zsh.export $(NOLINKMODS_zutil)
	rm -f $@
	$(DLLINK) $(NXPIMP_zutil) $(NNTRYOPT) $(MODDOBJS_zutil) $(NOLINKMODS_zutil) $(LIBS) 

zutil.mdhi: zutil.mdhs $(INCS_zutil)
	@test -f $@ || echo 'do not delete this file' > $@

zutil.mdhs: zutil.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) zutil.mdh.tmp
	@if cmp -s zutil.mdh zutil.mdh.tmp; then \
	    rm -f zutil.mdh.tmp; \
	    echo "\`zutil.mdh' is up to date."; \
	else \
	    mv -f zutil.mdh.tmp zutil.mdh; \
	    echo "Updated \`zutil.mdh'."; \
	fi
	echo 'timestamp for zutil.mdh against zutil.mdd' > $@

zutil.mdh:  $(dir_top)/Src/Zle/complete.mdh $(dir_top)/Src/zsh.mdh   zutil.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) zutil.mdh.tmp
	@mv -f zutil.mdh.tmp zutil.mdh
	@echo "Updated \`zutil.mdh'."

zutil.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQszutil_module'; \
	    echo '#define have_zshQszutil_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQszutil'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQszutil'; \
	    echo '#   define cleanup_ cleanup_zshQszutil'; \
	    echo '#   define setup_ setup_zshQszutil'; \
	    echo '#   define finish_ finish_zshQszutil'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQszutil */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQscomplete 1'; \
	    echo '# include "$(dir_top)/Src/Zle/complete.mdh"'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQszutil) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQszutil && MODULE */'; \
	    for epro in $(EPRO_zutil); do \
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
	    echo '#endif /* !have_zshQszutil_module */'; \
	) > $@

$(MODOBJS_zutil) $(MODDOBJS_zutil): zutil.mdh

##### ===== DEPENDENCIES FOR REMOTE MODULES ===== #####

$(dir_top)/Src/Zle/complete.mdh: FORCE
	@cd $(dir_top)/Src/Zle && $(MAKE) $(MAKEDEFS) complete.mdh

$(dir_top)/Src/zsh.mdh:
	false # should only happen with make -n

$(dir_top)/Src/Zle/complete.export: FORCE
	@cd $(dir_top)/Src/Zle && $(MAKE) $(MAKEDEFS) complete.export

$(dir_top)/Src/zsh.export:
	false # should only happen with make -n

$(dir_top)/Src/Zle/complete.$(DL_EXT): FORCE
	@cd $(dir_top)/Src/Zle && $(MAKE) $(MAKEDEFS) complete.$(DL_EXT)

$(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) :
	false # should only happen with make -n

##### End of Makefile.in
