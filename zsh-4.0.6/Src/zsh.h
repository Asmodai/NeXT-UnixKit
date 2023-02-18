/*
 * zsh.h - standard header file
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

#define trashzle()      trashzleptr()
#define zleread(X,Y,H)  zlereadptr(X,Y,H)
#define spaceinline(X)  spaceinlineptr(X)
#define zrefresh()      refreshptr()

#define compctlread(N,A,O,R) compctlreadptr(N,A,O,R)

/* A few typical macros */
#define minimum(a,b)  ((a) < (b) ? (a) : (b))

/*
 * Our longest integer type:  will be a 64 bit either if long already is,
 * or if we found some alternative such as long long.
 * Currently we only define this to be longer than a long if --enable-lfs
 * was given.  That enables internal use of 64-bit types even if
 * no actual large file support is present.
 */
#ifdef ZSH_64_BIT_TYPE
typedef ZSH_64_BIT_TYPE zlong;
#ifdef ZSH_64_BIT_UTYPE
typedef ZSH_64_BIT_UTYPE zulong;
#else
typedef unsigned zlong zulong;
#endif
#else
typedef long zlong;
typedef unsigned long zulong;
#endif

/*
 * Double float support requires 64-bit alignment, so if longs and
 * pointers are less we need to pad out.
 */
#ifndef LONG_IS_64_BIT
# define PAD_64_BIT 1
#endif

/* math.c */
typedef struct {
    union {
        zlong l;
        double d;
    } u;
    int type;
} mnumber;

#define MN_INTEGER 1            /* mnumber is integer */
#define MN_FLOAT   2            /* mnumber is floating point */

typedef struct mathfunc *MathFunc;
typedef mnumber (*NumMathFunc)(char *, int, mnumber *, int);
typedef mnumber (*StrMathFunc)(char *, char *, int);

struct mathfunc {
    MathFunc next;
    char *name;
    int flags;
    NumMathFunc nfunc;
    StrMathFunc sfunc;
    char *module;
    int minargs;
    int maxargs;
    int funcid;
};

#define MFF_STR      1
#define MFF_ADDED    2

#define NUMMATHFUNC(name, func, min, max, id) \
    { NULL, name, 0, func, NULL, NULL, min, max, id }
#define STRMATHFUNC(name, func, id) \
    { NULL, name, MFF_STR, NULL, func, NULL, 0, 0, id }

/* Character tokens are sometimes casted to (unsigned char)'s.         * 
 * Unfortunately, some compilers don't correctly cast signed to        * 
 * unsigned promotions; i.e. (int)(unsigned char)((char) -1) evaluates * 
 * to -1, instead of 255 like it should.  We circumvent the troubles   * 
 * of such shameful delinquency by casting to a larger unsigned type   * 
 * then back down to unsigned char.                                    */

#ifdef BROKEN_SIGNED_TO_UNSIGNED_CASTING
# define STOUC(X)       ((unsigned char)(unsigned short)(X))
#else
# define STOUC(X)       ((unsigned char)(X))
#endif

/* Meta together with the character following Meta denotes the character *
 * which is the exclusive or of 32 and the character following Meta.     *
 * This is used to represent characters which otherwise has special      *
 * meaning for zsh.  These are the characters for which the imeta() test *
 * is true: the null character, and the characters from Meta to Marker.  */

#define Meta            ((char) 0x83)

/* Note that the fourth character in DEFAULT_IFS is Meta *
 * followed by a space which denotes the null character. */

#define DEFAULT_IFS     " \t\n\203 "

/* Character tokens */
#define Pound           ((char) 0x84)
#define String          ((char) 0x85)
#define Hat             ((char) 0x86)
#define Star            ((char) 0x87)
#define Inpar           ((char) 0x88)
#define Outpar          ((char) 0x89)
#define Qstring         ((char) 0x8a)
#define Equals          ((char) 0x8b)
#define Bar             ((char) 0x8c)
#define Inbrace         ((char) 0x8d)
#define Outbrace        ((char) 0x8e)
#define Inbrack         ((char) 0x8f)
#define Outbrack        ((char) 0x90)
#define Tick            ((char) 0x91)
#define Inang           ((char) 0x92)
#define Outang          ((char) 0x93)
#define Quest           ((char) 0x94)
#define Tilde           ((char) 0x95)
#define Qtick           ((char) 0x96)
#define Comma           ((char) 0x97)
#define Snull           ((char) 0x98)
#define Dnull           ((char) 0x99)
#define Bnull           ((char) 0x9a)
#define Nularg          ((char) 0x9b)

#define INULL(x)        (((x) & 0xfc) == 0x98)

/* Marker used in paramsubst for rc_expand_param */
#define Marker          ((char) 0x9c)

/* chars that need to be quoted if meant literally */

#define SPECCHARS "#$^*()=|{}[]`<>?~;&\n\t \\\'\""

enum {
    NULLTOK,            /* 0  */
    SEPER,
    NEWLIN,
    SEMI,
    DSEMI,
    AMPER,              /* 5  */
    INPAR,
    OUTPAR,
    DBAR,
    DAMPER,
    OUTANG,             /* 10 */
    OUTANGBANG,
    DOUTANG,
    DOUTANGBANG,
    INANG,
    INOUTANG,           /* 15 */
    DINANG,
    DINANGDASH,
    INANGAMP,
    OUTANGAMP,
    AMPOUTANG,          /* 20 */
    OUTANGAMPBANG,
    DOUTANGAMP,
    DOUTANGAMPBANG,
    TRINANG,
    BAR,                /* 25 */
    BARAMP,
    INOUTPAR,
    DINPAR,
    DOUTPAR,
    AMPERBANG,          /* 30 */
    SEMIAMP,
    DOUTBRACK,
    STRING,
    ENVSTRING,
    ENVARRAY,           /* 35 */
    ENDINPUT,
    LEXERR,

    /* Tokens for reserved words */
    BANG,       /* !         */
    DINBRACK,   /* [[        */
    INBRACE,    /* {         */ /* 40 */
    OUTBRACE,   /* }         */
    CASE,       /* case      */
    COPROC,     /* coproc    */
    DO,         /* do        */
    DONE,       /* done      */ /* 45 */
    ELIF,       /* elif      */
    ELSE,       /* else      */
    ZEND,       /* end       */
    ESAC,       /* esac      */
    FI,         /* fi        */ /* 50 */
    FOR,        /* for       */
    FOREACH,    /* foreach   */
    FUNC,       /* function  */
    IF,         /* if        */
    NOCORRECT,  /* nocorrect */ /* 55 */
    REPEAT,     /* repeat    */
    SELECT,     /* select    */
    THEN,       /* then      */
    TIME,       /* time      */
    UNTIL,      /* until     */ /* 60 */
    WHILE       /* while     */
};

/* Redirection types.  If you modify this, you may also have to modify *
 * redirtab in parse.c and getredirs() in text.c and the IS_* macros   *
 * below.                                                              */

enum {
    REDIR_WRITE,                /* > */
    REDIR_WRITENOW,             /* >| */
    REDIR_APP,          /* >> */
    REDIR_APPNOW,               /* >>| */
    REDIR_ERRWRITE,             /* &>, >& */
    REDIR_ERRWRITENOW,  /* >&| */
    REDIR_ERRAPP,               /* >>& */
    REDIR_ERRAPPNOW,            /* >>&| */
    REDIR_READWRITE,            /* <> */
    REDIR_READ,         /* < */
    REDIR_HEREDOC,              /* << */
    REDIR_HEREDOCDASH,  /* <<- */
    REDIR_HERESTR,              /* <<< */
    REDIR_MERGEIN,              /* <&n */
    REDIR_MERGEOUT,             /* >&n */
    REDIR_CLOSE,                /* >&-, <&- */
    REDIR_INPIPE,               /* < <(...) */
    REDIR_OUTPIPE               /* > >(...) */
};

