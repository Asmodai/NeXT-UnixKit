/*
 * cond.c - evaluate conditional expressions
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "zsh.mdh"
#include "cond.pro"

int tracingcond;

static char *condstr[COND_MOD] = {
    "!", "&&", "||", "==", "!=", "<", ">", "-nt", "-ot", "-ef", "-eq",
    "-ne", "-lt", "-gt", "-le", "-ge"
};

/**/
int
evalcond(Estate state)
{
    struct stat *st;
    char *left, *right;
    Wordcode pcode;
    wordcode code;
    int ctype, htok = 0;

 rec:

    left = right = NULL;
    pcode = state->pc++;
    code = *pcode;
    ctype = WC_COND_TYPE(code);

    switch (ctype) {
    case COND_NOT:
	if (tracingcond)
	    fprintf(xtrerr, " %s", condstr[ctype]);
	return !evalcond(state);
    case COND_AND:
	if (evalcond(state)) {
	    if (tracingcond)
		fprintf(xtrerr, " %s", condstr[ctype]);
	    goto rec;
	} else {
	    state->pc = pcode + (WC_COND_SKIP(code) + 1);
	    return 0;
	}
    case COND_OR:
	if (!evalcond(state)) {
	    if (tracingcond)
		fprintf(xtrerr, " %s", condstr[ctype]);
	    goto rec;
	} else {
	    state->pc = pcode + (WC_COND_SKIP(code) + 1);
	    return 1;
	}
    case COND_MOD:
    case COND_MODI:
	{
	    Conddef cd;
	    char *name = ecgetstr(state, EC_NODUP, NULL), **strs;
	    int l = WC_COND_SKIP(code);

	    if (ctype == COND_MOD)
		strs = ecgetarr(state, l, EC_DUP, NULL);
	    else {
		char *sbuf[3];

		sbuf[0] = ecgetstr(state, EC_NODUP, NULL);
		sbuf[1] = ecgetstr(state, EC_NODUP, NULL);
		sbuf[2] = NULL;

		strs = arrdup(sbuf);
		l = 2;
	    }
	    if ((cd = getconddef((ctype == COND_MODI), name + 1, 1))) {
		if (ctype == COND_MOD &&
		    (l < cd->min || (cd->max >= 0 && l > cd->max))) {
		    zerr("unrecognized condition: `%s'", name, 0);
		    return 0;
		}
		if (tracingcond)
		    tracemodcond(name, strs, ctype == COND_MODI);
		return cd->handler(strs, cd->condid);
	    }
	    else {
		char *s = strs[0];

		strs[0] = dupstring(name);
		name = s;

		if (name && name[0] == '-' &&
		    (cd = getconddef(0, name + 1, 1))) {
		    if (l < cd->min || (cd->max >= 0 && l > cd->max)) {
			zerr("unrecognized condition: `%s'", name, 0);
			return 0;
		    }
		    if (tracingcond)
			tracemodcond(name, strs, ctype == COND_MODI);
		    return cd->handler(strs, cd->condid);
		} else
		    zerr("unrecognized condition: `%s'", name, 0);
	    }
	    return 0;
	}
    }
    left = ecgetstr(state, EC_DUPTOK, &htok);
    if (htok) {
	singsub(&left);
	untokenize(left);
    }
    if (ctype <= COND_GE && ctype != COND_STREQ && ctype != COND_STRNEQ) {
	right = ecgetstr(state, EC_DUPTOK, &htok);
	if (htok) {
	    singsub(&right);
	    untokenize(right);
	}
    }
    if (tracingcond) {
	if (ctype < COND_MOD) {
	    char *rt = (char *) right;
	    if (ctype == COND_STREQ || ctype == COND_STRNEQ) {
		rt = dupstring(ecrawstr(state->prog, state->pc, NULL));
		singsub(&rt);
		untokenize(rt);
	    }
	    fprintf(xtrerr, " %s %s %s", left, condstr[ctype], rt);
	} else
	    fprintf(xtrerr, " -%c %s", ctype, left);
    }

    if (ctype >= COND_EQ && ctype <= COND_GE) {
	mnumber mn1, mn2;
	mn1 = matheval(left);
	mn2 = matheval(right);

	if (((mn1.type|mn2.type) & (MN_INTEGER|MN_FLOAT)) ==
	    (MN_INTEGER|MN_FLOAT)) {
	    /* promote to float */
	    if (mn1.type & MN_INTEGER) {
		mn1.type = MN_FLOAT;
		mn1.u.d = (double)mn1.u.l;
	    }
	    if (mn2.type & MN_INTEGER) {
		mn2.type = MN_FLOAT;
		mn2.u.d = (double)mn2.u.l;
	    }
	}
	switch(ctype) {
	case COND_EQ:
	    return (mn1.type & MN_FLOAT) ? (mn1.u.d == mn2.u.d) :
		(mn1.u.l == mn2.u.l);
	case COND_NE:
	    return (mn1.type & MN_FLOAT) ? (mn1.u.d != mn2.u.d) :
		(mn1.u.l != mn2.u.l);
	case COND_LT:
	    return (mn1.type & MN_FLOAT) ? (mn1.u.d < mn2.u.d) :
		(mn1.u.l < mn2.u.l);
	case COND_GT:
	    return (mn1.type & MN_FLOAT) ? (mn1.u.d > mn2.u.d) :
		(mn1.u.l > mn2.u.l);
	case COND_LE:
	    return (mn1.type & MN_FLOAT) ? (mn1.u.d <= mn2.u.d) :
		(mn1.u.l <= mn2.u.l);
	case COND_GE:
	    return (mn1.type & MN_FLOAT) ? (mn1.u.d >= mn2.u.d) :
		(mn1.u.l >= mn2.u.l);
	}
    }

    switch (ctype) {
    case COND_STREQ:
    case COND_STRNEQ:
	{
	    int test, npat = state->pc[1];
	    Patprog pprog = state->prog->pats[npat];

	    if (pprog == dummy_patprog1 || pprog == dummy_patprog2) {
		char *opat;
		int save;

		right = dupstring(opat = ecrawstr(state->prog, state->pc,
						  &htok));
		if (htok)
		    singsub(&right);
		save = (!(state->prog->flags & EF_HEAP) &&
			!strcmp(opat, right) && pprog != dummy_patprog2);

		if (!(pprog = patcompile(right, (save ? PAT_ZDUP : PAT_STATIC),
					 NULL)))
		    zerr("bad pattern: %s", right, 0);
		else if (save)
		    state->prog->pats[npat] = pprog;
	    }
	    state->pc += 2;
	    test = (pprog && pattry(pprog, left));

	    return (ctype == COND_STREQ ? test : !test);
	}
    case COND_STRLT:
	return strcmp(left, right) < 0;
    case COND_STRGTR:
	return strcmp(left, right) > 0;
    case 'e':
    case 'a':
	return (doaccess(left, F_OK));
    case 'b':
	return (S_ISBLK(dostat(left)));
    case 'c':
	return (S_ISCHR(dostat(left)));
    case 'd':
	return (S_ISDIR(dostat(left)));
    case 'f':
	return (S_ISREG(dostat(left)));
    case 'g':
	return (!!(dostat(left) & S_ISGID));
    case 'k':
	return (!!(dostat(left) & S_ISVTX));
    case 'n':
	return (!!strlen(left));
    case 'o':
	return (optison(left));
    case 'p':
	return (S_ISFIFO(dostat(left)));
    case 'r':
	return (doaccess(left, R_OK));
    case 's':
	return ((st = getstat(left)) && !!(st->st_size));
    case 'S':
	return (S_ISSOCK(dostat(left)));
    case 'u':
	return (!!(dostat(left) & S_ISUID));
    case 'w':
	return (doaccess(left, W_OK));
    case 'x':
	if (privasserted()) {
	    mode_t mode = dostat(left);
	    return (mode & S_IXUGO) || S_ISDIR(mode);
	}
	return doaccess(left, X_OK);
    case 'z':
	return (!strlen(left));
    case 'h':
    case 'L':
	return (S_ISLNK(dolstat(left)));
    case 'O':
	return ((st = getstat(left)) && st->st_uid == geteuid());
    case 'G':
	return ((st = getstat(left)) && st->st_gid == getegid());
    case 'N':
	return ((st = getstat(left)) && st->st_atime <= st->st_mtime);
    case 't':
	return isatty(mathevali(left));
    case COND_NT:
    case COND_OT:
	{
	    time_t a;

	    if (!(st = getstat(left)))
		return 0;
	    a = st->st_mtime;
	    if (!(st = getstat(right)))
		return 0;
	    return (ctype == COND_NT) ? a > st->st_mtime : a < st->st_mtime;
	}
    case COND_EF:
	{
	    dev_t d;
	    ino_t i;

	    if (!(st = getstat(left)))
		return 0;
	    d = st->st_dev;
	    i = st->st_ino;
	    if (!(st = getstat(right)))
		return 0;
	    return d == st->st_dev && i == st->st_ino;
	}
    default:
	zerr("bad cond code", NULL, 0);
    }
    return 0;
}


