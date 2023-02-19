/*
 * mem.c - memory management
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
#include "mem.pro"

/*
	There are two ways to allocate memory in zsh.  The first way is
	to call zalloc/zcalloc, which call malloc/calloc directly.  It
	is legal to call realloc() or free() on memory allocated this way.
	The second way is to call zhalloc/hcalloc, which allocates memory
	from one of the memory pools on the heap stack.  Such memory pools 
	will automatically created when the heap allocation routines are
	called.  To be sure that they are freed at appropriate times
	one should call pushheap() before one starts using heaps and
	popheap() after that (when the memory allocated on the heaps since
	the last pushheap() isn't needed anymore).
	pushheap() saves the states of all currently allocated heaps and
	popheap() resets them to the last state saved and destroys the
	information about that state.  If you called pushheap() and
	allocated some memory on the heaps and then come to a place where
	you don't need the allocated memory anymore but you still want
	to allocate memory on the heap, you should call freeheap().  This
	works like popheap(), only that it doesn't free the information
	about the heap states (i.e. the heaps are like after the call to
	pushheap() and you have to call popheap some time later).

	Memory allocated in this way does not have to be freed explicitly;
	it will all be freed when the pool is destroyed.  In fact,
	attempting to free this memory may result in a core dump.

	If possible, the heaps are allocated using mmap() so that the
	(*real*) heap isn't filled up with empty zsh heaps. If mmap()
	is not available and zsh's own allocator is used, we use a simple trick
	to avoid that: we allocate a large block of memory before allocating
	a heap pool, this memory is freed again immediately after the pool
	is allocated. If there are only small blocks on the free list this
	guarantees that the memory for the pool is at the end of the memory
	which means that we can give it back to the system when the pool is
	freed.
*/

#if defined(HAVE_SYS_MMAN_H) && defined(HAVE_MMAP) && defined(HAVE_MUNMAP)

#include <sys/mman.h>

#if defined(MAP_ANONYMOUS) && defined(MAP_PRIVATE)

#define USE_MMAP 1
#define MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE)

#endif
#endif

#ifdef ZSH_MEM_WARNING
# ifndef DEBUG
#  define DEBUG 1
# endif
#endif

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)

static int h_m[1025], h_push, h_pop, h_free;

#endif

/* Make sure we align to the longest fundamental type. */
union mem_align {
    zlong l;
    double d;
};

#define H_ISIZE  sizeof(union mem_align)
#define HEAPSIZE (16384 - H_ISIZE)
#define HEAP_ARENA_SIZE (HEAPSIZE - sizeof(struct heap))
#define HEAPFREE (16384 - H_ISIZE)

/* list of zsh heaps */

static Heap heaps;

/* a heap with free space, not always correct (it will be the last heap
 * if that was newly allocated but it may also be another one) */

static Heap fheap;

/* Use new heaps from now on. This returns the old heap-list. */

/**/
mod_export Heap
new_heaps(void)
{
    Heap h;

    queue_signals();
    h = heaps;

    fheap = heaps = NULL;
    unqueue_signals();

    return h;
}

/* Re-install the old heaps again, freeing the new ones. */

/**/
mod_export void
old_heaps(Heap old)
{
    Heap h, n;

    queue_signals();
    for (h = heaps; h; h = n) {
	n = h->next;
	DPUTS(h->sp, "BUG: old_heaps() with pushed heaps");
#ifdef USE_MMAP
	munmap((void *) h, sizeof(*h));
#else
	zfree(h, sizeof(*h));
#endif
    }
    heaps = old;
    fheap = NULL;
    unqueue_signals();
}

/* Temporarily switch to other heaps (or back again). */

/**/
mod_export Heap
switch_heaps(Heap new)
{
    Heap h;

    queue_signals();
    h = heaps;

    heaps = new;
    fheap = NULL;
    unqueue_signals();

    return h;
}

/* save states of zsh heaps */

/**/
mod_export void
pushheap(void)
{
    Heap h;
    Heapstack hs;

    queue_signals();

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)
    h_push++;
#endif

    for (h = heaps; h; h = h->next) {
	DPUTS(!h->used, "BUG: empty heap");
	hs = (Heapstack) zalloc(sizeof(*hs));
	hs->next = h->sp;
	h->sp = hs;
	hs->used = h->used;
    }
    unqueue_signals();
}

/* reset heaps to previous state */

/**/
mod_export void
freeheap(void)
{
    Heap h, hn, hl = NULL;

    queue_signals();

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)
    h_free++;
#endif

    fheap = NULL;
    for (h = heaps; h; h = hn) {
	hn = h->next;
	if (h->sp) {
#ifdef ZSH_MEM_DEBUG
	    memset(arena(h) + h->sp->used, 0xff, h->used - h->sp->used);
#endif
	    h->used = h->sp->used;
	    if (!fheap && h->used < HEAP_ARENA_SIZE)
		fheap = h;
	    hl = h;
	} else {
#ifdef USE_MMAP
	    munmap((void *) h, h->size);
#else
	    zfree(h, HEAPSIZE);
#endif
	}
    }
    if (hl)
	hl->next = NULL;
    else
	heaps = NULL;

    unqueue_signals();
}

/* reset heap to previous state and destroy state information */

