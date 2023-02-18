/*
 * compcore.c - the complete module, completion core code
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1999 Sven Wischnowsky
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

#include "complete.mdh"
#include "compcore.pro"

/* The last completion widget called. */

static Widget lastcompwidget;

/* Flags saying what we have to do with the result. */

/**/
int useexact, useline, uselist, forcelist, startauto;

/**/
mod_export int iforcemenu;

/* Non-zero if we should go back to the last prompt. */

/**/
mod_export int dolastprompt;

/* Non-zero if we should keep an old list. */

/**/
mod_export int oldlist, oldins;

/* Original prefix/suffix lengths. Flag saying if they changed. */

/**/
int origlpre, origlsuf, lenchanged;

/* This is used to decide when the cursor should be moved to the end of    *
 * the inserted word: 0 - never, 1 - only when a single match is inserted, *
 * 2 - when a full match is inserted (single or menu), 3 - always.         */

/**/
int movetoend;

/* The match and group number to insert when starting menucompletion.   */

/**/
mod_export int insmnum, insspace;

#if 0
/* group-numbers in compstate[insert] */
int insgnum, insgroup; /* mod_export */
#endif

/* Information about menucompletion. */

/**/
mod_export struct menuinfo minfo;

/* Number of matches accepted with accept-and-menu-complete */

/**/
mod_export int menuacc;

/* Brace insertion stuff. */

/**/
int hasunqu, useqbr, brpcs, brscs;

/* Flags saying in what kind of string we are. */

/**/
mod_export int ispar, linwhat;

/* A parameter expansion prefix (like ${). */

/**/
char *parpre;

/* Flags for parameter expansions for new style completion. */

/**/
int parflags;

/* Match flags for all matches in this group. */

/**/
mod_export int mflags;

/* Flags saying how the parameter expression we are in is quoted. */

/**/
int parq, eparq;

/* We store the following prefixes/suffixes:                               *
 * ipre,ripre  -- the ignored prefix (quoted and unquoted)                 *
 * isuf        -- the ignored suffix                                       */

/**/
mod_export char *ipre, *ripre, *isuf;

/* The list of matches.  fmatches contains the matches we first ignore *
 * because of fignore.                                                 */

/**/
mod_export LinkList matches;
/**/
LinkList fmatches;

/* This holds the list of matches-groups. lastmatches holds the last list of 
 * permanently allocated matches, pmatches is the same for the list
 * currently built, amatches is the heap allocated stuff during completion
 * (after all matches have been generated it is an alias for pmatches), and
 * lmatches/lastlmatches is a pointer to the last element in the lists. */

/**/
mod_export Cmgroup lastmatches, pmatches, amatches, lmatches, lastlmatches;

/* Non-zero if we have permanently allocated matches (old and new). */

/**/
mod_export int hasoldlist, hasperm;

/* Non-zero if we have a match representing all other matches. */

/**/
int hasallmatch;

/* Non-zero if we have newly added matches. */

/**/
mod_export int newmatches;

/* Number of permanently allocated matches and groups. */

/**/
mod_export int permmnum, permgnum, lastpermmnum, lastpermgnum;

/* The total number of matches and the number of matches to be listed. */

/**/
mod_export int nmatches;
/**/
mod_export int smatches;

/* != 0 if more than one match and at least two different matches */

/**/
mod_export int diffmatches;

/* The number of messages. */

/**/
mod_export int nmessages;

/* != 0 if only explanation strings should be printed */

/**/
mod_export int onlyexpl;

/* Information about the matches for listing. */

/**/
mod_export struct cldata listdat;

/* This flag is non-zero if we are completing a pattern (with globcomplete) */

/**/
mod_export int ispattern, haspattern;

/* Non-zero if at least one match was added without/with -U. */

/**/
mod_export int hasmatched, hasunmatched;

/* The current group of matches. */

/**/
Cmgroup mgroup;

/* Match counter: all matches. */

/**/
mod_export int mnum;

/* The match counter when unambig_data() was called. */

/**/
mod_export int unambig_mnum;

/* Length of longest/shortest match. */

/**/
int maxmlen, minmlen;

/* This holds the explanation strings we have to print in this group and *
 * a pointer to the current cexpl structure. */

/**/
LinkList expls;

/**/
mod_export Cexpl curexpl;

/* A stack of completion matchers to be used. */

/**/
mod_export Cmlist mstack;

/* The completion matchers used when building new stuff for the line. */

/**/
mod_export Cmlist bmatchers;

/* A list with references to all matchers we used. */

/**/
mod_export LinkList matchers;

/* A heap of free Cline structures. */

/**/
mod_export Cline freecl;

/* Ambiguous information. */

/**/
mod_export Aminfo ainfo, fainfo;

/* The memory heap to use for new style completion generation. */

/**/
mod_export Heap compheap;

/* A list of some data.
 *
 * Well, actually, it's the list of all compctls used so far, but since
 * conceptually we don't know anything about compctls here... */

/**/
mod_export LinkList allccs;

/* This says what of the state the line is in when completion is started *
 * came from a previous completion. If the FC_LINE bit is set, the       *
 * string was inserted. If FC_INWORD is set, the last completion moved   *
 * the cursor into the word although it was at the end of it when the    *
 * last completion was invoked.                                          *
 * This is used to detect if the string should be taken as an exact      *
 * match (see do_ambiguous()) and if the cursor has to be moved to the   *
 * end of the word before generating the completions.                    */

/**/
int fromcomp;

/* This holds the end-position of the last string inserted into the line. */

/**/
int lastend;

#define inststr(X) inststrlen((X),1,-1)

/* Main completion entry point, called from zle. */

/**/
int
do_completion(Hookdef dummy, Compldat dat)
{
    int ret = 0, lst = dat->lst, incmd = dat->incmd, osl = showinglist;
    char *s = dat->s;
    char *opm;
    LinkNode n;

    pushheap();

    ainfo = fainfo = NULL;
    matchers = newlinklist();

    zsfree(compqstack);
    compqstack = ztrdup("\\");
    if (instring == 2)
	compqstack[0] = '"';
    else if (instring)
	compqstack[0] = '\'';

    hasunqu = 0;
    useline = (wouldinstab ? -1 : (lst != COMP_LIST_COMPLETE));
    useexact = isset(RECEXACT);
    zsfree(compexactstr);
    compexactstr = ztrdup("");
    uselist = (useline ?
	       ((isset(AUTOLIST) && !isset(BASHAUTOLIST)) ? 
		(isset(LISTAMBIGUOUS) ? 3 : 2) : 0) : 1);
    zsfree(comppatmatch);
    opm = comppatmatch = ztrdup(useglob ? "*" : "");
    zsfree(comppatinsert);
    comppatinsert = ztrdup("menu");
    forcelist = 0;
    haspattern = 0;
    complistmax = getiparam("LISTMAX");
    zsfree(complastprompt);
    complastprompt = ztrdup(((isset(ALWAYSLASTPROMPT) && zmult == 1) ||
			     (unset(ALWAYSLASTPROMPT) && zmult != 1)) ?
			    "yes" : "");
    dolastprompt = 1;
    zsfree(complist);
    complist = ztrdup(isset(LISTROWSFIRST) ?
		      (isset(LISTPACKED) ? "packed rows" : "rows") :
		      (isset(LISTPACKED) ? "packed" : ""));
    startauto = isset(AUTOMENU);
    movetoend = ((cs == we || isset(ALWAYSTOEND)) ? 2 : 1);
    showinglist = 0;
    hasmatched = hasunmatched = 0;
    minmlen = 1000000;
    maxmlen = -1;
    compignored = 0;
    nmessages = 0;
    hasallmatch = 0;

    /* Make sure we have the completion list and compctl. */
    if (makecomplist(s, incmd, lst)) {
	/* Error condition: feeeeeeeeeeeeep(). */
	cs = 0;
	foredel(ll);
	inststr(origline);
	cs = origcs;
	clearlist = 1;
	ret = 1;
	minfo.cur = NULL;
	if (useline < 0)
	    ret = selfinsert(zlenoargs);
	goto compend;
    }
    zsfree(lastprebr);
    zsfree(lastpostbr);
    lastprebr = lastpostbr = NULL;

    if (comppatmatch && *comppatmatch && comppatmatch != opm)
	haspattern = 1;
    if (iforcemenu) {
	if (nmatches)
	    do_ambig_menu();
	ret = !nmatches;
    } else if (useline < 0)
	ret = selfinsert(zlenoargs);
    else if (!useline && uselist) {
	/* All this and the guy only wants to see the list, sigh. */
	cs = 0;
	foredel(ll);
	inststr(origline);
	cs = origcs;
	showinglist = -2;
    } else if (useline == 2 && nmatches > 1) {
	do_allmatches(1);

	minfo.cur = NULL;

	if (forcelist)
	    showinglist = -2;
	else
	    invalidatelist();
    } else if (useline) {
	/* We have matches. */
	if (nmatches > 1 && diffmatches) {
	    /* There is more than one match. */
	    ret = do_ambiguous();

	    if (!showinglist && uselist && listshown && (usemenu == 2 || oldlist))
		showinglist = osl;
	} else if (nmatches == 1 || (nmatches > 1 && !diffmatches)) {
	    /* Only one match. */
	    Cmgroup m = amatches;

	    while (!m->mcount)
		m = m->next;
	    minfo.cur = NULL;
	    minfo.asked = 0;
	    do_single(m->matches[0]);
	    if (forcelist) {
		if (uselist)
		    showinglist = -2;
		else
		    clearlist = 1;
	    } else
		invalidatelist();
	} else if (nmessages && forcelist) {
	    if (uselist)
		showinglist = -2;
	    else
		clearlist = 1;
	}
    } else {
	invalidatelist();
	if (forcelist)
	    clearlist = 1;
	cs = 0;
	foredel(ll);
	inststr(origline);
	cs = origcs;
    }
    /* Print the explanation strings if needed. */
    if (!showinglist && validlist && usemenu != 2 && uselist &&
	(nmatches != 1 || diffmatches) &&
	useline >= 0 && useline != 2 && (!oldlist || !listshown)) {
	onlyexpl = 3;
	showinglist = -2;
    }
 compend:
    for (n = firstnode(matchers); n; incnode(n))
	freecmatcher((Cmatcher) getdata(n));

    ll = strlen((char *)line);
    if (cs > ll)
	cs = ll;
    popheap();

    return ret;
}

/* Before and after hooks called by zle. */

static int oldmenucmp;

