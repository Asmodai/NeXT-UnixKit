/*
 * glob.c - filename generation
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
#include "glob.pro"

#if defined(OFF_T_IS_64_BIT) && defined(__GNUC__)
# define ALIGN64 __attribute__((aligned(8)))
#else
# define ALIGN64
#endif

/* flag for CSHNULLGLOB */

typedef struct gmatch *Gmatch; 

struct gmatch {
    char *name;
    off_t size ALIGN64;
    long atime;
    long mtime;
    long ctime;
    long links;
    off_t _size ALIGN64;
    long _atime;
    long _mtime;
    long _ctime;
    long _links;
};

#define GS_NAME   1
#define GS_DEPTH  2
#define GS_SIZE   4
#define GS_ATIME  8
#define GS_MTIME 16
#define GS_CTIME 32
#define GS_LINKS 64

#define GS_SHIFT  5
#define GS__SIZE  (GS_SIZE << GS_SHIFT)
#define GS__ATIME (GS_ATIME << GS_SHIFT)
#define GS__MTIME (GS_MTIME << GS_SHIFT)
#define GS__CTIME (GS_CTIME << GS_SHIFT)
#define GS__LINKS (GS_LINKS << GS_SHIFT)

#define GS_DESC  4096

#define GS_NORMAL (GS_SIZE | GS_ATIME | GS_MTIME | GS_CTIME | GS_LINKS)
#define GS_LINKED (GS_NORMAL << GS_SHIFT)

/**/
int badcshglob;
 
/**/
int pathpos;		/* position in pathbuf (needed by pattern code) */

/**/
char *pathbuf;		/* pathname buffer (needed by pattern code) */

typedef struct stat *Statptr;	 /* This makes the Ultrix compiler happy.  Go figure. */

/* modifier for unit conversions */

#define TT_DAYS 0
#define TT_HOURS 1
#define TT_MINS 2
#define TT_WEEKS 3
#define TT_MONTHS 4
#define TT_SECONDS 5

#define TT_BYTES 0
#define TT_POSIX_BLOCKS 1
#define TT_KILOBYTES 2
#define TT_MEGABYTES 3


typedef int (*TestMatchFunc) _((char *, struct stat *, off_t, char *));

struct qual {
    struct qual *next;		/* Next qualifier, must match                */
    struct qual *or;		/* Alternative set of qualifiers to match    */
    TestMatchFunc func;		/* Function to call to test match            */
    off_t data ALIGN64;		/* Argument passed to function               */
    int sense;			/* Whether asserting or negating             */
    int amc;			/* Flag for which time to test (a, m, c)     */
    int range;			/* Whether to test <, > or = (as per signum) */
    int units;			/* Multiplier for time or size, respectively */
    char *sdata;		/* currently only: expression to eval        */
};

/* Prefix, suffix for doing zle trickery */

/**/
mod_export char *glob_pre, *glob_suf;

/* struct to easily save/restore current state */

struct globdata {
    int gd_pathpos;
    char *gd_pathbuf;

    int gd_matchsz;		/* size of matchbuf                     */
    int gd_matchct;		/* number of matches found              */
    int gd_pathbufsz;		/* size of pathbuf			*/
    int gd_pathbufcwd;		/* where did we chdir()'ed		*/
    Gmatch gd_matchbuf;		/* array of matches                     */
    Gmatch gd_matchptr;		/* &matchbuf[matchct]                   */
    char *gd_colonmod;		/* colon modifiers in qualifier list    */

    /* Qualifiers pertaining to current pattern */
    struct qual *gd_quals;

    /* Other state values for current pattern */
    int gd_qualct, gd_qualorct;
    int gd_range, gd_amc, gd_units;
    int gd_gf_nullglob, gd_gf_markdirs, gd_gf_noglobdots, gd_gf_listtypes;
    int gd_gf_numsort;
    int gd_gf_follow, gd_gf_sorts, gd_gf_nsorts, gd_gf_sortlist[11];

    char *gd_glob_pre, *gd_glob_suf;
};

/* The variable with the current globbing state and convenience macros */

static struct globdata curglobdata;

#define matchsz       (curglobdata.gd_matchsz)
#define matchct       (curglobdata.gd_matchct)
#define pathbufsz     (curglobdata.gd_pathbufsz)
#define pathbufcwd    (curglobdata.gd_pathbufcwd)
#define matchbuf      (curglobdata.gd_matchbuf)
#define matchptr      (curglobdata.gd_matchptr)
#define colonmod      (curglobdata.gd_colonmod)
#define quals         (curglobdata.gd_quals)
#define qualct        (curglobdata.gd_qualct)
#define qualorct      (curglobdata.gd_qualorct)
#define g_range       (curglobdata.gd_range)
#define g_amc         (curglobdata.gd_amc)
#define g_units       (curglobdata.gd_units)
#define gf_nullglob   (curglobdata.gd_gf_nullglob)
#define gf_markdirs   (curglobdata.gd_gf_markdirs)
#define gf_noglobdots (curglobdata.gd_gf_noglobdots)
#define gf_listtypes  (curglobdata.gd_gf_listtypes)
#define gf_numsort    (curglobdata.gd_gf_numsort)
#define gf_follow     (curglobdata.gd_gf_follow)
#define gf_sorts      (curglobdata.gd_gf_sorts)
#define gf_nsorts     (curglobdata.gd_gf_nsorts)
#define gf_sortlist   (curglobdata.gd_gf_sortlist)

/* and macros for save/restore */

#define save_globstate(N) \
  do { \
    memcpy(&(N), &curglobdata, sizeof(struct globdata)); \
    (N).gd_pathpos = pathpos; \
    (N).gd_pathbuf = pathbuf; \
    (N).gd_pathbufsz = 0; \
    (N).gd_pathbuf = NULL; \
    (N).gd_glob_pre = glob_pre; \
    (N).gd_glob_suf = glob_suf; \
  } while (0)

#define restore_globstate(N) \
  do { \
    zfree(pathbuf, pathbufsz); \
    memcpy(&curglobdata, &(N), sizeof(struct globdata)); \
    pathpos = (N).gd_pathpos; \
    pathbuf = (N).gd_pathbuf; \
    glob_pre = (N).gd_glob_pre; \
    glob_suf = (N).gd_glob_suf; \
  } while (0)

/* pathname component in filename patterns */

struct complist {
    Complist next;
    Patprog pat;
    int closure;		/* 1 if this is a (foo/)# */
    int follow; 		/* 1 to go thru symlinks */
};

/* Next character after one which may be a Meta (x is any char *) */
#define METANEXT(x)	(*(x) == Meta ? (x)+2 : (x)+1)
/*
 * Increment pointer which may be on a Meta (x is a pointer variable),
 * returning the incremented value (i.e. like pre-increment).
 */
#define METAINC(x)	((x) += (*(x) == Meta) ? 2 : 1)
/*
 * Return unmetafied char from string (x is any char *)
 */
#define UNMETA(x)	(*(x) == Meta ? (x)[1] ^ 32 : *(x))

/* Add a component to pathbuf: This keeps track of how    *
 * far we are into a file name, since each path component *
 * must be matched separately.                            */

/**/
static void
addpath(char *s)
{
    DPUTS(!pathbuf, "BUG: pathbuf not initialised");
    while (pathpos + (int) strlen(s) + 1 >= pathbufsz)
	pathbuf = realloc(pathbuf, pathbufsz *= 2);
    while ((pathbuf[pathpos++] = *s++));
    pathbuf[pathpos - 1] = '/';
    pathbuf[pathpos] = '\0';
}

/* stat the filename s appended to pathbuf.  l should be true for lstat,    *
 * false for stat.  If st is NULL, the file is only chechked for existance. *
 * s == "" is treated as s == ".".  This is necessary since on most systems *
 * foo/ can be used to reference a non-directory foo.  Returns nonzero if   *
 * the file does not exists.                                                */

/**/
static int
statfullpath(const char *s, struct stat *st, int l)
{
    char buf[PATH_MAX];

    DPUTS(strlen(s) + !*s + pathpos - pathbufcwd >= PATH_MAX,
	  "BUG: statfullpath(): pathname too long");
    strcpy(buf, pathbuf + pathbufcwd);
    strcpy(buf + pathpos - pathbufcwd, s);
    if (!*s && *buf) {
	/*
	 * Don't add the '.' if the path so far is empty, since
	 * then we get bogus empty strings inserted as files.
	 */
	buf[pathpos - pathbufcwd] = '.';
	buf[pathpos - pathbufcwd + 1] = '\0';
	l = 0;
    }
    unmetafy(buf, NULL);
    if (!st)
	return access(buf, F_OK) && (!l || readlink(buf, NULL, 0));
    return l ? lstat(buf, st) : stat(buf, st);
}

/* This may be set by qualifier functions to an array of strings to insert
 * into the list instead of the original string. */

char **inserts;

/* add a match to the list */

