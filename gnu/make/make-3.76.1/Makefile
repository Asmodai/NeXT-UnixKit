# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.2 from Makefile.am

# Copyright (C) 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy, distribute and modify it.

# -*-Makefile-*-, or close enough


SHELL = /bin/sh

srcdir = .
top_srcdir = .
prefix = /tmp/make_build
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

pkgdatadir = $(datadir)/make
pkglibdir = $(libdir)/make
pkgincludedir = $(includedir)/make

top_builddir = .

ACLOCAL = /private/Net/nova/Users/asmodai/Projects/NeXT-UnixKit/NEW/gnu/make/make-3.76.1/missing aclocal
AUTOCONF = /private/Net/nova/Users/asmodai/Projects/NeXT-UnixKit/NEW/gnu/make/make-3.76.1/missing autoconf
AUTOMAKE = /private/Net/nova/Users/asmodai/Projects/NeXT-UnixKit/NEW/gnu/make/make-3.76.1/missing automake
AUTOHEADER = /private/Net/nova/Users/asmodai/Projects/NeXT-UnixKit/NEW/gnu/make/make-3.76.1/missing autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = true
PRE_INSTALL = true
POST_INSTALL = true
NORMAL_UNINSTALL = true
PRE_UNINSTALL = true
POST_UNINSTALL = true
CC = gcc
CPP = gcc -E -traditional-cpp
LIBOBJS =  getloadavg.o
MAKEINFO = makeinfo
PACKAGE = make
REMOTE = stub
VERSION = 3.76.1

AUTOMAKE_OPTIONS = 1.2

bin_PROGRAMS =	make

make_SOURCES =	main.c commands.c job.c dir.c file.c misc.c read.c remake.c \
		rule.c implicit.c default.c variable.c expand.c function.c \
		vpath.c version.c ar.c arscan.c remote-$(REMOTE).c \
		commands.h dep.h filedef.h job.h make.h rule.h variable.h \
		signame.c signame.h \
		getopt.c getopt1.c getopt.h
make_LDADD =	 getloadavg.o  glob/libglob.a

info_TEXINFOS =	make.texinfo

INCLUDES =	-I$(srcdir)/glob -DLIBDIR=\"$(libdir)\" -DINCLUDEDIR=\"$(includedir)\"

BUILT_SOURCES =	README build.sh.in

EXTRA_DIST = 	make.man $(BUILT_SOURCES) remote-cstms.c\
		make-stds.texi texinfo.tex SCOPTIONS SMakefile\
		README.Amiga Makefile.ami config.ami make.lnk amiga.c amiga.h\
		README.DOS Makefile.DOS configure.bat dosbuild.bat configh.dos\
		README.W32 NMakefile config.h.W32 build_w32.bat subproc.bat\
		readme.vms makefile.vms makefile.com config.h-vms vmsdir.h\
		vmsfunctions.c vmsify.c

SUBDIRS =	glob

MOSTLYCLEANFILES = loadavg.c
CLEANFILES =	loadavg
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = config.h
CONFIG_CLEAN_FILES =  build.sh
PROGRAMS =  $(bin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I.
CPPFLAGS = 
LDFLAGS = -arch m68k -arch i386 -arch hppa -arch sparc 
LIBS = -L/usr/local/lib 
make_OBJECTS =  main.o commands.o job.o dir.o file.o misc.o read.o \
remake.o rule.o implicit.o default.o variable.o expand.o function.o \
vpath.o version.o ar.o arscan.o remote-$(REMOTE).o signame.o getopt.o \
getopt1.o
make_DEPENDENCIES =   getloadavg.o  glob/libglob.a
make_LDFLAGS = 
CFLAGS = -O3 -arch m68k -arch i386 -arch hppa -arch sparc 
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
LINK = $(CC) $(CFLAGS) $(LDFLAGS) -o $@
TEXI2DVI = texi2dvi
TEXINFO_TEX = $(srcdir)/texinfo.tex
INFO_DEPS = make.info
DVIS = make.dvi
TEXINFOS = make.texinfo
DIST_COMMON =  README AUTHORS COPYING ChangeLog INSTALL Makefile.am \
Makefile.in NEWS acconfig.h aclocal.m4 alloca.c build.sh.in config.h.in \
configure configure.in getloadavg.c install-sh missing mkinstalldirs \
stamp-h.in texinfo.tex


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP = --best
SOURCES = $(make_SOURCES)
OBJECTS = $(make_OBJECTS)

default: all

.SUFFIXES:
.SUFFIXES: .c .dvi .info .o .ps .texi .texinfo
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps Makefile

Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

$(ACLOCAL_M4):  configure.in 
	cd $(srcdir) && $(ACLOCAL)

config.status: $(srcdir)/configure
	$(SHELL) ./config.status --recheck
$(srcdir)/configure: $(srcdir)/configure.in $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)
	cd $(srcdir) && $(AUTOCONF)