#define IS_WRITE_FILE(X)      ((X)>=REDIR_WRITE && (X)<=REDIR_READWRITE)
#define IS_APPEND_REDIR(X)    (IS_WRITE_FILE(X) && ((X) & 2))
#define IS_CLOBBER_REDIR(X)   (IS_WRITE_FILE(X) && ((X) & 1))
#define IS_ERROR_REDIR(X)     ((X)>=REDIR_ERRWRITE && (X)<=REDIR_ERRAPPNOW)
#define IS_READFD(X)          (((X)>=REDIR_READWRITE && (X)<=REDIR_MERGEIN) || (X)==REDIR_INPIPE)
#define IS_REDIROP(X)         ((X)>=OUTANG && (X)<=TRINANG)

/* Flags for input stack */
#define INP_FREE      (1<<0)    /* current buffer can be free'd            */
#define INP_ALIAS     (1<<1)    /* expanding alias or history              */
#define INP_HIST      (1<<2)    /* expanding history                       */
#define INP_CONT      (1<<3)    /* continue onto previously stacked input  */
#define INP_ALCONT    (1<<4)    /* stack is continued from alias expn.     */
#define INP_LINENO    (1<<5)    /* update line number                      */

/* Flags for metafy */
#define META_REALLOC    0
#define META_USEHEAP    1
#define META_STATIC     2
#define META_DUP        3
#define META_ALLOC      4
#define META_NOALLOC    5
#define META_HEAPDUP    6
#define META_HREALLOC   7


/**************************/
/* Abstract types for zsh */
/**************************/

typedef struct linknode  *LinkNode;
typedef struct linklist  *LinkList;
typedef struct hashnode  *HashNode;
typedef struct hashtable *HashTable;

typedef struct optname   *Optname;
typedef struct reswd     *Reswd;
typedef struct alias     *Alias;
typedef struct param     *Param;
typedef struct paramdef  *Paramdef;
typedef struct cmdnam    *Cmdnam;
typedef struct shfunc    *Shfunc;
typedef struct funcstack *Funcstack;
typedef struct funcwrap  *FuncWrap;
typedef struct builtin   *Builtin;
typedef struct nameddir  *Nameddir;
typedef struct module    *Module;
typedef struct linkedmod *Linkedmod;

typedef struct patprog   *Patprog;
typedef struct process   *Process;
typedef struct job       *Job;
typedef struct value     *Value;
typedef struct conddef   *Conddef;
typedef struct redir     *Redir;
typedef struct complist  *Complist;
typedef struct heap      *Heap;
typedef struct heapstack *Heapstack;
typedef struct histent   *Histent;
typedef struct hookdef   *Hookdef;

typedef struct asgment   *Asgment;


/********************************/
/* Definitions for linked lists */
/********************************/

/* linked list abstract data type */

struct linknode {
    LinkNode next;
    LinkNode last;
    void *dat;
};

struct linklist {
    LinkNode first;
    LinkNode last;
};

/* Macros for manipulating link lists */

#define addlinknode(X,Y)    insertlinknode(X,(X)->last,Y)
#define zaddlinknode(X,Y)   zinsertlinknode(X,(X)->last,Y)
#define uaddlinknode(X,Y)   uinsertlinknode(X,(X)->last,Y)
#define empty(X)            ((X)->first == NULL)
#define nonempty(X)         ((X)->first != NULL)
#define firstnode(X)        ((X)->first)
#define getaddrdata(X)      (&((X)->dat))
#define getdata(X)          ((X)->dat)
#define setdata(X,Y)        ((X)->dat = (Y))
#define lastnode(X)         ((X)->last)
#define nextnode(X)         ((X)->next)
#define prevnode(X)         ((X)->last)
#define peekfirst(X)        ((X)->first->dat)
#define pushnode(X,Y)       insertlinknode(X,(LinkNode) X,Y)
#define zpushnode(X,Y)      zinsertlinknode(X,(LinkNode) X,Y)
#define incnode(X)          (X = nextnode(X))
#define firsthist()         (hist_ring? hist_ring->down->histnum : curhist)
#define setsizednode(X,Y,Z) ((X)->first[(Y)].dat = (void *) (Z))

/* stack allocated linked lists */

#define local_list0(N) struct linklist N
#define init_list0(N) \
    do { \
        (N).first = NULL; \
        (N).last = (LinkNode) &(N); \
    } while (0)
#define local_list1(N) struct linklist N; struct linknode __n0
#define init_list1(N,V0) \
    do { \
        (N).first = &__n0; \
        (N).last = &__n0; \
        __n0.next = NULL; \
        __n0.last = (LinkNode) &(N); \
        __n0.dat = (void *) (V0); \
    } while (0)

/********************************/
/* Definitions for syntax trees */
/********************************/

/* These are control flags that are passed *
 * down the execution pipeline.            */
#define Z_TIMED  (1<<0) /* pipeline is being timed                   */
#define Z_SYNC   (1<<1) /* run this sublist synchronously       (;)  */
#define Z_ASYNC  (1<<2) /* run this sublist asynchronously      (&)  */
#define Z_DISOWN (1<<3) /* run this sublist without job control (&|) */
/* (1<<4) is used for Z_END, see the wordcode definitions */
/* (1<<5) is used for Z_SIMPLE, see the wordcode definitions */

/* Condition types. */

#define COND_NOT    0
#define COND_AND    1
#define COND_OR     2
#define COND_STREQ  3
#define COND_STRNEQ 4
#define COND_STRLT  5
#define COND_STRGTR 6
#define COND_NT     7
#define COND_OT     8
#define COND_EF     9
#define COND_EQ    10
#define COND_NE    11
#define COND_LT    12
#define COND_GT    13
#define COND_LE    14
#define COND_GE    15
#define COND_MOD   16
#define COND_MODI  17

typedef int (*CondHandler) _((char **, int));

struct conddef {
    Conddef next;               /* next in list                       */
    char *name;                 /* the condition name                 */
    int flags;                  /* see CONDF_* below                  */
    CondHandler handler;        /* handler function                   */
    int min;                    /* minimum number of strings          */
    int max;                    /* maximum number of strings          */
    int condid;                 /* for overloading handler functions  */
    char *module;               /* module to autoload                 */
};

#define CONDF_INFIX  1
#define CONDF_ADDED  2

#define CONDDEF(name, flags, handler, min, max, condid) \
    { NULL, name, flags, handler, min, max, condid, NULL }

/* tree element for redirection lists */

struct redir {
    int type;
    int fd1, fd2;
    char *name;
};

/* The number of fds space is allocated for  *
 * each time a multio must increase in size. */
#define MULTIOUNIT 8

/* A multio is a list of fds associated with a certain fd.       *
 * Thus if you do "foo >bar >ble", the multio for fd 1 will have *
 * two fds, the result of open("bar",...), and the result of     *
 * open("ble",....).                                             */

/* structure used for multiple i/o redirection */
/* one for each fd open                        */

struct multio {
    int ct;                     /* # of redirections on this fd                 */
    int rflag;                  /* 0 if open for reading, 1 if open for writing */
    int pipe;                   /* fd of pipe if ct > 1                         */
    int fds[MULTIOUNIT];        /* list of src/dests redirected to/from this fd */
};

