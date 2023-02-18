/*
 * loop.c - loop execution
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
#include "loop.pro"

/* # of nested loops we are in */
 
/**/
int loops;
 
/* # of continue levels */
 
/**/
mod_export int contflag;
 
/* # of break levels */
 
/**/
mod_export int breaks;

/**/
int
execfor(Estate state, int do_exec)
{
    Wordcode end, loop;
    wordcode code = state->pc[-1];
    int iscond = (WC_FOR_TYPE(code) == WC_FOR_COND), ctok = 0, atok = 0;
    char *name, *str, *cond = NULL, *advance = NULL;
    zlong val = 0;
    LinkList args = NULL;

    name = ecgetstr(state, EC_NODUP, NULL);
    end = state->pc + WC_FOR_SKIP(code);

    if (iscond) {
	str = dupstring(name);
	singsub(&str);
	if (isset(XTRACE)) {
	    char *str2 = dupstring(str);
	    untokenize(str2);
	    printprompt4();
	    fprintf(xtrerr, "%s\n", str2);
	    fflush(xtrerr);
	}
	if (!errflag)
	    matheval(str);
	if (errflag) {
	    state->pc = end;
	    return lastval = errflag;
	}
	cond = ecgetstr(state, EC_NODUP, &ctok);
	advance = ecgetstr(state, EC_NODUP, &atok);
    } else if (WC_FOR_TYPE(code) == WC_FOR_LIST) {
	int htok = 0;

	if (!(args = ecgetlist(state, *state->pc++, EC_DUPTOK, &htok))) {
	    state->pc = end;
	    return 0;
	}
	if (htok)
	    execsubst(args);
    } else {
	char **x;

	args = newlinklist();
	for (x = pparams; *x; x++)
	    addlinknode(args, dupstring(*x));
    }
    lastval = 0;
    loops++;
    pushheap();
    cmdpush(CS_FOR);
    loop = state->pc;
    for (;;) {
	if (iscond) {
	    if (ctok) {
		str = dupstring(cond);
		singsub(&str);
	    } else
		str = cond;
	    if (!errflag) {
		while (iblank(*str))
		    str++;
		if (*str) {
		    if (isset(XTRACE)) {
			printprompt4();
			fprintf(xtrerr, "%s\n", str);
			fflush(xtrerr);
		    }
		    val = mathevali(str);
		} else
		    val = 1;
	    }
	    if (errflag) {
		if (breaks)
		    breaks--;
		lastval = 1;
		break;
	    }
	    if (!val)
		break;
	} else {
	    if (!args || !(str = (char *) ugetnode(args)))
		break;
	    if (isset(XTRACE)) {
		printprompt4();
		fprintf(xtrerr, "%s=%s\n", name, str);
		fflush(xtrerr);
	    }
	    setsparam(name, ztrdup(str));
	}
	state->pc = loop;
	execlist(state, 1, do_exec && args && empty(args));
	if (breaks) {
	    breaks--;
	    if (breaks || !contflag)
		break;
	    contflag = 0;
	}
	if (retflag)
	    break;
	if (iscond && !errflag) {
	    if (atok) {
		str = dupstring(advance);
		singsub(&str);
	    } else
		str = advance;
	    if (isset(XTRACE)) {
		printprompt4();
		fprintf(xtrerr, "%s\n", str);
		fflush(xtrerr);
	    }
	    if (!errflag)
		matheval(str);
	}
	if (errflag) {
	    if (breaks)
		breaks--;
	    lastval = 1;
	    break;
	}
	freeheap();
    }
    popheap();
    cmdpop();
    loops--;
    state->pc = end;
    return lastval;
}