/**/
static void
insert(char *s, int checked)
{
    struct stat buf, buf2, *bp;
    char *news = s;
    int statted = 0;

    queue_signals();
    inserts = NULL;

    if (gf_listtypes || gf_markdirs) {
	/* Add the type marker to the end of the filename */
	mode_t mode;
	checked = statted = 1;
	if (statfullpath(s, &buf, 1)) {
	    unqueue_signals();
	    return;
	}
	mode = buf.st_mode;
	if (gf_follow) {
	    if (!S_ISLNK(mode) || statfullpath(s, &buf2, 0))
		memcpy(&buf2, &buf, sizeof(buf));
	    statted |= 2;
	    mode = buf2.st_mode;
	}
	if (gf_listtypes || S_ISDIR(mode)) {
	    int ll = strlen(s);

	    news = (char *) hcalloc(ll + 2);
	    strcpy(news, s);
	    news[ll] = file_type(mode);
	    news[ll + 1] = '\0';
	}
    }
    if (qualct || qualorct) {
	/* Go through the qualifiers, rejecting the file if appropriate */
	struct qual *qo, *qn;

	if (!statted && statfullpath(s, &buf, 1)) {
	    unqueue_signals();
	    return;
	}
	news = dyncat(pathbuf, news);

	statted = 1;
	qo = quals;
	for (qn = qo; qn && qn->func;) {
	    g_range = qn->range;
	    g_amc = qn->amc;
	    g_units = qn->units;
	    if ((qn->sense & 2) && !(statted & 2)) {
		/* If (sense & 2), we're following links */
		if (!S_ISLNK(buf.st_mode) || statfullpath(s, &buf2, 0))
		    memcpy(&buf2, &buf, sizeof(buf));
		statted |= 2;
	    }
	    bp = (qn->sense & 2) ? &buf2 : &buf;
	    /* Reject the file if the function returned zero *
	     * and the sense was positive (sense&1 == 0), or *
	     * vice versa.                                   */
	    if ((!((qn->func) (news, bp, qn->data, qn->sdata)) ^ qn->sense) & 1) {
		/* Try next alternative, or return if there are no more */
		if (!(qo = qo->or)) {
		    unqueue_signals();
		    return;
		}
		qn = qo;
		continue;
	    }
	    qn = qn->next;
	}
    } else if (!checked) {
	if (statfullpath(s, NULL, 1)) {
	    unqueue_signals();
	    return;
	}
	statted = 1;
	news = dyncat(pathbuf, news);
    } else
	news = dyncat(pathbuf, news);

    while (!inserts || (news = dupstring(*inserts++))) {
	if (colonmod) {
	    /* Handle the remainder of the qualifer:  e.g. (:r:s/foo/bar/). */
	    s = colonmod;
	    modify(&news, &s);
	}
	if (!statted && (gf_sorts & GS_NORMAL)) {
	    statfullpath(s, &buf, 1);
	    statted = 1;
	}
	if (!(statted & 2) && (gf_sorts & GS_LINKED)) {
	    if (statted) {
		if (!S_ISLNK(buf.st_mode) || statfullpath(s, &buf2, 0))
		    memcpy(&buf2, &buf, sizeof(buf));
	    } else if (statfullpath(s, &buf2, 0))
		statfullpath(s, &buf2, 1);
	    statted |= 2;
	}
	matchptr->name = news;
	if (statted & 1) {
	    matchptr->size = buf.st_size;
	    matchptr->atime = buf.st_atime;
	    matchptr->mtime = buf.st_mtime;
	    matchptr->ctime = buf.st_ctime;
	    matchptr->links = buf.st_nlink;
	}
	if (statted & 2) {
	    matchptr->_size = buf2.st_size;
	    matchptr->_atime = buf2.st_atime;
	    matchptr->_mtime = buf2.st_mtime;
	    matchptr->_ctime = buf2.st_ctime;
	    matchptr->_links = buf2.st_nlink;
	}
	matchptr++;

	if (++matchct == matchsz) {
	    matchbuf = (Gmatch )realloc((char *)matchbuf,
					sizeof(struct gmatch) * (matchsz *= 2));

	    matchptr = matchbuf + matchct;
	}
	if (!inserts)
	    break;
    }
    unqueue_signals();
}

/* Check to see if str is eligible for filename generation. */

/**/
mod_export int
haswilds(char *str)
{
    /* `[' and `]' are legal even if bad patterns are usually not. */
    if ((*str == Inbrack || *str == Outbrack) && !str[1])
	return 0;

    /* If % is immediately followed by ?, then that ? is     *
     * not treated as a wildcard.  This is so you don't have *
     * to escape job references such as %?foo.               */
    if (str[0] == '%' && str[1] == Quest)
	str[1] = '?';

    for (; *str; str++) {
	switch (*str) {
	    case Inpar:
	    case Bar:
	    case Star:
	    case Inbrack:
	    case Inang:
	    case Quest:
		return 1;
	    case Pound:
	    case Hat:
		if (isset(EXTENDEDGLOB))
		    return 1;
		break;
	}
    }
    return 0;
}

/* Do the globbing:  scanner is called recursively *
 * with successive bits of the path until we've    *
 * tried all of it.                                */

/**/
static void
scanner(Complist q)
{
    Patprog p;
    int closure;
    int pbcwdsav = pathbufcwd;
    int errssofar = errsfound;
    struct dirsav ds;

    ds.ino = ds.dev = 0;
    ds.dirname = NULL;
    ds.dirfd = ds.level = -1;
    if (!q)
	return;

    if ((closure = q->closure)) {
	/* (foo/)# - match zero or more dirs */
	if (q->closure == 2)	/* (foo/)## - match one or more dirs */
	    q->closure = 1;
	else
	    scanner(q->next);
    }
    p = q->pat;
    /* Now the actual matching for the current path section. */
    if (p->flags & PAT_PURES) {
	/*
	 * It's a straight string to the end of the path section.
	 */
	char *str = (char *)p + p->startoff;
	int l = p->patmlen;

	if (l + !l + pathpos - pathbufcwd >= PATH_MAX) {
	    int err;

	    if (l >= PATH_MAX)
		return;
	    err = lchdir(pathbuf + pathbufcwd, &ds, 0);
	    if (err == -1)
		return;
	    if (err) {
		zerr("current directory lost during glob", NULL, 0);
		return;
	    }
	    pathbufcwd = pathpos;
	}
	if (q->next) {
	    /* Not the last path section. Just add it to the path. */
	    int oppos = pathpos;

	    if (!errflag) {
		int add = 1;

		if (q->closure && *pathbuf) {
		    if (!strcmp(str, "."))
			add = 0;
		    else if (!strcmp(str, "..")) {
			struct stat sc, sr;

			add = (stat("/", &sr) || stat(pathbuf, &sc) ||
			       sr.st_ino != sc.st_ino ||
			       sr.st_dev != sc.st_dev);
		    }
		}
		if (add) {
		    addpath(str);
		    if (!closure || !statfullpath("", NULL, 1))
			scanner((q->closure) ? q : q->next);
		    pathbuf[pathpos = oppos] = '\0';
		}
	    }
	} else
	    insert(str, 0);
    } else {
	/* Do pattern matching on current path section. */
	char *fn = pathbuf[pathbufcwd] ? unmeta(pathbuf + pathbufcwd) : ".";
	int dirs = !!q->next;
	DIR *lock = opendir(fn);
	char *subdirs = NULL;
	int subdirlen = 0;

	if (lock == NULL)
	    return;
	while ((fn = zreaddir(lock, 1)) && !errflag) {
	    /* prefix and suffix are zle trickery */
	    if (!dirs && !colonmod &&
		((glob_pre && !strpfx(glob_pre, fn))
		 || (glob_suf && !strsfx(glob_suf, fn))))
		continue;
	    errsfound = errssofar;
	    if (pattry(p, fn)) {
		/* if this name matchs the pattern... */
		if (pbcwdsav == pathbufcwd &&
		    strlen(fn) + pathpos - pathbufcwd >= PATH_MAX) {
		    int err;

		    DPUTS(pathpos == pathbufcwd,
			  "BUG: filename longer than PATH_MAX");
		    err = lchdir(pathbuf + pathbufcwd, &ds, 0);
		    if (err == -1)
			break;
		    if (err) {
			zerr("current directory lost during glob", NULL, 0);
			break;
		    }
		    pathbufcwd = pathpos;
		}
		if (dirs) {
		    int l;

		    /*
		     * If not the last component in the path:
		     *
		     * If we made an approximation in the new path segment,
		     * then it is possible we made too many errors.  For
		     * example, (ab)#(cb)# will match the directory abcb
		     * with one error if allowed to, even though it can
		     * match with none.  This will stop later parts of the
		     * path matching, so we need to check by reducing the
		     * maximum number of errors and seeing if the directory
		     * still matches.  Luckily, this is not a terribly
		     * common case, since complex patterns typically occur
		     * in the last part of the path which is not affected
		     * by this problem.
		     */
		    if (errsfound > errssofar) {
			forceerrs = errsfound - 1;
			while (forceerrs >= errssofar) {
			    errsfound = errssofar;
			    if (!pattry(p, fn))
				break;
			    forceerrs = errsfound - 1;
			}
			errsfound = forceerrs + 1;
			forceerrs = -1;
		    }
		    if (closure) {
			/* if matching multiple directories */
			struct stat buf;

			if (statfullpath(fn, &buf, !q->follow)) {
			    if (errno != ENOENT && errno != EINTR &&
				errno != ENOTDIR && !errflag) {
				zwarn("%e: %s", fn, errno);
			    }
			    continue;
			}
			if (!S_ISDIR(buf.st_mode))
			    continue;
		    }
		    l = strlen(fn) + 1;
		    subdirs = hrealloc(subdirs, subdirlen, subdirlen + l
				       + sizeof(int));
		    strcpy(subdirs + subdirlen, fn);
		    subdirlen += l;
		    /* store the count of errors made so far, too */
		    memcpy(subdirs + subdirlen, (char *)&errsfound,
			   sizeof(int));
		    subdirlen += sizeof(int);
		} else
		    /* if the last filename component, just add it */
		    insert(fn, 1);
	    }
	}
	closedir(lock);
	if (subdirs) {
	    int oppos = pathpos;

	    for (fn = subdirs; fn < subdirs+subdirlen; ) {
		addpath(fn);
		fn += strlen(fn) + 1;
		memcpy((char *)&errsfound, fn, sizeof(int));
		fn += sizeof(int);
		scanner((q->closure) ? q : q->next);  /* scan next level */
		pathbuf[pathpos = oppos] = '\0';
	    }
	    hrealloc(subdirs, subdirlen, 0);
	}
    }
    if (pbcwdsav < pathbufcwd) {
	if (restoredir(&ds))
	    zerr("current directory lost during glob", NULL, 0);
	zsfree(ds.dirname);
	if (ds.dirfd >= 0)
	    close(ds.dirfd);
	pathbufcwd = pbcwdsav;
    }
}

