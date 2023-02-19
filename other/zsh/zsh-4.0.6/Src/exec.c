/*
 * exec.c - command execution
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
#include "exec.pro"

/* used to suppress ERREXIT and trapping of SIGZERR, SIGEXIT. */

/**/
int noerrexit;

/* suppress error messages */
 
/**/
mod_export int noerrs;
 
/* do not save history on exec and exit */

/**/
int nohistsave;
 
/* error/break flag */
 
/**/
mod_export int errflag;
 
/* Status of return from a trap */
 
/**/
int trapreturn;
 
/* != 0 if this is a subshell */
 
/**/
int subsh;
 
/* != 0 if we have a return pending */
 
/**/
mod_export int retflag;

/**/
long lastval2;
 
/* The table of file descriptors.  A table element is zero if the  *
 * corresponding fd is not used by the shell.  It is greater than  *
 * 1 if the fd is used by a <(...) or >(...) substitution and 1 if *
 * it is an internal file descriptor which must be closed before   *
 * executing an external command.  The first ten elements of the   *
 * table is not used.  A table element is set by movefd and cleard *
 * by zclose.                                                      */

/**/
char *fdtable;

/* The allocated size of fdtable */

/**/
int fdtable_size;

/* The highest fd that marked with nonzero in fdtable */

/**/
int max_zsh_fd;

/* input fd from the coprocess */

/**/
mod_export int coprocin;

/* output fd from the coprocess */

/**/
mod_export int coprocout;

/* != 0 if the line editor is active */

/**/
mod_export int zleactive;

/* pid of process undergoing 'process substitution' */
 
/**/
pid_t cmdoutpid;
 
/* exit status of process undergoing 'process substitution' */
 
/**/
int cmdoutval;

/* The context in which a shell function is called, see SFC_* in zsh.h. */ 

/**/
mod_export int sfcontext;

/* Stack to save some variables before executing a signal handler function */

/**/
struct execstack *exstack;

/* Stack with names of functions currently active. */

/**/
mod_export Funcstack funcstack;

#define execerr() if (!forked) { lastval = 1; goto done; } else _exit(1)

static LinkList args;
static int doneps4;
static char *STTYval;

/* Execution functions. */

static int (*execfuncs[]) _((Estate, int)) = {
    execcursh, exectime, execfuncdef, execfor, execselect,
    execwhile, execrepeat, execcase, execif, execcond,
    execarith, execautofn
};

/* parse string into a list */

/**/
mod_export Eprog
parse_string(char *s, int ln)
{
    Eprog p;
    int oldlineno = lineno;

    lexsave();
    inpush(s, (ln ? INP_LINENO : 0), NULL);
    strinbeg(0);
    lineno = ln ? 1 : -1;
    p = parse_list();
    lineno = oldlineno;
    strinend();
    inpop();
    lexrestore();
    return p;
}

/**/
#ifdef HAVE_GETRLIMIT

/* the resource limits for the shell and its children */

/**/
mod_export struct rlimit current_limits[RLIM_NLIMITS], limits[RLIM_NLIMITS];
 
/**/
mod_export int
zsetlimit(int limnum, char *nam)
{
    if (limits[limnum].rlim_max != current_limits[limnum].rlim_max ||
	limits[limnum].rlim_cur != current_limits[limnum].rlim_cur) {
	if (setrlimit(limnum, limits + limnum)) {
	    if (nam)
		zwarnnam(nam, "setrlimit failed: %e", NULL, errno);
	    return -1;
	}
	current_limits[limnum] = limits[limnum];
    }
    return 0;
}

/**/
mod_export int
setlimits(char *nam)
{
    int limnum;
    int ret = 0;

    for (limnum = 0; limnum < RLIM_NLIMITS; limnum++)
	if (zsetlimit(limnum, nam))
	    ret++;
    return ret;
}

/**/
#endif /* HAVE_GETRLIMIT */

/* fork and set limits */

/**/
static pid_t
zfork(void)
{
    pid_t pid;

    if (thisjob >= MAXJOB - 1) {
	zerr("job table full", NULL, 0);
	return -1;
    }
    pid = fork();
    if (pid == -1) {
	zerr("fork failed: %e", NULL, errno);
	return -1;
    }
#ifdef HAVE_GETRLIMIT
    if (!pid)
	/* set resource limits for the child process */
	setlimits(NULL);
#endif
    return pid;
}

/*
 *   Allen Edeln gebiet ich Andacht,
 *   Hohen und Niedern von Heimdalls Geschlecht;
 *   Ich will list_pipe's Wirken kuenden
 *   Die aeltesten Sagen, der ich mich entsinne...
 *
 * In most shells, if you do something like:
 *
 *   cat foo | while read a; do grep $a bar; done
 *
 * the shell forks and executes the loop in the sub-shell thus created.
 * In zsh this traditionally executes the loop in the current shell, which
 * is nice to have if the loop does something to change the shell, like
 * setting parameters or calling builtins.
 * Putting the loop in a sub-shell makes live easy, because the shell only
 * has to put it into the job-structure and then treats it as a normal
 * process. Suspending and interrupting is no problem then.
 * Some years ago, zsh either couldn't suspend such things at all, or
 * it got really messed up when users tried to do it. As a solution, we
 * implemented the list_pipe-stuff, which has since then become a reason
 * for many nightmares.
 * Pipelines like the one above are executed by the functions in this file
 * which call each other (and sometimes recursively). The one above, for
 * example would lead to a function call stack roughly like:
 *
 *  execlist->execpline->execcmd->execwhile->execlist->execpline
 *
 * (when waiting for the grep, ignoring execpline2 for now). At this time,
 * zsh has build two job-table entries for it: one for the cat and one for
 * the grep. If the user hits ^Z at this point (and jobbing is used), the 
 * shell is notified that the grep was suspended. The list_pipe flag is
 * used to tell the execpline where it was waiting that it was in a pipeline
 * with a shell construct at the end (which may also be a shell function or
 * several other things). When zsh sees the suspended grep, it forks to let
 * the sub-shell execute the rest of the while loop. The parent shell walks
 * up in the function call stack to the first execpline. There it has to find
 * out that it has just forked and then has to add information about the sub-
 * shell (its pid and the text for it) in the job entry of the cat. The pid
 * is passed down in the list_pipe_pid variable.
 * But there is a problem: the suspended grep is a child of the parent shell
 * and can't be adopted by the sub-shell. So the parent shell also has to 
 * keep the information about this process (more precisely: this pipeline)
 * by keeping the job table entry it created for it. The fact that there
 * are two jobs which have to be treated together is remembered by setting
 * the STAT_SUPERJOB flag in the entry for the cat-job (which now also
 * contains a process-entry for the whole loop -- the sub-shell) and by
 * setting STAT_SUBJOB in the job of the grep-job. With that we can keep
 * sub-jobs from being displayed and we can handle an fg/bg on the super-
 * job correctly. When the super-job is continued, the shell also wakes up
 * the sub-job. But then, the grep will exit sometime. Now the parent shell
 * has to remember not to try to wake it up again (in case of another ^Z).
 * It also has to wake up the sub-shell (which suspended itself immediately
 * after creation), so that the rest of the loop is executed by it.
 * But there is more: when the sub-shell is created, the cat may already
 * have exited, so we can't put the sub-shell in the process group of it.
 * In this case, we put the sub-shell in the process group of the parent
 * shell and in any case, the sub-shell has to put all commands executed
 * by it into its own process group, because only this way the parent
 * shell can control them since it only knows the process group of the sub-
 * shell. Of course, this information is also important when putting a job
 * in the foreground, where we have to attach its process group to the
 * controlling tty.
 * All this is made more difficult because we have to handle return values
 * correctly. If the grep is signaled, its exit status has to be propagated
 * back to the parent shell which needs it to set the exit status of the
 * super-job. And of course, when the grep is signaled (including ^C), the
 * loop has to be stopped, etc.
 * The code for all this is distributed over three files (exec.c, jobs.c,
 * and signals.c) and none of them is a simple one. So, all in all, there
 * may still be bugs, but considering the complexity (with race conditions,
 * signal handling, and all that), this should probably be expected.
 */

/**/
int list_pipe = 0, simple_pline = 0;

static pid_t list_pipe_pid;
static int nowait, pline_level = 0;
static int list_pipe_child = 0, list_pipe_job;
static char list_pipe_text[JOBTEXTSIZE];

/* execute a current shell command */

/**/
static int
execcursh(Estate state, int do_exec)
{
    Wordcode end = state->pc + WC_CURSH_SKIP(state->pc[-1]);

    if (!list_pipe && thisjob != list_pipe_job)
	deletejob(jobtab + thisjob);
    cmdpush(CS_CURSH);
    execlist(state, 1, do_exec);
    cmdpop();

    state->pc = end;

    return lastval;
}

/* execve after handling $_ and #! */

#define POUNDBANGLIMIT 64

/**/
static int
zexecve(char *pth, char **argv)
{
    int eno;
    static char buf[PATH_MAX * 2];
    char **eep;

    unmetafy(pth, NULL);
    for (eep = argv; *eep; eep++)
	if (*eep != pth)
	    unmetafy(*eep, NULL);
    for (eep = environ; *eep; eep++)
	if (**eep == '_' && (*eep)[1] == '=')
	    break;
    buf[0] = '_';
    buf[1] = '=';
    if (*pth == '/')
	strcpy(buf + 2, pth);
    else
	sprintf(buf + 2, "%s/%s", pwd, pth);
    if (!*eep)
	eep[1] = NULL;
    *eep = buf;
    closedumps();
    execve(pth, argv, environ);

    /* If the execve returns (which in general shouldn't happen),   *
     * then check for an errno equal to ENOEXEC.  This errno is set *
     * if the process file has the appropriate access permission,   *
     * but has an invalid magic number in its header.               */
    if ((eno = errno) == ENOEXEC) {
	char execvebuf[POUNDBANGLIMIT + 1], *ptr, *ptr2, *argv0;
	int fd, ct, t0;

	if ((fd = open(pth, O_RDONLY|O_NOCTTY)) >= 0) {
	    argv0 = *argv;
	    *argv = pth;
	    ct = read(fd, execvebuf, POUNDBANGLIMIT);
	    close(fd);
	    if (ct > 0) {
		if (execvebuf[0] == '#') {
		    if (execvebuf[1] == '!') {
			for (t0 = 0; t0 != ct; t0++)
			    if (execvebuf[t0] == '\n')
				break;
			while (inblank(execvebuf[t0]))
			    execvebuf[t0--] = '\0';
			execvebuf[POUNDBANGLIMIT] = '\0';
			for (ptr = execvebuf + 2; *ptr && *ptr == ' '; ptr++);
			for (ptr2 = ptr; *ptr && *ptr != ' '; ptr++);
			if (*ptr) {
			    *ptr = '\0';
			    argv[-2] = ptr2;
			    argv[-1] = ptr + 1;
			    execve(ptr2, argv - 2, environ);
			} else {
			    argv[-1] = ptr2;
			    execve(ptr2, argv - 1, environ);
			}
		    } else {
			argv[-1] = "sh";
			execve("/bin/sh", argv - 1, environ);
		    }
		} else {
		    for (t0 = 0; t0 != ct; t0++)
			if (!execvebuf[t0])
			    break;
		    if (t0 == ct) {
			argv[-1] = "sh";
			execve("/bin/sh", argv - 1, environ);
		    }
		}
	    } else
		eno = errno;
	    *argv = argv0;
	} else
	    eno = errno;
    }
    /* restore the original arguments and path but do not bother with *
     * null characters as these cannot be passed to external commands *
     * anyway.  So the result is truncated at the first null char.    */
    pth = metafy(pth, -1, META_NOALLOC);
    for (eep = argv; *eep; eep++)
	if (*eep != pth)
	    (void) metafy(*eep, -1, META_NOALLOC);
    return eno;
}

#define MAXCMDLEN (PATH_MAX*4)

/* test whether we really want to believe the error number */

/**/
static int
isgooderr(int e, char *dir)
{
    /*
     * Maybe the directory was unreadable, or maybe it wasn't
     * even a directory. 
     */
    return ((e != EACCES || !access(dir, X_OK)) &&
	    e != ENOENT && e != ENOTDIR); 
}

/* execute an external command */

