/*
 * zle_keymap.c - keymaps and key bindings
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

#include "zle.mdh"

/*
 * Keymap structures:
 *
 * There is a hash table of keymap names.  Each name just points to a keymap.
 * More than one name may point to the same keymap.
 *
 * Each keymap consists of a table of bindings for each character, and a
 * hash table of multi-character key bindings.  The keymap has no individual
 * name, but maintains a reference count.
 *
 * In a keymap's table of initial bindings, each character is either bound to
 * a thingy, or is a prefix (in which case NULL is stored).  Those prefix
 * entries are matched by more complex entries in the multi-character
 * binding hash table.  Each entry in this hash table (which is indexed by
 * metafied key sequence) either has a normal thingy binding or a string to
 * send (in which case the NULL thingy is used).  Each entry also has a count
 * of other entries for which it is a prefix.
 */

typedef struct keymapname *KeymapName;
typedef struct key *Key;

struct keymapname {
    HashNode next;	/* next in the hash chain */
    char *nam;		/* name of the keymap */
    int flags;		/* various flags (see below) */
    Keymap keymap;	/* the keymap itsef */
};

#define KMN_IMMORTAL (1<<1)

struct keymap {
    Thingy first[256];	/* base binding of each character */
    HashTable multi;	/* multi-character bindings */
    int flags;		/* various flags (see below) */
    int rc;		/* reference count */
};

#define KM_IMMUTABLE (1<<1)

struct key {
    HashNode next;	/* next in hash chain */
    char *nam;		/* key sequence (metafied) */
    Thingy bind;	/* binding of this key sequence */
    char *str;		/* string for send-string (metafied) */
    int prefixct;	/* number of sequences for which this is a prefix */
};

/* This structure is used when listing keymaps. */

struct bindstate {
    int flags;
    char *kmname;
    char *firstseq;
    char *lastseq;
    Thingy bind;
    char *str;
    char *prefix;
    int prefixlen;
};

/* This structure is used when scanning for prefix bindings to remove */

struct remprefstate {
    Keymap km;
    char *prefix;
    int prefixlen;
};

#define BS_LIST (1<<0)
#define BS_ALL  (1<<1)

/* local functions */

#include "zle_keymap.pro"

/* currently selected keymap, and its name */

/**/
Keymap curkeymap, localkeymap;
/**/
char *curkeymapname;

/* the hash table of keymap names */

/**/
mod_export HashTable keymapnamtab;

/* key sequence reading data */

/**/
char *keybuf;

static int keybuflen, keybufsz = 20;

/* last command executed with execute-named-command */

static Thingy lastnamed;

/**********************************/
/* hashtable management functions */
/**********************************/

/**/
static void
createkeymapnamtab(void)
{
    keymapnamtab = newhashtable(7, "keymapnamtab", NULL);

    keymapnamtab->hash        = hasher;
    keymapnamtab->emptytable  = emptyhashtable;
    keymapnamtab->filltable   = NULL;
    keymapnamtab->cmpnodes    = strcmp;
    keymapnamtab->addnode     = addhashnode;
    keymapnamtab->getnode     = gethashnode2;
    keymapnamtab->getnode2    = gethashnode2;
    keymapnamtab->removenode  = removehashnode;
    keymapnamtab->disablenode = NULL;
    keymapnamtab->enablenode  = NULL;
    keymapnamtab->freenode    = freekeymapnamnode;
    keymapnamtab->printnode   = NULL;
}

/**/
static KeymapName
makekeymapnamnode(Keymap keymap)
{
    KeymapName kmn = (KeymapName) zcalloc(sizeof(*kmn));

    kmn->keymap = keymap;
    return kmn;
}

/**/
static void
freekeymapnamnode(HashNode hn)
{
    KeymapName kmn = (KeymapName) hn;

    zsfree(kmn->nam);
    if(!--kmn->keymap->rc)
	deletekeymap(kmn->keymap);
    zfree(kmn, sizeof(*kmn));
}

/**/
static HashTable
newkeytab(char *kmname)
{
    HashTable ht = newhashtable(19,
	kmname ?  dyncat("keytab:", kmname) : "keytab:", NULL);

    ht->hash        = hasher;
    ht->emptytable  = emptyhashtable;
    ht->filltable   = NULL;
    ht->cmpnodes    = strcmp;
    ht->addnode     = addhashnode;
    ht->getnode     = gethashnode2;
    ht->getnode2    = gethashnode2;
    ht->removenode  = removehashnode;
    ht->disablenode = NULL;
    ht->enablenode  = NULL;
    ht->freenode    = freekeynode;
    ht->printnode   = NULL;

    return ht;
}

/**/
static Key
makekeynode(Thingy t, char *str)
{
    Key k = (Key) zcalloc(sizeof(*k));

    k->bind = t;
    k->str = str;
    return k;
}

/**/
static void
freekeynode(HashNode hn)
{
    Key k = (Key) hn;

    zsfree(k->nam);
    unrefthingy(k->bind);
    zsfree(k->str);
    zfree(k, sizeof(*k));
}

/**************************/
/* main keymap operations */
/**************************/

static HashTable copyto;