/**/
int
before_complete(Hookdef dummy, int *lst)
{
    oldmenucmp = menucmp;

    if (showagain && validlist)
	showinglist = -2;
    showagain = 0;

    /* If we are doing a menu-completion... */

    if (menucmp && *lst != COMP_LIST_EXPAND && 
	(menucmp != 1 || !compwidget || compwidget == lastcompwidget)) {
	do_menucmp(*lst);
	return 1;
    }
    if (menucmp && validlist && *lst == COMP_LIST_COMPLETE) {
	showinglist = -2;
	onlyexpl = listdat.valid = 0;
	return 1;
    }
    lastcompwidget = compwidget;

    /* We may have to reset the cursor to its position after the   *
     * string inserted by the last completion. */

    if ((fromcomp & FC_INWORD) && (cs = lastend) > ll)
	cs = ll;

    /* Check if we have to start a menu-completion (via automenu). */

    if (startauto && lastambig &&
	(!isset(BASHAUTOLIST) || lastambig == 2))
	usemenu = 2;

    return 0;
}

/**/
int
after_complete(Hookdef dummy, int *dat)
{
    if (menucmp && !oldmenucmp) {
	struct chdata cdat;
	int ret;

	cdat.matches = amatches;
	cdat.num = nmatches;
	cdat.nmesg = nmessages;
	cdat.cur = NULL;
	if ((ret = runhookdef(MENUSTARTHOOK, (void *) &cdat))) {
	    dat[1] = 0;
	    menucmp = menuacc = 0;
	    minfo.cur = NULL;
	    if (ret >= 2) {
		fixsuffix();
		cs = 0;
		foredel(ll);
		inststr(origline);
		cs = origcs;
		if (ret == 2) {
		    clearlist = 1;
		    invalidatelist();
		}
	    }
	}
    }
    return 0;
}

/* This calls the given completion widget function. */

static int parwb, parwe, paroffs;

/**/
static void
callcompfunc(char *s, char *fn)
{
    Eprog prog;
    int lv = lastval;
    char buf[20];

    if ((prog = getshfunc(fn)) != &dummy_eprog) {
	char **p, *tmp;
	int aadd = 0, usea = 1, icf = incompfunc, osc = sfcontext;
	unsigned int rset, kset;
	Param *ocrpms = comprpms, *ockpms = compkpms;

	comprpms = (Param *) zalloc(CP_REALPARAMS * sizeof(Param));
	compkpms = (Param *) zalloc(CP_KEYPARAMS * sizeof(Param));

	rset = CP_ALLREALS;
	kset = CP_ALLKEYS &
	    ~(CP_PARAMETER | CP_REDIRECT | CP_QUOTE | CP_QUOTING |
	      CP_EXACTSTR | CP_OLDLIST | CP_OLDINS |
	      (useglob ? 0 : CP_PATMATCH));
	zsfree(compvared);
	if (varedarg) {
	    compvared = ztrdup(varedarg);
	    kset |= CP_VARED;
	} else
	    compvared = ztrdup("");
	if (!*complastprompt)
	    kset &= ~CP_LASTPROMPT;
	zsfree(compcontext);
	zsfree(compparameter);
	zsfree(compredirect);
	compparameter = compredirect = "";
	if (ispar)
	    compcontext = (ispar == 2 ? "brace_parameter" : "parameter");
	else if (linwhat == IN_MATH) {
	    if (insubscr) {
		compcontext = "subscript";
		if (varname) {
		    compparameter = varname;
		    kset |= CP_PARAMETER;
		}
	    } else
		compcontext = "math";
	    usea = 0;
	} else if (lincmd) {
	    if (insubscr) {
		compcontext = "subscript";
		kset |= CP_PARAMETER;
	    } else
		compcontext = "command";
	} else if (linredir) {
	    compcontext = "redirect";
	    if (rdstr)
		compredirect = rdstr;
	    kset |= CP_REDIRECT;
	} else
	    switch (linwhat) {
	    case IN_ENV:
		compcontext = (linarr ? "array_value" : "value");
		compparameter = varname;
		kset |= CP_PARAMETER;
		if (!clwpos) {
		    clwpos = 1;
		    clwnum = 2;
		    zsfree(clwords[1]);
		    clwords[1] = ztrdup(s);
		    zsfree(clwords[2]);
		    clwords[2] = NULL;
		}
		aadd = 1;
		break;
	    case IN_COND:
		compcontext = "condition";
		break;
	    default:
		if (cmdstr)
		    compcontext = "command";
		else {
		    compcontext = "value";
		    kset |= CP_PARAMETER;
		    if (clwords[0])
			compparameter = clwords[0];
		    aadd = 1;
		}
	    }
	compcontext = ztrdup(compcontext);
	if (compwords)
	    freearray(compwords);
	if (usea && (!aadd || clwords[0])) {
	    char **q;

	    q = compwords = (char **)
		zalloc((clwnum + 1) * sizeof(char *));
	    for (p = clwords + aadd; *p; p++, q++)
		untokenize(*q = ztrdup(*p));
	    *q = NULL;
	} else
	    compwords = (char **) zcalloc(sizeof(char *));

	compparameter = ztrdup(compparameter);
	compredirect = ztrdup(compredirect);
	zsfree(compquote);
	zsfree(compquoting);
	if (instring) {
	    if (instring == 1) {
		compquote = ztrdup("\'");
		compquoting = ztrdup("single");
	    } else {
		compquote = ztrdup("\"");
		compquoting = ztrdup("double");
	    }
	    kset |= CP_QUOTE | CP_QUOTING;
	} else if (inbackt) {
	    compquote = ztrdup("`");
	    compquoting = ztrdup("backtick");
	    kset |= CP_QUOTE | CP_QUOTING;
	} else {
	    compquote = ztrdup("");
	    compquoting = ztrdup("");
	}
	zsfree(compprefix);
	zsfree(compsuffix);
	if (unset(COMPLETEINWORD)) {
	    tmp = (linwhat == IN_MATH ? dupstring(s) : multiquote(s, 0));
	    untokenize(tmp);
	    compprefix = ztrdup(tmp);
	    compsuffix = ztrdup("");
	} else {
	    char *ss, sav;
	    
	    ss = s + offs;

	    sav = *ss;
	    *ss = '\0';
	    tmp = (linwhat == IN_MATH ? dupstring(s) : multiquote(s, 0));
	    untokenize(tmp);
	    compprefix = ztrdup(tmp);
	    *ss = sav;
	    ss = (linwhat == IN_MATH ? dupstring(ss) : multiquote(ss, 0));
	    untokenize(ss);
	    compsuffix = ztrdup(ss);
	}
	zsfree(compiprefix);
	zsfree(compisuffix);
	if (parwb < 0) {
	    compiprefix = ztrdup("");
	    compisuffix = ztrdup("");
	} else {
	    int l;

	    compiprefix = (char *) zalloc((l = wb - parwb) + 1);
	    memcpy(compiprefix, line + parwb, l);
	    compiprefix[l] = '\0';
	    compisuffix = (char *) zalloc((l = parwe - we) + 1);
	    memcpy(compisuffix, line + we, l);
	    compisuffix[l] = '\0';

	    wb = parwb;
	    we = parwe;
	    offs = paroffs;
	}
	zsfree(compqiprefix);
	compqiprefix = ztrdup(qipre ? qipre : "");
	zsfree(compqisuffix);
	compqisuffix = ztrdup(qisuf ? qisuf : "");
	origlpre = (strlen(compqiprefix) + strlen(compiprefix) +
		    strlen(compprefix));
	origlsuf = (strlen(compqisuffix) + strlen(compisuffix) +
		    strlen(compsuffix));
	lenchanged = 0;
	compcurrent = (usea ? (clwpos + 1 - aadd) : 0);

	zsfree(complist);
	switch (uselist) {
	case 0: complist = ""; kset &= ~CP_LIST; break;
	case 1: complist = "list"; break;
	case 2: complist = "autolist"; break;
	case 3: complist = "ambiguous"; break;
	}
	if (isset(LISTPACKED))
	    complist = dyncat(complist, " packed");
	if (isset(LISTROWSFIRST))
	    complist = dyncat(complist, " rows");

	complist = ztrdup(complist);
	zsfree(compinsert);
	if (useline) {
	    switch (usemenu) {
	    case 0:
		compinsert = (isset(AUTOMENU) ?
			      "automenu-unambiguous" :
			      "unambiguous");
		break;
	    case 1: compinsert = "menu"; break;
	    case 2: compinsert = "automenu"; break;
	    }
	} else {
	    compinsert = "";
	    kset &= ~CP_INSERT;
	}
	compinsert = (useline < 0 ? tricat("tab ", "", compinsert) :
		      ztrdup(compinsert));
	zsfree(compexact);
	if (useexact)
	    compexact = ztrdup("accept");
	else {
	    compexact = ztrdup("");
	    kset &= ~CP_EXACT;
	}
	zsfree(comptoend);
	if (movetoend == 1)
	    comptoend = ztrdup("single");
	else
	    comptoend = ztrdup("match");
	zsfree(compoldlist);
	zsfree(compoldins);
	if (hasoldlist && lastpermmnum) {
	    if (listshown)
		compoldlist = "shown";
	    else
		compoldlist = "yes";
	    kset |= CP_OLDLIST;
	    if (minfo.cur) {
		sprintf(buf, "%d", (*(minfo.cur))->gnum);
		compoldins = buf;
		kset |= CP_OLDINS;
	    } else
		compoldins = "";
	} else
	    compoldlist = compoldins = "";
	compoldlist = ztrdup(compoldlist);
	compoldins = ztrdup(compoldins);

	incompfunc = 1;
	startparamscope();
	makecompparams();
	comp_setunset(rset, (~rset & CP_ALLREALS),
		      kset, (~kset & CP_ALLKEYS));
	makezleparams(1);
	sfcontext = SFC_CWIDGET;
	NEWHEAPS(compheap) {
	    LinkList largs = NULL;
	    int olv = lastval;

	    if (*cfargs) {
		char **p = cfargs;

		largs = newlinklist();
		addlinknode(largs, dupstring(fn));
		while (*p)
		    addlinknode(largs, dupstring(*p++));
	    }
	    doshfunc(fn, prog, largs, 0, 0);
	    cfret = lastval;
	    lastval = olv;
	} OLDHEAPS;
	sfcontext = osc;
	endparamscope();
	lastcmd = 0;
	incompfunc = icf;

	if (!complist)
	    uselist = 0;
	else if (!strncmp(complist, "list", 4))
	    uselist = 1;
	else if (!strncmp(complist, "auto", 4))
	    uselist = 2;
	else if (!strncmp(complist, "ambig", 5))
	    uselist = 3;
	else
	    uselist = 0;
	forcelist = (complist && strstr(complist, "force"));
	onlyexpl = (complist ? ((strstr(complist, "expl") ? 1 : 0) |
				(strstr(complist, "messages") ? 2 : 0)) : 0);

	if (!compinsert)
	    useline = 0;
	else if (strstr(compinsert, "tab"))
	    useline = -1;
	else if (!strcmp(compinsert, "unambig") ||
		 !strcmp(compinsert, "unambiguous") ||
		 !strcmp(compinsert, "automenu-unambiguous"))
	    useline = 1, usemenu = 0;
	else if (!strcmp(compinsert, "all"))
	    useline = 2, usemenu = 0;
	else if (idigit(*compinsert)) {
#if 0
	    /* group-numbers in compstate[insert] */
	    char *m;
#endif
	    useline = 1; usemenu = 3;
	    insmnum = atoi(compinsert);
#if 0
	    /* group-numbers in compstate[insert] */
	    if ((m = strchr(compinsert, ':'))) {
		insgroup = 1;
		insgnum = atoi(m + 1);
	    }
#endif
	    insspace = (compinsert[strlen(compinsert) - 1] == ' ');
	} else {
	    char *p;

	    if (strpfx("menu", compinsert))
		useline = 1, usemenu = 1;
	    else if (strpfx("auto", compinsert))
		useline = 1, usemenu = 2;
	    else
		useline = usemenu = 0;

	    if (useline && (p = strchr(compinsert, ':'))) {
		insmnum = atoi(++p);
#if 0
		/* group-numbers in compstate[insert] */
		if ((p = strchr(p, ':'))) {
		    insgroup = 1;
		    insgnum = atoi(p + 1);
		}
#endif
	    }
	}
	startauto = ((compinsert &&
		      !strcmp(compinsert, "automenu-unambiguous")) ||
		     (bashlistfirst && isset(AUTOMENU) &&
                      (!compinsert || !*compinsert)));
	useexact = (compexact && !strcmp(compexact, "accept"));

	if (!comptoend || !*comptoend)
	    movetoend = 0;
	else if (!strcmp(comptoend, "single"))
	    movetoend = 1;
	else if (!strcmp(comptoend, "always"))
	    movetoend = 3;
	else
	    movetoend = 2;

	oldlist = (hasoldlist && compoldlist && !strcmp(compoldlist, "keep"));
	oldins = (hasoldlist && minfo.cur &&
		  compoldins && !strcmp(compoldins, "keep"));

	zfree(comprpms, CP_REALPARAMS * sizeof(Param));
	zfree(compkpms, CP_KEYPARAMS * sizeof(Param));
	comprpms = ocrpms;
	compkpms = ockpms;
    }
    lastval = lv;
}