/* structure for foo=bar assignments */

struct asgment {
    struct asgment *next;
    char *name;
    char *value;
};

/* lvalue for variable assignment/expansion */

struct value {
    int isarr;
    Param pm;           /* parameter node                      */
    int inv;            /* should we return the index ?        */
    int start;          /* first element of array slice, or -1 */
    int end;            /* 1-rel last element of array slice, or -1 */
    char **arr;         /* cache for hash turned into array */
};

#define MAX_ARRLEN    262144

/********************************************/
/* Defintions for word code                 */
/********************************************/

typedef unsigned int wordcode;
typedef wordcode *Wordcode;

typedef struct funcdump *FuncDump;
typedef struct eprog *Eprog;

struct funcdump {
    FuncDump next;              /* next in list */
    dev_t dev;                  /* device */
    ino_t ino;                  /* indoe number */
    int fd;                     /* file descriptor */
    Wordcode map;               /* pointer to header */
    Wordcode addr;              /* mapped region */
    int len;                    /* length */
    int count;                  /* reference count */
};

struct eprog {
    int flags;                  /* EF_* below */
    int len;                    /* total block length */
    int npats;                  /* Patprog cache size */
    Patprog *pats;              /* the memory block, the patterns */
    Wordcode prog;              /* memory block ctd, the code */
    char *strs;                 /* memory block ctd, the strings */
    Shfunc shf;                 /* shell function for autoload */
    FuncDump dump;              /* dump file this is in */
};

#define EF_REAL 1
#define EF_HEAP 2
#define EF_MAP  4
#define EF_RUN  8

typedef struct estate *Estate;

struct estate {
    Eprog prog;                 /* the eprog executed */
    Wordcode pc;                /* program counter, current pos */
    char *strs;                 /* strings from prog */
};

typedef struct eccstr *Eccstr;

struct eccstr {
    Eccstr left, right;
    char *str;
    wordcode offs, aoffs;
    int nfunc;
};

#define EC_NODUP  0
#define EC_DUP    1
#define EC_DUPTOK 2

#define WC_CODEBITS 5

#define wc_code(C)   ((C) & ((wordcode) ((1 << WC_CODEBITS) - 1)))
#define wc_data(C)   ((C) >> WC_CODEBITS)
#define wc_bdata(D)  ((D) << WC_CODEBITS)
#define wc_bld(C,D)  (((wordcode) (C)) | (((wordcode) (D)) << WC_CODEBITS))

#define WC_END      0
#define WC_LIST     1
#define WC_SUBLIST  2
#define WC_PIPE     3
#define WC_REDIR    4
#define WC_ASSIGN   5
#define WC_SIMPLE   6
#define WC_SUBSH    7
#define WC_CURSH    8
#define WC_TIMED    9
#define WC_FUNCDEF 10
#define WC_FOR     11
#define WC_SELECT  12
#define WC_WHILE   13
#define WC_REPEAT  14
#define WC_CASE    15
#define WC_IF      16
#define WC_COND    17
#define WC_ARITH   18
#define WC_AUTOFN  19

#define WCB_END()           wc_bld(WC_END, 0)

#define WC_LIST_TYPE(C)     wc_data(C)
#define Z_END               (1<<4) 
#define Z_SIMPLE            (1<<5)
#define WC_LIST_SKIP(C)     (wc_data(C) >> 6)
#define WCB_LIST(T,O)       wc_bld(WC_LIST, ((T) | ((O) << 6)))

#define WC_SUBLIST_TYPE(C)  (wc_data(C) & ((wordcode) 3))
#define WC_SUBLIST_END      0
#define WC_SUBLIST_AND      1
#define WC_SUBLIST_OR       2
#define WC_SUBLIST_FLAGS(C) (wc_data(C) & ((wordcode) 0x1c))
#define WC_SUBLIST_COPROC   4
#define WC_SUBLIST_NOT      8
#define WC_SUBLIST_SIMPLE  16
#define WC_SUBLIST_SKIP(C)  (wc_data(C) >> 5)
#define WCB_SUBLIST(T,F,O)  wc_bld(WC_SUBLIST, ((T) | (F) | ((O) << 5)))

#define WC_PIPE_TYPE(C)     (wc_data(C) & ((wordcode) 1))
#define WC_PIPE_END         0
#define WC_PIPE_MID         1
#define WC_PIPE_LINENO(C)   (wc_data(C) >> 1)
#define WCB_PIPE(T,L)       wc_bld(WC_PIPE, ((T) | ((L) << 1)))

#define WC_REDIR_TYPE(C)    wc_data(C)
#define WCB_REDIR(T)        wc_bld(WC_REDIR, (T))

#define WC_ASSIGN_TYPE(C)   (wc_data(C) & ((wordcode) 1))
#define WC_ASSIGN_SCALAR    0
#define WC_ASSIGN_ARRAY     1
#define WC_ASSIGN_NUM(C)    (wc_data(C) >> 1)
#define WCB_ASSIGN(T,N)     wc_bld(WC_ASSIGN, ((T) | ((N) << 1)))

#define WC_SIMPLE_ARGC(C)   wc_data(C)
#define WCB_SIMPLE(N)       wc_bld(WC_SIMPLE, (N))

#define WC_SUBSH_SKIP(C)    wc_data(C)
#define WCB_SUBSH(O)        wc_bld(WC_SUBSH, (O))

#define WC_CURSH_SKIP(C)    wc_data(C)
#define WCB_CURSH(O)        wc_bld(WC_CURSH, (O))

#define WC_TIMED_TYPE(C)    wc_data(C)
#define WC_TIMED_EMPTY      0
#define WC_TIMED_PIPE       1
#define WCB_TIMED(T)        wc_bld(WC_TIMED, (T))

#define WC_FUNCDEF_SKIP(C)  wc_data(C)
#define WCB_FUNCDEF(O)      wc_bld(WC_FUNCDEF, (O))

#define WC_FOR_TYPE(C)      (wc_data(C) & 3)
#define WC_FOR_PPARAM       0
#define WC_FOR_LIST         1
#define WC_FOR_COND         2
#define WC_FOR_SKIP(C)      (wc_data(C) >> 2)
#define WCB_FOR(T,O)        wc_bld(WC_FOR, ((T) | ((O) << 2)))

#define WC_SELECT_TYPE(C)   (wc_data(C) & 1)
#define WC_SELECT_PPARAM    0
#define WC_SELECT_LIST      1
#define WC_SELECT_SKIP(C)   (wc_data(C) >> 1)
#define WCB_SELECT(T,O)     wc_bld(WC_SELECT, ((T) | ((O) << 1)))

#define WC_WHILE_TYPE(C)    (wc_data(C) & 1)
#define WC_WHILE_WHILE      0
#define WC_WHILE_UNTIL      1
#define WC_WHILE_SKIP(C)    (wc_data(C) >> 1)
#define WCB_WHILE(T,O)      wc_bld(WC_WHILE, ((T) | ((O) << 1)))

#define WC_REPEAT_SKIP(C)   wc_data(C)
#define WCB_REPEAT(O)       wc_bld(WC_REPEAT, (O))

#define WC_CASE_TYPE(C)     (wc_data(C) & 3)
#define WC_CASE_HEAD        0
#define WC_CASE_OR          1
#define WC_CASE_AND         2
#define WC_CASE_SKIP(C)     (wc_data(C) >> 2)
#define WCB_CASE(T,O)       wc_bld(WC_CASE, ((T) | ((O) << 2)))

