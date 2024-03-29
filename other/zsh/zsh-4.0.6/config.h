/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/***** begin user configuration section *****/

/* Define this to be the location of your password file */
#define PASSWD_FILE "/etc/passwd"

/* Define this to be the name of your NIS/YP password *
 * map (if applicable)                                */
#define PASSWD_MAP "passwd.byname"

/* Define to 1 if you want user names to be cached */
#define CACHE_USERNAMES 1

/* Define to 1 if system supports job control */
#define JOB_CONTROL 1

/* Define this if you use "suspended" instead of "stopped" */
#define USE_SUSPENDED 1
 
/* The default history buffer size in lines */
#define DEFAULT_HISTSIZE 30

/* The default editor for the fc builtin */
#define DEFAULT_FCEDIT "vi"

/* The default prefix for temporary files */
#define DEFAULT_TMPPREFIX "/tmp/zsh"


/***** end of user configuration section            *****/
/***** shouldn't have to change anything below here *****/

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define if the `getpgrp' function takes no argument.  */
/* #undef GETPGRP_VOID */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
/* #undef HAVE_ALLOCA_H */

/* Define if you have a working `mmap' system call.  */
/* #undef HAVE_MMAP */

/* Define if you have the strcoll function and it is properly defined.  */
/* #undef HAVE_STRCOLL */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
/* #undef HAVE_SYS_WAIT_H */

/* Define to `int' if <sys/types.h> doesn't define.  */
#define mode_t int

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
#define pid_t int

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly.  */
/* #undef STAT_MACROS_BROKEN */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* The global file to source absolutely first whenever zsh is run; *
 * if undefined, don't source anything                             */
#define GLOBAL_ZSHENV "/etc/zshenv"

/* The global file to source whenever zsh is run; *
 * if undefined, don't source anything            */
#define GLOBAL_ZSHRC "/etc/zshrc"

/* The global file to source whenever zsh is run as a login shell; *
 * if undefined, don't source anything                             */
#define GLOBAL_ZLOGIN "/etc/zlogin"

/* The global file to source whenever zsh is run as a login shell, *
 * before zshrc is read; if undefined, don't source anything       */
#define GLOBAL_ZPROFILE "/etc/zprofile"

/* The global file to source whenever zsh was run as a login shell.  *
 * This is sourced right before exiting.  If undefined, don't source *
 * anything                                                          */
#define GLOBAL_ZLOGOUT "/etc/zlogout"

/* Define to 1 if compiler could initialise a union */
#define HAVE_UNION_INIT 1

/* Define to 1 if compiler incorrectly cast signed to unsigned */
/* #undef BROKEN_SIGNED_TO_UNSIGNED_CASTING */

/* Define to 1 if compiler supports variable-length arrays */
#define HAVE_VARIABLE_LENGTH_ARRAYS 1

/* Define if your system defines TIOCGWINSZ in sys/ioctl.h.  */
#define GWINSZ_IN_SYS_IOCTL 1

/* Define to 1 if you have NIS */
/* #undef HAVE_NIS */

/* Define to 1 if you have NISPLUS */
/* #undef HAVE_NIS_PLUS */

/* Define to 1 if you have RFS superroot directory. */
/* #undef HAVE_SUPERROOT */

/* Define to 1 if you need to use the native getcwd */
/* #undef USE_GETCWD */

/* Define to the path of the /dev/fd filesystem */
/* #undef PATH_DEV_FD */

/* Define if sys/time.h and sys/select.h cannot be both included */
/* #undef TIME_H_SELECT_H_CONFLICTS */

/* Define to be the machine type (microprocessor class or machine model) */
#define MACHTYPE "i386"

/* Define to be the name of the operating system */
#define OSTYPE "nextstep3"

/* Define to 1 if ANSI function prototypes are usable.  */
#define PROTOTYPES 1

/* Define to be location of utmp file. */
#define PATH_UTMP_FILE "/etc/utmp"

/* Define to be location of utmpx file. */
/* #undef PATH_UTMPX_FILE */

/* Define to be location of wtmp file. */
#define PATH_WTMP_FILE "/usr/adm/wtmp"

/* Define to be location of wtmpx file. */
/* #undef PATH_WTMPX_FILE */

/* Define to 1 if struct utmp is defined by a system header */
#define HAVE_STRUCT_UTMP 1