/**/
mod_export void
popheap(void)
{
    Heap h, hn, hl = NULL;
    Heapstack hs;

    queue_signals();

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)
    h_pop++;
#endif

    fheap = NULL;
    for (h = heaps; h; h = hn) {
	hn = h->next;
	if ((hs = h->sp)) {
	    h->sp = hs->next;
#ifdef ZSH_MEM_DEBUG
	    memset(arena(h) + hs->used, 0xff, h->used - hs->used);
#endif
	    h->used = hs->used;
	    if (!fheap && h->used < HEAP_ARENA_SIZE)
		fheap = h;
	    zfree(hs, sizeof(*hs));

	    hl = h;
	} else {
#ifdef USE_MMAP
	    munmap((void *) h, h->size);
#else
	    zfree(h, HEAPSIZE);
#endif
	}
    }
    if (hl)
	hl->next = NULL;
    else
	heaps = NULL;

    unqueue_signals();
}

/* allocate memory from the current memory pool */

/**/
mod_export void *
zhalloc(size_t size)
{
    Heap h;
    size_t n;

    size = (size + H_ISIZE - 1) & ~(H_ISIZE - 1);

    queue_signals();

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)
    h_m[size < (1024 * H_ISIZE) ? (size / H_ISIZE) : 1024]++;
#endif

    /* find a heap with enough free space */

    for (h = ((fheap && HEAP_ARENA_SIZE >= (size + fheap->used)) ? fheap : heaps);
	 h; h = h->next) {
	if (HEAP_ARENA_SIZE >= (n = size + h->used)) {
	    void *ret;

	    h->used = n;
	    ret = arena(h) + n - size;
	    unqueue_signals();
	    return ret;
	}
    }
    {
	Heap hp;
        /* not found, allocate new heap */
#if defined(ZSH_MEM) && !defined(USE_MMAP)
	static int called = 0;
	void *foo = called ? (void *)malloc(HEAPFREE) : NULL;
            /* tricky, see above */
#endif

	n = HEAP_ARENA_SIZE > size ? HEAPSIZE : size + sizeof(*h);
	for (hp = NULL, h = heaps; h; hp = h, h = h->next);

#ifdef USE_MMAP
	{
	    static size_t pgsz = 0;

	    if (!pgsz) {

#ifdef _SC_PAGESIZE
		pgsz = sysconf(_SC_PAGESIZE);     /* SVR4 */
#else
# ifdef _SC_PAGE_SIZE
		pgsz = sysconf(_SC_PAGE_SIZE);    /* HPUX */
# else
		pgsz = getpagesize();
# endif
#endif

		pgsz--;
	    }
	    n = (n + pgsz) & ~pgsz;
	    h = (Heap) mmap(NULL, n, PROT_READ | PROT_WRITE,
			    MMAP_FLAGS, -1, 0);
	    if (h == ((Heap) -1)) {
		zerr("fatal error: out of heap memory", NULL, 0);
		exit(1);
	    }
	    h->size = n;
	}
#else
	h = (Heap) zalloc(n);
#endif

#if defined(ZSH_MEM) && !defined(USE_MMAP)
	if (called)
	    zfree(foo, HEAPFREE);
	called = 1;
#endif

	h->used = size;
	h->next = NULL;
	h->sp = NULL;

	if (hp)
	    hp->next = h;
	else
	    heaps = h;
	fheap = h;

	unqueue_signals();
	return arena(h);
    }
}

/**/
mod_export void *
hrealloc(char *p, size_t old, size_t new)
{
    Heap h, ph;

    old = (old + H_ISIZE - 1) & ~(H_ISIZE - 1);
    new = (new + H_ISIZE - 1) & ~(H_ISIZE - 1);

    if (old == new)
	return p;
    if (!old && !p)
	return zhalloc(new);

    /* find the heap with p */

    queue_signals();
    for (h = heaps, ph = NULL; h; ph = h, h = h->next)
	if (p >= arena(h) && p < arena(h) + HEAP_ARENA_SIZE)
	    break;

    DPUTS(!h, "BUG: hrealloc() called for non-heap memory.");
    DPUTS(h->sp && arena(h) + h->sp->used > p,
	  "BUG: hrealloc() wants to realloc pushed memory");

    if (p + old < arena(h) + h->used) {
	if (new > old) {
	    char *ptr = (char *) zhalloc(new);
	    memcpy(ptr, p, old);
#ifdef ZSH_MEM_DEBUG
	    memset(p, 0xff, old);
#endif
	    unqueue_signals();
	    return ptr;
	} else {
	    unqueue_signals();
	    return new ? p : NULL;
	}
    }

    DPUTS(p + old != arena(h) + h->used, "BUG: hrealloc more than allocated");

    if (p == arena(h)) {
	if (!new) {
	    if (ph)
		ph->next = h->next;
	    else
		heaps = h->next;
	    fheap = NULL;
#ifdef USE_MMAP
	    munmap((void *) h, h->size);
#else
	    zfree(h, HEAPSIZE);
#endif
	    unqueue_signals();
	    return NULL;
	}
#ifndef USE_MMAP
	if (old > HEAP_ARENA_SIZE || new > HEAP_ARENA_SIZE) {
	    size_t n = HEAP_ARENA_SIZE > new ? HEAPSIZE : new + sizeof(*h);

	    if (ph)
		ph->next = h = (Heap) realloc(h, n);
	    else
		heaps = h = (Heap) realloc(h, n);
	}
	h->used = new;
	unqueue_signals();
	return arena(h);
#endif
    }
#ifndef USE_MMAP
    DPUTS(h->used > HEAP_ARENA_SIZE, "BUG: hrealloc at invalid address");
#endif
    if (h->used + (new - old) <= HEAP_ARENA_SIZE) {
	h->used += new - old;
	unqueue_signals();
	return p;
    } else {
	char *t = zhalloc(new);
	memcpy(t, p, old > new ? new : old);
	h->used -= old;
#ifdef ZSH_MEM_DEBUG
	memset(p, 0xff, old);
#endif
	unqueue_signals();
	return t;
    }
}