#define WC_IF_TYPE(C)       (wc_data(C) & 3)
#define WC_IF_HEAD          0
#define WC_IF_IF            1
#define WC_IF_ELIF          2
#define WC_IF_ELSE          3
#define WC_IF_SKIP(C)       (wc_data(C) >> 2)
#define WCB_IF(T,O)         wc_bld(WC_IF, ((T) | ((O) << 2)))

#define WC_COND_TYPE(C)     (wc_data(C) & 127)
#define WC_COND_SKIP(C)     (wc_data(C) >> 7)
#define WCB_COND(T,O)       wc_bld(WC_COND, ((T) | ((O) << 7)))

#define WCB_ARITH()         wc_bld(WC_ARITH, 0)

#define WCB_AUTOFN()        wc_bld(WC_AUTOFN, 0)

/********************************************/
/* Defintions for job table and job control */
/********************************************/

#ifdef NEED_LINUX_TASKS_H
#include <linux/tasks.h>
#endif

/* entry in the job table */

struct job {
    pid_t gleader;              /* process group leader of this job  */
    pid_t other;                /* subjob id or subshell pid         */
    int stat;                   /* see STATs below                   */
    char *pwd;                  /* current working dir of shell when *
                                 * this job was spawned              */
    struct process *procs;      /* list of processes                 */
    LinkList filelist;          /* list of files to delete when done */
    int stty_in_env;            /* if STTY=... is present            */
    struct ttyinfo *ty;         /* the modes specified by STTY       */
};

#define STAT_CHANGED    (1<<0)  /* status changed and not reported      */
#define STAT_STOPPED    (1<<1)  /* all procs stopped or exited          */
#define STAT_TIMED      (1<<2)  /* job is being timed                   */
#define STAT_DONE       (1<<3)  /* job is done                          */
#define STAT_LOCKED     (1<<4)  /* shell is finished creating this job, */
                                /*   may be deleted from job table      */
#define STAT_NOPRINT    (1<<5)  /* job was killed internally,           */
                                /*   we don't want to show that         */
#define STAT_INUSE      (1<<6)  /* this job entry is in use             */
#define STAT_SUPERJOB   (1<<7)  /* job has a subjob                     */
#define STAT_SUBJOB     (1<<8)  /* job is a subjob                      */
#define STAT_WASSUPER   (1<<9)  /* was a super-job, sub-job needs to be */
                                /* deleted */
#define STAT_CURSH      (1<<10) /* last command is in current shell     */
#define STAT_NOSTTY     (1<<11) /* the tty settings are not inherited   */
                                /* from this job when it exits.         */
#define STAT_ATTACH     (1<<12) /* delay reattaching shell to tty       */
#define STAT_SUBLEADER  (1<<13) /* is super-job, but leader is sub-shell */

#define SP_RUNNING -1           /* fake status for jobs currently running */

struct timeinfo {
    long ut;                    /* user space time   */
    long st;                    /* system space time */
};

#define JOBTEXTSIZE 80

/* node in job process lists */

struct process {
    struct process *next;
    pid_t pid;                  /* process id                       */
    char text[JOBTEXTSIZE];     /* text to print when 'jobs' is run */
    int status;                 /* return code from waitpid/wait3() */
    struct timeinfo ti;
    struct timeval bgtime;      /* time job was spawned             */
    struct timeval endtime;     /* time job exited                  */
};

struct execstack {
    struct execstack *next;

    LinkList args;
    pid_t list_pipe_pid;
    int nowait;
    int pline_level;
    int list_pipe_child;
    int list_pipe_job;
    char list_pipe_text[JOBTEXTSIZE];
    int lastval;
    int noeval;
    int badcshglob;
    pid_t cmdoutpid;
    int cmdoutval;
    int trapreturn;
    int noerrs;
    int subsh_close;
    char *underscore;
};

struct heredocs {
    struct heredocs *next;
    int type;
    int pc;
    char *str;
};

struct dirsav {
    int dirfd, level;
    char *dirname;
    dev_t dev;
    ino_t ino;
};

#define MAX_PIPESTATS 256

/*******************************/
/* Definitions for Hash Tables */
/*******************************/

typedef void *(*VFunc) _((void *));
typedef void (*FreeFunc) _((void *));

typedef unsigned (*HashFunc)       _((char *));
typedef void     (*TableFunc)      _((HashTable));
typedef void     (*AddNodeFunc)    _((HashTable, char *, void *));
typedef HashNode (*GetNodeFunc)    _((HashTable, char *));
typedef HashNode (*RemoveNodeFunc) _((HashTable, char *));
typedef void     (*FreeNodeFunc)   _((HashNode));
typedef int      (*CompareFunc)    _((const char *, const char *));

/* type of function that is passed to *
 * scanhashtable or scanmatchtable    */
typedef void     (*ScanFunc)       _((HashNode, int));
typedef void     (*ScanTabFunc)    _((HashTable, ScanFunc, int));

typedef void (*PrintTableStats) _((HashTable));

/* hash table for standard open hashing */

struct hashtable {
    /* HASHTABLE DATA */
    int hsize;                  /* size of nodes[]  (number of hash values)   */
    int ct;                     /* number of elements                         */
    HashNode *nodes;            /* array of size hsize                        */

    /* HASHTABLE METHODS */
    HashFunc hash;              /* pointer to hash function for this table    */
    TableFunc emptytable;       /* pointer to function to empty table         */
    TableFunc filltable;        /* pointer to function to fill table          */
    CompareFunc cmpnodes;       /* pointer to function to compare two nodes     */
    AddNodeFunc addnode;        /* pointer to function to add new node        */
    GetNodeFunc getnode;        /* pointer to function to get an enabled node */
    GetNodeFunc getnode2;       /* pointer to function to get node            */
                                /* (getnode2 will ignore DISABLED flag)       */
    RemoveNodeFunc removenode;  /* pointer to function to delete a node       */
    ScanFunc disablenode;       /* pointer to function to disable a node      */
    ScanFunc enablenode;        /* pointer to function to enable a node       */
    FreeNodeFunc freenode;      /* pointer to function to free a node         */
    ScanFunc printnode;         /* pointer to function to print a node        */
    ScanTabFunc scantab;        /* pointer to function to scan table          */

#ifdef HASHTABLE_INTERNAL_MEMBERS
    HASHTABLE_INTERNAL_MEMBERS  /* internal use in hashtable.c                */
#endif
};

/* generic hash table node */

struct hashnode {
    HashNode next;              /* next in hash chain */
    char *nam;                  /* hash key           */
    int flags;                  /* various flags      */
};

/* The flag to disable nodes in a hash table.  Currently  *
 * you can disable builtins, shell functions, aliases and *
 * reserved words.                                        */
#define DISABLED        (1<<0)

/* node in shell option table */

struct optname {
    HashNode next;              /* next in hash chain */
    char *nam;                  /* hash data */
    int flags;
    int optno;                  /* option number */
};

/* node in shell reserved word hash table (reswdtab) */

struct reswd {
    HashNode next;              /* next in hash chain        */
    char *nam;                  /* name of reserved word     */
    int flags;                  /* flags                     */
    int token;                  /* corresponding lexer token */
};

/* node in alias hash table (aliastab) */

struct alias {
    HashNode next;              /* next in hash chain       */
    char *nam;                  /* hash data                */
    int flags;                  /* flags for alias types    */
    char *text;                 /* expansion of alias       */
    int inuse;                  /* alias is being expanded  */
};