/* Define to 1 if struct utmpx is defined by a system header */
/* #undef HAVE_STRUCT_UTMPX */

/* Define if your system's struct utmp has a member named ut_host.  */
#define HAVE_STRUCT_UTMP_UT_HOST 1

/* Define if your system's struct utmpx has a member named ut_host.  */
/* #undef HAVE_STRUCT_UTMPX_UT_HOST */

/* Define if your system's struct utmpx has a member named ut_xtime.  */
/* #undef HAVE_STRUCT_UTMPX_UT_XTIME */

/* Define if your system's struct utmpx has a member named ut_tv.  */
/* #undef HAVE_STRUCT_UTMPX_UT_TV */

/* Define if your system's struct dirent has a member named d_ino.  */
/* #undef HAVE_STRUCT_DIRENT_D_INO */

/* Define if your system's struct dirent has a member named d_stat.  */
/* #undef HAVE_STRUCT_DIRENT_D_STAT */

/* Define if your system's struct direct has a member named d_ino.  */
#define HAVE_STRUCT_DIRECT_D_INO 1

/* Define if your system's struct direct has a member named d_stat.  */
/* #undef HAVE_STRUCT_DIRECT_D_STAT */

/* Define if your system's struct sockaddr_in6 has a member named sin6_scope_id.  */
/* #undef HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID */

/* Define to be a string corresponding the vendor of the machine */
#define VENDOR "next"

/* Define to limit job table size */
#define MAXJOB 50
/* #undef NEED_LINUX_TASKS_H */

/* Define if your system defines `struct winsize' in sys/ptem.h.  */
/* #undef WINSIZE_IN_PTEM */

/* Define to 1 if you want to debug zsh */
/* #undef DEBUG */

/* Define to 1 if you want to use zsh's own memory allocation routines */
/* #undef ZSH_MEM */

/* Define to 1 if you want to debug zsh memory allocation routines */
/* #undef ZSH_MEM_DEBUG */

/* Define to 1 if you want to turn on warnings of memory allocation errors */
/* #undef ZSH_MEM_WARNING */

/* Define to 1 if you want to turn on memory checking for free() */
/* #undef ZSH_SECURE_FREE */

/* Define to 1 if you want to get debugging information on internal *
 * hash tables.  This turns on the `hashinfo' builtin.              */
/* #undef ZSH_HASH_DEBUG */

/* Undefine this if you don't want to get a restricted shell *
 * when zsh is exec'd with basename that starts with r.      *
 * By default this is defined.                               */
#define RESTRICTED_R 1

/* Define for Maildir support */
/* #undef MAILDIR_SUPPORT */

/* Define for function depth limits */
/* #undef MAX_FUNCTION_DEPTH */

/* Define if you want locale features.  By default this is defined. */
#define CONFIG_LOCALE 1

/* Define to 1 if your termcap library has the ospeed variable */
/* #undef HAVE_OSPEED */
/* Define to 1 if you have ospeed, but it is not defined in termcap.h */
/* #undef MUST_DEFINE_OSPEED */

/* Define to 1 if tgetent() accepts NULL as a buffer */
/* #undef TGETENT_ACCEPTS_NULL */

/* Define to 1 if you use POSIX style signal handling */
/* #undef POSIX_SIGNALS */

/* Define to 1 if you use BSD style signal handling (and can block signals) */
#define BSD_SIGNALS 1

/* Define to 1 if you use SYS style signal handling (and can block signals) */
/* #undef SYSV_SIGNALS */

/* Define to 1 if you have no signal blocking at all (bummer) */
/* #undef NO_SIGNAL_BLOCKING */

/* Define to `unsigned int' if <sys/types.h> or <signal.h> doesn't define */
#define sigset_t unsigned int

/* Define to 1 if struct timezone is defined by a system header */
#define HAVE_STRUCT_TIMEZONE 1

/* Define to 1 if there is a prototype defined for brk() on your system */
/* #undef HAVE_BRK_PROTO */

/* Define to 1 if there is a prototype defined for sbrk() on your system */
/* #undef HAVE_SBRK_PROTO */

/* Define to 1 if there is a prototype defined for ioctl() on your system */
/* #undef HAVE_IOCTL_PROTO */

/* Define to 1 if there is a prototype defined for mknod() on your system */
/* #undef HAVE_MKNOD_PROTO */