/* Create the completion list.  This is called whenever some bit of   *
 * completion code needs the list.                                    *
 * Along with the list is maintained the prefixes/suffixes etc.  When *
 * any of this becomes invalid -- e.g. if some text is changed on the *
 * command line -- invalidatelist() should be called, to set          *
 * validlist to zero and free up the memory used.  This function      *
 * returns non-zero on error.                                         */

/**/
static int
makecomplist(char *s, int incmd, int lst)
{
    char *p;
    int owb = wb, owe = we, ooffs = offs;

    /* Inside $... ? */
    if (compfunc && (p = check_param(s, 0, 0))) {
	s = p;
	parwb = owb;
	parwe = owe;
	paroffs = ooffs;
    } else
	parwb = -1;

    linwhat = inwhat;

    if (compfunc) {
	char *os = s;
	int onm = nmatches, odm = diffmatches, osi = movefd(0);

	bmatchers = NULL;
	mstack = NULL;

	ainfo = (Aminfo) hcalloc(sizeof(struct aminfo));
	fainfo = (Aminfo) hcalloc(sizeof(struct aminfo));

	freecl = NULL;

	if (!validlist)
	    lastambig = 0;
	amatches = NULL;
	mnum = 0;
	unambig_mnum = -1;
	isuf = NULL;
	insmnum = 1;
#if 0
	/* group-numbers in compstate[insert] */
	insgnum = 1;
	insgroup = 0;
#endif
	oldlist = oldins = 0;
	begcmgroup("default", 0);
	menucmp = menuacc = newmatches = onlyexpl = 0;

	s = dupstring(os);
	callcompfunc(s, compfunc);
	endcmgroup(NULL);

	/* Needed for compcall. */
	runhookdef(COMPCTLCLEANUPHOOK, NULL);

	if (oldlist) {
	    nmatches = onm;
	    diffmatches = odm;
	    validlist = 1;
	    amatches = lastmatches;
	    lmatches = lastlmatches;
	    if (pmatches) {
		freematches(pmatches, 1);
		pmatches = NULL;
		hasperm = 0;
	    }
	    redup(osi, 0);

	    return 0;
	}
	if (lastmatches) {
	    freematches(lastmatches, 1);
	    lastmatches = NULL;
	}
	permmatches(1);
	amatches = pmatches;
	lastpermmnum = permmnum;
	lastpermgnum = permgnum;

	lastmatches = pmatches;
	lastlmatches = lmatches;
	pmatches = NULL;
	hasperm = 0;
	hasoldlist = 1;

	if ((nmatches || nmessages) && !errflag) {
	    validlist = 1;

	    redup(osi, 0);

	    return 0;
	}
	redup(osi, 0);
	return 1;
    } else {
	struct ccmakedat dat;

	dat.str = s;
	dat.incmd = incmd;
	dat.lst = lst;
	runhookdef(COMPCTLMAKEHOOK, (void *) &dat);

	/* Needed for compcall. */
	runhookdef(COMPCTLCLEANUPHOOK, NULL);

	return dat.lst;
    }
}

/**/
mod_export char *
multiquote(char *s, int ign)
{
    if (s) {
	char *os = s, *p = compqstack;

	if (p && *p && (ign < 1 || p[ign])) {
	    if (ign > 0)
		p += ign;
	    while (*p) {
		if (ign >= 0 || p[1])
		    s = bslashquote(s, NULL,
				    (*p == '\'' ? 1 : (*p == '"' ? 2 : 0)));
		p++;
	    }
	}
	return (s == os ? dupstring(s) : s);
    }
    DPUTS(1, "BUG: null pointer in multiquote()");
    return NULL;
}

/**/
mod_export char *
tildequote(char *s, int ign)
{
    if (s) {
	int tilde;

	if ((tilde = (*s == '~')))
	    *s = 'x';
	s = multiquote(s, ign);
	if (tilde)
	    *s = '~';

	return s;
    }
    DPUTS(1, "BUG: null pointer in tildequote()");
    return NULL;
}

/* Check if we have to complete a parameter name. */

/**/
mod_export char *
check_param(char *s, int set, int test)
{
    char *p;

    zsfree(parpre);
    parpre = NULL;

    if (!test)
	ispar = parq = eparq = 0;
    /* Try to find a `$'. */
    for (p = s + offs; p > s && *p != String && *p != Qstring; p--);
    if (*p == String || *p == Qstring) {
	/* Handle $$'s */
	while (p > s && (p[-1] == String || p[-1] == Qstring))
	    p--;
	while ((p[1] == String || p[1] == Qstring) &&
	       (p[2] == String || p[2] == Qstring))
	    p += 2;
    }
    if ((*p == String || *p == Qstring) && p[1] != Inpar && p[1] != Inbrack) {
	/* This is really a parameter expression (not $(...) or $[...]). */
	char *b = p + 1, *e = b;
	int n = 0, br = 1, nest = 0;

	if (*b == Inbrace) {
	    char *tb = b;

	    /* If this is a ${...}, see if we are before the '}'. */
	    if (!skipparens(Inbrace, Outbrace, &tb))
		return NULL;

	    /* Ignore the possible (...) flags. */
	    b++, br++;
	    n = skipparens(Inpar, Outpar, &b);

	    for (tb = p - 1; tb > s && *tb != Outbrace && *tb != Inbrace; tb--);
	    if (tb > s && *tb == Inbrace && (tb[-1] == String || *tb == Qstring))
		nest = 1;
	}

	/* Ignore the stuff before the parameter name. */
	for (; *b; b++)
	    if (*b != '^' && *b != Hat &&
		*b != '=' && *b != Equals &&
		*b != '~' && *b != Tilde)
		break;
	if (*b == '#' || *b == Pound || *b == '+')
	    b++;

	e = b;
	if (br) {
	    while (*e == (test ? Dnull : '"'))
		e++, parq++;
	    if (!test)
		b = e;
	}
	/* Find the end of the name. */
	if (*e == Quest || *e == Star || *e == String || *e == Qstring ||
	    *e == '?'   || *e == '*'  || *e == '$'    ||
	    *e == '-'   || *e == '!'  || *e == '@')
	    e++;
	else if (idigit(*e))
	    while (idigit(*e))
		e++;
	else if (iident(*e))
	    while (iident(*e) ||
		   (comppatmatch && *comppatmatch && (*e == Star || *e == Quest)))
		e++;

	/* Now make sure that the cursor is inside the name. */
	if (offs <= e - s && offs >= b - s && n <= 0) {
	    char sav;

	    if (br) {
		p = e;
		while (*p == (test ? Dnull : '"'))
		    p++, parq--, eparq++;
	    }
	    /* It is. */
	    if (test)
		return b;
	    /* If we were called from makecomplistflags(), we have to set the
	     * global variables. */

	    if (set) {
		if (br >= 2) {
		    mflags |= CMF_PARBR;
		    if (nest)
			mflags |= CMF_PARNEST;
		}
		/* Get the prefix (anything up to the character before the name). */
		isuf = dupstring(e);
		untokenize(isuf);
		sav = *b;
		*b = *e = '\0';
		ripre = dyncat((ripre ? ripre : ""), s);
		ipre = dyncat((ipre ? ipre : ""), s);
		*b = sav;

		untokenize(ipre);
	    }
	    /* Save the prefix. */
	    if (compfunc) {
		parflags = (br >= 2 ? CMF_PARBR | (nest ? CMF_PARNEST : 0) : 0);
		sav = *b;
		*b = '\0';
		untokenize(parpre = ztrdup(s));
		*b = sav;
	    }
	    /* And adjust wb, we, and offs again. */
	    offs -= b - s;
	    wb = cs - offs;
	    we = wb + e - b;
	    ispar = (br >= 2 ? 2 : 1);
	    b[we-wb] = '\0';
	    return b;
	}
    }
    return NULL;
}

/* Copy the given string and remove backslashes from the copy and return it. */

/**/
mod_export char *
rembslash(char *s)
{
    char *t = s = dupstring(s);

    while (*s)
	if (*s == '\\') {
	    chuck(s);
	    if (*s)
		s++;
	} else
	    s++;

    return t;
}

/* This should probably be moved into tokenize(). */

/**/
mod_export char *
ctokenize(char *p)
{
    char *r = p;
    int bslash = 0;

    tokenize(p);

    for (p = r; *p; p++) {
	if (*p == '\\')
	    bslash = 1;
	else {
	    if (*p == '$' || *p == '{' || *p == '}') {
		if (bslash)
		    p[-1] = Bnull;
		else
		    *p = (*p == '$' ? String :
			  (*p == '{' ? Inbrace : Outbrace));
	    }
	    bslash = 0;
	}
    }
    return r;
}