/* This function tokenizes a zsh glob pattern */

/**/
static Complist
parsecomplist(char *instr)
{
    Patprog p1;
    Complist l1;
    char *str;
    int compflags = gf_noglobdots ? (PAT_FILE|PAT_NOGLD) : PAT_FILE;

    if (instr[0] == Star && instr[1] == Star &&
        (instr[2] == '/' || (instr[2] == Star && instr[3] == '/'))) {
	/* Match any number of directories. */
	int follow;

	/* with three stars, follow symbolic links */
	follow = (instr[2] == Star);
	instr += (3 + follow);

	/* Now get the next path component if there is one. */
	l1 = (Complist) zhalloc(sizeof *l1);
	if ((l1->next = parsecomplist(instr)) == NULL) {
	    errflag = 1;
	    return NULL;
	}
	l1->pat = patcompile(NULL, compflags | PAT_ANY, NULL);
	l1->closure = 1;		/* ...zero or more times. */
	l1->follow = follow;
	return l1;
    }

    /* Parse repeated directories such as (dir/)# and (dir/)## */
    if (*(str = instr) == Inpar && !skipparens(Inpar, Outpar, (char **)&str) &&
        *str == Pound && isset(EXTENDEDGLOB) && str[-2] == '/') {
	instr++;
	if (!(p1 = patcompile(instr, compflags, &instr)))
	    return NULL;
	if (instr[0] == '/' && instr[1] == Outpar && instr[2] == Pound) {
	    int pdflag = 0;

	    instr += 3;
	    if (*instr == Pound) {
		pdflag = 1;
		instr++;
	    }
	    l1 = (Complist) zhalloc(sizeof *l1);
	    l1->pat = p1;
	    l1->closure = 1 + pdflag;
	    l1->follow = 0;
	    l1->next = parsecomplist(instr);
	    return (l1->pat) ? l1 : NULL;
	}
    } else {
	/* parse single path component */
	if (!(p1 = patcompile(instr, compflags|PAT_FILET, &instr)))
	    return NULL;
	/* then do the remaining path compoents */
	if (*instr == '/' || !*instr) {
	    int ef = *instr == '/';

	    l1 = (Complist) zhalloc(sizeof *l1);
	    l1->pat = p1;
	    l1->closure = 0;
	    l1->next = ef ? parsecomplist(instr+1) : NULL;
	    return (ef && !l1->next) ? NULL : l1;
	}
    }
    errflag = 1;
    return NULL;
}

/* turn a string into a Complist struct:  this has path components */

/**/
static Complist
parsepat(char *str)
{
    long assert;

    patcompstart();
    /*
     * Check for initial globbing flags, so that they don't form
     * a bogus path component.
     */
    if ((*str == Inpar && str[1] == Pound && isset(EXTENDEDGLOB)) ||
	(isset(KSHGLOB) && *str == '@' && str[1] == Inpar &&
	 str[2] == Pound)) {
	str += (*str == Inpar) ? 2 : 3;
	if (!patgetglobflags(&str, &assert))
	    return NULL;
    }

    /* Now there is no (#X) in front, we can check the path. */
    if (!pathbuf)
	pathbuf = zalloc(pathbufsz = PATH_MAX);
    DPUTS(pathbufcwd, "BUG: glob changed directory");
    if (*str == '/') {		/* pattern has absolute path */
	str++;
	pathbuf[0] = '/';
	pathbuf[pathpos = 1] = '\0';
    } else			/* pattern is relative to pwd */
	pathbuf[pathpos = 0] = '\0';

    return parsecomplist(str);
}

/* get number after qualifier */

/**/
static off_t
qgetnum(char **s)
{
    off_t v = 0;

    if (!idigit(**s)) {
	zerr("number expected", NULL, 0);
	return 0;
    }
    while (idigit(**s))
	v = v * 10 + *(*s)++ - '0';
    return v;
}

/* get mode spec after qualifier */

/**/
static zlong
qgetmodespec(char **s)
{
    zlong yes = 0, no = 0, val, mask, t;
    char *p = *s, c, how, end;

    if ((c = *p) == '=' || c == Equals || c == '+' || c == '-' ||
	c == '?' || c == Quest || (c >= '0' && c <= '7')) {
	end = 0;
	c = 0;
    } else {
	end = (c == '<' ? '>' :
	       (c == '[' ? ']' :
		(c == '{' ? '}' :
		 (c == Inang ? Outang :
		  (c == Inbrack ? Outbrack :
		   (c == Inbrace ? Outbrace : c))))));
	p++;
    }
    do {
	mask = 0;
	while (((c = *p) == 'u' || c == 'g' || c == 'o' || c == 'a') && end) {
	    switch (c) {
	    case 'o': mask |= 01007; break;
	    case 'g': mask |= 02070; break;
	    case 'u': mask |= 04700; break;
	    case 'a': mask |= 07777; break;
	    }
	    p++;
	}
	how = ((c == '+' || c == '-') ? c : '=');
	if (c == '+' || c == '-' || c == '=' || c == Equals)
	    p++;
	val = 0;
	if (mask) {
	    while ((c = *p++) != ',' && c != end) {
		switch (c) {
		case 'x': val |= 00111; break;
		case 'w': val |= 00222; break;
		case 'r': val |= 00444; break;
		case 's': val |= 06000; break;
		case 't': val |= 01000; break;
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		    t = ((zlong) c - '0');
		    val |= t | (t << 3) | (t << 6);
		    break;
		default:
		    zerr("invalid mode specification", NULL, 0);
		    return 0;
		}
	    }
	    if (how == '=' || how == '+') {
		yes |= val & mask;
		val = ~val;
	    }
	    if (how == '=' || how == '-')
		no |= val & mask;
	} else {
	    t = 07777;
	    while ((c = *p) == '?' || c == Quest ||
		   (c >= '0' && c <= '7')) {
		if (c == '?' || c == Quest) {
		    t = (t << 3) | 7;
		    val <<= 3;
		} else {
		    t <<= 3;
		    val = (val << 3) | ((zlong) c - '0');
		}
		p++;
	    }
	    if (end && c != end && c != ',') {
		zerr("invalid mode specification", NULL, 0);
		return 0;
	    }
	    if (how == '=') {
		yes = (yes & ~t) | val;
		no = (no & ~t) | (~val & ~t);
	    } else if (how == '+')
		yes |= val;
	    else
		no |= val;
	}
    } while (end && c != end);

    *s = p;
    return ((yes & 07777) | ((no & 07777) << 12));
}

static int
gmatchcmp(Gmatch a, Gmatch b)
{
    int i, *s;
    off_t r = 0L;

    for (i = gf_nsorts, s = gf_sortlist; i; i--, s++) {
	switch (*s & ~GS_DESC) {
	case GS_NAME:
	    r = notstrcmp(&a->name, &b->name);
	    break;
	case GS_DEPTH:
	    {
		char *aptr = a->name, *bptr = b->name;
		int slasha = 0, slashb = 0;
		/* Count slashes.  Trailing slashes don't count. */
		while (*aptr && *aptr == *bptr)
		    aptr++, bptr++;
		if (*aptr)
		    for (; aptr[1]; aptr++)
			if (*aptr == '/') {
			    slasha = 1;
			    break;
			}
		if (*bptr)
		    for (; bptr[1]; bptr++)
			if (*bptr == '/') {
			    slashb = 1;
			    break;
			}
		r = slasha - slashb;
	    }
	    break;
	case GS_SIZE:
	    r = b->size - a->size;
	    break;
	case GS_ATIME:
	    r = a->atime - b->atime;
	    break;
	case GS_MTIME:
	    r = a->mtime - b->mtime;
	    break;
	case GS_CTIME:
	    r = a->ctime - b->ctime;
	    break;
	case GS_LINKS:
	    r = b->links - a->links;
	    break;
	case GS__SIZE:
	    r = b->_size - a->_size;
	    break;
	case GS__ATIME:
	    r = a->_atime - b->_atime;
	    break;
	case GS__MTIME:
	    r = a->_mtime - b->_mtime;
	    break;
	case GS__CTIME:
	    r = a->_ctime - b->_ctime;
	    break;
	case GS__LINKS:
	    r = b->_links - a->_links;
	    break;
	}
	if (r)
	    return (int) ((*s & GS_DESC) ? -r : r);
    }
    return 0;
}

/* Main entry point to the globbing code for filename globbing. *
 * np points to a node in the list list which will be expanded  *
 * into a series of nodes.                                      */

