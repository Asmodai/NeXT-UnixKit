/*
 * jobs.c - job control
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
#include "jobs.pro"

/* the process group of the shell */

/**/
mod_export pid_t mypgrp;
 
/* the job we are working on */
 
/**/
mod_export int thisjob;

/* the current job (+) */
 
/**/
mod_export int curjob;
 
/* the previous job (-) */
 
/**/
mod_export int prevjob;
 
/* the job table */
 
/**/
mod_export struct job jobtab[MAXJOB];

/* shell timings */
 
/**/
struct tms shtms;
 
/* 1 if ttyctl -f has been executed */
 
/**/
int ttyfrozen;

/* Previous values of errflag and breaks if the signal handler had to
 * change them. And a flag saying if it did that. */

/**/
int prev_errflag, prev_breaks, errbrk_saved;

static struct timeval dtimeval, now;

/**/
int numpipestats, pipestats[MAX_PIPESTATS];

/* Diff two timevals for elapsed-time computations */

/**/
static struct timeval *
dtime(struct timeval *dt, struct timeval *t1, struct timeval *t2)
{
    dt->tv_sec = t2->tv_sec - t1->tv_sec;
    dt->tv_usec = t2->tv_usec - t1->tv_usec;
    if (dt->tv_usec < 0) {
	dt->tv_usec += 1000000.0;
	dt->tv_sec -= 1.0;
    }
    return dt;
}

/* change job table entry from stopped to running */

/**/
void
makerunning(Job jn)
{
    Process pn;

    jn->stat &= ~STAT_STOPPED;
    for (pn = jn->procs; pn; pn = pn->next)
#if 0
	if (WIFSTOPPED(pn->status) && 
	    (!(jn->stat & STAT_SUPERJOB) || pn->next))
	    pn->status = SP_RUNNING;
#endif
        if (WIFSTOPPED(pn->status))
	    pn->status = SP_RUNNING;

    if (jn->stat & STAT_SUPERJOB)
	makerunning(jobtab + jn->other);
}

/* Find process and job associated with pid.         *
 * Return 1 if search was successful, else return 0. */

/**/
int
findproc(pid_t pid, Job *jptr, Process *pptr)
{
    Process pn;
    int i;

    for (i = 1; i < MAXJOB; i++)
	for (pn = jobtab[i].procs; pn; pn = pn->next)
	    if (pn->pid == pid) {
		*pptr = pn;
		*jptr = jobtab + i;
		return 1;
	    }

    return 0;
}

/* Find the super-job of a sub-job. */

/**/
static int
super_job(int sub)
{
    int i;

    for (i = 1; i < MAXJOB; i++)
	if ((jobtab[i].stat & STAT_SUPERJOB) &&
	    jobtab[i].other == sub &&
	    jobtab[i].gleader)
	    return i;
    return 0;
}

/**/
static int
handle_sub(int job, int fg)
{
    Job jn = jobtab + job, sj = jobtab + jn->other;

    if ((sj->stat & STAT_DONE) || !sj->procs) {
	struct process *p;
		    
	for (p = sj->procs; p; p = p->next)
	    if (WIFSIGNALED(p->status)) {
		if (jn->gleader != mypgrp && jn->procs->next)
		    killpg(jn->gleader, WTERMSIG(p->status));
		else
		    kill(jn->procs->pid, WTERMSIG(p->status));
		kill(sj->other, SIGCONT);
		kill(sj->other, WTERMSIG(p->status));
		break;
	    }
	if (!p) {
	    int cp;

	    jn->stat &= ~STAT_SUPERJOB;
	    jn->stat |= STAT_WASSUPER;

	    if ((cp = ((WIFEXITED(jn->procs->status) ||
			WIFSIGNALED(jn->procs->status)) &&
		       killpg(jn->gleader, 0) == -1))) {
		Process p;
		for (p = jn->procs; p->next; p = p->next);
		jn->gleader = p->pid;
	    }
	    /* This deleted the job too early if the parent
	       shell waited for a command in a list that will
	       be executed by the sub-shell (e.g.: if we have
	       `ls|if true;then sleep 20;cat;fi' and ^Z the
	       sleep, the rest will be executed by a sub-shell,
	       but the parent shell gets notified for the
	       sleep.
	       deletejob(sj); */
	    /* If this super-job contains only the sub-shell,
	       we have to attach the tty to its process group
	       now. */
	    if ((fg || thisjob == job) &&
		(!jn->procs->next || cp || jn->procs->pid != jn->gleader))
		attachtty(jn->gleader);
	    kill(sj->other, SIGCONT);
	}
	curjob = jn - jobtab;
    } else if (sj->stat & STAT_STOPPED) {
	struct process *p;

	jn->stat |= STAT_STOPPED;
	for (p = jn->procs; p; p = p->next)
	    if (p->status == SP_RUNNING ||
		(!WIFEXITED(p->status) && !WIFSIGNALED(p->status)))
		p->status = sj->procs->status;
	curjob = jn - jobtab;
	printjob(jn, !!isset(LONGLISTJOBS), 1);
	return 1;
    }
    return 0;
}

/* Update status of process that we have just WAIT'ed for */

/**/
void
update_process(Process pn, int status)
{
    struct timezone dummy_tz;
    long childs, childu;

    childs = shtms.tms_cstime;
    childu = shtms.tms_cutime;
    times(&shtms);                          /* get time-accounting info          */

    pn->status = status;                    /* save the status returned by WAIT  */
    pn->ti.st  = shtms.tms_cstime - childs; /* compute process system space time */
    pn->ti.ut  = shtms.tms_cutime - childu; /* compute process user space time   */

    gettimeofday(&pn->endtime, &dummy_tz);  /* record time process exited        */
}

/* Update status of job, possibly printing it */