/* is this alias global */
#define ALIAS_GLOBAL    (1<<1)

/* node in command path hash table (cmdnamtab) */

struct cmdnam {
    HashNode next;              /* next in hash chain */
    char *nam;                  /* hash data          */
    int flags;
    union {
        char **name;            /* full pathname for external commands */
        char *cmd;              /* file name for hashed commands       */
    }
    u;
};

/* flag for nodes explicitly added to *
 * cmdnamtab with hash builtin        */
#define HASHED          (1<<1)

/* node in shell function hash table (shfunctab) */

struct shfunc {
    HashNode next;              /* next in hash chain     */
    char *nam;                  /* name of shell function */
    int flags;                  /* various flags          */
    Eprog funcdef;              /* function definition    */
};

/* Shell function context types. */

#define SFC_NONE     0          /* no function running */
#define SFC_DIRECT   1          /* called directly from the user */
#define SFC_SIGNAL   2          /* signal handler */
#define SFC_HOOK     3          /* one of the special functions */
#define SFC_WIDGET   4          /* user defined widget */
#define SFC_COMPLETE 5          /* called from completion code */
#define SFC_CWIDGET  6          /* new style completion widget */

/* node in function stack */

struct funcstack {
    Funcstack prev;             /* previous in stack */
    char *name;                 /* name of function called */
};

/* node in list of function call wrappers */

typedef int (*WrapFunc) _((Eprog, FuncWrap, char *));

struct funcwrap {
    FuncWrap next;
    int flags;
    WrapFunc handler;
    Module module;
};

#define WRAPF_ADDED 1

#define WRAPDEF(func) \
    { NULL, 0, func, NULL }

/* node in builtin command hash table (builtintab) */

typedef int (*HandlerFunc) _((char *, char **, char *, int));
#define NULLBINCMD ((HandlerFunc) 0)

struct builtin {
    HashNode next;              /* next in hash chain                                 */
    char *nam;                  /* name of builtin                                    */
    int flags;                  /* various flags                                      */
    HandlerFunc handlerfunc;    /* pointer to function that executes this builtin     */
    int minargs;                /* minimum number of arguments                        */
    int maxargs;                /* maximum number of arguments, or -1 for no limit    */
    int funcid;                 /* xbins (see above) for overloaded handlerfuncs      */
    char *optstr;               /* string of legal options                            */
    char *defopts;              /* options set by default for overloaded handlerfuncs */
};

#define BUILTIN(name, flags, handler, min, max, funcid, optstr, defopts) \
    { NULL, name, flags, handler, min, max, funcid, optstr, defopts }
#define BIN_PREFIX(name, flags) \
    BUILTIN(name, flags | BINF_PREFIX, NULLBINCMD, 0, 0, 0, NULL, NULL)

/* builtin flags */
/* DISABLE IS DEFINED AS (1<<0) */
#define BINF_PLUSOPTS           (1<<1)  /* +xyz legal */
#define BINF_R                  (1<<2)  /* this is the builtin `r' (fc -e -) */
#define BINF_PRINTOPTS          (1<<3)
#define BINF_ADDED              (1<<4)  /* is in the builtins hash table */
#define BINF_FCOPTS             (1<<5)
#define BINF_TYPEOPT            (1<<6)
#define BINF_ECHOPTS            (1<<7)
#define BINF_MAGICEQUALS        (1<<8)  /* needs automatic MAGIC_EQUAL_SUBST substitution */
#define BINF_PREFIX             (1<<9)
#define BINF_DASH               (1<<10)
#define BINF_BUILTIN            (1<<11)
#define BINF_COMMAND            (1<<12)
#define BINF_EXEC               (1<<13)
#define BINF_NOGLOB             (1<<14)
#define BINF_PSPECIAL           (1<<15)

#define BINF_TYPEOPTS   (BINF_TYPEOPT|BINF_PLUSOPTS)

struct module {
    char *nam;
    int flags;
    union {
        void *handle;
        Linkedmod linked;
        char *alias;
    } u;
    LinkList deps;
    int wrapper;
};

#define MOD_BUSY    (1<<0)
#define MOD_UNLOAD  (1<<1)
#define MOD_SETUP   (1<<2)
#define MOD_LINKED  (1<<3)
#define MOD_INIT_S  (1<<4)
#define MOD_INIT_B  (1<<5)
#define MOD_ALIAS   (1<<6)

typedef int (*Module_func) _((Module));

struct linkedmod {
    char *name;
    Module_func setup;
    Module_func boot;
    Module_func cleanup;
    Module_func finish;
};

/* C-function hooks */

typedef int (*Hookfn) _((Hookdef, void *));

struct hookdef {
    Hookdef next;
    char *name;
    Hookfn def;
    int flags;
    LinkList funcs;
};

#define HOOKF_ALL 1

#define HOOKDEF(name, func, flags) { NULL, name, (Hookfn) func, flags, NULL }

/*
 * Types used in pattern matching.  Most of these longs could probably
 * happily be ints.
 */

struct patprog {
    long                startoff;  /* length before start of programme */
    long                size;      /* total size from start of struct */
    long                mustoff;   /* offset to string that must be present */
    int                 globflags; /* globbing flags to set at start */
    int                 globend;   /* globbing flags set after finish */
    int                 flags;     /* PAT_* flags */
    int                 patmlen;   /* length of pure string or longest match */
    int                 patnpar;   /* number of active parentheses */
    char                patstartch;
};

/* Flags used in pattern matchers (Patprog) and passed down to patcompile */

#define PAT_FILE        0x0001  /* Pattern is a file name */
#define PAT_FILET       0x0002  /* Pattern is top level file, affects ~ */
#define PAT_ANY         0x0004  /* Match anything (cheap "*") */
#define PAT_NOANCH      0x0008  /* Not anchored at end */
#define PAT_NOGLD       0x0010  /* Don't glob dots */
#define PAT_PURES       0x0020  /* Pattern is a pure string: set internally */
#define PAT_STATIC      0x0040  /* Don't copy pattern to heap as per default */
#define PAT_SCAN        0x0080  /* Scanning, so don't try must-match test */
#define PAT_ZDUP        0x0100  /* Copy pattern in real memory */
#define PAT_NOTSTART    0x0200  /* Start of string is not real start */
#define PAT_NOTEND      0x0400  /* End of string is not real end */

/* Globbing flags: lower 8 bits gives approx count */
#define GF_LCMATCHUC    0x0100
#define GF_IGNCASE      0x0200
#define GF_BACKREF      0x0400
#define GF_MATCHREF     0x0800

/* Dummy Patprog pointers. Used mainly in executable code, but the
 * pattern code needs to know about it, too. */

#define dummy_patprog1 ((Patprog) 1)
#define dummy_patprog2 ((Patprog) 2)

/* node used in parameter hash table (paramtab) */

struct param {
    HashNode next;              /* next in hash chain */
    char *nam;                  /* hash data          */
    int flags;                  /* PM_* flags         */

    /* the value of this parameter */
    union {
        void *data;             /* used by special parameter functions    */
        char **arr;             /* value if declared array   (PM_ARRAY)   */
        char *str;              /* value if declared string  (PM_SCALAR)  */
        zlong val;              /* value if declared integer (PM_INTEGER) */
        double dval;            /* value if declared float
                                                    (PM_EFLOAT|PM_FFLOAT) */
        HashTable hash;         /* value if declared assoc   (PM_HASHED)  */
    } u;

