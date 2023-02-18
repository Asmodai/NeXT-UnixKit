# Generated automatically from Makefile.in by configure.
##### Makefile.in generated automatically by mkmakemod.sh
##### DO NOT EDIT!

##### ===== DEFINITIONS ===== #####

makefile = Makefile
dir_top = ../..
subdir = Src/Zle

MODOBJS = modobjs.compctl modobjs.complete modobjs.complist modobjs.computil modobjs.zle modobjs.zleparameter
MODULES =
MDDS    = compctl.mdd complete.mdd complist.mdd computil.mdd zle.mdd zleparameter.mdd
MDHS    = compctl.mdh complete.mdh complist.mdh computil.mdh zle.mdh zleparameter.mdh
PROTOS  = proto.compctl proto.complete proto.complist proto.computil proto.zle proto.zleparameter
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


##### ===== DEPENDENCIES GENERATED FROM compctl.mdd ===== #####

MODOBJS_compctl = compctl.o
MODDOBJS_compctl = compctl..o  $(NNTRYOBJ)
SYMS_compctl = compctl.syms 
EPRO_compctl = compctl.epro
INCS_compctl = $(EPRO_compctl) 
EXPIMP_compctl =  $(IMPOPT)complete.export $(IMPOPT)zle.export $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)compctl.export
NXPIMP_compctl =
LINKMODS_compctl =  complete.$(DL_EXT)  zle.$(DL_EXT)  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_compctl = 

proto.compctl: $(EPRO_compctl)
$(SYMS_compctl): $(PROTODEPS)

compctl.export: $(SYMS_compctl)
	( echo '#!'; cat $(SYMS_compctl) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.compctl: $(MODOBJS_compctl)
	echo '' $(MODOBJS_compctl) | sed 's" " Zle/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.compctl: compctl.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) compctl.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/compctl.$(DL_EXT)

uninstall.modules.compctl:
	rm -f $(DESTDIR)$(MODDIR)/zsh/compctl.$(DL_EXT)

compctl.$(DL_EXT): $(MODDOBJS_compctl) compctl.export  complete.export zle.export $(dir_top)/Src/zsh.export $(NOLINKMODS_compctl)
	rm -f $@
	$(DLLINK) $(NXPIMP_compctl) $(NNTRYOPT) $(MODDOBJS_compctl) $(NOLINKMODS_compctl) $(LIBS) 

compctl.mdhi: compctl.mdhs $(INCS_compctl)
	@test -f $@ || echo 'do not delete this file' > $@

compctl.mdhs: compctl.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) compctl.mdh.tmp
	@if cmp -s compctl.mdh compctl.mdh.tmp; then \
	    rm -f compctl.mdh.tmp; \
	    echo "\`compctl.mdh' is up to date."; \
	else \
	    mv -f compctl.mdh.tmp compctl.mdh; \
	    echo "Updated \`compctl.mdh'."; \
	fi
	echo 'timestamp for compctl.mdh against compctl.mdd' > $@

compctl.mdh:  complete.mdh zle.mdh $(dir_top)/Src/zsh.mdh compctl.h  compctl.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) compctl.mdh.tmp
	@mv -f compctl.mdh.tmp compctl.mdh
	@echo "Updated \`compctl.mdh'."

compctl.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQscompctl_module'; \
	    echo '#define have_zshQscompctl_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQscompctl'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQscompctl'; \
	    echo '#   define cleanup_ cleanup_zshQscompctl'; \
	    echo '#   define setup_ setup_zshQscompctl'; \
	    echo '#   define finish_ finish_zshQscompctl'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQscompctl */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQscomplete 1'; \
	    echo '# include "complete.mdh"'; \
	    echo '# define IMPORTING_MODULE_zshQszle 1'; \
	    echo '# include "zle.mdh"'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '/* Extra headers for this module */'; \
	    for hdr in compctl.h; do \
		if test -f $$hdr; then \
		    echo '# include "'$$hdr'"'; \
		else \
		    echo '# include "$(sdir)/'$$hdr'"'; \
		fi; \
	    done; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQscompctl) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQscompctl && MODULE */'; \
	    for epro in $(EPRO_compctl); do \
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
	    echo '#endif /* !have_zshQscompctl_module */'; \
	) > $@

$(MODOBJS_compctl) $(MODDOBJS_compctl): compctl.mdh