/**/
void
zglob(LinkList list, LinkNode np, int nountok)
{
    struct qual *qo, *qn, *ql;
    LinkNode node = prevnode(np);
    char *str;				/* the pattern                   */
    int sl;				/* length of the pattern         */
    Complist q;				/* pattern after parsing         */
    char *ostr = (char *)getdata(np);	/* the pattern before the parser */
					/* chops it up                   */
    int first = 0, end = -1;		/* index of first match to return */
					/* and index+1 of the last match */
    struct globdata saved;		/* saved glob state              */

    if (unset(GLOBOPT) || !haswilds(ostr)) {
	if (!nountok)
	    untokenize(ostr);
	return;
    }
    save_globstate(saved);

    str = dupstring(ostr);
    sl = strlen(str);
    uremnode(list, np);

    /* Initialise state variables for current file pattern */
    qo = qn = quals = ql = NULL;
    qualct = qualorct = 0;
    colonmod = NULL;
    gf_nullglob = isset(NULLGLOB);
    gf_markdirs = isset(MARKDIRS);
    gf_listtypes = gf_follow = 0;
    gf_noglobdots = unset(GLOBDOTS);
    gf_numsort = isset(NUMERICGLOBSORT);
    gf_sorts = gf_nsorts = 0;

    /* Check for qualifiers */
    if (isset(BAREGLOBQUAL) && str[sl - 1] == Outpar) {
	char *s;

	/* Check these are really qualifiers, not a set of *
	 * alternatives or exclusions                      */
	for (s = str + sl - 2; *s != Inpar; s--)
	    if (*s == Bar || *s == Outpar ||
		(isset(EXTENDEDGLOB) && *s == Tilde))
		break;
	if (*s == Inpar && (!isset(EXTENDEDGLOB) || s[1] != Pound)) {
	    /* Real qualifiers found. */
	    int sense = 0;	   /* bit 0 for match (0)/don't match (1)   */
				   /* bit 1 for follow links (2), don't (0) */
	    off_t data = 0;	   /* Any numerical argument required       */
	    char *sdata = NULL; /* Any list argument required            */
	    int (*func) _((char *, Statptr, off_t, char *));

	    str[sl-1] = 0;
	    *s++ = 0;
	    while (*s && !colonmod) {
		func = (int (*) _((char *, Statptr, off_t, char *)))0;
		if (idigit(*s)) {
		    /* Store numeric argument for qualifier */
		    func = qualflags;
		    data = 0;
		    sdata = NULL;
		    while (idigit(*s))
			data = data * 010 + (*s++ - '0');
		} else if (*s == ',') {
		    /* A comma separates alternative sets of qualifiers */
		    s++;
		    sense = 0;
		    if (qualct) {
			qn = (struct qual *)hcalloc(sizeof *qn);
			qo->or = qn;
			qo = qn;
			qualorct++;
			qualct = 0;
			ql = NULL;
		    }
		} else
		    switch (*s++) {
		    case ':':
			/* Remaining arguments are history-type     *
			 * colon substitutions, handled separately. */
			colonmod = s - 1;
			untokenize(colonmod);
			break;
		    case Hat:
		    case '^':
			/* Toggle sense:  go from positive to *
			 * negative match and vice versa.     */
			sense ^= 1;
			break;
		    case '-':
			/* Toggle matching of symbolic links */
			sense ^= 2;
			break;
		    case '@':
			/* Match symbolic links */
			func = qualislnk;
			break;
		    case Equals:
		    case '=':
			/* Match sockets */
			func = qualissock;
			break;
		    case 'p':
			/* Match named pipes */
			func = qualisfifo;
			break;
		    case '/':
			/* Match directories */
			func = qualisdir;
			break;
		    case '.':
			/* Match regular files */
			func = qualisreg;
			break;
		    case '%':
			/* Match special files: block, *
			 * character or any device     */
			if (*s == 'b')
			    s++, func = qualisblk;
			else if (*s == 'c')
			    s++, func = qualischr;
			else
			    func = qualisdev;
			break;
		    case Star:
			/* Match executable plain files */
			func = qualiscom;
			break;
		    case 'R':
			/* Match world-readable files */
			func = qualflags;
			data = 0004;
			break;
		    case 'W':
			/* Match world-writeable files */
			func = qualflags;
			data = 0002;
			break;
		    case 'X':
			/* Match world-executable files */
			func = qualflags;
			data = 0001;
			break;
		    case 'A':
			func = qualflags;
			data = 0040;
			break;
		    case 'I':
			func = qualflags;
			data = 0020;
			break;
		    case 'E':
			func = qualflags;
			data = 0010;
			break;
		    case 'r':
			/* Match files readable by current process */
			func = qualflags;
			data = 0400;
			break;
		    case 'w':
			/* Match files writeable by current process */
			func = qualflags;
			data = 0200;
			break;
		    case 'x':
			/* Match files executable by current process */
			func = qualflags;
			data = 0100;
			break;
		    case 's':
			/* Match setuid files */
			func = qualflags;
			data = 04000;
			break;
		    case 'S':
			/* Match setgid files */
			func = qualflags;
			data = 02000;
			break;
		    case 't':
			func = qualflags;
			data = 01000;
			break;
		    case 'd':
			/* Match device files by device number  *
			 * (as given by stat's st_dev element). */
			func = qualdev;
			data = qgetnum(&s);
			break;
		    case 'l':
			/* Match files with the given no. of hard links */
			func = qualnlink;
			g_amc = -1;
			goto getrange;
		    case 'U':
			/* Match files owned by effective user ID */
			func = qualuid;
			data = geteuid();
			break;
		    case 'G':
			/* Match files owned by effective group ID */
			func = qualgid;
			data = getegid();
			break;
		    case 'u':
			/* Match files owned by given user id */
			func = qualuid;
			/* either the actual uid... */
			if (idigit(*s))
			    data = qgetnum(&s);
			else {
			    /* ... or a user name */
			    char sav, *tt;

			    /* Find matching delimiters */
			    tt = get_strarg(s);
			    if (!*tt) {
				zerr("missing end of name",
				     NULL, 0);
				data = 0;
			    } else {
#ifdef HAVE_GETPWNAM
				struct passwd *pw;
				sav = *tt;
				*tt = '\0';

				if ((pw = getpwnam(s + 1)))
				    data = pw->pw_uid;
				else {
				    zerr("unknown user", NULL, 0);
				    data = 0;
				}
				*tt = sav;
#else /* !HAVE_GETPWNAM */
				sav = *tt;
				zerr("unknown user", NULL, 0);
				data = 0;
#endif /* !HAVE_GETPWNAM */
				if (sav)
				    s = tt + 1;
				else
				    s = tt;
			    }
			}
			break;
		    case 'g':
			/* Given gid or group id... works like `u' */
			func = qualgid;
			/* either the actual gid... */
			if (idigit(*s))
			    data = qgetnum(&s);
			else {
			    /* ...or a delimited group name. */
			    char sav, *tt;

			    tt = get_strarg(s);
			    if (!*tt) {
				zerr("missing end of name",
				     NULL, 0);
				data = 0;
			    } else {
#ifdef HAVE_GETGRNAM
				struct group *gr;
				sav = *tt;
				*tt = '\0';

				if ((gr = getgrnam(s + 1)))
				    data = gr->gr_gid;
				else {
				    zerr("unknown group", NULL, 0);
				    data = 0;
				}
				*tt = sav;
#else /* !HAVE_GETGRNAM */
				sav = *tt;
				zerr("unknown group", NULL, 0);
				data = 0;
#endif /* !HAVE_GETGRNAM */
				if (sav)
				    s = tt + 1;
				else
				    s = tt;
			    }
			}
			break;
		    case 'f':
			/* Match modes with chmod-spec. */
			func = qualmodeflags;
			data = qgetmodespec(&s);
			break;
		    case 'M':
			/* Mark directories with a / */
			if ((gf_markdirs = !(sense & 1)))
			    gf_follow = sense & 2;
			break;
		    case 'T':
			/* Mark types in a `ls -F' type fashion */
			if ((gf_listtypes = !(sense & 1)))
			    gf_follow = sense & 2;
			break;
		    case 'N':
			/* Nullglob:  remove unmatched patterns. */
			gf_nullglob = !(sense & 1);
			break;
		    case 'D':
			/* Glob dots: match leading dots implicitly */
			gf_noglobdots = sense & 1;
			break;
		    case 'n':
			/* Numeric glob sort */
			gf_numsort = !(sense & 1);
			break;
		    case 'a':
			/* Access time in given range */
			g_amc = 0;
			func = qualtime;
			goto getrange;
		    case 'm':
			/* Modification time in given range */
			g_amc = 1;
			func = qualtime;
			goto getrange;
		    case 'c':
			/* Inode creation time in given range */
			g_amc = 2;
			func = qualtime;
			goto getrange;
		    case 'L':
			/* File size (Length) in given range */
			func = qualsize;
			g_amc = -1;
			/* Get size multiplier */
			g_units = TT_BYTES;
			if (*s == 'p' || *s == 'P')
			    g_units = TT_POSIX_BLOCKS, ++s;
			else if (*s == 'k' || *s == 'K')
			    g_units = TT_KILOBYTES, ++s;
			else if (*s == 'm' || *s == 'M')
			    g_units = TT_MEGABYTES, ++s;
		      getrange:
			/* Get time multiplier */
			if (g_amc >= 0) {
			    g_units = TT_DAYS;
			    if (*s == 'h')
				g_units = TT_HOURS, ++s;
			    else if (*s == 'm')
				g_units = TT_MINS, ++s;
			    else if (*s == 'w')
				g_units = TT_WEEKS, ++s;
			    else if (*s == 'M')
				g_units = TT_MONTHS, ++s;
			    else if (*s == 's')
				g_units = TT_SECONDS, ++s;
			}
			/* See if it's greater than, equal to, or less than */
			if ((g_range = *s == '+' ? 1 : *s == '-' ? -1 : 0))
			    ++s;
			data = qgetnum(&s);
			break;

		    case 'o':
		    case 'O':
			{
			    int t;

			    switch (*s) {
			    case 'n': t = GS_NAME; break;
			    case 'L': t = GS_SIZE; break;
			    case 'l': t = GS_LINKS; break;
			    case 'a': t = GS_ATIME; break;
			    case 'm': t = GS_MTIME; break;
			    case 'c': t = GS_CTIME; break;
			    case 'd': t = GS_DEPTH; break;
			    default:
				zerr("unknown sort specifier", NULL, 0);
				restore_globstate(saved);
				return;
			    }
			    if ((sense & 2) && !(t & (GS_NAME|GS_DEPTH)))
				t <<= GS_SHIFT;
			    if (gf_sorts & t) {
				zerr("doubled sort specifier", NULL, 0);
				restore_globstate(saved);
				return;
			    }
			    gf_sorts |= t;
			    gf_sortlist[gf_nsorts++] = t |
				(((sense & 1) ^ (s[-1] == 'O')) ? GS_DESC : 0);
			    s++;
			    break;
			}
		    case 'e':
			{
			    char sav, *tt = get_strarg(s);

			    if (!*tt) {
				zerr("missing end of string", NULL, 0);
				data = 0;
			    } else {
				sav = *tt;
				*tt = '\0';
				func = qualsheval;
				sdata = dupstring(s + 1);
				untokenize(sdata);
				*tt = sav;
				if (sav)
				    s = tt + 1;
				else
				    s = tt;
			    }
			    break;
			}
		    case '[':
		    case Inbrack:
			{
			    char *os = --s;
			    struct value v;

			    v.isarr = SCANPM_WANTVALS;
			    v.pm = NULL;
			    v.end = -1;
			    v.inv = 0;
			    if (getindex(&s, &v, 0) || s == os) {
				zerr("invalid subscript", NULL, 0);
				restore_globstate(saved);
				return;
			    }
			    first = v.start;
			    end = v.end;
			    break;
			}
		    default:
			zerr("unknown file attribute", NULL, 0);
			restore_globstate(saved);
			return;
		    }
		if (func) {
		    /* Requested test is performed by function func */
		    if (!qn)
			qn = (struct qual *)hcalloc(sizeof *qn);
		    if (ql)
			ql->next = qn;
		    ql = qn;
		    if (!quals)
			quals = qo = qn;
		    qn->func = func;
		    qn->sense = sense;
		    qn->data = data;
		    qn->sdata = sdata;
		    qn->range = g_range;
		    qn->units = g_units;
		    qn->amc = g_amc;
		    qn = NULL;
		    qualct++;
		}
		if (errflag) {
		    restore_globstate(saved);
		    return;
		}
	    }
	}
    }
    q = parsepat(str);
    if (!q || errflag) {	/* if parsing failed */
	restore_globstate(saved);
	if (unset(BADPATTERN)) {
	    if (!nountok)
		untokenize(ostr);
	    insertlinknode(list, node, ostr);
	    return;
	}
	errflag = 0;
	zerr("bad pattern: %s", ostr, 0);
	return;
    }
    if (!gf_nsorts) {
	gf_sortlist[0] = gf_sorts = GS_NAME;
	gf_nsorts = 1;
    }
    /* Initialise receptacle for matched files, *
     * expanded by insert() where necessary.    */
    matchptr = matchbuf = (Gmatch)zalloc((matchsz = 16) *
					 sizeof(struct gmatch));
    matchct = 0;
    pattrystart();

    /* The actual processing takes place here: matches go into  *
     * matchbuf.  This is the only top-level call to scanner(). */
    scanner(q);

    /* Deal with failures to match depending on options */
    if (matchct)
	badcshglob |= 2;	/* at least one cmd. line expansion O.K. */
    else if (!gf_nullglob) {
	if (isset(CSHNULLGLOB)) {
	    badcshglob |= 1;	/* at least one cmd. line expansion failed */
	} else if (isset(NOMATCH)) {
	    zerr("no matches found: %s", ostr, 0);
	    free(matchbuf);
	    return;
	} else {
	    /* treat as an ordinary string */
	    untokenize(matchptr->name = dupstring(ostr));
	    matchptr++;
	    matchct = 1;
	}
    }
    /* Sort arguments in to lexical (and possibly numeric) order. *
     * This is reversed to facilitate insertion into the list.    */
    qsort((void *) & matchbuf[0], matchct, sizeof(struct gmatch),
	       (int (*) _((const void *, const void *)))gmatchcmp);

    if (first < 0) {
	first += matchct;
	if (first < 0)
	    first = 0;
    }
    if (end < 0)
	end += matchct + 1;
    else if (end > matchct)
	end = matchct;
    if ((end -= first) > 0) {
	matchptr = matchbuf + matchct - first - end;
	while (end-- > 0) {		/* insert matches in the arg list */
	    insertlinknode(list, node, matchptr->name);
	    matchptr++;
	}
    }
    free(matchbuf);

    restore_globstate(saved);
}

/* Return the order of two strings, taking into account *
 * possible numeric order if NUMERICGLOBSORT is set.    *
 * The comparison here is reversed.                     */

/**/
static int
notstrcmp(char **a, char **b)
{
    char *c = *b, *d = *a;
    int cmp;

#ifdef HAVE_STRCOLL
    cmp = strcoll(c, d);
#endif
    for (; *c == *d && *c; c++, d++);
#ifndef HAVE_STRCOLL
    cmp = (int)STOUC(*c) - (int)STOUC(*d);
#endif
    if (gf_numsort && (idigit(*c) || idigit(*d))) {
	for (; c > *b && idigit(c[-1]); c--, d--);
	if (idigit(*c) && idigit(*d)) {
	    while (*c == '0')
		c++;
	    while (*d == '0')
		d++;
	    for (; idigit(*c) && *c == *d; c++, d++);
	    if (idigit(*c) || idigit(*d)) {
		cmp = (int)STOUC(*c) - (int)STOUC(*d);
		while (idigit(*c) && idigit(*d))
		    c++, d++;
		if (idigit(*c) && !idigit(*d))
		    return 1;
		if (idigit(*d) && !idigit(*c))
		    return -1;
	    }
	}
    }
    return cmp;
}

/* Return the trailing character for marking file types */

/**/
mod_export char
file_type(mode_t filemode)
{
    if(S_ISBLK(filemode))
	return '#';
    else if(S_ISCHR(filemode))
	return '%';
    else if(S_ISDIR(filemode))
	return '/';
    else if(S_ISFIFO(filemode))
	return '|';
    else if(S_ISLNK(filemode))
	return '@';
    else if(S_ISREG(filemode))
	return (filemode & S_IXUGO) ? '*' : ' ';
    else if(S_ISSOCK(filemode))
	return '=';
    else
	return '?';
}

/* check to see if str is eligible for brace expansion */

/**/
mod_export int
hasbraces(char *str)
{
    char *lbr, *mbr, *comma;

    if (isset(BRACECCL)) {
	/* In this case, any properly formed brace expression  *
	 * will match and expand to the characters in between. */
	int bc, c;

	for (bc = 0; (c = *str); ++str)
	    if (c == Inbrace) {
		if (!bc && str[1] == Outbrace)
		    *str++ = '{', *str = '}';
		else
		    bc++;
	    } else if (c == Outbrace) {
		if (!bc)
		    *str = '}';
		else if (!--bc)
		    return 1;
	    }
	return 0;
    }
    /* Otherwise we need to look for... */
    lbr = mbr = comma = NULL;
    for (;;) {
	switch (*str++) {
	case Inbrace:
	    if (!lbr) {
		lbr = str - 1;
		while (idigit(*str))
		    str++;
		if (*str == '.' && str[1] == '.') {
		    str++;
		    while (idigit(*++str));
		    if (*str == Outbrace &&
			(idigit(lbr[1]) || idigit(str[-1])))
			return 1;
		}
	    } else {
		char *s = --str;

		if (skipparens(Inbrace, Outbrace, &str)) {
		    *lbr = *s = '{';
		    if (comma)
			str = comma;
		    if (mbr && mbr < str)
			str = mbr;
		    lbr = mbr = comma = NULL;
		} else if (!mbr)
		    mbr = s;
	    }
	    break;
	case Outbrace:
	    if (!lbr)
		str[-1] = '}';
	    else if (comma)
		return 1;
	    else {
		*lbr = '{';
		str[-1] = '}';
		if (mbr)
		    str = mbr;
		mbr = lbr = NULL;
	    }
	    break;
	case Comma:
	    if (!lbr)
		str[-1] = ',';
	    else if (!comma)
		comma = str - 1;
	    break;
	case '\0':
	    if (lbr)
		*lbr = '{';
	    if (!mbr && !comma)
		return 0;
	    if (comma)
		str = comma;
	    if (mbr && mbr < str)
		str = mbr;
	    lbr = mbr = comma = NULL;
	    break;
	}
    }
}

/* expand stuff like >>*.c */

/**/
int
xpandredir(struct redir *fn, LinkList tab)
{
    char *nam;
    struct redir *ff;
    int ret = 0;
    local_list1(fake);

    /* Stick the name in a list... */
    init_list1(fake, fn->name);
    /* ...which undergoes all the usual shell expansions */
    prefork(&fake, isset(MULTIOS) ? 0 : PF_SINGLE);
    /* Globbing is only done for multios. */
    if (!errflag && isset(MULTIOS))
	globlist(&fake, 0);
    if (errflag)
	return 0;
    if (nonempty(&fake) && !nextnode(firstnode(&fake))) {
	/* Just one match, the usual case. */
	char *s = peekfirst(&fake);
	fn->name = s;
	untokenize(s);
	if (fn->type == REDIR_MERGEIN || fn->type == REDIR_MERGEOUT) {
	    if (s[0] == '-' && !s[1])
		fn->type = REDIR_CLOSE;
	    else if (s[0] == 'p' && !s[1]) 
		fn->fd2 = -2;
	    else {
		while (idigit(*s))
		    s++;
		if (!*s && s > fn->name)
		    fn->fd2 = zstrtol(fn->name, NULL, 10);
		else if (fn->type == REDIR_MERGEIN)
		    zerr("file number expected", NULL, 0);
		else
		    fn->type = REDIR_ERRWRITE;
	    }
	}
    } else if (fn->type == REDIR_MERGEIN)
	zerr("file number expected", NULL, 0);
    else {
	if (fn->type == REDIR_MERGEOUT)
	    fn->type = REDIR_ERRWRITE;
	while ((nam = (char *)ugetnode(&fake))) {
	    /* Loop over matches, duplicating the *
	     * redirection for each file found.   */
	    ff = (struct redir *) zhalloc(sizeof *ff);
	    *ff = *fn;
	    ff->name = nam;
	    addlinknode(tab, ff);
	    ret = 1;
	}
    }
    return ret;
}

/* brace expansion */

/**/
mod_export void
xpandbraces(LinkList list, LinkNode *np)
{
    LinkNode node = (*np), last = prevnode(node);
    char *str = (char *)getdata(node), *str3 = str, *str2;
    int prev, bc, comma, dotdot;

    for (; *str != Inbrace; str++);
    /* First, match up braces and see what we have. */
    for (str2 = str, bc = comma = dotdot = 0; *str2; ++str2)
	if (*str2 == Inbrace)
	    ++bc;
	else if (*str2 == Outbrace) {
	    if (--bc == 0)
		break;
	} else if (bc == 1) {
	    if (*str2 == Comma)
		++comma;	/* we have {foo,bar} */
	    else if (*str2 == '.' && str2[1] == '.')
		dotdot++;	/* we have {num1..num2} */
	}
    DPUTS(bc, "BUG: unmatched brace in xpandbraces()");
    if (!comma && dotdot) {
	/* Expand range like 0..10 numerically: comma or recursive
	   brace expansion take precedence. */
	char *dots, *p;
	LinkNode olast = last;
	/* Get the first number of the range */
	int rstart = zstrtol(str+1,&dots,10), rend = 0, err = 0, rev = 0;
	int wid1 = (dots - str) - 1, wid2 = (str2 - dots) - 2;
	int strp = str - str3;
      
	if (dots == str + 1 || *dots != '.' || dots[1] != '.')
	    err++;
	else {
	    /* Get the last number of the range */
	    rend = zstrtol(dots+2,&p,10);
	    if (p == dots+2 || p != str2)
		err++;
	}
	if (!err) {
	    /* If either no. begins with a zero, pad the output with   *
	     * zeroes. Otherwise, choose a min width to suppress them. */
	    int minw = (str[1] == '0') ? wid1 : (dots[2] == '0' ) ? wid2 :
		(wid2 > wid1) ? wid1 : wid2;
	    if (rstart > rend) {
		/* Handle decreasing ranges correctly. */
		int rt = rend;
		rend = rstart;
		rstart = rt;
		rev = 1;
	    }
	    uremnode(list, node);
	    for (; rend >= rstart; rend--) {
		/* Node added in at end, so do highest first */
		p = dupstring(str3);
		sprintf(p + strp, "%0*d", minw, rend);
		strcat(p + strp, str2 + 1);
		insertlinknode(list, last, p);
		if (rev)	/* decreasing:  add in reverse order. */
		    last = nextnode(last);
	    }
	    *np = nextnode(olast);
	    return;
	}
    }
    if (!comma && isset(BRACECCL)) {	/* {a-mnop} */
	/* Here we expand each character to a separate node,      *
	 * but also ranges of characters like a-m.  ccl is a      *
	 * set of flags saying whether each character is present; *
	 * the final list is in lexical order.                    */
	char ccl[256], *p;
	unsigned char c1, c2, lastch;
	unsigned int len, pl;

	uremnode(list, node);
	memset(ccl, 0, sizeof(ccl) / sizeof(ccl[0]));
	for (p = str + 1, lastch = 0; p < str2;) {
	    if (itok(c1 = *p++))
		c1 = ztokens[c1 - STOUC(Pound)];
	    if ((char) c1 == Meta)
		c1 = 32 ^ *p++;
	    if (itok(c2 = *p))
		c2 = ztokens[c2 - STOUC(Pound)];
	    if ((char) c2 == Meta)
		c2 = 32 ^ p[1];
	    if (c1 == '-' && lastch && p < str2 && (int)lastch <= (int)c2) {
		while ((int)lastch < (int)c2)
		    ccl[lastch++] = 1;
		lastch = 0;
	    } else
		ccl[lastch = c1] = 1;
	}
	pl = str - str3;
	len = pl + strlen(++str2) + 2;
	for (p = ccl + 255; p-- > ccl;)
	    if (*p) {
		c1 = p - ccl;
		if (imeta(c1)) {
		    str = hcalloc(len + 1);
		    str[pl] = Meta;
		    str[pl+1] = c1 ^ 32;
		    strcpy(str + pl + 2, str2);
		} else {
		    str = hcalloc(len);
		    str[pl] = c1;
		    strcpy(str + pl + 1, str2);
		}
		memcpy(str, str3, pl);
		insertlinknode(list, last, str);
	    }
	*np = nextnode(last);
	return;
    }
    prev = str++ - str3;
    str2++;
    uremnode(list, node);
    node = last;
    /* Finally, normal comma expansion               *
     * str1{foo,bar}str2 -> str1foostr2 str1barstr2. *
     * Any number of intervening commas is allowed.  */
    for (;;) {
	char *zz, *str4;
	int cnt;

	for (str4 = str, cnt = 0; cnt || (*str != Comma && *str !=
					  Outbrace); str++) {
	    if (*str == Inbrace)
		cnt++;
	    else if (*str == Outbrace)
		cnt--;
	    DPUTS(!*str, "BUG: illegal brace expansion");
	}
	/* Concatenate the string before the braces (str3), the section *
	 * just found (str4) and the text after the braces (str2)       */
	zz = (char *) hcalloc(prev + (str - str4) + strlen(str2) + 1);
	ztrncpy(zz, str3, prev);
	strncat(zz, str4, str - str4);
	strcat(zz, str2);
	/* and add this text to the argument list. */
	insertlinknode(list, node, zz);
	incnode(node);
	if (*str != Outbrace)
	    str++;
	else
	    break;
    }
    *np = nextnode(last);
}

/* check to see if a matches b (b is not a filename pattern) */

/**/
int
matchpat(char *a, char *b)
{
    Patprog p = patcompile(b, PAT_STATIC, NULL);

    if (!p) {
	zerr("bad pattern: %s", b, 0);
	return 0;
    }
    return pattry(p, a);
}

/* do the ${foo%%bar}, ${foo#bar} stuff */
/* please do not laugh at this code. */

struct repldata {
    int b, e;			/* beginning and end of chunk to replace */
    char *replstr;		/* replacement string to use */
};
typedef struct repldata *Repldata;

/* 
 * List of bits of matches to concatenate with replacement string.
 * The data is a struct repldata.  It is not used in cases like
 * ${...//#foo/bar} even though SUB_GLOBAL is set, since the match
 * is anchored.  It goes on the heap.
 */

static LinkList repllist;

/* Having found a match in getmatch, decide what part of string
 * to return.  The matched part starts b characters into string s
 * and finishes e characters in: 0 <= b <= e <= strlen(s)
 * (yes, empty matches should work).
 * fl is a set of the SUB_* matches defined in zsh.h from SUB_MATCH onwards;
 * the lower parts are ignored.
 * replstr is the replacement string for a substitution
 */

/**/
static char *
get_match_ret(char *s, int b, int e, int fl, char *replstr)
{
    char buf[80], *r, *p, *rr;
    int ll = 0, l = strlen(s), bl = 0, t = 0, i;

    if (replstr) {
	if (fl & SUB_DOSUBST) {
	    replstr = dupstring(replstr);
	    singsub(&replstr);
	    untokenize(replstr);
	}
	if ((fl & SUB_GLOBAL) && repllist) {
	    /* We are replacing the chunk, just add this to the list */
	    Repldata rd = (Repldata) zhalloc(sizeof(*rd));
	    rd->b = b;
	    rd->e = e;
	    rd->replstr = replstr;
	    addlinknode(repllist, rd);
	    return s;
	}
	ll += strlen(replstr);
    }
    if (fl & SUB_MATCH)			/* matched portion */
	ll += 1 + (e - b);
    if (fl & SUB_REST)		/* unmatched portion */
	ll += 1 + (l - (e - b));
    if (fl & SUB_BIND) {
	/* position of start of matched portion */
	sprintf(buf, "%d ", b + 1);
	ll += (bl = strlen(buf));
    }
    if (fl & SUB_EIND) {
	/* position of end of matched portion */
	sprintf(buf + bl, "%d ", e + 1);
	ll += (bl = strlen(buf));
    }
    if (fl & SUB_LEN) {
	/* length of matched portion */
	sprintf(buf + bl, "%d ", e - b);
	ll += (bl = strlen(buf));
    }
    if (bl)
	buf[bl - 1] = '\0';

    rr = r = (char *) hcalloc(ll);

    if (fl & SUB_MATCH) {
	/* copy matched portion to new buffer */
	for (i = b, p = s + b; i < e; i++)
	    *rr++ = *p++;
	t = 1;
    }
    if (fl & SUB_REST) {
	/* Copy unmatched portion to buffer.  If both portions *
	 * requested, put a space in between (why?)            */
	if (t)
	    *rr++ = ' ';
	/* there may be unmatched bits at both beginning and end of string */
	for (i = 0, p = s; i < b; i++)
	    *rr++ = *p++;
	if (replstr)
	    for (p = replstr; *p; )
		*rr++ = *p++;
	for (i = e, p = s + e; i < l; i++)
	    *rr++ = *p++;
	t = 1;
    }
    *rr = '\0';
    if (bl) {
	/* if there was a buffer (with a numeric result), add it; *
	 * if there was other stuff too, stick in a space first.  */
	if (t)
	    *rr++ = ' ';
	strcpy(rr, buf);
    }
    return r;
}

static Patprog
compgetmatch(char *pat, int *flp, char **replstrp)
{
    Patprog p;
    /*
     * Flags to pattern compiler:  use static buffer since we only
     * have one pattern at a time; we will try the must-match test ourselves,
     * so tell the pattern compiler we are scanning.
     */

    /* int patflags = PAT_STATIC|PAT_SCAN|PAT_NOANCH;*/

    /* Unfortunately, PAT_STATIC doesn't work if we have a replstr with
     * something like ${x#...} in it which will be singsub()ed below because
     * that would overwrite the pattern buffer. */

    int patflags = PAT_SCAN|PAT_NOANCH | (*replstrp ? 0 : PAT_STATIC);

    /*
     * Search is anchored to the end of the string if we want to match
     * it all, or if we are matching at the end of the string and not
     * using substrings.
     */
    if ((*flp & SUB_ALL) || ((*flp & SUB_END) && !(*flp & SUB_SUBSTR)))
	patflags &= ~PAT_NOANCH;
    p = patcompile(pat, patflags, NULL);
    if (!p) {
	zerr("bad pattern: %s", pat, 0);
	return NULL;
    }
    if (*replstrp) {
	if (p->patnpar || (p->globend & GF_MATCHREF)) {
	    /*
	     * Either backreferences or match references, so we
	     * need to re-substitute replstr each time round.
	     */
	    *flp |= SUB_DOSUBST;
	} else {
	    singsub(replstrp);
	    untokenize(*replstrp);
	}
    }

    return p;
}

/*
 * This is called from paramsubst to get the match for ${foo#bar} etc.
 * fl is a set of the SUB_* flags defined in zsh.h
 * *sp points to the string we have to modify. The n'th match will be
 * returned in *sp. The heap is used to get memory for the result string.
 * replstr is the replacement string from a ${.../orig/repl}, in
 * which case pat is the original.
 *
 * n is now ignored unless we are looking for a substring, in
 * which case the n'th match from the start is counted such that
 * there is no more than one match from each position.
 */

/**/
int
getmatch(char **sp, char *pat, int fl, int n, char *replstr)
{
    Patprog p;

    if (!(p = compgetmatch(pat, &fl, &replstr)))
	return 1;

    return igetmatch(sp, p, fl, n, replstr);
}

/**/
void
getmatcharr(char ***ap, char *pat, int fl, int n, char *replstr)
{
    char **arr = *ap, **pp;
    Patprog p;

    if (!(p = compgetmatch(pat, &fl, &replstr)))
	return;

    *ap = pp = hcalloc(sizeof(char *) * (arrlen(arr) + 1));
    while ((*pp = *arr++))
	if (igetmatch(pp, p, fl, n, replstr))
	    pp++;
}

/**/
static void
set_pat_start(Patprog p, int offs)
{
    /*
     * If we are messing around with the test string by advancing up
     * it from the start, we need to tell the pattern matcher that
     * a start-of-string assertion, i.e. (#s), should fail.  Hence
     * we test whether the offset of the real start of string from
     * the actual start, passed as offs, is zero.
     */
    if (offs)
	p->flags |= PAT_NOTSTART;
    else
	p->flags &= ~PAT_NOTSTART;
}

/**/
static void
set_pat_end(Patprog p, char null_me)
{
    /*
     * If we are messing around with the string by shortening it at the
     * tail, we need to tell the pattern matcher that an end-of-string
     * assertion, i.e. (#e), should fail.  Hence we test whether
     * the character null_me about to be zapped is or is not already a null.
     */
    if (null_me)
	p->flags |= PAT_NOTEND;
    else
	p->flags &= ~PAT_NOTEND;
}

/**/
static int
igetmatch(char **sp, Patprog p, int fl, int n, char *replstr)
{
    char *s = *sp, *t, sav;
    int i, l = strlen(*sp), ml = ztrlen(*sp), matched = 1;

    repllist = NULL;

    /* perform must-match test for complex closures */
    if (p->mustoff && !strstr((char *)s, (char *)p + p->mustoff))
	matched = 0;

    /* in case we used the prog before... */
    p->flags &= ~(PAT_NOTSTART|PAT_NOTEND);

    if (fl & SUB_ALL) {
	i = matched && pattry(p, s);
	*sp = get_match_ret(*sp, 0, i ? l : 0, fl, i ? replstr : 0);
	if (! **sp && (((fl & SUB_MATCH) && !i) || ((fl & SUB_REST) && i)))
	    return 0;
	return 1;
    }
    if (matched) {
	switch (fl & (SUB_END|SUB_LONG|SUB_SUBSTR)) {
	case 0:
	case SUB_LONG:
	    /*
	     * Largest/smallest possible match at head of string.
	     * First get the longest match...
	     */
	    if (pattry(p, s)) {
		char *mpos = patinput;
		if (!(fl & SUB_LONG) && !(p->flags & PAT_PURES)) {
		    /*
		     * ... now we know whether it's worth looking for the
		     * shortest, which we do by brute force.
		     */
		    for (t = s; t < mpos; METAINC(t)) {
			sav = *t;
			set_pat_end(p, sav);
			*t = '\0';
			if (pattry(p, s)) {
			    mpos = patinput;
			    *t = sav;
			    break;
			}
			*t = sav;
		    }
		}
		*sp = get_match_ret(*sp, 0, mpos-s, fl, replstr);
		return 1;
	    }
	    break;

	case SUB_END:
	    /* Smallest possible match at tail of string:  *
	     * move back down string until we get a match. *
	     * There's no optimization here.               */
	    patoffset = ml;
	    for (t = s + l; t >= s; t--, patoffset--) {
		set_pat_start(p, t-s);
		if (pattry(p, t)) {
		    *sp = get_match_ret(*sp, t - s, l, fl, replstr);
		    patoffset = 0;
		    return 1;
		}
		if (t > s+1 && t[-2] == Meta)
		    t--;
	    }
	    patoffset = 0;
	    break;

	case (SUB_END|SUB_LONG):
	    /* Largest possible match at tail of string:       *
	     * move forward along string until we get a match. *
	     * Again there's no optimisation.                  */
	    for (i = 0, t = s; i < l; i++, t++, patoffset++) {
		set_pat_start(p, t-s);
		if (pattry(p, t)) {
		    *sp = get_match_ret(*sp, i, l, fl, replstr);
		    patoffset = 0;
		    return 1;
		}
		if (*t == Meta)
		    i++, t++;
	    }
	    patoffset = 0;
	    break;

	case SUB_SUBSTR:
	    /* Smallest at start, but matching substrings. */
	    set_pat_start(p, l);
	    if (!(fl & SUB_GLOBAL) && pattry(p, s + l) && !--n) {
		*sp = get_match_ret(*sp, 0, 0, fl, replstr);
		return 1;
	    } /* fall through */
	case (SUB_SUBSTR|SUB_LONG):
	    /* longest or smallest at start with substrings */
	    t = s;
	    if (fl & SUB_GLOBAL)
		repllist = newlinklist();
	    do {
		/* loop over all matches for global substitution */
		matched = 0;
		for (; t < s + l; t++, patoffset++) {
		    /* Find the longest match from this position. */
		    set_pat_start(p, t-s);
		    if (pattry(p, t)) {
			char *mpos = patinput;
			if (!(fl & SUB_LONG) && !(p->flags & PAT_PURES)) {
			    char *ptr;
			    for (ptr = t; ptr < mpos; METAINC(ptr)) {
				sav = *ptr;
				set_pat_end(p, sav);
				*ptr = '\0';
				if (pattry(p, t)) {
				    mpos = patinput;
				    *ptr = sav;
				    break;
				}
				*ptr = sav;
			    }
			}
			if (!--n || (n <= 0 && (fl & SUB_GLOBAL))) {
			    *sp = get_match_ret(*sp, t-s, mpos-s, fl, replstr);
			    if (mpos == t)
				METAINC(mpos);
			}
			if (!(fl & SUB_GLOBAL)) {
			    if (n) {
				/*
				 * Looking for a later match: in this case,
				 * we can continue looking for matches from
				 * the next character, even if it overlaps
				 * with what we just found.
				 */
				continue;
			    } else {
				patoffset = 0;
				return 1;
			    }
			}
			/*
			 * For a global match, we need to skip the stuff
			 * which is already marked for replacement.
			 */
			matched = 1;
			for ( ; t < mpos; t++, patoffset++)
			    if (*t == Meta)
				t++;
			break;
		    }
		    if (*t == Meta)
			t++;
		}
	    } while (matched);
	    patoffset = 0;
	    /*
	     * check if we can match a blank string, if so do it
	     * at the start.  Goodness knows if this is a good idea
	     * with global substitution, so it doesn't happen.
	     */
	    set_pat_start(p, l);
	    if ((fl & (SUB_LONG|SUB_GLOBAL)) == SUB_LONG &&
		pattry(p, s + l) && !--n) {
		*sp = get_match_ret(*sp, 0, 0, fl, replstr);
		return 1;
	    }
	    break;

	case (SUB_END|SUB_SUBSTR):
	case (SUB_END|SUB_LONG|SUB_SUBSTR):
	    /* Longest/shortest at end, matching substrings.       */
	    patoffset = ml;
	    if (!(fl & SUB_LONG)) {
		set_pat_start(p, l);
		if (pattry(p, s + l) && !--n) {
		    *sp = get_match_ret(*sp, l, l, fl, replstr);
		    patoffset = 0;
		    return 1;
		}
	    }
	    patoffset--;
	    for (t = s + l - 1; t >= s; t--, patoffset--) {
		if (t > s && t[-1] == Meta)
		    t--;
		set_pat_start(p, t-s);
		if (pattry(p, t) && !--n) {
		    /* Found the longest match */
		    char *mpos = patinput;
		    if (!(fl & SUB_LONG) && !(p->flags & PAT_PURES)) {
			char *ptr;
			for (ptr = t; ptr < mpos; METAINC(ptr)) {
			    sav = *ptr;
			    set_pat_end(p, sav);
			    *ptr = '\0';
			    if (pattry(p, t)) {
				mpos = patinput;
				*ptr = sav;
				break;
			    }
			    *ptr = sav;
			}
		    }
		    *sp = get_match_ret(*sp, t-s, mpos-s, fl, replstr);
		    patoffset = 0;
		    return 1;
		}
	    }
	    patoffset = ml;
	    set_pat_start(p, l);
	    if ((fl & SUB_LONG) && pattry(p, s + l) && !--n) {
		*sp = get_match_ret(*sp, l, l, fl, replstr);
		patoffset = 0;
		return 1;
	    }
	    patoffset = 0;
	    break;
	}
    }

    if (repllist && nonempty(repllist)) {
	/* Put all the bits of a global search and replace together. */
	LinkNode nd;
	Repldata rd;
	int lleft = 0;		/* size of returned string */
	char *ptr, *start;

	i = 0;			/* start of last chunk we got from *sp */
	for (nd = firstnode(repllist); nd; incnode(nd)) {
	    rd = (Repldata) getdata(nd);
	    lleft += rd->b - i; /* previous chunk of *sp */
	    lleft += strlen(rd->replstr);	/* the replaced bit */
	    i = rd->e;		/* start of next chunk of *sp */
	}
	lleft += l - i;	/* final chunk from *sp */
	start = t = zhalloc(lleft+1);
	i = 0;
	for (nd = firstnode(repllist); nd; incnode(nd)) {
	    rd = (Repldata) getdata(nd);
	    memcpy(t, s + i, rd->b - i);
	    t += rd->b - i;
	    ptr = rd->replstr;
	    while (*ptr)
		*t++ = *ptr++;
	    i = rd->e;
	}
	memcpy(t, s + i, l - i);
	start[lleft] = '\0';
	*sp = (char *)start;
	return 1;
    }

    /* munge the whole string: no match, so no replstr */
    *sp = get_match_ret(*sp, 0, 0, fl, 0);
    return 1;
}

