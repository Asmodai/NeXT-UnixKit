# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/grep
pkglibdir = $(libdir)/grep
pkgincludedir = $(includedir)/grep

top_builddir = ..

ACLOCAL = /private/Net/nova/Users/asmodai/Projects/NeXT-UnixKit/NEW/gnu/grep/grep-2.4.2/missing aclocal
AUTOCONF = /private/Net/nova/Users/asmodai/Projects/NeXT-UnixKit/NEW/gnu/grep/grep-2.4.2/missing autoconf
AUTOMAKE = /private/Net/nova/Users/asmodai/Projects/NeXT-UnixKit/NEW/gnu/grep/grep-2.4.2/missing automake
AUTOHEADER = /private/Net/nova/Users/asmodai/Projects/NeXT-UnixKit/NEW/gnu/grep/grep-2.4.2/missing autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = i386-next-nextstep3
host_triplet = i386-next-nextstep3
AWK = gawk
CATALOGS =  cs.gmo de.gmo el.gmo eo.gmo es.gmo et.gmo fr.gmo hr.gmo id.gmo it.gmo ja.gmo ko.gmo nl.gmo no.gmo pl.gmo pt_BR.gmo ru.gmo sl.gmo sv.gmo
CATOBJEXT = .gmo
CC = gcc
CPP = gcc -E -traditional-cpp
DATADIRNAME = share
EXEEXT = 
GENCAT = 
GETCONF = getconf
GMOFILES =  cs.gmo de.gmo el.gmo eo.gmo es.gmo et.gmo fr.gmo hr.gmo id.gmo it.gmo ja.gmo ko.gmo nl.gmo no.gmo pl.gmo pt_BR.gmo ru.gmo sl.gmo sv.gmo
GMSGFMT = /usr/X11/bin/msgfmt
GT_NO = 
GT_YES = #YES#
INCLUDE_LOCALE_H = #include <locale.h>
INSTOBJEXT = .mo
INTLDEPS = $(top_builddir)/intl/libintl.a
INTLLIBS = $(top_builddir)/intl/libintl.a
INTLOBJS = $(GETTOBJS)
LIBOBJS = stpcpy$U.o regex$U.o
MAKEINFO = makeinfo
MKINSTALLDIRS = ./mkinstalldirs
MSGFMT = /usr/X11/bin/msgfmt
PACKAGE = grep
POFILES =  cs.po de.po el.po eo.po es.po et.po fr.po hr.po id.po it.po ja.po ko.po nl.po no.po pl.po pt_BR.po ru.po sl.po sv.po
POSUB = po
RANLIB = ranlib
SEP = :
U = 
USE_INCLUDED_LIBINTL = yes
USE_NLS = yes
VERSION = 2.4.2
l = 

AUTOMAKE_OPTIONS = ansi2knr no-dependencies

SUBDIRS = posix

LN = ln

# We let configure figure out if we need the regex.c or not
# it is provided in GNU/Linux system
# regex.c regex.h
bin_PROGRAMS = grep egrep fgrep
base_sources = grep.c grep.h                dfa.c dfa.h                kwset.c  kwset.h                obstack.c obstack.h 	       savedir.c savedir.h                getopt.c getopt1.c getopt.h                search.c getpagesize.h system.h

egrep_SOURCES = $(base_sources) egrepmat.c
fgrep_SOURCES = $(base_sources) fgrepmat.c
grep_SOURCES = $(base_sources) grepmat.c

LDADD =  stpcpy$U.o regex$U.o $(top_builddir)/intl/libintl.a
datadir = $(prefix)/share
localedir = $(datadir)/locale
INCLUDES = -I../intl -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST =               dosbuf.c              vms_fab.c vms_fab.h              regex.h

mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
bin_PROGRAMS =  grep$(EXEEXT) egrep$(EXEEXT) fgrep$(EXEEXT)
PROGRAMS =  $(bin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I..
CPPFLAGS = 
LDFLAGS = -arch m68k -arch i386 -arch hppa -arch sparc 
LIBS = 
ANSI2KNR = 
grep_OBJECTS =  grep$U.o dfa$U.o kwset$U.o obstack$U.o savedir$U.o \
getopt$U.o getopt1$U.o search$U.o grepmat$U.o
grep_LDADD = $(LDADD)
grep_DEPENDENCIES =   stpcpy$U.o regex$U.o
grep_LDFLAGS = 
egrep_OBJECTS =  grep$U.o dfa$U.o kwset$U.o obstack$U.o savedir$U.o \
getopt$U.o getopt1$U.o search$U.o egrepmat$U.o
egrep_LDADD = $(LDADD)
egrep_DEPENDENCIES =   stpcpy$U.o regex$U.o
egrep_LDFLAGS = 
fgrep_OBJECTS =  grep$U.o dfa$U.o kwset$U.o obstack$U.o savedir$U.o \
getopt$U.o getopt1$U.o search$U.o fgrepmat$U.o
fgrep_LDADD = $(LDADD)
fgrep_DEPENDENCIES =   stpcpy$U.o regex$U.o
fgrep_LDFLAGS = 
CFLAGS = -O3 -arch m68k -arch i386 -arch hppa -arch sparc 
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
DIST_COMMON =  Makefile.am Makefile.in alloca.c ansi2knr.1 ansi2knr.c \
memchr.c regex.c stpcpy.c


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
SOURCES = $(grep_SOURCES) $(egrep_SOURCES) $(fgrep_SOURCES)
OBJECTS = $(grep_OBJECTS) $(egrep_OBJECTS) $(fgrep_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu src/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	     $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

.c.o:
	$(COMPILE) -c $<

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

mostlyclean-krextra:

clean-krextra:
	-rm -f ansi2knr

distclean-krextra:

maintainer-clean-krextra:
ansi2knr: ansi2knr.o
	$(LINK) ansi2knr.o $(LIBS)
ansi2knr.o: $(CONFIG_HEADER)


mostlyclean-kr:
	-rm -f *_.c

clean-kr:

distclean-kr:

maintainer-clean-kr:

grep$(EXEEXT): $(grep_OBJECTS) $(grep_DEPENDENCIES)
	@rm -f grep$(EXEEXT)
	$(LINK) $(grep_LDFLAGS) $(grep_OBJECTS) $(grep_LDADD) $(LIBS)

egrep$(EXEEXT): $(egrep_OBJECTS) $(egrep_DEPENDENCIES)
	@rm -f egrep$(EXEEXT)
	$(LINK) $(egrep_LDFLAGS) $(egrep_OBJECTS) $(egrep_LDADD) $(LIBS)

fgrep$(EXEEXT): $(fgrep_OBJECTS) $(fgrep_DEPENDENCIES)
	@rm -f fgrep$(EXEEXT)
	$(LINK) $(fgrep_LDFLAGS) $(fgrep_OBJECTS) $(fgrep_LDADD) $(LIBS)
alloca_.c: alloca.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/alloca.c; then echo $(srcdir)/alloca.c; else echo alloca.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > alloca_.c
dfa_.c: dfa.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/dfa.c; then echo $(srcdir)/dfa.c; else echo dfa.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > dfa_.c
egrepmat_.c: egrepmat.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/egrepmat.c; then echo $(srcdir)/egrepmat.c; else echo egrepmat.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > egrepmat_.c
fgrepmat_.c: fgrepmat.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/fgrepmat.c; then echo $(srcdir)/fgrepmat.c; else echo fgrepmat.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > fgrepmat_.c
getopt_.c: getopt.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/getopt.c; then echo $(srcdir)/getopt.c; else echo getopt.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > getopt_.c
getopt1_.c: getopt1.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/getopt1.c; then echo $(srcdir)/getopt1.c; else echo getopt1.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > getopt1_.c
grep_.c: grep.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/grep.c; then echo $(srcdir)/grep.c; else echo grep.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > grep_.c
grepmat_.c: grepmat.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/grepmat.c; then echo $(srcdir)/grepmat.c; else echo grepmat.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > grepmat_.c
kwset_.c: kwset.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/kwset.c; then echo $(srcdir)/kwset.c; else echo kwset.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > kwset_.c
memchr_.c: memchr.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/memchr.c; then echo $(srcdir)/memchr.c; else echo memchr.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > memchr_.c
obstack_.c: obstack.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/obstack.c; then echo $(srcdir)/obstack.c; else echo obstack.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > obstack_.c
regex_.c: regex.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/regex.c; then echo $(srcdir)/regex.c; else echo regex.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > regex_.c
savedir_.c: savedir.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/savedir.c; then echo $(srcdir)/savedir.c; else echo savedir.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > savedir_.c
search_.c: search.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/search.c; then echo $(srcdir)/search.c; else echo search.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > search_.c
stpcpy_.c: stpcpy.c $(ANSI2KNR)
	$(CPP) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) `if test -f $(srcdir)/stpcpy.c; then echo $(srcdir)/stpcpy.c; else echo stpcpy.c; fi` | sed 's/^# \([0-9]\)/#line \1/' | $(ANSI2KNR) > stpcpy_.c
alloca_.o dfa_.o egrepmat_.o fgrepmat_.o getopt_.o getopt1_.o grep_.o \
grepmat_.o kwset_.o memchr_.o obstack_.o regex_.o savedir_.o search_.o \
stpcpy_.o : $(ANSI2KNR)

# This directory's subdirectories are mostly independent; you can cd
# into them and run `make' without going through this Makefile.
# To change the values of `make' variables: instead of editing Makefiles,
# (1) if the variable is set in `config.status', edit `config.status'
#     (which will cause the Makefiles to be regenerated when you run `make');
# (2) otherwise, pass the desired values on the `make' command line.



all-recursive install-data-recursive install-exec-recursive \
installdirs-recursive install-recursive uninstall-recursive  \
check-recursive installcheck-recursive info-recursive dvi-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	rev=''; list='$(SUBDIRS)'; for subdir in $$list; do \
	  rev="$$subdir $$rev"; \
	  test "$$subdir" = "." && dot_seen=yes; \
	done; \
	test "$$dot_seen" = "no" && rev=". $$rev"; \
	target=`echo $@ | sed s/-recursive//`; \
	for subdir in $$rev; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test "$$subdir" = . || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) tags); \
	done

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS: tags-recursive $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
   if test "$$subdir" = .; then :; else \
	    test -f $$subdir/TAGS && tags="$$tags -i $$here/$$subdir/TAGS"; \
   fi; \
	done; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = src

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
	for subdir in $(SUBDIRS); do \
	  if test "$$subdir" = .; then :; else \
	    test -d $(distdir)/$$subdir \
	    || mkdir $(distdir)/$$subdir \
	    || exit 1; \
	    chmod 777 $(distdir)/$$subdir; \
	    (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) top_distdir=../$(top_distdir) distdir=../$(distdir)/$$subdir distdir) \
	      || exit 1; \
	  fi; \
	done
