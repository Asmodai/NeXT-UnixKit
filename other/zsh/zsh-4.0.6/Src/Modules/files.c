/*
 * files.c - file operation builtins
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1996-1997 Andrew Main
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Andrew Main or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Andrew Main and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Andrew Main and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Andrew Main and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "files.mdh"

typedef int (*MoveFunc) _((char const *, char const *));
typedef int (*RecurseFunc) _((char *, char *, struct stat const *, void *));

#ifndef STDC_HEADERS
extern int link _((const char *, const char *));
extern int symlink _((const char *, const char *));
extern int rename _((const char *, const char *));
#endif

struct recursivecmd;

#include "files.pro"

/**/
static int
ask(void)
{
    int a = getchar(), c;
    for(c = a; c != EOF && c != '\n'; )
	c = getchar();
    return a == 'y' || a == 'Y';
}

/* sync builtin */

/**/
static int
bin_sync(char *nam, char **args, char *ops, int func)
{
    sync();
    return 0;
}

/* mkdir builtin */

/**/
static int
bin_mkdir(char *nam, char **args, char *ops, int func)
{
    mode_t oumask = umask(0);
    mode_t mode = 0777 & ~oumask;
    int err = 0;

    umask(oumask);
    if(ops['m']) {
	char *str = *args++, *ptr;

	if(!*args) {
	    zwarnnam(nam, "not enough arguments", NULL, 0);
	    return 1;
	}
	mode = zstrtol(str, &ptr, 8);
	if(!*str || *ptr) {
	    zwarnnam(nam, "invalid mode `%s'", str, 0);
	    return 1;
	}
    }
    for(; *args; args++) {
	char *ptr = strchr(*args, 0);

	while(ptr > *args + (**args == '/') && *--ptr == '/')
	    *ptr = 0;
	if(ops['p']) {
	    char *ptr = *args;

	    for(;;) {
		while(*ptr == '/')
		    ptr++;
		while(*ptr && *ptr != '/')
		    ptr++;
		if(!*ptr) {
		    err |= domkdir(nam, *args, mode, 1);
		    break;
		} else {
		    int e;

		    *ptr = 0;
		    e = domkdir(nam, *args, mode | 0300, 1);
		    if(e) {
			err = 1;
			break;
		    }
		    *ptr = '/';
		}
	    }
	} else
	    err |= domkdir(nam, *args, mode, 0);
    }
    return err;
}

/**/
static int
domkdir(char *nam, char *path, mode_t mode, int p)
{
    int err;
    mode_t oumask;
    char const *rpath = unmeta(path);

    if(p) {
	struct stat st;

	if(!lstat(rpath, &st) && S_ISDIR(st.st_mode))
	    return 0;
    }
    oumask = umask(0);
    err = mkdir(path, mode) ? errno : 0;
    umask(oumask);
    if(!err)
	return 0;
    zwarnnam(nam, "cannot make directory `%s': %e", path, err);
    return 1;
}

/* rmdir builtin */

/**/
static int
bin_rmdir(char *nam, char **args, char *ops, int func)
{
    int err = 0;

    for(; *args; args++) {
	char *rpath = unmeta(*args);

	if(!rpath) {
	    zwarnnam(nam, "%s: %e", *args, ENAMETOOLONG);
	    err = 1;
	} else if(rmdir(rpath)) {
	    zwarnnam(nam, "cannot remove directory `%s': %e", *args, errno);
	    err = 1;
	}
    }
    return err;
}

/* ln and mv builtins */

#define BIN_LN 0
#define BIN_MV 1

#define MV_NODIRS (1<<0)
#define MV_FORCE  (1<<1)
#define MV_INTER  (1<<2)
#define MV_ASKNW  (1<<3)
#define MV_ATOMIC (1<<4)

/* bin_ln actually does three related jobs: hard linking, symbolic *
 * linking, and renaming.  If called as mv it renames, otherwise   *
 * it looks at the -s option.  If hard linking, it will refuse to  *
 * attempt linking to a directory unless the -d option is given.   */

