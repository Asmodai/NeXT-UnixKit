# Makefile for GNU patch.

# Copyright (C) 1993, 1997, 1998, 1999, 2001, 2002, 2003 Free Software
# Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.
# If not, write to the Free Software Foundation,
# 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#### Start of system configuration section. ####

srcdir = .




CC = cc
ed_PROGRAM = /bin/ed
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
transform = s,x,x,

CFLAGS = -g -O2
CPPFLAGS = 
DEFS = -DHAVE_CONFIG_H
EXEEXT = 
LDFLAGS = 
LIBOBJS =  error$U.o
LIBS = 
OBJEXT = o
PACKAGE_NAME = patch
PACKAGE_VERSION = 2.5.9

prefix = /usr/local
exec_prefix = ${prefix}

bindir = $(exec_prefix)/bin

# Where to put the manual pages.
mandir = ${prefix}/man
man1dir = $(mandir)/man1
# Extension (including `.') for the manual page filenames.
man1ext = .1

# Hook for nonstandard builds.
CONFIG_STATUS = config.status

#### End of system configuration section. ####

SHELL = /bin/sh

LIBSRCS = error.c malloc.c memchr.c mkdir.c \
	realloc.c rmdir.c strcasecmp.c strncasecmp.c
SRCS = $(LIBSRCS) \
	addext.c argmatch.c backupfile.c \
	basename.c dirname.c \
	getopt.c getopt1.c inp.c \
	maketime.c partime.c \
	patch.c pch.c \
	quote.c quotearg.c quotesys.c \
	util.c version.c xmalloc.c
OBJS = $(LIBOBJS) \
	addext.$(OBJEXT) argmatch.$(OBJEXT) backupfile.$(OBJEXT) \
	basename.$(OBJEXT) dirname.$(OBJEXT) \
	getopt.$(OBJEXT) getopt1.$(OBJEXT) inp.$(OBJEXT) \
	maketime.$(OBJEXT) partime.$(OBJEXT) \
	patch.$(OBJEXT) pch.$(OBJEXT) \
	quote.$(OBJEXT) quotearg.$(OBJEXT) quotesys.$(OBJEXT) \
	util.$(OBJEXT) version.$(OBJEXT) xmalloc.$(OBJEXT)
HDRS = argmatch.h backupfile.h common.h dirname.h \
	error.h getopt.h gettext.h \
	inp.h maketime.h partime.h pch.h \
	quote.h quotearg.h quotesys.h \
	unlocked-io.h util.h version.h xalloc.h
MISC = AUTHORS COPYING ChangeLog INSTALL Makefile.in NEWS README \
	aclocal.m4 \
	config.hin configure configure.ac \
	install-sh mkinstalldirs patch.man stdbool.h.in
DISTFILES = $(MISC) $(SRCS) $(HDRS)
DISTFILES_M4 = $(ACINCLUDE_INPUTS)
DISTFILES_PC = pc/chdirsaf.c
DISTFILES_PC_DJGPP = pc/djgpp/README pc/djgpp/config.sed \
	pc/djgpp/configure.bat pc/djgpp/configure.sed

patch_name = `echo patch | sed '$(transform)'`

all:: patch$(EXEEXT)

info::
check::
installcheck::

COMPILE = $(CC) -c $(CPPFLAGS) $(DEFS) -Ded_PROGRAM=\"$(ed_PROGRAM)\" \
	-I. -I$(srcdir) $(CFLAGS)

.c.$(OBJEXT):
	$(COMPILE) $<

patch$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

install:: all installdirs
	$(INSTALL_PROGRAM) patch$(EXEEXT) $(bindir)/$(patch_name)$(EXEEXT)
	-$(INSTALL_DATA) $(srcdir)/patch.man $(man1dir)/$(patch_name)$(man1ext)

installdirs::
	$(SHELL) $(srcdir)/mkinstalldirs $(bindir) $(man1dir)

install-strip::
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install

uninstall::
	rm -f $(bindir)/$(patch_name)$(EXEEXT)
	rm -f $(man1dir)/$(patch_name)$(man1ext)

Makefile: Makefile.in $(CONFIG_STATUS)
	$(SHELL) $(CONFIG_STATUS)
config.status: configure
	$(SHELL) $(CONFIG_STATUS) --recheck
configure: configure.ac $(srcdir)/aclocal.m4
	cd $(srcdir) && autoconf
config.hin: configure.ac $(srcdir)/aclocal.m4
	cd $(srcdir) && rm -f config.hin && autoheader
