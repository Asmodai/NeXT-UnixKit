/*
 * options.c - shell options
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
#include "options.pro"

/* current emulation (used to decide which set of option letters is used) */

/**/
int emulation;
 
/* the options; e.g. if opts[SHGLOB] != 0, SH_GLOB is turned on */
 
/**/
mod_export char opts[OPT_SIZE];
 
/* Option name hash table */

/**/
mod_export HashTable optiontab;
 
/* The canonical option name table */

#define OPT_CSH		EMULATE_CSH
#define OPT_KSH		EMULATE_KSH
#define OPT_SH		EMULATE_SH
#define OPT_ZSH		EMULATE_ZSH

#define OPT_ALL		(OPT_CSH|OPT_KSH|OPT_SH|OPT_ZSH)
#define OPT_BOURNE	(OPT_KSH|OPT_SH)
#define OPT_BSHELL	(OPT_KSH|OPT_SH|OPT_ZSH)
#define OPT_NONBOURNE	(OPT_ALL & ~OPT_BOURNE)
#define OPT_NONZSH	(OPT_ALL & ~OPT_ZSH)

#define OPT_EMULATE	(1<<5)	/* option is relevant to emulation */
#define OPT_SPECIAL	(1<<6)	/* option should never be set by emulate() */
#define OPT_ALIAS	(1<<7)	/* option is an alias to an other option */

#define defset(X) (!!((X)->flags & emulation))

/*
 * Note that option names should usually be fewer than 20 characters long
 * to avoid formatting problems.
 */