config.h: stamp-h
stamp-h: $(srcdir)/config.h.in $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES= CONFIG_HEADERS=config.h \
	     $(SHELL) ./config.status
	@echo timestamp > stamp-h
$(srcdir)/config.h.in: $(srcdir)/stamp-h.in
$(srcdir)/stamp-h.in: $(top_srcdir)/configure.in $(ACLOCAL_M4) acconfig.h
	cd $(top_srcdir) && $(AUTOHEADER)
	@echo timestamp > $(srcdir)/stamp-h.in

mostlyclean-hdr:

clean-hdr:

distclean-hdr:
	rm -f config.h

maintainer-clean-hdr:
build.sh: $(top_builddir)/config.status build.sh.in
	cd $(top_builddir) && CONFIG_FILES=$@ CONFIG_HEADERS= ./config.status

mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_PROGRAM) $$p $(bindir)/`echo $$p|sed '$(transform)'`"; \
	     $(INSTALL_PROGRAM) $$p $(bindir)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(bindir)/`echo $$p|sed '$(transform)'`; \
	done

.c.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	rm -f *.o core

clean-compile:

distclean-compile:
	rm -f *.tab.c

maintainer-clean-compile:

make: $(make_OBJECTS) $(make_DEPENDENCIES)
	@rm -f make
	$(LINK) $(make_LDFLAGS) $(make_OBJECTS) $(make_LDADD) $(LIBS)

make.info: make.texinfo
make.dvi: make.texinfo


DVIPS = dvips

.texi.info:
	@cd $(srcdir) && rm -f $@ $@-[0-9] $@-[0-9][0-9]
	cd $(srcdir) \
	  && $(MAKEINFO) `echo $< | sed 's,.*/,,'`

.texi.dvi:
	TEXINPUTS=$(srcdir):$$TEXINPUTS \
	  MAKEINFO='$(MAKEINFO) -I $(srcdir)' $(TEXI2DVI) $<

.texi:
	@cd $(srcdir) && rm -f $@ $@-[0-9] $@-[0-9][0-9]
	cd $(srcdir) \
	  && $(MAKEINFO) `echo $< | sed 's,.*/,,'`

.texinfo.info:
	@cd $(srcdir) && rm -f $@ $@-[0-9] $@-[0-9][0-9]
	cd $(srcdir) \
	  && $(MAKEINFO) `echo $< | sed 's,.*/,,'`

.texinfo:
	@cd $(srcdir) && rm -f $@ $@-[0-9] $@-[0-9][0-9]
	cd $(srcdir) \
	  && $(MAKEINFO) `echo $< | sed 's,.*/,,'`

.texinfo.dvi:
	TEXINPUTS=$(srcdir):$$TEXINPUTS \
	  MAKEINFO='$(MAKEINFO) -I $(srcdir)' $(TEXI2DVI) $<
.dvi.ps:
	$(DVIPS) $< -o $@