/**/
void
update_job(Job jn)
{
    Process pn;
    int job;
    int val = 0, status = 0;
    int somestopped = 0, inforeground = 0;

    for (pn = jn->procs; pn; pn = pn->next) {
	if (pn->status == SP_RUNNING)      /* some processes in this job are running       */
	    return;                        /* so no need to update job table entry         */
	if (WIFSTOPPED(pn->status))        /* some processes are stopped                   */
	    somestopped = 1;               /* so job is not done, but entry needs updating */
	if (!pn->next)                     /* last job in pipeline determines exit status  */
	    val = (WIFSIGNALED(pn->status)) ? 0200 | WTERMSIG(pn->status) :
		WEXITSTATUS(pn->status);
	if (pn->pid == jn->gleader)        /* if this process is process group leader      */
	    status = pn->status;
    }

    job = jn - jobtab;   /* compute job number */

    if (somestopped) {
	if (jn->stty_in_env && !jn->ty) {
	    jn->ty = (struct ttyinfo *) zalloc(sizeof(struct ttyinfo));
	    gettyinfo(jn->ty);
	}
	if (jn->stat & STAT_STOPPED) {
	    if (jn->stat & STAT_SUBJOB) {
		/* If we have `cat foo|while read a; grep $a bar;done'
		 * and have hit ^Z, the sub-job is stopped, but the
		 * super-job may still be running, waiting to be stopped
		 * or to exit. So we have to send it a SIGTSTP. */
		int i;

		if ((i = super_job(job)))
		    killpg(jobtab[i].gleader, SIGTSTP);
	    }
	    return;
	}
    }
    {                   /* job is done or stopped, remember return value */
	lastval2 = val;
	/* If last process was run in the current shell, keep old status
	 * and let it handle its own traps, but always allow the test
	 * for the pgrp.
	 */
	if (jn->stat & STAT_CURSH)
	    inforeground = 1;
	else if (job == thisjob) {
	    lastval = val;
	    inforeground = 2;
	}
    }

    if (shout && !ttyfrozen && !jn->stty_in_env && !zleactive &&
	job == thisjob && !somestopped && !(jn->stat & STAT_NOSTTY))
	gettyinfo(&shttyinfo);

    if (isset(MONITOR)) {
	pid_t pgrp = gettygrp();           /* get process group of tty      */

	/* is this job in the foreground of an interactive shell? */
	if (mypgrp != pgrp && inforeground &&
	    (jn->gleader == pgrp || (pgrp > 1 && kill(-pgrp, 0) == -1))) {
	    if (list_pipe) {
		if (somestopped || (pgrp > 1 && kill(-pgrp, 0) == -1)) {
		    attachtty(mypgrp);
		    /* check window size and adjust if necessary */
		    adjustwinsize(0);
		} else {
		    /*
		     * Oh, dear, we're right in the middle of some confusion
		     * of shell jobs on the righthand side of a pipeline, so
		     * it's death to call attachtty() just yet.  Mark the
		     * fact in the job, so that the attachtty() will be called
		     * when the job is finally deleted.
		     */
		    jn->stat |= STAT_ATTACH;
		}
		/* If we have `foo|while true; (( x++ )); done', and hit
		 * ^C, we have to stop the loop, too. */
		if ((val & 0200) && inforeground == 1) {
		    if (!errbrk_saved) {
			errbrk_saved = 1;
			prev_breaks = breaks;
			prev_errflag = errflag;
		    }
		    breaks = loops;
		    errflag = 1;
		    inerrflush();
		}
	    } else {
		attachtty(mypgrp);
		/* check window size and adjust if necessary */
		adjustwinsize(0);
	    }
	}
    } else if (list_pipe && (val & 0200) && inforeground == 1) {
	if (!errbrk_saved) {
	    errbrk_saved = 1;
	    prev_breaks = breaks;
	    prev_errflag = errflag;
	}
	breaks = loops;
	errflag = 1;
	inerrflush();
    }
    if (somestopped && jn->stat & STAT_SUPERJOB)
	return;
    jn->stat |= (somestopped) ? STAT_CHANGED | STAT_STOPPED :
	STAT_CHANGED | STAT_DONE;
    if (job == thisjob && (jn->stat & STAT_DONE)) {
	int i;
	Process p;

	for (p = jn->procs, i = 0; p && i < MAX_PIPESTATS; p = p->next, i++)
	    pipestats[i] = ((WIFSIGNALED(p->status)) ?
			    0200 | WTERMSIG(p->status) :
			    WEXITSTATUS(p->status));
	if ((jn->stat & STAT_CURSH) && i < MAX_PIPESTATS)
	    pipestats[i++] = lastval;
	numpipestats = i;
    }
    if (!inforeground &&
	(jn->stat & (STAT_SUBJOB | STAT_DONE)) == (STAT_SUBJOB | STAT_DONE)) {
	int su;

	if ((su = super_job(jn - jobtab)))
	    handle_sub(su, 0);
    }
    if ((jn->stat & (STAT_DONE | STAT_STOPPED)) == STAT_STOPPED) {
	prevjob = curjob;
	curjob = job;
    }
    if ((isset(NOTIFY) || job == thisjob) && (jn->stat & STAT_LOCKED)) {
	printjob(jn, !!isset(LONGLISTJOBS), 0);
	if (zleactive)
	    zrefresh();
    }
    if (sigtrapped[SIGCHLD] && job != thisjob)
	dotrap(SIGCHLD);

    /* When MONITOR is set, the foreground process runs in a different *
     * process group from the shell, so the shell will not receive     *
     * terminal signals, therefore we we pretend that the shell got    *
     * the signal too.                                                 */
    if (inforeground == 2 && isset(MONITOR) && WIFSIGNALED(status)) {
	int sig = WTERMSIG(status);

	if (sig == SIGINT || sig == SIGQUIT) {
	    if (sigtrapped[sig]) {
		dotrap(sig);
		/* We keep the errflag as set or not by dotrap.
		 * This is to fulfil the promise to carry on
		 * with the jobs if trap returns zero.
		 * Setting breaks = loops ensures a consistent return
		 * status if inside a loop.  Maybe the code in loops
		 * should be changed.
		 */
		if (errflag)
		    breaks = loops;
	    } else {
		breaks = loops;
		errflag = 1;
	    }
	}
    }
}

