/*
 * zpty.c - sub-processes with pseudo terminals
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 2000 Sven Wischnowsky
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Sven Wischnowsky or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Sven Wischnowsky and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Sven Wischnowsky and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Sven Wischnowsky and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "zpty.mdh"
#include "zpty.pro"

/* The number of bytes we normally read when given no pattern and the
 * upper bound on the number of bytes we read (even if we are give a
 * pattern). */

#define READ_MAX (1024 * 1024)

typedef struct ptycmd *Ptycmd;

struct ptycmd {
    Ptycmd next;
    char *name;
    char **args;
    int fd;
    int pid;
    int echo;
    int nblock;
    int fin;
    int read;
    char *old;
    int olen;
};

static Ptycmd ptycmds;

static int
ptynonblock(int fd)
{
#ifdef O_NDELAY
# ifdef O_NONBLOCK
#  define NONBLOCK (O_NDELAY|O_NONBLOCK)
# else /* !O_NONBLOCK */
#  define NONBLOCK O_NDELAY
# endif /* !O_NONBLOCK */
#else /* !O_NDELAY */
# ifdef O_NONBLOCK
#  define NONBLOCK O_NONBLOCK
# else /* !O_NONBLOCK */
#  define NONBLOCK 0
# endif /* !O_NONBLOCK */
#endif /* !O_NDELAY */

#if NONBLOCK
    long mode;

    mode = fcntl(fd, F_GETFL, 0);
    if (mode != -1 && !(mode & NONBLOCK) &&
	!fcntl(fd, F_SETFL, mode | NONBLOCK))
	return 1;

#endif /* NONBLOCK */
    return 0;

#undef NONBLOCK
}

/**/
static int
ptygettyinfo(int fd, struct ttyinfo *ti)
{
    if (fd != -1) {
#ifdef HAVE_TERMIOS_H
# ifdef HAVE_TCGETATTR
	if (tcgetattr(fd, &ti->tio) == -1)
# else
	if (ioctl(fd, TCGETS, &ti->tio) == -1)
# endif
	    return 1;
#else
# ifdef HAVE_TERMIO_H
	ioctl(fd, TCGETA, &ti->tio);
# else
	ioctl(fd, TIOCGETP, &ti->sgttyb);
	ioctl(fd, TIOCLGET, &ti->lmodes);
	ioctl(fd, TIOCGETC, &ti->tchars);
	ioctl(fd, TIOCGLTC, &ti->ltchars);
# endif
#endif
	return 0;
    }
    return 1;
}

/**/
static void
ptysettyinfo(int fd, struct ttyinfo *ti)
{
    if (fd != -1) {
#ifdef HAVE_TERMIOS_H
# ifdef HAVE_TCGETATTR
#  ifndef TCSADRAIN
#   define TCSADRAIN 1	/* XXX Princeton's include files are screwed up */
#  endif
	tcsetattr(fd, TCSADRAIN, &ti->tio);
    /* if (tcsetattr(SHTTY, TCSADRAIN, &ti->tio) == -1) */
# else
	ioctl(fd, TCSETS, &ti->tio);
    /* if (ioctl(SHTTY, TCSETS, &ti->tio) == -1) */
# endif
	/*	zerr("settyinfo: %e",NULL,errno)*/ ;
#else
# ifdef HAVE_TERMIO_H
	ioctl(fd, TCSETA, &ti->tio);
# else
	ioctl(fd, TIOCSETN, &ti->sgttyb);
	ioctl(fd, TIOCLSET, &ti->lmodes);
	ioctl(fd, TIOCSETC, &ti->tchars);
	ioctl(fd, TIOCSLTC, &ti->ltchars);
# endif
#endif
    }
}

static Ptycmd
getptycmd(char *name)
{
    Ptycmd p;

    for (p = ptycmds; p; p = p->next)
	if (!strcmp(p->name, name))
	    return p;

    return NULL;
}

/**** maybe we should use configure here */
/**** and we certainly need more/better #if tests */