/**/
void
execute(Cmdnam not_used_yet, int dash)
{
    Cmdnam cn;
    char buf[MAXCMDLEN], buf2[MAXCMDLEN];
    char *s, *z, *arg0;
    char **argv, **pp;
    int eno = 0, ee;

    arg0 = (char *) peekfirst(args);
    if (isset(RESTRICTED) && strchr(arg0, '/')) {
	zerr("%s: restricted", arg0, 0);
	_exit(1);
    }

    /* If the parameter STTY is set in the command's environment, *
     * we first run the stty command with the value of this       *
     * parameter as it arguments.                                 */
    if ((s = STTYval) && isatty(0) && (GETPGRP() == getpid())) {
	LinkList exargs = args;
	char *t = tricat("stty", " ", s);

	STTYval = 0;	/* this prevents infinite recursion */
	zsfree(s);
	args = NULL;
	execstring(t, 1, 0);
	zsfree(t);
	args = exargs;
    } else if (s) {
	STTYval = 0;
	zsfree(s);
    }

    cn = (Cmdnam) cmdnamtab->getnode(cmdnamtab, arg0);

    /* If ARGV0 is in the commands environment, we use *
     * that as argv[0] for this external command       */
    if (unset(RESTRICTED) && (z = zgetenv("ARGV0"))) {
	setdata(firstnode(args), (void *) ztrdup(z));
	delenv(z - 6);
    } else if (dash) {
    /* Else if the pre-command `-' was given, we add `-' *
     * to the front of argv[0] for this command.         */
	sprintf(buf2, "-%s", arg0);
	setdata(firstnode(args), (void *) ztrdup(buf2));
    }

    argv = makecline(args);
    closem(3);
    child_unblock();
    if ((int) strlen(arg0) >= PATH_MAX) {
	zerr("command too long: %s", arg0, 0);
	_exit(1);
    }
    for (s = arg0; *s; s++)
	if (*s == '/') {
	    errno = zexecve(arg0, argv);
	    if (arg0 == s || unset(PATHDIRS) ||
		(arg0[0] == '.' && (arg0 + 1 == s ||
				    (arg0[1] == '.' && arg0 + 2 == s)))) {
		zerr("%e: %s", arg0, errno);
		_exit(1);
	    }
	    break;
	}

    if (cn) {
	char nn[PATH_MAX], *dptr;

	if (cn->flags & HASHED)
	    strcpy(nn, cn->u.cmd);
	else {
	    for (pp = path; pp < cn->u.name; pp++)
		if (!**pp || (**pp == '.' && (*pp)[1] == '\0')) {
		    ee = zexecve(arg0, argv);
		    if (isgooderr(ee, *pp))
			eno = ee;
		} else if (**pp != '/') {
		    z = buf;
		    strucpy(&z, *pp);
		    *z++ = '/';
		    strcpy(z, arg0);
		    ee = zexecve(buf, argv);
		    if (isgooderr(ee, *pp))
			eno = ee;
		}
	    strcpy(nn, cn->u.name ? *(cn->u.name) : "");
	    strcat(nn, "/");
	    strcat(nn, cn->nam);
	}
	ee = zexecve(nn, argv);

	if ((dptr = strrchr(nn, '/')))
	    *dptr = '\0';
	if (isgooderr(ee, *nn ? nn : "/"))
	    eno = ee;
    }
    for (pp = path; *pp; pp++)
	if (!(*pp)[0] || ((*pp)[0] == '.' && !(*pp)[1])) {
	    ee = zexecve(arg0, argv);
	    if (isgooderr(ee, *pp))
		eno = ee;
	} else {
	    z = buf;
	    strucpy(&z, *pp);
	    *z++ = '/';
	    strcpy(z, arg0);
	    ee = zexecve(buf, argv);
	    if (isgooderr(ee, *pp))
		eno = ee;
	}
    if (eno)
	zerr("%e: %s", arg0, eno);
    else
	zerr("command not found: %s", arg0, 0);
    _exit(1);
}

#define RET_IF_COM(X) { if (iscom(X)) return docopy ? dupstring(X) : arg0; }

/*
 * Get the full pathname of an external command.
 * If the second argument is zero, return the first argument if found;
 * if non-zero, return the path using heap memory.  (RET_IF_COM(X), above).
 */

/**/
mod_export char *
findcmd(char *arg0, int docopy)
{
    char **pp;
    char *z, *s, buf[MAXCMDLEN];
    Cmdnam cn;

    cn = (Cmdnam) cmdnamtab->getnode(cmdnamtab, arg0);
    if (!cn && isset(HASHCMDS))
	cn = hashcmd(arg0, path);
    if ((int) strlen(arg0) > PATH_MAX)
	return NULL;
    for (s = arg0; *s; s++)
	if (*s == '/') {
	    RET_IF_COM(arg0);
	    if (arg0 == s || unset(PATHDIRS)) {
		return NULL;
	    }
	    break;
	}
    if (cn) {
	char nn[PATH_MAX];

	if (cn->flags & HASHED)
	    strcpy(nn, cn->u.cmd);
	else {
	    for (pp = path; pp < cn->u.name; pp++)
		if (**pp != '/') {
		    z = buf;
		    if (**pp) {
			strucpy(&z, *pp);
			*z++ = '/';
		    }
		    strcpy(z, arg0);
		    RET_IF_COM(buf);
		}
	    strcpy(nn, cn->u.name ? *(cn->u.name) : "");
	    strcat(nn, "/");
	    strcat(nn, cn->nam);
	}
	RET_IF_COM(nn);
    }
    for (pp = path; *pp; pp++) {
	z = buf;
	if (**pp) {
	    strucpy(&z, *pp);
	    *z++ = '/';
	}
	strcpy(z, arg0);
	RET_IF_COM(buf);
    }
    return NULL;
}

/**/
int
iscom(char *s)
{
    struct stat statbuf;
    char *us = unmeta(s);

    return (access(us, X_OK) == 0 && stat(us, &statbuf) >= 0 &&
	    S_ISREG(statbuf.st_mode));
}

/**/
int
isreallycom(Cmdnam cn)
{
    char fullnam[MAXCMDLEN];

    if (cn->flags & HASHED)
	strcpy(fullnam, cn->u.cmd);
    else if (!cn->u.name)
	return 0;
    else {
	strcpy(fullnam, *(cn->u.name));
	strcat(fullnam, "/");
	strcat(fullnam, cn->nam);
    }
    return iscom(fullnam);
}

/**/
int
isrelative(char *s)
{
    if (*s != '/')
	return 1;
    for (; *s; s++)
	if (*s == '.' && s[-1] == '/' &&
	    (s[1] == '/' || s[1] == '\0' ||
	     (s[1] == '.' && (s[2] == '/' || s[2] == '\0'))))
	    return 1;
    return 0;
}

/**/
mod_export Cmdnam
hashcmd(char *arg0, char **pp)
{
    Cmdnam cn;
    char *s, buf[PATH_MAX];
    char **pq;

    for (; *pp; pp++)
	if (**pp == '/') {
	    s = buf;
	    strucpy(&s, *pp);
	    *s++ = '/';
	    if ((s - buf) + strlen(arg0) >= PATH_MAX)
		continue;
	    strcpy(s, arg0);
	    if (iscom(buf))
		break;
	}

    if (!*pp)
	return NULL;

    cn = (Cmdnam) zcalloc(sizeof *cn);
    cn->flags = 0;
    cn->u.name = pp;
    cmdnamtab->addnode(cmdnamtab, ztrdup(arg0), cn);

    if (isset(HASHDIRS)) {
	for (pq = pathchecked; pq <= pp; pq++)
	    hashdir(pq);
	pathchecked = pp + 1;
    }

    return cn;
}

/* execute a string */

/**/
mod_export void
execstring(char *s, int dont_change_job, int exiting)
{
    Eprog prog;

    pushheap();
    if ((prog = parse_string(s, 0)))
	execode(prog, dont_change_job, exiting);
    popheap();
}

/**/
mod_export void
execode(Eprog p, int dont_change_job, int exiting)
{
    struct estate s;

    s.prog = p;
    s.pc = p->prog;
    s.strs = p->strs;

    execlist(&s, dont_change_job, exiting);
}

/* Execute a simplified command. This is used to execute things that
 * will run completely in the shell, so that we can by-pass all that
 * nasty job-handling and redirection stuff in execpline and execcmd. */

/**/
static int
execsimple(Estate state)
{
    wordcode code = *state->pc++;
    int lv;

    if (errflag)
	return (lastval = 1);

    if (code)
	lineno = code - 1;

    code = wc_code(*state->pc++);

    if (code == WC_ASSIGN) {
	cmdoutval = 0;
	addvars(state, state->pc - 1, 0);
	if (isset(XTRACE)) {
	    fputc('\n', xtrerr);
	    fflush(xtrerr);
	}
	lv = (errflag ? errflag : cmdoutval);
    } else
	lv = (execfuncs[code - WC_CURSH])(state, 0);

    return lastval = lv;
}

/* Main routine for executing a list.                                *
 * exiting means that the (sub)shell we are in is a definite goner   *
 * after the current list is finished, so we may be able to exec the *
 * last command directly instead of forking.  If dont_change_job is  *
 * nonzero, then restore the current job number after executing the  *
 * list.                                                             */

/**/
void
execlist(Estate state, int dont_change_job, int exiting)
{
    static int donetrap;
    Wordcode next;
    wordcode code;
    int ret, cj, csp, ltype;
    int old_pline_level, old_list_pipe, oldlineno;
    /*
     * ERREXIT only forces the shell to exit if the last command in a &&
     * or || fails.  This is the case even if an earlier command is a
     * shell function or other current shell structure, so we have to set
     * noerrexit here if the sublist is not of type END.
     */
    int oldnoerrexit = noerrexit;

    cj = thisjob;
    old_pline_level = pline_level;
    old_list_pipe = list_pipe;
    oldlineno = lineno;

    if (sourcelevel && unset(SHINSTDIN))
	pline_level = list_pipe = 0;

    /* Loop over all sets of comands separated by newline, *
     * semi-colon or ampersand (`sublists').               */
    code = *state->pc++;
    while (wc_code(code) == WC_LIST && !breaks && !retflag) {
	ltype = WC_LIST_TYPE(code);
	csp = cmdsp;

	if (ltype & Z_SIMPLE) {
	    next = state->pc + WC_LIST_SKIP(code);
	    execsimple(state);
	    state->pc = next;
	    goto sublist_done;
	}
	/* Reset donetrap:  this ensures that a trap is only *
	 * called once for each sublist that fails.          */
	donetrap = 0;

	/* Loop through code followed by &&, ||, or end of sublist. */
	code = *state->pc++;
	while (wc_code(code) == WC_SUBLIST) {
	    next = state->pc + WC_SUBLIST_SKIP(code);
	    if (!oldnoerrexit)
		noerrexit = (WC_SUBLIST_TYPE(code) != WC_SUBLIST_END);
	    switch (WC_SUBLIST_TYPE(code)) {
	    case WC_SUBLIST_END:
		/* End of sublist; just execute, ignoring status. */
		if (WC_SUBLIST_FLAGS(code) & WC_SUBLIST_SIMPLE)
		    execsimple(state);
		else
		    execpline(state, code, ltype, (ltype & Z_END) && exiting);
		state->pc = next;
		goto sublist_done;
		break;
	    case WC_SUBLIST_AND:
		/* If the return code is non-zero, we skip pipelines until *
		 * we find a sublist followed by ORNEXT.                   */
		if ((ret = ((WC_SUBLIST_FLAGS(code) & WC_SUBLIST_SIMPLE) ?
			    execsimple(state) :
			    execpline(state, code, Z_SYNC, 0)))) {
		    state->pc = next;
		    code = *state->pc++;
		    next = state->pc + WC_SUBLIST_SKIP(code);
		    while (wc_code(code) == WC_SUBLIST &&
			   WC_SUBLIST_TYPE(code) == WC_SUBLIST_AND) {
			state->pc = next;
			code = *state->pc++;
			next = state->pc + WC_SUBLIST_SKIP(code);
		    }
		    if (wc_code(code) != WC_SUBLIST) {
			/* We've skipped to the end of the list, not executing *
			 * the final pipeline, so don't perform error handling *
			 * for this sublist.                                   */
			donetrap = 1;
			goto sublist_done;
		    } else if (WC_SUBLIST_TYPE(code) == WC_SUBLIST_END)
			donetrap = 1;
		}
		cmdpush(CS_CMDAND);
		break;
	    case WC_SUBLIST_OR:
		/* If the return code is zero, we skip pipelines until *
		 * we find a sublist followed by ANDNEXT.              */
		if (!(ret = ((WC_SUBLIST_FLAGS(code) & WC_SUBLIST_SIMPLE) ?
			     execsimple(state) :
			     execpline(state, code, Z_SYNC, 0)))) {
		    state->pc = next;
		    code = *state->pc++;
		    next = state->pc + WC_SUBLIST_SKIP(code);
		    while (wc_code(code) == WC_SUBLIST &&
			   WC_SUBLIST_TYPE(code) == WC_SUBLIST_OR) {
			state->pc = next;
			code = *state->pc++;
			next = state->pc + WC_SUBLIST_SKIP(code);
		    }
		    if (wc_code(code) != WC_SUBLIST) {
			/* We've skipped to the end of the list, not executing *
			 * the final pipeline, so don't perform error handling *
			 * for this sublist.                                   */
			donetrap = 1;
			goto sublist_done;
		    } else if (WC_SUBLIST_TYPE(code) == WC_SUBLIST_END)
			donetrap = 1;
		}
		cmdpush(CS_CMDOR);
		break;
	    }
	    state->pc = next;
	    code = *state->pc++;
	}
	state->pc--;
sublist_done:

	noerrexit = oldnoerrexit;

	if (sigtrapped[SIGDEBUG]) {
	    exiting = donetrap;
	    ret = lastval;
	    dotrap(SIGDEBUG);
	    lastval = ret;
	    donetrap = exiting;
	    noerrexit = oldnoerrexit;
	}

	cmdsp = csp;

	/* Check whether we are suppressing traps/errexit *
	 * (typically in init scripts) and if we haven't  *
	 * already performed them for this sublist.       */
	if (!noerrexit && !donetrap) {
	    if (sigtrapped[SIGZERR] && lastval) {
		dotrap(SIGZERR);
		donetrap = 1;
	    }
	    if (lastval && isset(ERREXIT)) {
		if (sigtrapped[SIGEXIT])
		    dotrap(SIGEXIT);
		if (mypid != getpid())
		    _exit(lastval);
		else
		    exit(lastval);
	    }
	}
	if (ltype & Z_END)
	    break;
	code = *state->pc++;
    }
    pline_level = old_pline_level;
    list_pipe = old_list_pipe;
    lineno = oldlineno;
    if (dont_change_job)
	thisjob = cj;
}

/* Execute a pipeline.                                                *
 * last1 is a flag that this command is the last command in a shell   *
 * that is about to exit, so we can exec instead of forking.  It gets *
 * passed all the way down to execcmd() which actually makes the      *
 * decision.  A 0 is always passed if the command is not the last in  *
 * the pipeline.  This function assumes that the sublist is not NULL. *
 * If last1 is zero but the command is at the end of a pipeline, we   *
 * pass 2 down to execcmd().                                          *
 */