/**/
int
execselect(Estate state, int do_exec)
{
    Wordcode end, loop;
    wordcode code = state->pc[-1];
    char *str, *s, *name;
    LinkNode n;
    int i, usezle;
    FILE *inp;
    size_t more;
    LinkList args;

    end = state->pc + WC_FOR_SKIP(code);
    name = ecgetstr(state, EC_NODUP, NULL);

    if (WC_SELECT_TYPE(code) == WC_SELECT_PPARAM) {
	char **x;

	args = newlinklist();
	for (x = pparams; *x; x++)
	    addlinknode(args, dupstring(*x));
    } else {
	int htok = 0;

	if (!(args = ecgetlist(state, *state->pc++, EC_DUPTOK, &htok))) {
	    state->pc = end;
	    return 0;
	}
	if (htok)
	    execsubst(args);
    }
    if (!args || empty(args)) {
	state->pc = end;
	return 1;
    }
    loops++;
    lastval = 0;
    pushheap();
    cmdpush(CS_SELECT);
    usezle = interact && SHTTY != -1 && isset(USEZLE);
    inp = fdopen(dup(usezle ? SHTTY : 0), "r");
    more = selectlist(args, 0);
    loop = state->pc;
    for (;;) {
	for (;;) {
	    if (empty(bufstack)) {
	    	if (usezle) {
		    int oef = errflag;

		    isfirstln = 1;
		    str = (char *)zleread(prompt3, NULL, 0);
		    if (errflag)
			str = NULL;
		    errflag = oef;
	    	} else {
		    str = promptexpand(prompt3, 0, NULL, NULL);
		    zputs(str, stderr);
		    free(str);
		    fflush(stderr);
		    str = fgets(zalloc(256), 256, inp);
	    	}
	    } else
		str = (char *)getlinknode(bufstack);
	    if (!str || errflag) {
		if (breaks)
		    breaks--;
		fprintf(stderr, "\n");
		fflush(stderr);
		goto done;
	    }
	    if ((s = strchr(str, '\n')))
		*s = '\0';
	    if (*str)
	      break;
	    more = selectlist(args, more);
	}
	setsparam("REPLY", ztrdup(str));
	i = atoi(str);
	if (!i)
	    str = "";
	else {
	    for (i--, n = firstnode(args); n && i; incnode(n), i--);
	    if (n)
		str = (char *) getdata(n);
	    else
		str = "";
	}
	setsparam(name, ztrdup(str));
	state->pc = loop;
	execlist(state, 1, 0);
	freeheap();
	if (breaks) {
	    breaks--;
	    if (breaks || !contflag)
		break;
	    contflag = 0;
	}
	if (retflag || errflag)
	    break;
    }
  done:
    cmdpop();
    popheap();
    fclose(inp);
    loops--;
    state->pc = end;
    return lastval;
}

/* And this is used to print select lists. */

/**/
size_t
selectlist(LinkList l, size_t start)
{
    size_t longest = 1, fct, fw = 0, colsz, t0, t1, ct;
    LinkNode n;
    char **arr, **ap;

    trashzle();
    ct = countlinknodes(l);
    ap = arr = (char **) zhalloc((countlinknodes(l) + 1) * sizeof(char **));

    for (n = (LinkNode) firstnode(l); n; incnode(n))
	*ap++ = (char *)getdata(n);
    *ap = NULL;
    for (ap = arr; *ap; ap++)
	if (strlen(*ap) > longest)
	    longest = strlen(*ap);
    t0 = ct;
    longest++;
    while (t0)
	t0 /= 10, longest++;
    /* to compensate for added ')' */
    fct = (columns - 1) / (longest + 3);
    if (fct == 0)
	fct = 1;
    else
	fw = (columns - 1) / fct;
    colsz = (ct + fct - 1) / fct;
    for (t1 = start; t1 != colsz && t1 - start < lines - 2; t1++) {
	ap = arr + t1;
	do {
	    int t2 = strlen(*ap) + 2, t3;

	    fprintf(stderr, "%d) %s", t3 = ap - arr + 1, *ap);
	    while (t3)
		t2++, t3 /= 10;
	    for (; t2 < fw; t2++)
		fputc(' ', stderr);
	    for (t0 = colsz; t0 && *ap; t0--, ap++);
	}
	while (*ap);
	fputc('\n', stderr);
    }

 /* Below is a simple attempt at doing it the Korn Way..
       ap = arr;
       t0 = 0;
       do {
           t0++;
           fprintf(stderr,"%d) %s\n",t0,*ap);
           ap++;
       }
       while (*ap);*/
    fflush(stderr);

    return t1 < colsz ? t1 : 0;
}

/**/
int
execwhile(Estate state, int do_exec)
{
    Wordcode end, loop;
    wordcode code = state->pc[-1];
    int olderrexit, oldval, isuntil = (WC_WHILE_TYPE(code) == WC_WHILE_UNTIL);

    end = state->pc + WC_WHILE_SKIP(code);
    olderrexit = noerrexit;
    oldval = 0;
    pushheap();
    cmdpush(isuntil ? CS_UNTIL : CS_WHILE);
    loops++;
    loop = state->pc;
    for (;;) {
	state->pc = loop;
	noerrexit = 1;
	execlist(state, 1, 0);
	noerrexit = olderrexit;
	if (!((lastval == 0) ^ isuntil)) {
	    if (breaks)
		breaks--;
	    lastval = oldval;
	    break;
	}
	if (retflag) {
	    lastval = oldval;
	    break;
	}
	execlist(state, 1, 0);
	if (breaks) {
	    breaks--;
	    if (breaks || !contflag)
		break;
	    contflag = 0;
	}
	if (errflag) {
	    lastval = 1;
	    break;
	}
	if (retflag)
	    break;
	freeheap();
	oldval = lastval;
    }
    cmdpop();
    popheap();
    loops--;
    state->pc = end;
    return lastval;
}