/* Define to 1 if select() is defined in <sys/socket.h>, ie BeOS R4.51*/
#define SELECT_IN_SYS_SOCKET_H 1

/* Define to 1 if system has working FIFO's */
#define HAVE_FIFOS 1

/* Define to 1 if struct rlimit uses quad_t */
/* #undef RLIM_T_IS_QUAD_T */

/* Define to 1 if struct rlimit uses long long */
/* #undef RLIM_T_IS_LONG_LONG */

/* Define to 1 if rlimit uses unsigned */
/* #undef RLIM_T_IS_UNSIGNED */

/* Define to the type used in struct rlimit */
#define rlim_t long

/* Define to 1 if /bin/sh does not interpret \ escape sequences */
/* #undef SH_USE_BSD_ECHO */

/* Define to 1 if system has working link() */
#define HAVE_LINK 1

/* Define to 1 if kill(pid, 0) doesn't return ESRCH, ie BeOS R4.51 */
/* #undef BROKEN_KILL_ESRCH */

/* Define to 1 if sigsuspend() is broken, ie BeOS R4.51 */
#define BROKEN_POSIX_SIGSUSPEND 1

/* Define to 1 if getpwnam() is faked, ie BeOS R4.51 */
/* #undef GETPWNAM_FAKED */

/* Define to 1 if tcsetpgrp() doesn't work, ie BeOS R4.51 */
#define BROKEN_TCSETPGRP 1

/* Define to 1 if an underscore has to be prepended to dlsym() argument */
/* #undef DLSYM_NEEDS_UNDERSCORE */

/* Define to 1 if multiple modules defining the same symbol are OK */
/* #undef DYNAMIC_NAME_CLASH_OK */

/* The extension used for dynamically loaded modules */
#define DL_EXT ""

/* Define to 1 if you want to use dynamically loaded modules */
/* #undef DYNAMIC */

/* Define to 1 if you want to use dynamically loaded modules on AIX */
/* #undef AIXDYNAMIC */

/* Define to 1 if you want to use dynamically loaded modules on HPUX 10 */
/* #undef HPUXDYNAMIC */

/* Define to `unsigned long' if <sys/types.h> doesn't define. */
/* #undef ino_t */

/*
 * Definitions used when a long is less than eight byte, to try to
 * provide some support for eight byte operations.
 *
 * Note that ZSH_64_BIT_TYPE, OFF_T_IS_64_BIT, INO_T_IS_64_BIT do *not* get
 * defined if long is already 64 bits, since in that case no special handling
 * is required.
 */
/* Define to 1 if long is 64 bits */
/* #undef LONG_IS_64_BIT */

/* Define to a 64 bit integer type if there is one, but long is shorter */
#define ZSH_64_BIT_TYPE long long

/* Define to an unsigned variant of ZSH_64_BIT_TYPE if that is defined */
#define ZSH_64_BIT_UTYPE unsigned long long

/* Define to 1 if off_t is 64 bit (for large file support) */
/* #undef OFF_T_IS_64_BIT */

/* Define to 1 if ino_t is 64 bit (for large file support) */
/* #undef INO_T_IS_64_BIT */

/* Define to 1 if h_errno is not defined by the system */
/* #undef USE_LOCAL_H_ERRNO */

/* Define if you have the termcap boolcodes symbol.  */
/* #undef HAVE_BOOLCODES */

/* Define if you have the termcap numcodes symbol.  */
/* #undef HAVE_NUMCODES */

/* Define if you have the termcap strcodes symbol.  */
/* #undef HAVE_STRCODES */

/* Define if you have the terminfo boolnames symbol.  */
/* #undef HAVE_BOOLNAMES */

/* Define if you have the terminfo numnames symbol.  */
/* #undef HAVE_NUMNAMES */

/* Define if you have the terminfo strnames symbol.  */
/* #undef HAVE_STRNAMES */

/* Define if term.h chokes without curses.h */
/* #undef TERM_H_NEEDS_CURSES_H */

/* Define to the base type of the third argument of accept */
#define SOCKLEN_T int

/* Define if you have the _mktemp function.  */
/* #undef HAVE__MKTEMP */

/* Define if you have the brk function.  */
#define HAVE_BRK 1

/* Define if you have the cap_get_proc function.  */
/* #undef HAVE_CAP_GET_PROC */