##### ===== DEPENDENCIES GENERATED FROM complete.mdd ===== #####

MODOBJS_complete = complete.o compcore.o compmatch.o compresult.o
MODDOBJS_complete = complete..o compcore..o compmatch..o compresult..o  $(NNTRYOBJ)
SYMS_complete = complete.syms compcore.syms compmatch.syms compresult.syms 
EPRO_complete = complete.epro compcore.epro compmatch.epro compresult.epro
INCS_complete = $(EPRO_complete) 
EXPIMP_complete =  $(IMPOPT)zle.export $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)complete.export
NXPIMP_complete =
LINKMODS_complete =  zle.$(DL_EXT)  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_complete = 

proto.complete: $(EPRO_complete)
$(SYMS_complete): $(PROTODEPS)

complete.export: $(SYMS_complete)
	( echo '#!'; cat $(SYMS_complete) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.complete: $(MODOBJS_complete)
	echo '' $(MODOBJS_complete) | sed 's" " Zle/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.complete: complete.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) complete.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/complete.$(DL_EXT)

uninstall.modules.complete:
	rm -f $(DESTDIR)$(MODDIR)/zsh/complete.$(DL_EXT)

complete.$(DL_EXT): $(MODDOBJS_complete) complete.export  zle.export $(dir_top)/Src/zsh.export $(NOLINKMODS_complete)
	rm -f $@
	$(DLLINK) $(NXPIMP_complete) $(NNTRYOPT) $(MODDOBJS_complete) $(NOLINKMODS_complete) $(LIBS) 

complete.mdhi: complete.mdhs $(INCS_complete)
	@test -f $@ || echo 'do not delete this file' > $@

complete.mdhs: complete.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) complete.mdh.tmp
	@if cmp -s complete.mdh complete.mdh.tmp; then \
	    rm -f complete.mdh.tmp; \
	    echo "\`complete.mdh' is up to date."; \
	else \
	    mv -f complete.mdh.tmp complete.mdh; \
	    echo "Updated \`complete.mdh'."; \
	fi
	echo 'timestamp for complete.mdh against complete.mdd' > $@

complete.mdh:  zle.mdh $(dir_top)/Src/zsh.mdh comp.h  complete.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) complete.mdh.tmp
	@mv -f complete.mdh.tmp complete.mdh
	@echo "Updated \`complete.mdh'."

complete.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQscomplete_module'; \
	    echo '#define have_zshQscomplete_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQscomplete'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQscomplete'; \
	    echo '#   define cleanup_ cleanup_zshQscomplete'; \
	    echo '#   define setup_ setup_zshQscomplete'; \
	    echo '#   define finish_ finish_zshQscomplete'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQscomplete */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQszle 1'; \
	    echo '# include "zle.mdh"'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '/* Extra headers for this module */'; \
	    for hdr in comp.h; do \
		if test -f $$hdr; then \
		    echo '# include "'$$hdr'"'; \
		else \
		    echo '# include "$(sdir)/'$$hdr'"'; \
		fi; \
	    done; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQscomplete) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQscomplete && MODULE */'; \
	    for epro in $(EPRO_complete); do \
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
	    echo '#endif /* !have_zshQscomplete_module */'; \
	) > $@

$(MODOBJS_complete) $(MODDOBJS_complete): complete.mdh

##### ===== DEPENDENCIES GENERATED FROM complist.mdd ===== #####

MODOBJS_complist = complist.o
MODDOBJS_complist = complist..o  $(NNTRYOBJ)
SYMS_complist = complist.syms 
EPRO_complist = complist.epro
INCS_complist = $(EPRO_complist) 
EXPIMP_complist =  $(IMPOPT)complete.export $(IMPOPT)zle.export $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)complist.export
NXPIMP_complist =
LINKMODS_complist =  complete.$(DL_EXT)  zle.$(DL_EXT)  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_complist = 

proto.complist: $(EPRO_complist)
$(SYMS_complist): $(PROTODEPS)