/**/
mod_export char *
comp_str(int *ipl, int *pl, int untok)
{
    char *p = dupstring(compprefix);
    char *s = dupstring(compsuffix);
    char *ip = dupstring(compiprefix);
    char *str;
    int lp, ls, lip;

    if (!untok) {
	ctokenize(p);
	remnulargs(p);
	ctokenize(s);
	remnulargs(s);
    }
    lp = strlen(p);
    ls = strlen(s);
    lip = strlen(ip);
    str = zhalloc(lip + lp + ls + 1);
    strcpy(str, ip);
    strcat(str, p);
    strcat(str, s);

    if (ipl)
	*ipl = lip;
    if (pl)
	*pl = lp;

    return str;
}

/**/
int
set_comp_sep(void)
{
    int lip, lp;
    char *s = comp_str(&lip, &lp, 1);
    LinkList foo = newlinklist();
    LinkNode n;
    int owe = we, owb = wb, ocs = cs, swb, swe, scs, soffs, ne = noerrs;
    int tl, got = 0, i = 0, cur = -1, oll = ll, sl, remq;
    int ois = instring, oib = inbackt, noffs = lp, ona = noaliases;
    char *tmp, *p, *ns, *ol = (char *) line, sav, *qp, *qs, *ts, qc = '\0';

    s += lip;
    wb += lip;
    untokenize(s);

    swb = swe = soffs = 0;
    ns = NULL;

    /* Put the string in the lexer buffer and call the lexer to *
     * get the words we have to expand.                        */
    zleparse = 1;
    addedx = 1;
    noerrs = 1;
    lexsave();
    tmp = (char *) zhalloc(tl = 3 + strlen(s));
    tmp[0] = ' ';
    memcpy(tmp + 1, s, noffs);
    tmp[(scs = cs = 1 + noffs)] = 'x';
    strcpy(tmp + 2 + noffs, s + noffs);
    if ((remq = (*compqstack == '\\')))
	tmp = rembslash(tmp);
    inpush(dupstrspace(tmp), 0, NULL);
    line = (unsigned char *) tmp;
    ll = tl - 1;
    strinbeg(0);
    noaliases = 1;
    do {
	ctxtlex();
	if (tok == LEXERR) {
	    int j;

	    if (!tokstr)
		break;
	    for (j = 0, p = tokstr; *p; p++)
		if (*p == Snull || *p == Dnull)
		    j++;
	    if (j & 1) {
		tok = STRING;
		if (p > tokstr && p[-1] == ' ')
		    p[-1] = '\0';
	    }
	}
	if (tok == ENDINPUT || tok == LEXERR)
	    break;
	if (tokstr && *tokstr)
	    addlinknode(foo, (p = ztrdup(tokstr)));
	else
	    p = NULL;
	if (!got && !zleparse) {
	    DPUTS(!p, "no current word in substr");
	    got = 1;
	    cur = i;
	    swb = wb - 1;
	    swe = we - 1;
	    soffs = cs - swb;
	    chuck(p + soffs);
	    ns = dupstring(p);
	}
	i++;
    } while (tok != ENDINPUT && tok != LEXERR);
    noaliases = ona;
    strinend();
    inpop();
    errflag = zleparse = 0;
    noerrs = ne;
    lexrestore();
    wb = owb;
    we = owe;
    cs = ocs;
    line = (unsigned char *) ol;
    ll = oll;
    if (cur < 0 || i < 1)
	return 1;
    owb = offs;
    offs = soffs;
    if ((p = check_param(ns, 0, 1))) {
	for (p = ns; *p; p++)
	    if (*p == Dnull)
		*p = '"';
	    else if (*p == Snull)
		*p = '\'';
    }
    offs = owb;

    untokenize(ts = dupstring(ns));

    if (*ns == Snull || *ns == Dnull) {
	instring = (*ns == Snull ? 1 : 2);
	inbackt = 0;
	swb++;
	if (ns[strlen(ns) - 1] == *ns && ns[1])
	    swe--;
	zsfree(autoq);
	autoq = ztrdup(compqstack[1] ? "" :
		       multiquote(*ns == Snull ? "'" : "\"", 1));
	qc = (*ns == Snull ? '\'' : '"');
	ts++;
    } else {
	instring = 0;
	zsfree(autoq);
	autoq = NULL;
    }
    for (p = ns, i = swb; *p; p++, i++) {
	if (INULL(*p)) {
	    if (i < scs) {
		if (remq && *p == Bnull && p[1])
		    swb -= 2;
	    }
	    if (p[1] || *p != Bnull) {
		if (*p == Bnull) {
		    if (scs == i + 1)
			scs++, soffs++;
		} else {
		    if (scs > i--)
			scs--;
		}
	    } else {
		if (scs == swe)
		    scs--;
	    }
	    chuck(p--);
	}
    }
    ns = ts;

    if (instring && strchr(compqstack, '\\')) {
	int rl = strlen(ns), ql = strlen(multiquote(ns, !!compqstack[1]));

	if (ql > rl)
	    swb -= ql - rl;
    }
    sav = s[(i = swb - 1)];
    s[i] = '\0';
    qp = rembslash(s);
    s[i] = sav;
    if (swe < swb)
	swe = swb;
    swe--;
    sl = strlen(s);
    if (swe > sl) {
	swe = sl;
	if (strlen(ns) > swe - swb + 1)
	    ns[swe - swb + 1] = '\0';
    }
    qs = rembslash(s + swe);
    sl = strlen(ns);
    if (soffs > sl)
	soffs = sl;

    {
	int set = CP_QUOTE | CP_QUOTING, unset = 0;

	p = tricat((instring ? (instring == 1 ? "'" : "\"") : "\\"),
		   compqstack, "");
	zsfree(compqstack);
	compqstack = p;

	zsfree(compquote);
	zsfree(compquoting);
	if (instring == 2) {
	    compquote = "\"";
	    compquoting = "double";
	} else if (instring == 1) {
	    compquote = "'";
	    compquoting = "single";
	} else {
	    compquote = compquoting = "";
	    unset = set;
	    set = 0;
	}
	compquote = ztrdup(compquote);
	compquoting = ztrdup(compquoting);
	comp_setunset(0, 0, set, unset);

	zsfree(compprefix);
	zsfree(compsuffix);
	if (unset(COMPLETEINWORD)) {
	    untokenize(ns);
	    compprefix = ztrdup(ns);
	    compsuffix = ztrdup("");
	} else {
	    char *ss, sav;
	    
	    ss = ns + soffs;

	    sav = *ss;
	    *ss = '\0';
	    untokenize(ns);
	    compprefix = ztrdup(ns);
	    *ss = sav;
	    untokenize(ss);
	    compsuffix = ztrdup(ss);
	}
	tmp = tricat(compqiprefix, compiprefix, multiquote(qp, 1));
	zsfree(compqiprefix);
	compqiprefix = tmp;
	tmp = tricat(multiquote(qs, 1), compisuffix, compqisuffix);
	zsfree(compqisuffix);
	compqisuffix = tmp;
	zsfree(compiprefix);
	compiprefix = ztrdup("");
	zsfree(compisuffix);
	compisuffix = ztrdup("");
	freearray(compwords);
	i = countlinknodes(foo);
	compwords = (char **) zalloc((i + 1) * sizeof(char *));
	for (n = firstnode(foo), i = 0; n; incnode(n), i++) {
	    p = compwords[i] = (char *) getdata(n);
	    untokenize(p);
	}
	compcurrent = cur + 1;
	compwords[i] = NULL;
    }
    instring = ois;
    inbackt = oib;

    return 0;
}

/* This stores the strings from the list in an array. */

/**/
mod_export void
set_list_array(char *name, LinkList l)
{
    char **a, **p;
    LinkNode n;

    a = (char **) zalloc((countlinknodes(l) + 1) * sizeof(char *));
    for (p = a, n = firstnode(l); n; incnode(n))
	*p++ = ztrdup((char *) getdata(n));
    *p = NULL;

    setaparam(name, a);
}

/* Get the words from a variable or a (list of words). */

/**/
mod_export char **
get_user_var(char *nam)
{
    if (!nam)
	return NULL;
    else if (*nam == '(') {
	/* It's a (...) list, not a parameter name. */
	char *ptr, *s, **uarr, **aptr;
	int count = 0, notempty = 0, brk = 0;
	LinkList arrlist = newlinklist();

	ptr = dupstring(nam);
	s = ptr + 1;
	while (*++ptr) {
	    if (*ptr == '\\' && ptr[1])
		chuck(ptr), notempty = 1;
	    else if (*ptr == ',' || inblank(*ptr) || *ptr == ')') {
		if (*ptr == ')')
		    brk++;
		if (notempty) {
		    *ptr = '\0';
		    count++;
		    if (*s == '\n')
			s++;
		    addlinknode(arrlist, s);
		}
		s = ptr + 1;
		notempty = 0;
	    } else {
		notempty = 1;
		if (*ptr == Meta)
		    ptr++;
	    }
	    if (brk)
		break;
	}
	if (!brk || !count)
	    return NULL;
	*ptr = '\0';
	aptr = uarr = (char **) zhalloc(sizeof(char *) * (count + 1));

	while ((*aptr++ = (char *)ugetnode(arrlist)));
	uarr[count] = NULL;
	return uarr;
    } else {
	/* Otherwise it should be a parameter name. */
	char **arr = NULL, *val;

	queue_signals();
	if ((arr = getaparam(nam)) || (arr = gethparam(nam)))
	    arr = (incompfunc ? arrdup(arr) : arr);
	else if ((val = getsparam(nam))) {
	    arr = (char **) zhalloc(2*sizeof(char *));
	    arr[0] = (incompfunc ? dupstring(val) : val);
	    arr[1] = NULL;
	}
	unqueue_signals();
	return arr;
    }
}

static char **
get_data_arr(char *name, int keys)
{
    struct value vbuf;
    char **ret;
    Value v;

    queue_signals();
    if (!(v = fetchvalue(&vbuf, &name, 1,
			 (keys ? SCANPM_WANTKEYS : SCANPM_WANTVALS) |
			 SCANPM_MATCHMANY)))
	ret = NULL;
    else
	ret = getarrvalue(v);
    unqueue_signals();

    return ret;
}

/* This is used by compadd to add a couple of matches. The arguments are
 * the strings given via options. The last argument is the array with
 * the matches. */