/* Define if you have the difftime function.  */
#define HAVE_DIFFTIME 1

/* Define if you have the dlclose function.  */
/* #undef HAVE_DLCLOSE */

/* Define if you have the dlerror function.  */
/* #undef HAVE_DLERROR */

/* Define if you have the dlopen function.  */
/* #undef HAVE_DLOPEN */

/* Define if you have the dlsym function.  */
/* #undef HAVE_DLSYM */

/* Define if you have the faccessx function.  */
/* #undef HAVE_FACCESSX */

/* Define if you have the fchdir function.  */
/* #undef HAVE_FCHDIR */

/* Define if you have the fseeko function.  */
/* #undef HAVE_FSEEKO */

/* Define if you have the ftello function.  */
/* #undef HAVE_FTELLO */

/* Define if you have the ftruncate function.  */
#define HAVE_FTRUNCATE 1

/* Define if you have the getenv function.  */
#define HAVE_GETENV 1

/* Define if you have the getgrgid function.  */
#define HAVE_GETGRGID 1

/* Define if you have the getgrnam function.  */
#define HAVE_GETGRNAM 1

/* Define if you have the gethostbyname2 function.  */
/* #undef HAVE_GETHOSTBYNAME2 */

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the getipnodebyname function.  */
/* #undef HAVE_GETIPNODEBYNAME */

/* Define if you have the getlogin function.  */
#define HAVE_GETLOGIN 1

/* Define if you have the getpagesize function.  */
#define HAVE_GETPAGESIZE 1

/* Define if you have the getpwent function.  */
#define HAVE_GETPWENT 1

/* Define if you have the getpwnam function.  */
#define HAVE_GETPWNAM 1

/* Define if you have the getpwuid function.  */
#define HAVE_GETPWUID 1

/* Define if you have the getrlimit function.  */
#define HAVE_GETRLIMIT 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the inet_aton function.  */
/* #undef HAVE_INET_ATON */

/* Define if you have the inet_ntop function.  */
/* #undef HAVE_INET_NTOP */

/* Define if you have the inet_pton function.  */
/* #undef HAVE_INET_PTON */

/* Define if you have the initgroups function.  */
#define HAVE_INITGROUPS 1

/* Define if you have the killpg function.  */
#define HAVE_KILLPG 1

/* Define if you have the lchown function.  */
/* #undef HAVE_LCHOWN */

/* Define if you have the load function.  */
/* #undef HAVE_LOAD */

/* Define if you have the loadbind function.  */
/* #undef HAVE_LOADBIND */

/* Define if you have the loadquery function.  */
/* #undef HAVE_LOADQUERY */

/* Define if you have the lstat function.  */
#define HAVE_LSTAT 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the mkfifo function.  */
/* #undef HAVE_MKFIFO */

/* Define if you have the msync function.  */
/* #undef HAVE_MSYNC */

/* Define if you have the munmap function.  */
/* #undef HAVE_MUNMAP */

/* Define if you have the nice function.  */
#define HAVE_NICE 1

/* Define if you have the nis_list function.  */
/* #undef HAVE_NIS_LIST */

/* Define if you have the pathconf function.  */
/* #undef HAVE_PATHCONF */

/* Define if you have the poll function.  */
/* #undef HAVE_POLL */

/* Define if you have the putenv function.  */
/* #undef HAVE_PUTENV */

/* Define if you have the readlink function.  */
#define HAVE_READLINK 1

/* Define if you have the sbrk function.  */
#define HAVE_SBRK 1

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the seteuid function.  */
#define HAVE_SETEUID 1

/* Define if you have the setlocale function.  */
#define HAVE_SETLOCALE 1

/* Define if you have the setpgid function.  */
/* #undef HAVE_SETPGID */

/* Define if you have the setpgrp function.  */
#define HAVE_SETPGRP 1

/* Define if you have the setresuid function.  */
/* #undef HAVE_SETRESUID */

/* Define if you have the setreuid function.  */
#define HAVE_SETREUID 1

/* Define if you have the setsid function.  */
/* #undef HAVE_SETSID */

/* Define if you have the setuid function.  */
#define HAVE_SETUID 1

/* Define if you have the setupterm function.  */
/* #undef HAVE_SETUPTERM */

/* Define if you have the shl_findsym function.  */
/* #undef HAVE_SHL_FINDSYM */