/* allocate memory from the current memory pool and clear it */

/**/
mod_export void *
hcalloc(size_t size)
{
    void *ptr;

    ptr = zhalloc(size);
    memset(ptr, 0, size);
    return ptr;
}

/* allocate permanent memory */

/**/
mod_export void *
zalloc(size_t size)
{
    void *ptr;

    if (!size)
	size = 1;
    queue_signals();
    if (!(ptr = (void *) malloc(size))) {
	zerr("fatal error: out of memory", NULL, 0);
	exit(1);
    }
    unqueue_signals();

    return ptr;
}

/**/
mod_export void *
zcalloc(size_t size)
{
    void *ptr;

    if (!size)
	size = 1;
    queue_signals();
    if (!(ptr = (void *) malloc(size))) {
	zerr("fatal error: out of memory", NULL, 0);
	exit(1);
    }
    unqueue_signals();
    memset(ptr, 0, size);

    return ptr;
}

/* This front-end to realloc is used to make sure we have a realloc *
 * that conforms to POSIX realloc.  Older realloc's can fail if     *
 * passed a NULL pointer, but POSIX realloc should handle this.  A  *
 * better solution would be for configure to check if realloc is    *
 * POSIX compliant, but I'm not sure how to do that.                */

/**/
mod_export void *
zrealloc(void *ptr, size_t size)
{
    queue_signals();
    if (ptr) {
	if (size) {
	    /* Do normal realloc */
	    if (!(ptr = (void *) realloc(ptr, size))) {
		zerr("fatal error: out of memory", NULL, 0);
		exit(1);
	    }
	    unqueue_signals();
	    return ptr;
	}
	else
	    /* If ptr is not NULL, but size is zero, *
	     * then object pointed to is freed.      */
	    free(ptr);

	ptr = NULL;
    } else {
	/* If ptr is NULL, then behave like malloc */
	ptr = malloc(size);
    }
    unqueue_signals();

    return ptr;
}

/**/
#ifdef ZSH_MEM

/*
   Below is a simple segment oriented memory allocator for systems on
   which it is better than the system's one. Memory is given in blocks
   aligned to an integer multiple of sizeof(union mem_align), which will
   probably be 64-bit as it is the longer of zlong or double. Each block is
   preceded by a header which contains the length of the data part (in
   bytes). In allocated blocks only this field of the structure m_hdr is
   senseful. In free blocks the second field (next) is a pointer to the next
   free segment on the free list.

   On top of this simple allocator there is a second allocator for small
   chunks of data. It should be both faster and less space-consuming than
   using the normal segment mechanism for such blocks.
   For the first M_NSMALL-1 possible sizes memory is allocated in arrays
   that can hold M_SNUM blocks. Each array is stored in one segment of the
   main allocator. In these segments the third field of the header structure
   (free) contains a pointer to the first free block in the array. The
   last field (used) gives the number of already used blocks in the array.

   If the macro name ZSH_MEM_DEBUG is defined, some information about the memory
   usage is stored. This information can than be viewed by calling the
   builtin `mem' (which is only available if ZSH_MEM_DEBUG is set).

   If ZSH_MEM_WARNING is defined, error messages are printed in case of errors.

   If ZSH_SECURE_FREE is defined, free() checks if the given address is really
   one that was returned by malloc(), it ignores it if it wasn't (printing
   an error message if ZSH_MEM_WARNING is also defined).
*/
#if !defined(__hpux) && !defined(DGUX) && !defined(__osf__)
# if defined(_BSD)
#  ifndef HAVE_BRK_PROTO
   extern int brk _((caddr_t));
#  endif
#  ifndef HAVE_SBRK_PROTO
   extern caddr_t sbrk _((int));
#  endif
# else
#  ifndef HAVE_BRK_PROTO
   extern int brk _((void *));
#  endif
#  ifndef HAVE_SBRK_PROTO
   extern void *sbrk _((int));
#  endif
# endif
#endif

#if defined(_BSD) && !defined(STDC_HEADERS)
# define FREE_RET_T   int
# define FREE_ARG_T   char *
# define FREE_DO_RET
# define MALLOC_RET_T char *
# define MALLOC_ARG_T size_t
#else
# define FREE_RET_T   void
# define FREE_ARG_T   void *
# define MALLOC_RET_T void *
# define MALLOC_ARG_T size_t
#endif