/**/
mod_export Keymap
newkeymap(Keymap tocopy, char *kmname)
{
    Keymap km = zcalloc(sizeof(*km));
    int i;

    km->rc = 0;
    km->multi = newkeytab(kmname);
    if(tocopy) {
	for(i = 256; i--; )
	    km->first[i] = refthingy(tocopy->first[i]);
	copyto = km->multi;
	scanhashtable(tocopy->multi, 0, 0, 0, scancopykeys, 0);
    } else {
	for(i = 256; i--; )
	    km->first[i] = refthingy(t_undefinedkey);
    }
    return km;
}

/**/
static void
scancopykeys(HashNode hn, int flags)
{
    Key k = (Key) hn;
    Key kn = zalloc(sizeof(*k));

    memcpy(kn, k, sizeof(*k));
    refthingy(kn->bind);
    kn->str = ztrdup(k->str);
    copyto->addnode(copyto, ztrdup(k->nam), kn);
}

/**/
void
deletekeymap(Keymap km)
{
    int i;

    deletehashtable(km->multi);
    for(i = 256; i--; )
	unrefthingy(km->first[i]);
    zfree(km, sizeof(*km));
}

static Keymap skm_km;
static int skm_last;
static KeyScanFunc skm_func;
static void *skm_magic;

/**/
void
scankeymap(Keymap km, int sort, KeyScanFunc func, void *magic)
{
    char m[3];

    skm_km = km;
    skm_last = sort ? -1 : 255;
    skm_func = func;
    skm_magic = magic;
    scanhashtable(km->multi, sort, 0, 0, scankeys, 0);
    if(!sort)
	skm_last = -1;
    while(skm_last < 255) {
	skm_last++;
	if(km->first[skm_last] && km->first[skm_last] != t_undefinedkey) {
	    m[0] = skm_last;
	    metafy(m, 1, META_NOALLOC);
	    func(m, km->first[skm_last], NULL, magic);
	}
    }
}

/**/
static void
scankeys(HashNode hn, int flags)
{
    Key k = (Key) hn;
    int f = k->nam[0] == Meta ? STOUC(k->nam[1])^32 : STOUC(k->nam[0]);
    char m[3];

    while(skm_last < f) {
	skm_last++;
	if(skm_km->first[skm_last] &&
	   skm_km->first[skm_last] != t_undefinedkey) {
	    m[0] = skm_last;
	    metafy(m, 1, META_NOALLOC);
	    skm_func(m, skm_km->first[skm_last], NULL, skm_magic);
	}
    }
    skm_func(k->nam, k->bind, k->str, skm_magic);
}

/**************************/
/* keymap name operations */
/**************************/

/**/
Keymap
openkeymap(char *name)
{
    KeymapName n = (KeymapName) keymapnamtab->getnode(keymapnamtab, name);
    return n ? n->keymap : NULL;
}

/**/
mod_export int
unlinkkeymap(char *name, int ignm)
{
    KeymapName n = (KeymapName) keymapnamtab->getnode(keymapnamtab, name);
    if(!n)
	return 2;
    if(!ignm && (n->flags & KMN_IMMORTAL))
	return 1;
    keymapnamtab->freenode(keymapnamtab->removenode(keymapnamtab, name));
    return 0;
}

/**/
mod_export int
linkkeymap(Keymap km, char *name, int imm)
{
    KeymapName n = (KeymapName) keymapnamtab->getnode(keymapnamtab, name);
    if(n) {
	if(n->flags & KMN_IMMORTAL)
	    return 1;
	if(n->keymap == km)
	    return 0;
	if(!--n->keymap->rc)
	    deletekeymap(n->keymap);
	n->keymap = km;
    } else {
	n = makekeymapnamnode(km);
	if (imm)
	    n->flags |= KMN_IMMORTAL;
	keymapnamtab->addnode(keymapnamtab, ztrdup(name), n);
    }
    km->rc++;
    return 0;
}

/* Select a keymap as the current ZLE keymap.  Can optionally fall back *
 * on the guaranteed safe keymap if it fails.                           */

/**/
int
selectkeymap(char *name, int fb)
{
    Keymap km = openkeymap(name);

    if(!km) {
	char *nm = niceztrdup(name);
	char *msg = tricat("No such keymap `", nm, "'");

	zsfree(nm);
	showmsg(msg);
	zsfree(msg);
	if(!fb)
	    return 1;
	km = openkeymap(name = ".safe");
    }
    curkeymapname = name;
    curkeymap = km;
    return 0;
}

/* Select a local key map. */

/**/
mod_export void
selectlocalmap(Keymap m)
{
    localkeymap = m;
}

/* Reopen the currently selected keymap, in case it got deleted.  This *
 * should be called after doing anything that might have run an        *
 * arbitrary user-specified command.                                   */

/**/
void
reselectkeymap(void)
{
    selectkeymap(curkeymapname, 1);
}

/******************************/
/* operations on key bindings */
/******************************/

/* Add/delete/change a keybinding in some keymap.  km is the keymap to be *
 * altered.  seq is the metafied key sequence whose binding is to change. *
 * bind is the thingy to which the key sequence is to be bound.  For      *
 * send-string, bind is NULL and str is the metafied key sequence to push *
 * back onto the input.                                                   */