/**/
static int
bin_ln(char *nam, char **args, char *ops, int func)
{
    MoveFunc move;
    int flags, err = 0;
    char **a, *ptr, *rp, *buf;
    struct stat st;
    size_t blen;


    if(func == BIN_MV) {
	move = (MoveFunc) rename;
	flags = ops['f'] ? 0 : MV_ASKNW;
	flags |= MV_ATOMIC;
    } else {
	flags = ops['f'] ? MV_FORCE : 0;
#ifdef HAVE_LSTAT
	if(ops['s'])
	    move = (MoveFunc) symlink;
	else
#endif
	{
	    move = (MoveFunc) link;
	    if(!ops['d'])
		flags |= MV_NODIRS;
	}
    }
    if(ops['i'] && !ops['f'])
	flags |= MV_INTER;
    for(a = args; a[1]; a++) ;
    if(a != args) {
	rp = unmeta(*a);
	if(rp && !stat(rp, &st) && S_ISDIR(st.st_mode))
	    goto havedir;
    }
    if(a > args+1) {
	zwarnnam(nam, "last of many arguments must be a directory", NULL, 0);
	return 1;
    }
    if(!args[1]) {
	ptr = strrchr(args[0], '/');
	if(ptr)
	    args[1] = ptr+1;
	else
	    args[1] = args[0];
    }
    return domove(nam, move, args[0], args[1], flags);
 havedir:
    buf = ztrdup(*a);
    *a = NULL;
    buf = appstr(buf, "/");
    blen = strlen(buf);
    for(; *args; args++) {

	ptr = strrchr(*args, '/');
	if(ptr)
	    ptr++;
	else
	    ptr = *args;

	buf[blen] = 0;
	buf = appstr(buf, ptr);
	err |= domove(nam, move, *args, buf, flags);
    }
    zsfree(buf);
    return err;
}

/**/
static int
domove(char *nam, MoveFunc move, char *p, char *q, int flags)
{
    struct stat st;
    char *pbuf, *qbuf;

    pbuf = ztrdup(unmeta(p));
    qbuf = unmeta(q);
    if(flags & MV_NODIRS) {
	errno = EISDIR;
	if(lstat(pbuf, &st) || S_ISDIR(st.st_mode)) {
	    zwarnnam(nam, "%s: %e", p, errno);
	    zsfree(pbuf);
	    return 1;
	}
    }
    if(!lstat(qbuf, &st)) {
	int doit = flags & MV_FORCE;
	if(S_ISDIR(st.st_mode)) {
	    zwarnnam(nam, "%s: cannot overwrite directory", q, 0);
	    zsfree(pbuf);
	    return 1;
	} else if(flags & MV_INTER) {
	    nicezputs(nam, stderr);
	    fputs(": replace `", stderr);
	    nicezputs(q, stderr);
	    fputs("'? ", stderr);
	    fflush(stderr);
	    if(!ask()) {
		zsfree(pbuf);
		return 0;
	    }
	    doit = 1;
	} else if((flags & MV_ASKNW) &&
		!S_ISLNK(st.st_mode) &&
		access(qbuf, W_OK)) {
	    nicezputs(nam, stderr);
	    fputs(": replace `", stderr);
	    nicezputs(q, stderr);
	    fprintf(stderr, "', overriding mode %04o? ",
		mode_to_octal(st.st_mode));
	    fflush(stderr);
	    if(!ask()) {
		zsfree(pbuf);
		return 0;
	    }
	    doit = 1;
	}
	if(doit && !(flags & MV_ATOMIC))
	    unlink(qbuf);
    }
    if(move(pbuf, qbuf)) {
	zwarnnam(nam, "%s: %e", p, errno);
	zsfree(pbuf);
	return 1;
    }
    zsfree(pbuf);
    return 0;
}

/* general recursion */

struct recursivecmd {
    char *nam;
    int opt_noerr;
    int opt_recurse;
    int opt_safe;
    RecurseFunc dirpre_func;
    RecurseFunc dirpost_func;
    RecurseFunc leaf_func;
    void *magic;
};