install-info-am: $(INFO_DEPS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(infodir)
	@for file in $(INFO_DEPS); do \
	  d=$(srcdir); \
	  for ifile in `cd $$d && echo $$file $$file-[0-9] $$file-[0-9][0-9]`; do \
	    if test -f $$d/$$ifile; then \
	      echo " $(INSTALL_DATA) $$d/$$ifile $(infodir)/$$ifile"; \
	      $(INSTALL_DATA) $$d/$$ifile $(infodir)/$$ifile; \
	    else : ; fi; \
	  done; \
	done
	@$(POST_INSTALL)
	@if $(SHELL) -c 'install-info --version | sed 1q | fgrep -s -v -i debian' >/dev/null 2>&1; then \
	  for file in $(INFO_DEPS); do \
	    echo " install-info --info-dir=$(infodir) $(infodir)/$$file";\
	    install-info --info-dir=$(infodir) $(infodir)/$$file || :;\
	  done; \
	else : ; fi

uninstall-info:
	$(PRE_UNINSTALL)
	@if $(SHELL) -c 'install-info --version | sed 1q | fgrep -s -v -i debian' >/dev/null 2>&1; then \
	  ii=yes; \
	else ii=; fi; \
	for file in $(INFO_DEPS); do \
	  test -z $ii || install-info --info-dir=$(infodir) --remove $$file; \
	done
	$(NORMAL_UNINSTALL)
	for file in $(INFO_DEPS); do \
	  (cd $(infodir) && rm -f $$file $$file-[0-9] $$file-[0-9][0-9]); \
	done

dist-info: $(INFO_DEPS)
	for base in $(INFO_DEPS); do \
	  d=$(srcdir); \
	  for file in `cd $$d && eval echo $$base*`; do \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file; \
	  done; \
	done

mostlyclean-aminfo:
	rm -f make.aux make.cp make.cps make.dvi make.fn make.fns make.ky \
	  make.log make.pg make.toc make.tp make.tps make.vr make.vrs \
	  make.op make.tr make.cv

clean-aminfo:

distclean-aminfo:

maintainer-clean-aminfo:
	for i in $(INFO_DEPS); do rm -f `eval echo $$i*`; done

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
	for subdir in $(SUBDIRS); do \
	  target=`echo $@ | sed s/-recursive//`; \
	  echo "Making $$target in $$subdir"; \
	  (cd $$subdir && $(MAKE) $$target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	rev=''; for subdir in $(SUBDIRS); do rev="$$rev $$subdir"; done; \
	for subdir in $$rev; do \
	  target=`echo $@ | sed s/-recursive//`; \
	  echo "Making $$target in $$subdir"; \
	  (cd $$subdir && $(MAKE) $$target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  (cd $$subdir && $(MAKE) tags); \
	done

tags: TAGS

ID: $(HEADERS) $(SOURCES)
	here=`pwd` && cd $(srcdir) && mkid -f$$here/ID $(SOURCES) $(HEADERS)

TAGS: tags-recursive $(HEADERS) $(SOURCES) config.h.in $(TAGS_DEPENDENCIES)
	tags=; \
	here=`pwd`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test -f $$subdir/TAGS && tags="$$tags -i $$here/$$subdir/TAGS"; \
	done; \
	test -z "$(ETAGS_ARGS)config.h.in$(SOURCES)$(HEADERS)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags config.h.in $(SOURCES) $(HEADERS) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	rm -f TAGS ID

maintainer-clean-tags:

distdir = $(PACKAGE)-$(VERSION)
top_distdir = $(distdir)

# This target untars the dist file and tries a VPATH configuration.  Then
# it guarantees that the distribution is self-contained by making another
# tarfile.
distcheck: dist
	rm -rf $(distdir)
	GZIP=$(GZIP) $(TAR) zxf $(distdir).tar.gz
	mkdir $(distdir)/=build
	mkdir $(distdir)/=inst
	dc_install_base=`cd $(distdir)/=inst && pwd`; \
	cd $(distdir)/=build \
	  && ../configure --srcdir=.. --prefix=$$dc_install_base \
	  && $(MAKE) \
	  && $(MAKE) dvi \
	  && $(MAKE) check \
	  && $(MAKE) install \
	  && $(MAKE) installcheck \
	  && $(MAKE) dist
	rm -rf $(distdir)
	@echo "========================"; \
	echo "$(distdir).tar.gz is ready for distribution"; \
	echo "========================"
dist: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP) $(TAR) chozf $(distdir).tar.gz $(distdir)
	rm -rf $(distdir)
dist-all: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP) $(TAR) chozf $(distdir).tar.gz $(distdir)
	rm -rf $(distdir)
distdir: $(DISTFILES)
	rm -rf $(distdir)
	mkdir $(distdir)
	-chmod 777 $(distdir)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  test -f $(distdir)/$$file \
	  || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	  || cp -p $$d/$$file $(distdir)/$$file; \
	done
	for subdir in $(SUBDIRS); do		\
	  test -d $(distdir)/$$subdir		\
	  || mkdir $(distdir)/$$subdir		\
	  || exit 1;				\
	  chmod 777 $(distdir)/$$subdir;	\
	  (cd $$subdir && $(MAKE) top_distdir=../$(top_distdir)/$$subdir distdir=../$(distdir)/$$subdir distdir) \
	    || exit 1; \
	done
	$(MAKE) top_distdir="$(top_distdir)" distdir="$(distdir)" dist-info
	$(MAKE) top_distdir="$(top_distdir)" distdir="$(distdir)" dist-hook
alloca.o alloca.lo: alloca.c config.h
ar.o ar.lo: ar.c make.h config.h filedef.h dep.h glob/fnmatch.h
arscan.o arscan.lo: arscan.c make.h config.h
commands.o commands.lo: commands.c make.h config.h dep.h filedef.h \
	variable.h job.h commands.h
default.o default.lo: default.c make.h config.h rule.h dep.h filedef.h \
	job.h commands.h variable.h
dir.o dir.lo: dir.c make.h config.h glob/glob.h
expand.o expand.lo: expand.c make.h config.h filedef.h job.h commands.h \
	variable.h
file.o file.lo: file.c make.h config.h dep.h filedef.h job.h commands.h \
	variable.h
function.o function.lo: function.c make.h config.h filedef.h variable.h \
	dep.h job.h commands.h
getloadavg.o getloadavg.lo: getloadavg.c config.h
getopt.o getopt.lo: getopt.c config.h getopt.h
getopt1.o getopt1.lo: getopt1.c config.h getopt.h
implicit.o implicit.lo: implicit.c make.h config.h rule.h dep.h \
	filedef.h
job.o job.lo: job.c make.h config.h job.h filedef.h commands.h \
	variable.h
main.o main.lo: main.c make.h config.h dep.h filedef.h variable.h job.h \
	commands.h getopt.h
misc.o misc.lo: misc.c make.h config.h dep.h
read.o read.lo: read.c make.h config.h dep.h filedef.h job.h commands.h \
	variable.h glob/glob.h
remake.o remake.lo: remake.c make.h config.h filedef.h job.h commands.h \
	dep.h
remote-stub.o remote-stub.lo: remote-stub.c make.h config.h filedef.h \
	job.h commands.h
rule.o rule.lo: rule.c make.h config.h dep.h filedef.h job.h commands.h \
	variable.h rule.h
signame.o signame.lo: signame.c config.h signame.h
variable.o variable.lo: variable.c make.h config.h dep.h filedef.h job.h \
	commands.h variable.h
version.o version.lo: version.c config.h
vpath.o vpath.lo: vpath.c make.h config.h filedef.h variable.h

info: $(INFO_DEPS) info-recursive
dvi: $(DVIS) dvi-recursive
check: all
	$(MAKE) check-recursive check-local
installcheck: installcheck-recursive
all-recursive-am: config.h
	$(MAKE) all-recursive

all-am: Makefile $(INFO_DEPS) $(PROGRAMS) config.h

install-exec-am: install-binPROGRAMS

install-data-am: install-info-am

uninstall-am: uninstall-binPROGRAMS uninstall-info

install-exec: install-exec-recursive install-exec-am
	@$(NORMAL_INSTALL)

install-data: install-data-recursive install-data-am
	@$(NORMAL_INSTALL)

install: install-recursive install-exec-am install-data-am
	@:

uninstall: uninstall-recursive uninstall-am

all: all-recursive-am all-am

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' INSTALL_SCRIPT='$(INSTALL_PROGRAM)' install
installdirs: installdirs-recursive
	$(mkinstalldirs)  $(bindir) $(infodir)


mostlyclean-generic:
	test -z "$(MOSTLYCLEANFILES)" || rm -f $(MOSTLYCLEANFILES)

clean-generic:
	test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	rm -f Makefile $(DISTCLEANFILES)
	rm -f config.cache config.log stamp-h stamp-h[0-9]*
	test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)