/* set the previous job to something reasonable */

/**/
static void
setprevjob(void)
{
    int i;

    for (i = MAXJOB - 1; i; i--)
	if ((jobtab[i].stat & STAT_INUSE) && (jobtab[i].stat & STAT_STOPPED) &&
	    !(jobtab[i].stat & STAT_SUBJOB) && i != curjob && i != thisjob) {
	    prevjob = i;
	    return;
	}

    for (i = MAXJOB - 1; i; i--)
	if ((jobtab[i].stat & STAT_INUSE) && !(jobtab[i].stat & STAT_SUBJOB) &&
	    i != curjob && i != thisjob) {
	    prevjob = i;
	    return;
	}

    prevjob = -1;
}

static long clktck = 0;

/**/
static void
set_clktck(void)
{
#ifdef _SC_CLK_TCK
    if (!clktck)
	/* fetch clock ticks per second from *
	 * sysconf only the first time       */
	clktck = sysconf(_SC_CLK_TCK);
#else
# ifdef __NeXT__
    /* NeXTStep 3.3 defines CLK_TCK wrongly */
    clktck = 60;
# else
#  ifdef CLK_TCK
    clktck = CLK_TCK;
#  else
#   ifdef HZ
     clktck = HZ;
#   else
     clktck = 60;
#   endif
#  endif
# endif
#endif
}

/**/
static void
printhhmmss(double secs)
{
    int mins = (int) secs / 60;
    int hours = mins / 60;

    secs -= 60 * mins;
    mins -= 60 * hours;
    if (hours)
	fprintf(stderr, "%d:%02d:%05.2f", hours, mins, secs);
    else if (mins)
	fprintf(stderr,      "%d:%05.2f",        mins, secs);
    else
	fprintf(stderr,           "%.3f",              secs);
}

/**/
static void
printtime(struct timeval *real, struct timeinfo *ti, char *desc)
{
    char *s;
    double elapsed_time, user_time, system_time;
    int percent;

    if (!desc)
	desc = "";

    set_clktck();
    /* go ahead and compute these, since almost every TIMEFMT will have them */
    elapsed_time = real->tv_sec + real->tv_usec / 1000000.0;
    user_time    = ti->ut / (double) clktck;
    system_time  = ti->st / (double) clktck;
    percent      =  100.0 * (ti->ut + ti->st)
	/ (clktck * real->tv_sec + clktck * real->tv_usec / 1000000.0);

    queue_signals();
    if (!(s = getsparam("TIMEFMT")))
	s = DEFAULT_TIMEFMT;

    for (; *s; s++)
	if (*s == '%')
	    switch (*++s) {
	    case 'E':
		fprintf(stderr, "%4.2fs", elapsed_time);
		break;
	    case 'U':
		fprintf(stderr, "%4.2fs", user_time);
		break;
	    case 'S':
		fprintf(stderr, "%4.2fs", system_time);
		break;
	    case '*':
		switch (*++s) {
		case 'E':
		    printhhmmss(elapsed_time);
		    break;
		case 'U':
		    printhhmmss(user_time);
		    break;
		case 'S':
		    printhhmmss(system_time);
		    break;
		default:
		    fprintf(stderr, "%%*");
		    s--;
		    break;
		}
		break;
	    case 'P':
		fprintf(stderr, "%d%%", percent);
		break;
	    case 'J':
		fprintf(stderr, "%s", desc);
		break;
	    case '%':
		putc('%', stderr);
		break;
	    case '\0':
		s--;
		break;
	    default:
		fprintf(stderr, "%%%c", *s);
		break;
	} else
	    putc(*s, stderr);
    unqueue_signals();
    putc('\n', stderr);
    fflush(stderr);
}

/**/
static void
dumptime(Job jn)
{
    Process pn;

    if (!jn->procs)
	return;
    for (pn = jn->procs; pn; pn = pn->next)
	printtime(dtime(&dtimeval, &pn->bgtime, &pn->endtime), &pn->ti, pn->text);
}

/* Check whether shell should report the amount of time consumed   *
 * by job.  This will be the case if we have preceded the command  *
 * with the keyword time, or if REPORTTIME is non-negative and the *
 * amount of time consumed by the job is greater than REPORTTIME   */

/**/
static int
should_report_time(Job j)
{
    struct value vbuf;
    Value v;
    char *s = "REPORTTIME";
    int reporttime;

    /* if the time keyword was used */
    if (j->stat & STAT_TIMED)
	return 1;

    queue_signals();
    if (!(v = getvalue(&vbuf, &s, 0)) ||
	(reporttime = getintvalue(v)) < 0) {
	unqueue_signals();
	return 0;
    }
    unqueue_signals();
    /* can this ever happen? */
    if (!j->procs)
	return 0;

    set_clktck();
    return ((j->procs->ti.ut + j->procs->ti.st) / clktck >= reporttime);
}

/* !(lng & 3) means jobs    *
 *  (lng & 1) means jobs -l *
 *  (lng & 2) means jobs -p
 *  (lng & 4) means jobs -d
 *
 * synch = 0 means asynchronous
 * synch = 1 means synchronous
 * synch = 2 means called synchronously from jobs
*/

/**/
void
printjob(Job jn, int lng, int synch)
{
    Process pn;
    int job = jn - jobtab, len = 9, sig, sflag = 0, llen;
    int conted = 0, lineleng = columns, skip = 0, doputnl = 0;
    FILE *fout = (synch == 2) ? stdout : shout;

    if (jn->stat & STAT_NOPRINT)
	return;

    if (lng < 0) {
	conted = 1;
	lng = 0;
    }

/* find length of longest signame, check to see */
/* if we really need to print this job          */

    for (pn = jn->procs; pn; pn = pn->next) {
	if (jn->stat & STAT_SUPERJOB &&
	    jn->procs->status == SP_RUNNING && !pn->next)
	    pn->status = SP_RUNNING;
	if (pn->status != SP_RUNNING) {
	    if (WIFSIGNALED(pn->status)) {
		sig = WTERMSIG(pn->status);
		llen = strlen(sigmsg(sig));
		if (WCOREDUMP(pn->status))
		    llen += 14;
		if (llen > len)
		    len = llen;
		if (sig != SIGINT && sig != SIGPIPE)
		    sflag = 1;
		if (job == thisjob && sig == SIGINT)
		    doputnl = 1;
	    } else if (WIFSTOPPED(pn->status)) {
		sig = WSTOPSIG(pn->status);
		if ((int)strlen(sigmsg(sig)) > len)
		    len = strlen(sigmsg(sig));
		if (job == thisjob && sig == SIGTSTP)
		    doputnl = 1;
	    } else if (isset(PRINTEXITVALUE) && isset(SHINSTDIN) &&
		       WEXITSTATUS(pn->status))
		sflag = 1;
	}
    }

/* print if necessary */

    if (interact && jobbing && ((jn->stat & STAT_STOPPED) || sflag ||
				job != thisjob)) {
	int len2, fline = 1;
	Process qn;

	if (!synch)
	    trashzle();
	if (doputnl && !synch)
	    putc('\n', fout);
	for (pn = jn->procs; pn;) {
	    len2 = ((job == thisjob) ? 5 : 10) + len;	/* 2 spaces */
	    if (lng & 3)
		qn = pn->next;
	    else
		for (qn = pn->next; qn; qn = qn->next) {
		    if (qn->status != pn->status)
			break;
		    if ((int)strlen(qn->text) + len2 + ((qn->next) ? 3 : 0) > lineleng)
			break;
		    len2 += strlen(qn->text) + 2;
		}
	    if (job != thisjob) {
		if (fline)
		    fprintf(fout, "[%ld]  %c ",
			    (long)(jn - jobtab),
			    (job == curjob) ? '+'
			    : (job == prevjob) ? '-' : ' ');
		else
		    fprintf(fout, (job > 9) ? "        " : "       ");
	    } else
		fprintf(fout, "zsh: ");
	    if (lng & 1)
		fprintf(fout, "%ld ", (long) pn->pid);
	    else if (lng & 2) {
		pid_t x = jn->gleader;

		fprintf(fout, "%ld ", (long) x);
		do
		    skip++;
		while ((x /= 10));
		skip++;
		lng &= ~3;
	    } else
		fprintf(fout, "%*s", skip, "");
	    if (pn->status == SP_RUNNING) {
		if (!conted)
		    fprintf(fout, "running%*s", len - 7 + 2, "");
		else
		    fprintf(fout, "continued%*s", len - 9 + 2, "");
	    }
	    else if (WIFEXITED(pn->status)) {
		if (WEXITSTATUS(pn->status))
		    fprintf(fout, "exit %-4d%*s", WEXITSTATUS(pn->status),
			    len - 9 + 2, "");
		else
		    fprintf(fout, "done%*s", len - 4 + 2, "");
	    } else if (WIFSTOPPED(pn->status))
		fprintf(fout, "%-*s", len + 2, sigmsg(WSTOPSIG(pn->status)));
	    else if (WCOREDUMP(pn->status))
		fprintf(fout, "%s (core dumped)%*s",
			sigmsg(WTERMSIG(pn->status)),
			(int)(len - 14 + 2 - strlen(sigmsg(WTERMSIG(pn->status)))), "");
	    else
		fprintf(fout, "%-*s", len + 2, sigmsg(WTERMSIG(pn->status)));
	    for (; pn != qn; pn = pn->next)
		fprintf(fout, (pn->next) ? "%s | " : "%s", pn->text);
	    putc('\n', fout);
	    fline = 0;
	}
	fflush(fout);
    } else if (doputnl && interact && !synch) {
	putc('\n', fout);
	fflush(fout);
    }

/* print "(pwd now: foo)" messages: with (lng & 4) we are printing
 * the directory where the job is running, otherwise the current directory
 */

    if ((lng & 4) || (interact && job == thisjob &&
		      jn->pwd && strcmp(jn->pwd, pwd))) {
	fprintf(shout, "(pwd %s: ", (lng & 4) ? "" : "now");
	fprintdir(((lng & 4) && jn->pwd) ? jn->pwd : pwd, shout);
	fprintf(shout, ")\n");
	fflush(shout);
    }
/* delete job if done */

    if (jn->stat & STAT_DONE) {
	if (should_report_time(jn))
	    dumptime(jn);
	deletejob(jn);
	if (job == curjob) {
	    curjob = prevjob;
	    prevjob = job;
	}
	if (job == prevjob)
	    setprevjob();
    } else
	jn->stat &= ~STAT_CHANGED;
}

/**/
void
deletefilelist(LinkList file_list)
{
    char *s;
    if (file_list) {
	while ((s = (char *)getlinknode(file_list))) {
	    unlink(s);
	    zsfree(s);
	}
	zfree(file_list, sizeof(struct linklist));
    }
}

/**/
void
deletejob(Job jn)
{
    struct process *pn, *nx;

    if (jn->stat & STAT_ATTACH) {
	attachtty(mypgrp);
	adjustwinsize(0);
    }

    pn = jn->procs;
    jn->procs = NULL;
    for (; pn; pn = nx) {
	nx = pn->next;
	zfree(pn, sizeof(struct process));
    }
    deletefilelist(jn->filelist);

    if (jn->ty)
	zfree(jn->ty, sizeof(struct ttyinfo));
    if (jn->pwd)
	zsfree(jn->pwd);
    jn->pwd = NULL;
    if (jn->stat & STAT_WASSUPER)
	deletejob(jobtab + jn->other);
    jn->gleader = jn->other = 0;
    jn->stat = jn->stty_in_env = 0;
    jn->procs = NULL;
    jn->filelist = NULL;
    jn->ty = NULL;
}

/* add a process to the current job */

/**/
void
addproc(pid_t pid, char *text)
{
    Process pn;
    struct timezone dummy_tz;

    pn = (Process) zcalloc(sizeof *pn);
    pn->pid = pid;
    if (text)
	strcpy(pn->text, text);
    else
	*pn->text = '\0';
    gettimeofday(&pn->bgtime, &dummy_tz);
    pn->status = SP_RUNNING;
    pn->next = NULL;

    /* if this is the first process we are adding to *
     * the job, then it's the group leader.          */
    if (!jobtab[thisjob].gleader)
	jobtab[thisjob].gleader = pid;

    /* attach this process to end of process list of current job */
    if (jobtab[thisjob].procs) {
	Process n;

	for (n = jobtab[thisjob].procs; n->next; n = n->next);
	pn->next = NULL;
	n->next = pn;
    } else {
	/* first process for this job */
	jobtab[thisjob].procs = pn;
    }
    /* If the first process in the job finished before any others were *
     * added, maybe STAT_DONE got set incorrectly.  This can happen if *
     * a $(...) was waited for and the last existing job in the        *
     * pipeline was already finished.  We need to be very careful that *
     * there was no call to printjob() between then and now, else      *
     * the job will already have been deleted from the table.          */
    jobtab[thisjob].stat &= ~STAT_DONE;
}

/* Check if we have files to delete.  We need to check this to see *
 * if it's all right to exec a command without forking in the last *
 * component of subshells or after the `-c' option.                */

/**/
int
havefiles(void)
{
    int i;

    for (i = 1; i < MAXJOB; i++)
	if (jobtab[i].stat && jobtab[i].filelist)
	    return 1;
    return 0;

}

/* wait for a particular process */

/**/
void
waitforpid(pid_t pid)
{
    int first = 1, q = queue_signal_level();

    /* child_block() around this loop in case #ifndef WNOHANG */
    dont_queue_signals();
    child_block();		/* unblocked in child_suspend() */
    while (!errflag && (kill(pid, 0) >= 0 || errno != ESRCH)) {
	if (first)
	    first = 0;
	else
	    kill(pid, SIGCONT);

	child_suspend(SIGINT);
	child_block();
    }
    child_unblock();
    restore_queue_signals(q);
}

/* wait for a job to finish */

/**/
static void
zwaitjob(int job, int sig)
{
    int q = queue_signal_level();
    Job jn = jobtab + job;

    dont_queue_signals();
    child_block();		 /* unblocked during child_suspend() */
    if (jn->procs) {		 /* if any forks were done         */
	jn->stat |= STAT_LOCKED;
	if (jn->stat & STAT_CHANGED)
	    printjob(jn, !!isset(LONGLISTJOBS), 1);
	while (!errflag && jn->stat &&
	       !(jn->stat & STAT_DONE) &&
	       !(interact && (jn->stat & STAT_STOPPED))) {
	    child_suspend(sig);
	    /* Commenting this out makes ^C-ing a job started by a function
	       stop the whole function again.  But I guess it will stop
	       something else from working properly, we have to find out
	       what this might be.  --oberon

	    errflag = 0; */
	    if (subsh) {
		killjb(jn, SIGCONT);
		jn->stat &= ~STAT_STOPPED;
	    }
	    if (jn->stat & STAT_SUPERJOB)
		if (handle_sub(jn - jobtab, 1))
		    break;
	    child_block();
	}
    } else {
	deletejob(jn);
	pipestats[0] = lastval;
	numpipestats = 1;
    }
    child_unblock();
    restore_queue_signals(q);
}

/* wait for running job to finish */

/**/
void
waitjobs(void)
{
    Job jn = jobtab + thisjob;

    if (jn->procs)
	zwaitjob(thisjob, 0);
    else {
	deletejob(jn);
	pipestats[0] = lastval;
	numpipestats = 1;
    }
    thisjob = -1;
}

/* clear job table when entering subshells */

/**/
mod_export void
clearjobtab(void)
{
    int i;

    for (i = 1; i < MAXJOB; i++)
    {
	if (jobtab[i].ty)
	    zfree(jobtab[i].ty, sizeof(struct ttyinfo));
	if (jobtab[i].pwd)
	    zsfree(jobtab[i].pwd);
    }

    memset(jobtab, 0, sizeof(jobtab)); /* zero out table */
}

/* Get a free entry in the job table and initialize it. */

/**/
int
initjob(void)
{
    int i;

    for (i = 1; i < MAXJOB; i++)
	if (!jobtab[i].stat) {
	    jobtab[i].stat = STAT_INUSE;
	    if (jobtab[i].pwd) {
		zsfree(jobtab[i].pwd);
		jobtab[i].pwd = NULL;
	    }
	    jobtab[i].gleader = 0;
	    return i;
	}

    zerr("job table full or recursion limit exceeded", NULL, 0);
    return -1;
}

/**/
void
setjobpwd(void)
{
    int i;

    for (i = 1; i < MAXJOB; i++)
	if (jobtab[i].stat && !jobtab[i].pwd)
	    jobtab[i].pwd = ztrdup(pwd);
}

/* print pids for & */

/**/
void
spawnjob(void)
{
    Process pn;

    /* if we are not in a subshell */
    if (!subsh) {
	if (curjob == -1 || !(jobtab[curjob].stat & STAT_STOPPED)) {
	    curjob = thisjob;
	    setprevjob();
	} else if (prevjob == -1 || !(jobtab[prevjob].stat & STAT_STOPPED))
	    prevjob = thisjob;
	if (interact && jobbing && jobtab[thisjob].procs) {
	    fprintf(stderr, "[%d]", thisjob);
	    for (pn = jobtab[thisjob].procs; pn; pn = pn->next)
		fprintf(stderr, " %ld", (long) pn->pid);
	    fprintf(stderr, "\n");
	    fflush(stderr);
	}
    }
    if (!jobtab[thisjob].procs)
	deletejob(jobtab + thisjob);
    else
	jobtab[thisjob].stat |= STAT_LOCKED;
    thisjob = -1;
}

/**/
void
shelltime(void)
{
    struct timeinfo ti;
    struct timezone dummy_tz;
    struct tms buf;

    times(&buf);
    ti.ut = buf.tms_utime;
    ti.st = buf.tms_stime;
    gettimeofday(&now, &dummy_tz);
    printtime(dtime(&dtimeval, &shtimer, &now), &ti, "shell");
    ti.ut = buf.tms_cutime;
    ti.st = buf.tms_cstime;
    printtime(dtime(&dtimeval, &shtimer, &now), &ti, "children");
}

/* see if jobs need printing */
 
/**/
void
scanjobs(void)
{
    int i;
 
    for (i = 1; i < MAXJOB; i++)
        if (jobtab[i].stat & STAT_CHANGED)
            printjob(jobtab + i, 0, 1);
}

/**** job control builtins ****/

/* This simple function indicates whether or not s may represent      *
 * a number.  It returns true iff s consists purely of digits and     *
 * minuses.  Note that minus may appear more than once, and the empty *
 * string will produce a `true' response.                             */

/**/
static int
isanum(char *s)
{
    while (*s == '-' || idigit(*s))
	s++;
    return *s == '\0';
}

/* Make sure we have a suitable current and previous job set. */

/**/
static void
setcurjob(void)
{
    if (curjob == thisjob ||
	(curjob != -1 && !(jobtab[curjob].stat & STAT_INUSE))) {
	curjob = prevjob;
	setprevjob();
	if (curjob == thisjob ||
	    (curjob != -1 && !((jobtab[curjob].stat & STAT_INUSE) &&
			       curjob != thisjob))) {
	    curjob = prevjob;
	    setprevjob();
	}
    }
}

/* Convert a job specifier ("%%", "%1", "%foo", "%?bar?", etc.) *
 * to a job number.                                             */

/**/
static int
getjob(char *s, char *prog)
{
    int jobnum, returnval;

    /* if there is no %, treat as a name */
    if (*s != '%')
	goto jump;
    s++;
    /* "%%", "%+" and "%" all represent the current job */
    if (*s == '%' || *s == '+' || !*s) {
	if (curjob == -1) {
	    zwarnnam(prog, "no current job", NULL, 0);
	    returnval = -1;
	    goto done;
	}
	returnval = curjob;
	goto done;
    }
    /* "%-" represents the previous job */
    if (*s == '-') {
	if (prevjob == -1) {
	    zwarnnam(prog, "no previous job", NULL, 0);
	    returnval = -1;
	    goto done;
	}
	returnval = prevjob;
	goto done;
    }
    /* a digit here means we have a job number */
    if (idigit(*s)) {
	jobnum = atoi(s);
	if (jobnum && jobnum < MAXJOB && jobtab[jobnum].stat &&
	    !(jobtab[jobnum].stat & STAT_SUBJOB) && jobnum != thisjob) {
	    returnval = jobnum;
	    goto done;
	}
	zwarnnam(prog, "%%%s: no such job", s, 0);
	returnval = -1;
	goto done;
    }
    /* "%?" introduces a search string */
    if (*s == '?') {
	struct process *pn;

	for (jobnum = MAXJOB - 1; jobnum >= 0; jobnum--)
	    if (jobtab[jobnum].stat && !(jobtab[jobnum].stat & STAT_SUBJOB) &&
		jobnum != thisjob)
		for (pn = jobtab[jobnum].procs; pn; pn = pn->next)
		    if (strstr(pn->text, s + 1)) {
			returnval = jobnum;
			goto done;
		    }
	zwarnnam(prog, "job not found: %s", s, 0);
	returnval = -1;
	goto done;
    }
  jump:
    /* anything else is a job name, specified as a string that begins the
    job's command */
    if ((jobnum = findjobnam(s)) != -1) {
	returnval = jobnum;
	goto done;
    }
    /* if we get here, it is because none of the above succeeded and went
    to done */
    zwarnnam(prog, "job not found: %s", s, 0);
    returnval = -1;
  done:
    return returnval;
}

/* For jobs -Z (which modifies the shell's name as seen in ps listings).  *
 * hackzero is the start of the safely writable space, and hackspace is   *
 * its length, excluding a final NUL terminator that will always be left. */

static char *hackzero;
static int hackspace;

/* Initialise the jobs -Z system.  The technique is borrowed from perl: *
 * check through the argument and environment space, to see how many of *
 * the strings are in contiguous space.  This determines the value of   *
 * hackspace.                                                           */

/**/
void
init_hackzero(char **argv, char **envp)
{
    char *p, *q;

    hackzero = *argv;
    p = strchr(hackzero, 0);
    while(*++argv) {
	q = *argv;
	if(q != p+1)
	    goto done;
	p = strchr(q, 0);
    }
    for(; *envp; envp++) {
	q = *envp;
	if(q != p+1)
	    goto done;
	p = strchr(q, 0);
    }
    done:
    hackspace = p - hackzero;
}

/* bg, disown, fg, jobs, wait: most of the job control commands are     *
 * here.  They all take the same type of argument.  Exception: wait can *
 * take a pid or a job specifier, whereas the others only work on jobs. */