/**/
mod_export int
bindkey(Keymap km, char *seq, Thingy bind, char *str)
{
    Key k;
    int f = seq[0] == Meta ? STOUC(seq[1])^32 : STOUC(seq[0]);
    char *buf, *ptr;

    if(km->flags & KM_IMMUTABLE)
	return 1;
    if(!*seq)
	return 2;
    if(!bind || ztrlen(seq) > 1) {
	/* key needs to become a prefix if isn't one already */
	if(km->first[f]) {
	    char fs[3];
	    fs[0] = f;
	    metafy(fs, 1, META_NOALLOC);
	    km->multi->addnode(km->multi, ztrdup(fs),
		makekeynode(km->first[f], NULL));
	    km->first[f] = NULL;
	}
	k = (Key) km->multi->getnode(km->multi, seq);
    } else {
	/* If the sequence is a prefix entry only due to being *
	 * a send-string binding, we can remove that entry.    */
	if(!km->first[f]) {
	    k = (Key) km->multi->getnode(km->multi, seq);
	    if(!k->prefixct)
		km->multi->freenode(km->multi->removenode(km->multi, seq));
	    else
		goto domulti;
	} else
	    unrefthingy(km->first[f]);
	/* Just replace the single-character binding. */
	km->first[f] = bind;
	return 0;
    }
    domulti:
    buf = ztrdup(seq);
    ptr = strchr(buf, 0);
    if(bind == t_undefinedkey) {
	if(k) {
	    zsfree(k->str);
	    unrefthingy(k->bind);
	    k->bind = t_undefinedkey;
	    k->str = NULL;
	    while(!k->prefixct && k->bind == t_undefinedkey) {
		km->multi->freenode(km->multi->removenode(km->multi, buf));
		*--ptr = 0;
		if(ptr[-1] == Meta)
		    *--ptr = 0;
		k = (Key) km->multi->getnode(km->multi, buf);
		k->prefixct--;
		if(!k->prefixct && k->bind &&
		    (!buf[1] || (buf[0] == Meta && !buf[2]))) {
		    km->first[f] = refthingy(k->bind);
		    km->multi->freenode(km->multi->removenode(km->multi, buf));
		    break;
		}
	    }
	}
    } else {
	if(!k) {
	    int added;

	    km->multi->addnode(km->multi, ztrdup(buf), makekeynode(bind, ztrdup(str)));
	    do {
		*--ptr = 0;
		if(ptr > buf && ptr[-1] == Meta)
		    *--ptr = 0;
		k = (Key) km->multi->getnode(km->multi, buf);
		if((added = !k))
		    km->multi->addnode(km->multi, ztrdup(buf),
			k = makekeynode(refthingy(t_undefinedkey), NULL));
		k->prefixct++;
	    } while(added);
	} else {
	    unrefthingy(k->bind);
	    zsfree(k->str);
	    k->bind = bind;
	    k->str = bind ? NULL : ztrdup(str);
	}
    }
    free(buf);
    return 0;
}

/* Look up a key binding.  The binding is returned.  In the case of a  *
 * send-string, NULL is returned and *strp is modified to point to the *
 * metafied string of characters to be pushed back.                    */

/**/
Thingy
keybind(Keymap km, char *seq, char **strp)
{
    Key k;

    if(ztrlen(seq) == 1) {
	int f = seq[0] == Meta ? STOUC(seq[1])^32 : STOUC(seq[0]);
	Thingy bind = km->first[f];

	if(bind)
	    return bind;
    }
    k = (Key) km->multi->getnode(km->multi, seq);
    if(!k)
	return t_undefinedkey;
    *strp = k->str;
    return k->bind;
}

/* Check whether a key sequence is a prefix of a longer bound sequence. *
 * One oddity: if *nothing* in the keymap is bound, this returns true   *
 * for the empty sequence, even though this is not strictly accurate.   */

/**/
static int
keyisprefix(Keymap km, char *seq)
{
    Key k;

    if(!*seq)
	return 1;
    if(ztrlen(seq) == 1) {
	int f = seq[0] == Meta ? STOUC(seq[1])^32 : STOUC(seq[0]);

	if(km->first[f])
	    return 0;
    }
    k = (Key) km->multi->getnode(km->multi, seq);
    return k && k->prefixct;
}

/*******************/
/* bindkey builtin */
/*******************/

/*
 * THE BINDKEY BUILTIN
 *
 * Keymaps can be specified to bindkey in the following ways:
 *
 *   -e   select "emacs", also link it to "main"
 *   -v   select "viins", also link it to "main"
 *   -a   select "vicmd"
 *   -M   first argument gives map name
 *        defaults to "main"
 *
 * These operations cannot have a keymap selected in the normal way:
 *
 *   -l   list all the keymap names
 *   -d   delete all keymaps and reset to the default state (no arguments)
 *   -D   delete named keymaps
 *   -A   link the two named keymaps (2 arguments)
 *   -N   create new empty keymap (1 argument)
 *   -N   create new keymap, copying the second named keymap (2 arguments)
 *
 * Other operations:
 *
 *   -m   add the meta bindings to the selected keymap (no arguments)
 *   -r   unbind each named string in the selected keymap
 *   -s   bind send-strings in the selected keymap (2+ arguments)
 *        bind commands in the selected keymap (2+ arguments)
 *        display one binding in the selected keymap (1 argument)
 *        display the entire selected keymap (no arguments)
 *
 * There is an exception that the entire keymap display will not be performed
 * if the -e or -v options were used.
 *
 * Other options:
 *
 *   -L   do listings in the form of bindkey commands
 *   -R   for the binding operations, accept ranges instead of sequences
 */