/**/
static int
execpline(Estate state, wordcode slcode, int how, int last1)
{
    int ipipe[2], opipe[2];
    int pj, newjob;
    int old_simple_pline = simple_pline;
    int slflags = WC_SUBLIST_FLAGS(slcode);
    wordcode code = *state->pc++;
    static int lastwj, lpforked;

    if (wc_code(code) != WC_PIPE)
	return lastval = (slflags & WC_SUBLIST_NOT) != 0;
    else if (slflags & WC_SUBLIST_NOT)
	last1 = 0;

    pj = thisjob;
    ipipe[0] = ipipe[1] = opipe[0] = opipe[1] = 0;
    child_block();

    /* get free entry in job table and initialize it */
    if ((thisjob = newjob = initjob()) == -1) {
	child_unblock();
	return 1;
    }
    if (how & Z_TIMED)
	jobtab[thisjob].stat |= STAT_TIMED;

    if (slflags & WC_SUBLIST_COPROC) {
	how = Z_ASYNC;
	if (coprocin >= 0) {
	    zclose(coprocin);
	    zclose(coprocout);
	}
	mpipe(ipipe);
	mpipe(opipe);
	coprocin = ipipe[0];
	coprocout = opipe[1];
	fdtable[coprocin] = fdtable[coprocout] = 0;
    }
    /* This used to set list_pipe_pid=0 unconditionally, but in things
     * like `ls|if true; then sleep 20; cat; fi' where the sleep was
     * stopped, the top-level execpline() didn't get the pid for the
     * sub-shell because it was overwritten. */
    if (!pline_level++) {
        list_pipe_pid = 0;
	nowait = 0;
	simple_pline = (WC_PIPE_TYPE(code) == WC_PIPE_END);
	list_pipe_job = newjob;
    }
    lastwj = lpforked = 0;
    execpline2(state, code, how, opipe[0], ipipe[1], last1);
    pline_level--;
    if (how & Z_ASYNC) {
	lastwj = newjob;

        if (thisjob == list_pipe_job)
            list_pipe_job = 0;
	jobtab[thisjob].stat |= STAT_NOSTTY;
	if (slflags & WC_SUBLIST_COPROC) {
	    zclose(ipipe[1]);
	    zclose(opipe[0]);
	}
	if (how & Z_DISOWN) {
	    deletejob(jobtab + thisjob);
	    thisjob = -1;
	}
	else
	    spawnjob();
	child_unblock();
	return 0;
    } else {
	if (newjob != lastwj) {
	    Job jn = jobtab + newjob;
	    int updated;

	    if (newjob == list_pipe_job && list_pipe_child)
		_exit(0);

	    lastwj = thisjob = newjob;

	    if (list_pipe || (pline_level && !(how & Z_TIMED)))
		jn->stat |= STAT_NOPRINT;

	    if (nowait) {
		if(!pline_level) {
		    struct process *pn, *qn;

		    curjob = newjob;
		    DPUTS(!list_pipe_pid, "invalid list_pipe_pid");
		    addproc(list_pipe_pid, list_pipe_text);

		    /* If the super-job contains only the sub-shell, the
		       sub-shell is the group leader. */
		    if (!jn->procs->next || lpforked == 2) {
			jn->gleader = list_pipe_pid;
			jn->stat |= STAT_SUBLEADER;
		    }
		    for (pn = jobtab[jn->other].procs; pn; pn = pn->next)
			if (WIFSTOPPED(pn->status))
			    break;

		    if (pn) {
			for (qn = jn->procs; qn->next; qn = qn->next);
			qn->status = pn->status;
		    }

		    jn->stat &= ~(STAT_DONE | STAT_NOPRINT);
		    jn->stat |= STAT_STOPPED | STAT_CHANGED | STAT_LOCKED;
		    printjob(jn, !!isset(LONGLISTJOBS), 1);
		}
		else if (newjob != list_pipe_job)
		    deletejob(jn);
		else
		    lastwj = -1;
	    }

	    errbrk_saved = 0;
	    for (; !nowait;) {
		if (list_pipe_child) {
		    jn->stat |= STAT_NOPRINT;
		    makerunning(jn);
		}
		if (!(jn->stat & STAT_LOCKED)) {
		    updated = !!jobtab[thisjob].procs;
		    waitjobs();
		    child_block();
		} else
		    updated = 0;
		if (!updated &&
		    list_pipe_job && jobtab[list_pipe_job].procs &&
		    !(jobtab[list_pipe_job].stat & STAT_STOPPED)) {
		    child_unblock();
		    child_block();
		}
		if (list_pipe_child &&
		    jn->stat & STAT_DONE &&
		    lastval2 & 0200)
		    killpg(mypgrp, lastval2 & ~0200);
		if (!list_pipe_child && !lpforked && !subsh && jobbing &&
		    (list_pipe || last1 || pline_level) &&
		    ((jn->stat & STAT_STOPPED) ||
		     (list_pipe_job && pline_level &&
		      (jobtab[list_pipe_job].stat & STAT_STOPPED)))) {
		    pid_t pid;
		    int synch[2];

		    pipe(synch);

		    if ((pid = fork()) == -1) {
			trashzle();
			close(synch[0]);
			close(synch[1]);
			putc('\n', stderr);
			fprintf(stderr, "zsh: job can't be suspended\n");
			fflush(stderr);
			makerunning(jn);
			killjb(jn, SIGCONT);
			thisjob = newjob;
		    }
		    else if (pid) {
			char dummy;

			lpforked = 
			    (killpg(jobtab[list_pipe_job].gleader, 0) == -1 ? 2 : 1);
			list_pipe_pid = pid;
			nowait = errflag = 1;
			breaks = loops;
			close(synch[1]);
			read(synch[0], &dummy, 1);
			close(synch[0]);
			/* If this job has finished, we leave it as a
			 * normal (non-super-) job. */
			if (!(jn->stat & STAT_DONE)) {
			    jobtab[list_pipe_job].other = newjob;
			    jobtab[list_pipe_job].stat |= STAT_SUPERJOB;
			    jn->stat |= STAT_SUBJOB | STAT_NOPRINT;
			    jn->other = pid;
			}
			if ((list_pipe || last1) && jobtab[list_pipe_job].procs)
			    killpg(jobtab[list_pipe_job].gleader, SIGSTOP);
			break;
		    }
		    else {
			close(synch[0]);
			entersubsh(Z_ASYNC, 0, 0);
			if (jobtab[list_pipe_job].procs) {
			    if (setpgrp(0L, mypgrp = jobtab[list_pipe_job].gleader)
				== -1) {
				setpgrp(0L, mypgrp = getpid());
			    }
			} else
			    setpgrp(0L, mypgrp = getpid());
			close(synch[1]);
			kill(getpid(), SIGSTOP);
			list_pipe = 0;
			list_pipe_child = 1;
			opts[INTERACTIVE] = 0;
			if (errbrk_saved) {
			    errflag = prev_errflag;
			    breaks = prev_breaks;
			}
			break;
		    }
		}
		else if (subsh && jn->stat & STAT_STOPPED)
		    thisjob = newjob;
		else
		    break;
	    }
	    child_unblock();

	    if (list_pipe && (lastval & 0200) && pj >= 0 &&
		(!(jn->stat & STAT_INUSE) || (jn->stat & STAT_DONE))) {
		deletejob(jn);
		jn = jobtab + pj;
		killjb(jn, lastval & ~0200);
	    }
	    if (list_pipe_child ||
		((jn->stat & STAT_DONE) &&
		 (list_pipe || (pline_level && !(jn->stat & STAT_SUBJOB)))))
		deletejob(jn);
	    thisjob = pj;

	}
	if (slflags & WC_SUBLIST_NOT)
	    lastval = !lastval;
    }
    if (!pline_level)
	simple_pline = old_simple_pline;
    return lastval;
}

static int subsh_close = -1;

/* execute pipeline.  This function assumes the `pline' is not NULL. */

/**/
static void
execpline2(Estate state, wordcode pcode,
	   int how, int input, int output, int last1)
{
    pid_t pid;
    int pipes[2];

    if (breaks || retflag)
	return;

    if (WC_PIPE_LINENO(pcode))
	lineno = WC_PIPE_LINENO(pcode) - 1;

    if (pline_level == 1) {
	if ((how & Z_ASYNC) || (!sfcontext && !sourcelevel))
	    strcpy(list_pipe_text,
		   getjobtext(state->prog,
			      state->pc + (WC_PIPE_TYPE(pcode) == WC_PIPE_END ?
					   0 : 1)));
	else
	    list_pipe_text[0] = '\0';
    }
    if (WC_PIPE_TYPE(pcode) == WC_PIPE_END)
	execcmd(state, input, output, how, last1 ? 1 : 2);
    else {
	int old_list_pipe = list_pipe;
	Wordcode next = state->pc + (*state->pc), pc;
	wordcode code;

	state->pc++;
	for (pc = state->pc; wc_code(code = *pc) == WC_REDIR; pc += 3);

	mpipe(pipes);

	/* if we are doing "foo | bar" where foo is a current *
	 * shell command, do foo in a subshell and do the     *
	 * rest of the pipeline in the current shell.         */
	if (wc_code(code) >= WC_CURSH && (how & Z_SYNC)) {
	    int synch[2];

	    pipe(synch);
	    if ((pid = fork()) == -1) {
		close(synch[0]);
		close(synch[1]);
		zerr("fork failed: %e", NULL, errno);
	    } else if (pid) {
		char dummy, *text;

		text = getjobtext(state->prog, state->pc);
		addproc(pid, text);
		close(synch[1]);
		read(synch[0], &dummy, 1);
		close(synch[0]);
	    } else {
		zclose(pipes[0]);
		close(synch[0]);
		entersubsh(how, 2, 0);
		close(synch[1]);
		execcmd(state, input, pipes[1], how, 0);
		_exit(lastval);
	    }
	} else {
	    /* otherwise just do the pipeline normally. */
	    subsh_close = pipes[0];
	    execcmd(state, input, pipes[1], how, 0);
	}
	zclose(pipes[1]);
	state->pc = next;

	/* if another execpline() is invoked because the command is *
	 * a list it must know that we're already in a pipeline     */
	cmdpush(CS_PIPE);
	list_pipe = 1;
	execpline2(state, *state->pc++, how, pipes[0], output, last1);
	list_pipe = old_list_pipe;
	cmdpop();
	zclose(pipes[0]);
	subsh_close = -1;
    }
}

/* make the argv array */

/**/
static char **
makecline(LinkList list)
{
    LinkNode node;
    char **argv, **ptr;

    /* A bigger argv is necessary for executing scripts */
    ptr = argv = 2 + (char **) hcalloc((countlinknodes(list) + 4) *
				       sizeof(char *));

    if (isset(XTRACE)) {
	if (!doneps4)
	    printprompt4();

	for (node = firstnode(list); node; incnode(node)) {
	    *ptr++ = (char *)getdata(node);
	    zputs(getdata(node), xtrerr);
	    if (nextnode(node))
		fputc(' ', xtrerr);
	}
	fputc('\n', xtrerr);
	fflush(xtrerr);
    } else {
	for (node = firstnode(list); node; incnode(node))
	    *ptr++ = (char *)getdata(node);
    }
    *ptr = NULL;
    return (argv);
}

/**/
mod_export void
untokenize(char *s)
{
    if (*s) {
	int c;

	while ((c = *s++))
	    if (itok(c)) {
		char *p = s - 1;

		if (c != Nularg)
		    *p++ = ztokens[c - Pound];

		while ((c = *s++)) {
		    if (itok(c)) {
			if (c != Nularg)
			    *p++ = ztokens[c - Pound];
		    } else
			*p++ = c;
		}
		*p = '\0';
		break;
	    }
    }
}

/* Open a file for writing redirection */

/**/
static int
clobber_open(struct redir *f)
{
    struct stat buf;
    int fd, oerrno;

    /* If clobbering, just open. */
    if (isset(CLOBBER) || IS_CLOBBER_REDIR(f->type))
	return open(unmeta(f->name),
		O_WRONLY | O_CREAT | O_TRUNC | O_NOCTTY, 0666);

    /* If not clobbering, attempt to create file exclusively. */
    if ((fd = open(unmeta(f->name),
		   O_WRONLY | O_CREAT | O_EXCL | O_NOCTTY, 0666)) >= 0)
	return fd;

    /* If that fails, we are still allowed to open non-regular files. *
     * Try opening, and if it's a regular file then close it again    *
     * because we weren't supposed to open it.                        */
    oerrno = errno;
    if ((fd = open(unmeta(f->name), O_WRONLY | O_NOCTTY)) != -1) {
	if(!fstat(fd, &buf) && !S_ISREG(buf.st_mode))
	    return fd;
	close(fd);
    }
    errno = oerrno;
    return -1;
}

/* size of buffer for tee and cat processes */
#define TCBUFSIZE 4092

/* close an multio (success) */

/**/
static void
closemn(struct multio **mfds, int fd)
{
    if (fd >= 0 && mfds[fd] && mfds[fd]->ct >= 2) {
	struct multio *mn = mfds[fd];
	char buf[TCBUFSIZE];
	int len, i;

	if (zfork()) {
	    for (i = 0; i < mn->ct; i++)
		zclose(mn->fds[i]);
	    zclose(mn->pipe);
	    mn->ct = 1;
	    mn->fds[0] = fd;
	    return;
	}
	/* pid == 0 */
	closeallelse(mn);
	if (mn->rflag) {
	    /* tee process */
	    while ((len = read(mn->pipe, buf, TCBUFSIZE)) != 0) {
		if (len < 0) {
		    if (errno == EINTR)
			continue;
		    else
			break;
		}
		for (i = 0; i < mn->ct; i++)
		    write(mn->fds[i], buf, len);
	    }
	} else {
	    /* cat process */
	    for (i = 0; i < mn->ct; i++)
		while ((len = read(mn->fds[i], buf, TCBUFSIZE)) != 0) {
		    if (len < 0) {
			if (errno == EINTR)
			    continue;
			else
			    break;
		    }
		    write(mn->pipe, buf, len);
		}
	}
	_exit(0);
    }
}

/* close all the mnodes (failure) */

/**/
static void
closemnodes(struct multio **mfds)
{
    int i, j;

    for (i = 0; i < 10; i++)
	if (mfds[i]) {
	    for (j = 0; j < mfds[i]->ct; j++)
		zclose(mfds[i]->fds[j]);
	    mfds[i] = NULL;
	}
}

/**/
static void
closeallelse(struct multio *mn)
{
    int i, j;
    long openmax;

    openmax = zopenmax();

    for (i = 0; i < openmax; i++)
	if (mn->pipe != i) {
	    for (j = 0; j < mn->ct; j++)
		if (mn->fds[j] == i)
		    break;
	    if (j == mn->ct)
		zclose(i);
	}
}