complist.export: $(SYMS_complist)
	( echo '#!'; cat $(SYMS_complist) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.complist: $(MODOBJS_complist)
	echo '' $(MODOBJS_complist) | sed 's" " Zle/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.complist: complist.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) complist.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/complist.$(DL_EXT)

uninstall.modules.complist:
	rm -f $(DESTDIR)$(MODDIR)/zsh/complist.$(DL_EXT)

complist.$(DL_EXT): $(MODDOBJS_complist) complist.export  complete.export zle.export $(dir_top)/Src/zsh.export $(NOLINKMODS_complist)
	rm -f $@
	$(DLLINK) $(NXPIMP_complist) $(NNTRYOPT) $(MODDOBJS_complist) $(NOLINKMODS_complist) $(LIBS) 

complist.mdhi: complist.mdhs $(INCS_complist)
	@test -f $@ || echo 'do not delete this file' > $@

complist.mdhs: complist.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) complist.mdh.tmp
	@if cmp -s complist.mdh complist.mdh.tmp; then \
	    rm -f complist.mdh.tmp; \
	    echo "\`complist.mdh' is up to date."; \
	else \
	    mv -f complist.mdh.tmp complist.mdh; \
	    echo "Updated \`complist.mdh'."; \
	fi
	echo 'timestamp for complist.mdh against complist.mdd' > $@

complist.mdh:  complete.mdh zle.mdh $(dir_top)/Src/zsh.mdh   complist.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) complist.mdh.tmp
	@mv -f complist.mdh.tmp complist.mdh
	@echo "Updated \`complist.mdh'."

complist.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQscomplist_module'; \
	    echo '#define have_zshQscomplist_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQscomplist'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQscomplist'; \
	    echo '#   define cleanup_ cleanup_zshQscomplist'; \
	    echo '#   define setup_ setup_zshQscomplist'; \
	    echo '#   define finish_ finish_zshQscomplist'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQscomplist */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQscomplete 1'; \
	    echo '# include "complete.mdh"'; \
	    echo '# define IMPORTING_MODULE_zshQszle 1'; \
	    echo '# include "zle.mdh"'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQscomplist) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQscomplist && MODULE */'; \
	    for epro in $(EPRO_complist); do \
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
	    echo '#endif /* !have_zshQscomplist_module */'; \
	) > $@

$(MODOBJS_complist) $(MODDOBJS_complist): complist.mdh

##### ===== DEPENDENCIES GENERATED FROM computil.mdd ===== #####

MODOBJS_computil = computil.o
MODDOBJS_computil = computil..o  $(NNTRYOBJ)
SYMS_computil = computil.syms 
EPRO_computil = computil.epro
INCS_computil = $(EPRO_computil) 
EXPIMP_computil =  $(IMPOPT)complete.export $(IMPOPT)zle.export $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)computil.export
NXPIMP_computil =
LINKMODS_computil =  complete.$(DL_EXT)  zle.$(DL_EXT)  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_computil = 

proto.computil: $(EPRO_computil)
$(SYMS_computil): $(PROTODEPS)

computil.export: $(SYMS_computil)
	( echo '#!'; cat $(SYMS_computil) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.computil: $(MODOBJS_computil)
	echo '' $(MODOBJS_computil) | sed 's" " Zle/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.computil: computil.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) computil.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/computil.$(DL_EXT)

uninstall.modules.computil:
	rm -f $(DESTDIR)$(MODDIR)/zsh/computil.$(DL_EXT)

computil.$(DL_EXT): $(MODDOBJS_computil) computil.export  complete.export zle.export $(dir_top)/Src/zsh.export $(NOLINKMODS_computil)
	rm -f $@
	$(DLLINK) $(NXPIMP_computil) $(NNTRYOPT) $(MODDOBJS_computil) $(NOLINKMODS_computil) $(LIBS) 

computil.mdhi: computil.mdhs $(INCS_computil)
	@test -f $@ || echo 'do not delete this file' > $@

computil.mdhs: computil.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) computil.mdh.tmp
	@if cmp -s computil.mdh computil.mdh.tmp; then \
	    rm -f computil.mdh.tmp; \
	    echo "\`computil.mdh' is up to date."; \
	else \
	    mv -f computil.mdh.tmp computil.mdh; \
	    echo "Updated \`computil.mdh'."; \
	fi
	echo 'timestamp for computil.mdh against computil.mdd' > $@

computil.mdh:  complete.mdh zle.mdh $(dir_top)/Src/zsh.mdh   computil.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) computil.mdh.tmp
	@mv -f computil.mdh.tmp computil.mdh
	@echo "Updated \`computil.mdh'."