maintainer-clean-generic:
	test -z "$(MAINTAINERCLEANFILES)" || rm -f $(MAINTAINERCLEANFILES)
	test -z "$(BUILT_SOURCES)" || rm -f $(BUILT_SOURCES)
mostlyclean-am:  mostlyclean-hdr mostlyclean-binPROGRAMS \
		mostlyclean-compile mostlyclean-aminfo mostlyclean-tags \
		mostlyclean-generic

clean-am:  clean-hdr clean-binPROGRAMS clean-compile clean-aminfo \
		clean-tags clean-generic mostlyclean-am

distclean-am:  distclean-hdr distclean-binPROGRAMS distclean-compile \
		distclean-aminfo distclean-tags distclean-generic \
		clean-am

maintainer-clean-am:  maintainer-clean-hdr maintainer-clean-binPROGRAMS \
		maintainer-clean-compile maintainer-clean-aminfo \
		maintainer-clean-tags maintainer-clean-generic \
		distclean-am

mostlyclean:  mostlyclean-recursive mostlyclean-am

clean:  clean-recursive clean-am

distclean:  distclean-recursive distclean-am
	rm -f config.status

maintainer-clean:  maintainer-clean-recursive maintainer-clean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."
	rm -f config.status

.PHONY: default mostlyclean-hdr distclean-hdr clean-hdr \
maintainer-clean-hdr mostlyclean-binPROGRAMS distclean-binPROGRAMS \
clean-binPROGRAMS maintainer-clean-binPROGRAMS uninstall-binPROGRAMS \
install-binPROGRAMS mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile install-info-am uninstall-info \
mostlyclean-aminfo distclean-aminfo clean-aminfo \
maintainer-clean-aminfo install-data-recursive uninstall-data-recursive \
install-exec-recursive uninstall-exec-recursive installdirs-recursive \
uninstalldirs-recursive all-recursive check-recursive \
installcheck-recursive info-recursive dvi-recursive \
mostlyclean-recursive distclean-recursive clean-recursive \
maintainer-clean-recursive tags tags-recursive mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir info dvi \
check-local installcheck all-recursive-am all-am install-exec-am \
install-data-am uninstall-am install-exec install-data install \
uninstall all installdirs mostlyclean-generic distclean-generic \
clean-generic maintainer-clean-generic clean mostlyclean distclean \
maintainer-clean