/* A multio is a list of fds associated with a certain fd.       *
 * Thus if you do "foo >bar >ble", the multio for fd 1 will have *
 * two fds, the result of open("bar",...), and the result of     *
 * open("ble",....).                                             */

/* Add a fd to an multio.  fd1 must be < 10, and may be in any state. *
 * fd2 must be open, and is `consumed' by this function.  Note that   *
 * fd1 == fd2 is possible, and indicates that fd1 was really closed.  *
 * We effectively do `fd2 = movefd(fd2)' at the beginning of this     *
 * function, but in most cases we can avoid an extra dup by delaying  *
 * the movefd: we only >need< to move it if we're actually doing a    *
 * multiple redirection.                                              */

/**/
static void
addfd(int forked, int *save, struct multio **mfds, int fd1, int fd2, int rflag)
{
    int pipes[2];

    if (!mfds[fd1] || unset(MULTIOS)) {
	if(!mfds[fd1]) {		/* starting a new multio */
	    mfds[fd1] = (struct multio *) zhalloc(sizeof(struct multio));
	    if (!forked && save[fd1] == -2)
		save[fd1] = (fd1 == fd2) ? -1 : movefd(fd1);
	}
	redup(fd2, fd1);
	mfds[fd1]->ct = 1;
	mfds[fd1]->fds[0] = fd1;
	mfds[fd1]->rflag = rflag;
    } else {
	if (mfds[fd1]->rflag != rflag) {
	    zerr("file mode mismatch on fd %d", NULL, fd1);
	    return;
	}
	if (mfds[fd1]->ct == 1) {	/* split the stream */
	    mfds[fd1]->fds[0] = movefd(fd1);
	    mfds[fd1]->fds[1] = movefd(fd2);
	    mpipe(pipes);
	    mfds[fd1]->pipe = pipes[1 - rflag];
	    redup(pipes[rflag], fd1);
	    mfds[fd1]->ct = 2;
	} else {		/* add another fd to an already split stream */
	    if(!(mfds[fd1]->ct % MULTIOUNIT)) {
		int new = sizeof(struct multio) + sizeof(int) * mfds[fd1]->ct;
		int old = new - sizeof(int) * MULTIOUNIT;
		mfds[fd1] = hrealloc((char *)mfds[fd1], old, new);
	    }
	    mfds[fd1]->fds[mfds[fd1]->ct++] = movefd(fd2);
	}
    }
    if (subsh_close >= 0 && !fdtable[subsh_close])
	subsh_close = -1;
}

/**/
static void
addvars(Estate state, Wordcode pc, int export)
{
    LinkList vl;
    int xtr, isstr, htok = 0;
    char **arr, **ptr, *name;
    Wordcode opc = state->pc;
    wordcode ac;
    local_list1(svl);

    xtr = isset(XTRACE);
    if (xtr) {
	printprompt4();
	doneps4 = 1;
    }
    state->pc = pc;
    while (wc_code(ac = *state->pc++) == WC_ASSIGN) {
	name = ecgetstr(state, EC_DUPTOK, &htok);
	if (htok)
	    untokenize(name);
	if (xtr)
	    fprintf(xtrerr, "%s=", name);
	if ((isstr = (WC_ASSIGN_TYPE(ac) == WC_ASSIGN_SCALAR))) {
	    init_list1(svl, ecgetstr(state, EC_DUPTOK, &htok));
	    vl = &svl;
	} else
	    vl = ecgetlist(state, WC_ASSIGN_NUM(ac), EC_DUPTOK, &htok);

	if (vl && htok) {
	    prefork(vl, (isstr ? (PF_SINGLE|PF_ASSIGN) :
			 PF_ASSIGN));
	    if (errflag) {
		state->pc = opc;
		return;
	    }
	    if (isset(GLOBASSIGN) || !isstr)
		globlist(vl, 0);
	    if (errflag) {
		state->pc = opc;
		return;
	    }
	}
	if (isstr && (empty(vl) || !nextnode(firstnode(vl)))) {
	    Param pm;
	    char *val;
	    int allexp;

	    if (empty(vl))
		val = ztrdup("");
	    else {
		untokenize(peekfirst(vl));
		val = ztrdup(ugetnode(vl));
	    }
	    if (xtr)
		fprintf(xtrerr, "%s ", val);
	    if (export && !strchr(name, '[')) {
		if (export < 0 && isset(RESTRICTED) &&
		    (pm = (Param) paramtab->removenode(paramtab, name)) &&
		    (pm->flags & PM_RESTRICTED)) {
		    zerr("%s: restricted", pm->nam, 0);
		    zsfree(val);
		    state->pc = opc;
		    return;
		}
		if (strcmp(name, "STTY") == 0) {
		    zsfree(STTYval);
		    STTYval = ztrdup(val);
		}
		allexp = opts[ALLEXPORT];
		opts[ALLEXPORT] = 1;
		pm = setsparam(name, val);
		opts[ALLEXPORT] = allexp;
	    } else
		pm = setsparam(name, val);
	    if (errflag) {
		state->pc = opc;
		return;
	    }
	    continue;
	}
	if (vl) {
	    ptr = arr = (char **) zalloc(sizeof(char **) *
					 (countlinknodes(vl) + 1));

	    while (nonempty(vl))
		*ptr++ = ztrdup((char *) ugetnode(vl));
	} else
	    ptr = arr = (char **) zalloc(sizeof(char **));

	*ptr = NULL;
	if (xtr) {
	    fprintf(xtrerr, "( ");
	    for (ptr = arr; *ptr; ptr++)
		fprintf(xtrerr, "%s ", *ptr);
	    fprintf(xtrerr, ") ");
	}
	setaparam(name, arr);
	if (errflag) {
	    state->pc = opc;
	    return;
	}
    }
    state->pc = opc;
}

/**/
void
setunderscore(char *str)
{
    if (str && *str) {
	int l = strlen(str) + 1, nl = (l + 31) & ~31;

	if (nl > underscorelen || (underscorelen - nl) > 64) {
	    zfree(underscore, underscorelen);
	    underscore = (char *) zalloc(underscorelen = nl);
	}
	strcpy(underscore, str);
	underscoreused = l;
    } else {
	if (underscorelen > 128) {
	    zfree(underscore, underscorelen);
	    underscore = (char *) zalloc(underscorelen = 32);
	}
	*underscore = '\0';
	underscoreused = 1;
    }
}

/* These describe the type of expansions that need to be done on the words
 * used in the thing we are about to execute. They are set in execcmd() and
 * used in execsubst() which might be called from one of the functions
 * called from execcmd() (like execfor() and so on). */

static int esprefork, esglob = 1;

/**/
void
execsubst(LinkList strs)
{
    if (strs) {
	prefork(strs, esprefork);
	if (esglob) {
	    LinkList ostrs = strs;
	    globlist(strs, 0);
	    strs = ostrs;
	}
    }
}