/**/
static int
doaccess(char *s, int c)
{
#ifdef HAVE_FACCESSX
    if (!strncmp(s, "/dev/fd/", 8))
	return !faccessx(atoi(s + 8), c, ACC_SELF);
#endif
    return !access(unmeta(s), c);
}


static struct stat st;

/**/
static struct stat *
getstat(char *s)
{
    char *us;

/* /dev/fd/n refers to the open file descriptor n.  We always use fstat *
 * in this case since on Solaris /dev/fd/n is a device special file     */
    if (!strncmp(s, "/dev/fd/", 8)) {
	if (fstat(atoi(s + 8), &st))
	    return NULL;
        return &st;
    }

    if (!(us = unmeta(s)))
        return NULL;
    if (stat(us, &st))
	return NULL;
    return &st;
}


/**/
static mode_t
dostat(char *s)
{
    struct stat *statp;

    if (!(statp = getstat(s)))
	return 0;
    return statp->st_mode;
}


/* pem@aaii.oz; needed since dostat now uses "stat" */

/**/
static mode_t
dolstat(char *s)
{
    if (lstat(unmeta(s), &st) < 0)
	return 0;
    return st.st_mode;
}


/**/
static int
optison(char *s)
{
    int i;

    if (strlen(s) == 1)
	i = optlookupc(*s);
    else
	i = optlookup(s);
    if (!i) {
	zerr("no such option: %s", s, 0);
	return 0;
    } else if(i < 0)
	return unset(-i);
    else
	return isset(i);
}

/**/
mod_export char *
cond_str(char **args, int num, int raw)
{
    char *s = args[num];

    if (has_token(s)) {
	singsub(&s);
	if (!raw)
	    untokenize(s);
    }
    return s;
}

/**/
mod_export zlong
cond_val(char **args, int num)
{
    char *s = args[num];

    if (has_token(s)) {
	singsub(&s);
	untokenize(s);
    }
    return mathevali(s);
}

/**/
mod_export int
cond_match(char **args, int num, char *str)
{
    char *s = args[num];

    singsub(&s);

    return matchpat(str, s);
}

/**/
static void
tracemodcond(char *name, char **args, int inf)
{
    char **aptr;

    args = arrdup(args);
    for (aptr = args; *aptr; aptr++)
	untokenize(*aptr);
    if (inf) {
	fprintf(xtrerr, " %s %s %s", args[0], name, args[1]);
    } else {
	fprintf(xtrerr, " %s", name);
	while (*args)
	    fprintf(xtrerr, " %s", *args++);
    }
}