# --------------- Local DIST Section

# Install the w32 subdirectory
#
dist-hook:
	(cd $(srcdir); \
	 w32=`find w32 -follow \( -name CVS -prune \) -o -type f -print`; \
	 tar chf - $$w32) \
	| (cd $(distdir); tar xfBp -)

# --------------- Local CHECK Section

check-local: check-loadavg check-regression
.PHONY: check-loadavg check-regression

# > check-loadavg
#
loadavg: loadavg.c config.h
	@rm -f loadavg
	$(LINK) -I. -I$(srcdir) -DHAVE_CONFIG_H -DTEST $(make_LDFLAGS) loadavg.c $(LIBS)
# We copy getloadavg.c into a different file rather than compiling it
# directly because some compilers clobber getloadavg.o in the process.
loadavg.c: getloadavg.c
	ln $(srcdir)/getloadavg.c loadavg.c || \
	cp $(srcdir)/getloadavg.c loadavg.c
check-loadavg: loadavg
	@echo The system uptime program believes the load average to be:
	-uptime
	@echo The GNU load average checking code believes:
	-./loadavg

# > check-regression
#
# Look for the make test suite, and run it if found.  Look in MAKE_TEST if
# specified, or else in the srcdir or the distdir, their parents, and _their_
# parents.
#
check-regression: all
	here=`pwd`; testdir=""; \
	  case "$(MAKE_TEST)" in "") \
	    for d1 in $$here $(srcdir); do \
	      for d2 in ../.. .. .; do \
		all=`echo $$d1/$$d2/make-test-[0-9]*/run_make_tests`; \
		case "$$all" in \
		  "$$d1/$$d2/make-test-[0-9]*/run_make_tests") : ;; \
		  *) try=`for x in $$all; do echo $$x; done | sort | tail -1`;\
		     testdir=`dirname $$try` ;; esac; \
            done; done ;; \
	    *) testdir="$(MAKE_TEST)" ;; \
	  esac; \
	  case "$$testdir" in \
	    "") echo "Couldn't find make-test-* test suite."; exit 0;; \
	  esac; \
	  echo "cd $$testdir && ./run_make_tests -make_path $$here/make"; \
	  cd $$testdir && ./run_make_tests -make_path $$here/make

# --------------- Maintainer's Section

# Note this requires GNU make.  Not to worry, since it will only be included
# in the Makefile if we're in the maintainer's environment.
#include $(srcdir)/maintMakefile

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