/**/
static void
execcmd(Estate state, int input, int output, int how, int last1)
{
    HashNode hn = NULL;
    LinkNode node;
    Redir fn;
    struct multio *mfds[10];
    char *text;
    int save[10];
    int fil, dfil, is_cursh, type, do_exec = 0, i, htok = 0;
    int nullexec = 0, assign = 0, forked = 0;
    int is_shfunc = 0, is_builtin = 0, is_exec = 0;
    /* Various flags to the command. */
    int cflags = 0, checked = 0;
    LinkList redir;
    wordcode code;
    Wordcode beg = state->pc, varspc;
    FILE *oxtrerr = xtrerr;

    doneps4 = 0;
    redir = (wc_code(*state->pc) == WC_REDIR ? ecgetredirs(state) : NULL);
    if (wc_code(*state->pc) == WC_ASSIGN) {
	varspc = state->pc;
	while (wc_code((code = *state->pc)) == WC_ASSIGN)
	    state->pc += (WC_ASSIGN_TYPE(code) == WC_ASSIGN_SCALAR ?
			  3 : WC_ASSIGN_NUM(code) + 2);
    } else
	varspc = NULL;

    code = *state->pc++;

    type = wc_code(code);

    /* It would be nice if we could use EC_DUPTOK instead of EC_DUP here.
     * But for that we would need to check/change all builtins so that
     * they don't modify their argument strings. */
    args = (type == WC_SIMPLE ?
	    ecgetlist(state, WC_SIMPLE_ARGC(code), EC_DUP, &htok) : NULL);

    for (i = 0; i < 10; i++) {
	save[i] = -2;
	mfds[i] = NULL;
    }

    /* If the command begins with `%', then assume it is a *
     * reference to a job in the job table.                */
    if (type == WC_SIMPLE && args && nonempty(args) &&
	*(char *)peekfirst(args) == '%') {
	pushnode(args, dupstring((how & Z_DISOWN)
				 ? "disown" : (how & Z_ASYNC) ? "bg" : "fg"));
	how = Z_SYNC;
    }

    /* If AUTORESUME is set, the command is SIMPLE, and doesn't have *
     * any redirections, then check if it matches as a prefix of a   *
     * job currently in the job table.  If it does, then we treat it *
     * as a command to resume this job.                              */
    if (isset(AUTORESUME) && type == WC_SIMPLE && (how & Z_SYNC) &&
	args && nonempty(args) && (!redir || empty(redir)) && !input &&
	!nextnode(firstnode(args))) {
	if (unset(NOTIFY))
	    scanjobs();
	if (findjobnam(peekfirst(args)) != -1)
	    pushnode(args, dupstring("fg"));
    }

    /* Check if it's a builtin needing automatic MAGIC_EQUALS_SUBST      *
     * handling.  Things like typeset need this.  We can't detect the    *
     * command if it contains some tokens (e.g. x=ex; ${x}port), so this *
     * only works in simple cases.  has_token() is called to make sure   *
     * this really is a simple case.                                     */
    if (type == WC_SIMPLE) {
	while (args && nonempty(args)) {
	    char *cmdarg = (char *) peekfirst(args);
	    checked = !has_token(cmdarg);
	    if (!checked)
		break;
	    if (!(cflags & (BINF_BUILTIN | BINF_COMMAND)) &&
		(hn = shfunctab->getnode(shfunctab, cmdarg))) {
		is_shfunc = 1;
		break;
	    }
	    if (!(hn = builtintab->getnode(builtintab, cmdarg))) {
		checked = !(cflags & BINF_BUILTIN);
		break;
	    }
	    if (!(hn->flags & BINF_PREFIX)) {
		is_builtin = 1;

		/* autoload the builtin if necessary */
		if (!((Builtin) hn)->handlerfunc) {
		    load_module(((Builtin) hn)->optstr);
		    hn = builtintab->getnode(builtintab, cmdarg);
		}
		assign = (hn->flags & BINF_MAGICEQUALS);
		break;
	    }
	    cflags &= ~BINF_BUILTIN & ~BINF_COMMAND;
	    cflags |= hn->flags;
	    uremnode(args, firstnode(args));
	    hn = NULL;
	    checked = 0;
	    if ((cflags & BINF_COMMAND) && unset(POSIXBUILTINS))
		break;
	}
    }

    /* Do prefork substitutions */
    esprefork = (assign || isset(MAGICEQUALSUBST)) ? PF_TYPESET : 0;
    if (args && htok)
	prefork(args, esprefork);

    if (type == WC_SIMPLE) {
	int unglobbed = 0;

	for (;;) {
	    char *cmdarg;

	    if (!(cflags & BINF_NOGLOB))
		while (!checked && !errflag && args && nonempty(args) &&
		       has_token((char *) peekfirst(args)))
		    zglob(args, firstnode(args), 0);
	    else if (!unglobbed) {
		for (node = firstnode(args); node; incnode(node))
		    untokenize((char *) getdata(node));
		unglobbed = 1;
	    }

	    /* Current shell should not fork unless the *
	     * exec occurs at the end of a pipeline.    */
	    if ((cflags & BINF_EXEC) && last1)
		do_exec = 1;

	    /* Empty command */
	    if (!args || empty(args)) {
		if (redir && nonempty(redir)) {
		    if (do_exec) {
			/* Was this "exec < foobar"? */
			nullexec = 1;
			break;
		    } else if (varspc) {
			nullexec = 2;
			break;
		    } else if (!nullcmd || !*nullcmd || opts[CSHNULLCMD] ||
			       (cflags & BINF_PREFIX)) {
			zerr("redirection with no command", NULL, 0);
			errflag = lastval = 1;
			return;
		    } else if (!nullcmd || !*nullcmd || opts[SHNULLCMD]) {
			if (!args)
			    args = newlinklist();
			addlinknode(args, dupstring(":"));
		    } else if (readnullcmd && *readnullcmd &&
			       ((Redir) peekfirst(redir))->type == REDIR_READ &&
			       !nextnode(firstnode(redir))) {
			if (!args)
			    args = newlinklist();
			addlinknode(args, dupstring(readnullcmd));
		    } else {
			if (!args)
			    args = newlinklist();
			addlinknode(args, dupstring(nullcmd));
		    }
		} else if ((cflags & BINF_PREFIX) && (cflags & BINF_COMMAND)) {
		    lastval = 0;
		    return;
		} else {
		    cmdoutval = 0;
		    if (varspc)
			addvars(state, varspc, 0);
		    if (errflag)
			lastval = errflag;
		    else
			lastval = cmdoutval;
		    if (isset(XTRACE)) {
			fputc('\n', xtrerr);
			fflush(xtrerr);
		    }
		    return;
		}
	    } else if (isset(RESTRICTED) && (cflags & BINF_EXEC) && do_exec) {
		zerrnam("exec", "%s: restricted",
			(char *) getdata(firstnode(args)), 0);
		lastval = 1;
		return;
	    }

	    if (errflag || checked ||
		(unset(POSIXBUILTINS) && (cflags & BINF_COMMAND)))
		break;

	    cmdarg = (char *) peekfirst(args);
	    if (!(cflags & (BINF_BUILTIN | BINF_COMMAND)) &&
		(hn = shfunctab->getnode(shfunctab, cmdarg))) {
		is_shfunc = 1;
		break;
	    }
	    if (!(hn = builtintab->getnode(builtintab, cmdarg))) {
		if (cflags & BINF_BUILTIN) {
		    zwarn("no such builtin: %s", cmdarg, 0);
		    lastval = 1;
		    return;
		}
		break;
	    }
	    if (!(hn->flags & BINF_PREFIX)) {
		is_builtin = 1;

		/* autoload the builtin if necessary */
		if (!((Builtin) hn)->handlerfunc) {
		    load_module(((Builtin) hn)->optstr);
		    hn = builtintab->getnode(builtintab, cmdarg);
		}
		break;
	    }
	    cflags &= ~BINF_BUILTIN & ~BINF_COMMAND;
	    cflags |= hn->flags;
	    uremnode(args, firstnode(args));
	    hn = NULL;
	}
    }

    if (errflag) {
	lastval = 1;
	return;
    }

    /* Get the text associated with this command. */
    if ((how & Z_ASYNC) ||
	(!sfcontext && !sourcelevel && (jobbing || (how & Z_TIMED))))
	text = getjobtext(state->prog, beg);
    else
	text = NULL;

    /* Set up special parameter $_ */

    setunderscore((args && nonempty(args)) ? ((char *) getdata(lastnode(args))) : "");

    /* Warn about "rm *" */
    if (type == WC_SIMPLE && interact && unset(RMSTARSILENT) &&
	isset(SHINSTDIN) && args && nonempty(args) &&
	nextnode(firstnode(args)) && !strcmp(peekfirst(args), "rm")) {
	LinkNode node, next;

	for (node = nextnode(firstnode(args)); node && !errflag; node = next) {
	    char *s = (char *) getdata(node);
	    int l = strlen(s);

	    next = nextnode(node);
	    if (s[0] == Star && !s[1]) {
		if (!checkrmall(pwd))
		    uremnode(args, node);
	    } else if (l > 2 && s[l - 2] == '/' && s[l - 1] == Star) {
		char t = s[l - 2];

		s[l - 2] = 0;
		if (!checkrmall(s))
		    uremnode(args, node);
		s[l - 2] = t;
	    }
	}
	if (!nextnode(firstnode(args)))
	    errflag = 1;
    }

    if (errflag) {
	lastval = 1;
	return;
    }

    if (type == WC_SIMPLE && !nullexec) {
	char *s;
	char trycd = (isset(AUTOCD) && isset(SHINSTDIN) &&
		      (!redir || empty(redir)) && args && !empty(args) &&
		      !nextnode(firstnode(args)) && *(char *)peekfirst(args));

	DPUTS((!args || empty(args)), "BUG: empty(args) in exec.c");
	if (!hn) {
	    /* Resolve external commands */
	    char *cmdarg = (char *) peekfirst(args);
	    char **checkpath = pathchecked;
	    int dohashcmd = isset(HASHCMDS);

	    hn = cmdnamtab->getnode(cmdnamtab, cmdarg);
	    if (hn && trycd && !isreallycom((Cmdnam)hn)) {
		if (!(((Cmdnam)hn)->flags & HASHED)) {
		    checkpath = path;
		    dohashcmd = 1;
		}
		cmdnamtab->removenode(cmdnamtab, cmdarg);
		cmdnamtab->freenode(hn);
		hn = NULL;
	    }
	    if (!hn && dohashcmd && strcmp(cmdarg, "..")) {
		for (s = cmdarg; *s && *s != '/'; s++);
		if (!*s)
		    hn = (HashNode) hashcmd(cmdarg, checkpath);
	    }
	}

	/* If no command found yet, see if it  *
	 * is a directory we should AUTOCD to. */
	if (!hn && trycd && (s = cancd(peekfirst(args)))) {
	    peekfirst(args) = (void *) s;
	    pushnode(args, dupstring("cd"));
	    if ((hn = builtintab->getnode(builtintab, "cd")))
		is_builtin = 1;
	}
    }

    /* This is nonzero if the command is a current shell procedure? */
    is_cursh = (is_builtin || is_shfunc || nullexec || type >= WC_CURSH);

    /**************************************************************************
     * Do we need to fork?  We need to fork if:                               *
     * 1) The command is supposed to run in the background. (or)              *
     * 2) There is no `exec' flag, and either:                                *
     *    a) This is a builtin or shell function with output piped somewhere. *
     *    b) This is an external command and we can't do a `fake exec'.       *
     *                                                                        *
     * A `fake exec' is possible if we have all the following conditions:     *
     * 1) last1 flag is 1.  This indicates that the current shell will not    *
     *    be needed after the current command.  This is typically the case    *
     *    when when the command is the last stage in a subshell, or is the    *
     *    last command after the option `-c'.                                 *
     * 2) We don't have any traps set.                                        *
     * 3) We don't have any files to delete.                                  *
     *                                                                        *
     * The condition above for a `fake exec' will also work for a current     *
     * shell command such as a builtin, but doesn't really buy us anything    *
     * (doesn't save us a process), since it is already running in the        *
     * current shell.                                                         *
     **************************************************************************/

    if ((how & Z_ASYNC) ||
	(!do_exec &&
	 (((is_builtin || is_shfunc) && output) ||
	  (!is_cursh && (last1 != 1 || nsigtrapped || havefiles()))))) {

	pid_t pid;
	int synch[2];
	char dummy;

	child_block();
	pipe(synch);

	if ((pid = zfork()) == -1) {
	    close(synch[0]);
	    close(synch[1]);
	    return;
	} if (pid) {
	    close(synch[1]);
	    read(synch[0], &dummy, 1);
	    close(synch[0]);
#ifdef PATH_DEV_FD
	    closem(2);
#endif
	    if (how & Z_ASYNC) {
		lastpid = (zlong) pid;
	    } else if (!jobtab[thisjob].stty_in_env && varspc) {
		/* search for STTY=... */
		Wordcode p = varspc;
		wordcode ac;

		while (wc_code(ac = *p) == WC_ASSIGN) {
		    if (!strcmp(ecrawstr(state->prog, p + 1, NULL), "STTY")) {
			jobtab[thisjob].stty_in_env = 1;
			break;
		    }
		    p += (WC_ASSIGN_TYPE(ac) == WC_ASSIGN_SCALAR ?
			  3 : WC_ASSIGN_NUM(ac) + 2);
		}
	    }
	    addproc(pid, text);
	    return;
	}
	/* pid == 0 */
	close(synch[0]);
	entersubsh(how, (type != WC_SUBSH) && !(how & Z_ASYNC) ? 2 : 1, 0);
	close(synch[1]);
	forked = 1;
	if (sigtrapped[SIGINT] & ZSIG_IGNORED)
	    holdintr();
#ifdef HAVE_NICE
	/* Check if we should run background jobs at a lower priority. */
	if ((how & Z_ASYNC) && isset(BGNICE))
	    nice(5);
#endif /* HAVE_NICE */

    } else if (is_cursh) {
	/* This is a current shell procedure that didn't need to fork.    *
	 * This includes current shell procedures that are being exec'ed, *
	 * as well as null execs.                                         */
	jobtab[thisjob].stat |= STAT_CURSH;
    } else {
	/* This is an exec (real or fake) for an external command.    *
	 * Note that any form of exec means that the subshell is fake *
	 * (but we may be in a subshell already).                     */
	is_exec = 1;
    }

    if ((esglob = !(cflags & BINF_NOGLOB)) && args && htok) {
	LinkList oargs = args;
	globlist(args, 0);
	args = oargs;
    }
    if (errflag) {
	lastval = 1;
	goto err;
    }

    /* Make a copy of stderr for xtrace output before redirecting */
    fflush(xtrerr);
    if (isset(XTRACE) && xtrerr == stderr &&
	(type < WC_SUBSH || type == WC_TIMED)) {
	if (!(xtrerr = fdopen(movefd(dup(fileno(stderr))), "w")))
	    xtrerr = stderr;
	else
	    fdtable[fileno(xtrerr)] = 3;
    }

    /* Add pipeline input/output to mnodes */
    if (input)
	addfd(forked, save, mfds, 0, input, 0);
    if (output)
	addfd(forked, save, mfds, 1, output, 1);

    /* Do process substitutions */
    if (redir)
	spawnpipes(redir);

    /* Do io redirections */
    while (redir && nonempty(redir)) {
	fn = (Redir) ugetnode(redir);
	DPUTS(fn->type == REDIR_HEREDOC || fn->type == REDIR_HEREDOCDASH,
	      "BUG: unexpanded here document");
	if (fn->type == REDIR_INPIPE) {
	    if (fn->fd2 == -1) {
		closemnodes(mfds);
		fixfds(save);
		execerr();
	    }
	    addfd(forked, save, mfds, fn->fd1, fn->fd2, 0);
	} else if (fn->type == REDIR_OUTPIPE) {
	    if (fn->fd2 == -1) {
		closemnodes(mfds);
		fixfds(save);
		execerr();
	    }
	    addfd(forked, save, mfds, fn->fd1, fn->fd2, 1);
	} else {
	    if (fn->type != REDIR_HERESTR && xpandredir(fn, redir))
		continue;
	    if (errflag) {
		closemnodes(mfds);
		fixfds(save);
		execerr();
	    }
	    if (isset(RESTRICTED) && IS_WRITE_FILE(fn->type)) {
		zwarn("writing redirection not allowed in restricted mode", NULL, 0);
		execerr();
	    }
	    if (unset(EXECOPT))
		continue;
	    switch(fn->type) {
	    case REDIR_HERESTR:
		fil = getherestr(fn);
		if (fil == -1) {
		    closemnodes(mfds);
		    fixfds(save);
		    if (errno != EINTR)
			zwarn("%e", NULL, errno);
		    execerr();
		}
		addfd(forked, save, mfds, fn->fd1, fil, 0);
		break;
	    case REDIR_READ:
	    case REDIR_READWRITE:
		if (fn->type == REDIR_READ)
		    fil = open(unmeta(fn->name), O_RDONLY | O_NOCTTY);
		else
		    fil = open(unmeta(fn->name),
			       O_RDWR | O_CREAT | O_NOCTTY, 0666);
		if (fil == -1) {
		    closemnodes(mfds);
		    fixfds(save);
		    if (errno != EINTR)
			zwarn("%e: %s", fn->name, errno);
		    execerr();
		}
		addfd(forked, save, mfds, fn->fd1, fil, 0);
		/* If this is 'exec < file', read from stdin, *
		 * not terminal, unless `file' is a terminal. */
		if (nullexec == 1 && fn->fd1 == 0 &&
		    isset(SHINSTDIN) && interact && !zleactive)
		    init_io();
		break;
	    case REDIR_CLOSE:
		if (!forked && fn->fd1 < 10 && save[fn->fd1] == -2)
		    save[fn->fd1] = movefd(fn->fd1);
		closemn(mfds, fn->fd1);
		zclose(fn->fd1);
		break;
	    case REDIR_MERGEIN:
	    case REDIR_MERGEOUT:
		if (fn->fd2 < 10)
		    closemn(mfds, fn->fd2);
		if (fn->fd2 > 9 &&
		    (fdtable[fn->fd2] ||
		     fn->fd2 == coprocin ||
		     fn->fd2 == coprocout)) {
		    fil = -1;
		    errno = EBADF;
		} else {
		    int fd = fn->fd2;
		    if(fd == -2)
			fd = (fn->type == REDIR_MERGEOUT) ? coprocout : coprocin;
		    fil = dup(fd);
		}
		if (fil == -1) {
		    char fdstr[4];

		    closemnodes(mfds);
		    fixfds(save);
		    if (fn->fd2 != -2)
		    	sprintf(fdstr, "%d", fn->fd2);
		    zwarn("%s: %e", fn->fd2 == -2 ? "coprocess" : fdstr, errno);
		    execerr();
		}
		addfd(forked, save, mfds, fn->fd1, fil, fn->type == REDIR_MERGEOUT);
		break;
	    default:
		if (IS_APPEND_REDIR(fn->type))
		    fil = open(unmeta(fn->name),
			       (unset(CLOBBER) && !IS_CLOBBER_REDIR(fn->type)) ?
			       O_WRONLY | O_APPEND | O_NOCTTY :
			       O_WRONLY | O_APPEND | O_CREAT | O_NOCTTY, 0666);
		else
		    fil = clobber_open(fn);
		if(fil != -1 && IS_ERROR_REDIR(fn->type))
		    dfil = dup(fil);
		else
		    dfil = 0;
		if (fil == -1 || dfil == -1) {
		    if(fil != -1)
			close(fil);
		    closemnodes(mfds);
		    fixfds(save);
		    if (errno != EINTR)
			zwarn("%e: %s", fn->name, errno);
		    execerr();
		}
		addfd(forked, save, mfds, fn->fd1, fil, 1);
		if(IS_ERROR_REDIR(fn->type))
		    addfd(forked, save, mfds, 2, dfil, 1);
		break;
	    }
	}
    }

    /* We are done with redirection.  close the mnodes, *
     * spawning tee/cat processes as necessary.         */
    for (i = 0; i < 10; i++)
	if (mfds[i] && mfds[i]->ct >= 2)
	    closemn(mfds, i);

    if (nullexec) {
	if (nullexec == 1) {
	    /*
	     * If nullexec is 1 we specifically *don't* restore the original
	     * fd's before returning.
	     */
	    for (i = 0; i < 10; i++)
		if (save[i] != -2)
		    zclose(save[i]);
	    goto done;
	}
	/*
	 * If nullexec is 2, we have variables to add with the redirections
	 * in place.
	 */
	if (varspc)
	    addvars(state, varspc, 0);
	lastval = errflag ? errflag : cmdoutval;
	if (isset(XTRACE)) {
	    fputc('\n', xtrerr);
	    fflush(xtrerr);
	}
    } else if (isset(EXECOPT) && !errflag) {
	/*
	 * We delay the entersubsh() to here when we are exec'ing
	 * the current shell (including a fake exec to run a builtin then
	 * exit) in case there is an error return.
	 */
	if (is_exec)
	    entersubsh(how, (type != WC_SUBSH) ? 2 : 1, 1);
	if (type >= WC_CURSH) {
	    if (last1 == 1)
		do_exec = 1;
	    lastval = (execfuncs[type - WC_CURSH])(state, do_exec);
	} else if (is_builtin || is_shfunc) {
	    LinkList restorelist = 0, removelist = 0;
	    /* builtin or shell function */

	    if (!forked && ((cflags & BINF_COMMAND) ||
			    (unset(POSIXBUILTINS) && !assign) ||
			    (isset(POSIXBUILTINS) && !is_shfunc &&
			     !(hn->flags & BINF_PSPECIAL)))) {
		if (varspc)
		    save_params(state, varspc, &restorelist, &removelist);
		else
		    restorelist = removelist = NULL;
	    }
	    if (varspc) {
		/* Export this if the command is a shell function,
		 * but not if it's a builtin.
		 */
		addvars(state, varspc, is_shfunc);
		if (errflag) {
		    if (restorelist)
			restore_params(restorelist, removelist);
		    lastval = 1;
		    fixfds(save);
		    goto done;
		}
	    }

	    if (is_shfunc) {
		/* It's a shell function */

#ifdef PATH_DEV_FD
		int i;

		for (i = 10; i <= max_zsh_fd; i++)
		    if (fdtable[i] > 1)
			fdtable[i]++;
#endif
		if (subsh_close >= 0)
		    zclose(subsh_close);
		subsh_close = -1;

		execshfunc((Shfunc) hn, args);
#ifdef PATH_DEV_FD
		for (i = 10; i <= max_zsh_fd; i++)
		    if (fdtable[i] > 1)
			if (--(fdtable[i]) <= 2)
			    zclose(i);
#endif
	    } else {
		/* It's a builtin */
		if (forked)
		    closem(1);
		lastval = execbuiltin(args, (Builtin) hn);
#ifdef PATH_DEV_FD
		closem(2);
#endif
		fflush(stdout);
		if (save[1] == -2) {
		    if (ferror(stdout)) {
			zwarn("write error: %e", NULL, errno);
			clearerr(stdout);
		    }
		} else
		    clearerr(stdout);
	    }
	    if (isset(PRINTEXITVALUE) && isset(SHINSTDIN) &&
		lastval && !subsh) {
		fprintf(stderr, "zsh: exit %ld\n", (long)lastval);
	    }

	    if (do_exec) {
		if (subsh)
		    _exit(lastval);

		/* If we are exec'ing a command, and we are not in a subshell, *
		 * then check if we should save the history file.              */
		if (isset(RCS) && interact && !nohistsave)
		    savehistfile(NULL, 1, HFILE_USE_OPTIONS);
		exit(lastval);
	    }
	    if (restorelist)
		restore_params(restorelist, removelist);

	} else {
	    if (!forked)
		setiparam("SHLVL", --shlvl);
	    if (do_exec) {
		/* If we are exec'ing a command, and we are not *
		 * in a subshell, then save the history file.   */
		if (!subsh && isset(RCS) && interact && !nohistsave)
		    savehistfile(NULL, 1, HFILE_USE_OPTIONS);
	    }
	    if (type == WC_SIMPLE) {
		if (varspc) {
		    addvars(state, varspc, -1);
		    if (errflag)
			_exit(1);
		}
		closem(1);
		if (coprocin)
		    zclose(coprocin);
		if (coprocout)
		    zclose(coprocout);
#ifdef HAVE_GETRLIMIT
		if (!forked)
		    setlimits(NULL);
#endif
		if (how & Z_ASYNC) {
		    zsfree(STTYval);
		    STTYval = 0;
		}
		execute((Cmdnam) hn, cflags & BINF_DASH);
	    } else {		/* ( ... ) */
		DPUTS(varspc,
		      "BUG: assigment before complex command");
		list_pipe = 0;
		if (subsh_close >= 0)
		    zclose(subsh_close);
                subsh_close = -1;
		/* If we're forked (and we should be), no need to return */
		DPUTS(last1 != 1 && !forked, "BUG: not exiting?");
		execlist(state, 0, 1);
	    }
	}
    }

  err:
    if (forked)
	_exit(lastval);
    fixfds(save);

 done:
    if (xtrerr != oxtrerr) {
	fil = fileno(xtrerr);
	fclose(xtrerr);
	xtrerr = oxtrerr;
	zclose(fil);
    }

    zsfree(STTYval);
    STTYval = 0;
}