#if defined(__SVR4) || defined(sinix) || defined(__CYGWIN__)

#if !defined(__CYGWIN__)
#include <sys/stropts.h>
#endif

static int
get_pty(int master, int *retfd)
{
    static char *name;
    static int mfd, sfd;

    int ret;

    if (master) {
	if ((mfd = open("/dev/ptmx", O_RDWR|O_NOCTTY)) < 0)
	    return 1;

	if (grantpt(mfd) || unlockpt(mfd) || !(name = ptsname(mfd))) {
	    close(mfd);
	    return 1;
	}
	*retfd = mfd;

	return 0;
    }
    if ((sfd = open(name, O_RDWR
#ifndef __CYGWIN__
		    /* It is not clear whether this flag is actually needed. */
		    |O_NOCTTY
#endif
	)) < 0) {
	close(mfd);
	return 1;
    }
#if !defined(__CYGWIN__)
    if ((ret = ioctl(sfd, I_FIND, "ptem")) != 1)
       if (ret == -1 || ioctl(sfd, I_PUSH, "ptem") == -1) {
	   close(mfd);
	   close(sfd);
	   return 1;
       }
    if ((ret = ioctl(sfd, I_FIND, "ldterm")) != 1)
       if (ret == -1 || ioctl(sfd, I_PUSH, "ldterm") == -1) {
	   close(mfd);
	   close(sfd);
	   return 1;
       }
    if ((ret = ioctl(sfd, I_FIND, "ttcompat")) != 1)
       if (ret == -1 || ioctl(sfd, I_PUSH, "ttcompat") == -1) {
	   close(mfd);
	   close(sfd);
	   return 1;
       }
#endif /* !defined(__CYGWIN__) */

    *retfd = sfd;

    return 0;
}

#else /* ! (defined(__SVR4) || defined(sinix) || defined(__CYGWIN__)) */

static int
get_pty(int master, int *retfd)
{

#ifdef __linux
    static char char1[] = "abcdefghijklmnopqrstuvwxyz";
    static char char2[] = "0123456789abcdef";
#else /* __linux */
# ifdef __FreeBSD__
    static char char1[] = "pqrsPQRS";
    static char char2[] = "0123456789abcdefghijklmnopqrstuv";
# else /* __FreeBSD__ */
    static char char1[] = "pqrstuvwxyzPQRST";
    static char char2[] = "0123456789abcdef";
# endif /* __FreeBSD__ */
#endif /* __linux */

    static char name[11];
    static int mfd, sfd;
    char *p1, *p2;

    if (master) {
	strcpy(name, "/dev/ptyxx");

	for (p1 = char1; *p1; p1++) {
	    name[8] = *p1;
	    for (p2 = char2; *p2; p2++) {
		name[9] = *p2;
		if ((mfd = open(name, O_RDWR|O_NOCTTY)) >= 0) {
		    *retfd = mfd;

		    return 0;
		}
	    }
	}
    }
    name[5] = 't';
    if ((sfd = open(name, O_RDWR|O_NOCTTY)) >= 0) {
	*retfd = sfd;

	return 0;
    }
    close(mfd);

    return 1;
}

#endif /* __SVR4 */