stdbool.h: stdbool.h.in
	sed -e 's/@''HAVE__BOOL''@/0/g' \
	  <$(srcdir)/stdbool.h.in >stdbool.h

M4DIR = $(srcdir)/m4
ACINCLUDE_INPUTS = \
  $(M4DIR)/backupfile.m4 \
  $(M4DIR)/d-ino.m4 \
  $(M4DIR)/dirname.m4 \
  $(M4DIR)/dos.m4 \
  $(M4DIR)/error.m4 \
  $(M4DIR)/getopt.m4 \
  $(M4DIR)/malloc.m4 \
  $(M4DIR)/mbrtowc.m4 \
  $(M4DIR)/mbstate_t.m4 \
  $(M4DIR)/memchr.m4 \
  $(M4DIR)/mkdir.m4 \
  $(M4DIR)/onceonly.m4 \
  $(M4DIR)/quote.m4 \
  $(M4DIR)/quotearg.m4 \
  $(M4DIR)/realloc.m4 \
  $(M4DIR)/rmdir.m4 \
  $(M4DIR)/setmode.m4 \
  $(M4DIR)/stdbool.m4 \
  $(M4DIR)/unlocked-io.m4 \
  $(M4DIR)/utimbuf.m4 \
  $(M4DIR)/xalloc.m4

$(srcdir)/aclocal.m4: $(ACINCLUDE_INPUTS)
	cat $(ACINCLUDE_INPUTS) >$(srcdir)/aclocal.m4

TAGS: $(HDRS) $(SRCS)
	etags $(HDRS) $(SRCS)

mostlyclean::
	rm -f core* *core *.$(OBJEXT) *_.c stdbool.h

clean:: mostlyclean
	rm -f patch$(EXEEXT)

distclean:: clean
	rm -f Makefile config.cache config.log config.status config.h

maintainer-clean::
	@echo "This command is intended for maintainers to use;"
	@echo "rebuilding the deleted files requires special tools."
	$(MAKE) distclean
	rm -f TAGS

PV = $(PACKAGE_NAME)-$(PACKAGE_VERSION)

dist:: $(DISTFILES) $(DISTFILES_M4) $(DISTFILES_PC) $(DISTFILES_PC_DJGPP)
	rm -rf $(PV)
	mkdir $(PV) $(PV)/m4 $(PV)/pc $(PV)/pc/djgpp
	cp -p $(DISTFILES) $(PV)
	cp -p $(DISTFILES_M4) $(PV)/m4
	cp -p $(DISTFILES_PC) $(PV)/pc
	cp -p $(DISTFILES_PC_DJGPP) $(PV)/pc/djgpp
	tar -chf - $(PV) | gzip -9 >$(PV).tar.gz
	rm -rf $(PV)

$(OBJS): config.h
COMMON = common.h stdbool.h
addext.$(OBJEXT): backupfile.h dirname.h
argmatch.$(OBJEXT): argmatch.h gettext.h error.h \
	quote.h quotearg.h unlocked-io.h
backupfile.$(OBJEXT): argmatch.h backupfile.h dirname.h
basename.$(OBJEXT): dirname.h
dirname.$(OBJEXT): dirname.h xalloc.h
error.$(OBJEXT): error.h gettext.h unlocked-io.h
getopt.$(OBJEXT) getopt1.$(OBJEXT): getopt.h
inp.$(OBJEXT): backupfile.h $(COMMON) inp.h pch.h quotearg.h util.h xalloc.h
maketime.$(OBJEXT): maketime.h partime.h
mkdir.$(OBJEXT): dirname.h xalloc.h
partime.$(OBJEXT): partime.h
patch.$(OBJEXT): argmatch.h backupfile.h $(COMMON) getopt.h inp.h \
	pch.h quotearg.h util.h version.h xalloc.h
pch.$(OBJEXT): backupfile.h $(COMMON) dirname.h inp.h pch.h quotearg.h util.h
quote.$(OBJECT): quote.h quotearg.h
quotearg.$(OBJEXT): gettext.h quotearg.h xalloc.h
quotesys.$(OBJEXT): quotesys.h
strncasecmp.$(OBJEXT): strcasecmp.c
util.$(OBJEXT): backupfile.h $(COMMON) dirname.h maketime.h \
	partime.h quotearg.h quotesys.h util.h version.h xalloc.h
version.$(OBJEXT): $(COMMON) version.h
xmalloc.$(OBJEXT): error.h gettext.h xalloc.h