/* Arrange to have variables restored. */

/**/
static void
save_params(Estate state, Wordcode pc, LinkList *restore_p, LinkList *remove_p)
{
    Param pm;
    char *s;
    wordcode ac;

    *restore_p = newlinklist();
    *remove_p = newlinklist();

    while (wc_code(ac = *pc) == WC_ASSIGN) {
	s = ecrawstr(state->prog, pc + 1, NULL);
	if ((pm = (Param) paramtab->getnode(paramtab, s))) {
	    if (!(pm->flags & PM_SPECIAL)) {
		paramtab->removenode(paramtab, s);
	    } else if (!(pm->flags & PM_READONLY) &&
		       (unset(RESTRICTED) || !(pm->flags & PM_RESTRICTED))) {
		Param tpm = (Param) zhalloc(sizeof *tpm);
		tpm->nam = pm->nam;
		copyparam(tpm, pm, 1);
		pm = tpm;
	    }
	    addlinknode(*remove_p, s);
	    addlinknode(*restore_p, pm);
	} else
	    addlinknode(*remove_p, s);

	pc += (WC_ASSIGN_TYPE(ac) == WC_ASSIGN_SCALAR ?
	       3 : WC_ASSIGN_NUM(ac) + 2);
    }
}

/* Restore saved parameters after executing a shfunc or builtin */

/**/
static void
restore_params(LinkList restorelist, LinkList removelist)
{
    Param pm;
    char *s;

    /* remove temporary parameters */
    while ((s = (char *) ugetnode(removelist))) {
	if ((pm = (Param) paramtab->getnode(paramtab, s)) &&
	    !(pm->flags & PM_SPECIAL)) {
	    pm->flags &= ~PM_READONLY;
	    unsetparam_pm(pm, 0, 0);
	}
    }

    if (restorelist) {
	/* restore saved parameters */
	while ((pm = (Param) ugetnode(restorelist))) {
	    if (pm->flags & PM_SPECIAL) {
		Param tpm = (Param) paramtab->getnode(paramtab, pm->nam);

		DPUTS(!tpm || PM_TYPE(pm->flags) != PM_TYPE(tpm->flags) ||
		      !(pm->flags & PM_SPECIAL),
		      "BUG: in restoring special parameters");
		tpm->flags = pm->flags;
		switch (PM_TYPE(pm->flags)) {
		case PM_SCALAR:
		    tpm->sets.cfn(tpm, pm->u.str);
		    break;
		case PM_INTEGER:
		    tpm->sets.ifn(tpm, pm->u.val);
		    break;
		case PM_EFLOAT:
		case PM_FFLOAT:
		    tpm->sets.ffn(tpm, pm->u.dval);
		    break;
		case PM_ARRAY:
		    tpm->sets.afn(tpm, pm->u.arr);
		    break;
		case PM_HASHED:
		    tpm->sets.hfn(tpm, pm->u.hash);
		    break;
		}
	    } else
		paramtab->addnode(paramtab, pm->nam, pm);
	    if ((pm->flags & PM_EXPORTED) && ((s = getsparam(pm->nam))))
		pm->env = addenv(pm->nam, s, pm->flags);
	}
    }
}

/* restore fds after redirecting a builtin */

/**/
static void
fixfds(int *save)
{
    int old_errno = errno;
    int i;

    for (i = 0; i != 10; i++)
	if (save[i] != -2)
	    redup(save[i], i);
    errno = old_errno;
}

/**/
static void
entersubsh(int how, int cl, int fake)
{
    int sig;

    if (cl != 2)
	for (sig = 0; sig < VSIGCOUNT; sig++)
	    if (!(sigtrapped[sig] & ZSIG_FUNC))
		unsettrap(sig);
    if (unset(MONITOR)) {
	if (how & Z_ASYNC) {
	    settrap(SIGINT, NULL);
	    settrap(SIGQUIT, NULL);
	    if (isatty(0)) {
		close(0);
		if (open("/dev/null", O_RDWR | O_NOCTTY)) {
		    zerr("can't open /dev/null: %e", NULL, errno);
		    _exit(1);
		}
	    }
	}
    } else if (thisjob != -1 && cl) {
	if (jobtab[list_pipe_job].gleader && (list_pipe || list_pipe_child)) {
	    if (setpgrp(0L, jobtab[list_pipe_job].gleader) == -1 ||
		killpg(jobtab[list_pipe_job].gleader, 0) == -1) {
		jobtab[list_pipe_job].gleader =
		    jobtab[thisjob].gleader = (list_pipe_child ? mypgrp : getpid());
		setpgrp(0L, jobtab[list_pipe_job].gleader);
		if (how & Z_SYNC)
		    attachtty(jobtab[thisjob].gleader);
	    }
	}
	else if (!jobtab[thisjob].gleader ||
		 setpgrp(0L, jobtab[thisjob].gleader) == -1) {
	    jobtab[thisjob].gleader = getpid();
	    if (list_pipe_job != thisjob &&
		!jobtab[list_pipe_job].gleader)
		jobtab[list_pipe_job].gleader = jobtab[thisjob].gleader;
	    setpgrp(0L, jobtab[thisjob].gleader);
	    if (how & Z_SYNC)
		attachtty(jobtab[thisjob].gleader);
	}
    }
    if (!fake)
	subsh = 1;
    if (SHTTY != -1) {
	shout = NULL;
	zclose(SHTTY);
	SHTTY = -1;
    }
    if (isset(MONITOR)) {
	signal_default(SIGTTOU);
	signal_default(SIGTTIN);
	signal_default(SIGTSTP);
    }
    if (interact) {
	signal_default(SIGTERM);
	if (!(sigtrapped[SIGINT] & ZSIG_IGNORED))
	    signal_default(SIGINT);
    }
    if (!(sigtrapped[SIGQUIT] & ZSIG_IGNORED))
	signal_default(SIGQUIT);
    opts[MONITOR] = opts[USEZLE] = 0;
    zleactive = 0;
    if (cl)
	clearjobtab();
    times(&shtms);
}

/* close internal shell fds */

/**/
mod_export void
closem(int how)
{
    int i;

    for (i = 10; i <= max_zsh_fd; i++)
	if (fdtable[i] && (!how || fdtable[i] == how))
	    zclose(i);
}

/* convert here document into a here string */

/**/
char *
gethere(char *str, int typ)
{
    char *buf;
    int bsiz, qt = 0, strip = 0;
    char *s, *t, *bptr, c;

    for (s = str; *s; s++)
	if (INULL(*s)) {
	    *s = Nularg;
	    qt = 1;
	}
    untokenize(str);
    if (typ == REDIR_HEREDOCDASH) {
	strip = 1;
	while (*str == '\t')
	    str++;
    }
    bptr = buf = zalloc(bsiz = 256);
    for (;;) {
	t = bptr;

	while ((c = hgetc()) == '\t' && strip)
	    ;
	for (;;) {
	    if (bptr == buf + bsiz) {
		buf = realloc(buf, 2 * bsiz);
		t = buf + bsiz - (bptr - t);
		bptr = buf + bsiz;
		bsiz *= 2;
	    }
	    if (lexstop || c == '\n')
		break;
	    *bptr++ = c;
	    c = hgetc();
	}
	*bptr = '\0';
	if (!strcmp(t, str))
	    break;
	if (lexstop) {
	    t = bptr;
	    break;
	}
	*bptr++ = '\n';
    }
    if (t > buf && t[-1] == '\n')
	t--;
    *t = '\0';
    if (!qt) {
	int ef = errflag;

	parsestr(buf);

	if (!errflag)
	    errflag = ef;
    }
    s = dupstring(buf);
    zfree(buf, bsiz);
    return s;
}

/* open here string fd */

/**/
static int
getherestr(struct redir *fn)
{
    char *s, *t;
    int fd, len;

    t = fn->name;
    singsub(&t);
    untokenize(t);
    unmetafy(t, &len);
    t[len++] = '\n';
    s = gettempname();
    if (!s || (fd = open(s, O_CREAT|O_WRONLY|O_EXCL|O_NOCTTY, 0600)) == -1)
	return -1;
    write(fd, t, len);
    close(fd);
    fd = open(s, O_RDONLY | O_NOCTTY);
    unlink(s);
    return fd;
}

/* $(...) */

/**/
LinkList
getoutput(char *cmd, int qt)
{
    Eprog prog;
    int pipes[2];
    pid_t pid;
    Wordcode pc;

    if (!(prog = parse_string(cmd, 0)))
	return NULL;

    pc = prog->prog;
    if (prog != &dummy_eprog &&
	wc_code(pc[0]) == WC_LIST && (WC_LIST_TYPE(pc[0]) & Z_END) &&
	wc_code(pc[1]) == WC_SUBLIST && !WC_SUBLIST_FLAGS(pc[1]) &&
	WC_SUBLIST_TYPE(pc[1]) == WC_SUBLIST_END &&
	wc_code(pc[2]) == WC_PIPE && WC_PIPE_TYPE(pc[2]) == WC_PIPE_END &&
	wc_code(pc[3]) == WC_REDIR && WC_REDIR_TYPE(pc[3]) == REDIR_READ && 
	!pc[4] &&
	wc_code(pc[6]) == WC_SIMPLE && !WC_SIMPLE_ARGC(pc[6])) {
	/* $(< word) */
	int stream;
	char *s = dupstring(ecrawstr(prog, pc + 5, NULL));

	singsub(&s);
	if (errflag)
	    return NULL;
	untokenize(s);
	if ((stream = open(unmeta(s), O_RDONLY | O_NOCTTY)) == -1) {
	    zerr("%e: %s", s, errno);
	    return NULL;
	}
	return readoutput(stream, qt);
    }
    mpipe(pipes);
    child_block();
    cmdoutval = 0;
    if ((cmdoutpid = pid = zfork()) == -1) {
	/* fork error */
	zclose(pipes[0]);
	zclose(pipes[1]);
	errflag = 1;
	cmdoutpid = 0;
	child_unblock();
	return NULL;
    } else if (pid) {
	LinkList retval;

	zclose(pipes[1]);
	retval = readoutput(pipes[0], qt);
	fdtable[pipes[0]] = 0;
	waitforpid(pid);		/* unblocks */
	lastval = cmdoutval;
	return retval;
    }
    /* pid == 0 */
    child_unblock();
    zclose(pipes[0]);
    redup(pipes[1], 1);
    opts[MONITOR] = 0;
    entersubsh(Z_SYNC, 1, 0);
    cmdpush(CS_CMDSUBST);
    execode(prog, 0, 1);
    cmdpop();
    close(1);
    _exit(lastval);
    zerr("exit returned in child!!", NULL, 0);
    kill(getpid(), SIGKILL);
    return NULL;
}