static int
newptycmd(char *nam, char *pname, char **args, int echo, int nblock)
{
    Ptycmd p;
    int master, slave, pid;
    Eprog prog;

    prog = parse_string(zjoin(args, ' ', 1), 0);
    if (!prog) {
	errflag = 0;
	return 1;
    }

    if (get_pty(1, &master)) {
	zwarnnam(nam, "can't open pseudo terminal: %e", NULL, errno);
	return 1;
    }
    if ((pid = fork()) == -1) {
	zwarnnam(nam, "can't create pty command %s: %e", pname, errno);
	close(master);
	return 1;
    } else if (!pid) {
	/* This code copied from the clone module, except for getting *
	 * the descriptor from get_pty() and duplicating it to 0/1/2. */

	clearjobtab();
	ppid = getppid();
	mypid = getpid();
#ifdef HAVE_SETSID
	if (setsid() != mypid) {
	    zwarnnam(nam, "failed to create new session: %e", NULL, errno);
#endif
#ifdef TIOCNOTTY
	    if (ioctl(SHTTY, TIOCNOTTY, 0))
		zwarnnam(nam, "%e", NULL, errno);
	    setpgrp(0L, mypid);
#endif
#ifdef HAVE_SETSID
	}
#endif

	if (get_pty(0, &slave))
	    exit(1);
#ifdef TIOCGWINSZ
	/* Set the window size before associating with the terminal *
	 * so that we don't get hit with a SIGWINCH.  I'm paranoid. */
	if (interact) {
	    struct ttyinfo info;

	    if (ioctl(slave, TIOCGWINSZ, (char *) &info.winsize) == 0) {
		info.winsize.ws_row = lines;
		info.winsize.ws_col = columns;
		ioctl(slave, TIOCSWINSZ, (char *) &info.winsize);
	    }
	}
#endif /* TIOCGWINSZ */

	if (!echo) {
	    struct ttyinfo info;

	    if (!ptygettyinfo(slave, &info)) {
#ifdef HAVE_TERMIOS_H
		info.tio.c_lflag &= ~ECHO;
#else
#ifdef HAVE_TERMIO_H
		info.tio.c_lflag &= ~ECHO;
#else
		info.tio.lmodes &= ~ECHO; /**** dunno if this is right */
#endif
#endif
		ptysettyinfo(slave, &info);
	    }
	}

#ifdef TIOCSCTTY
	ioctl(slave, TIOCSCTTY, 0);
#endif

	close(0);
	close(1);
	close(2);

	dup2(slave, 0);
	dup2(slave, 1);
	dup2(slave, 2);

	closem(0);
	close(slave);
	close(master);
	close(coprocin);
	close(coprocout);
	init_io();
	setsparam("TTY", ztrdup(ttystrname));

	opts[INTERACTIVE] = 0;
	execode(prog, 1, 0);
	stopmsg = 2;
	zexit(lastval, 0);
    }
    master = movefd(master);

    p = (Ptycmd) zalloc(sizeof(*p));

    p->name = ztrdup(pname);
    p->args = zarrdup(args);
    p->fd = master;
    p->pid = pid;
    p->echo = echo;
    p->nblock = nblock;
    p->fin = 0;
    p->read = -1;
    p->old = NULL;
    p->olen = 0;

    p->next = ptycmds;
    ptycmds = p;

    if (nblock)
	ptynonblock(master);

    return 0;
}

static void
deleteptycmd(Ptycmd cmd)
{
    Ptycmd p, q;

    for (q = NULL, p = ptycmds; p != cmd; q = p, p = p->next);

    if (p != cmd)
	return;

    if (q)
	q->next = p->next;
    else
	ptycmds = p->next;

    zsfree(p->name);
    freearray(p->args);

    zclose(cmd->fd);

    /* We kill the process group the command put itself in. */

    kill(-(p->pid), SIGHUP);

    zfree(p, sizeof(*p));
}

static void
deleteallptycmds(void)
{
    Ptycmd p, n;

    for (p = ptycmds; p; p = n) {
	n = p->next;
	deleteptycmd(p);
    }
}

/**** a better process handling would be nice */

static void
checkptycmd(Ptycmd cmd)
{
    char c;
    int r;

    if (cmd->read != -1 || cmd->fin)
	return;
    if ((r = read(cmd->fd, &c, 1)) < 0) {
	if (kill(cmd->pid, 0) < 0) {
	    cmd->fin = 1;
	    zclose(cmd->fd);
	}
	return;
    }
    if (r) cmd->read = (int) c;
}

