/*
 * system.h - system configuration header file
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

#if 0
/*
 * Setting _XPG_IV here is actually wrong and is not needed
 * with currently supported versions (5.43C20 and above)
 */
#ifdef sinix
# define _XPG_IV 1
#endif
#endif

/* NeXT has half-implemented POSIX support *
 * which currently fools configure         */
#ifdef __NeXT__
# undef HAVE_TERMIOS_H
# undef HAVE_SYS_UTSNAME_H
#endif

#ifdef PROTOTYPES
# define _(Args) Args
#else
# define _(Args) ()
#endif

#ifndef HAVE_ALLOCA
# define alloca zhalloc
#else
# ifdef __GNUC__
#  define alloca __builtin_alloca
# else
#  if HAVE_ALLOCA_H
#   include <alloca.h>
#  else
#   ifdef _AIX
 #   pragma alloca
#   else
#    ifndef alloca
char *alloca _((size_t));
#    endif
#   endif
#  endif
# endif
#endif

/*
 * libc.h in an optional package for Debian Linux is broken (it
 * defines dup() as a synonym for dup2(), which has a different
 * number of arguments), so just include it for next.
 */
#ifdef __NeXT__
# ifdef HAVE_LIBC_H
#  include <libc.h>
# endif
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <signal.h>
#include <setjmp.h>

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif

#ifdef HAVE_GRP_H
# include <grp.h>
#endif

#ifdef HAVE_DIRENT_H
# include <dirent.h>
#else /* !HAVE_DIRENT_H */
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
# define dirent direct
# undef HAVE_STRUCT_DIRENT_D_INO
# undef HAVE_STRUCT_DIRENT_D_STAT
# ifdef HAVE_STRUCT_DIRECT_D_INO
#  define HAVE_STRUCT_DIRENT_D_INO HAVE_STRUCT_DIRECT_D_INO
# endif
# ifdef HAVE_STRUCT_DIRECT_D_STAT
#  define HAVE_STRUCT_DIRENT_D_STAT HAVE_STRUCT_DIRECT_D_STAT
# endif
#endif /* !HAVE_DIRENT_H */

#ifdef HAVE_STDLIB_H
# ifdef ZSH_MEM
   /* malloc and calloc are macros in GNU's stdlib.h unless the
    * the __MALLOC_0_RETURNS_NULL macro is defined */
#  define __MALLOC_0_RETURNS_NULL
# endif
# include <stdlib.h>
#endif

#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/* This is needed by some old SCO unices */
#ifndef HAVE_STRUCT_TIMEZONE
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif

/* There's more than one non-standard way to get at this data */
#if !defined(HAVE_STRUCT_DIRENT_D_INO) && defined(HAVE_STRUCT_DIRENT_D_STAT)
# define d_ino d_stat.st_ino
# define HAVE_STRUCT_DIRENT_D_INO HAVE_STRUCT_DIRENT_D_STAT
#endif /* !HAVE_STRUCT_DIRENT_D_INO && HAVE_STRUCT_DIRENT_D_STAT */

/* Sco needs the following include for struct utimbuf *
 * which is strange considering we do not use that    *
 * anywhere in the code                               */
#ifdef __sco
# include <utime.h>
#endif

#ifdef HAVE_SYS_TIMES_H
# include <sys/times.h>
#endif

#if STDC_HEADERS || HAVE_STRING_H
# include <string.h>
/* An ANSI string.h and pre-ANSI memory.h might conflict.  */
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#else   /* not STDC_HEADERS and not HAVE_STRING_H */
# include <strings.h>
/* memory.h and strings.h conflict on some systems.  */
#endif /* not STDC_HEADERS and not HAVE_STRING_H */

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#ifdef HAVE_VARIABLE_LENGTH_ARRAYS
# define VARARR(X,Y,Z)	X (Y)[Z]
#else
# define VARARR(X,Y,Z)	X *(Y) = (X *) alloca(sizeof(X) * (Z))
#endif

/* we should handle unlimited sizes from pathconf(_PC_PATH_MAX) */
/* but this is too much trouble                                 */
#ifndef PATH_MAX
# ifdef MAXPATHLEN
#  define PATH_MAX MAXPATHLEN
# else
#  ifdef _POSIX_PATH_MAX
#   define PATH_MAX _POSIX_PATH_MAX
#  else
    /* so we will just pick something */
#   define PATH_MAX 1024
#  endif
# endif
#endif

#ifndef OPEN_MAX
# ifdef NOFILE
#  define OPEN_MAX NOFILE
# else
   /* so we will just pick something */