computil.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQscomputil_module'; \
	    echo '#define have_zshQscomputil_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQscomputil'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQscomputil'; \
	    echo '#   define cleanup_ cleanup_zshQscomputil'; \
	    echo '#   define setup_ setup_zshQscomputil'; \
	    echo '#   define finish_ finish_zshQscomputil'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQscomputil */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQscomplete 1'; \
	    echo '# include "complete.mdh"'; \
	    echo '# define IMPORTING_MODULE_zshQszle 1'; \
	    echo '# include "zle.mdh"'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQscomputil) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQscomputil && MODULE */'; \
	    for epro in $(EPRO_computil); do \
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
	    echo '#endif /* !have_zshQscomputil_module */'; \
	) > $@

$(MODOBJS_computil) $(MODDOBJS_computil): computil.mdh

##### ===== DEPENDENCIES GENERATED FROM zle.mdd ===== #####

MODOBJS_zle = zle_bindings.o zle_hist.o zle_keymap.o zle_main.o zle_misc.o zle_move.o zle_params.o zle_refresh.o zle_thingy.o zle_tricky.o zle_utils.o zle_vi.o zle_word.o
MODDOBJS_zle = zle_bindings..o zle_hist..o zle_keymap..o zle_main..o zle_misc..o zle_move..o zle_params..o zle_refresh..o zle_thingy..o zle_tricky..o zle_utils..o zle_vi..o zle_word..o  $(NNTRYOBJ)
SYMS_zle = zle_bindings.syms zle_hist.syms zle_keymap.syms zle_main.syms zle_misc.syms zle_move.syms zle_params.syms zle_refresh.syms zle_thingy.syms zle_tricky.syms zle_utils.syms zle_vi.syms zle_word.syms 
EPRO_zle = zle_bindings.epro zle_hist.epro zle_keymap.epro zle_main.epro zle_misc.epro zle_move.epro zle_params.epro zle_refresh.epro zle_thingy.epro zle_tricky.epro zle_utils.epro zle_vi.epro zle_word.epro
INCS_zle = $(EPRO_zle) 
EXPIMP_zle =  $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)zle.export
NXPIMP_zle =
LINKMODS_zle =  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_zle = 

proto.zle: $(EPRO_zle)
$(SYMS_zle): $(PROTODEPS)

zle.export: $(SYMS_zle)
	( echo '#!'; cat $(SYMS_zle) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.zle: $(MODOBJS_zle)
	echo '' $(MODOBJS_zle) | sed 's" " Zle/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.zle: zle.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) zle.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/zle.$(DL_EXT)

uninstall.modules.zle:
	rm -f $(DESTDIR)$(MODDIR)/zsh/zle.$(DL_EXT)

zle.$(DL_EXT): $(MODDOBJS_zle) zle.export  $(dir_top)/Src/zsh.export $(NOLINKMODS_zle)
	rm -f $@
	$(DLLINK) $(NXPIMP_zle) $(NNTRYOPT) $(MODDOBJS_zle) $(NOLINKMODS_zle) $(LIBS) 

zle.mdhi: zle.mdhs $(INCS_zle)
	@test -f $@ || echo 'do not delete this file' > $@

zle.mdhs: zle.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) zle.mdh.tmp
	@if cmp -s zle.mdh zle.mdh.tmp; then \
	    rm -f zle.mdh.tmp; \
	    echo "\`zle.mdh' is up to date."; \
	else \
	    mv -f zle.mdh.tmp zle.mdh; \
	    echo "Updated \`zle.mdh'."; \
	fi
	echo 'timestamp for zle.mdh against zle.mdd' > $@

zle.mdh:  $(dir_top)/Src/zsh.mdh zle.h zle_things.h  zle.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) zle.mdh.tmp
	@mv -f zle.mdh.tmp zle.mdh
	@echo "Updated \`zle.mdh'."

zle.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQszle_module'; \
	    echo '#define have_zshQszle_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQszle'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQszle'; \
	    echo '#   define cleanup_ cleanup_zshQszle'; \
	    echo '#   define setup_ setup_zshQszle'; \
	    echo '#   define finish_ finish_zshQszle'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQszle */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '/* Extra headers for this module */'; \
	    for hdr in zle.h zle_things.h; do \
		if test -f $$hdr; then \
		    echo '# include "'$$hdr'"'; \
		else \
		    echo '# include "$(sdir)/'$$hdr'"'; \
		fi; \
	    done; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQszle) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQszle && MODULE */'; \
	    for epro in $(EPRO_zle); do \
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
	    echo '#endif /* !have_zshQszle_module */'; \
	) > $@