    /* pointer to function to set value of this parameter */
    union {
        void (*cfn) _((Param, char *));
        void (*ifn) _((Param, zlong));
        void (*ffn) _((Param, double));
        void (*afn) _((Param, char **));
        void (*hfn) _((Param, HashTable));
    } sets;

    /* pointer to function to get value of this parameter */
    union {
        char *(*cfn) _((Param));
        zlong (*ifn) _((Param));
        double (*ffn) _((Param));
        char **(*afn) _((Param));
        HashTable (*hfn) _((Param));
    } gets;

    /* pointer to function to unset this parameter */
    void (*unsetfn) _((Param, int));

    int ct;                     /* output base or field width            */
    char *env;                  /* location in environment, if exported  */
    char *ename;                /* name of corresponding environment var */
    Param old;                  /* old struct for use with local         */
    int level;                  /* if (old != NULL), level of localness  */
};

/* flags for parameters */

/* parameter types */
#define PM_SCALAR       0       /* scalar                                   */
#define PM_ARRAY        (1<<0)  /* array                                    */
#define PM_INTEGER      (1<<1)  /* integer                                  */
#define PM_EFLOAT       (1<<2)  /* double with %e output                    */
#define PM_FFLOAT       (1<<3)  /* double with %f output                    */
#define PM_HASHED       (1<<4)  /* association                              */

#define PM_TYPE(X) \
  (X & (PM_SCALAR|PM_INTEGER|PM_EFLOAT|PM_FFLOAT|PM_ARRAY|PM_HASHED))

#define PM_LEFT         (1<<5)  /* left justify, remove leading blanks      */
#define PM_RIGHT_B      (1<<6)  /* right justify, fill with leading blanks  */
#define PM_RIGHT_Z      (1<<7)  /* right justify, fill with leading zeros   */
#define PM_LOWER        (1<<8)  /* all lower case                           */

/* The following are the same since they *
 * both represent -u option to typeset   */
#define PM_UPPER        (1<<9)  /* all upper case                           */
#define PM_UNDEFINED    (1<<9)  /* undefined (autoloaded) shell function    */

#define PM_READONLY     (1<<10) /* readonly                                 */
#define PM_TAGGED       (1<<11) /* tagged                                   */
#define PM_EXPORTED     (1<<12) /* exported                                 */

/* The following are the same since they *
 * both represent -U option to typeset   */
#define PM_UNIQUE       (1<<13) /* remove duplicates                        */
#define PM_UNALIASED    (1<<13) /* do not expand aliases when autoloading   */

#define PM_HIDE         (1<<14) /* Special behaviour hidden by local        */
#define PM_HIDEVAL      (1<<15) /* Value not shown in `typeset' commands    */
#define PM_TIED         (1<<16) /* array tied to colon-path or v.v.         */

/* Remaining flags do not correspond directly to command line arguments */
#define PM_LOCAL        (1<<17) /* this parameter will be made local        */
#define PM_SPECIAL      (1<<18) /* special builtin parameter                */
#define PM_DONTIMPORT   (1<<19) /* do not import this variable              */
#define PM_RESTRICTED   (1<<20) /* cannot be changed in restricted mode     */
#define PM_UNSET        (1<<21) /* has null value                           */
#define PM_REMOVABLE    (1<<22) /* special can be removed from paramtab     */
#define PM_AUTOLOAD     (1<<23) /* autoloaded from module                   */
#define PM_NORESTORE    (1<<24) /* do not restore value of local special    */
#define PM_HASHELEM     (1<<25) /* is a hash-element */
#define PM_NAMEDDIR     (1<<26) /* has a corresponding nameddirtab entry    */

/* The option string corresponds to the first of the variables above */
#define TYPESET_OPTSTR "aiEFALRZlurtxUhHT"

/* These typeset options take an optional numeric argument */
#define TYPESET_OPTNUM "LRZiEF"

/* Flags for extracting elements of arrays and associative arrays */
#define SCANPM_WANTVALS   (1<<0)
#define SCANPM_WANTKEYS   (1<<1)
#define SCANPM_WANTINDEX  (1<<2)
#define SCANPM_MATCHKEY   (1<<3)
#define SCANPM_MATCHVAL   (1<<4)
#define SCANPM_MATCHMANY  (1<<5)
#define SCANPM_ASSIGNING  (1<<6)
#define SCANPM_KEYMATCH   (1<<7)
#define SCANPM_DQUOTED    (1<<8)
#define SCANPM_ISVAR_AT   ((-1)<<15)    /* Only sign bit is significant */

/*
 * Flags for doing matches inside parameter substitutions, i.e.
 * ${...#...} and friends.  This could be an enum, but so
 * could a lot of other things.
 */

#define SUB_END         0x0001  /* match end instead of begining, % or %%  */
#define SUB_LONG        0x0002  /* % or # doubled, get longest match */
#define SUB_SUBSTR      0x0004  /* match a substring */
#define SUB_MATCH       0x0008  /* include the matched portion */
#define SUB_REST        0x0010  /* include the unmatched portion */
#define SUB_BIND        0x0020  /* index of beginning of string */
#define SUB_EIND        0x0040  /* index of end of string */
#define SUB_LEN         0x0080  /* length of match */
#define SUB_ALL         0x0100  /* match complete string */
#define SUB_GLOBAL      0x0200  /* global substitution ${..//all/these} */
#define SUB_DOSUBST     0x0400  /* replacement string needs substituting */

/* Flags as the second argument to prefork */
#define PF_TYPESET      0x01    /* argument handled like typeset foo=bar */
#define PF_ASSIGN       0x02    /* argument handled like the RHS of foo=bar */
#define PF_SINGLE       0x04    /* single word substitution */

struct paramdef {
    char *name;
    int flags;
    void *var;
    void *set;
    void *get;
    void *unset;
};

#define PARAMDEF(name, flags, var, set, get, unset) \
    { name, flags, (void *) var, (void *) set, (void *) get, (void *) unset }
#define INTPARAMDEF(name, var) \
    { name, PM_INTEGER, (void *) var, (void *) intvarsetfn, \
      (void *) intvargetfn, (void *) stdunsetfn }
#define STRPARAMDEF(name, var) \
    { name, PM_SCALAR, (void *) var, (void *) strvarsetfn, \
      (void *) strvargetfn, (void *) stdunsetfn }
#define ARRPARAMDEF(name, var) \
    { name, PM_ARRAY, (void *) var, (void *) arrvarsetfn, \
      (void *) arrvargetfn, (void *) stdunsetfn }

/* node for named directory hash table (nameddirtab) */

struct nameddir {
    HashNode next;              /* next in hash chain               */
    char *nam;                  /* directory name                   */
    int flags;                  /* see below                        */
    char *dir;                  /* the directory in full            */
    int diff;                   /* strlen(.dir) - strlen(.nam)      */
};

/* flags for named directories */
/* DISABLED is defined (1<<0) */
#define ND_USERNAME     (1<<1)  /* nam is actually a username       */
#define ND_NOABBREV     (1<<2)  /* never print as abbrev (PWD or OLDPWD) */


/* flags for controlling printing of hash table nodes */
#define PRINT_NAMEONLY          (1<<0)
#define PRINT_TYPE              (1<<1)
#define PRINT_LIST              (1<<2)
#define PRINT_KV_PAIR           (1<<3)
#define PRINT_INCLUDEVALUE      (1<<4)