/**/
static int
recursivecmd(char *nam, int opt_noerr, int opt_recurse, int opt_safe,
    char **args, RecurseFunc dirpre_func, RecurseFunc dirpost_func,
    RecurseFunc leaf_func, void *magic)
{
    int err = 0, len;
    char *rp, *s;
    struct dirsav ds;
    struct recursivecmd reccmd;

    reccmd.nam = nam;
    reccmd.opt_noerr = opt_noerr;
    reccmd.opt_recurse = opt_recurse;
    reccmd.opt_safe = opt_safe;
    reccmd.dirpre_func = dirpre_func;
    reccmd.dirpost_func = dirpost_func;
    reccmd.leaf_func = leaf_func;
    reccmd.magic = magic;
    ds.ino = ds.dev = 0;
    ds.dirname = NULL;
    ds.dirfd = ds.level = -1;
    if (opt_recurse || opt_safe) {
	if ((ds.dirfd = open(".", O_RDONLY|O_NOCTTY)) < 0 &&
	    zgetdir(&ds) && *ds.dirname != '/')
	    ds.dirfd = open("..", O_RDONLY|O_NOCTTY);
    }
    for(; !errflag && !(err & 2) && *args; args++) {
	rp = ztrdup(*args);
	unmetafy(rp, &len);
	if (opt_safe) {
	    s = strrchr(rp, '/');
	    if (s && !s[1]) {
		while (*s == '/' && s > rp)
		    *s-- = '\0';
		while (*s != '/' && s > rp)
		    s--;
	    }
	    if (s && s[1]) {
		int e;

		*s = '\0';
		e = lchdir(s > rp ? rp : "/", &ds, 1);
		err |= -e;
		if (!e) {
		    struct dirsav d;

		    d.ino = d.dev = 0;
		    d.dirname = NULL;
		    d.dirfd = d.level = -1;
		    err |= recursivecmd_doone(&reccmd, *args, s + 1, &d, 0);
		    zsfree(d.dirname);
		    if (restoredir(&ds))
			err |= 2;
		} else if(!opt_noerr)
		    zwarnnam(nam, "%s: %e", *args, errno);
	    } else
		err |= recursivecmd_doone(&reccmd, *args, rp, &ds, 0);
	} else
	    err |= recursivecmd_doone(&reccmd, *args, rp, &ds, 1);
	zfree(rp, len + 1);
    }
    if ((err & 2) && ds.dirfd >= 0 && restoredir(&ds) && zchdir(pwd)) {
	zsfree(pwd);
	pwd = ztrdup("/");
	chdir(pwd);
    }
    if (ds.dirfd >= 0)
	close(ds.dirfd);
    zsfree(ds.dirname);
    return !!err;
}

/**/
static int
recursivecmd_doone(struct recursivecmd const *reccmd,
    char *arg, char *rp, struct dirsav *ds, int first)
{
    struct stat st, *sp = NULL;

    if(reccmd->opt_recurse && !lstat(rp, &st)) {
	if(S_ISDIR(st.st_mode))
	    return recursivecmd_dorec(reccmd, arg, rp, &st, ds, first);
	sp = &st;
    }
    return reccmd->leaf_func(arg, rp, sp, reccmd->magic);
}