/* read output of command substitution */

/**/
mod_export LinkList
readoutput(int in, int qt)
{
    LinkList ret;
    char *buf, *ptr;
    int bsiz, c, cnt = 0;
    FILE *fin;

    fin = fdopen(in, "r");
    ret = newlinklist();
    ptr = buf = (char *) hcalloc(bsiz = 64);
    while ((c = fgetc(fin)) != EOF || errno == EINTR) {
	if (c == EOF) {
	    errno = 0;
	    clearerr(fin);
	    continue;
	}
	if (imeta(c)) {
	    *ptr++ = Meta;
	    c ^= 32;
	    cnt++;
	}
	if (++cnt >= bsiz) {
	    char *pp = (char *) hcalloc(bsiz *= 2);

	    memcpy(pp, buf, cnt - 1);
	    ptr = (buf = pp) + cnt - 1;
	}
	*ptr++ = c;
    }
    fclose(fin);
    while (cnt && ptr[-1] == '\n')
	ptr--, cnt--;
    *ptr = '\0';
    if (qt) {
	if (!cnt) {
	    *ptr++ = Nularg;
	    *ptr = '\0';
	}
	addlinknode(ret, buf);
    } else {
	char **words = spacesplit(buf, 0, 1, 0);

	while (*words) {
	    if (isset(GLOBSUBST))
		tokenize(*words);
	    addlinknode(ret, *words++);
	}
    }
    return ret;
}

/**/
static Eprog
parsecmd(char *cmd)
{
    char *str;
    Eprog prog;

    for (str = cmd + 2; *str && *str != Outpar; str++);
    if (!*str || cmd[1] != Inpar) {
	zerr("oops.", NULL, 0);
	return NULL;
    }
    *str = '\0';
    if (str[1] || !(prog = parse_string(cmd + 2, 0))) {
	zerr("parse error in process substitution", NULL, 0);
	return NULL;
    }
    return prog;
}

/* =(...) */

/**/
char *
getoutputfile(char *cmd)
{
    pid_t pid;
    char *nam;
    Eprog prog;
    int fd;

    if (thisjob == -1)
	return NULL;
    if (!(prog = parsecmd(cmd)))
	return NULL;
    if (!(nam = gettempname()))
	return NULL;

    nam = ztrdup(nam);

    if (!jobtab[thisjob].filelist)
	jobtab[thisjob].filelist = znewlinklist();
    zaddlinknode(jobtab[thisjob].filelist, nam);

    child_block();
    fd = open(nam, O_WRONLY | O_CREAT | O_EXCL | O_NOCTTY, 0600);

    if (fd < 0 || (cmdoutpid = pid = zfork()) == -1) {
	/* fork or open error */
	child_unblock();
	return nam;
    } else if (pid) {
	int os;

	close(fd);
	os = jobtab[thisjob].stat;
	waitforpid(pid);
	cmdoutval = 0;
	jobtab[thisjob].stat = os;
	return nam;
    }

    /* pid == 0 */
    redup(fd, 1);
    opts[MONITOR] = 0;
    entersubsh(Z_SYNC, 1, 0);
    cmdpush(CS_CMDSUBST);
    execode(prog, 0, 1);
    cmdpop();
    close(1);
    _exit(lastval);
    zerr("exit returned in child!!", NULL, 0);
    kill(getpid(), SIGKILL);
    return NULL;
}

#if !defined(PATH_DEV_FD) && defined(HAVE_FIFOS)
/* get a temporary named pipe */

static char *
namedpipe(void)
{
    char *tnam = gettempname();

# ifdef HAVE_MKFIFO
    if (mkfifo(tnam, 0600) < 0)
# else
    if (mknod(tnam, 0010600, 0) < 0)
# endif
	return NULL;
    return tnam;
}
#endif /* ! PATH_DEV_FD && HAVE_FIFOS */

/* <(...) or >(...) */