/**/
int
bin_bindkey(char *name, char **argv, char *ops, int func)
{
    static struct opn {
	char o;
	char selp;
	int (*func) _((char *, char *, Keymap, char **, char *, char));
	int min, max;
    } const opns[] = {
	{ 'l', 0, bin_bindkey_lsmaps, 0,  0 },
	{ 'd', 0, bin_bindkey_delall, 0,  0 },
	{ 'D', 0, bin_bindkey_del,    1, -1 },
	{ 'A', 0, bin_bindkey_link,   2,  2 },
	{ 'N', 0, bin_bindkey_new,    1,  2 },
	{ 'm', 1, bin_bindkey_meta,   0,  0 },
	{ 'r', 1, bin_bindkey_bind,   1, -1 },
	{ 's', 1, bin_bindkey_bind,   2, -1 },
	{ 0,   1, bin_bindkey_bind,   0, -1 },
    };
    struct opn const *op, *opp;
    char *kmname;
    Keymap km;
    int n;

    /* select operation and ensure no clashing arguments */
    for(op = opns; op->o && !ops[STOUC(op->o)]; op++) ;
    if(op->o)
	for(opp = op; (++opp)->o; )
	    if(ops[STOUC(opp->o)]) {
		zwarnnam(name, "incompatible operation selection options",
		    NULL, 0);
		return 1;
	    }
    n = ops['e'] + ops['v'] + ops['a'] + ops['M'];
    if(!op->selp && n) {
	zwarnnam(name, "keymap cannot be selected with -%c", NULL, op->o);
	return 1;
    }
    if(n > 1) {
	zwarnnam(name, "incompatible keymap selection options", NULL, 0);
	return 1;
    }

    /* keymap selection */
    if(op->selp) {
	if(ops['e'])
	    kmname = "emacs";
	else if(ops['v'])
	    kmname = "viins";
	else if(ops['a'])
	    kmname = "vicmd";
	else if(ops['M']) {
	    kmname = *argv++;
	    if(!kmname) {
		zwarnnam(name, "-M option requires a keymap argument", NULL, 0);
		return 1;
	    }
	} else
	    kmname = "main";
	km = openkeymap(kmname);
	if(!km) {
	    zwarnnam(name, "no such keymap `%s'", kmname, 0);
	    return 1;
	}
	if(ops['e'] || ops['v'])
	    linkkeymap(km, "main", 0);
    } else {
	kmname = NULL;
	km = NULL;
    }

    /* listing is a special case */
    if(!op->o && (!argv[0] || !argv[1])) {
	if(ops['e'] || ops['v'])
	    return 0;
	return bin_bindkey_list(name, kmname, km, argv, ops, op->o);
    }

    /* check number of arguments */
    for(n = 0; argv[n]; n++) ;
    if(n < op->min) {
	zwarnnam(name, "not enough arguments for -%c", NULL, op->o);
	return 1;
    } else if(op->max != -1 && n > op->max) {
	zwarnnam(name, "too many arguments for -%c", NULL, op->o);
	return 1;
    }

    /* pass on the work to the operation function */
    return op->func(name, kmname, km, argv, ops, op->o);
}

/* list the available keymaps */

/**/
static int
bin_bindkey_lsmaps(char *name, char *kmname, Keymap km, char **argv, char *ops, char func)
{
    scanhashtable(keymapnamtab, 1, 0, 0, scanlistmaps, ops['L']);
    return 0;
}

/**/
static void
scanlistmaps(HashNode hn, int list)
{
    KeymapName n = (KeymapName) hn;

    if(list) {
	fputs("bindkey -N ", stdout);
	if(n->nam[0] == '-')
	    fputs("-- ", stdout);
	quotedzputs(n->nam, stdout);
    } else
	nicezputs(n->nam, stdout);
    putchar('\n');
}

/* reset all keymaps to the default state */

/**/
static int
bin_bindkey_delall(char *name, char *kmname, Keymap km, char **argv, char *ops, char func)
{
    keymapnamtab->emptytable(keymapnamtab);
    default_bindings();
    return 0;
}

/* delete named keymaps */

/**/
static int
bin_bindkey_del(char *name, char *kmname, Keymap km, char **argv, char *ops, char func)
{
    int ret = 0;

    do {
	int r = unlinkkeymap(*argv, 0);
	if(r == 1)
	    zwarnnam(name, "keymap name `%s' is protected", *argv, 0);
	else if(r == 2)
	    zwarnnam(name, "no such keymap `%s'", *argv, 0);
	ret |= !!r;
    } while(*++argv);
    return ret;
}

/* link named keymaps */