/**/
int
bin_fg(char *name, char **argv, char *ops, int func)
{
    int job, lng, firstjob = -1, retval = 0;

    if (ops['Z']) {
	int len;

	if(isset(RESTRICTED)) {
	    zwarnnam(name, "-Z is restricted", NULL, 0);
	    return 1;
	}
	if(!argv[0] || argv[1]) {
	    zwarnnam(name, "-Z requires one argument", NULL, 0);
	    return 1;
	}
	queue_signals();
	unmetafy(*argv, &len);
	if(len > hackspace)
	    len = hackspace;
	memcpy(hackzero, *argv, len);
	memset(hackzero + len, 0, hackspace - len);
	unqueue_signals();
	return 0;
    }

    lng = (ops['l']) ? 1 : (ops['p']) ? 2 : 0;
    if (ops['d'])
	lng |= 4;
    
    if ((func == BIN_FG || func == BIN_BG) && !jobbing) {
	/* oops... maybe bg and fg should have been disabled? */
	zwarnnam(name, "no job control in this shell.", NULL, 0);
	return 1;
    }

    queue_signals();
    /* If necessary, update job table. */
    if (unset(NOTIFY))
	scanjobs();

    setcurjob();

    if (func == BIN_JOBS)
        /* If you immediately type "exit" after "jobs", this      *
         * will prevent zexit from complaining about stopped jobs */
	stopmsg = 2;
    if (!*argv) {
	/* This block handles all of the default cases (no arguments).  bg,
	fg and disown act on the current job, and jobs and wait act on all the
	jobs. */
 	if (func == BIN_FG || func == BIN_BG || func == BIN_DISOWN) {
	    /* W.r.t. the above comment, we'd better have a current job at this
	    point or else. */
	    if (curjob == -1 || (jobtab[curjob].stat & STAT_NOPRINT)) {
		zwarnnam(name, "no current job", NULL, 0);
		unqueue_signals();
		return 1;
	    }
	    firstjob = curjob;
	} else if (func == BIN_JOBS) {
	    /* List jobs. */
	    for (job = 0; job != MAXJOB; job++)
		if (job != thisjob && jobtab[job].stat) {
		    if ((!ops['r'] && !ops['s']) ||
			(ops['r'] && ops['s']) ||
			(ops['r'] && !(jobtab[job].stat & STAT_STOPPED)) ||
			(ops['s'] && jobtab[job].stat & STAT_STOPPED))
			printjob(job + jobtab, lng, 2);
		}
	    unqueue_signals();
	    return 0;
	} else {   /* Must be BIN_WAIT, so wait for all jobs */
	    for (job = 0; job != MAXJOB; job++)
		if (job != thisjob && jobtab[job].stat)
		    zwaitjob(job, SIGINT);
	    unqueue_signals();
	    return 0;
	}
    }

    /* Defaults have been handled.  We now have an argument or two, or three...
    In the default case for bg, fg and disown, the argument will be provided by
    the above routine.  We now loop over the arguments. */
    for (; (firstjob != -1) || *argv; (void)(*argv && argv++)) {
	int stopped, ocj = thisjob;

	if (func == BIN_WAIT && isanum(*argv)) {
	    /* wait can take a pid; the others can't. */
	    pid_t pid = (long)atoi(*argv);
	    Job j;
	    Process p;

	    if (findproc(pid, &j, &p))
		waitforpid(pid);
	    else
		zwarnnam(name, "pid %d is not a child of this shell", 0, pid);
	    retval = lastval2;
	    thisjob = ocj;
	    continue;
	}
	/* The only type of argument allowed now is a job spec.  Check it. */
	job = (*argv) ? getjob(*argv, name) : firstjob;
	firstjob = -1;
	if (job == -1) {
	    retval = 1;
	    break;
	}
	if (!(jobtab[job].stat & STAT_INUSE) ||
	    (jobtab[job].stat & STAT_NOPRINT)) {
	    zwarnnam(name, "no such job: %d", 0, job);
	    unqueue_signals();
	    return 1;
	}
	/* We have a job number.  Now decide what to do with it. */
	switch (func) {
	case BIN_FG:
	case BIN_BG:
	case BIN_WAIT:
	    if (func == BIN_BG)
		jobtab[job].stat |= STAT_NOSTTY;
	    if ((stopped = (jobtab[job].stat & STAT_STOPPED)))
		makerunning(jobtab + job);
	    else if (func == BIN_BG) {
		/* Silly to bg a job already running. */
		zwarnnam(name, "job already in background", NULL, 0);
		thisjob = ocj;
		unqueue_signals();
		return 1;
	    }
	    /* It's time to shuffle the jobs around!  Reset the current job,
	    and pick a sensible secondary job. */
	    if (curjob == job) {
		curjob = prevjob;
		prevjob = (func == BIN_BG) ? -1 : job;
	    }
	    if (prevjob == job || prevjob == -1)
		setprevjob();
	    if (curjob == -1) {
		curjob = prevjob;
		setprevjob();
	    }
	    if (func != BIN_WAIT)
		/* for bg and fg -- show the job we are operating on */
		printjob(jobtab + job, (stopped) ? -1 : 0, 1);
	    if (func != BIN_BG) {		/* fg or wait */
		if (jobtab[job].pwd && strcmp(jobtab[job].pwd, pwd)) {
		    fprintf(shout, "(pwd : ");
		    fprintdir(jobtab[job].pwd, shout);
		    fprintf(shout, ")\n");
		}
		fflush(shout);
		if (func != BIN_WAIT) {		/* fg */
		    thisjob = job;
		    if ((jobtab[job].stat & STAT_SUPERJOB) &&
			((!jobtab[job].procs->next ||
			  (jobtab[job].stat & STAT_SUBLEADER) ||
			  killpg(jobtab[job].gleader, 0) == -1)) &&
			jobtab[jobtab[job].other].gleader)
			attachtty(jobtab[jobtab[job].other].gleader);
		    else
			attachtty(jobtab[job].gleader);
		}
	    }
	    if (stopped) {
		if (func != BIN_BG && jobtab[job].ty)
		    settyinfo(jobtab[job].ty);
		killjb(jobtab + job, SIGCONT);
	    }
	    if (func == BIN_WAIT)
		zwaitjob(job, SIGINT);
	    if (func != BIN_BG) {
		waitjobs();
		retval = lastval2;
	    }
	    break;
	case BIN_JOBS:
	    printjob(job + jobtab, lng, 2);
	    break;
	case BIN_DISOWN:
	    if (jobtab[job].stat & STAT_STOPPED)
                zwarnnam(name,
#ifdef USE_SUSPENDED
                         "warning: job is suspended",
#else
                         "warning: job is stopped",
#endif
                         NULL, 0);
	    deletejob(jobtab + job);
	    break;
	}
	thisjob = ocj;
    }
    unqueue_signals();
    return retval;
}