/**/
static int
recursivecmd_dorec(struct recursivecmd const *reccmd,
    char *arg, char *rp, struct stat const *sp, struct dirsav *ds, int first)
{
    char *fn;
    DIR *d;
    int err, err1;
    struct dirsav dsav;
    char *files = NULL;
    int fileslen = 0;

    err1 = reccmd->dirpre_func(arg, rp, sp, reccmd->magic);
    if(err1 & 2)
	return 2;

    err = -lchdir(rp, ds, !first);
    if (err) {
	if(!reccmd->opt_noerr)
	    zwarnnam(reccmd->nam, "%s: %e", arg, errno);
	return err;
    }
    err = err1;

    dsav.ino = dsav.dev = 0;
    dsav.dirname = NULL;
    dsav.dirfd = dsav.level = -1;
    d = opendir(".");
    if(!d) {
	if(!reccmd->opt_noerr)
	    zwarnnam(reccmd->nam, "%s: %e", arg, errno);
	err = 1;
    } else {
	int arglen = strlen(arg) + 1;

	while (!errflag && (fn = zreaddir(d, 1))) {
	    int l = strlen(fn) + 1;
	    files = hrealloc(files, fileslen, fileslen + l);
	    strcpy(files + fileslen, fn);
	    fileslen += l;
	}
	closedir(d);
	for (fn = files; !errflag && !(err & 2) && fn < files + fileslen;) {
	    int l = strlen(fn) + 1;
	    VARARR(char, narg, arglen + l);

	    strcpy(narg,arg);
	    narg[arglen-1] = '/';
	    strcpy(narg + arglen, fn);
	    unmetafy(fn, NULL);
	    err |= recursivecmd_doone(reccmd, narg, fn, &dsav, 0);
	    fn += l;
	}
	hrealloc(files, fileslen, 0);
    }
    zsfree(dsav.dirname);
    if (err & 2)
	return 2;
    if (restoredir(ds)) {
	if(!reccmd->opt_noerr)
	    zwarnnam(reccmd->nam, "failed to return to previous directory: %e",
		     NULL, errno);
	return 2;
    }
    return err | reccmd->dirpost_func(arg, rp, sp, reccmd->magic);
}

/**/
static int
recurse_donothing(char *arg, char *rp, struct stat const *sp, void *magic)
{
    return 0;
}

/* rm builtin */

struct rmmagic {
    char *nam;
    int opt_force;
    int opt_interact;
    int opt_unlinkdir;
};

/**/
static int
rm_leaf(char *arg, char *rp, struct stat const *sp, void *magic)
{
    struct rmmagic *rmm = magic;
    struct stat st;

    if(!rmm->opt_unlinkdir || !rmm->opt_force) {
	if(!sp) {
	    if(!lstat(rp, &st))
		sp = &st;
	}
	if(sp) {
	    if(!rmm->opt_unlinkdir && S_ISDIR(sp->st_mode)) {
		if(rmm->opt_force)
		    return 0;
		zwarnnam(rmm->nam, "%s: %e", arg, EISDIR);
		return 1;
	    }
	    if(rmm->opt_interact) {
		nicezputs(rmm->nam, stderr);
		fputs(": remove `", stderr);
		nicezputs(arg, stderr);
		fputs("'? ", stderr);
		fflush(stderr);
		if(!ask())
		    return 0;
	    } else if(!rmm->opt_force &&
		    !S_ISLNK(sp->st_mode) &&
		    access(rp, W_OK)) {
		nicezputs(rmm->nam, stderr);
		fputs(": remove `", stderr);
		nicezputs(arg, stderr);
		fprintf(stderr, "', overriding mode %04o? ",
		    mode_to_octal(sp->st_mode));
		fflush(stderr);
		if(!ask())
		    return 0;
	    }
	}
    }
    if(unlink(rp) && !rmm->opt_force) {
	zwarnnam(rmm->nam, "%s: %e", arg, errno);
	return 1;
    }
    return 0;
}

/**/
static int
rm_dirpost(char *arg, char *rp, struct stat const *sp, void *magic)
{
    struct rmmagic *rmm = magic;

    if(rmm->opt_interact) {
	nicezputs(rmm->nam, stderr);
	fputs(": remove `", stderr);
	nicezputs(arg, stderr);
	fputs("'? ", stderr);
	fflush(stderr);
	if(!ask())
	    return 0;
    }
    if(rmdir(rp) && !rmm->opt_force) {
	zwarnnam(rmm->nam, "%s: %e", arg, errno);
	return 1;
    }
    return 0;
}

/**/
static int
bin_rm(char *nam, char **args, char *ops, int func)
{
    struct rmmagic rmm;
    int err;

    rmm.nam = nam;
    rmm.opt_force = ops['f'];
    rmm.opt_interact = ops['i'] && !ops['f'];
    rmm.opt_unlinkdir = ops['d'];
    err = recursivecmd(nam, ops['f'], ops['r'] && !ops['d'], ops['s'],
	args, recurse_donothing, rm_dirpost, rm_leaf, &rmm);
    return ops['f'] ? 0 : err;
}