/* flags for printing for the whence builtin */
#define PRINT_WHENCE_CSH        (1<<5)
#define PRINT_WHENCE_VERBOSE    (1<<6)
#define PRINT_WHENCE_SIMPLE     (1<<7)
#define PRINT_WHENCE_FUNCDEF    (1<<9)
#define PRINT_WHENCE_WORD       (1<<10)

/***********************************/
/* Definitions for history control */
/***********************************/

/* history entry */

struct histent {
    HashNode hash_next;         /* next in hash chain               */
    char *text;                 /* the history line itself          */
    int flags;                  /* Misc flags                       */

    Histent up;                 /* previous line (moving upward)    */
    Histent down;               /* next line (moving downward)      */
    char *zle_text;             /* the edited history line          */
    time_t stim;                /* command started time (datestamp) */
    time_t ftim;                /* command finished time            */
    short *words;               /* Position of words in history     */
                                /*   line:  as pairs of start, end  */
    int nwords;                 /* Number of words in history line  */
    int histnum;                /* A sequential history number      */
};

#define HIST_MAKEUNIQUE 0x00000001      /* Kill this new entry if not unique */
#define HIST_OLD        0x00000002      /* Command is already written to disk*/
#define HIST_READ       0x00000004      /* Command was read back from disk*/
#define HIST_DUP        0x00000008      /* Command duplicates a later line */
#define HIST_FOREIGN    0x00000010      /* Command came from another shell */
#define HIST_TMPSTORE   0x00000020      /* Kill when user enters another cmd */

#define GETHIST_UPWARD  (-1)
#define GETHIST_DOWNWARD  1
#define GETHIST_EXACT     0

/* Parts of the code where history expansion is disabled *
 * should be within a pair of STOPHIST ... ALLOWHIST     */

#define STOPHIST (stophist += 4);
#define ALLOWHIST (stophist -= 4);

#define HISTFLAG_DONE   1
#define HISTFLAG_NOEXEC 2
#define HISTFLAG_RECALL 4
#define HISTFLAG_SETTY  8

#define HFILE_APPEND            0x0001
#define HFILE_SKIPOLD           0x0002
#define HFILE_SKIPDUPS          0x0004
#define HFILE_SKIPFOREIGN       0x0008
#define HFILE_FAST              0x0010
#define HFILE_USE_OPTIONS       0x8000

/******************************************/
/* Definitions for programable completion */
/******************************************/

/* Nothing special. */
#define IN_NOTHING 0
/* In command position. */
#define IN_CMD     1
/* In a mathematical environment. */
#define IN_MATH    2
/* In a condition. */
#define IN_COND    3
/* In a parameter assignment (e.g. `foo=bar'). */
#define IN_ENV     4


/******************************/
/* Definition for zsh options */
/******************************/

/* Possible values of emulation */

#define EMULATE_CSH  (1<<1) /* C shell */
#define EMULATE_KSH  (1<<2) /* Korn shell */
#define EMULATE_SH   (1<<3) /* Bourne shell */
#define EMULATE_ZSH  (1<<4) /* `native' mode */

/* option indices */

enum {
    OPT_INVALID,
    ALIASESOPT,
    ALLEXPORT,
    ALWAYSLASTPROMPT,
    ALWAYSTOEND,
    APPENDHISTORY,
    AUTOCD,
    AUTOLIST,
    AUTOMENU,
    AUTONAMEDIRS,
    AUTOPARAMKEYS,
    AUTOPARAMSLASH,
    AUTOPUSHD,
    AUTOREMOVESLASH,
    AUTORESUME,
    BADPATTERN,
    BANGHIST,
    BAREGLOBQUAL,
    BASHAUTOLIST,
    BEEP,
    BGNICE,
    BRACECCL,
    BSDECHO,
    CBASES,
    CDABLEVARS,
    CHASEDOTS,
    CHASELINKS,
    CHECKJOBS,
    CLOBBER,
    COMPLETEALIASES,
    COMPLETEINWORD,
    CORRECT,
    CORRECTALL,
    CSHJUNKIEHISTORY,
    CSHJUNKIELOOPS,
    CSHJUNKIEQUOTES,
    CSHNULLCMD,
    CSHNULLGLOB,
    EQUALS,
    ERREXIT,
    EXECOPT,
    EXTENDEDGLOB,
    EXTENDEDHISTORY,
    FLOWCONTROL,
    FUNCTIONARGZERO,
    GLOBOPT,
    GLOBALEXPORT,
    GLOBALRCS,
    GLOBASSIGN,
    GLOBCOMPLETE,
    GLOBDOTS,
    GLOBSUBST,
    HASHCMDS,
    HASHDIRS,
    HASHLISTALL,
    HISTALLOWCLOBBER,
    HISTBEEP,
    HISTEXPIREDUPSFIRST,
    HISTFINDNODUPS,
    HISTIGNOREALLDUPS,
    HISTIGNOREDUPS,
    HISTIGNORESPACE,
    HISTNOFUNCTIONS,
    HISTNOSTORE,
    HISTREDUCEBLANKS,
    HISTSAVENODUPS,
    HISTVERIFY,
    HUP,
    IGNOREBRACES,
    IGNOREEOF,
    INCAPPENDHISTORY,
    INTERACTIVE,
    INTERACTIVECOMMENTS,
    KSHARRAYS,
    KSHAUTOLOAD,
    KSHGLOB,
    KSHOPTIONPRINT,
    KSHTYPESET,
    LISTAMBIGUOUS,
    LISTBEEP,
    LISTPACKED,
    LISTROWSFIRST,
    LISTTYPES,
    LOCALOPTIONS,
    LOCALTRAPS,
    LOGINSHELL,
    LONGLISTJOBS,
    MAGICEQUALSUBST,
    MAILWARNING,
    MARKDIRS,
    MENUCOMPLETE,
    MONITOR,
    MULTIOS,
    NOMATCH,
    NOTIFY,
    NULLGLOB,
    NUMERICGLOBSORT,
    OCTALZEROES,
    OVERSTRIKE,
    PATHDIRS,
    POSIXBUILTINS,
    PRINTEIGHTBIT,
    PRINTEXITVALUE,
    PRIVILEGED,
    PROMPTBANG,
    PROMPTCR,
    PROMPTPERCENT,
    PROMPTSUBST,
    PUSHDIGNOREDUPS,
    PUSHDMINUS,
    PUSHDSILENT,
    PUSHDTOHOME,
    RCEXPANDPARAM,
    RCQUOTES,
    RCS,
    RECEXACT,
    RESTRICTED,
    RMSTARSILENT,
    RMSTARWAIT,
    SHAREHISTORY,
    SHFILEEXPANSION,
    SHGLOB,
    SHINSTDIN,
    SHNULLCMD,
    SHOPTIONLETTERS,
    SHORTLOOPS,
    SHWORDSPLIT,
    SINGLECOMMAND,
    SINGLELINEZLE,
    SUNKEYBOARDHACK,
    UNSET,
    VERBOSE,
    XTRACE,
    USEZLE,
    DVORAK,
    OPT_SIZE
};

#undef isset
#define isset(X) (opts[X])
#define unset(X) (!opts[X])

#define interact (isset(INTERACTIVE))
#define jobbing  (isset(MONITOR))
#define islogin  (isset(LOGINSHELL))

/***********************************************/
/* Defintions for terminal and display control */
/***********************************************/

/* tty state structure */

struct ttyinfo {
#ifdef HAVE_TERMIOS_H
    struct termios tio;
#else
# ifdef HAVE_TERMIO_H
    struct termio tio;
# else
    struct sgttyb sgttyb;
    int lmodes;
    struct tchars tchars;
    struct ltchars ltchars;
# endif
#endif
#ifdef TIOCGWINSZ
    struct winsize winsize;
#endif
};