static struct optname optns[] = {
{NULL, "aliases",	      OPT_EMULATE|OPT_ALL,	 ALIASESOPT},
{NULL, "allexport",	      OPT_EMULATE,		 ALLEXPORT},
{NULL, "alwayslastprompt",    OPT_ALL,			 ALWAYSLASTPROMPT},
{NULL, "alwaystoend",	      0,			 ALWAYSTOEND},
{NULL, "appendhistory",	      OPT_ALL,			 APPENDHISTORY},
{NULL, "autocd",	      OPT_EMULATE,		 AUTOCD},
{NULL, "autolist",	      OPT_ALL,			 AUTOLIST},
{NULL, "automenu",	      OPT_ALL,			 AUTOMENU},
{NULL, "autonamedirs",	      0,			 AUTONAMEDIRS},
{NULL, "autoparamkeys",	      OPT_ALL,			 AUTOPARAMKEYS},
{NULL, "autoparamslash",      OPT_ALL,			 AUTOPARAMSLASH},
{NULL, "autopushd",	      0,			 AUTOPUSHD},
{NULL, "autoremoveslash",     OPT_ALL,			 AUTOREMOVESLASH},
{NULL, "autoresume",	      0,			 AUTORESUME},
{NULL, "badpattern",	      OPT_EMULATE|OPT_NONBOURNE, BADPATTERN},
{NULL, "banghist",	      OPT_NONBOURNE,		 BANGHIST},
{NULL, "bareglobqual",        OPT_EMULATE|OPT_ZSH,       BAREGLOBQUAL},
{NULL, "bashautolist",	      0,                         BASHAUTOLIST},
{NULL, "beep",		      OPT_ALL,			 BEEP},
{NULL, "bgnice",	      OPT_EMULATE|OPT_NONBOURNE, BGNICE},
{NULL, "braceccl",	      OPT_EMULATE,		 BRACECCL},
{NULL, "bsdecho",	      OPT_EMULATE|OPT_SH,	 BSDECHO},
{NULL, "cbases",	      0,			 CBASES},
{NULL, "cdablevars",	      OPT_EMULATE,		 CDABLEVARS},
{NULL, "chasedots",	      OPT_EMULATE,		 CHASEDOTS},
{NULL, "chaselinks",	      OPT_EMULATE,		 CHASELINKS},
{NULL, "checkjobs",	      OPT_EMULATE|OPT_ZSH,	 CHECKJOBS},
{NULL, "clobber",	      OPT_EMULATE|OPT_ALL,	 CLOBBER},
{NULL, "completealiases",     0,			 COMPLETEALIASES},
{NULL, "completeinword",      0,			 COMPLETEINWORD},
{NULL, "correct",	      0,			 CORRECT},
{NULL, "correctall",	      0,			 CORRECTALL},
{NULL, "cshjunkiehistory",    OPT_EMULATE|OPT_CSH,	 CSHJUNKIEHISTORY},
{NULL, "cshjunkieloops",      OPT_EMULATE|OPT_CSH,	 CSHJUNKIELOOPS},
{NULL, "cshjunkiequotes",     OPT_EMULATE|OPT_CSH,	 CSHJUNKIEQUOTES},
{NULL, "cshnullcmd",	      OPT_EMULATE|OPT_CSH,	 CSHNULLCMD},
{NULL, "cshnullglob",	      OPT_EMULATE|OPT_CSH,	 CSHNULLGLOB},
{NULL, "equals",	      OPT_EMULATE|OPT_ZSH,	 EQUALS},
{NULL, "errexit",	      OPT_EMULATE,		 ERREXIT},
{NULL, "exec",		      OPT_ALL,			 EXECOPT},
{NULL, "extendedglob",	      OPT_EMULATE,		 EXTENDEDGLOB},
{NULL, "extendedhistory",     OPT_CSH,			 EXTENDEDHISTORY},
{NULL, "flowcontrol",	      OPT_ALL,			 FLOWCONTROL},
{NULL, "functionargzero",     OPT_EMULATE|OPT_NONBOURNE, FUNCTIONARGZERO},
{NULL, "glob",		      OPT_EMULATE|OPT_ALL,	 GLOBOPT},
{NULL, "globalexport",        OPT_EMULATE|OPT_ZSH,	 GLOBALEXPORT},
{NULL, "globalrcs",           OPT_ALL,			 GLOBALRCS},
{NULL, "globassign",	      OPT_EMULATE|OPT_CSH,	 GLOBASSIGN},
{NULL, "globcomplete",	      0,			 GLOBCOMPLETE},
{NULL, "globdots",	      OPT_EMULATE,		 GLOBDOTS},
{NULL, "globsubst",	      OPT_EMULATE|OPT_NONZSH,	 GLOBSUBST},
{NULL, "hashcmds",	      OPT_ALL,			 HASHCMDS},
{NULL, "hashdirs",	      OPT_ALL,			 HASHDIRS},
{NULL, "hashlistall",	      OPT_ALL,			 HASHLISTALL},
{NULL, "histallowclobber",    0,			 HISTALLOWCLOBBER},
{NULL, "histbeep",	      OPT_ALL,			 HISTBEEP},
{NULL, "histexpiredupsfirst", 0,			 HISTEXPIREDUPSFIRST},
{NULL, "histfindnodups",      0,			 HISTFINDNODUPS},
{NULL, "histignorealldups",   0,			 HISTIGNOREALLDUPS},
{NULL, "histignoredups",      0,			 HISTIGNOREDUPS},
{NULL, "histignorespace",     0,			 HISTIGNORESPACE},
{NULL, "histnofunctions",     0,			 HISTNOFUNCTIONS},
{NULL, "histnostore",	      0,			 HISTNOSTORE},
{NULL, "histreduceblanks",    0,			 HISTREDUCEBLANKS},
{NULL, "histsavenodups",      0,			 HISTSAVENODUPS},
{NULL, "histverify",	      0,			 HISTVERIFY},
{NULL, "hup",		      OPT_EMULATE|OPT_ZSH,	 HUP},
{NULL, "ignorebraces",	      OPT_EMULATE|OPT_SH,	 IGNOREBRACES},
{NULL, "ignoreeof",	      0,			 IGNOREEOF},
{NULL, "incappendhistory",    0,			 INCAPPENDHISTORY},
{NULL, "interactive",	      OPT_SPECIAL,		 INTERACTIVE},
{NULL, "interactivecomments", OPT_BOURNE,		 INTERACTIVECOMMENTS},
{NULL, "ksharrays",	      OPT_EMULATE|OPT_BOURNE,	 KSHARRAYS},
{NULL, "kshautoload",	      OPT_EMULATE|OPT_BOURNE,	 KSHAUTOLOAD},
{NULL, "kshglob",             OPT_EMULATE|OPT_KSH,       KSHGLOB},
{NULL, "kshoptionprint",      OPT_EMULATE|OPT_KSH,	 KSHOPTIONPRINT},
{NULL, "kshtypeset",          OPT_EMULATE|OPT_KSH,	 KSHTYPESET},
{NULL, "listambiguous",	      OPT_ALL,			 LISTAMBIGUOUS},
{NULL, "listbeep",	      OPT_ALL,			 LISTBEEP},
{NULL, "listpacked",	      0,			 LISTPACKED},
{NULL, "listrowsfirst",	      0,			 LISTROWSFIRST},
{NULL, "listtypes",	      OPT_ALL,			 LISTTYPES},
{NULL, "localoptions",	      OPT_EMULATE|OPT_KSH,	 LOCALOPTIONS},
{NULL, "localtraps",	      OPT_EMULATE|OPT_KSH,	 LOCALTRAPS},
{NULL, "login",		      OPT_SPECIAL,		 LOGINSHELL},
{NULL, "longlistjobs",	      0,			 LONGLISTJOBS},
{NULL, "magicequalsubst",     OPT_EMULATE,		 MAGICEQUALSUBST},
{NULL, "mailwarning",	      0,			 MAILWARNING},
{NULL, "markdirs",	      0,			 MARKDIRS},
{NULL, "menucomplete",	      0,			 MENUCOMPLETE},
{NULL, "monitor",	      OPT_SPECIAL,		 MONITOR},
{NULL, "multios",	      OPT_EMULATE|OPT_ZSH,	 MULTIOS},
{NULL, "nomatch",	      OPT_EMULATE|OPT_NONBOURNE, NOMATCH},
{NULL, "notify",	      OPT_ZSH,			 NOTIFY},
{NULL, "nullglob",	      OPT_EMULATE,		 NULLGLOB},
{NULL, "numericglobsort",     OPT_EMULATE,		 NUMERICGLOBSORT},
{NULL, "octalzeroes",         OPT_EMULATE|OPT_SH,	 OCTALZEROES},
{NULL, "overstrike",	      0,			 OVERSTRIKE},
{NULL, "pathdirs",	      OPT_EMULATE,		 PATHDIRS},
{NULL, "posixbuiltins",	      OPT_EMULATE|OPT_BOURNE,	 POSIXBUILTINS},
{NULL, "printeightbit",       0,                         PRINTEIGHTBIT},
{NULL, "printexitvalue",      0,			 PRINTEXITVALUE},
{NULL, "privileged",	      OPT_SPECIAL,		 PRIVILEGED},
{NULL, "promptbang",	      OPT_KSH,			 PROMPTBANG},
{NULL, "promptcr",	      OPT_ALL,			 PROMPTCR},
{NULL, "promptpercent",	      OPT_NONBOURNE,		 PROMPTPERCENT},
{NULL, "promptsubst",	      OPT_KSH,			 PROMPTSUBST},
{NULL, "pushdignoredups",     OPT_EMULATE,		 PUSHDIGNOREDUPS},
{NULL, "pushdminus",	      OPT_EMULATE,		 PUSHDMINUS},
{NULL, "pushdsilent",	      0,			 PUSHDSILENT},
{NULL, "pushdtohome",	      OPT_EMULATE,		 PUSHDTOHOME},
{NULL, "rcexpandparam",	      OPT_EMULATE,		 RCEXPANDPARAM},
{NULL, "rcquotes",	      OPT_EMULATE,		 RCQUOTES},
{NULL, "rcs",		      OPT_ALL,			 RCS},
{NULL, "recexact",	      0,			 RECEXACT},
{NULL, "restricted",	      OPT_SPECIAL,		 RESTRICTED},
{NULL, "rmstarsilent",	      OPT_BOURNE,		 RMSTARSILENT},
{NULL, "rmstarwait",	      0,			 RMSTARWAIT},
{NULL, "sharehistory",	      OPT_KSH,			 SHAREHISTORY},
{NULL, "shfileexpansion",     OPT_EMULATE|OPT_BOURNE,	 SHFILEEXPANSION},
{NULL, "shglob",	      OPT_EMULATE|OPT_BOURNE,	 SHGLOB},
{NULL, "shinstdin",	      OPT_SPECIAL,		 SHINSTDIN},
{NULL, "shnullcmd",           OPT_EMULATE|OPT_BOURNE,	 SHNULLCMD},
{NULL, "shoptionletters",     OPT_EMULATE|OPT_BOURNE,	 SHOPTIONLETTERS},
{NULL, "shortloops",	      OPT_EMULATE|OPT_NONBOURNE, SHORTLOOPS},
{NULL, "shwordsplit",	      OPT_EMULATE|OPT_BOURNE,	 SHWORDSPLIT},
{NULL, "singlecommand",	      OPT_SPECIAL,		 SINGLECOMMAND},
{NULL, "singlelinezle",	      OPT_KSH,			 SINGLELINEZLE},
{NULL, "sunkeyboardhack",     0,			 SUNKEYBOARDHACK},
{NULL, "unset",		      OPT_EMULATE|OPT_BSHELL,	 UNSET},
{NULL, "verbose",	      0,			 VERBOSE},
{NULL, "xtrace",	      0,			 XTRACE},
{NULL, "zle",		      OPT_SPECIAL,		 USEZLE},
{NULL, "braceexpand",	      OPT_ALIAS, /* ksh/bash */	 -IGNOREBRACES},
{NULL, "dotglob",	      OPT_ALIAS, /* bash */	 GLOBDOTS},
{NULL, "hashall",	      OPT_ALIAS, /* bash */	 HASHCMDS},
{NULL, "histappend",	      OPT_ALIAS, /* bash */	 APPENDHISTORY},
{NULL, "histexpand",	      OPT_ALIAS, /* bash */	 BANGHIST},
{NULL, "log",		      OPT_ALIAS, /* ksh */	 -HISTNOFUNCTIONS},
{NULL, "mailwarn",	      OPT_ALIAS, /* bash */	 MAILWARNING},
{NULL, "onecmd",	      OPT_ALIAS, /* bash */	 SINGLECOMMAND},
{NULL, "physical",	      OPT_ALIAS, /* ksh/bash */	 CHASELINKS},
{NULL, "promptvars",	      OPT_ALIAS, /* bash */	 PROMPTSUBST},
{NULL, "stdin",		      OPT_ALIAS, /* ksh */	 SHINSTDIN},
{NULL, "trackall",	      OPT_ALIAS, /* ksh */	 HASHCMDS},
{NULL, "dvorak",	      0,			 DVORAK},
{NULL, NULL, 0, 0}
};