#  define OPEN_MAX 64
# endif
#endif
#ifndef HAVE_SYSCONF
# define zopenmax() ((long) OPEN_MAX)
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# include <sys/file.h>
#endif

/* The following will only be defined if <sys/wait.h> is POSIX.    *
 * So we don't have to worry about union wait. But some machines   *
 * (NeXT) include <sys/wait.h> from other include files, so we     *
 * need to undef and then redefine the wait macros if <sys/wait.h> *
 * is not POSIX.                                                   */

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#else
# undef WIFEXITED
# undef WEXITSTATUS
# undef WIFSIGNALED
# undef WTERMSIG
# undef WCOREDUMP
# undef WIFSTOPPED
# undef WSTOPSIG
#endif

/* missing macros for wait/waitpid/wait3 */
#ifndef WIFEXITED
# define WIFEXITED(X) (((X)&0377)==0)
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(X) (((X)>>8)&0377)
#endif
#ifndef WIFSIGNALED
# define WIFSIGNALED(X) (((X)&0377)!=0&&((X)&0377)!=0177)
#endif
#ifndef WTERMSIG
# define WTERMSIG(X) ((X)&0177)
#endif
#ifndef WCOREDUMP
# define WCOREDUMP(X) ((X)&0200)
#endif
#ifndef WIFSTOPPED
# define WIFSTOPPED(X) (((X)&0377)==0177)
#endif
#ifndef WSTOPSIG
# define WSTOPSIG(X) (((X)>>8)&0377)
#endif

#ifdef HAVE_SYS_SELECT_H
# ifndef TIME_H_SELECT_H_CONFLICTS
#  include <sys/select.h>
# endif
#elif defined(SELECT_IN_SYS_SOCKET_H)
# include <sys/socket.h>
#endif

#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

#ifdef HAVE_TERMIOS_H
# ifdef __sco
   /* termios.h includes sys/termio.h instead of sys/termios.h; *
    * hence the declaration for struct termios is missing       */
#  include <sys/termios.h>
# else
#  include <termios.h>
# endif
# ifdef _POSIX_VDISABLE
#  define VDISABLEVAL _POSIX_VDISABLE
# else
#  define VDISABLEVAL 0
# endif
# define HAS_TIO 1
#else    /* not TERMIOS */
# ifdef HAVE_TERMIO_H
#  include <termio.h>
#  define VDISABLEVAL -1
#  define HAS_TIO 1
# else   /* not TERMIOS and TERMIO */
#  include <sgtty.h>
# endif  /* HAVE_TERMIO_H  */
#endif   /* HAVE_TERMIOS_H */

#if defined(GWINSZ_IN_SYS_IOCTL) || defined(CLOBBERS_TYPEAHEAD)
# include <sys/ioctl.h>
#endif
#ifdef WINSIZE_IN_PTEM
# include <sys/stream.h>
# include <sys/ptem.h>
#endif

#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif

#define DEFAULT_WORDCHARS "*?_-.[]~=/&;!#$%^(){}<>"
#define DEFAULT_TIMEFMT   "%J  %U user %S system %P cpu %*E total"

/* Posix getpgrp takes no argument, while the BSD version *
 * takes the process ID as an argument                    */
#ifdef GETPGRP_VOID
# define GETPGRP() getpgrp()
#else
# define GETPGRP() getpgrp(0)
#endif

#ifndef HAVE_GETLOGIN
# define getlogin() cuserid(NULL)
#endif

#ifdef HAVE_SETPGID
# define setpgrp setpgid
#endif

/* can we set the user/group id of a process */

#ifndef HAVE_SETUID
# ifdef HAVE_SETREUID
#  define setuid(X) setreuid(X,X)
#  define setgid(X) setregid(X,X)
#  define HAVE_SETUID
# endif
#endif

/* can we set the effective user/group id of a process */

#ifndef HAVE_SETEUID
# ifdef HAVE_SETREUID
#  define seteuid(X) setreuid(-1,X)
#  define setegid(X) setregid(-1,X)
#  define HAVE_SETEUID
# else
#  ifdef HAVE_SETRESUID
#   define seteuid(X) setresuid(-1,X,-1)
#   define setegid(X) setresgid(-1,X,-1)
#   define HAVE_SETEUID
#  endif
# endif
#endif

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
# if defined(__hpux) && !defined(RLIMIT_CPU)
/* HPUX does have the BSD rlimits in the kernel.  Officially they are *
 * unsupported but quite a few of them like RLIMIT_CORE seem to work. *
 * All the following are in the <sys/resource.h> but made visible     *
 * only for the kernel.                                               */