/* structure for building free list in blocks holding small blocks */

struct m_shdr {
    struct m_shdr *next;	/* next one on free list */
#ifdef PAD_64_BIT
    /* dummy to make this 64-bit aligned */
    struct m_shdr *dummy;
#endif
};

struct m_hdr {
    zlong len;			/* length of memory block */
#if defined(PAD_64_BIT) && !defined(ZSH_64_BIT_TYPE)
    /* either 1 or 2 zlong's, whichever makes up 64 bits. */
    zlong dummy1;
#endif
    struct m_hdr *next;		/* if free: next on free list
				   if block of small blocks: next one with
				                 small blocks of same size*/
    struct m_shdr *free;	/* if block of small blocks: free list */
    zlong used;			/* if block of small blocks: number of used
				                                     blocks */
#if defined(PAD_64_BIT) && !defined(ZSH_64_BIT_TYPE)
    zlong dummy2;
#endif
};


/* alignment for memory blocks */

#define M_ALIGN (sizeof(union mem_align))

/* length of memory header, length of first field of memory header and
   minimal size of a block left free (if we allocate memory and take a
   block from the free list that is larger than needed, it must have at
   least M_MIN extra bytes to be splitted; if it has, the rest is put on
   the free list) */

#define M_HSIZE (sizeof(struct m_hdr))
#if defined(PAD_64_BIT) && !defined(ZSH_64_BIT_TYPE)
# define M_ISIZE (2*sizeof(zlong))
#else
# define M_ISIZE (sizeof(zlong))
#endif
#define M_MIN   (2 * M_ISIZE)

/* M_FREE  is the number of bytes that have to be free before memory is
 *         given back to the system
 * M_KEEP  is the number of bytes that will be kept when memory is given
 *         back; note that this has to be less than M_FREE
 * M_ALLOC is the number of extra bytes to request from the system */

#define M_FREE  32768
#define M_KEEP  16384
#define M_ALLOC M_KEEP

/* a pointer to the last free block, a pointer to the free list (the blocks
   on this list are kept in order - lowest address first) */

static struct m_hdr *m_lfree, *m_free;

/* system's pagesize */

static long m_pgsz = 0;

/* the highest and the lowest valid memory addresses, kept for fast validity
   checks in free() and to find out if and when we can give memory back to
   the system */

static char *m_high, *m_low;

/* Management of blocks for small blocks:
   Such blocks are kept in lists (one list for each of the sizes that are
   allocated in such blocks).  The lists are stored in the m_small array.
   M_SIDX() calculates the index into this array for a given size.  M_SNUM
   is the size (in small blocks) of such blocks.  M_SLEN() calculates the
   size of the small blocks held in a memory block, given a pointer to the
   header of it.  M_SBLEN() gives the size of a memory block that can hold
   an array of small blocks, given the size of these small blocks.  M_BSLEN()
   caculates the size of the small blocks held in a memory block, given the
   length of that block (including the header of the memory block.  M_NSMALL
   is the number of possible block sizes that small blocks should be used
   for. */


#define M_SIDX(S)  ((S) / M_ISIZE)
#define M_SNUM     128
#define M_SLEN(M)  ((M)->len / M_SNUM)
#if defined(PAD_64_BIT) && !defined(ZSH_64_BIT_TYPE)
/* Include the dummy in the alignment */
#define M_SBLEN(S) ((S) * M_SNUM + sizeof(struct m_shdr *) +  \
		    2*sizeof(zlong) + sizeof(struct m_hdr *))
#define M_BSLEN(S) (((S) - sizeof(struct m_shdr *) -  \
		     2*sizeof(zlong) - sizeof(struct m_hdr *)) / M_SNUM)
#else
#define M_SBLEN(S) ((S) * M_SNUM + sizeof(struct m_shdr *) +  \
		    sizeof(zlong) + sizeof(struct m_hdr *))
#define M_BSLEN(S) (((S) - sizeof(struct m_shdr *) -  \
		     sizeof(zlong) - sizeof(struct m_hdr *)) / M_SNUM)
#endif
#define M_NSMALL    8

static struct m_hdr *m_small[M_NSMALL];

#ifdef ZSH_MEM_DEBUG

static int m_s = 0, m_b = 0;
static int m_m[1025], m_f[1025];

static struct m_hdr *m_l;

#endif /* ZSH_MEM_DEBUG */