/* chown builtin */

struct chownmagic {
    char *nam;
    uid_t uid;
    gid_t gid;
};

/**/
static int
chown_dochown(char *arg, char *rp, struct stat const *sp, void *magic)
{
    struct chownmagic *chm = magic;

    if(lchown(rp, chm->uid, chm->gid)) {
	zwarnnam(chm->nam, "%s: %e", arg, errno);
	return 1;
    }
    return 0;
}

/**/
static unsigned long getnumeric(char *p, int *errp)
{
    unsigned long ret;

    if(*p < '0' || *p > '9') {
	*errp = 1;
	return 0;
    }
    ret = strtoul(p, &p, 10);
    *errp = !!*p;
    return ret;
}

enum { BIN_CHOWN, BIN_CHGRP };

/**/
static int
bin_chown(char *nam, char **args, char *ops, int func)
{
    struct chownmagic chm;
    char *uspec = ztrdup(*args), *p = uspec;
    char *end;

    chm.nam = nam;
    if(func == BIN_CHGRP) {
	chm.uid = -1;
	goto dogroup;
    }
    end = strchr(uspec, ':');
    if(!end)
	end = strchr(uspec, '.');
    if(end == uspec) {
	chm.uid = -1;
	p++;
	goto dogroup;
    } else {
	struct passwd *pwd;
	if(end)
	    *end = 0;
	pwd = getpwnam(p);
	if(pwd)
	    chm.uid = pwd->pw_uid;
	else {
	    int err;
	    chm.uid = getnumeric(p, &err);
	    if(err) {
		zwarnnam(nam, "%s: no such user", p, 0);
		free(uspec);
		return 1;
	    }
	}
	if(end) {
	    p = end+1;
	    if(!*p) {
		if(!pwd && !(pwd = getpwuid(chm.uid))) {
		    zwarnnam(nam, "%s: no such user", uspec, 0);
		    free(uspec);
		    return 1;
		}
		chm.gid = pwd->pw_gid;
	    } else if(p[0] == ':' && !p[1]) {
		chm.gid = -1;
	    } else {
		struct group *grp;
		dogroup:
		grp = getgrnam(p);
		if(grp)
		    chm.gid = grp->gr_gid;
		else {
		    int err;
		    chm.gid = getnumeric(p, &err);
		    if(err) {
			zwarnnam(nam, "%s: no such group", p, 0);
			free(uspec);
			return 1;
		    }
		}
	    }
	 } else
	    chm.gid = -1;
    }
    free(uspec);
    return recursivecmd(nam, 0, ops['R'], ops['s'],
	args + 1, chown_dochown, recurse_donothing, chown_dochown, &chm);
}

/* module paraphernalia */

#ifdef HAVE_LSTAT
# define LN_OPTS "dfis"
#else
# define LN_OPTS "dfi"
#endif

static struct builtin bintab[] = {
    BUILTIN("chgrp", 0, bin_chown, 2, -1, BIN_CHGRP, "Rs",    NULL),
    BUILTIN("chown", 0, bin_chown, 2, -1, BIN_CHOWN, "Rs",    NULL),
    BUILTIN("ln",    0, bin_ln,    1, -1, BIN_LN,    LN_OPTS, NULL),
    BUILTIN("mkdir", 0, bin_mkdir, 1, -1, 0,         "pm",    NULL),
    BUILTIN("mv",    0, bin_ln,    2, -1, BIN_MV,    "fi",    NULL),
    BUILTIN("rm",    0, bin_rm,    1, -1, 0,         "dfirs", NULL),
    BUILTIN("rmdir", 0, bin_rmdir, 1, -1, 0,         NULL,    NULL),
    BUILTIN("sync",  0, bin_sync,  0,  0, 0,         NULL,    NULL),
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
    return !addbuiltins(m->nam, bintab, sizeof(bintab)/sizeof(*bintab));
}

/**/
int
cleanup_(Module m)
{
    deletebuiltins(m->nam, bintab, sizeof(bintab)/sizeof(*bintab));
    return 0;
}

/**/
int
finish_(Module m)
{
    return 0;
}
