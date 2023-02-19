/*
 * ztype.h - character classification macros
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

#define IDIGIT   (1 <<  0)
#define IALNUM   (1 <<  1)
#define IBLANK   (1 <<  2)
#define INBLANK  (1 <<  3)
#define ITOK     (1 <<  4)
#define ISEP     (1 <<  5)
#define IALPHA   (1 <<  6)
#define IIDENT   (1 <<  7)
#define IUSER    (1 <<  8)
#define ICNTRL   (1 <<  9)
#define IWORD    (1 << 10)
#define ISPECIAL (1 << 11)
#define IMETA    (1 << 12)
#define IWSEP    (1 << 13)
#define _icom(X,Y) (typtab[STOUC(X)] & Y)
#define idigit(X) _icom(X,IDIGIT)
#define ialnum(X) _icom(X,IALNUM)
#define iblank(X) _icom(X,IBLANK)	/* blank, not including \n */
#define inblank(X) _icom(X,INBLANK)	/* blank or \n */
#define itok(X) _icom(X,ITOK)
#define isep(X) _icom(X,ISEP)
#define ialpha(X) _icom(X,IALPHA)
#define iident(X) _icom(X,IIDENT)
#define iuser(X) _icom(X,IUSER)	/* username char */
#define icntrl(X) _icom(X,ICNTRL)
#define iword(X) _icom(X,IWORD)
#define ispecial(X) _icom(X,ISPECIAL)
#define imeta(X) _icom(X,IMETA)
#define iwsep(X) _icom(X,IWSEP)