static int
ptyread(char *nam, Ptycmd cmd, char **args)
{
    int blen, used, seen = 0, ret = 0;
    char *buf;
    Patprog prog = NULL;

    if (*args && args[1]) {
	char *p;

	if (args[2]) {
	    zwarnnam(nam, "too many arguments", NULL, 0);
	    return 1;
	}
	p = dupstring(args[1]);
	tokenize(p);
	remnulargs(p);
	if (!(prog = patcompile(p, PAT_STATIC, NULL))) {
	    zwarnnam(nam, "bad pattern: %s", args[1], 0);
	    return 1;
	}
    } else
	fflush(stdout);

    if (cmd->old) {
	used = cmd->olen;
	buf = (char *) zhalloc((blen = 256 + used) + 1);
	memcpy(buf, cmd->old, cmd->olen);
	zfree(cmd->old, cmd->olen);
	cmd->old = NULL;
	cmd->olen = 0;
    } else {
	used = 0;
	buf = (char *) zhalloc((blen = 256) + 1);
    }
    if (cmd->read != -1) {
	buf[used] = (char) cmd->read;
	buf[used + 1] = '\0';
	seen = used = 1;
	cmd->read = -1;
    }
    do {
	if (!ret) {
	    checkptycmd(cmd);
	    if (cmd->fin)
		break;
	}
	if (cmd->read != -1 || (ret = read(cmd->fd, buf + used, 1)) == 1) {
	    if (cmd->read != -1) {
		ret = 1;
		buf[used] = (char) cmd->read;
		cmd->read = -1;
	    }
	    seen = 1;
	    if (++used == blen) {
		if (!*args) {
		    write(1, buf, used);
		    used = 0;
		} else {
		    buf = hrealloc(buf, blen, blen << 1);
		    blen <<= 1;
		}
	    }
	}
	buf[used] = '\0';

	if (!prog && (ret <= 0 || (*args && buf[used - 1] == '\n')))
	    break;
    } while (!(errflag || breaks || retflag || contflag) &&
	     used < READ_MAX && !(prog && ret && pattry(prog, buf)));

    if (prog && ret < 0 &&
#ifdef EWOULDBLOCK
	errno == EWOULDBLOCK
#else
#ifdef EAGAIN
	errno == EAGAIN
#endif
#endif
	) {
	cmd->old = (char *) zalloc(cmd->olen = used);
	memcpy(cmd->old, buf, cmd->olen);

	return 1;
    }
    if (*args)
	setsparam(*args, ztrdup(metafy(buf, used, META_HREALLOC)));
    else if (used)
	write(1, buf, used);

    return (seen ? 0 : cmd->fin + 1);
}

static int
ptywritestr(Ptycmd cmd, char *s, int len)
{
    int written, all = 0;

    for (; !errflag && !breaks && !retflag && !contflag && len;
	 len -= written, s += written) {
	if ((written = write(cmd->fd, s, len)) < 0 && cmd->nblock &&
#ifdef EWOULDBLOCK
	    errno == EWOULDBLOCK
#else
#ifdef EAGAIN
	    errno == EAGAIN
#endif
#endif
	    )
	    return !all;
	if (written < 0) {
	    checkptycmd(cmd);
	    if (cmd->fin)
		break;
	    written = 0;
	}
	if (written > 0)
	    all += written;
    }
    return (all ? 0 : cmd->fin + 1);
}

static int
ptywrite(Ptycmd cmd, char **args, int nonl)
{
    if (*args) {
	char sp = ' ', *tmp;
	int len;

	while (*args) {
	    unmetafy((tmp = dupstring(*args)), &len);
	    if (ptywritestr(cmd, tmp, len) ||
		(*++args && ptywritestr(cmd, &sp, 1)))
		return 1;
	}
	if (!nonl) {
	    sp = '\n';
	    if (ptywritestr(cmd, &sp, 1))
		return 1;
	}
    } else {
	int n;
	char buf[BUFSIZ];

	while ((n = read(0, buf, BUFSIZ)) > 0)
	    if (ptywritestr(cmd, buf, n))
		return 1;
    }
    return 0;
}