/**/
static int
bin_bindkey_link(char *name, char *kmname, Keymap km, char **argv, char *ops, char func)
{
    km = openkeymap(argv[0]);
    if(!km) {
	zwarnnam(name, "no such keymap `%s'", argv[0], 0);
	return 1;
    } else if(linkkeymap(km, argv[1], 0)) {
	zwarnnam(name, "keymap name `%s' is protected", argv[1], 0);
	return 1;
    }
    return 0;
}

/* create a new keymap */

/**/
static int
bin_bindkey_new(char *name, char *kmname, Keymap km, char **argv, char *ops, char func)
{
    KeymapName kmn = (KeymapName) keymapnamtab->getnode(keymapnamtab, argv[0]);

    if(kmn && (kmn -> flags & KMN_IMMORTAL)) {
	zwarnnam(name, "keymap name `%s' is protected", argv[0], 0);
	return 1;
    }
    if(argv[1]) {
	km = openkeymap(argv[1]);
	if(!km) {
	    zwarnnam(name, "no such keymap `%s'", argv[0], 0);
	    return 1;
	}
    } else
	km = NULL;
    linkkeymap(newkeymap(km, argv[0]), argv[0], 0);
    return 0;
}

/* Add standard meta bindings to a keymap.  Only sequences currently either *
 * unbound or bound to self-insert are affected.  Note that the use of      *
 * bindkey() is quite necessary: if this function were to go through the    *
 * km->first table itself, it would miss any prefix sequences that should   *
 * be rebound.                                                              */

/**/
static int
bin_bindkey_meta(char *name, char *kmname, Keymap km, char **argv, char *ops, char func)
{
    char m[3], *str;
    int i;
    Thingy fn;

    if(km->flags & KM_IMMUTABLE) {
	zwarnnam(name, "keymap `%s' is protected", kmname, 0);
	return 1;
    }
    for(i = 128; i < 256; i++)
	if(metabind[i - 128] != z_undefinedkey) {
	    m[0] = i;
	    metafy(m, 1, META_NOALLOC);
	    fn = keybind(km, m, &str);
	    if(fn == t_selfinsert || fn == t_undefinedkey)
		bindkey(km, m, refthingy(Th(metabind[i - 128])), NULL);
	}
    return 0;
}

/* Change key bindings.  func can be:              *
 *   'r'  bind sequences to undefined-key          *
 *   's'  bind sequneces to specified send-strings *
 *   0    bind sequences to specified functions    *
 * If the -R option is used, bind to key ranges    *
 * instead of single key sequences.                */

/**/
static int
bin_bindkey_bind(char *name, char *kmname, Keymap km, char **argv, char *ops, char func)
{
    int ret = 0;

    if(!func || func == 's') {
	char **a;

	for(a = argv+2; *a; a++)
	    if(!*++a) {
		zwarnnam(name, "even number of arguments required", NULL, 0);
		return 1;
	    }
    }
    if(km->flags & KM_IMMUTABLE) {
	zwarnnam(name, "keymap `%s' is protected", kmname, 0);
	return 1;
    }
    if (func == 'r' && ops['p']) {
	char *useq, *bseq;
	int len;
	struct remprefstate rps;
	rps.km = km;
	while ((useq = *argv++)) {
	    bseq = getkeystring(useq, &len, 2, NULL);
	    rps.prefix = metafy(bseq, len, META_USEHEAP);
	    rps.prefixlen = strlen(rps.prefix);
	    scankeymap(km, 0, scanremoveprefix, &rps);
	}
	return 0;
    }
    do {
	char *useq = *argv, *bseq, *seq, *str;
	int len;
	Thingy fn;

	if(func == 'r') {
	    fn = refthingy(t_undefinedkey);
	    str = NULL;
	} else if(func == 's') {
	    str = getkeystring(*++argv, &len, 2, NULL);
	    fn = NULL;
	    str = metafy(str, len, META_HREALLOC);
	} else {
	    fn = rthingy(*++argv);
	    str = NULL;
	}
	bseq = getkeystring(useq, &len, 2, NULL);
	seq = metafy(bseq, len, META_USEHEAP);
	if(ops['R']) {
	    int first, last;
	    char m[3];

	    if(len < 2 || len > 2 + (bseq[1] == '-') ||
	       (first = STOUC(bseq[0])) > (last = STOUC(bseq[len - 1]))) {
		zwarnnam(name, "malformed key range `%s'", useq, 0);
		ret = 1;
	    } else {
		for(; first <= last; first++) {
		    m[0] = first;
		    metafy(m, 1, META_NOALLOC);
		    bindkey(km, m, refthingy(fn), str);
		}
		unrefthingy(fn);
	    }
	} else {
	    if(bindkey(km, seq, fn, str)) {
		zwarnnam(name, "cannot bind to an empty key sequence", NULL, 0);
		ret = 1;
	    }
	}
    } while(*++argv);
    return ret;
}

/* Remove bindings for key sequences which have the given (proper) prefix. */

/**/
static void
scanremoveprefix(char *seq, Thingy bind, char *str, void *magic)
{
    struct remprefstate *rps = magic;

    if (strncmp(seq, rps->prefix, rps->prefixlen) || !seq[rps->prefixlen])
	return;

    bindkey(rps->km, seq, refthingy(t_undefinedkey), NULL);
}