$(MODOBJS_zle) $(MODDOBJS_zle): zle.mdh

zle_things.h: thingies.list zle_things.sed
	( \
	    echo '/** zle_things.h                              **/'; \
	    echo '/** indices of and pointers to known thingies **/'; \
	    echo; \
	    echo 'enum {'; \
	    sed -n -f $(sdir)/zle_things.sed < thingies.list; \
	    echo '    ZLE_BUILTIN_THINGY_COUNT'; \
	    echo '};'; \
	) > $@

zle_widget.h: widgets.list zle_widget.sed
	( \
	    echo '/** zle_widget.h                                **/'; \
	    echo '/** indices of and pointers to internal widgets **/'; \
	    echo; \
	    echo 'enum {'; \
	    sed -n -f $(sdir)/zle_widget.sed < widgets.list; \
	    echo '    ZLE_BUILTIN_WIDGET_COUNT'; \
	    echo '};'; \
	) > $@

thingies.list: iwidgets.list
	( \
	    echo '/** thingies.list                            **/'; \
	    echo '/** thingy structures for the known thingies **/'; \
	    echo; \
	    echo '/* format: T("name", TH_FLAGS, w_widget, t_nextthingy) */'; \
	    echo; \
	    sed -e 's/#.*//; /^$$/d; s/" *,.*/"/' \
		-e 's/^"/T("/; s/$$/, 0,/; h' \
		-e 's/-//g; s/^.*"\(.*\)".*/w_\1, t_D\1)/' \
		-e 'H; g; s/\n/ /' \
		< $(sdir)/iwidgets.list; \
	    sed -e 's/#.*//; /^$$/d; s/" *,.*/"/' \
		-e 's/^"/T("./; s/$$/, TH_IMMORTAL,/; h' \
		-e 's/-//g; s/^.*"\.\(.*\)".*/w_\1, t_\1)/' \
		-e 'H; g; s/\n/ /' \
		< $(sdir)/iwidgets.list; \
	) > $@

widgets.list: iwidgets.list
	( \
	    echo '/** widgets.list                               **/'; \
	    echo '/** widget structures for the internal widgets **/'; \
	    echo; \
	    echo '/* format: W(ZLE_FLAGS, t_firstname, functionname) */'; \
	    echo; \
	    sed -e 's/#.*//; /^$$/d; s/-//g' \
		-e 's/^"\(.*\)" *, *\([^ ]*\) *, *\(.*\)/W(\3, t_\1, \2)/' \
		< $(sdir)/iwidgets.list; \
	) > $@

zle_bindings.o zle_bindings..o: zle_widget.h widgets.list thingies.list

clean-here: clean.zle
clean.zle:
	rm -f zle_things.h zle_widget.h widgets.list thingies.list

##### ===== DEPENDENCIES GENERATED FROM zleparameter.mdd ===== #####

MODOBJS_zleparameter = zleparameter.o
MODDOBJS_zleparameter = zleparameter..o  $(NNTRYOBJ)
SYMS_zleparameter = zleparameter.syms 
EPRO_zleparameter = zleparameter.epro
INCS_zleparameter = $(EPRO_zleparameter) 
EXPIMP_zleparameter =  $(IMPOPT)zle.export $(IMPOPT)$(dir_top)/Src/zsh.export $(EXPOPT)zleparameter.export
NXPIMP_zleparameter =
LINKMODS_zleparameter =  zle.$(DL_EXT)  $(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) 
NOLINKMODS_zleparameter = 

proto.zleparameter: $(EPRO_zleparameter)
$(SYMS_zleparameter): $(PROTODEPS)