/* defines for whether tabs expand to spaces */
#if defined(HAVE_TERMIOS_H) || defined(HAVE_TERMIO_H)
#define SGTTYFLAG       shttyinfo.tio.c_oflag
#else   /* we're using sgtty */
#define SGTTYFLAG       shttyinfo.sgttyb.sg_flags
#endif
# ifdef TAB3
#define SGTABTYPE       TAB3
# else
#  ifdef OXTABS
#define SGTABTYPE       OXTABS
#  else
#define SGTABTYPE       XTABS
#  endif
# endif

/* flags for termflags */

#define TERM_BAD        0x01    /* terminal has extremely basic capabilities */
#define TERM_UNKNOWN    0x02    /* unknown terminal type */
#define TERM_NOUP       0x04    /* terminal has no up capability */
#define TERM_SHORT      0x08    /* terminal is < 3 lines high */
#define TERM_NARROW     0x10    /* terminal is < 3 columns wide */

/* interesting termcap strings */

#define TCCLEARSCREEN   0
#define TCLEFT          1
#define TCMULTLEFT      2
#define TCRIGHT         3
#define TCMULTRIGHT     4
#define TCUP            5
#define TCMULTUP        6
#define TCDOWN          7
#define TCMULTDOWN      8
#define TCDEL           9
#define TCMULTDEL      10
#define TCINS          11
#define TCMULTINS      12
#define TCCLEAREOD     13
#define TCCLEAREOL     14
#define TCINSLINE      15
#define TCDELLINE      16
#define TCNEXTTAB      17
#define TCBOLDFACEBEG  18
#define TCSTANDOUTBEG  19
#define TCUNDERLINEBEG 20
#define TCALLATTRSOFF  21
#define TCSTANDOUTEND  22
#define TCUNDERLINEEND 23
#define TCHORIZPOS     24
#define TCUPCURSOR     25
#define TCDOWNCURSOR   26
#define TCLEFTCURSOR   27
#define TCRIGHTCURSOR  28
#define TC_COUNT       29

#define tccan(X) (tclen[X])

#define TXTBOLDFACE   0x01
#define TXTSTANDOUT   0x02
#define TXTUNDERLINE  0x04
#define TXTDIRTY      0x80

#define txtisset(X)  (txtattrmask & (X))
#define txtset(X)    (txtattrmask |= (X))
#define txtunset(X)  (txtattrmask &= ~(X))

#define TXTNOBOLDFACE   0x10
#define TXTNOSTANDOUT   0x20
#define TXTNOUNDERLINE  0x40

#define txtchangeisset(X)       (txtchange & (X))
#define txtchangeset(X, Y)      (txtchange |= (X), txtchange &= ~(Y))

/****************************************/
/* Definitions for the %_ prompt escape */
/****************************************/

#define CMDSTACKSZ 256
#define cmdpush(X) do { \
                       if (cmdsp >= 0 && cmdsp < CMDSTACKSZ) \
                           cmdstack[cmdsp++]=(X); \
                   } while (0)
#ifdef DEBUG
# define cmdpop()  do { \
                       if (cmdsp <= 0) { \
                           fputs("BUG: cmdstack empty\n", stderr); \
                           fflush(stderr); \
                       } else cmdsp--; \
                   } while (0)
#else
# define cmdpop()   do { if (cmdsp > 0) cmdsp--; } while (0)
#endif

#define CS_FOR          0
#define CS_WHILE        1
#define CS_REPEAT       2
#define CS_SELECT       3
#define CS_UNTIL        4
#define CS_IF           5
#define CS_IFTHEN       6
#define CS_ELSE         7
#define CS_ELIF         8
#define CS_MATH         9
#define CS_COND        10
#define CS_CMDOR       11
#define CS_CMDAND      12
#define CS_PIPE        13
#define CS_ERRPIPE     14
#define CS_FOREACH     15
#define CS_CASE        16
#define CS_FUNCDEF     17
#define CS_SUBSH       18
#define CS_CURSH       19
#define CS_ARRAY       20
#define CS_QUOTE       21
#define CS_DQUOTE      22
#define CS_BQUOTE      23
#define CS_CMDSUBST    24
#define CS_MATHSUBST   25
#define CS_ELIFTHEN    26
#define CS_HEREDOC     27
#define CS_HEREDOCD    28
#define CS_BRACE       29
#define CS_BRACEPAR    30

/*********************
 * Memory management *
 *********************/

/* heappush saves the current heap state using this structure */

struct heapstack {
    struct heapstack *next;     /* next one in list for this heap */
    size_t used;
};

/* A zsh heap. */

struct heap {
    struct heap *next;          /* next one                                  */
    size_t size;                /* size of heap                              */
    size_t used;                /* bytes used from the heap                  */
    struct heapstack *sp;       /* used by pushheap() to save the value used */

/* Uncomment the following if the struct needs padding to 64-bit size. */
/* Make sure sizeof(heap) is a multiple of 8 
#if defined(PAD_64_BIT)
# if !defined(__GNUC__)
    size_t dummy;
# else
    size_t dummy __attribute__((aligned (8)));
# endif
#endif
*/
#define arena(X)        ((char *) (X) + sizeof(struct heap))
}
#if defined(PAD_64_BIT) && defined(__GNUC__)
/* GCC 2.5.8 only accepts this for struct fields. */
/*__attribute__((aligned (8))) */
#endif
;

# define NEWHEAPS(h)    do { Heap _switch_oldheaps = h = new_heaps(); do
# define OLDHEAPS       while (0); old_heaps(_switch_oldheaps); } while (0);

# define SWITCHHEAPS(o, h)  do { o = switch_heaps(h); do
# define SWITCHBACKHEAPS(o) while (0); switch_heaps(o); } while (0);

/****************/
/* Debug macros */
/****************/

#ifdef DEBUG
# define DPUTS(X,Y) if (!(X)) {;} else dputs(Y)
#else
# define DPUTS(X,Y)
#endif

/**************************/
/* Signal handling macros */
/**************************/

/* These used in the sigtrapped[] array */

#define ZSIG_TRAPPED    (1<<0)  /* Signal is trapped */
#define ZSIG_IGNORED    (1<<1)  /* Signal is ignored */
#define ZSIG_FUNC       (1<<2)  /* Trap is a function, not an eval list */
/* Mask to get the above flags */
#define ZSIG_MASK       (ZSIG_TRAPPED|ZSIG_IGNORED|ZSIG_FUNC)
/* No. of bits to shift local level when storing in sigtrapped */
#define ZSIG_SHIFT      3

/**********************************/
/* Flags to third argument of zle */
/**********************************/

#define ZLRF_HISTORY    0x01    /* OK to access the history list */
#define ZLRF_NOSETTY    0x02    /* Don't set tty before return */

/****************/
/* Entry points */
/****************/

/* compctl entry point pointers */

typedef int (*CompctlReadFn) _((char *, char **, char *, char *));

/* ZLE entry point pointers */

typedef void (*ZleVoidFn) _((void));
typedef void (*ZleVoidIntFn) _((int));
typedef unsigned char * (*ZleReadFn) _((char *, char *, int));

/***************************************/
/* Hooks in core.                      */
/***************************************/

#define EXITHOOK       (zshhooks + 0)
#define BEFORETRAPHOOK (zshhooks + 1)
#define AFTERTRAPHOOK  (zshhooks + 2)
