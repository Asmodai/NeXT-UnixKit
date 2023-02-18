/*
 * comp.h - header file for completion
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

typedef struct cmatcher  *Cmatcher;
typedef struct cmlist    *Cmlist;
typedef struct cpattern  *Cpattern;
typedef struct menuinfo  *Menuinfo;
typedef struct cexpl *Cexpl;
typedef struct cmgroup *Cmgroup;
typedef struct cmatch *Cmatch;

/* This is for explantion strings. */

struct cexpl {
    char *str;			/* the string */
    int count;			/* the number of matches */
    int fcount;			/* number of matches with fignore ignored */
};

/* This describes a group of matches. */

struct cmgroup {
    char *name;			/* the name of this group */
    Cmgroup prev;		/* previous on the list */
    Cmgroup next;		/* next one in list */
    int flags;			/* see CGF_* below */
    int mcount;			/* number of matches */
    Cmatch *matches;		/* the matches */
    int lcount;			/* number of things to list here */
    int llcount;		/* number of line-displays */
    char **ylist;		/* things to list */
    int ecount;			/* number of explanation string */
    Cexpl *expls;		/* explanation strings */
    int ccount;			/* number of compctls used */
    LinkList lexpls;		/* list of explanation string while building */
    LinkList lmatches;		/* list of matches */
    LinkList lfmatches;		/* list of matches without fignore */
    LinkList lallccs;		/* list of used compctls */
    int num;			/* number of this group */
    int nbrbeg;			/* number of opened braces */
    int nbrend;			/* number of closed braces */
    int new;			/* new matches since last permalloc() */
    /* The following is collected/used during listing. */
    int dcount;			/* number of matches to list in columns */
    int cols;			/* number of columns */
    int lins;			/* number of lines */
    int width;			/* column width */
    int *widths;		/* column widths for listpacked */
    int totl;			/* total length */
    int shortest;		/* length of shortest match */
    Cmgroup perm;		/* perm. alloced version of this group */
};


#define CGF_NOSORT   1		/* don't sort this group */
#define CGF_LINES    2		/* these are to be printed on different lines */
#define CGF_HASDL    4		/* has display strings printed on separate lines */
#define CGF_UNIQALL  8		/* remove all duplicates */
#define CGF_UNIQCON 16		/* remove consecutive duplicates */
#define CGF_PACKED  32		/* LIST_PACKED for this group */
#define CGF_ROWS    64		/* LIST_ROWS_FIRST for this group */

/* This is the struct used to hold matches. */

struct cmatch {
    char *str;			/* the match itself */
    char *ipre;			/* ignored prefix, has to be re-inserted */
    char *ripre;		/* ignored prefix, unquoted */
    char *isuf;			/* ignored suffix */
    char *ppre;			/* the path prefix */
    char *psuf;			/* the path suffix */
    char *prpre;		/* path prefix for opendir */
    char *pre;			/* prefix string from -P */
    char *suf;			/* suffix string from -S */
    char *disp;			/* string to display (compadd -d) */
    char *autoq;		/* closing quote to add automatically */
    int flags;			/* see CMF_* below */
    int *brpl;			/* places where to put the brace prefixes */
    int *brsl;			/* ...and the suffixes */
    char *rems;			/* when to remove the suffix */
    char *remf;			/* shell function to call for suffix-removal */
    int qipl;			/* length of quote-prefix */
    int qisl;			/* length of quote-suffix */
    int rnum;			/* group relative number */
    int gnum;			/* global number */
};

#define CMF_FILE     (1<< 0)	/* this is a file */
#define CMF_REMOVE   (1<< 1)	/* remove the suffix */
#define CMF_ISPAR    (1<< 2)	/* is paramter expansion */
#define CMF_PARBR    (1<< 3)	/* paramter expansion with a brace */
#define CMF_PARNEST  (1<< 4)	/* nested paramter expansion */
#define CMF_NOLIST   (1<< 5)	/* should not be listed */
#define CMF_DISPLINE (1<< 6)	/* display strings one per line */
#define CMF_HIDE     (1<< 7)	/* temporarily hide this one */
#define CMF_NOSPACE  (1<< 8)	/* don't add a space */
#define CMF_PACKED   (1<< 9)	/* prefer LIST_PACKED */
#define CMF_ROWS     (1<<10)	/* prefer LIST_ROWS_FIRST */
#define CMF_MULT     (1<<11)	/* string appears more than once */
#define CMF_FMULT    (1<<12)	/* first of multiple equal strings */
#define CMF_ALL      (1<<13)	/* a match representing all other matches */

/* Stuff for completion matcher control. */

struct cmlist {
    Cmlist next;		/* next one in the list of global matchers */
    Cmatcher matcher;		/* the matcher definition */
    char *str;			/* the string for it */
};

struct cmatcher {
    int refc;			/* reference counter */
    Cmatcher next;		/* next matcher */
    int flags;			/* see CMF_* below */
    Cpattern line;		/* what matches on the line */
    int llen;			/* length of line pattern */
    Cpattern word;		/* what matches in the word */
    int wlen;			/* length of word pattern */
    Cpattern left;		/* left anchor */
    int lalen;			/* length of left anchor */
    Cpattern right;		/* right anchor */
    int ralen;			/* length of right anchor */
};

#define CMF_LINE  1
#define CMF_LEFT  2
#define CMF_RIGHT 4
#define CMF_INTER 8

struct cpattern {
    Cpattern next;		/* next sub-pattern */
    unsigned char tab[256];	/* table of matched characters */
    int equiv;			/* if this is a {...} class */
};

/* This is a special return value for parse_cmatcher(), *
 * signalling an error. */

#define pcm_err ((Cmatcher) 1)

/* Information about what to put on the line as the unambiguous string.
 * The code always keeps lists of these structs up to date while
 * matches are added (in the aminfo structs below).
 * The lists have two levels: in the first one we have one struct per
 * word-part, where parts are separated by the anchors of `*' patterns.
 * These structs have pointers (in the prefix and suffix fields) to
 * lists of cline structs describing the strings before or after the
 * the anchor. */

typedef struct cline *Cline;
typedef struct clsub Clsub;

struct cline {
    Cline next;
    int flags;
    char *line;
    int llen;
    char *word;
    int wlen;
    char *orig;
    int olen;
    int slen;
    Cline prefix, suffix;
    int min, max;
};

#define CLF_MISS      1
#define CLF_DIFF      2
#define CLF_SUF       4
#define CLF_MID       8
#define CLF_NEW      16
#define CLF_LINE     32
#define CLF_JOIN     64
#define CLF_MATCHED 128
#define CLF_SKIP    256

/* Information for ambiguous completions. One for fignore ignored and   *
 * one for normal completion. */

typedef struct aminfo *Aminfo;

struct aminfo {
    Cmatch firstm;		/* the first match                        */
    int exact;			/* if there was an exact match            */
    Cmatch exactm;		/* the exact match (if any)               */
    int count;			/* number of matches                      */
    Cline line;			/* unambiguous line string                */
};

/* Information about menucompletion stuff. */

struct menuinfo {
    Cmgroup group;		/* position in the group list */
    Cmatch *cur;		/* match currently inserted */
    int pos;			/* begin on line */
    int len;			/* length of inserted string */
    int end;			/* end on the line */
    int we;			/* non-zero if the cursor was at the end */
    int insc;			/* length of suffix inserted */
    int asked;			/* we asked if the list should be shown */
    char *prebr;		/* prefix before a brace, if any */
    char *postbr;		/* suffix after a brace */
};

/* Flags for compadd and addmatches(). */

#define CAF_QUOTE    1
#define CAF_NOSORT   2
#define CAF_MATCH    4
#define CAF_UNIQCON  8
#define CAF_UNIQALL 16
#define CAF_ARRAYS  32
#define CAF_KEYS    64
#define CAF_ALL    128

/* Data for compadd and addmatches() */

typedef struct cadata *Cadata;

struct cadata {
    char *ipre;			/* ignored prefix (-i) */
    char *isuf;			/* ignored suffix (-I) */
    char *ppre;			/* `path' prefix (-p) */
    char *psuf;			/* `path' suffix (-s) */
    char *prpre;		/* expanded `path' prefix (-W) */
    char *pre;			/* prefix to insert (-P) */
    char *suf;			/* suffix to insert (-S) */
    char *group;		/* name of the group (-[JV]) */
    char *rems;			/* remove suffix on chars... (-r) */
    char *remf;			/* function to remove suffix (-R) */
    char *ign;			/* ignored suffixes (-F) */
    int flags;			/* CMF_* flags (-[fqn]) */
    int aflags;			/* CAF_* flags (-[QUa]) */
    Cmatcher match;		/* match spec (parsed from -M) */
    char *exp;			/* explanation (-X) */
    char *apar;			/* array to store matches in (-A) */
    char *opar;			/* array to store originals in (-O) */
    char *dpar;			/* array to delete non-matches in (-D) */
    char *disp;			/* array with display lists (-d) */
    char *mesg;			/* message to show unconditionally (-x) */
};

/* List data. */

typedef struct cldata *Cldata;

struct cldata {
    int columns;		/* screen width */
    int lines;			/* screen height */
    int menuacc;		/* value of global menuacc */
    int valid;			/* no need to calculate anew */
    int nlist;			/* number of matches to list */
    int nlines;			/* number of lines needed */
    int hidden;			/* != 0 if there are hidden matches */
    int onlyexpl;		/* != 0 if only explanations to print */
    int showall;		/* != 0 if hidden matches should be shown */
};

typedef void (*CLPrintFunc)(Cmgroup, Cmatch *, int, int, int, int,
			    char *, struct stat *);

/* Flags for fromcomp. */

#define FC_LINE   1
#define FC_INWORD 2

/* Flags for special parameters. */

#define CPN_WORDS      0
#define CP_WORDS       (1 <<  CPN_WORDS)
#define CPN_CURRENT    1
#define CP_CURRENT     (1 <<  CPN_CURRENT)
#define CPN_PREFIX     2
#define CP_PREFIX      (1 <<  CPN_PREFIX)
#define CPN_SUFFIX     3
#define CP_SUFFIX      (1 <<  CPN_SUFFIX)
#define CPN_IPREFIX    4
#define CP_IPREFIX     (1 <<  CPN_IPREFIX)
#define CPN_ISUFFIX    5
#define CP_ISUFFIX     (1 <<  CPN_ISUFFIX)
#define CPN_QIPREFIX   6
#define CP_QIPREFIX    (1 <<  CPN_QIPREFIX)
#define CPN_QISUFFIX   7
#define CP_QISUFFIX    (1 <<  CPN_QISUFFIX)
#define CPN_COMPSTATE  8
#define CP_COMPSTATE   (1 <<  CPN_COMPSTATE)

#define CP_REALPARAMS  9
#define CP_ALLREALS    ((unsigned int) 0x1ff)


#define CPN_NMATCHES   0
#define CP_NMATCHES    (1 << CPN_NMATCHES)
#define CPN_CONTEXT    1
#define CP_CONTEXT     (1 << CPN_CONTEXT)
#define CPN_PARAMETER  2
#define CP_PARAMETER   (1 << CPN_PARAMETER)
#define CPN_REDIRECT   3
#define CP_REDIRECT    (1 << CPN_REDIRECT)
#define CPN_QUOTE      4
#define CP_QUOTE       (1 << CPN_QUOTE)
#define CPN_QUOTING    5
#define CP_QUOTING     (1 << CPN_QUOTING)
#define CPN_RESTORE    6
#define CP_RESTORE     (1 << CPN_RESTORE)
#define CPN_LIST       7
#define CP_LIST        (1 << CPN_LIST)
#define CPN_INSERT     8
#define CP_INSERT      (1 << CPN_INSERT)
#define CPN_EXACT      9
#define CP_EXACT       (1 << CPN_EXACT)
#define CPN_EXACTSTR   10
#define CP_EXACTSTR    (1 << CPN_EXACTSTR)
#define CPN_PATMATCH   11
#define CP_PATMATCH    (1 << CPN_PATMATCH)
#define CPN_PATINSERT  12
#define CP_PATINSERT   (1 << CPN_PATINSERT)
#define CPN_UNAMBIG    13
#define CP_UNAMBIG     (1 << CPN_UNAMBIG)
#define CPN_UNAMBIGC   14
#define CP_UNAMBIGC    (1 << CPN_UNAMBIGC)
#define CPN_UNAMBIGP   15
#define CP_UNAMBIGP    (1 << CPN_UNAMBIGP)
#define CPN_INSERTP    16
#define CP_INSERTP     (1 << CPN_INSERTP)
#define CPN_LISTMAX    17
#define CP_LISTMAX     (1 << CPN_LISTMAX)
#define CPN_LASTPROMPT 18
#define CP_LASTPROMPT  (1 << CPN_LASTPROMPT)
#define CPN_TOEND      19
#define CP_TOEND       (1 << CPN_TOEND)
#define CPN_OLDLIST    20
#define CP_OLDLIST     (1 << CPN_OLDLIST)
#define CPN_OLDINS     21
#define CP_OLDINS      (1 << CPN_OLDINS)
#define CPN_VARED      22
#define CP_VARED       (1 << CPN_VARED)
#define CPN_LISTLINES  23
#define CP_LISTLINES   (1 << CPN_LISTLINES)
#define CPN_QUOTES     24
#define CP_QUOTES      (1 << CPN_QUOTES)
#define CPN_IGNORED    25
#define CP_IGNORED     (1 << CPN_IGNORED)

#define CP_KEYPARAMS   26
#define CP_ALLKEYS     ((unsigned int) 0x3ffffff)

/* Hooks. */

#define INSERTMATCHHOOK     (comphooks + 0)
#define MENUSTARTHOOK       (comphooks + 1)
#define COMPCTLMAKEHOOK     (comphooks + 2)
#define COMPCTLCLEANUPHOOK  (comphooks + 3)
#define COMPLISTMATCHESHOOK (comphooks + 4)

/* compctl hook data struct */

struct ccmakedat {
    char *str;
    int incmd;
    int lst;
};

/* Data given to offered hooks. */

typedef struct chdata *Chdata;

struct chdata {
    Cmgroup matches;		/* the matches generated */
    int num;			/* the number of matches */
    int nmesg;			/* the number of messages */
    Cmatch cur;			/* current match or NULL */
};