info-am:
info: info-recursive
dvi-am:
dvi: dvi-recursive
check-am: all-am
check: check-recursive
installcheck-am:
installcheck: installcheck-recursive
install-exec-am: install-binPROGRAMS
install-exec: install-exec-recursive

install-data-am:
install-data: install-data-recursive

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-recursive
uninstall-am: uninstall-binPROGRAMS
uninstall: uninstall-recursive
all-am: Makefile $(ANSI2KNR) $(PROGRAMS)
all-redirect: all-recursive
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs: installdirs-recursive
installdirs-am:
	$(mkinstalldirs)  $(DESTDIR)$(bindir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-binPROGRAMS mostlyclean-compile \
		mostlyclean-krextra mostlyclean-kr mostlyclean-tags \
		mostlyclean-generic

mostlyclean: mostlyclean-recursive

clean-am:  clean-binPROGRAMS clean-compile clean-krextra clean-kr \
		clean-tags clean-generic mostlyclean-am

clean: clean-recursive

distclean-am:  distclean-binPROGRAMS distclean-compile distclean-krextra \
		distclean-kr distclean-tags distclean-generic clean-am

distclean: distclean-recursive

maintainer-clean-am:  maintainer-clean-binPROGRAMS \
		maintainer-clean-compile maintainer-clean-krextra \
		maintainer-clean-kr maintainer-clean-tags \
		maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-recursive

.PHONY: mostlyclean-binPROGRAMS distclean-binPROGRAMS clean-binPROGRAMS \
maintainer-clean-binPROGRAMS uninstall-binPROGRAMS install-binPROGRAMS \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile mostlyclean-krextra distclean-krextra \
clean-krextra maintainer-clean-krextra mostlyclean-kr distclean-kr \
clean-kr maintainer-clean-kr install-data-recursive \
uninstall-data-recursive install-exec-recursive \
uninstall-exec-recursive installdirs-recursive uninstalldirs-recursive \
all-recursive check-recursive installcheck-recursive info-recursive \
dvi-recursive mostlyclean-recursive distclean-recursive clean-recursive \
maintainer-clean-recursive tags tags-recursive mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-am uninstall all-redirect all-am all installdirs-am \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