/* List key bindings.  If an argument is given, list just that one *
 * binding, otherwise list the entire keymap.  If the -L option is *
 * given, list in the form of bindkey commands.                    */

/**/
static int
bin_bindkey_list(char *name, char *kmname, Keymap km, char **argv, char *ops, char func)
{
    struct bindstate bs;

    bs.flags = ops['L'] ? BS_LIST : 0;
    bs.kmname = kmname;
    if(argv[0] && !ops['p']) {
	int len;
	char *seq;

	seq = getkeystring(argv[0], &len, 2, NULL);
	seq = metafy(seq, len, META_HREALLOC);
	bs.flags |= BS_ALL;
	bs.firstseq = bs.lastseq = seq;
	bs.bind = keybind(km, seq, &bs.str);
	bs.prefix = NULL;
	bs.prefixlen = 0;
	bindlistout(&bs);
    } else {
	/* empty prefix is equivalent to no prefix */
	if (ops['p'] && (!argv[0] || argv[0][0])) {
	    if (!argv[0]) {
		zwarnnam(name, "option -p requires a prefix string", NULL, 0);
		return 1;
	    }
	    bs.prefix = getkeystring(argv[0], &bs.prefixlen, 2, NULL);
	    bs.prefix = metafy(bs.prefix, bs.prefixlen, META_HREALLOC);
	    bs.prefixlen = strlen(bs.prefix);
	} else {
	    bs.prefix = NULL;
	    bs.prefixlen = 0;
	}
	bs.firstseq = ztrdup("");
	bs.lastseq = ztrdup("");
	bs.bind = t_undefinedkey;
	bs.str = NULL;
	scankeymap(km, 1, scanbindlist, &bs);
	bindlistout(&bs);
	zsfree(bs.firstseq);
	zsfree(bs.lastseq);
    }
    return 0;
}

/**/
static void
scanbindlist(char *seq, Thingy bind, char *str, void *magic)
{
    struct bindstate *bs = magic;

    if (bs->prefixlen &&
	(strncmp(seq, bs->prefix, bs->prefixlen) || !seq[bs->prefixlen]))
	return;

    if(bind == bs->bind && (bind || !strcmp(str, bs->str)) &&
       ztrlen(seq) == 1 && ztrlen(bs->lastseq) == 1) {
	int l = bs->lastseq[1] ?
	    STOUC(bs->lastseq[1]) ^ 32 : STOUC(bs->lastseq[0]);
	int t = seq[1] ? STOUC(seq[1]) ^ 32 : STOUC(seq[0]);

	if(t == l + 1) {
	    zsfree(bs->lastseq);
	    bs->lastseq = ztrdup(seq);
	    return;
	}
    }
    bindlistout(bs);
    zsfree(bs->firstseq);
    bs->firstseq = ztrdup(seq);
    zsfree(bs->lastseq);
    bs->lastseq = ztrdup(seq);
    bs->bind = bind;
    bs->str = str;
}

/**/
static void
bindlistout(struct bindstate *bs)
{
    int range;

    if(bs->bind == t_undefinedkey && !(bs->flags & BS_ALL))
	return;
    range = strcmp(bs->firstseq, bs->lastseq);
    if(bs->flags & BS_LIST) {
	int nodash = 1;

	fputs("bindkey ", stdout);
	if(range)
	    fputs("-R ", stdout);
	if(!bs->bind)
	    fputs("-s ", stdout);
	if(!strcmp(bs->kmname, "main"))
	    ;
	else if(!strcmp(bs->kmname, "vicmd"))
	    fputs("-a ", stdout);
	else {
	    fputs("-M ", stdout);
	    quotedzputs(bs->kmname, stdout);
	    putchar(' ');
	    nodash = 0;
	}
	if(nodash && bs->firstseq[0] == '-')
	    fputs("-- ", stdout);
    }
    printbind(bs->firstseq, stdout);
    if(range) {
	putchar('-');
	printbind(bs->lastseq, stdout);
    }
    putchar(' ');
    if(bs->bind) {
	((bs->flags & BS_LIST) ? quotedzputs : nicezputs)
	    (bs->bind->nam, stdout);
    } else
	printbind(bs->str, stdout);
    putchar('\n');
}

/****************************/
/* initialisation functions */
/****************************/

/* main initialisation entry point */

/**/
void
init_keymaps(void)
{
    createkeymapnamtab();
    default_bindings();
    keybuf = (char *)zalloc(keybufsz);
    lastnamed = refthingy(t_undefinedkey);
}

/* cleanup entry point (for unloading the zle module) */

/**/
void
cleanup_keymaps(void)
{
    unrefthingy(lastnamed);
    deletehashtable(keymapnamtab);
    zfree(keybuf, keybufsz);
}

static char *cursorptr;

/* utility function for termcap output routine to add to string */

static int 
add_cursor_char(int c)
{
    *cursorptr++ = c;
    return 0;
}

/* interrogate termcap for cursor keys and add bindings to keymap */