/* kill: send a signal to a process.  The process(es) may be specified *
 * by job specifier (see above) or pid.  A signal, defaulting to       *
 * SIGTERM, may be specified by name or number, preceded by a dash.    */

/**/
int
bin_kill(char *nam, char **argv, char *ops, int func)
{
    int sig = SIGTERM;
    int returnval = 0;

    /* check for, and interpret, a signal specifier */
    if (*argv && **argv == '-') {
	if (idigit((*argv)[1]))
	    /* signal specified by number */
	    sig = atoi(*argv + 1);
	else if ((*argv)[1] != '-' || (*argv)[2]) {
	    char *signame;

	    /* with argument "-l" display the list of signal names */
	    if ((*argv)[1] == 'l' && (*argv)[2] == '\0') {
		if (argv[1]) {
		    while (*++argv) {
			sig = zstrtol(*argv, &signame, 10);
			if (signame == *argv) {
			    for (sig = 1; sig <= SIGCOUNT; sig++)
				if (!cstrpcmp(sigs + sig, &signame))
				    break;
			    if (sig > SIGCOUNT) {
				zwarnnam(nam, "unknown signal: SIG%s",
					 signame, 0);
				returnval++;
			    } else
				printf("%d\n", sig);
			} else {
			    if (*signame) {
				zwarnnam(nam, "unknown signal: SIG%s",
					 signame, 0);
				returnval++;
			    } else {
				if (WIFSIGNALED(sig))
				    sig = WTERMSIG(sig);
				else if (WIFSTOPPED(sig))
				    sig = WSTOPSIG(sig);
				if (1 <= sig && sig <= SIGCOUNT)
				    printf("%s\n", sigs[sig]);
				else
				    printf("%d\n", sig);
			    }
			}
		    }
		    return returnval;
		}
		printf("%s", sigs[1]);
		for (sig = 2; sig <= SIGCOUNT; sig++)
		    printf(" %s", sigs[sig]);
		putchar('\n');
		return 0;
	    }
	    if ((*argv)[1] == 's' && (*argv)[2] == '\0')
		signame = *++argv;
	    else
		signame = *argv + 1;

	    /* check for signal matching specified name */
	    for (sig = 1; sig <= SIGCOUNT; sig++)
		if (!cstrpcmp(sigs + sig, &signame))
		    break;
	    if (*signame == '0' && !signame[1])
		sig = 0;
	    if (sig > SIGCOUNT) {
		zwarnnam(nam, "unknown signal: SIG%s", signame, 0);
		zwarnnam(nam, "type kill -l for a List of signals", NULL, 0);
		return 1;
	    }
	}
	argv++;
    }

    queue_signals();
    setcurjob();

    /* Remaining arguments specify processes.  Loop over them, and send the
    signal (number sig) to each process. */
    for (; *argv; argv++) {
	if (**argv == '%') {
	    /* job specifier introduced by '%' */
	    int p;

	    if ((p = getjob(*argv, nam)) == -1) {
		returnval++;
		continue;
	    }
	    if (killjb(jobtab + p, sig) == -1) {
		zwarnnam("kill", "kill %s failed: %e", *argv, errno);
		returnval++;
		continue;
	    }
	    /* automatically update the job table if sending a SIGCONT to a
	    job, and send the job a SIGCONT if sending it a non-stopping
	    signal. */
	    if (jobtab[p].stat & STAT_STOPPED) {
		if (sig == SIGCONT)
		    jobtab[p].stat &= ~STAT_STOPPED;
		if (sig != SIGKILL && sig != SIGCONT && sig != SIGTSTP
		    && sig != SIGTTOU && sig != SIGTTIN && sig != SIGSTOP)
		    killjb(jobtab + p, SIGCONT);
	    }
	} else if (!isanum(*argv)) {
	    zwarnnam("kill", "illegal pid: %s", *argv, 0);
	    returnval++;
	} else if (kill(atoi(*argv), sig) == -1) {
	    zwarnnam("kill", "kill %s failed: %e", *argv, errno);
	    returnval++;
	}
    }
    unqueue_signals();

    return returnval < 126 ? returnval : 1;
}

/* Suspend this shell */

/**/
int
bin_suspend(char *name, char **argv, char *ops, int func)
{
    /* won't suspend a login shell, unless forced */
    if (islogin && !ops['f']) {
	zwarnnam(name, "can't suspend login shell", NULL, 0);
	return 1;
    }
    if (jobbing) {
	/* stop ignoring signals */
	signal_default(SIGTTIN);
	signal_default(SIGTSTP);
	signal_default(SIGTTOU);
    }
    /* suspend ourselves with a SIGTSTP */
    kill(0, SIGTSTP);
    if (jobbing) {
	/* stay suspended */
	while (gettygrp() != mypgrp) {
	    sleep(1);
	    if (gettygrp() != mypgrp)
		kill(0, SIGTTIN);
	}
	/* restore signal handling */
	signal_ignore(SIGTTOU);
	signal_ignore(SIGTSTP);
	signal_ignore(SIGTTIN);
    }
    return 0;
}

/* find a job named s */

/**/
int
findjobnam(char *s)
{
    int jobnum;

    for (jobnum = MAXJOB - 1; jobnum >= 0; jobnum--)
	if (!(jobtab[jobnum].stat & (STAT_SUBJOB | STAT_NOPRINT)) &&
	    jobtab[jobnum].stat && jobtab[jobnum].procs && jobnum != thisjob &&
	    jobtab[jobnum].procs->text && strpfx(s, jobtab[jobnum].procs->text))
	    return jobnum;
    return -1;
}