#  define	RLIMIT_CPU	0
#  define	RLIMIT_FSIZE	1
#  define	RLIMIT_DATA	2
#  define	RLIMIT_STACK	3
#  define	RLIMIT_CORE	4
#  define	RLIMIT_RSS	5
#  define	RLIMIT_NOFILE   6
#  define	RLIMIT_OPEN_MAX	RLIMIT_NOFILE
#  define	RLIM_NLIMITS	7
#  define	RLIM_INFINITY	0x7fffffff
# endif
#endif

/* we use the SVR4 constant instead of the BSD one */
#if !defined(RLIMIT_NOFILE) && defined(RLIMIT_OFILE)
# define RLIMIT_NOFILE RLIMIT_OFILE
#endif
#if !defined(RLIMIT_VMEM) && defined(RLIMIT_AS)
# define RLIMIT_VMEM RLIMIT_AS
#endif

#ifdef HAVE_SYS_CAPABILITY_H
# include <sys/capability.h>
#endif

/* DIGBUFSIZ is the length of a buffer which can hold the -LONG_MAX-1 *
 * (or with ZSH_64_BIT_TYPE maybe -LONG_LONG_MAX-1)                   *
 * converted to printable decimal form including the sign and the     *
 * terminating null character. Below 0.30103 > lg 2.                  *
 * BDIGBUFSIZE is for a number converted to printable binary form.    */
#define DIGBUFSIZE ((int)(((sizeof(zlong) * 8) - 1) * 0.30103) + 3)
#define BDIGBUFSIZE ((int)((sizeof(zlong) * 8) + 4))

/* If your stat macros are broken, we will *
 * just undefine them.                     */

#ifdef STAT_MACROS_BROKEN
# undef S_ISBLK
# undef S_ISCHR
# undef S_ISDIR
# undef S_ISDOOR
# undef S_ISFIFO
# undef S_ISLNK
# undef S_ISMPB
# undef S_ISMPC
# undef S_ISNWK
# undef S_ISOFD
# undef S_ISOFL
# undef S_ISREG
# undef S_ISSOCK
#endif  /* STAT_MACROS_BROKEN.  */

/* If you are missing the stat macros, we *
 * define our own                         */

#ifndef S_IFMT
# define S_IFMT 0170000
#endif

#if !defined(S_ISBLK) && defined(S_IFBLK)
# define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#endif
#if !defined(S_ISCHR) && defined(S_IFCHR)
# define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif
#if !defined(S_ISDIR) && defined(S_IFDIR)
# define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISDOOR) && defined(S_IFDOOR)      /* Solaris */
# define S_ISDOOR(m) (((m) & S_IFMT) == S_IFDOOR)
#endif
#if !defined(S_ISFIFO) && defined(S_IFIFO)
# define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#endif
#if !defined(S_ISLNK) && defined(S_IFLNK)
# define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif
#if !defined(S_ISMPB) && defined(S_IFMPB)        /* V7 */
# define S_ISMPB(m) (((m) & S_IFMT) == S_IFMPB)
#endif
#if !defined(S_ISMPC) && defined(S_IFMPC)        /* V7 */
# define S_ISMPC(m) (((m) & S_IFMT) == S_IFMPC)
#endif
#if !defined(S_ISNWK) && defined(S_IFNWK)        /* HP/UX */
# define S_ISNWK(m) (((m) & S_IFMT) == S_IFNWK)
#endif
#if !defined(S_ISOFD) && defined(S_IFOFD)        /* Cray */
# define S_ISOFD(m) (((m) & S_IFMT) == S_IFOFD)
#endif
#if !defined(S_ISOFL) && defined(S_IFOFL)        /* Cray */
# define S_ISOFL(m) (((m) & S_IFMT) == S_IFOFL)
#endif
#if !defined(S_ISREG) && defined(S_IFREG)
# define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISSOCK) && defined(S_IFSOCK)
# define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif

/* We will pretend to have all file types on any system. */

#ifndef S_ISBLK
# define S_ISBLK(m) ((void)(m), 0)
#endif
#ifndef S_ISCHR
# define S_ISCHR(m) ((void)(m), 0)
#endif
#ifndef S_ISDIR
# define S_ISDIR(m) ((void)(m), 0)
#endif
#ifndef S_ISDOOR
# define S_ISDOOR(m) ((void)(m), 0)
#endif
#ifndef S_ISFIFO
# define S_ISFIFO(m) ((void)(m), 0)
#endif
#ifndef S_ISLNK
# define S_ISLNK(m) ((void)(m), 0)
#endif
#ifndef S_ISMPB
# define S_ISMPB(m) ((void)(m), 0)
#endif
#ifndef S_ISMPC
# define S_ISMPC(m) ((void)(m), 0)
#endif
#ifndef S_ISNWK
# define S_ISNWK(m) ((void)(m), 0)
#endif
#ifndef S_ISOFD
# define S_ISOFD(m) ((void)(m), 0)
#endif
#ifndef S_ISOFL
# define S_ISOFL(m) ((void)(m), 0)
#endif
#ifndef S_ISREG
# define S_ISREG(m) ((void)(m), 0)
#endif
#ifndef S_ISSOCK
# define S_ISSOCK(m) ((void)(m), 0)
#endif