/**/
int
addmatches(Cadata dat, char **argv)
{
    char *s, *ms, *lipre = NULL, *lisuf = NULL, *lpre = NULL, *lsuf = NULL;
    char **aign = NULL, **dparr = NULL, *oaq = autoq, *oppre = dat->ppre;
    char *oqp = qipre, *oqs = qisuf, qc, **disp = NULL, *ibuf = NULL;
    char **arrays = NULL;
    int lpl, lsl, pl, sl, bcp = 0, bcs = 0, bpadd = 0, bsadd = 0;
    int ppl = 0, psl = 0, ilen = 0;
    int llpl = 0, llsl = 0, nm = mnum, gflags = 0, ohp = haspattern;
    int isexact, doadd, ois = instring, oib = inbackt;
    Cline lc = NULL, pline = NULL, sline = NULL;
    Cmatch cm;
    struct cmlist mst;
    Cmlist oms = mstack;
    Patprog cp = NULL, *pign = NULL;
    LinkList aparl = NULL, oparl = NULL, dparl = NULL;
    Brinfo bp, bpl = brbeg, obpl, bsl = brend, obsl;
    Heap oldheap;

    if (!*argv && !(dat->aflags & CAF_ALL)) {
	SWITCHHEAPS(oldheap, compheap) {
	    /* Select the group in which to store the matches. */
	    gflags = (((dat->aflags & CAF_NOSORT ) ? CGF_NOSORT  : 0) |
		      ((dat->aflags & CAF_UNIQALL) ? CGF_UNIQALL : 0) |
		      ((dat->aflags & CAF_UNIQCON) ? CGF_UNIQCON : 0));
	    if (dat->group) {
		endcmgroup(NULL);
		begcmgroup(dat->group, gflags);
	    } else {
		endcmgroup(NULL);
		begcmgroup("default", 0);
	    }
	    if (dat->mesg)
		addmesg(dat->mesg);
	} SWITCHBACKHEAPS(oldheap);

	return 1;
    }
    for (bp = brbeg; bp; bp = bp->next)
	bp->curpos = ((dat->aflags & CAF_QUOTE) ? bp->pos : bp->qpos);
    for (bp = brend; bp; bp = bp->next)
	bp->curpos = ((dat->aflags & CAF_QUOTE) ? bp->pos : bp->qpos);

    if (dat->flags & CMF_ISPAR)
	dat->flags |= parflags;
    if (compquote && (qc = *compquote)) {
	if (qc == '`') {
	    instring = 0;
	    inbackt = 0;
	    autoq = "";
	} else {
	    char buf[2];

	    instring = (qc == '\'' ? 1 : 2);
	    inbackt = 0;
	    buf[0] = qc;
	    buf[1] = '\0';
	    autoq = multiquote(buf, 1);
	}
    } else {
	instring = inbackt = 0;
	autoq = NULL;
    }
    qipre = ztrdup(compqiprefix ? compqiprefix : "");
    qisuf = ztrdup(compqisuffix ? compqisuffix : "");

    useexact = (compexact && !strcmp(compexact, "accept"));

    /* Switch back to the heap that was used when the completion widget
     * was invoked. */
    SWITCHHEAPS(oldheap, compheap) {
	if ((doadd = (!dat->apar && !dat->opar && !dat->dpar))) {
	    if (dat->aflags & CAF_MATCH)
		hasmatched = 1;
	    else
		hasunmatched = 1;
	}
	if (dat->apar)
	    aparl = newlinklist();
	if (dat->opar)
	    oparl = newlinklist();
	if (dat->dpar) {
	    if (*(dat->dpar) == '(')
		dparr = NULL;
	    else if ((dparr = get_user_var(dat->dpar)) && !*dparr)
		dparr = NULL;
	    dparl = newlinklist();
	}
	if (dat->exp) {
	    curexpl = (Cexpl) zhalloc(sizeof(struct cexpl));
	    curexpl->count = curexpl->fcount = 0;
	    curexpl->str = dupstring(dat->exp);
	} else
	    curexpl = NULL;

	/* Store the matcher in our stack of matchers. */
	if (dat->match) {
	    mst.next = mstack;
	    mst.matcher = dat->match;
	    mstack = &mst;

	    add_bmatchers(dat->match);

	    addlinknode(matchers, dat->match);
	    dat->match->refc++;
	}
	if (mnum && (mstack || bmatchers))
	    update_bmatchers();

	/* Get the suffixes to ignore. */
	if (dat->ign && (aign = get_user_var(dat->ign))) {
	    char **ap, **sp, *tmp;
	    Patprog *pp, prog;

	    pign = (Patprog *) zhalloc((arrlen(aign) + 1) * sizeof(Patprog));

	    for (ap = sp = aign, pp = pign; (tmp = *ap); ap++) {
		tokenize(tmp);
		remnulargs(tmp);
		if (((tmp[0] == Quest && tmp[1] == Star) ||
		     (tmp[1] == Quest && tmp[0] == Star)) &&
		    tmp[2] && !haswilds(tmp + 2))
		    untokenize(*sp++ = tmp + 2);
		else if ((prog = patcompile(tmp, 0, NULL)))
		    *pp++ = prog;
	    }
	    *sp = NULL;
	    *pp = NULL;
	    if (!*aign)
		aign = NULL;
	    if (!*pign)
		pign = NULL;
	}
	/* Get the display strings. */
	if (dat->disp)
	    if ((disp = get_user_var(dat->disp)))
		disp--;
	/* Get the contents of the completion variables if we have
	 * to perform matching. */
	if (dat->aflags & CAF_MATCH) {
	    lipre = dupstring(compiprefix);
	    lisuf = dupstring(compisuffix);
	    lpre = dupstring(compprefix);
	    lsuf = dupstring(compsuffix);
	    llpl = strlen(lpre);
	    llsl = strlen(lsuf);

	    if (llpl + strlen(compqiprefix) + strlen(lipre) != origlpre ||
		llsl + strlen(compqisuffix) + strlen(lisuf) != origlsuf)
		lenchanged = 1;

	    /* Test if there is an existing -P prefix. */
	    if (dat->pre && *dat->pre) {
		pl = pfxlen(dat->pre, lpre);
		llpl -= pl;
		lpre += pl;
	    }
	}
	/* Now duplicate the strings we have from the command line. */
	if (dat->ipre)
	    dat->ipre = (lipre ? dyncat(lipre, dat->ipre) :
			 dupstring(dat->ipre));
	else if (lipre)
	    dat->ipre = lipre;
	if (dat->isuf)
	    dat->isuf = (lisuf ? dyncat(lisuf, dat->isuf) :
			 dupstring(dat->isuf));
	else if (lisuf)
	    dat->isuf = lisuf;
	if (dat->ppre) {
	    dat->ppre = ((dat->flags & CMF_FILE) ?
			 tildequote(dat->ppre, !!(dat->aflags & CAF_QUOTE)) :
			 multiquote(dat->ppre, !!(dat->aflags & CAF_QUOTE)));
	    lpl = strlen(dat->ppre);
	} else
	    lpl = 0;
	if (dat->psuf) {
	    dat->psuf = multiquote(dat->psuf, !!(dat->aflags & CAF_QUOTE));
	    lsl = strlen(dat->psuf);
	} else
	    lsl = 0;
	if (dat->aflags & CAF_MATCH) {
	    int ml, gfl = 0;
	    char *globflag = NULL;

	    if (comppatmatch && *comppatmatch &&
		dat->ppre && lpre[0] == '(' && lpre[1] == '#') {
		char *p;

		for (p = lpre + 2; *p && *p != ')'; p++);

		if (*p == ')') {
		    char sav = p[1];

		    p[1] = '\0';
		    globflag = dupstring(lpre);
		    gfl = p - lpre + 1;
		    p[1] = sav;

		    lpre = p + 1;
		    llpl -= gfl;
		}
	    }
	    if ((s = dat->ppre)) {
		if ((ml = match_str(lpre, s, &bpl, 0, NULL, 0, 0, 1)) >= 0) {
		    if (matchsubs) {
			Cline tmp = get_cline(NULL, 0, NULL, 0, NULL, 0, 0);

			tmp->prefix = matchsubs;
			if (matchlastpart)
			    matchlastpart->next = tmp;
			else
			    matchparts = tmp;
		    }
		    pline = matchparts;
		    lpre += ml;
		    llpl -= ml;
		    bcp = ml;
		    bpadd = strlen(s) - ml;
		} else {
		    if (llpl <= lpl && strpfx(lpre, s))
			lpre = dupstring("");
		    else if (llpl > lpl && strpfx(s, lpre))
			lpre += lpl;
		    else
			*argv = NULL;
		    bcp = lpl;
		}
	    }
	    if ((s = dat->psuf)) {
		if ((ml = match_str(lsuf, s, &bsl, 0, NULL, 1, 0, 1)) >= 0) {
		    if (matchsubs) {
			Cline tmp = get_cline(NULL, 0, NULL, 0, NULL, 0, CLF_SUF);

			tmp->suffix = matchsubs;
			if (matchlastpart)
			    matchlastpart->next = tmp;
			else
			    matchparts = tmp;
		    }
		    sline = revert_cline(matchparts);
		    lsuf[llsl - ml] = '\0';
		    llsl -= ml;
		    bcs = ml;
		    bsadd = strlen(s) - ml;
		} else {
		    if (llsl <= lsl && strsfx(lsuf, s))
			lsuf = dupstring("");
		    else if (llsl > lsl && strsfx(s, lsuf))
			lsuf[llsl - lsl] = '\0';
		    else
			*argv = NULL;
		    bcs = lsl;
		}
	    }
	    if (comppatmatch && *comppatmatch) {
		int is = (*comppatmatch == '*');
		char *tmp = (char *) zhalloc(2 + llpl + llsl + gfl);

		if (gfl) {
		    strcpy(tmp, globflag);
		    strcat(tmp, lpre);
		} else
		    strcpy(tmp, lpre);
		tmp[llpl + gfl] = 'x';
		strcpy(tmp + llpl + gfl + is, lsuf);

		tokenize(tmp);
		remnulargs(tmp);
		if (haswilds(tmp)) {
		    if (is)
			tmp[llpl + gfl] = Star;
		    if ((cp = patcompile(tmp, 0, NULL)))
			haspattern = 1;
		}
	    }
	}
	/* Select the group in which to store the matches. */
	gflags = (((dat->aflags & CAF_NOSORT ) ? CGF_NOSORT  : 0) |
		  ((dat->aflags & CAF_UNIQALL) ? CGF_UNIQALL : 0) |
		  ((dat->aflags & CAF_UNIQCON) ? CGF_UNIQCON : 0));
	if (dat->group) {
	    endcmgroup(NULL);
	    begcmgroup(dat->group, gflags);
	} else {
	    endcmgroup(NULL);
	    begcmgroup("default", 0);
	}
	if (dat->mesg)
	    addmesg(dat->mesg);
	if (*argv) {
	    if (dat->pre)
		dat->pre = dupstring(dat->pre);
	    if (dat->suf)
		dat->suf = dupstring(dat->suf);
	    if (!dat->prpre && (dat->prpre = oppre)) {
		singsub(&(dat->prpre));
		untokenize(dat->prpre);
	    } else
		dat->prpre = dupstring(dat->prpre);
	    /* Select the set of matches. */

	    if (dat->remf) {
		dat->remf = dupstring(dat->remf);
		dat->rems = NULL;
	    } else if (dat->rems)
		dat->rems = dupstring(dat->rems);

	    if (lpre)
		lpre = ((!(dat->aflags & CAF_QUOTE) &&
			 (!dat->ppre && (dat->flags & CMF_FILE))) ?
			tildequote(lpre, 1) : multiquote(lpre, 1));
	    if (lsuf)
		lsuf = multiquote(lsuf, 1);
	}
	/* Walk through the matches given. */
	obpl = bpl;
	obsl = bsl;
	if (dat->aflags & CAF_ARRAYS) {
	    Heap oldheap2;

	    SWITCHHEAPS(oldheap2, oldheap) {
		arrays = argv;
		argv = NULL;
		while (*arrays &&
		       (!(argv = get_data_arr(*arrays,
					      (dat->aflags & CAF_KEYS))) ||
			!*argv))
		    arrays++;
		arrays++;
		if (!argv) {
		    ms = NULL;
		    argv = &ms;
		}
	    } SWITCHBACKHEAPS(oldheap2);
	}
	if (dat->ppre)
	    ppl = strlen(dat->ppre);
	if (dat->psuf)
	    psl = strlen(dat->psuf);
	for (; (s = *argv); argv++) {
	    bpl = obpl;
	    bsl = obsl;
	    if (disp) {
		if (!*++disp)
		    disp = NULL;
	    }
	    sl = strlen(s);
	    if (aign || pign) {
		int il = ppl + sl + psl, addit = 1;

		if (il + 1> ilen)
		    ibuf = (char *) zhalloc((ilen = il) + 2);

		if (ppl)
		    memcpy(ibuf, dat->ppre, ppl);
		strcpy(ibuf + ppl, s);
		if (psl)
		    strcpy(ibuf + ppl + sl, dat->psuf);

		if (aign) {
		    /* Do the suffix-test. If the match has one of the
		     * suffixes from aign, we put it in the alternate set. */
		    char **pt = aign;
		    int filell;

		    for (; addit && *pt; pt++)
			addit = !((filell = strlen(*pt)) < il &&
				  !strcmp(*pt, ibuf + il - filell));
		}
		if (addit && pign) {
		    Patprog *pt = pign;

		    for (; addit && *pt; pt++)
			addit = !pattry(*pt, ibuf);
		}
		if (!addit) {
		    compignored++;
		    if (dparr && !*++dparr)
			dparr = NULL;
		    continue;
		}
	    }
	    if (!(dat->aflags & CAF_MATCH)) {
		if (dat->aflags & CAF_QUOTE)
		    ms = dupstring(s);
		else
		    sl = strlen(ms = multiquote(s, 0));
		lc = bld_parts(ms, sl, -1, NULL);
		isexact = 0;
	    } else if (!(ms = comp_match(lpre, lsuf, s, cp, &lc,
					 (!(dat->aflags & CAF_QUOTE) ?
					  (dat->ppre ||
					   !(dat->flags & CMF_FILE) ? 1 : 2) : 0),
					 &bpl, bcp, &bsl, bcs,
					 &isexact))) {
		if (dparr && !*++dparr)
		    dparr = NULL;
		continue;
	    }
	    if (doadd) {
		Brinfo bp;

		for (bp = obpl; bp; bp = bp->next)
		    bp->curpos += bpadd;
		for (bp = obsl; bp; bp = bp->next)
		    bp->curpos += bsadd;

		if ((cm = add_match_data(0, ms, lc, dat->ipre, NULL,
					 dat->isuf, dat->pre, dat->prpre,
					 dat->ppre, pline,
					 dat->psuf, sline,
					 dat->suf, dat->flags, isexact))) {
		    cm->rems = dat->rems;
		    cm->remf = dat->remf;
		    if (disp)
			cm->disp = dupstring(*disp);
		}
	    } else {
		if (dat->apar)
		    addlinknode(aparl, ms);
		if (dat->opar)
		    addlinknode(oparl, s);
		if (dat->dpar && dparr) {
		    addlinknode(dparl, *dparr);
		    if (!*++dparr)
			dparr = NULL;
		}
		free_cline(lc);
	    }
	    if ((dat->aflags & CAF_ARRAYS) && !argv[1]) {
		Heap oldheap2;

		SWITCHHEAPS(oldheap2, oldheap) {
		    argv = NULL;
		    while (*arrays &&
			   (!(argv = get_data_arr(*arrays,
						  (dat->aflags & CAF_KEYS))) ||
			    !*argv))
			arrays++;
		    arrays++;
		    if (!argv) {
			ms = NULL;
			argv = &ms;
		    }
		    argv--;
		} SWITCHBACKHEAPS(oldheap2);
	    }
	}
	if (dat->apar)
	    set_list_array(dat->apar, aparl);
	if (dat->opar)
	    set_list_array(dat->opar, oparl);
	if (dat->dpar)
	    set_list_array(dat->dpar, dparl);
	if (dat->exp)
	    addexpl();
	if (!hasallmatch && (dat->aflags & CAF_ALL)) {
	    Cmatch cm = (Cmatch) zhalloc(sizeof(struct cmatch));

	    memset(cm, 0, sizeof(struct cmatch));
	    cm->str = dupstring("<all>");
	    cm->flags = (dat->flags | CMF_ALL |
			 (complist ?
			  ((strstr(complist, "packed") ? CMF_PACKED : 0) |
			   (strstr(complist, "rows")   ? CMF_ROWS   : 0)) : 0));
	    if (disp) {
		if (!*++disp)
		    disp = NULL;
		if (disp)
		    cm->disp = dupstring(*disp);
	    } else {
		cm->disp = dupstring("");
		cm->flags |= CMF_DISPLINE;
	    }
	    mnum++;
	    ainfo->count++;
	    if (curexpl)
		curexpl->count++;

	    addlinknode(matches, cm);

	    newmatches = 1;
	    mgroup->new = 1;

	    hasallmatch = 1;
	}
    } SWITCHBACKHEAPS(oldheap);

    /* We switched back to the current heap, now restore the stack of
     * matchers. */
    mstack = oms;

    instring = ois;
    inbackt = oib;
    autoq = oaq;
    zsfree(qipre);
    zsfree(qisuf);
    qipre = oqp;
    qisuf = oqs;

    if (mnum == nm)
	haspattern = ohp;

    return (mnum == nm);
}