/* Define if you have the shl_load function.  */
/* #undef HAVE_SHL_LOAD */

/* Define if you have the shl_unload function.  */
/* #undef HAVE_SHL_UNLOAD */

/* Define if you have the sigaction function.  */
/* #undef HAVE_SIGACTION */

/* Define if you have the sigblock function.  */
#define HAVE_SIGBLOCK 1

/* Define if you have the sighold function.  */
/* #undef HAVE_SIGHOLD */

/* Define if you have the signgam function.  */
#define HAVE_SIGNGAM 1

/* Define if you have the sigprocmask function.  */
/* #undef HAVE_SIGPROCMASK */

/* Define if you have the sigrelse function.  */
/* #undef HAVE_SIGRELSE */

/* Define if you have the sigsetmask function.  */
#define HAVE_SIGSETMASK 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the sysconf function.  */
/* #undef HAVE_SYSCONF */

/* Define if you have the tcgetattr function.  */
/* #undef HAVE_TCGETATTR */

/* Define if you have the tcsetpgrp function.  */
/* #undef HAVE_TCSETPGRP */

/* Define if you have the tgetent function.  */
#define HAVE_TGETENT 1

/* Define if you have the tigetflag function.  */
/* #undef HAVE_TIGETFLAG */

/* Define if you have the tigetnum function.  */
/* #undef HAVE_TIGETNUM */

/* Define if you have the tigetstr function.  */
/* #undef HAVE_TIGETSTR */

/* Define if you have the uname function.  */
/* #undef HAVE_UNAME */

/* Define if you have the unload function.  */
/* #undef HAVE_UNLOAD */

/* Define if you have the wait3 function.  */
#define HAVE_WAIT3 1

/* Define if you have the waitpid function.  */
/* #undef HAVE_WAITPID */

/* Define if you have the <curses.h> header file.  */
/* #undef HAVE_CURSES_H */

/* Define if you have the <dirent.h> header file.  */
/* #undef HAVE_DIRENT_H */

/* Define if you have the <dl.h> header file.  */
/* #undef HAVE_DL_H */

/* Define if you have the <dlfcn.h> header file.  */
/* #undef HAVE_DLFCN_H */

/* Define if you have the <errno.h> header file.  */
#define HAVE_ERRNO_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <grp.h> header file.  */
#define HAVE_GRP_H 1

/* Define if you have the <libc.h> header file.  */
#define HAVE_LIBC_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
#define HAVE_LOCALE_H 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <netinet/in_systm.h> header file.  */
#define HAVE_NETINET_IN_SYSTM_H 1

/* Define if you have the <poll.h> header file.  */
/* #undef HAVE_POLL_H */

/* Define if you have the <pwd.h> header file.  */
#define HAVE_PWD_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/capability.h> header file.  */
/* #undef HAVE_SYS_CAPABILITY_H */

/* Define if you have the <sys/dir.h> header file.  */
#define HAVE_SYS_DIR_H 1

/* Define if you have the <sys/filio.h> header file.  */
/* #undef HAVE_SYS_FILIO_H */

/* Define if you have the <sys/mman.h> header file.  */
#define HAVE_SYS_MMAN_H 1

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/resource.h> header file.  */
#define HAVE_SYS_RESOURCE_H 1

/* Define if you have the <sys/select.h> header file.  */
/* #undef HAVE_SYS_SELECT_H */

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/times.h> header file.  */
#define HAVE_SYS_TIMES_H 1

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <sys/utsname.h> header file.  */
#define HAVE_SYS_UTSNAME_H 1

/* Define if you have the <term.h> header file.  */
/* #undef HAVE_TERM_H */

/* Define if you have the <termcap.h> header file.  */
/* #undef HAVE_TERMCAP_H */

/* Define if you have the <termio.h> header file.  */
/* #undef HAVE_TERMIO_H */

/* Define if you have the <termios.h> header file.  */
#define HAVE_TERMIOS_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <utmp.h> header file.  */
#define HAVE_UTMP_H 1

/* Define if you have the <utmpx.h> header file.  */
/* #undef HAVE_UTMPX_H */

/* Define if you have the cap library (-lcap).  */
/* #undef HAVE_LIBCAP */

/* Define if you have the dl library (-ldl).  */
/* #undef HAVE_LIBDL */

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */
