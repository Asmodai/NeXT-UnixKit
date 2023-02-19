/*
 * linklist.c - linked lists
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
#include "linklist.pro"

/* Get an empty linked list header */

/**/
mod_export LinkList
newlinklist(void)
{
    LinkList list;

    list = (LinkList) zhalloc(sizeof *list);
    list->first = NULL;
    list->last = (LinkNode) list;
    return list;
}

/**/
mod_export LinkList
znewlinklist(void)
{
    LinkList list;

    list = (LinkList) zalloc(sizeof *list);
    list->first = NULL;
    list->last = (LinkNode) list;
    return list;
}

/* Insert a node in a linked list after a given node */

/**/
mod_export LinkNode
insertlinknode(LinkList list, LinkNode node, void *dat)
{
    LinkNode tmp, new;

    tmp = node->next;
    node->next = new = (LinkNode) zhalloc(sizeof *tmp);
    new->last = node;
    new->dat = dat;
    new->next = tmp;
    if (tmp)
	tmp->last = new;
    else
	list->last = new;
    return new;
}

/**/
mod_export LinkNode
zinsertlinknode(LinkList list, LinkNode node, void *dat)
{
    LinkNode tmp, new;

    tmp = node->next;
    node->next = new = (LinkNode) zalloc(sizeof *tmp);
    new->last = node;
    new->dat = dat;
    new->next = tmp;
    if (tmp)
	tmp->last = new;
    else
	list->last = new;
    return new;
}

/* Insert an already-existing node into a linked list after a given node */

/**/
mod_export LinkNode
uinsertlinknode(LinkList list, LinkNode node, LinkNode new)
{
    LinkNode tmp = node->next;
    node->next = new;
    new->last = node;
    new->next = tmp;
    if (tmp)
	tmp->last = new;
    else
	list->last = new;
    return new;
}

/* Insert a list in another list */

/**/
mod_export void
insertlinklist(LinkList l, LinkNode where, LinkList x)
{
    LinkNode nx;

    nx = where->next;
    if (!l->first)
	return;
    where->next = l->first;
    l->last->next = nx;
    l->first->last = where;
    if (nx)
	nx->last = l->last;
    else
	x->last = l->last;
}

/* Get top node in a linked list */

/**/
mod_export void *
getlinknode(LinkList list)
{
    void *dat;
    LinkNode node;

    if (!(node = list->first))
	return NULL;
    dat = node->dat;
    list->first = node->next;
    if (node->next)
	node->next->last = (LinkNode) list;
    else
	list->last = (LinkNode) list;
    zfree(node, sizeof(struct linknode));
    return dat;
}

/* Get top node in a linked list without freeing */

/**/
mod_export void *
ugetnode(LinkList list)
{
    void *dat;
    LinkNode node;

    if (!(node = list->first))
	return NULL;
    dat = node->dat;
    list->first = node->next;
    if (node->next)
	node->next->last = (LinkNode) list;
    else
	list->last = (LinkNode) list;
    return dat;
}

/* Remove a node from a linked list */

/**/
mod_export void *
remnode(LinkList list, LinkNode nd)
{
    void *dat;

    nd->last->next = nd->next;
    if (nd->next)
	nd->next->last = nd->last;
    else
	list->last = nd->last;
    dat = nd->dat;
    zfree(nd, sizeof(struct linknode));

    return dat;
}

/* Remove a node from a linked list without freeing */

/**/
mod_export void *
uremnode(LinkList list, LinkNode nd)
{
    void *dat;

    nd->last->next = nd->next;
    if (nd->next)
	nd->next->last = nd->last;
    else
	list->last = nd->last;
    dat = nd->dat;
    return dat;
}

/* Free a linked list */

/**/
mod_export void
freelinklist(LinkList list, FreeFunc freefunc)
{
    LinkNode node, next;

    for (node = list->first; node; node = next) {
	next = node->next;
	if (freefunc)
	    freefunc(node->dat);
	zfree(node, sizeof(struct linknode));
    }
    zfree(list, sizeof(struct linklist));
}

/* Count the number of nodes in a linked list */

/**/
mod_export int
countlinknodes(LinkList list)
{
    LinkNode nd;
    int ct = 0;

    for (nd = firstnode(list); nd; incnode(nd), ct++);
    return ct;
}

/**/
mod_export void
rolllist(LinkList l, LinkNode nd)
{
    l->last->next = l->first;
    l->first->last = l->last;
    l->first = nd;
    l->last = nd->last;
    nd->last = (LinkNode) l;
    l->last->next = 0;
}

/**/
mod_export LinkList
newsizedlist(int size)
{
    LinkList list;
    LinkNode node;

    list = (LinkList) zhalloc(sizeof(struct linklist) +
			      (size * sizeof(struct linknode)));

    list->first = (LinkNode) (list + 1);
    for (node = list->first; size; size--, node++) {
	node->last = node - 1;
	node->next = node + 1;
    }
    list->last = node - 1;
    list->first->last = (LinkNode) list;
    node[-1].next = NULL;

    return list;
}

/**/
mod_export int
listcontains(LinkList list, void *dat)
{
    LinkNode node;

    for (node = firstnode(list); node; incnode(node))
	if (getdata(node) == dat)
	    return 1;

    return 0;
}