/* This adds all the data we have for a match. */

/**/
mod_export Cmatch
add_match_data(int alt, char *str, Cline line,
	       char *ipre, char *ripre, char *isuf,
	       char *pre, char *prpre,
	       char *ppre, Cline pline,
	       char *psuf, Cline sline,
	       char *suf, int flags, int exact)
{
    Cmatch cm;
    Aminfo ai = (alt ? fainfo : ainfo);
    int palen, salen, qipl, ipl, pl, ppl, qisl, isl, psl;
    int sl, lpl, lsl, ml;

    palen = salen = qipl = ipl = pl = ppl = qisl = isl = psl = 0;

    DPUTS(!line, "BUG: add_match_data() without cline");

    cline_matched(line);
    if (pline)
	cline_matched(pline);
    if (sline)
	cline_matched(sline);

    /* If there is a path suffix, we build a cline list for it and
     * append it to the list for the match itself. */
    if (!sline && psuf)
	salen = (psl = strlen(psuf));
    if (isuf)
	salen += (isl = strlen(isuf));
    if (qisuf)
	salen += (qisl = strlen(qisuf));

    if (salen) {
	Cline pp, p, s, sl = NULL;

	for (pp = NULL, p = line; p->next; pp = p, p = p->next);

	if (psl) {
	    s = bld_parts(psuf, psl, psl, &sl);

	    if (sline) {
		Cline sp;

		sline = cp_cline(sline, 1);

		for (sp = sline; sp->next; sp = sp->next);
		sp->next = s;
		s = sline;
		sline = NULL;
	    }
	    if (!(p->flags & (CLF_SUF | CLF_MID)) &&
		!p->llen && !p->wlen && !p->olen) {
		if (p->prefix) {
		    Cline q;

		    for (q = p->prefix; q->next; q = q->next);
		    q->next = s->prefix;
		    s->prefix = p->prefix;
		    p->prefix = NULL;
		}
		s->flags |= (p->flags & CLF_MATCHED) | CLF_MID;
		free_cline(p);
		if (pp)
		    pp->next = s;
		else
		    line = s;
	    } else
		p->next = s;
	}
	if (isl) {
	    Cline tsl;

	    s = bld_parts(isuf, isl, isl, &tsl);

	    if (sl)
		sl->next = s;
	    else if (sline) {
		Cline sp;

		sline = cp_cline(sline, 1);

		for (sp = sline; sp->next; sp = sp->next);
		sp->next = s;
		p->next = sline;
		sline = NULL;
	    } else
		p->next = s;

	    sl = tsl;
	}
	if (qisl) {
	    Cline qsl = bld_parts(dupstring(qisuf), qisl, qisl, NULL);

	    qsl->flags |= CLF_SUF;
	    qsl->suffix = qsl->prefix;
	    qsl->prefix = NULL;
	    if (sl)
		sl->next = qsl;
	    else if (sline) {
		Cline sp;

		sline = cp_cline(sline, 1);

		for (sp = sline; sp->next; sp = sp->next);
		sp->next = qsl;
		p->next = sline;
	    } else
		p->next = qsl;
	}
    } else if (sline) {
	Cline p;

	for (p = line; p->next; p = p->next);
	p->next = cp_cline(sline, 1);
    }
    /* The prefix is handled differently because the completion code
     * is much more eager to insert the -P prefix than it is to insert
     * the -S suffix. */
    if (qipre)
	palen = (qipl = strlen(qipre));
    if (ipre)
	palen += (ipl = strlen(ipre));
    if (pre)
	palen += (pl = strlen(pre));
    if (!pline && ppre)
	palen += (ppl = strlen(ppre));

    if (pl) {
	if (ppl || pline) {
	    Cline lp, p;

	    if (pline)
		for (p = cp_cline(pline, 1), lp = p; lp->next; lp = lp->next);
	    else
		p = bld_parts(ppre, ppl, ppl, &lp);

	    if (lp->prefix && !(line->flags & (CLF_SUF | CLF_MID)) &&
		!lp->llen && !lp->wlen && !lp->olen) {
		Cline lpp;

		for (lpp = lp->prefix; lpp->next; lpp = lpp->next);

		lpp->next = line->prefix;
		line->prefix = lp->prefix;
		lp->prefix = NULL;

		free_cline(lp);

		if (p != lp) {
		    Cline q;

		    for (q = p; q->next != lp; q = q->next);

		    q->next = line;
		    line = p;
		}
	    } else {
		lp->next = line;
		line = p;
	    }
	}
	if (pl) {
	    Cline lp, p = bld_parts(pre, pl, pl, &lp);

	    lp->next = line;
	    line = p;
	}
	if (ipl) {
	    Cline lp, p = bld_parts(ipre, ipl, ipl, &lp);

	    lp->next = line;
	    line = p;
	}
	if (qipl) {
	    Cline lp, p = bld_parts(dupstring(qipre), qipl, qipl, &lp);

	    lp->next = line;
	    line = p;
	}
    } else if (palen || pline) {
	Cline p, lp;

	if (palen) {
	    char *apre = (char *) zhalloc(palen);

	    if (qipl)
		memcpy(apre, qipre, qipl);
	    if (ipl)
		memcpy(apre + qipl, ipre, ipl);
	    if (pl)
		memcpy(apre + qipl + ipl, pre, pl);
	    if (ppl)
		memcpy(apre + qipl + ipl + pl, ppre, ppl);

	    p = bld_parts(apre, palen, palen, &lp);

	    if (pline)
		for (lp->next = cp_cline(pline, 1); lp->next; lp = lp->next);
	} else
	    for (p = lp = cp_cline(pline, 1); lp->next; lp = lp->next);

	if (lp->prefix && !(line->flags & CLF_SUF) &&
	    !lp->llen && !lp->wlen && !lp->olen) {
	    Cline lpp;

	    for (lpp = lp->prefix; lpp->next; lpp = lpp->next);

	    lpp->next = line->prefix;
	    line->prefix = lp->prefix;
	    lp->prefix = NULL;

	    free_cline(lp);

	    if (p != lp) {
		Cline q;

		for (q = p; q->next != lp; q = q->next);

		q->next = line;
		line = p;
	    }
	} else {
	    lp->next = line;
	    line = p;
	}
    }
    /* Allocate and fill the match structure. */
    cm = (Cmatch) zhalloc(sizeof(struct cmatch));
    cm->str = str;
    cm->ppre = (ppre && *ppre ? ppre : NULL);
    cm->psuf = (psuf && *psuf ? psuf : NULL);
    cm->prpre = ((flags & CMF_FILE) && prpre && *prpre ? prpre : NULL);
    if (qipre && *qipre)
	cm->ipre = (ipre && *ipre ? dyncat(qipre, ipre) : dupstring(qipre));
    else
	cm->ipre = (ipre && *ipre ? ipre : NULL);
    cm->ripre = (ripre && *ripre ? ripre : NULL);
    if (qisuf && *qisuf)
	cm->isuf = (isuf && *isuf ? dyncat(isuf, qisuf) : dupstring(qisuf));
    else
	cm->isuf = (isuf && *isuf ? isuf : NULL);
    cm->pre = pre;
    cm->suf = suf;
    cm->flags = (flags |
		 (complist ?
		  ((strstr(complist, "packed") ? CMF_PACKED : 0) |
		   (strstr(complist, "rows")   ? CMF_ROWS   : 0)) : 0));

    if ((*compqstack == '\\' && compqstack[1]) ||
	(autoq && *compqstack && compqstack[1] == '\\'))
	cm->flags |= CMF_NOSPACE;
    if (nbrbeg) {
	int *p;
	Brinfo bp;

	cm->brpl = (int *) zhalloc(nbrbeg * sizeof(int));

	for (p = cm->brpl, bp = brbeg; bp; p++, bp = bp->next)
	    *p = bp->curpos;
    } else
	cm->brpl = NULL;
    if (nbrend) {
	int *p;
	Brinfo bp;

	cm->brsl = (int *) zhalloc(nbrend * sizeof(int));

	for (p = cm->brsl, bp = brend; bp; p++, bp = bp->next)
	    *p = bp->curpos;
    } else
	cm->brsl = NULL;
    cm->qipl = qipl;
    cm->qisl = qisl;
    cm->autoq = dupstring(autoq ? autoq : (inbackt ? "`" : NULL));
    cm->rems = cm->remf = cm->disp = NULL;

    if ((lastprebr || lastpostbr) && !hasbrpsfx(cm, lastprebr, lastpostbr))
	return NULL;

    /* Then build the unambiguous cline list. */
    ai->line = join_clines(ai->line, line);

    mnum++;
    ai->count++;

    addlinknode((alt ? fmatches : matches), cm);

    newmatches = 1;
    mgroup->new = 1;
    if (alt)
	compignored++;

    if (!complastprompt || !*complastprompt)
	dolastprompt = 0;
    /* One more match for this explanation. */
    if (curexpl) {
	if (alt)
	    curexpl->fcount++;
	else
	    curexpl->count++;
    }
    if (!ai->firstm)
	ai->firstm = cm;

    sl = strlen(str);
    lpl = (cm->ppre ? strlen(cm->ppre) : 0);
    lsl = (cm->psuf ? strlen(cm->psuf) : 0);
    ml = sl + lpl + lsl;

    if (ml < minmlen)
	minmlen = ml;
    if (ml > maxmlen)
	maxmlen = ml;

    /* Do we have an exact match? More than one? */
    if (exact) {
	if (!ai->exact) {
	    ai->exact = useexact;
	    if (incompfunc && (!compexactstr || !*compexactstr)) {
		/* If a completion widget is active, we make the exact
		 * string available in `compstate'. */

		char *e;

		zsfree(compexactstr);
		compexactstr = e = (char *) zalloc(ml + 1);
		if (cm->ppre) {
		    strcpy(e, cm->ppre);
		    e += lpl;
		}
		strcpy(e, str);
		e += sl;
		if (cm->psuf)
		    strcpy(e, cm->psuf);
		comp_setunset(0, 0, CP_EXACTSTR, 0);
	    }
	    ai->exactm = cm;
	} else if (useexact && !matcheq(cm, ai->exactm)) {
	    ai->exact = 2;
	    ai->exactm = NULL;
	    if (incompfunc)
		comp_setunset(0, 0, 0, CP_EXACTSTR);
	}
    }
    return cm;
}