/**/
static int
bin_zpty(char *nam, char **args, char *ops, int func)
{
    if ((ops['r'] && ops['w']) ||
	((ops['r'] || ops['w']) && (ops['d'] || ops['e'] ||
				    ops['b'] || ops['L'])) ||
	(ops['w'] && ops['t']) ||
	(ops['n'] && (ops['b'] || ops['e'] || ops['r'] || ops['t'] ||
		      ops['d'] || ops['L'])) ||
	(ops['d'] && (ops['b'] || ops['e'] || ops['L'] || ops['t'])) ||
	(ops['L'] && (ops['b'] || ops['e']))) {
	zwarnnam(nam, "illegal option combination", NULL, 0);
	return 1;
    }
    if (ops['r'] || ops['w']) {
	Ptycmd p;

	if (!*args) {
	    zwarnnam(nam, "missing pty command name", NULL, 0);
	    return 1;
	} else if (!(p = getptycmd(*args))) {
	    zwarnnam(nam, "no such pty command: %s", *args, 0);
	    return 1;
	}
	if (p->fin)
	    return 2;
	if (ops['t'] && p->read == -1 && !read_poll(p->fd, &p->read, 0))
	    return 1;

	return (ops['r'] ?
		ptyread(nam, p, args + 1) :
		ptywrite(p, args + 1, ops['n']));
    } else if (ops['d']) {
	Ptycmd p;
	int ret = 0;

	if (*args) {
	    while (*args)
		if ((p = getptycmd(*args++)))
		    deleteptycmd(p);
		else {
		    zwarnnam(nam, "no such pty command: %s", args[-1], 0);
		    ret = 1;
		}
	} else
	    deleteallptycmds();

	return ret;
    } else if (ops['t']) {
	Ptycmd p;

	if (!*args) {
	    zwarnnam(nam, "missing pty command name", NULL, 0);
	    return 1;
	} else if (!(p = getptycmd(*args))) {
	    zwarnnam(nam, "no such pty command: %s", *args, 0);
	    return 1;
	}
	checkptycmd(p);
	return p->fin;
    } else if (*args) {
	if (!args[1]) {
	    zwarnnam(nam, "missing command", NULL, 0);
	    return 1;
	}
	if (getptycmd(*args)) {
	    zwarnnam(nam, "pty command name already used: %s", *args, 0);
	    return 1;
	}
	return newptycmd(nam, *args, args + 1, ops['e'], ops['b']);
    } else {
	Ptycmd p;
	char **a;

	for (p = ptycmds; p; p = p->next) {
	    checkptycmd(p);
	    if (ops['L'])
		printf("%s %s%s%s ", nam, (p->echo ? "-e " : ""),
		       (p->nblock ? "-b " : ""), p->name);
	    else if (p->fin)
		printf("(finished) %s: ", p->name);
	    else
		printf("(%d) %s: ", p->pid, p->name);
	    for (a = p->args; *a; ) {
		quotedzputs(*a++, stdout);
		if (*a)
		    putchar(' ');
	    }
	    putchar('\n');
	}
	return 0;
    }
}

static int
ptyhook(Hookdef d, void *dummy)
{
    deleteallptycmds();
    return 0;
}

static struct builtin bintab[] = {
    BUILTIN("zpty", 0, bin_zpty, 0, -1, 0, "ebdrwLnt", NULL),
};

/**/
int
setup_(Module m)
{
    return 0;
}

/**/
int
boot_(Module m)
{
    ptycmds = NULL;

    addhookfunc("exit", ptyhook);
    return !addbuiltins(m->nam, bintab, sizeof(bintab)/sizeof(*bintab));
}

/**/
int
cleanup_(Module m)
{
    deletehookfunc("exit", ptyhook);
    deleteallptycmds();
    deletebuiltins(m->nam, bintab, sizeof(bintab)/sizeof(*bintab));
    return 0;
}

/**/
int
finish_(Module m)
{
    return 0;
}
