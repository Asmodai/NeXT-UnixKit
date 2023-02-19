/*
 * cap.c - POSIX.1e (POSIX.6) capability set manipulation
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1997 Andrew Main
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Andrew Main or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Andrew Main and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Andrew Main and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Andrew Main and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "cap.mdh"
#include "cap.pro"

#ifdef HAVE_CAP_GET_PROC

static int
bin_cap(char *nam, char **argv, char *ops, int func)
{
    int ret = 0;
    cap_t caps;
    if(*argv) {
	caps = cap_from_text(*argv);
	if(!caps) {
	    zwarnnam(nam, "invalid capability string", NULL, 0);
	    return 1;
	}
	if(cap_set_proc(caps)) {
	    zwarnnam(nam, "can't change capabilites: %e", NULL, errno);
	    ret = 1;
	}
    } else {
	char *result = NULL;
	ssize_t length;
	caps = cap_get_proc();
	if(caps)
	    result = cap_to_text(caps, &length);
	if(!caps || !result) {
	    zwarnnam(nam, "can't get capabilites: %e", NULL, errno);
	    ret = 1;
	} else
	    puts(result);
    }
    cap_free(caps);
    return ret;
}

static int
bin_getcap(char *nam, char **argv, char *ops, int func)
{
    int ret = 0;

    do {
	char *result = NULL;
	ssize_t length;
	cap_t caps = cap_get_file(*argv);
	if(caps)
	    result = cap_to_text(caps, &length);
	if (!caps || !result) {
	    zwarnnam(nam, "%s: %e", *argv, errno);
	    ret = 1;
	} else
	    printf("%s %s\n", *argv, result);
	cap_free(caps);
    } while(*++argv);
    return ret;
}

static int
bin_setcap(char *nam, char **argv, char *ops, int func)
{
    cap_t caps;
    int ret = 0;

    caps = cap_from_text(*argv++);
    if(!caps) {
	zwarnnam(nam, "invalid capability string", NULL, 0);
	return 1;
    }

    do {
	if(cap_set_file(*argv, caps)) {
	    zwarnnam(nam, "%s: %e", *argv, errno);
	    ret = 1;
	}
    } while(*++argv);
    cap_free(caps);
    return ret;
}

#else /* !HAVE_CAP_GET_PROC */

# define bin_cap    bin_notavail
# define bin_getcap bin_notavail
# define bin_setcap bin_notavail

#endif /* !HAVE_CAP_GET_PROC */

/* module paraphernalia */

static struct builtin bintab[] = {
    BUILTIN("cap",    0, bin_cap,    0,  1, 0, NULL, NULL),
    BUILTIN("getcap", 0, bin_getcap, 1, -1, 0, NULL, NULL),
    BUILTIN("setcap", 0, bin_setcap, 2, -1, 0, NULL, NULL),
};

/**/
int
setup_(Module m)
{
    return 0;
}

/**/
int
boot_(Module m)
{
    return !addbuiltins(m->nam, bintab, sizeof(bintab)/sizeof(*bintab));
}

/**/
int
cleanup_(Module m)
{
    deletebuiltins(m->nam, bintab, sizeof(bintab)/sizeof(*bintab));
    return 0;
}

/**/
int
finish_(Module m)
{
    return 0;
}