/* This begins a new group of matches. */

/**/
mod_export void
begcmgroup(char *n, int flags)
{
    if (n) {
	Cmgroup p = amatches;

	while (p) {
	    if (p->name &&
		flags == (p->flags & (CGF_NOSORT|CGF_UNIQALL|CGF_UNIQCON)) &&
		!strcmp(n, p->name)) {
		mgroup = p;

		expls = p->lexpls;
		matches = p->lmatches;
		fmatches = p->lfmatches;
		allccs = p->lallccs;

		return;
	    }
	    p = p->next;
	}
    }
    mgroup = (Cmgroup) zhalloc(sizeof(struct cmgroup));
    mgroup->name = dupstring(n);
    mgroup->lcount = mgroup->llcount = mgroup->mcount = mgroup->ecount = 
	mgroup->ccount = 0;
    mgroup->flags = flags;
    mgroup->matches = NULL;
    mgroup->ylist = NULL;
    mgroup->expls = NULL;
    mgroup->perm = NULL;
    mgroup->new = mgroup->num = mgroup->nbrbeg = mgroup->nbrend = 0;

    mgroup->lexpls = expls = newlinklist();
    mgroup->lmatches = matches = newlinklist();
    mgroup->lfmatches = fmatches = newlinklist();

    mgroup->lallccs = allccs = ((flags & CGF_NOSORT) ? NULL : newlinklist());

    if ((mgroup->next = amatches))
	amatches->prev = mgroup;
    mgroup->prev = NULL;
    amatches = mgroup;
}

/* End the current group for now. */

/**/
mod_export void
endcmgroup(char **ylist)
{
    mgroup->ylist = ylist;
}

/* Add an explanation string to the current group, joining duplicates. */

/**/
mod_export void
addexpl(void)
{
    LinkNode n;
    Cexpl e;

    for (n = firstnode(expls); n; incnode(n)) {
	e = (Cexpl) getdata(n);
	if (e->count >= 0 && !strcmp(curexpl->str, e->str)) {
	    e->count += curexpl->count;
	    e->fcount += curexpl->fcount;

	    return;
	}
    }
    addlinknode(expls, curexpl);
    newmatches = 1;
}

/* Add a message to the current group. Make sure it is shown. */

/**/
mod_export void
addmesg(char *mesg)
{
    LinkNode n;
    Cexpl e;

    for (n = firstnode(expls); n; incnode(n)) {
	e = (Cexpl) getdata(n);
	if (e->count < 0 && !strcmp(mesg, e->str))
	    return;
    }
    e = (Cexpl) zhalloc(sizeof(*e));
    e->count = e->fcount = -1;
    e->str = dupstring(mesg);
    addlinknode(expls, e);
    newmatches = 1;
    mgroup->new = 1;
    nmessages++;
}

/* The comparison function for matches (used for sorting). */

/**/
static int
matchcmp(Cmatch *a, Cmatch *b)
{
    if ((*a)->disp) {
	if ((*b)->disp) {
	    if ((*a)->flags & CMF_DISPLINE) {
		if ((*b)->flags & CMF_DISPLINE)
		    return strcmp((*a)->disp, (*b)->disp);
		else
		    return -1;
	    } else {
		if ((*b)->flags & CMF_DISPLINE)
		    return 1;
		else
		    return strcmp((*a)->disp, (*b)->disp);
	    }
	}
	return -1;
    }
    if ((*b)->disp)
	return 1;

    return strbpcmp(&((*a)->str), &((*b)->str));
}

/* This tests whether two matches are equal (would produce the same
 * strings on the command line). */

#define matchstreq(a, b) ((!(a) && !(b)) || ((a) && (b) && !strcmp((a), (b))))

/**/
static int
matcheq(Cmatch a, Cmatch b)
{
    return matchstreq(a->ipre, b->ipre) &&
	matchstreq(a->pre, b->pre) &&
	matchstreq(a->ppre, b->ppre) &&
	matchstreq(a->psuf, b->psuf) &&
	matchstreq(a->suf, b->suf) &&
	((!a->disp && !b->disp && matchstreq(a->str, b->str)) ||
	 (a->disp && b->disp && !strcmp(a->disp, b->disp) &&
	  matchstreq(a->str, b->str)));
}

/* Make an array from a linked list. The second argument says whether *
 * the array should be sorted. The third argument is used to return   *
 * the number of elements in the resulting array. The fourth argument *
 * is used to return the number of NOLIST elements. */

