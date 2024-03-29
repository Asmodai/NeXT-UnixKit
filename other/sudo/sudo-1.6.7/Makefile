#
# Copyright (c) 1996, 1998-2003 Todd C. Miller <Todd.Miller@courtesan.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission
#    from the author.
#
# 4. Products derived from this software may not be called "Sudo" nor
#    may "Sudo" appear in their names without specific prior written
#    permission from the author.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
# THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Sponsored in part by the Defense Advanced Research Projects
# Agency (DARPA) and Air Force Research Laboratory, Air Force
# Materiel Command, USAF, under agreement number F39502-99-1-0512.
#
# Makefile.  Generated from Makefile.in by configure.
#
# $Sudo: Makefile.in,v 1.230 2003/03/15 20:31:02 millert Exp $
#

#### Start of system configuration section. ####

srcdir = .
authdir = $(srcdir)/auth


# Compiler & tools to use
CC = gcc
LEX = flex
YACC = yacc
NROFF = nroff

# Our install program supports extra flags...
INSTALL = $(SHELL) $(srcdir)/install-sh -c

# Libraries
LIBS = -posix -arch i386 -arch m68k -arch hppa -arch sparc
NET_LIBS = 
SUDO_LIBS =   $(LIBS) $(NET_LIBS)

# C preprocessor flags
CPPFLAGS = -I. -I$(srcdir) 

# Usually -O and/or -g
CFLAGS = -O3 -posix -arch i386 -arch m68k -arch hppa -arch sparc

# Flags to pass to the link stage
LDFLAGS = -lposix
SUDO_LDFLAGS =  $(LDFLAGS)

# Where to install things...
prefix = /Net/nova/Users/asmodai/Projects/NeXT-UnixKit/sudo-1.6.7/build
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
sbindir = $(exec_prefix)/sbin
sysconfdir = /etc
mandir = $(prefix)/man

# Directory in which to install sudo.
sudodir = $(bindir)

# Directory in which to install visudo
visudodir = $(sbindir)

# Directory in which to install the sudoers file
sudoersdir = $(sysconfdir)

# Directory in which to install the man page
mantype = man
mansectsu = 8
mansectform = 5
mandirsu = $(mandir)/$(mantype)$(mansectsu)
mandirform = $(mandir)/$(mantype)$(mansectform)

# User and group ids the installed files should be "owned" by
install_uid = 0
install_gid = 0

# User, group, and mode the sudoers file should be "owned" by (configure)
sudoers_uid = 0
sudoers_gid = 0
sudoers_mode = 0440

# Pass in paths and uid/gid + OS dependent defined
DEFS =  -D_PATH_SUDOERS=\"$(sudoersdir)/sudoers\" -D_PATH_SUDOERS_TMP=\"$(sudoersdir)/sudoers.tmp\" -DSUDOERS_UID=$(sudoers_uid) -DSUDOERS_GID=$(sudoers_gid) -DSUDOERS_MODE=$(sudoers_mode)

#### End of system configuration section. ####

SHELL = /bin/sh

PROGS = sudo visudo

SRCS = alloc.c alloca.c check.c def_data.c defaults.c env.c fileops.c \
       find_path.c fnmatch.c getcwd.c getspwuid.c goodpath.c \
       interfaces.c lex.yy.c lsearch.c logging.c parse.c parse.lex \
       parse.yacc set_perms.c sigaction.c snprintf.c strcasecmp.c strerror.c \
       strlcat.c strlcpy.c sudo.c sudo.tab.c testsudoers.c tgetpass.c utime.c \
       visudo.c $(AUTH_SRCS)

AUTH_SRCS = auth/afs.c auth/aix_auth.c auth/bsdauth.c auth/dce.c auth/fwtk.c \
	    auth/kerb4.c auth/kerb5.c auth/pam.c auth/passwd.c auth/rfc1938.c \
	    auth/secureware.c auth/securid.c auth/securid5.c auth/sia.c \
	    auth/sudo_auth.c

HDRS = compat.h def_data.h defaults.h ins_2001.h ins_classic.h ins_csops.h \
       ins_goons.h insults.h interfaces.h logging.h parse.h sudo.h sudo.tab.h \
       version.h auth/sudo_auth.h emul/fnmatch.h emul/search.h emul/utime.h

AUTH_OBJS = sudo_auth.o  passwd.o

PARSEOBJS = sudo.tab.o lex.yy.o alloc.o defaults.o

SUDOBJS = check.o env.o getspwuid.o goodpath.o fileops.o find_path.o \
	  interfaces.o logging.o parse.o set_perms.o sudo.o tgetpass.o \
	  $(AUTH_OBJS) $(PARSEOBJS)

VISUDOBJS = visudo.o fileops.o goodpath.o find_path.o $(PARSEOBJS)

TESTOBJS = interfaces.o testsudoers.o $(PARSEOBJS)

LIBOBJS =  lsearch$U.o fnmatch$U.o strlcpy$U.o strlcat$U.o snprintf$U.o 

VERSION = 1.6.7p4

DISTFILES = $(SRCS) $(HDRS) BUGS CHANGES HISTORY INSTALL INSTALL.configure \
	    LICENSE Makefile.in PORTING README RUNSON TODO TROUBLESHOOTING \
	    UPGRADE aclocal.m4 aixcrypt.exp config.guess config.h.in \
            config.sub configure configure.in def_data.in fnmatch.3 indent.pro \
	    install-sh mkdefaults mkinstalldirs pathnames.h.in sample.pam \
	    sample.syslog.conf sample.sudoers sudo.cat sudo.man.in sudo.pod \
	    sudoers sudoers.cat sudoers.man.in sudoers.pod visudo.cat \
	    visudo.man.in visudo.pod auth/API

BINFILES= BUGS CHANGES HISTORY LICENSE README TODO TROUBLESHOOTING \
	  UPGRADE install-sh mkinstalldirs sample.syslog.conf sample.sudoers \
	  sudo sudo.cat sudo.man sudo.pod sudoers sudoers.cat sudoers.man \
	  sudoers.pod visudo visudo.cat visudo.man visudo.pod

BINSPECIAL= INSTALL.binary Makefile.binary

SUDODEP = $(srcdir)/sudo.h $(srcdir)/compat.h $(srcdir)/defaults.h \
	  $(srcdir)/logging.h config.h def_data.h pathnames.h

AUTHDEP = $(SUDODEP) $(authdir)/sudo_auth.h

INSDEP = $(srcdir)/ins_2001.h $(srcdir)/ins_classic.h $(srcdir)/ins_csops.h \
	 $(srcdir)/ins_goons.h $(srcdir)/insults.h

all: $(PROGS)

.SUFFIXES: .o .c .h .lex .yacc .man .cat

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $<

.man.cat:
	@rm -f $(srcdir)/$@
	$(NROFF) -man $< > $(srcdir)/$@

sudo: $(SUDOBJS) $(LIBOBJS)
	$(CC) -o $@ $(SUDOBJS) $(LIBOBJS) $(SUDO_LDFLAGS) $(SUDO_LIBS)

visudo: $(VISUDOBJS) $(LIBOBJS)
	$(CC) -o $@ $(VISUDOBJS) $(LIBOBJS) $(LDFLAGS) $(LIBS) $(NET_LIBS)

testsudoers: $(TESTOBJS) $(LIBOBJS)
	$(CC) -o $@ $(TESTOBJS) $(LIBOBJS) $(LDFLAGS) $(LIBS) $(NET_LIBS)

# Uncomment the following if you want "make clean" to clean the parser
#PARSESRCS = sudo.tab.h sudo.tab.c lex.yy.c

# Uncomment the following if you intend to modify parse.yacc
#sudo.tab.c sudo.tab.h: parse.yacc
#	rm -f sudo.tab.h sudo.tab.c
#	$(YACC) -d -b sudo $(srcdir)/parse.yacc

# Uncomment the following if you intend to modify parse.lex
#lex.yy.c: parse.lex
#	rm -f lex.yy.c
#	$(LEX) $(srcdir)/parse.lex

# Uncomment the following if you intend to modify def_data.in
#def_data.h def_data.c: def_data.in
#	perl $(srcdir)/mkdefaults -o def_data $(srcdir)/def_data.in

# Dependencies (not counting auth functions)
alloc.o: alloc.c $(SUDODEP)
check.o: check.c $(SUDODEP)
env.o: env.c $(SUDODEP)
fileops.o: fileops.c $(SUDODEP)
find_path.o: find_path.c $(SUDODEP)
getspwuid.o: getspwuid.c $(SUDODEP)
goodpath.o: goodpath.c $(SUDODEP)
logging.o: logging.c $(SUDODEP)
set_perms.o: set_perms.c $(SUDODEP)
tgetpass.o: tgetpass.c $(SUDODEP)
visudo.o: visudo.c $(SUDODEP) version.h
sudo.o: sudo.c $(SUDODEP) interfaces.h version.h
interfaces.o: interfaces.c $(SUDODEP) interfaces.h
testsudoers.o: testsudoers.c $(SUDODEP) parse.h interfaces.h
parse.o: parse.c $(SUDODEP) parse.h interfaces.h
lex.yy.o: lex.yy.c $(SUDODEP) parse.h sudo.tab.h
sudo.tab.o: sudo.tab.c $(SUDODEP) parse.h sudo.tab.c sudo.tab.h
defaults.o: defaults.c $(SUDODEP) def_data.c auth/sudo_auth.h
fnmatch.o: fnmatch.c config.h compat.h emul/fnmatch.h
getcwd.o: getcwd.c config.h compat.h
lsearch.o: lsearch.c config.h compat.h emul/search.h
snprintf.o: snprintf.c config.h compat.h
strcasecmp.o: strcasecmp.c config.h
strlcat.o: strlcat.c config.h
strlcpy.o: strlcpy.c config.h
strerror.o: strerror.c config.h
utime.o: utime.c config.h pathnames.h compat.h emul/utime.h

# Authentication functions live in "auth" dir and so need extra care
sudo_auth.o: $(authdir)/sudo_auth.c $(AUTHDEP) $(INSDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/sudo_auth.c
afs.o: $(authdir)/afs.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/afs.c
aix_auth.o: $(authdir)/aix_auth.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/aix_auth.c
bsdauth.o: $(authdir)/bsdauth.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/bsdauth.c
dce.o: $(authdir)/dce.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/dce.c
fwtk.o: $(authdir)/fwtk.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/fwtk.c
kerb4.o: $(authdir)/kerb4.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/kerb4.c
kerb5.o: $(authdir)/kerb5.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/kerb5.c
pam.o: $(authdir)/pam.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/pam.c
passwd.o: $(authdir)/passwd.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/passwd.c
rfc1938.o: $(authdir)/rfc1938.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/rfc1938.c
secureware.o: $(authdir)/secureware.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/secureware.c
securid.o: $(authdir)/securid.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/securid.c
securid5.o: $(authdir)/securid5.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/securid5.c
sia.o: $(authdir)/sia.c $(AUTHDEP)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEFS) $(OPTIONS) $(authdir)/sia.c

sudo.man.in: $(srcdir)/sudo.pod
	@rm -f $(srcdir)/$@
	( cd $(srcdir); mansectsu=`echo @MANSECTSU@|tr A-Z a-z`; mansectform=`echo @MANSECTFORM@|tr A-Z a-z`; sed -n -e 1d -e '/^=pod/q' -e 's/^/.\\" /p' sudo.pod > $@; pod2man --quotes=none --date="`date '+%B %e, %Y'`" --section=$$mansectsu --release=$(VERSION) --center="MAINTENANCE COMMANDS" sudo.pod | sed -e "s/(5)/($$mansectform)/" -e "s/(8)/($$mansectsu)/" >> $@ )

sudo.man: sudo.man.in
	CONFIG_FILES=$@ CONFIG_HEADERS= sh ./config.status

sudo.cat: sudo.man

visudo.man.in: $(srcdir)/visudo.pod
	@rm -f $(srcdir)/$@
	( cd $(srcdir); mansectsu=`echo @MANSECTSU@|tr A-Z a-z`; mansectform=`echo @MANSECTFORM@|tr A-Z a-z`; sed -n -e 1d -e '/^=pod/q' -e 's/^/.\\" /p' visudo.pod > $@; pod2man --quotes=none --date="`date '+%B %e, %Y'`" --section=$$mansectsu --release=$(VERSION) --center="MAINTENANCE COMMANDS" visudo.pod | sed -e "s/(5)/($$mansectform)/" -e "s/(8)/($$mansectsu)/" >> $@ )

visudo.man: visudo.man.in
	CONFIG_FILES=$@ CONFIG_HEADERS= sh ./config.status

visudo.cat: visudo.man