/**/
char *
getproc(char *cmd)
{
#if !defined(HAVE_FIFOS) && !defined(PATH_DEV_FD)
    zerr("doesn't look like your system supports FIFOs.", NULL, 0);
    return NULL;
#else
    Eprog prog;
    int out = *cmd == Inang;
    char *pnam;
#ifndef PATH_DEV_FD
    int fd;
#else
    int pipes[2];
#endif

    if (thisjob == -1)
	return NULL;
#ifndef PATH_DEV_FD
    if (!(pnam = namedpipe()))
	return NULL;
#else
    pnam = hcalloc(strlen(PATH_DEV_FD) + 6);
#endif
    if (!(prog = parsecmd(cmd)))
	return NULL;
#ifndef PATH_DEV_FD
    if (!jobtab[thisjob].filelist)
	jobtab[thisjob].filelist = znewlinklist();
    zaddlinknode(jobtab[thisjob].filelist, ztrdup(pnam));

    if (zfork()) {
#else
    mpipe(pipes);
    if (zfork()) {
	sprintf(pnam, "%s/%d", PATH_DEV_FD, pipes[!out]);
	zclose(pipes[out]);
	fdtable[pipes[!out]] = 2;
#endif
	return pnam;
    }
#ifndef PATH_DEV_FD
    closem(0);
    fd = open(pnam, out ? O_WRONLY | O_NOCTTY : O_RDONLY | O_NOCTTY);
    if (fd == -1) {
	zerr("can't open %s: %e", pnam, errno);
	_exit(1);
    }
    entersubsh(Z_ASYNC, 1, 0);
    redup(fd, out);
#else
    entersubsh(Z_ASYNC, 1, 0);
    redup(pipes[out], out);
    closem(0);   /* this closes pipes[!out] as well */
#endif
    cmdpush(CS_CMDSUBST);
    execode(prog, 0, 1);
    cmdpop();
    zclose(out);
    _exit(lastval);
    return NULL;
#endif   /* HAVE_FIFOS and PATH_DEV_FD not defined */
}

/* > >(...) or < <(...) (does not use named pipes) */

/**/
static int
getpipe(char *cmd)
{
    Eprog prog;
    int pipes[2], out = *cmd == Inang;

    if (!(prog = parsecmd(cmd)))
	return -1;
    mpipe(pipes);
    if (zfork()) {
	zclose(pipes[out]);
	return pipes[!out];
    }
    entersubsh(Z_ASYNC, 1, 0);
    redup(pipes[out], out);
    closem(0);	/* this closes pipes[!out] as well */
    cmdpush(CS_CMDSUBST);
    execode(prog, 0, 1);
    cmdpop();
    _exit(lastval);
    return 0;
}

/* open pipes with fds >= 10 */

/**/
static void
mpipe(int *pp)
{
    pipe(pp);
    pp[0] = movefd(pp[0]);
    pp[1] = movefd(pp[1]);
}

/* Do process substitution with redirection */

/**/
static void
spawnpipes(LinkList l)
{
    LinkNode n;
    Redir f;
    char *str;

    n = firstnode(l);
    for (; n; incnode(n)) {
	f = (Redir) getdata(n);
	if (f->type == REDIR_OUTPIPE || f->type == REDIR_INPIPE) {
	    str = f->name;
	    f->fd2 = getpipe(str);
	}
    }
}

extern int tracingcond;

/* evaluate a [[ ... ]] */

/**/
static int
execcond(Estate state, int do_exec)
{
    int stat;

    state->pc--;
    if (isset(XTRACE)) {
	printprompt4();
	fprintf(xtrerr, "[[");
	tracingcond++;
    }
    cmdpush(CS_COND);
    stat = !evalcond(state);
    cmdpop();
    if (isset(XTRACE)) {
	fprintf(xtrerr, " ]]\n");
	fflush(xtrerr);
	tracingcond--;
    }
    return stat;
}

/* evaluate a ((...)) arithmetic command */

/**/
static int
execarith(Estate state, int do_exec)
{
    char *e;
    zlong val = 0;
    int htok = 0;

    if (isset(XTRACE)) {
	printprompt4();
	fprintf(xtrerr, "((");
    }
    cmdpush(CS_MATH);
    e = ecgetstr(state, EC_DUPTOK, &htok);
    if (htok)
	singsub(&e);
    if (isset(XTRACE))
	fprintf(xtrerr, " %s", e);

    val = mathevali(e);

    cmdpop();

    if (isset(XTRACE)) {
	fprintf(xtrerr, " ))\n");
	fflush(xtrerr);
    }
    errflag = 0;
    return !val;
}

/* perform time ... command */

/**/
static int
exectime(Estate state, int do_exec)
{
    int jb;

    jb = thisjob;
    if (WC_TIMED_TYPE(state->pc[-1]) == WC_TIMED_EMPTY) {
	shelltime();
	return 0;
    }
    execpline(state, *state->pc++, Z_TIMED|Z_SYNC, 0);
    thisjob = jb;
    return lastval;
}

/* Define a shell function */

/**/
static int
execfuncdef(Estate state, int do_exec)
{
    Shfunc shf;
    char *s;
    int signum, nprg, sbeg, nstrs, npats, len, plen, i, htok = 0;
    Wordcode beg = state->pc, end;
    Eprog prog;
    Patprog *pp;
    LinkList names;

    end = beg + WC_FUNCDEF_SKIP(state->pc[-1]);
    if (!(names = ecgetlist(state, *state->pc++, EC_DUPTOK, &htok))) {
	state->pc = end;
	return 0;
    }
    nprg = end - beg;
    sbeg = *state->pc++;
    nstrs = *state->pc++;
    npats = *state->pc++;

    nprg = (end - state->pc);
    plen = nprg * sizeof(wordcode);
    len = plen + (npats * sizeof(Patprog)) + nstrs;

    if (htok)
	execsubst(names);

    while ((s = (char *) ugetnode(names))) {
	prog = (Eprog) zalloc(sizeof(*prog));
	prog->npats = npats;
	prog->len = len;
	if (state->prog->dump) {
	    prog->flags = EF_MAP;
	    incrdumpcount(state->prog->dump);
	    prog->pats = pp = (Patprog *) zalloc(npats * sizeof(Patprog));
	    prog->prog = state->pc;
	    prog->strs = state->strs + sbeg;
	    prog->dump = state->prog->dump;
	} else {
	    prog->flags = EF_REAL;
	    prog->pats = pp = (Patprog *) zalloc(len);
	    prog->prog = (Wordcode) (prog->pats + npats);
	    prog->strs = (char *) (prog->prog + nprg);
	    prog->dump = NULL;
	    memcpy(prog->prog, state->pc, plen);
	    memcpy(prog->strs, state->strs + sbeg, nstrs);
	}
	for (i = npats; i--; pp++)
	    *pp = dummy_patprog1;
	prog->shf = NULL;

	shf = (Shfunc) zalloc(sizeof(*shf));
	shf->funcdef = prog;
	shf->flags = 0;

	/* is this shell function a signal trap? */
	if (!strncmp(s, "TRAP", 4) &&
	    (signum = getsignum(s + 4)) != -1) {
	    if (settrap(signum, shf->funcdef)) {
		freeeprog(shf->funcdef);
		zfree(shf, sizeof(*shf));
		state->pc = end;
		return 1;
	    }
	    sigtrapped[signum] |= ZSIG_FUNC;
	}
	shfunctab->addnode(shfunctab, ztrdup(s), shf);
    }
    state->pc = end;
    return 0;
}

/* Main entry point to execute a shell function. */

/**/
static void
execshfunc(Shfunc shf, LinkList args)
{
    LinkList last_file_list = NULL;
    unsigned char *ocs;
    int ocsp, osfc;

    if (errflag)
	return;

    if (!list_pipe && thisjob != list_pipe_job) {
	/* Without this deletejob the process table *
	 * would be filled by a recursive function. */
	last_file_list = jobtab[thisjob].filelist;
	jobtab[thisjob].filelist = NULL;
	deletejob(jobtab + thisjob);
    }
    if (isset(XTRACE)) {
	LinkNode lptr;
	printprompt4();
	if (args)
	    for (lptr = firstnode(args); lptr; incnode(lptr)) {
		if (lptr != firstnode(args))
		    fputc(' ', xtrerr);
		fprintf(xtrerr, "%s", (char *)getdata(lptr));
	    }
	fputc('\n', xtrerr);
	fflush(xtrerr);
    }
    ocs = cmdstack;
    ocsp = cmdsp;
    cmdstack = (unsigned char *) zalloc(CMDSTACKSZ);
    cmdsp = 0;
    if ((osfc = sfcontext) == SFC_NONE)
	sfcontext = SFC_DIRECT;
    doshfunc(shf->nam, shf->funcdef, args, shf->flags, 0);
    sfcontext = osfc;
    free(cmdstack);
    cmdstack = ocs;
    cmdsp = ocsp;

    if (!list_pipe)
	deletefilelist(last_file_list);
}

/* Function to execute the special type of command that represents an *
 * autoloaded shell function.  The command structure tells us which   *
 * function it is.  This function is actually called as part of the   *
 * execution of the autoloaded function itself, so when the function  *
 * has been autoloaded, its list is just run with no frills.          */

/**/
static int
execautofn(Estate state, int do_exec)
{
    Shfunc shf;
    char *oldscriptname;

    if (!(shf = loadautofn(state->prog->shf, 1, 0)))
	return 1;

    oldscriptname = scriptname;
    scriptname = dupstring(shf->nam);
    execode(shf->funcdef, 1, 0);
    scriptname = oldscriptname;

    return lastval;
}

/**/
Shfunc
loadautofn(Shfunc shf, int fksh, int autol)
{
    int noalias = noaliases, ksh = 1;
    Eprog prog;

    pushheap();

    noaliases = (shf->flags & PM_UNALIASED);
    prog = getfpfunc(shf->nam, &ksh);
    noaliases = noalias;

    if (ksh == 1)
	ksh = fksh;

    if (prog == &dummy_eprog) {
	/* We're not actually in the function; decrement locallevel */
	locallevel--;
	zwarn("%s: function definition file not found", shf->nam, 0);
	locallevel++;
	popheap();
	return NULL;
    }
    if (!prog)
	prog = &dummy_eprog;
    if (ksh == 2 || (ksh == 1 && isset(KSHAUTOLOAD))) {
	if (autol) {
	    prog->flags |= EF_RUN;

	    freeeprog(shf->funcdef);
	    if (prog->flags & EF_MAP)
		shf->funcdef = prog;
	    else
		shf->funcdef = dupeprog(prog, 0);
	    shf->flags &= ~PM_UNDEFINED;
	} else {
	    VARARR(char, n, strlen(shf->nam) + 1);
	    strcpy(n, shf->nam);
	    execode(prog, 1, 0);
	    shf = (Shfunc) shfunctab->getnode(shfunctab, n);
	    if (!shf || (shf->flags & PM_UNDEFINED)) {
		/* We're not actually in the function; decrement locallevel */
		locallevel--;
		zwarn("%s: function not defined by file", n, 0);
		locallevel++;
		popheap();
		return NULL;
	    }
	}
    } else {
	freeeprog(shf->funcdef);
	if (prog->flags & EF_MAP)
	    shf->funcdef = stripkshdef(prog, shf->nam);
	else
	    shf->funcdef = dupeprog(stripkshdef(prog, shf->nam), 0);
	shf->flags &= ~PM_UNDEFINED;
    }
    popheap();

    return shf;
}

/* execute a shell function */

/**/
mod_export void
doshfunc(char *name, Eprog prog, LinkList doshargs, int flags, int noreturnval)
/* If noreturnval is nonzero, then reset the current return *
 * value (lastval) to its value before the shell function   *
 * was executed.                                            */
{
    char **tab, **x, *oargv0;
    int oldzoptind, oldlastval, oldoptcind;
    char saveopts[OPT_SIZE], *oldscriptname = scriptname, *fname = dupstring(name);
    int obreaks;
    struct funcstack fstack;
#ifdef MAX_FUNCTION_DEPTH
    static int funcdepth;
#endif

    pushheap();

    oargv0 = NULL;
    obreaks = breaks;;
    if (trapreturn < 0)
	trapreturn--;
    oldlastval = lastval;

    starttrapscope();

    tab = pparams;
    if (!(flags & PM_UNDEFINED))
	scriptname = dupstring(name);
    oldzoptind = zoptind;
    zoptind = 1;
    oldoptcind = optcind;
    optcind = 0;

    /* We need to save the current options even if LOCALOPTIONS is *
     * not currently set.  That's because if it gets set in the    *
     * function we need to restore the original options on exit.   */
    memcpy(saveopts, opts, sizeof(opts));

    if (flags & PM_TAGGED)
	opts[XTRACE] = 1;
    opts[PRINTEXITVALUE] = 0;
    if (doshargs) {
	LinkNode node;

	node = doshargs->first;
	pparams = x = (char **) zcalloc(((sizeof *x) *
					 (1 + countlinknodes(doshargs))));
	if (isset(FUNCTIONARGZERO)) {
	    oargv0 = argzero;
	    argzero = ztrdup((char *) node->dat);
	}
	node = node->next;
	for (; node; node = node->next, x++)
	    *x = ztrdup((char *) node->dat);
    } else {
	pparams = (char **) zcalloc(sizeof *pparams);
	if (isset(FUNCTIONARGZERO)) {
	    oargv0 = argzero;
	    argzero = ztrdup(argzero);
	}
    }
#ifdef MAX_FUNCTION_DEPTH
    if(++funcdepth > MAX_FUNCTION_DEPTH)
    {
        zerr("maximum nested function level reached", NULL, 0);
	goto undoshfunc;
    }
#endif
    fstack.name = dupstring(name);
    fstack.prev = funcstack;
    funcstack = &fstack;

    if (prog->flags & EF_RUN) {
	Shfunc shf;

	runshfunc(prog, NULL, fstack.name);

	prog->flags &= ~EF_RUN;

	if (!(shf = (Shfunc) shfunctab->getnode(shfunctab,
						(name = fname)))) {
	    zwarn("%s: function not defined by file", name, 0);
	    if (noreturnval)
		errflag = 1;
	    else
		lastval = 1;
	    goto doneshfunc;
	}
	prog = shf->funcdef;
    }
    runshfunc(prog, wrappers, fstack.name);
 doneshfunc:
    funcstack = fstack.prev;
#ifdef MAX_FUNCTION_DEPTH
 undoshfunc:
    --funcdepth;
#endif
    if (retflag) {
	retflag = 0;
	breaks = obreaks;
    }
    freearray(pparams);
    if (oargv0) {
	zsfree(argzero);
	argzero = oargv0;
    }
    pparams = tab;
    optcind = oldoptcind;
    zoptind = oldzoptind;
    scriptname = oldscriptname;

    if (isset(LOCALOPTIONS)) {
	/* restore all shell options except PRIVILEGED and RESTRICTED */
	saveopts[PRIVILEGED] = opts[PRIVILEGED];
	saveopts[RESTRICTED] = opts[RESTRICTED];
	memcpy(opts, saveopts, sizeof(opts));
    } else {
	/* just restore a couple. */
	opts[XTRACE] = saveopts[XTRACE];
	opts[PRINTEXITVALUE] = saveopts[PRINTEXITVALUE];
	opts[LOCALOPTIONS] = saveopts[LOCALOPTIONS];
    }

    endtrapscope();

    if (trapreturn < -1)
	trapreturn++;
    if (noreturnval)
	lastval = oldlastval;
    popheap();
}

/* This finally executes a shell function and any function wrappers     *
 * defined by modules. This works by calling the wrapper function which *
 * in turn has to call back this function with the arguments it gets.   */

/**/
mod_export void
runshfunc(Eprog prog, FuncWrap wrap, char *name)
{
    int cont;
    VARARR(char, ou, underscoreused);

    memcpy(ou, underscore, underscoreused);

    while (wrap) {
	wrap->module->wrapper++;
	cont = wrap->handler(prog, wrap->next, name);
	wrap->module->wrapper--;

	if (!wrap->module->wrapper &&
	    (wrap->module->flags & MOD_UNLOAD))
	    unload_module(wrap->module, NULL);

	if (!cont)
	    return;
	wrap = wrap->next;
    }
    startparamscope();
    execode(prog, 1, 0);
    setunderscore(ou);
    endparamscope();
}

/* Search fpath for an undefined function.  Finds the file, and returns the *
 * list of its contents.                                                    */

/**/
Eprog
getfpfunc(char *s, int *ksh)
{
    char **pp, buf[PATH_MAX];
    off_t len;
    char *d;
    Eprog r;
    int fd;

    pp = fpath;
    for (; *pp; pp++) {
	if (strlen(*pp) + strlen(s) + 1 >= PATH_MAX)
	    continue;
	if (**pp)
	    sprintf(buf, "%s/%s", *pp, s);
	else
	    strcpy(buf, s);
	if ((r = try_dump_file(*pp, s, buf, ksh)))
	    return r;
	unmetafy(buf, NULL);
	if (!access(buf, R_OK) && (fd = open(buf, O_RDONLY | O_NOCTTY)) != -1) {
	    if ((len = lseek(fd, 0, 2)) != -1) {
		d = (char *) zalloc(len + 1);
		lseek(fd, 0, 0);
		if (read(fd, d, len) == len) {
		    char *oldscriptname = scriptname;

		    close(fd);
		    d[len] = '\0';
		    d = metafy(d, len, META_REALLOC);

		    scriptname = dupstring(s);
		    r = parse_string(d, 1);
		    scriptname = oldscriptname;

		    zfree(d, len + 1);

		    return r;
		} else
		    close(fd);

		zfree(d, len + 1);
	    } else
		close(fd);
	}
    }
    return &dummy_eprog;
}

/* Handle the most common type of ksh-style autoloading, when doing a      *
 * zsh-style autoload.  Given the list read from an autoload file, and the *
 * name of the function being defined, check to see if the file consists   *
 * entirely of a single definition for that function.  If so, use the      *
 * contents of that definition.  Otherwise, use the entire file.           */

/**/
Eprog
stripkshdef(Eprog prog, char *name)
{
    Wordcode pc = prog->prog;
    wordcode code;

    if (!prog)
	return NULL;
    code = *pc++;
    if (wc_code(code) != WC_LIST ||
	(WC_LIST_TYPE(code) & (Z_SYNC|Z_END|Z_SIMPLE)) != (Z_SYNC|Z_END|Z_SIMPLE))
	return prog;
    pc++;
    code = *pc++;
    if (wc_code(code) != WC_FUNCDEF ||
	*pc != 1 || strcmp(name, ecrawstr(prog, pc + 1, NULL)))
	return prog;

    {
	Eprog ret;
	Wordcode end = pc + WC_FUNCDEF_SKIP(code);
	int sbeg = pc[2], nstrs = pc[3], nprg, npats = pc[4], plen, len, i;
	Patprog *pp;

	pc += 5;

	nprg = end - pc;
	plen = nprg * sizeof(wordcode);
	len = plen + (npats * sizeof(Patprog)) + nstrs;

	if (prog->flags & EF_MAP) {
	    ret = prog;
	    free(prog->pats);
	    ret->pats = pp = (Patprog *) zalloc(npats * sizeof(Patprog));
	    ret->prog = pc;
	    ret->strs = prog->strs + sbeg;
	} else {
	    ret = (Eprog) zhalloc(sizeof(*ret));
	    ret->flags = EF_HEAP;
	    ret->pats = pp = (Patprog *) zhalloc(len);
	    ret->prog = (Wordcode) (ret->pats + npats);
	    ret->strs = (char *) (ret->prog + nprg);
	    memcpy(ret->prog, pc, plen);
	    memcpy(ret->strs, prog->strs + sbeg, nstrs);
	    ret->dump = NULL;
	}
	ret->len = len;
	ret->npats = npats;
	for (i = npats; i--; pp++)
	    *pp = dummy_patprog1;
	ret->shf = NULL;

	return ret;
    }
}

/* check to see if AUTOCD applies here */

/**/
static char *
cancd(char *s)
{
    int nocdpath = s[0] == '.' &&
    (s[1] == '/' || !s[1] || (s[1] == '.' && (s[2] == '/' || !s[1])));
    char *t;

    if (*s != '/') {
	char sbuf[PATH_MAX], **cp;

	if (cancd2(s))
	    return s;
	if (access(unmeta(s), X_OK) == 0)
	    return NULL;
	if (!nocdpath)
	    for (cp = cdpath; *cp; cp++) {
		if (strlen(*cp) + strlen(s) + 1 >= PATH_MAX)
		    continue;
		if (**cp)
		    sprintf(sbuf, "%s/%s", *cp, s);
		else
		    strcpy(sbuf, s);
		if (cancd2(sbuf)) {
		    doprintdir = -1;
		    return dupstring(sbuf);
		}
	    }
	if ((t = cd_able_vars(s))) {
	    if (cancd2(t)) {
		doprintdir = -1;
		return t;
	    }
	}
	return NULL;
    }
    return cancd2(s) ? s : NULL;
}

/**/
static int
cancd2(char *s)
{
    struct stat buf;
    char *us, *us2 = NULL;
    int ret;

    /*
     * If CHASEDOTS and CHASELINKS are not set, we want to rationalize the
     * path by removing foo/.. combinations in the logical rather than
     * the physical path.  If either is set, we test the physical path.
     */
    if (!isset(CHASEDOTS) && !isset(CHASELINKS)) {
	if (*s != '/')
	    us = tricat(pwd[1] ? pwd : "", "/", s);
	else
	    us = ztrdup(s);
	fixdir(us2 = us);
    } else
	us = unmeta(s);
    ret = !(access(us, X_OK) || stat(us, &buf) || !S_ISDIR(buf.st_mode));
    if (us2)
	free(us2);
    return ret;
}

/**/
void
execsave(void)
{
    struct execstack *es;

    es = (struct execstack *) malloc(sizeof(struct execstack));
    es->args = args;
    es->list_pipe_pid = list_pipe_pid;
    es->nowait = nowait;
    es->pline_level = pline_level;
    es->list_pipe_child = list_pipe_child;
    es->list_pipe_job = list_pipe_job;
    strcpy(es->list_pipe_text, list_pipe_text);
    es->lastval = lastval;
    es->noeval = noeval;
    es->badcshglob = badcshglob;
    es->cmdoutpid = cmdoutpid;
    es->cmdoutval = cmdoutval;
    es->trapreturn = trapreturn;
    es->noerrs = noerrs;
    es->subsh_close = subsh_close;
    es->underscore = ztrdup(underscore);
    es->next = exstack;
    exstack = es;
    noerrs = cmdoutpid = 0;
}

/**/
void
execrestore(void)
{
    struct execstack *en;

    DPUTS(!exstack, "BUG: execrestore() without execsave()");
    args = exstack->args;
    list_pipe_pid = exstack->list_pipe_pid;
    nowait = exstack->nowait;
    pline_level = exstack->pline_level;
    list_pipe_child = exstack->list_pipe_child;
    list_pipe_job = exstack->list_pipe_job;
    strcpy(list_pipe_text, exstack->list_pipe_text);
    lastval = exstack->lastval;
    noeval = exstack->noeval;
    badcshglob = exstack->badcshglob;
    cmdoutpid = exstack->cmdoutpid;
    cmdoutval = exstack->cmdoutval;
    trapreturn = exstack->trapreturn;
    noerrs = exstack->noerrs;
    subsh_close = exstack->subsh_close;
    setunderscore(exstack->underscore);
    zsfree(exstack->underscore);
    en = exstack->next;
    free(exstack);
    exstack = en;
}