/* Option letters */

#define optletters (isset(SHOPTIONLETTERS) ? kshletters : zshletters)

#define FIRST_OPT '0'
#define LAST_OPT 'y'

static short zshletters[LAST_OPT - FIRST_OPT + 1] = {
    /* 0 */  CORRECT,
    /* 1 */  PRINTEXITVALUE,
    /* 2 */ -BADPATTERN,
    /* 3 */ -NOMATCH,
    /* 4 */  GLOBDOTS,
    /* 5 */  NOTIFY,
    /* 6 */  BGNICE,
    /* 7 */  IGNOREEOF,
    /* 8 */  MARKDIRS,
    /* 9 */  AUTOLIST,
    /* : */  0,
    /* ; */  0,
    /* < */  0,
    /* = */  0,
    /* > */  0,
    /* ? */  0,
    /* @ */  0,
    /* A */  0,			/* use with set for arrays */
    /* B */ -BEEP,
    /* C */ -CLOBBER,
    /* D */  PUSHDTOHOME,
    /* E */  PUSHDSILENT,
    /* F */ -GLOBOPT,
    /* G */  NULLGLOB,
    /* H */  RMSTARSILENT,
    /* I */  IGNOREBRACES,
    /* J */  AUTOCD,
    /* K */ -BANGHIST,
    /* L */  SUNKEYBOARDHACK,
    /* M */  SINGLELINEZLE,
    /* N */  AUTOPUSHD,
    /* O */  CORRECTALL,
    /* P */  RCEXPANDPARAM,
    /* Q */  PATHDIRS,
    /* R */  LONGLISTJOBS,
    /* S */  RECEXACT,
    /* T */  CDABLEVARS,
    /* U */  MAILWARNING,
    /* V */ -PROMPTCR,
    /* W */  AUTORESUME,
    /* X */  LISTTYPES,
    /* Y */  MENUCOMPLETE,
    /* Z */  USEZLE,
    /* [ */  0,
    /* \ */  0,
    /* ] */  0,
    /* ^ */  0,
    /* _ */  0,
    /* ` */  0,
    /* a */  ALLEXPORT,
    /* b */  0,			/* in non-Bourne shells, end of options */
    /* c */  0,			/* command follows */
    /* d */ -GLOBALRCS,
    /* e */  ERREXIT,
    /* f */ -RCS,
    /* g */  HISTIGNORESPACE,
    /* h */  HISTIGNOREDUPS,
    /* i */  INTERACTIVE,
    /* j */  0,
    /* k */  INTERACTIVECOMMENTS,
    /* l */  LOGINSHELL,
    /* m */  MONITOR,
    /* n */ -EXECOPT,
    /* o */  0,			/* long option name follows */
    /* p */  PRIVILEGED,
    /* q */  0,
    /* r */  RESTRICTED,
    /* s */  SHINSTDIN,
    /* t */  SINGLECOMMAND,
    /* u */ -UNSET,
    /* v */  VERBOSE,
    /* w */  CHASELINKS,
    /* x */  XTRACE,
    /* y */  SHWORDSPLIT,
};