/**/
static void
add_cursor_key(Keymap km, int tccode, Thingy thingy, int defchar)
{
    char buf[2048];
    int ok = 0;

    /*
     * Be careful not to try too hard with bindings for dubious or
     * dysfunctional terminals.
     */
    if (tccan(tccode) && !(termflags & (TERM_NOUP|TERM_BAD|TERM_UNKNOWN))) {
	/*
	 * We can use the real termcap sequence.  We need to
	 * persuade termcap to output `move cursor 1 char' and capture it.
	 */
	cursorptr = buf;
	tputs(tcstr[tccode], 1, add_cursor_char);
	*cursorptr = '\0';

	/*
	 * Sanity checking.  If the cursor key is zero-length (unlikely,
	 * but this is termcap we're talking about), or it's a single
	 * character, then we don't bind it.
	 */
	if (buf[0] && buf[1] && (buf[0] != Meta || buf[2]))
	    ok = 1;
    }
    if (!ok) {
	/* Assume the normal VT100-like values. */
	sprintf(buf, "\33[%c", defchar);
    }
    bindkey(km, buf, refthingy(thingy), NULL);

    /*
     * If the string looked like \e[? or \eO?, bind the other one, too.
     * This is necessary to make cursor keys work on many xterms with
     * both normal and application modes.
     */
    if (buf[0] == '\33' && (buf[1] == '[' || buf[1] == 'O') && 
	buf[2] && !buf[3])
    {
	buf[1] = (buf[1] == '[') ? 'O' : '[';
	bindkey(km, buf, refthingy(thingy), NULL);
    }
}

/* Create the default keymaps.  For efficiency reasons, this function   *
 * assigns directly to the km->first array.  It knows that there are no *
 * prefix bindings in the way, and that it is using a simple keymap.    */

/**/
static void
default_bindings(void)
{
    Keymap vmap = newkeymap(NULL, "viins");
    Keymap emap = newkeymap(NULL, "emacs");
    Keymap amap = newkeymap(NULL, "vicmd");
    Keymap smap = newkeymap(NULL, ".safe");
    Keymap vimaps[2], kptr;
    char buf[3], *ed;
    int i;

    /* vi insert mode and emacs mode:  *
     *   0-31   taken from the tables  *
     *  32-126  self-insert            *
     * 127      same as entry[8]       *
     * 128-255  self-insert            */
    for (i = 0; i < 32; i++) {
	vmap->first[i] = refthingy(Th(viinsbind[i]));
	emap->first[i] = refthingy(Th(emacsbind[i]));
    }
    for (i = 32; i < 256; i++) {
	vmap->first[i] = refthingy(t_selfinsert);
	emap->first[i] = refthingy(t_selfinsert);
    }
    unrefthingy(t_selfinsert);
    unrefthingy(t_selfinsert);
    vmap->first[127] = refthingy(vmap->first[8]);
    emap->first[127] = refthingy(emap->first[8]);

    /* vi command mode:              *
     *   0-127  taken from the table *
     * 128-255  undefined-key        */
    for (i = 0; i < 128; i++)
	amap->first[i] = refthingy(Th(vicmdbind[i]));
    for (i = 128; i < 256; i++)
	amap->first[i] = refthingy(t_undefinedkey);

    /* safe fallback keymap:
     *   0-255  self-insert, except: *
     *    '\n'  accept-line          *
     *    '\r'  accept-line          */
    for (i = 0; i < 256; i++)
	smap->first[i] = refthingy(t_selfinsert);
    unrefthingy(t_selfinsert);
    unrefthingy(t_selfinsert);
    smap->first['\n'] = refthingy(t_acceptline);
    smap->first['\r'] = refthingy(t_acceptline);

    /* vt100 arrow keys are bound by default, for historical reasons. *
     * Both standard and keypad modes are supported.                  */

    vimaps[0] = vmap;
    vimaps[1] = amap;
    for (i = 0; i < 2; i++) {
	kptr = vimaps[i];
	/* vi command and insert modes: arrow keys */
	add_cursor_key(kptr, TCUPCURSOR, t_uplineorhistory, 'A');
	add_cursor_key(kptr, TCDOWNCURSOR, t_downlineorhistory, 'B');
	add_cursor_key(kptr, TCLEFTCURSOR, t_vibackwardchar, 'D');
	add_cursor_key(kptr, TCRIGHTCURSOR, t_viforwardchar, 'C');
    }

    /* emacs mode: arrow keys */ 
    add_cursor_key(emap, TCUPCURSOR, t_uplineorhistory, 'A');
    add_cursor_key(emap, TCDOWNCURSOR, t_downlineorhistory, 'B');
    add_cursor_key(emap, TCLEFTCURSOR, t_backwardchar, 'D');
    add_cursor_key(emap, TCRIGHTCURSOR, t_forwardchar, 'C');
   
    /* emacs mode: ^X sequences */
    bindkey(emap, "\30*",   refthingy(t_expandword), NULL);
    bindkey(emap, "\30g",   refthingy(t_listexpand), NULL);
    bindkey(emap, "\30G",   refthingy(t_listexpand), NULL);
    bindkey(emap, "\30\16", refthingy(t_infernexthistory), NULL);
    bindkey(emap, "\30\13", refthingy(t_killbuffer), NULL);
    bindkey(emap, "\30\6",  refthingy(t_vifindnextchar), NULL);
    bindkey(emap, "\30\17", refthingy(t_overwritemode), NULL);
    bindkey(emap, "\30\25", refthingy(t_undo), NULL);
    bindkey(emap, "\30\26", refthingy(t_vicmdmode), NULL);
    bindkey(emap, "\30\12", refthingy(t_vijoin), NULL);
    bindkey(emap, "\30\2",  refthingy(t_vimatchbracket), NULL);
    bindkey(emap, "\30s",   refthingy(t_historyincrementalsearchforward), NULL);
    bindkey(emap, "\30r",   refthingy(t_historyincrementalsearchbackward), NULL);
    bindkey(emap, "\30u",   refthingy(t_undo), NULL);
    bindkey(emap, "\30\30", refthingy(t_exchangepointandmark), NULL);
    bindkey(emap, "\30=",   refthingy(t_whatcursorposition), NULL);

    /* emacs mode: ESC sequences, all taken from the meta binding table */
    buf[0] = '\33';
    buf[2] = 0;
    for (i = 0; i < 128; i++)
	if (metabind[i] != z_undefinedkey) {
	    buf[1] = i;
	    bindkey(emap, buf, refthingy(Th(metabind[i])), NULL);
	}

    /* Put the keymaps in the right namespace.  The "main" keymap  *
     * will be linked to the "emacs" keymap, except that if VISUAL *
     * or EDITOR contain the string "vi" then it will be linked to *
     * the "viins" keymap.                                         */
    linkkeymap(vmap, "viins", 0);
    linkkeymap(emap, "emacs", 0);
    linkkeymap(amap, "vicmd", 0);
    linkkeymap(smap, ".safe", 1);
    if (((ed = zgetenv("VISUAL")) && strstr(ed, "vi")) ||
	((ed = zgetenv("EDITOR")) && strstr(ed, "vi")))
	linkkeymap(vmap, "main", 0);
    else
	linkkeymap(emap, "main", 0);

    /* the .safe map cannot be modified or deleted */
    smap->flags |= KM_IMMUTABLE;
}