/**/
int
execrepeat(Estate state, int do_exec)
{
    Wordcode end, loop;
    wordcode code = state->pc[-1];
    int count, htok = 0;
    char *tmp;

    end = state->pc + WC_REPEAT_SKIP(code);

    lastval = 0;
    tmp = ecgetstr(state, EC_DUPTOK, &htok);
    if (htok)
	singsub(&tmp);
    count = atoi(tmp);
    pushheap();
    cmdpush(CS_REPEAT);
    loops++;
    loop = state->pc;
    while (count-- > 0) {
	state->pc = loop;
	execlist(state, 1, 0);
	freeheap();
	if (breaks) {
	    breaks--;
	    if (breaks || !contflag)
		break;
	    contflag = 0;
	}
	if (errflag) {
	    lastval = 1;
	    break;
	}
	if (retflag)
	    break;
    }
    cmdpop();
    popheap();
    loops--;
    state->pc = end;
    return lastval;
}

/**/
int
execif(Estate state, int do_exec)
{
    Wordcode end, next;
    wordcode code = state->pc[-1];
    int olderrexit, s = 0, run = 0;

    olderrexit = noerrexit;
    end = state->pc + WC_IF_SKIP(code);

    if (!noerrexit)
	noerrexit = 1;
    while (state->pc < end) {
	code = *state->pc++;
	if (wc_code(code) != WC_IF ||
	    (run = (WC_IF_TYPE(code) == WC_IF_ELSE))) {
	    if (run)
		run = 2;
	    break;
	}
	next = state->pc + WC_IF_SKIP(code);
	cmdpush(s ? CS_ELIF : CS_IF);
	execlist(state, 1, 0);
	cmdpop();
	if (!lastval) {
	    run = 1;
	    break;
	}
	if (retflag)
	    break;
	s = 1;
	state->pc = next;
    }
    noerrexit = olderrexit;

    if (run) {
	cmdpush(run == 2 ? CS_ELSE : (s ? CS_ELIFTHEN : CS_IFTHEN));
	execlist(state, 1, do_exec);
	cmdpop();
    } else
	lastval = 0;
    state->pc = end;

    return lastval;
}

/**/
int
execcase(Estate state, int do_exec)
{
    Wordcode end, next;
    wordcode code = state->pc[-1];
    char *word, *pat;
    int npat, save;
    Patprog *spprog, pprog;

    end = state->pc + WC_CASE_SKIP(code);

    word = ecgetstr(state, EC_DUP, NULL);
    singsub(&word);
    untokenize(word);
    lastval = 0;

    cmdpush(CS_CASE);
    while (state->pc < end) {
	code = *state->pc++;
	if (wc_code(code) != WC_CASE)
	    break;

	pat = NULL;
	pprog = NULL;
	save = 0;
	npat = state->pc[1];
	spprog = state->prog->pats + npat;

	next = state->pc + WC_CASE_SKIP(code);

	if (isset(XTRACE)) {
	    char *pat2, *opat;

	    pat = dupstring(opat = ecrawstr(state->prog, state->pc, NULL));
	    singsub(&pat);
	    save = (!(state->prog->flags & EF_HEAP) &&
		    !strcmp(pat, opat) && *spprog != dummy_patprog2);

	    pat2 = dupstring(pat);
	    untokenize(pat2);
	    printprompt4();
	    fprintf(xtrerr, "case %s (%s)\n", word, pat2);
	    fflush(xtrerr);
	}
	state->pc += 2;

	if (*spprog != dummy_patprog1 && *spprog != dummy_patprog2)
	    pprog = *spprog;

	if (!pprog) {
	    if (!pat) {
		char *opat;
		int htok = 0;

		pat = dupstring(opat = ecrawstr(state->prog,
						state->pc - 2, &htok));
		if (htok)
		    singsub(&pat);
		save = (!(state->prog->flags & EF_HEAP) &&
			!strcmp(pat, opat) && *spprog != dummy_patprog2);
	    }
	    if (!(pprog = patcompile(pat, (save ? PAT_ZDUP : PAT_STATIC),
				     NULL)))
		zerr("bad pattern: %s", pat, 0);
	    else if (save)
		*spprog = pprog;
	}
	if (pprog && pattry(pprog, word)) {
	    execlist(state, 1, ((WC_CASE_TYPE(code) == WC_CASE_OR) &&
				do_exec));
	    while (!retflag && wc_code(code) == WC_CASE &&
		   WC_CASE_TYPE(code) == WC_CASE_AND) {
		state->pc = next;
		code = *state->pc;
		state->pc += 3;
		next = state->pc + WC_CASE_SKIP(code) - 2;
		execlist(state, 1, ((WC_CASE_TYPE(code) == WC_CASE_OR) &&
				    do_exec));
	    }
	    break;
	} else
	    state->pc = next;
    }
    cmdpop();

    state->pc = end;

    return lastval;
}