/**/
static Cmatch *
makearray(LinkList l, int type, int flags, int *np, int *nlp, int *llp)
{
    Cmatch *ap, *bp, *cp, *rp;
    LinkNode nod;
    int n, nl = 0, ll = 0;

    /* Build an array for the matches. */
    rp = ap = (Cmatch *) hcalloc(((n = countlinknodes(l)) + 1) *
				 sizeof(Cmatch));

    /* And copy them into it. */
    for (nod = firstnode(l); nod; incnode(nod))
	*ap++ = (Cmatch) getdata(nod);
    *ap = NULL;

    if (!type) {
	if (flags) {
	    char **ap, **bp, **cp;

	    /* Now sort the array (it contains strings). */
	    qsort((void *) rp, n, sizeof(char *),
		  (int (*) _((const void *, const void *)))strbpcmp);

	    /* And delete the ones that occur more than once. */
	    for (ap = cp = (char **) rp; *ap; ap++) {
		*cp++ = *ap;
		for (bp = ap; bp[1] && !strcmp(*ap, bp[1]); bp++, n--);
		ap = bp;
	    }
	    *cp = NULL;
	}
    } else {
	if (!(flags & CGF_NOSORT)) {
	    /* Now sort the array (it contains matches). */
	    qsort((void *) rp, n, sizeof(Cmatch),
		  (int (*) _((const void *, const void *)))matchcmp);

	    if (!(flags & CGF_UNIQCON)) {
		int dup;

		/* And delete the ones that occur more than once. */
		for (ap = cp = rp; *ap; ap++) {
		    *cp++ = *ap;
		    for (bp = ap; bp[1] && matcheq(*ap, bp[1]); bp++, n--);
		    ap = bp;
		    /* Mark those, that would show the same string in the list. */
		    for (dup = 0; bp[1] && !(*ap)->disp && !(bp[1])->disp &&
			     !strcmp((*ap)->str, (bp[1])->str); bp++) {
			(bp[1])->flags |= CMF_MULT;
			dup = 1;
		    }
		    if (dup)
			(*ap)->flags |= CMF_FMULT;
		}
		*cp = NULL;
	    }
	    for (ap = rp; *ap; ap++) {
		if ((*ap)->disp && ((*ap)->flags & CMF_DISPLINE))
		    ll++;
		if ((*ap)->flags & (CMF_NOLIST | CMF_MULT))
		    nl++;
	    }
	} else {
	    if (!(flags & CGF_UNIQALL) && !(flags & CGF_UNIQCON)) {
                int dup;

		for (ap = rp; *ap; ap++) {
		    for (bp = cp = ap + 1; *bp; bp++) {
			if (!matcheq(*ap, *bp))
			    *cp++ = *bp;
			else
			    n--;
		    }
		    *cp = NULL;
                    if (!(*ap)->disp) {
                        for (dup = 0, bp = ap + 1; *bp; bp++)
                            if (!(*bp)->disp &&
                                !((*bp)->flags & CMF_MULT) &&
                                !strcmp((*ap)->str, (*bp)->str)) {
                                (*bp)->flags |= CMF_MULT;
                                dup = 1;
                            }
                        if (dup)
                            (*ap)->flags |= CMF_FMULT;
                    }
		}
	    } else if (!(flags & CGF_UNIQCON)) {
		int dup;

		for (ap = cp = rp; *ap; ap++) {
		    *cp++ = *ap;
		    for (bp = ap; bp[1] && matcheq(*ap, bp[1]); bp++, n--);
		    ap = bp;
		    for (dup = 0; bp[1] && !(*ap)->disp && !(bp[1])->disp &&
			     !strcmp((*ap)->str, (bp[1])->str); bp++) {
			(bp[1])->flags |= CMF_MULT;
			dup = 1;
		    }
		    if (dup)
			(*ap)->flags |= CMF_FMULT;
		}
		*cp = NULL;
	    }
	    for (ap = rp; *ap; ap++) {
		if ((*ap)->disp && ((*ap)->flags & CMF_DISPLINE))
		    ll++;
		if ((*ap)->flags & (CMF_NOLIST | CMF_MULT))
		    nl++;
	    }
	}
    }
    if (np)
	*np = n;
    if (nlp)
	*nlp = nl;
    if (llp)
	*llp = ll;
    return rp;
}

/* This duplicates one match. */

/**/
static Cmatch
dupmatch(Cmatch m, int nbeg, int nend)
{
    Cmatch r;

    r = (Cmatch) zcalloc(sizeof(struct cmatch));

    r->str = ztrdup(m->str);
    r->ipre = ztrdup(m->ipre);
    r->ripre = ztrdup(m->ripre);
    r->isuf = ztrdup(m->isuf);
    r->ppre = ztrdup(m->ppre);
    r->psuf = ztrdup(m->psuf);
    r->prpre = ztrdup(m->prpre);
    r->pre = ztrdup(m->pre);
    r->suf = ztrdup(m->suf);
    r->flags = m->flags;
    if (m->brpl) {
	int *p, *q, i;

	r->brpl = (int *) zalloc(nbeg * sizeof(int));

	for (p = r->brpl, q = m->brpl, i = nbeg; i--; p++, q++)
	    *p = *q;
    } else
	r->brpl = NULL;
    if (m->brsl) {
	int *p, *q, i;

	r->brsl = (int *) zalloc(nend * sizeof(int));

	for (p = r->brsl, q = m->brsl, i = nend; i--; p++, q++)
	    *p = *q;
    } else
	r->brsl = NULL;
    r->rems = ztrdup(m->rems);
    r->remf = ztrdup(m->remf);
    r->autoq = ztrdup(m->autoq);
    r->qipl = m->qipl;
    r->qisl = m->qisl;
    r->disp = ztrdup(m->disp);

    return r;
}

/* This duplicates all groups of matches. */

/**/
mod_export int
permmatches(int last)
{
    Cmgroup g = amatches, n, opm;
    Cmatch *p, *q;
    Cexpl *ep, *eq, e, o;
    LinkList mlist;
    static int fi = 0;
    int nn, nl, ll, gn = 1, mn = 1, rn, ofi = fi;

    if (pmatches && !newmatches) {
	if (last && fi)
	    ainfo = fainfo;
	return fi;
    }
    newmatches = fi = 0;

    opm = pmatches;
    pmatches = lmatches = NULL;
    nmatches = smatches = diffmatches = 0;

    if (!ainfo->count) {
	if (last)
	    ainfo = fainfo;
	fi = 1;
    }
    while (g) {
	if (fi != ofi || !g->perm || g->new) {
	    if (fi)
		/* We have no matches, try ignoring fignore. */
		mlist = g->lfmatches;
	    else
		mlist = g->lmatches;

	    g->matches = makearray(mlist, 1, g->flags, &nn, &nl, &ll);
	    g->mcount = nn;
	    if ((g->lcount = nn - nl) < 0)
		g->lcount = 0;
	    g->llcount = ll;
	    if (g->ylist) {
		g->lcount = arrlen(g->ylist);
		smatches = 2;
	    }
	    g->expls = (Cexpl *) makearray(g->lexpls, 0, 0, &(g->ecount),
					   NULL, NULL);

	    g->ccount = 0;

	    nmatches += g->mcount;
	    smatches += g->lcount;

	    if (g->mcount > 1)
		diffmatches = 1;

	    n = (Cmgroup) zcalloc(sizeof(struct cmgroup));

	    if (g->perm) {
		g->perm->next = NULL;
		freematches(g->perm, 0);
	    }
	    g->perm = n;

	    if (!lmatches)
		lmatches = n;
	    if (pmatches)
		pmatches->prev = n;
	    n->next = pmatches;
	    pmatches = n;
	    n->prev = NULL;
	    n->num = gn++;
	    n->flags = g->flags;
	    n->mcount = g->mcount;
	    n->matches = p = (Cmatch *) zcalloc((n->mcount + 1) * sizeof(Cmatch));
	    n->name = ztrdup(g->name);
	    for (q = g->matches; *q; q++, p++)
		*p = dupmatch(*q, nbrbeg, nbrend);
	    *p = NULL;

	    n->lcount = g->lcount;
	    n->llcount = g->llcount;
	    if (g->ylist)
		n->ylist = zarrdup(g->ylist);
	    else
		n->ylist = NULL;

	    if ((n->ecount = g->ecount)) {
		n->expls = ep = (Cexpl *) zcalloc((n->ecount + 1) * sizeof(Cexpl));
		for (eq = g->expls; (o = *eq); eq++, ep++) {
		    *ep = e = (Cexpl) zcalloc(sizeof(struct cexpl));
		    e->count = (fi ? o->fcount : o->count);
		    e->fcount = 0;
		    e->str = ztrdup(o->str);
		}
		*ep = NULL;
	    } else
		n->expls = NULL;

	    n->widths = NULL;
	} else {
	    if (!lmatches)
		lmatches = g->perm;
	    if (pmatches)
		pmatches->prev = g->perm;
	    g->perm->next = pmatches;
	    pmatches = g->perm;
	    g->perm->prev = NULL;

	    nmatches += g->mcount;
	    smatches += g->lcount;

	    if (g->mcount > 1)
		diffmatches = 1;

	    g->num = gn++;
	}
	g->new = 0;
	g = g->next;
    }
    for (g = pmatches, p = NULL; g; g = g->next) {
	g->nbrbeg = nbrbeg;
	g->nbrend = nbrend;
	for (rn = 1, q = g->matches; *q; q++) {
	    (*q)->rnum = rn++;
	    (*q)->gnum = mn++;
	}
	if (!diffmatches && *g->matches) {
	    if (p) {
		if (!matcheq(*g->matches, *p))
		    diffmatches = 1;
	    } else
		p = g->matches;
	}
    }
    hasperm = 1;
    permmnum = mn - 1;
    permgnum = gn - 1;
    listdat.valid = 0;

    return fi;
}

/* This frees one match. */

/**/
static void
freematch(Cmatch m, int nbeg, int nend)
{
    if (!m) return;

    zsfree(m->str);
    zsfree(m->ipre);
    zsfree(m->ripre);
    zsfree(m->isuf);
    zsfree(m->ppre);
    zsfree(m->psuf);
    zsfree(m->pre);
    zsfree(m->suf);
    zsfree(m->prpre);
    zsfree(m->rems);
    zsfree(m->remf);
    zsfree(m->disp);
    zsfree(m->autoq);
    if (m->brpl)
	zfree(m->brpl, nbeg * sizeof(int));
    if (m->brsl)
	zfree(m->brsl, nend * sizeof(int));

    zfree(m, sizeof(m));
}

/* This frees the groups of matches. */

/**/
mod_export void
freematches(Cmgroup g, int cm)
{
    Cmgroup n;
    Cmatch *m;
    Cexpl *e;

    while (g) {
	n = g->next;

	for (m = g->matches; *m; m++)
	    freematch(*m, g->nbrbeg, g->nbrend);
	free(g->matches);

	if (g->ylist)
	    freearray(g->ylist);

	if ((e = g->expls)) {
	    while (*e) {
		zsfree((*e)->str);
		free(*e);
		e++;
	    }
	    free(g->expls);
	}
	zsfree(g->name);
	free(g);

	g = n;
    }
    if (cm)
	minfo.cur = NULL;
}