sudoers.man.in: $(srcdir)/sudoers.pod
	@rm -f $(srcdir)/$@
	( cd $(srcdir); mansectsu=`echo @MANSECTSU@|tr A-Z a-z`; mansectform=`echo @MANSECTFORM@|tr A-Z a-z`; sed -n -e 1d -e '/^=pod/q' -e 's/^/.\\" /p' sudoers.pod > $@; pod2man --quotes=none --date="`date '+%B %e, %Y'`" --section=$$mansectform --release=$(VERSION) --center="MAINTENANCE COMMANDS" sudoers.pod | sed -e "s/(5)/($$mansectform)/" -e "s/(8)/($$mansectsu)/" >> $@ )

sudoers.man:: sudoers.man.in
	CONFIG_FILES=$@ CONFIG_HEADERS= sh ./config.status

sudoers.cat: sudoers.man

install: install-dirs install-binaries install-sudoers install-man

install-dirs:
	$(SHELL) $(srcdir)/mkinstalldirs $(DESTDIR)$(sudodir) \
	    $(DESTDIR)$(visudodir) $(DESTDIR)$(sudoersdir) \
	    $(DESTDIR)$(mandirsu) $(DESTDIR)$(mandirform)

install-binaries: $(PROGS)
	$(INSTALL) -O $(install_uid) -G $(install_gid) -M 4111 -s sudo $(DESTDIR)$(sudodir)/sudo
	$(INSTALL) -O $(install_uid) -G $(install_gid) -M 0111 -s visudo $(DESTDIR)$(visudodir)/visudo

install-sudoers:
	test -f $(DESTDIR)$(sudoersdir)/sudoers || \
	    $(INSTALL) -O $(sudoers_uid) -G $(sudoers_gid) -M $(sudoers_mode) \
		$(srcdir)/sudoers $(DESTDIR)$(sudoersdir)/sudoers

install-man:
	$(INSTALL) -O $(install_uid) -G $(install_gid) -M 0444 ./sudo.$(mantype) $(DESTDIR)$(mandirsu)/sudo.$(mansectsu)
	$(INSTALL) -O $(install_uid) -G $(install_gid) -M 0444 ./visudo.$(mantype) $(DESTDIR)$(mandirsu)/visudo.$(mansectsu)
	$(INSTALL) -O $(install_uid) -G $(install_gid) -M 0444 ./sudoers.$(mantype) $(DESTDIR)$(mandirform)/sudoers.$(mansectform)


check:
	@echo nothing to check

tags: $(SRCS)
	ctags $(SRCS)

TAGS: $(SRCS)
	etags $(SRCS)

clean:
	-rm -f *.o $(PROGS) testsudoers core sudo.core visudo.core \
	       testsudoers.core

mostlyclean: clean

distclean: clean
	-rm -f Makefile *.man pathnames.h config.h config.status config.cache \
	       config.log $(PARSESRCS)

clobber: distclean

realclean: distclean
	rm -f TAGS tags

cleandir: realclean

dist:
	rm -f ../sudo-$(VERSION).tar.gz
	( cd .. ; TF="/tmp/sudo.dist$$$$" ; rm -f $$TF ; for i in $(DISTFILES) ; \
	  do echo sudo-$(VERSION)/$$i >> $$TF ; done ; \
	  tar Ocf sudo-$(VERSION).tar \
	  `cat $$TF` && gzip --best sudo-$(VERSION).tar && rm -f $$TF)
	ls -l ../sudo-$(VERSION).tar.gz

bindist:
	@mkdir tmp.`arch -l`
	@mkdir tmp.`arch -l`/sudo-$(VERSION)
	( \
	  tdir=tmp.`arch -l`/sudo-$(VERSION) ; \
	  for i in $(BINFILES) ; do \
	    if [ -f $$i ]; then \
	      cp $$i $$tdir ; \
	    elif [ -f $(srcdir)/$$i ]; then \
	      cp $(srcdir)/$$i $$tdir ; \
	    else \
	      echo cannot find $$i ; \
	      exit 1 ; \
	    fi ; \
	  done ; \
	  cp $(srcdir)/INSTALL.binary $$tdir/INSTALL ; \
	  sed -e 's/@_MANTYPE@/$(mantype)/g' -e 's/@_mansectsu@/$(mansectsu)/g' \
	      -e 's/@_mansectform@/$(mansectform)/g' $(srcdir)/Makefile.binary \
	      > $$tdir/Makefile ; \
	)  
	strip sudo
	strip visudo
	( cd tmp.`arch -l` && tar Ocf ../sudo-$(VERSION)-`arch -l`.tar sudo-$(VERSION) )
	gzip --best sudo-$(VERSION)-`arch -l`.tar
	rm -rf tmp.`arch -l`