/* file mode permission bits */

#ifndef S_ISUID
# define S_ISUID 04000
#endif
#ifndef S_ISGID
# define S_ISGID 02000
#endif
#ifndef S_ISVTX
# define S_ISVTX 01000
#endif
#ifndef S_IRUSR
# define S_IRUSR 00400
#endif
#ifndef S_IWUSR
# define S_IWUSR 00200
#endif
#ifndef S_IXUSR
# define S_IXUSR 00100
#endif
#ifndef S_IRGRP
# define S_IRGRP 00040
#endif
#ifndef S_IWGRP
# define S_IWGRP 00020
#endif
#ifndef S_IXGRP
# define S_IXGRP 00010
#endif
#ifndef S_IROTH
# define S_IROTH 00004
#endif
#ifndef S_IWOTH
# define S_IWOTH 00002
#endif
#ifndef S_IXOTH
# define S_IXOTH 00001
#endif
#ifndef S_IRWXU
# define S_IRWXU (S_IRUSR|S_IWUSR|S_IXUSR)
#endif
#ifndef S_IRWXG
# define S_IRWXG (S_IRGRP|S_IWGRP|S_IXGRP)
#endif
#ifndef S_IRWXO
# define S_IRWXO (S_IROTH|S_IWOTH|S_IXOTH)
#endif
#ifndef S_IRUGO
# define S_IRUGO (S_IRUSR|S_IRGRP|S_IROTH)
#endif
#ifndef S_IWUGO
# define S_IWUGO (S_IWUSR|S_IWGRP|S_IWOTH)
#endif
#ifndef S_IXUGO
# define S_IXUGO (S_IXUSR|S_IXGRP|S_IXOTH)
#endif

#ifndef HAVE_LSTAT
# define lstat stat
#endif

#ifndef HAVE_READLINK
# define readlink(PATH, BUF, BUFSZ) \
    ((void)(PATH), (void)(BUF), (void)(BUFSZ), errno = ENOSYS, -1)
#endif

#ifndef F_OK          /* missing macros for access() */
# define F_OK 0
# define X_OK 1
# define W_OK 2
# define R_OK 4
#endif

#ifndef HAVE_LCHOWN
# define lchown chown
#endif

#ifndef HAVE_MEMCPY
# define memcpy memmove
#endif

#ifndef HAVE_MEMMOVE
# define memmove(dest, src, len) bcopy((src), (dest), (len))
#endif

#ifndef offsetof
# define offsetof(TYPE, MEM) ((char *)&((TYPE *)0)->MEM - (char *)(TYPE *)0)
#endif

extern char **environ;

/* These variables are sometimes defined in, *
 * and needed by, the termcap library.       */
#if MUST_DEFINE_OSPEED
extern char PC, *BC, *UP;
extern short ospeed;
#endif

/* Rename some global zsh variables to avoid *
 * possible name clashes with libc           */

#define cs zshcs
#define ll zshll

#ifndef O_NOCTTY
# define O_NOCTTY 0
#endif

#ifdef _LARGEFILE_SOURCE
#ifdef HAVE_FSEEKO
#define fseek fseeko
#endif
#ifdef HAVE_FTELLO
#define ftell ftello
#endif
#endif

/* Can't support job control without working tcsetgrp() */
#ifdef BROKEN_TCSETPGRP
#undef JOB_CONTROL
#endif /* BROKEN_TCSETPGRP */

#ifdef BROKEN_KILL_ESRCH
#undef ESRCH
#define ESRCH EINVAL
#endif /* BROKEN_KILL_ESRCH */

/* Can we do locale stuff? */
#undef USE_LOCALE
#if defined(CONFIG_LOCALE) && defined(HAVE_SETLOCALE) && defined(LC_ALL)
# define USE_LOCALE 1
#endif /* CONFIG_LOCALE && HAVE_SETLOCALE && LC_ALL */

#ifndef MAILDIR_SUPPORT
#define mailstat(X,Y) stat(X,Y)
#endif

#ifdef __CYGWIN__
# define IS_DIRSEP(c) ((c) == '/' || (c) == '\\')
#else
# define IS_DIRSEP(c) ((c) == '/')
#endif