/*************************/
/* reading key sequences */
/*************************/

/* read a sequence of keys that is bound to some command in a keymap */

/**/
char *
getkeymapcmd(Keymap km, Thingy *funcp, char **strp)
{
    Thingy func = t_undefinedkey;
    char *str = NULL;
    int lastlen = 0, lastc = c;

    keybuflen = 0;
    keybuf[0] = 0;
    while((c = getkeybuf(!!lastlen)) != EOF) {
	char *s;
	Thingy f;
	int loc = 1;

	if (!localkeymap ||
	    (f = keybind(localkeymap, keybuf, &s)) == t_undefinedkey)
	    loc = 0, f = keybind(km, keybuf, &s);

	if(f != t_undefinedkey) {
	    lastlen = keybuflen;
	    func = f;
	    str = s;
	    lastc = c;
	}
	if(!keyisprefix((loc ? localkeymap : km), keybuf))
	    break;
    }
    if(!lastlen && keybuflen)
	lastlen = keybuflen;
    else
	c = lastc;
    if(lastlen != keybuflen) {
	unmetafy(keybuf + lastlen, &keybuflen);
	ungetkeys(keybuf+lastlen, keybuflen);
	if(vichgflag)
	    vichgbufptr -= keybuflen;
	keybuf[lastlen] = 0;
    }
    *funcp = func;
    *strp = str;
    return keybuf;
}

/**/
static int
getkeybuf(int w)
{
    int c = getkey(w);

    if(c < 0)
	return EOF;
    if(keybuflen + 3 > keybufsz)
	keybuf = realloc(keybuf, keybufsz *= 2);
    if(imeta(c)) {
	keybuf[keybuflen++] = Meta;
	keybuf[keybuflen++] = c ^ 32;
    } else
	keybuf[keybuflen++] = c;
    keybuf[keybuflen] = 0;
    return c;
}

/* Push back the last command sequence read by getkeymapcmd(). *
 * Must be executed at most once after each getkeymapcmd().    */

/**/
mod_export void
ungetkeycmd(void)
{
    ungetkeys(keybuf, keybuflen);
}

/* read a command from the current keymap, with widgets */

/**/
mod_export Thingy
getkeycmd(void)
{
    Thingy func;
    int hops = 0;
    char *seq, *str;

    sentstring:
    seq = getkeymapcmd(curkeymap, &func, &str);
    if(!*seq)
	return NULL;
    if(!func) {
	int len;
	char *pb;

	if (++hops == 20) {
	    zerr("string inserting another one too many times", NULL, 0);
	    hops = 0;
	    return NULL;
	}
	pb = unmetafy(ztrdup(str), &len);
	ungetkeys(pb, len);
	zfree(pb, strlen(str) + 1);
	goto sentstring;
    }
    if (func == Th(z_executenamedcmd) && !statusline) {
	while(func == Th(z_executenamedcmd))
	    func = executenamedcommand("execute: ");
	if(!func)
	    func = t_undefinedkey;
	else if(func != Th(z_executelastnamedcmd)) {
	    unrefthingy(lastnamed);
	    lastnamed = refthingy(func);
	}
    }
    if (func == Th(z_executelastnamedcmd))
	func = lastnamed;
    return func;
}