MALLOC_RET_T
malloc(MALLOC_ARG_T size)
{
    struct m_hdr *m, *mp, *mt;
    long n, s, os = 0;
#ifndef USE_MMAP
    struct heap *h, *hp, *hf = NULL, *hfp = NULL;
#endif

    /* some systems want malloc to return the highest valid address plus one
       if it is called with an argument of zero */

    if (!size)
	return (MALLOC_RET_T) m_high;

    queue_signals();  /* just queue signals rather than handling them */

    /* first call, get page size */

    if (!m_pgsz) {

#ifdef _SC_PAGESIZE
	m_pgsz = sysconf(_SC_PAGESIZE);     /* SVR4 */
#else
# ifdef _SC_PAGE_SIZE
	m_pgsz = sysconf(_SC_PAGE_SIZE);    /* HPUX */
# else
	m_pgsz = getpagesize();
# endif
#endif

	m_free = m_lfree = NULL;
    }
    size = (size + M_ALIGN - 1) & ~(M_ALIGN - 1);

    /* Do we need a small block? */

    if ((s = M_SIDX(size)) && s < M_NSMALL) {
	/* yep, find a memory block with free small blocks of the
	   appropriate size (if we find it in this list, this means that
	   it has room for at least one more small block) */
	for (mp = NULL, m = m_small[s]; m && !m->free; mp = m, m = m->next);

	if (m) {
	    /* we found one */
	    struct m_shdr *sh = m->free;

	    m->free = sh->next;
	    m->used++;

	    /* if all small blocks in this block are allocated, the block is 
	       put at the end of the list blocks with small blocks of this
	       size (i.e., we try to keep blocks with free blocks at the
	       beginning of the list, to make the search faster) */

	    if (m->used == M_SNUM && m->next) {
		for (mt = m; mt->next; mt = mt->next);

		mt->next = m;
		if (mp)
		    mp->next = m->next;
		else
		    m_small[s] = m->next;
		m->next = NULL;
	    }
#ifdef ZSH_MEM_DEBUG
	    m_m[size / M_ISIZE]++;
#endif

	    unqueue_signals();
	    return (MALLOC_RET_T) sh;
	}
	/* we still want a small block but there were no block with a free
	   small block of the requested size; so we use the real allocation
	   routine to allocate a block for small blocks of this size */
	os = size;
	size = M_SBLEN(size);
    } else
	s = 0;

    /* search the free list for an block of at least the requested size */
    for (mp = NULL, m = m_free; m && m->len < size; mp = m, m = m->next);

#ifndef USE_MMAP

    /* if there is an empty zsh heap at a lower address we steal it and take
       the memory from it, putting the rest on the free list (remember
       that the blocks on the free list are ordered) */

    for (hp = NULL, h = heaps; h; hp = h, h = h->next)
	if (!h->used &&
	    (!hf || h < hf) &&
	    (!m || ((char *)m) > ((char *)h)))
	    hf = h, hfp = hp;

    if (hf) {
	/* we found such a heap */
	Heapstack hso, hsn;

	/* delete structures on the list holding the heap states */
	for (hso = hf->sp; hso; hso = hsn) {
	    hsn = hso->next;
	    zfree(hso, sizeof(*hso));
	}
	/* take it from the list of heaps */
	if (hfp)
	    hfp->next = hf->next;
	else
	    heaps = hf->next;
	/* now we simply free it and than search the free list again */
	zfree(hf, HEAPSIZE);

	for (mp = NULL, m = m_free; m && m->len < size; mp = m, m = m->next);
    }
#endif
    if (!m) {
	long nal;
	/* no matching free block was found, we have to request new
	   memory from the system */
	n = (size + M_HSIZE + M_ALLOC + m_pgsz - 1) & ~(m_pgsz - 1);

	if (((char *)(m = (struct m_hdr *)sbrk(n))) == ((char *)-1)) {
	    DPUTS(1, "MEM: allocation error at sbrk.");
	    unqueue_signals();
	    return NULL;
	}
	if ((nal = ((long)(char *)m) & (M_ALIGN-1))) {
	    if ((char *)sbrk(M_ALIGN - nal) == (char *)-1) {
		DPUTS(1, "MEM: allocation error at sbrk.");
		unqueue_signals();
		return NULL;
	    }
	    m = (struct m_hdr *) ((char *)m + (M_ALIGN - nal));
	}
	/* set m_low, for the check in free() */
	if (!m_low)
	    m_low = (char *)m;

#ifdef ZSH_MEM_DEBUG
	m_s += n;

	if (!m_l)
	    m_l = m;
#endif

	/* save new highest address */
	m_high = ((char *)m) + n;

	/* initialize header */
	m->len = n - M_ISIZE;
	m->next = NULL;

	/* put it on the free list and set m_lfree pointing to it */
	if ((mp = m_lfree))
	    m_lfree->next = m;
	m_lfree = m;
    }
    if ((n = m->len - size) > M_MIN) {
	/* the block we want to use has more than M_MIN bytes plus the
	   number of bytes that were requested; we split it in two and
	   leave the rest on the free list */
	struct m_hdr *mtt = (struct m_hdr *)(((char *)m) + M_ISIZE + size);

	mtt->len = n - M_ISIZE;
	mtt->next = m->next;

	m->len = size;

	/* put the rest on the list */
	if (m_lfree == m)
	    m_lfree = mtt;

	if (mp)
	    mp->next = mtt;
	else
	    m_free = mtt;
    } else if (mp) {
	/* the block we found wasn't the first one on the free list */
	if (m == m_lfree)
	    m_lfree = mp;
	mp->next = m->next;
    } else {
	/* it was the first one */
	m_free = m->next;
	if (m == m_lfree)
	    m_lfree = m_free;
    }

    if (s) {
	/* we are allocating a block that should hold small blocks */
	struct m_shdr *sh, *shn;

	/* build the free list in this block and set `used' filed */
	m->free = sh = (struct m_shdr *)(((char *)m) +
					 sizeof(struct m_hdr) + os);

	for (n = M_SNUM - 2; n--; sh = shn)
	    shn = sh->next = sh + s;
	sh->next = NULL;

	m->used = 1;

	/* put the block on the list of blocks holding small blocks if
	   this size */
	m->next = m_small[s];
	m_small[s] = m;

#ifdef ZSH_MEM_DEBUG
	m_m[os / M_ISIZE]++;
#endif

	unqueue_signals();
	return (MALLOC_RET_T) (((char *)m) + sizeof(struct m_hdr));
    }
#ifdef ZSH_MEM_DEBUG
    m_m[m->len < (1024 * M_ISIZE) ? (m->len / M_ISIZE) : 1024]++;
#endif

    unqueue_signals();
    return (MALLOC_RET_T) & m->next;
}

/* this is an internal free(); the second argument may, but need not hold
   the size of the block the first argument is pointing to; if it is the
   right size of this block, freeing it will be faster, though; the value
   0 for this parameter means: `don't know' */

/**/
mod_export void
zfree(void *p, int sz)
{
    struct m_hdr *m = (struct m_hdr *)(((char *)p) - M_ISIZE), *mp, *mt = NULL;
    int i;
# ifdef DEBUG
    int osz = sz;
# endif

#ifdef ZSH_SECURE_FREE
    sz = 0;
#else
    sz = (sz + M_ALIGN - 1) & ~(M_ALIGN - 1);
#endif

    if (!p)
	return;

    /* first a simple check if the given address is valid */
    if (((char *)p) < m_low || ((char *)p) > m_high ||
	((long)p) & (M_ALIGN - 1)) {
	DPUTS(1, "BUG: attempt to free storage at invalid address");
	return;
    }

    queue_signals();

  fr_rec:

    if ((i = sz / M_ISIZE) < M_NSMALL || !sz)
	/* if the given sizes says that it is a small block, find the
	   memory block holding it; we search all blocks with blocks
	   of at least the given size; if the size parameter is zero,
	   this means, that all blocks are searched */
	for (; i < M_NSMALL; i++) {
	    for (mp = NULL, mt = m_small[i];
		 mt && (((char *)mt) > ((char *)p) ||
			(((char *)mt) + mt->len) < ((char *)p));
		 mp = mt, mt = mt->next);

	    if (mt) {
		/* we found the block holding the small block */
		struct m_shdr *sh = (struct m_shdr *)p;

#ifdef ZSH_SECURE_FREE
		struct m_shdr *sh2;

		/* check if the given address is equal to the address of
		   the first small block plus an integer multiple of the
		   block size */
		if ((((char *)p) - (((char *)mt) + sizeof(struct m_hdr))) %
		    M_BSLEN(mt->len)) {

		    DPUTS(1, "BUG: attempt to free storage at invalid address");
		    unqueue_signals();
		    return;
		}
		/* check, if the address is on the (block-intern) free list */
		for (sh2 = mt->free; sh2; sh2 = sh2->next)
		    if (((char *)p) == ((char *)sh2)) {

			DPUTS(1, "BUG: attempt to free already free storage");
			unqueue_signals();
			return;
		    }
#endif
		DPUTS(M_BSLEN(mt->len) < osz,
		      "BUG: attempt to free more than allocated.");

#ifdef ZSH_MEM_DEBUG
		m_f[M_BSLEN(mt->len) / M_ISIZE]++;
		memset(sh, 0xff, M_BSLEN(mt->len));
#endif

		/* put the block onto the free list */
		sh->next = mt->free;
		mt->free = sh;

		if (--mt->used) {
		    /* if there are still used blocks in this block, we
		       put it at the beginning of the list with blocks
		       holding small blocks of the same size (since we
		       know that there is at least one free block in it,
		       this will make allocation of small blocks faster;
		       it also guarantees that long living memory blocks
		       are preferred over younger ones */
		    if (mp) {
			mp->next = mt->next;
			mt->next = m_small[i];
			m_small[i] = mt;
		    }
		    unqueue_signals();
		    return;
		}
		/* if there are no more used small blocks in this
		   block, we free the whole block */
		if (mp)
		    mp->next = mt->next;
		else
		    m_small[i] = mt->next;

		m = mt;
		p = (void *) & m->next;

		break;
	    } else if (sz) {
		/* if we didn't find a block and a size was given, try it
		   again as if no size were given */
		sz = 0;
		goto fr_rec;
	    }
	}
#ifdef ZSH_MEM_DEBUG
    if (!mt)
	m_f[m->len < (1024 * M_ISIZE) ? (m->len / M_ISIZE) : 1024]++;
#endif

#ifdef ZSH_SECURE_FREE
    /* search all memory blocks, if one of them is at the given address */
    for (mt = (struct m_hdr *)m_low;
	 ((char *)mt) < m_high;
	 mt = (struct m_hdr *)(((char *)mt) + M_ISIZE + mt->len))
	if (((char *)p) == ((char *)&mt->next))
	    break;

    /* no block was found at the given address */
    if (((char *)mt) >= m_high) {
	DPUTS(1, "BUG: attempt to free storage at invalid address");
	unqueue_signals();
	return;
    }
#endif

    /* see if the block is on the free list */
    for (mp = NULL, mt = m_free; mt && mt < m; mp = mt, mt = mt->next);

    if (m == mt) {
	/* it is, ouch! */
	DPUTS(1, "BUG: attempt to free already free storage");
	unqueue_signals();
	return;
    }
    DPUTS(m->len < osz, "BUG: attempt to free more than allocated");
#ifdef ZSH_MEM_DEBUG
    memset(p, 0xff, m->len);
#endif
    if (mt && ((char *)mt) == (((char *)m) + M_ISIZE + m->len)) {
	/* the block after the one we are freeing is free, we put them
	   together */
	m->len += mt->len + M_ISIZE;
	m->next = mt->next;

	if (mt == m_lfree)
	    m_lfree = m;
    } else
	m->next = mt;

    if (mp && ((char *)m) == (((char *)mp) + M_ISIZE + mp->len)) {
	/* the block before the one we are freeing is free, we put them
	   together */
	mp->len += m->len + M_ISIZE;
	mp->next = m->next;

	if (m == m_lfree)
	    m_lfree = mp;
    } else if (mp)
	/* otherwise, we just put it on the free list */
	mp->next = m;
    else {
	m_free = m;
	if (!m_lfree)
	    m_lfree = m_free;
    }

    /* if the block we have just freed was at the end of the process heap
       and now there is more than one page size of memory, we can give
       it back to the system (and we do it ;-) */
    if ((((char *)m_lfree) + M_ISIZE + m_lfree->len) == m_high &&
	m_lfree->len >= m_pgsz + M_MIN + M_FREE) {
	long n = (m_lfree->len - M_MIN - M_KEEP) & ~(m_pgsz - 1);

	m_lfree->len -= n;
#ifdef HAVE_BRK
	if (brk(m_high -= n) == -1) {
#else
	m_high -= n;
	if (sbrk(-n) == (void *)-1) {
#endif /* HAVE_BRK */
	    DPUTS(1, "MEM: allocation error at brk.");
	}

#ifdef ZSH_MEM_DEBUG
	m_b += n;
#endif
    }
    unqueue_signals();
}

FREE_RET_T
free(FREE_ARG_T p)
{
    zfree(p, 0);		/* 0 means: size is unknown */

#ifdef FREE_DO_RET
    return 0;
#endif
}

/* this one is for strings (and only strings, real strings, real C strings,
   those that have a zero byte at the end) */

/**/
mod_export void
zsfree(char *p)
{
    if (p)
	zfree(p, strlen(p) + 1);
}

MALLOC_RET_T
realloc(MALLOC_RET_T p, MALLOC_ARG_T size)
{
    struct m_hdr *m = (struct m_hdr *)(((char *)p) - M_ISIZE), *mp, *mt;
    char *r;
    int i, l = 0;

    /* some system..., see above */
    if (!p && size)
	return (MALLOC_RET_T) malloc(size);
    /* and some systems even do this... */
    if (!p || !size)
	return (MALLOC_RET_T) p;

    queue_signals();  /* just queue signals caught rather than handling them */

    /* check if we are reallocating a small block, if we do, we have
       to compute the size of the block from the sort of block it is in */
    for (i = 0; i < M_NSMALL; i++) {
	for (mp = NULL, mt = m_small[i];
	     mt && (((char *)mt) > ((char *)p) ||
		    (((char *)mt) + mt->len) < ((char *)p));
	     mp = mt, mt = mt->next);

	if (mt) {
	    l = M_BSLEN(mt->len);
	    break;
	}
    }
    if (!l)
	/* otherwise the size of the block is in the memory just before
	   the given address */
	l = m->len;

    /* now allocate the new block, copy the old contents, and free the
       old block */
    r = malloc(size);
    memcpy(r, (char *)p, (size > l) ? l : size);
    free(p);

    unqueue_signals();
    return (MALLOC_RET_T) r;
}

MALLOC_RET_T
calloc(MALLOC_ARG_T n, MALLOC_ARG_T size)
{
    long l;
    char *r;

    if (!(l = n * size))
	return (MALLOC_RET_T) m_high;

    r = malloc(l);

    memset(r, 0, l);

    return (MALLOC_RET_T) r;
}

#ifdef ZSH_MEM_DEBUG

/**/
int
bin_mem(char *name, char **argv, char *ops, int func)
{
    int i, ii, fi, ui, j;
    struct m_hdr *m, *mf, *ms;
    char *b, *c, buf[40];
    long u = 0, f = 0, to, cu;

    queue_signals();
    if (ops['v']) {
	printf("The lower and the upper addresses of the heap. Diff gives\n");
	printf("the difference between them, i.e. the size of the heap.\n\n");
    }
    printf("low mem %ld\t high mem %ld\t diff %ld\n",
	   (long)m_l, (long)m_high, (long)(m_high - ((char *)m_l)));

    if (ops['v']) {
	printf("\nThe number of bytes that were allocated using sbrk() and\n");
	printf("the number of bytes that were given back to the system\n");
	printf("via brk().\n");
    }
    printf("\nsbrk %d\tbrk %d\n", m_s, m_b);

    if (ops['v']) {
	printf("\nInformation about the sizes that were allocated or freed.\n");
	printf("For each size that were used the number of mallocs and\n");
	printf("frees is shown. Diff gives the difference between these\n");
	printf("values, i.e. the number of blocks of that size that is\n");
	printf("currently allocated. Total is the product of size and diff,\n");
	printf("i.e. the number of bytes that are allocated for blocks of\n");
	printf("this size. The last field gives the accumulated number of\n");
	printf("bytes for all sizes.\n");
    }
    printf("\nsize\tmalloc\tfree\tdiff\ttotal\tcum\n");
    for (i = 0, cu = 0; i < 1024; i++)
	if (m_m[i] || m_f[i]) {
	    to = (long) i * M_ISIZE * (m_m[i] - m_f[i]);
	    printf("%ld\t%d\t%d\t%d\t%ld\t%ld\n",
		   (long)i * M_ISIZE, m_m[i], m_f[i], m_m[i] - m_f[i],
		   to, (cu += to));
	}

    if (m_m[i] || m_f[i])
	printf("big\t%d\t%d\t%d\n", m_m[i], m_f[i], m_m[i] - m_f[i]);

    if (ops['v']) {
	printf("\nThe list of memory blocks. For each block the following\n");
	printf("information is shown:\n\n");
	printf("num\tthe number of this block\n");
	printf("tnum\tlike num but counted separatedly for used and free\n");
	printf("\tblocks\n");
	printf("addr\tthe address of this block\n");
	printf("len\tthe length of the block\n");
	printf("state\tthe state of this block, this can be:\n");
	printf("\t  used\tthis block is used for one big block\n");
	printf("\t  free\tthis block is free\n");
	printf("\t  small\tthis block is used for an array of small blocks\n");
	printf("cum\tthe accumulated sizes of the blocks, counted\n");
	printf("\tseparatedly for used and free blocks\n");
	printf("\nFor blocks holding small blocks the number of free\n");
	printf("blocks, the number of used blocks and the size of the\n");
	printf("blocks is shown. For otherwise used blocks the first few\n");
	printf("bytes are shown as an ASCII dump.\n");
    }
    printf("\nblock list:\nnum\ttnum\taddr\t\tlen\tstate\tcum\n");
    for (m = m_l, mf = m_free, ii = fi = ui = 1; ((char *)m) < m_high;
	 m = (struct m_hdr *)(((char *)m) + M_ISIZE + m->len), ii++) {
	for (j = 0, ms = NULL; j < M_NSMALL && !ms; j++)
	    for (ms = m_small[j]; ms; ms = ms->next)
		if (ms == m)
		    break;

	if (m == mf)
	    buf[0] = '\0';
	else if (m == ms)
	    sprintf(buf, "%ld %ld %ld", (long)(M_SNUM - ms->used),
		    (long)ms->used,
		    (long)(m->len - sizeof(struct m_hdr)) / M_SNUM + 1);

	else {
	    for (i = 0, b = buf, c = (char *)&m->next; i < 20 && i < m->len;
		 i++, c++)
		*b++ = (*c >= ' ' && *c < 127) ? *c : '.';
	    *b = '\0';
	}

	printf("%d\t%d\t%ld\t%ld\t%s\t%ld\t%s\n", ii,
	       (m == mf) ? fi++ : ui++,
	       (long)m, (long)m->len,
	       (m == mf) ? "free" : ((m == ms) ? "small" : "used"),
	       (m == mf) ? (f += m->len) : (u += m->len),
	       buf);

	if (m == mf)
	    mf = mf->next;
    }

    if (ops['v']) {
	printf("\nHere is some information about the small blocks used.\n");
	printf("For each size the arrays with the number of free and the\n");
	printf("number of used blocks are shown.\n");
    }
    printf("\nsmall blocks:\nsize\tblocks (free/used)\n");

    for (i = 0; i < M_NSMALL; i++)
	if (m_small[i]) {
	    printf("%ld\t", (long)i * M_ISIZE);

	    for (ii = 0, m = m_small[i]; m; m = m->next) {
		printf("(%ld/%ld) ", (long)(M_SNUM - m->used),
		       (long)m->used);
		if (!((++ii) & 7))
		    printf("\n\t");
	    }
	    putchar('\n');
	}
    if (ops['v']) {
	printf("\n\nBelow is some information about the allocation\n");
	printf("behaviour of the zsh heaps. First the number of times\n");
	printf("pushheap(), popheap(), and freeheap() were called.\n");
    }
    printf("\nzsh heaps:\n\n");

    printf("push %d\tpop %d\tfree %d\n\n", h_push, h_pop, h_free);

    if (ops['v']) {
	printf("\nThe next list shows for several sizes the number of times\n");
	printf("memory of this size were taken from heaps.\n\n");
    }
    printf("size\tmalloc\ttotal\n");
    for (i = 0; i < 1024; i++)
	if (h_m[i])
	    printf("%ld\t%d\t%ld\n", (long)i * H_ISIZE, h_m[i],
		   (long)i * H_ISIZE * h_m[i]);
    if (h_m[1024])
	printf("big\t%d\n", h_m[1024]);

    unqueue_signals();
    return 0;
}

#endif

/**/
#else				/* not ZSH_MEM */

/**/
mod_export void
zfree(void *p, int sz)
{
    if (p)
	free(p);
}

/**/
mod_export void
zsfree(char *p)
{
    if (p)
	free(p);
}

/**/
#endif