zleparameter.export: $(SYMS_zleparameter)
	( echo '#!'; cat $(SYMS_zleparameter) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > $@

modobjs.zleparameter: $(MODOBJS_zleparameter)
	echo '' $(MODOBJS_zleparameter) | sed 's" " Zle/"g' >> $(dir_src)/stamp-modobjs.tmp

install.modules.zleparameter: zleparameter.$(DL_EXT)
	$(SHELL) $(sdir_top)/mkinstalldirs $(DESTDIR)$(MODDIR)/zsh
	$(INSTALL_PROGRAM) $(STRIPFLAGS) zleparameter.$(DL_EXT) $(DESTDIR)$(MODDIR)/zsh/zleparameter.$(DL_EXT)

uninstall.modules.zleparameter:
	rm -f $(DESTDIR)$(MODDIR)/zsh/zleparameter.$(DL_EXT)

zleparameter.$(DL_EXT): $(MODDOBJS_zleparameter) zleparameter.export  zle.export $(dir_top)/Src/zsh.export $(NOLINKMODS_zleparameter)
	rm -f $@
	$(DLLINK) $(NXPIMP_zleparameter) $(NNTRYOPT) $(MODDOBJS_zleparameter) $(NOLINKMODS_zleparameter) $(LIBS) 

zleparameter.mdhi: zleparameter.mdhs $(INCS_zleparameter)
	@test -f $@ || echo 'do not delete this file' > $@

zleparameter.mdhs: zleparameter.mdd
	@$(MAKE) -f $(makefile) $(MAKEDEFS) zleparameter.mdh.tmp
	@if cmp -s zleparameter.mdh zleparameter.mdh.tmp; then \
	    rm -f zleparameter.mdh.tmp; \
	    echo "\`zleparameter.mdh' is up to date."; \
	else \
	    mv -f zleparameter.mdh.tmp zleparameter.mdh; \
	    echo "Updated \`zleparameter.mdh'."; \
	fi
	echo 'timestamp for zleparameter.mdh against zleparameter.mdd' > $@

zleparameter.mdh:  zle.mdh $(dir_top)/Src/zsh.mdh   zleparameter.mdhi
	@$(MAKE) -f $(makefile) $(MAKEDEFS) zleparameter.mdh.tmp
	@mv -f zleparameter.mdh.tmp zleparameter.mdh
	@echo "Updated \`zleparameter.mdh'."

zleparameter.mdh.tmp:
	@( \
	    echo '#ifndef have_zshQszleparameter_module'; \
	    echo '#define have_zshQszleparameter_module'; \
	    echo; \
	    echo '# ifndef IMPORTING_MODULE_zshQszleparameter'; \
	    if test no = yes; then \
		echo '#  ifndef MODULE'; \
	    fi; \
	    echo '#   define boot_ boot_zshQszleparameter'; \
	    echo '#   define cleanup_ cleanup_zshQszleparameter'; \
	    echo '#   define setup_ setup_zshQszleparameter'; \
	    echo '#   define finish_ finish_zshQszleparameter'; \
	    if test no = yes; then \
		echo '#  endif /* !MODULE */'; \
	    fi; \
	    echo '# endif /* !IMPORTING_MODULE_zshQszleparameter */'; \
	    echo; \
	    echo '/* Module dependencies */'; \
	    echo '# define IMPORTING_MODULE_zshQszle 1'; \
	    echo '# include "zle.mdh"'; \
	    echo '# define IMPORTING_MODULE_zshQsmain 1'; \
	    echo '# include "$(dir_top)/Src/zsh.mdh"'; \
	    echo; \
	    echo '# undef mod_import_variable'; \
	    echo '# undef mod_import_function'; \
	    echo '# if defined(IMPORTING_MODULE_zshQszleparameter) &&  defined(MODULE)'; \
	    echo '#  define mod_import_variable '; \
	    echo '#  define mod_import_function '; \
	    echo '# else'; \
	    echo '#  define mod_import_function'; \
	    echo '#  define mod_import_variable'; \
	    echo '# endif /* IMPORTING_MODULE_zshQszleparameter && MODULE */'; \
	    for epro in $(EPRO_zleparameter); do \
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
	    echo '#endif /* !have_zshQszleparameter_module */'; \
	) > $@

$(MODOBJS_zleparameter) $(MODDOBJS_zleparameter): zleparameter.mdh

##### ===== DEPENDENCIES FOR REMOTE MODULES ===== #####

$(dir_top)/Src/zsh.mdh:
	false # should only happen with make -n

$(dir_top)/Src/zsh.export:
	false # should only happen with make -n

$(dir_top)/Src/libzsh-$(VERSION).$(DL_EXT) :
	false # should only happen with make -n

##### End of Makefile.in