static short kshletters[LAST_OPT - FIRST_OPT + 1] = {
    /* 0 */  0,
    /* 1 */  0,
    /* 2 */  0,
    /* 3 */  0,
    /* 4 */  0,
    /* 5 */  0,
    /* 6 */  0,
    /* 7 */  0,
    /* 8 */  0,
    /* 9 */  0,
    /* : */  0,
    /* ; */  0,
    /* < */  0,
    /* = */  0,
    /* > */  0,
    /* ? */  0,
    /* @ */  0,
    /* A */  0,
    /* B */  0,
    /* C */ -CLOBBER,
    /* D */  0,
    /* E */  0,
    /* F */  0,
    /* G */  0,
    /* H */  0,
    /* I */  0,
    /* J */  0,
    /* K */  0,
    /* L */  0,
    /* M */  0,
    /* N */  0,
    /* O */  0,
    /* P */  0,
    /* Q */  0,
    /* R */  0,
    /* S */  0,
    /* T */  0,
    /* U */  0,
    /* V */  0,
    /* W */  0,
    /* X */  MARKDIRS,
    /* Y */  0,
    /* Z */  0,
    /* [ */  0,
    /* \ */  0,
    /* ] */  0,
    /* ^ */  0,
    /* _ */  0,
    /* ` */  0,
    /* a */  ALLEXPORT,
    /* b */  NOTIFY,
    /* c */  0,
    /* d */  0,
    /* e */  ERREXIT,
    /* f */ -GLOBOPT,
    /* g */  0,
    /* h */  0,
    /* i */  INTERACTIVE,
    /* j */  0,
    /* k */  0,
    /* l */  LOGINSHELL,
    /* m */  MONITOR,
    /* n */ -EXECOPT,
    /* o */  0,
    /* p */  PRIVILEGED,
    /* q */  0,
    /* r */  RESTRICTED,
    /* s */  SHINSTDIN,
    /* t */  SINGLECOMMAND,
    /* u */ -UNSET,
    /* v */  VERBOSE,
    /* w */  0,
    /* x */  XTRACE,
    /* y */  0,
};