/* blindly turn a string into a tokenised expression without lexing */

/**/
mod_export void
tokenize(char *s)
{
    char *t;
    int bslash = 0;

    for (; *s; s++) {
      cont:
	switch (*s) {
	case Bnull:
	case '\\':
	    if (bslash) {
		s[-1] = Bnull;
		break;
	    }
	    bslash = 1;
	    continue;
	case '<':
	    if (isset(SHGLOB))
		break;
	    if (bslash) {
		s[-1] = Bnull;
		break;
	    }
	    t = s;
	    while (idigit(*++s));
	    if (*s != '-')
		goto cont;
	    while (idigit(*++s));
	    if (*s != '>')
		goto cont;
	    *t = Inang;
	    *s = Outang;
	    break;
	case '(':
	case '|':
	case ')':
	    if (isset(SHGLOB))
		break;
	case '>':
	case '^':
	case '#':
	case '~':
	case '[':
	case ']':
	case '*':
	case '?':
	case '=':
	    for (t = ztokens; *t; t++)
		if (*t == *s) {
		    if (bslash)
			s[-1] = Bnull;
		    else
			*s = (t - ztokens) + Pound;
		    break;
		}
	}
	bslash = 0;
    }
}

/* remove unnecessary Nulargs */

/**/
mod_export void
remnulargs(char *s)
{
    if (*s) {
	char *o = s, c;

	while ((c = *s++))
	    if (INULL(c)) {
		char *t = s - 1;

		while ((c = *s++))
		    if (!INULL(c))
			*t++ = c;
		*t = '\0';
		if (!*o) {
		    o[0] = Nularg;
		    o[1] = '\0';
		}
		break;
	    }
    }
}

/* qualifier functions:  mostly self-explanatory, see glob(). */

/* device number */

/**/
static int
qualdev(char *name, struct stat *buf, off_t dv, char *dummy)
{
    return buf->st_dev == dv;
}

/* number of hard links to file */

/**/
static int
qualnlink(char *name, struct stat *buf, off_t ct, char *dummy)
{
    return (g_range < 0 ? buf->st_nlink < ct :
	    g_range > 0 ? buf->st_nlink > ct :
	    buf->st_nlink == ct);
}

/* user ID */

/**/
static int
qualuid(char *name, struct stat *buf, off_t uid, char *dummy)
{
    return buf->st_uid == uid;
}

/* group ID */

/**/
static int
qualgid(char *name, struct stat *buf, off_t gid, char *dummy)
{
    return buf->st_gid == gid;
}

/* device special file? */

/**/
static int
qualisdev(char *name, struct stat *buf, off_t junk, char *dummy)
{
    return S_ISBLK(buf->st_mode) || S_ISCHR(buf->st_mode);
}

/* block special file? */

/**/
static int
qualisblk(char *name, struct stat *buf, off_t junk, char *dummy)
{
    return S_ISBLK(buf->st_mode);
}

/* character special file? */

/**/
static int
qualischr(char *name, struct stat *buf, off_t junk, char *dummy)
{
    return S_ISCHR(buf->st_mode);
}

/* directory? */

/**/
static int
qualisdir(char *name, struct stat *buf, off_t junk, char *dummy)
{
    return S_ISDIR(buf->st_mode);
}

/* FIFO? */

/**/
static int
qualisfifo(char *name, struct stat *buf, off_t junk, char *dummy)
{
    return S_ISFIFO(buf->st_mode);
}

/* symbolic link? */

/**/
static int
qualislnk(char *name, struct stat *buf, off_t junk, char *dummy)
{
    return S_ISLNK(buf->st_mode);
}

/* regular file? */

/**/
static int
qualisreg(char *name, struct stat *buf, off_t junk, char *dummy)
{
    return S_ISREG(buf->st_mode);
}

/* socket? */

/**/
static int
qualissock(char *name, struct stat *buf, off_t junk, char *dummy)
{
    return S_ISSOCK(buf->st_mode);
}

/* given flag is set in mode */

/**/
static int
qualflags(char *name, struct stat *buf, off_t mod, char *dummy)
{
    return mode_to_octal(buf->st_mode) & mod;
}

/* mode matches specification */

/**/
static int
qualmodeflags(char *name, struct stat *buf, off_t mod, char *dummy)
{
    long v = mode_to_octal(buf->st_mode), y = mod & 07777, n = mod >> 12;

    return ((v & y) == y && !(v & n));
}

/* regular executable file? */

/**/
static int
qualiscom(char *name, struct stat *buf, off_t mod, char *dummy)
{
    return S_ISREG(buf->st_mode) && (buf->st_mode & S_IXUGO);
}

/* size in required range? */

/**/
static int
qualsize(char *name, struct stat *buf, off_t size, char *dummy)
{
#if defined(LONG_IS_64_BIT) || defined(OFF_T_IS_64_BIT)
# define QS_CAST_SIZE()
    off_t scaled = buf->st_size;
#else
# define QS_CAST_SIZE() (unsigned long)
    unsigned long scaled = (unsigned long)buf->st_size;
#endif

    switch (g_units) {
    case TT_POSIX_BLOCKS:
	scaled += 511l;
	scaled /= 512l;
	break;
    case TT_KILOBYTES:
	scaled += 1023l;
	scaled /= 1024l;
	break;
    case TT_MEGABYTES:
	scaled += 1048575l;
	scaled /= 1048576l;
	break;
    }

    return (g_range < 0 ? scaled < QS_CAST_SIZE() size :
	    g_range > 0 ? scaled > QS_CAST_SIZE() size :
	    scaled == QS_CAST_SIZE() size);
#undef QS_CAST_SIZE
}

/* time in required range? */

/**/
static int
qualtime(char *name, struct stat *buf, off_t days, char *dummy)
{
    time_t now, diff;

    time(&now);
    diff = now - (g_amc == 0 ? buf->st_atime : g_amc == 1 ? buf->st_mtime :
		  buf->st_ctime);
    /* handle multipliers indicating units */
    switch (g_units) {
    case TT_DAYS:
	diff /= 86400l;
	break;
    case TT_HOURS:
	diff /= 3600l;
	break;
    case TT_MINS:
	diff /= 60l;
	break;
    case TT_WEEKS:
	diff /= 604800l;
	break;
    case TT_MONTHS:
	diff /= 2592000l;
	break;
    }

    return (g_range < 0 ? diff < days :
	    g_range > 0 ? diff > days :
	    diff == days);
}

/* evaluate a string */

/**/
static int
qualsheval(char *name, struct stat *buf, off_t days, char *str)
{
    Eprog prog;

    if ((prog = parse_string(str, 0))) {
	int ef = errflag, lv = lastval, ret;

	unsetparam("reply");
	setsparam("REPLY", ztrdup(name));

	execode(prog, 1, 0);

	ret = lastval;
	errflag = ef;
	lastval = lv;

	if (!(inserts = getaparam("reply")) &&
	    !(inserts = gethparam("reply"))) {
	    char *tmp;

	    if ((tmp = getsparam("reply")) || (tmp = getsparam("REPLY"))) {
		static char *tmparr[2];

		tmparr[0] = tmp;
		tmparr[1] = NULL;

		inserts = tmparr;
	    }
	}
	return !ret;
    }
    return 0;
}