/* Initialisation of the option name hash table */

/**/
static void
printoptionnode(HashNode hn, int set)
{
    Optname on = (Optname) hn;
    int optno = on->optno;

    if (optno < 0)
	optno = -optno;
    if (isset(KSHOPTIONPRINT)) {
	if (defset(on))
	    printf("no%-19s %s\n", on->nam, isset(optno) ? "off" : "on");
	else
	    printf("%-21s %s\n", on->nam, isset(optno) ? "on" : "off");
    } else if (set == (isset(optno) ^ defset(on))) {
	if (set ^ isset(optno))
	    fputs("no", stdout);
	puts(on->nam);
    }
}

/**/
void
createoptiontable(void)
{
    Optname on;

    optiontab = newhashtable(101, "optiontab", NULL);

    optiontab->hash        = hasher;
    optiontab->emptytable  = NULL;
    optiontab->filltable   = NULL;
    optiontab->cmpnodes    = strcmp;
    optiontab->addnode     = addhashnode;
    optiontab->getnode     = gethashnode;
    optiontab->getnode2    = gethashnode2;
    optiontab->removenode  = NULL;
    optiontab->disablenode = disablehashnode;
    optiontab->enablenode  = enablehashnode;
    optiontab->freenode    = NULL;
    optiontab->printnode   = printoptionnode;

    for (on = optns; on->nam; on++)
	optiontab->addnode(optiontab, on->nam, on);
}

/* Setting of default options */

/**/
static void
setemulate(HashNode hn, int fully)
{
    Optname on = (Optname) hn;

    /* Set options: each non-special option is set according to the *
     * current emulation mode if either it is considered relevant   *
     * to emulation or we are doing a full emulation (as indicated  *
     * by the `fully' parameter).                                   */
    if (!(on->flags & OPT_ALIAS) &&
	((fully && !(on->flags & OPT_SPECIAL)) ||
	 (on->flags & OPT_EMULATE)))
	opts[on->optno] = defset(on);
}

/**/
void
emulate(const char *zsh_name, int fully)
{
    char ch = *zsh_name;

    if (ch == 'r')
	ch = zsh_name[1];

    /* Work out the new emulation mode */
    if (ch == 'c')
	emulation = EMULATE_CSH;
    else if (ch == 'k')
	emulation = EMULATE_KSH;
    else if (ch == 's' || ch == 'b')
	emulation = EMULATE_SH;
    else
	emulation = EMULATE_ZSH;

    scanhashtable(optiontab, 0, 0, 0, setemulate, fully);
}

/* setopt, unsetopt */

/**/
static void
setoption(HashNode hn, int value)
{
    dosetopt(((Optname) hn)->optno, value, 0);
}

/**/
int
bin_setopt(char *nam, char **args, char *ops, int isun)
{
    int action, optno, match = 0;

    /* With no arguments or options, display options. */
    if (!*args) {
	scanhashtable(optiontab, 1, 0, OPT_ALIAS, optiontab->printnode, !isun);
	return 0;
    }

    /* loop through command line options (begins with "-" or "+") */
    while (*args && (**args == '-' || **args == '+')) {
	action = (**args == '-') ^ isun;
	if(!args[0][1])
	    *args = "--";
	while (*++*args) {
	    if(**args == Meta)
		*++*args ^= 32;
	    /* The pseudo-option `--' signifies the end of options. */
	    if (**args == '-') {
		args++;
		goto doneoptions;
	    } else if (**args == 'o') {
		if (!*++*args)
		    args++;
		if (!*args) {
		    zwarnnam(nam, "string expected after -o", NULL, 0);
		    inittyptab();
		    return 1;
		}
		if(!(optno = optlookup(*args)))
		    zwarnnam(nam, "no such option: %s", *args, 0);
		else if(dosetopt(optno, action, 0))
		    zwarnnam(nam, "can't change option: %s", *args, 0);
		break;
	    } else if(**args == 'm') {
		match = 1;
	    } else {
	    	if (!(optno = optlookupc(**args)))
		    zwarnnam(nam, "bad option: -%c", NULL, **args);
		else if(dosetopt(optno, action, 0))
		    zwarnnam(nam, "can't change option: -%c", NULL, **args);
	    }
	}
	args++;
    }
    doneoptions:

    if (!match) {
	/* Not globbing the arguments -- arguments are simply option names. */
	while (*args) {
	    if(!(optno = optlookup(*args++)))
		zwarnnam(nam, "no such option: %s", args[-1], 0);
	    else if(dosetopt(optno, !isun, 0))
		zwarnnam(nam, "can't change option: %s", args[-1], 0);
	}
    } else {
	/* Globbing option (-m) set. */
	while (*args) {
	    Patprog pprog;
	    char *s, *t;

	    t = s = dupstring(*args);
	    while (*t)
		if (*t == '_')
		    chuck(t);
		else {
		    *t = tulower(*t);
		    t++;
		}

	    /* Expand the current arg. */
	    tokenize(s);
	    if (!(pprog = patcompile(s, PAT_STATIC, NULL))) {
		zwarnnam(nam, "bad pattern: %s", *args, 0);
		continue;
	    }
	    /* Loop over expansions. */
	    scanmatchtable(optiontab, pprog, 0, OPT_ALIAS, setoption, !isun);
	    args++;
	}
    }
    inittyptab();
    return 0;
}

/* Identify an option name */

/**/
mod_export int
optlookup(char const *name)
{
    char *s, *t;
    Optname n;

    s = t = dupstring(name);

    /* exorcise underscores, and change to lowercase */
    while (*t)
	if (*t == '_')
	    chuck(t);
	else {
	    *t = tulower(*t);
	    t++;
	}

    /* look up name in the table */
    if (s[0] == 'n' && s[1] == 'o' &&
	(n = (Optname) optiontab->getnode(optiontab, s + 2))) {
	return -n->optno;
    } else if ((n = (Optname) optiontab->getnode(optiontab, s)))
	return n->optno;
    else
	return OPT_INVALID;
}

/* Identify an option letter */

/**/
int
optlookupc(char c)
{
    if(c < FIRST_OPT || c > LAST_OPT)
	return 0;

    return optletters[c - FIRST_OPT];
}

/**/
static void
restrictparam(char *nam)
{
    Param pm = (Param) paramtab->getnode(paramtab, nam);

    if (pm) {
	pm->flags |= PM_SPECIAL | PM_RESTRICTED;
	return;
    }
    createparam(nam, PM_SCALAR | PM_UNSET | PM_SPECIAL | PM_RESTRICTED);
}

/* list of restricted parameters which are not otherwise special */
static char *rparams[] = {
    "SHELL", "HISTFILE", "LD_LIBRARY_PATH", "LD_AOUT_LIBRARY_PATH",
    "LD_PRELOAD", "LD_AOUT_PRELOAD", NULL
};

/* Set or unset an option, as a result of user request.  The option *
 * number may be negative, indicating that the sense is reversed    *
 * from the usual meaning of the option.                            */

/**/
mod_export int
dosetopt(int optno, int value, int force)
{
    if(!optno)
	return -1;
    if(optno < 0) {
	optno = -optno;
	value = !value;
    }
    if (optno == RESTRICTED) {
	if (isset(RESTRICTED))
	    return value ? 0 : -1;
	if (value) {
	    char **s;

	    for (s = rparams; *s; s++)
		restrictparam(*s);
	}
    } else if(!force && optno == EXECOPT && !value && interact) {
	/* cannot set noexec when interactive */
	return -1;
    } else if(!force && (optno == INTERACTIVE || optno == SHINSTDIN ||
	    optno == SINGLECOMMAND)) {
	if (opts[optno] == value)
	    return 0;
	/* it is not permitted to change the value of these options */
	return -1;
    } else if(!force && optno == USEZLE && value) {
	/* we require a terminal in order to use ZLE */
	if(!interact || SHTTY == -1 || !shout)
	    return -1;
    } else if(optno == PRIVILEGED && !value) {
	/* unsetting PRIVILEGED causes the shell to make itself unprivileged */
#ifdef HAVE_SETUID
	setuid(getuid());
	setgid(getgid());
#endif /* HAVE_SETUID */
#ifndef JOB_CONTROL
    } else if(optno == MONITOR && value) {
	    return -1;
#endif /* not JOB_CONTROL */
#ifdef GETPWNAM_FAKED
    } else if(optno == CDABLEVARS && value) {
	    return -1;
#endif /* GETPWNAM_FAKED */
    }
    opts[optno] = value;
    if (optno == BANGHIST || optno == SHINSTDIN)
	inittyptab();
    return 0;
}

/* Function to get value for special parameter `-' */

/**/
char *
dashgetfn(Param pm)
{
    static char buf[LAST_OPT - FIRST_OPT + 2];
    char *val = buf;
    int i;

    for(i = 0; i <= LAST_OPT - FIRST_OPT; i++) {
	int optno = optletters[i];
	if(optno && ((optno > 0) ? isset(optno) : unset(-optno)))
	    *val++ = FIRST_OPT + i;
    }
    *val = '\0';
    return buf;
}

/* Print option list for --help */

/**/
void
printoptionlist(void)
{
    short *lp;
    char c;

    printf("\nNamed options:\n");
    scanhashtable(optiontab, 1, 0, OPT_ALIAS, printoptionlist_printoption, 0);
    printf("\nOption aliases:\n");
    scanhashtable(optiontab, 1, OPT_ALIAS, 0, printoptionlist_printoption, 0);
    printf("\nOption letters:\n");
    for(lp = optletters, c = FIRST_OPT; c <= LAST_OPT; lp++, c++) {
	if(!*lp)
	    continue;
	printf("  -%c  ", c);
	printoptionlist_printequiv(*lp);
    }
}

/**/
static void
printoptionlist_printoption(HashNode hn, int ignored)
{
    Optname on = (Optname) hn;

    if(on->flags & OPT_ALIAS) {
	printf("  --%-19s  ", on->nam);
	printoptionlist_printequiv(on->optno);
    } else
	printf("  --%s\n", on->nam);
}

/**/
static void
printoptionlist_printequiv(int optno)
{
    int isneg = optno < 0;

    optno *= (isneg ? -1 : 1);
    printf("  equivalent to --%s%s\n", isneg ? "no-" : "", optns[optno-1].nam);
}
