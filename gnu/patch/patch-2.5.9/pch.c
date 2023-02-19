/* reading patches */

/* $Id: pch.c,v 1.44 2003/05/20 14:03:17 eggert Exp $ */

/* Copyright (C) 1986, 1987, 1988 Larry Wall

   Copyright (C) 1990, 1991, 1992, 1993, 1997, 1998, 1999, 2000, 2001,
   2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#define XTERN extern
#include <common.h>
#include <backupfile.h>
#include <dirname.h>
#include <inp.h>
#include <quotearg.h>
#include <util.h>
#undef XTERN
#define XTERN
#include <pch.h>

#define INITHUNKMAX 125			/* initial dynamic allocation size */

/* Patch (diff listing) abstract type. */

static FILE *pfp;			/* patch file pointer */
static int p_says_nonexistent[2];	/* [0] for old file, [1] for new:
		0 for existent and nonempty,
		1 for existent and probably (but not necessarily) empty,
		2 for nonexistent */
static int p_rfc934_nesting;		/* RFC 934 nesting level */
static time_t p_timestamp[2];		/* timestamps in patch headers */
static off_t p_filesize;		/* size of the patch file */
static LINENUM p_first;			/* 1st line number */
static LINENUM p_newfirst;		/* 1st line number of replacement */
static LINENUM p_ptrn_lines;		/* # lines in pattern */
static LINENUM p_repl_lines;		/* # lines in replacement text */
static LINENUM p_end = -1;		/* last line in hunk */
static LINENUM p_max;			/* max allowed value of p_end */
static LINENUM p_prefix_context;	/* # of prefix context lines */
static LINENUM p_suffix_context;	/* # of suffix context lines */
static LINENUM p_input_line;		/* current line # from patch file */
static char **p_line;			/* the text of the hunk */
static size_t *p_len;			/* line length including \n if any */
static char *p_Char;			/* +, -, and ! */
static LINENUM hunkmax = INITHUNKMAX;	/* size of above arrays */
static int p_indent;			/* indent to patch */
static bool p_strip_trailing_cr;	/* true if stripping trailing \r */
static bool p_pass_comments_through;	/* true if not ignoring # lines */
static file_offset p_base;		/* where to intuit this time */
static LINENUM p_bline;			/* line # of p_base */
static file_offset p_start;		/* where intuit found a patch */
static LINENUM p_sline;			/* and the line number for it */
static LINENUM p_hunk_beg;		/* line number of current hunk */
static LINENUM p_efake = -1;		/* end of faked up lines--don't free */
static LINENUM p_bfake = -1;		/* beg of faked up lines */

enum nametype { OLD, NEW, INDEX, NONE };

static char *scan_linenum (char *, LINENUM *);
static enum diff intuit_diff_type (void);
static enum nametype best_name (char * const *, int const *);
static int prefix_components (char *, bool);
static size_t pget_line (int, int, bool, bool);
static size_t get_line (void);
static bool incomplete_line (void);
static bool grow_hunkmax (void);
static void malformed (void) __attribute__ ((noreturn));
static void next_intuit_at (file_offset, LINENUM);
static void skip_to (file_offset, LINENUM);
static char get_ed_command_letter (char const *);

/* Prepare to look for the next patch in the patch file. */

void
re_patch (void)
{
    p_first = 0;
    p_newfirst = 0;
    p_ptrn_lines = 0;
    p_repl_lines = 0;
    p_end = -1;
    p_max = 0;
    p_indent = 0;
    p_strip_trailing_cr = false;
}

/* Open the patch file at the beginning of time. */

void
open_patch_file (char const *filename)
{
    file_offset file_pos = 0;
    struct stat st;
    if (!filename || !*filename || strEQ (filename, "-"))
      {
	file_offset stdin_pos;
#if HAVE_SETMODE_DOS
	if (binary_transput)
	  {
	    if (isatty (STDIN_FILENO))
	      fatal ("cannot read binary data from tty on this platform");
	    setmode (STDIN_FILENO, O_BINARY);
	  }
#endif
	if (fstat (STDIN_FILENO, &st) != 0)
	  pfatal ("fstat");
	if (S_ISREG (st.st_mode) && (stdin_pos = file_tell (stdin)) != -1)
	  {
	    pfp = stdin;
	    file_pos = stdin_pos;
	  }
	else
	  {
	    size_t charsread;
	    int exclusive = TMPPATNAME_needs_removal ? 0 : O_EXCL;
	    TMPPATNAME_needs_removal = 1;
	    pfp = fdopen (create_file (TMPPATNAME,
				       O_RDWR | O_BINARY | exclusive,
				       (mode_t) 0),
			  "w+b");
	    if (!pfp)
	      pfatal ("Can't open stream for file %s", quotearg (TMPPATNAME));
	    for (st.st_size = 0;
		 (charsread = fread (buf, 1, bufsize, stdin)) != 0;
		 st.st_size += charsread)
	      if (fwrite (buf, 1, charsread, pfp) != charsread)
		write_fatal ();
	    if (ferror (stdin) || fclose (stdin) != 0)
	      read_fatal ();
	    if (fflush (pfp) != 0
		|| file_seek (pfp, (file_offset) 0, SEEK_SET) != 0)
	      write_fatal ();
	  }
      }
    else
      {
	pfp = fopen (filename, binary_transput ? "rb" : "r");
	if (!pfp)
	  pfatal ("Can't open patch file %s", quotearg (filename));
	if (fstat (fileno (pfp), &st) != 0)
	  pfatal ("fstat");
      }
    p_filesize = st.st_size;
    if (p_filesize != (file_offset) p_filesize)
      fatal ("patch file is too long");
    next_intuit_at (file_pos, (LINENUM) 1);
    set_hunkmax();
}

/* Make sure our dynamically realloced tables are malloced to begin with. */

void
set_hunkmax (void)
{
    if (!p_line)
	p_line = (char **) malloc (hunkmax * sizeof *p_line);
    if (!p_len)
	p_len = (size_t *) malloc (hunkmax * sizeof *p_len);
    if (!p_Char)
	p_Char = malloc (hunkmax * sizeof *p_Char);
}

/* Enlarge the arrays containing the current hunk of patch. */

static bool
grow_hunkmax (void)
{
    hunkmax *= 2;
    assert (p_line && p_len && p_Char);
    if ((p_line = (char **) realloc (p_line, hunkmax * sizeof (*p_line)))
	&& (p_len = (size_t *) realloc (p_len, hunkmax * sizeof (*p_len)))
	&& (p_Char = realloc (p_Char, hunkmax * sizeof (*p_Char))))
      return true;
    if (!using_plan_a)
      memory_fatal ();
    /* Don't free previous values of p_line etc.,
       since some broken implementations free them for us.
       Whatever is null will be allocated again from within plan_a (),
       of all places.  */
    return false;
}

/* True if the remainder of the patch file contains a diff of some sort. */

bool
there_is_another_patch (void)
{
    if (p_base != 0 && p_base >= p_filesize) {
	if (verbosity == VERBOSE)
	    say ("done\n");
	return false;
    }
    if (verbosity == VERBOSE)
	say ("Hmm...");
    diff_type = intuit_diff_type();
    if (diff_type == NO_DIFF) {
	if (verbosity == VERBOSE)
	  say (p_base
	       ? "  Ignoring the trailing garbage.\ndone\n"
	       : "  I can't seem to find a patch in there anywhere.\n");
	if (! p_base && p_filesize)
	  fatal ("Only garbage was found in the patch input.");
	return false;
    }
    if (skip_rest_of_patch)
      {
	Fseek (pfp, p_start, SEEK_SET);
	p_input_line = p_sline - 1;
	return true;
      }
    if (verbosity == VERBOSE)
	say ("  %sooks like %s to me...\n",
	    (p_base == 0 ? "L" : "The next patch l"),
	    diff_type == UNI_DIFF ? "a unified diff" :
	    diff_type == CONTEXT_DIFF ? "a context diff" :
	    diff_type == NEW_CONTEXT_DIFF ? "a new-style context diff" :
	    diff_type == NORMAL_DIFF ? "a normal diff" :
	    "an ed script" );

    if (verbosity != SILENT)
      {
	if (p_indent)
	  say ("(Patch is indented %d space%s.)\n", p_indent, p_indent==1?"":"s");
	if (p_strip_trailing_cr)
	  say ("(Stripping trailing CRs from patch.)\n");
	if (! inname)
	  {
	    char numbuf[LINENUM_LENGTH_BOUND + 1];
	    say ("can't find file to patch at input line %s\n",
		 format_linenum (numbuf, p_sline));
	    if (diff_type != ED_DIFF)
	      say (strippath == -1
		   ? "Perhaps you should have used the -p or --strip option?\n"
		   : "Perhaps you used the wrong -p or --strip option?\n");
	  }
      }

    skip_to(p_start,p_sline);
    while (!inname) {
	if (force | batch) {
	    say ("No file to patch.  Skipping patch.\n");
	    skip_rest_of_patch = true;
	    return true;
	}
	ask ("File to patch: ");
	inname = fetchname (buf, 0, (time_t *) 0);
	if (inname)
	  {
	    if (stat (inname, &instat) == 0)
	      {
		inerrno = 0;
		invc = -1;
	      }
	    else
	      {
		perror (inname);
		fflush (stderr);
		free (inname);
		inname = 0;
	      }
	  }
	if (!inname) {
	    ask ("Skip this patch? [y] ");
	    if (*buf != 'n') {
		if (verbosity != SILENT)
		    say ("Skipping patch.\n");
		skip_rest_of_patch = true;
		return true;
	    }
	}
    }
    return true;
}

/* Determine what kind of diff is in the remaining part of the patch file. */

static enum diff
intuit_diff_type (void)
{
    register file_offset this_line = 0;
    register file_offset first_command_line = -1;
    char first_ed_command_letter = 0;
    LINENUM fcl_line = 0; /* Pacify `gcc -W'.  */
    register bool this_is_a_command = false;
    register bool stars_this_line = false;
    enum nametype i;
    char *name[3];
    struct stat st[3];
    int stat_errno[3];
    int version_controlled[3];
    register enum diff retval;

    name[OLD] = name[NEW] = name[INDEX] = 0;
    version_controlled[OLD] = -1;
    version_controlled[NEW] = -1;
    version_controlled[INDEX] = -1;
    p_rfc934_nesting = 0;
    p_timestamp[OLD] = p_timestamp[NEW] = (time_t) -1;
    p_says_nonexistent[OLD] = p_says_nonexistent[NEW] = 0;
    Fseek (pfp, p_base, SEEK_SET);
    p_input_line = p_bline - 1;
    for (;;) {
	register char *s;
	register char *t;
	register file_offset previous_line = this_line;
	register bool last_line_was_command = this_is_a_command;
	register bool stars_last_line = stars_this_line;
	register int indent = 0;
	char ed_command_letter;
	bool strip_trailing_cr;
	size_t chars_read;

	this_line = file_tell (pfp);
	chars_read = pget_line (0, 0, false, false);
	if (chars_read == (size_t) -1)
	  memory_fatal ();
	if (! chars_read) {
	    if (first_ed_command_letter) {
					/* nothing but deletes!? */
		p_start = first_command_line;
		p_sline = fcl_line;
		retval = ED_DIFF;
		goto scan_exit;
	    }
	    else {
		p_start = this_line;
		p_sline = p_input_line;
		return NO_DIFF;
	    }
	}
	strip_trailing_cr = 2 <= chars_read && buf[chars_read - 2] == '\r';
	for (s = buf; *s == ' ' || *s == '\t' || *s == 'X'; s++) {
	    if (*s == '\t')
		indent = (indent + 8) & ~7;
	    else
		indent++;
	}
	for (t = s;  ISDIGIT (*t) || *t == ',';  t++)
	  continue;
	this_is_a_command = (ISDIGIT (*s) &&
	  (*t == 'd' || *t == 'c' || *t == 'a') );
	if (first_command_line < 0
	    && ((ed_command_letter = get_ed_command_letter (s))
		|| this_is_a_command)) {
	    first_command_line = this_line;
	    first_ed_command_letter = ed_command_letter;
	    fcl_line = p_input_line;
	    p_indent = indent;		/* assume this for now */
	    p_strip_trailing_cr = strip_trailing_cr;
	}
	if (!stars_last_line && strnEQ(s, "*** ", 4))
	    name[OLD] = fetchname (s+4, strippath, &p_timestamp[OLD]);
	else if (strnEQ(s, "+++ ", 4))
	    /* Swap with NEW below.  */
	    name[OLD] = fetchname (s+4, strippath, &p_timestamp[OLD]);
	else if (strnEQ(s, "Index:", 6))
	    name[INDEX] = fetchname (s+6, strippath, (time_t *) 0);
	else if (strnEQ(s, "Prereq:", 7)) {
	    for (t = s + 7;  ISSPACE ((unsigned char) *t);  t++)
	      continue;
	    revision = t;
	    for (t = revision;  *t;  t++)
	      if (ISSPACE ((unsigned char) *t))
		{
		  char const *u;
		  for (u = t + 1;  ISSPACE ((unsigned char) *u);  u++)
		    continue;
		  if (*u)
		    {
		      char numbuf[LINENUM_LENGTH_BOUND + 1];
		      say ("Prereq: with multiple words at line %s of patch\n",
			   format_linenum (numbuf, this_line));
		    }
		  break;
		}
	    if (t == revision)
		revision = 0;
	    else {
		char oldc = *t;
		*t = '\0';
		revision = savestr (revision);
		*t = oldc;
	    }
	} else
	  {
	    for (t = s;  t[0] == '-' && t[1] == ' ';  t += 2)
	      continue;
	    if (strnEQ(t, "--- ", 4))
	      {
		time_t timestamp = (time_t) -1;
		name[NEW] = fetchname (t+4, strippath, &timestamp);
		if (timestamp != (time_t) -1)
		  {
		    p_timestamp[NEW] = timestamp;
		    p_rfc934_nesting = (t - s) >> 1;
		  }
	      }
	  }
	if ((diff_type == NO_DIFF || diff_type == ED_DIFF) &&
	  first_command_line >= 0 &&
	  strEQ(s, ".\n") ) {
	    p_start = first_command_line;
	    p_sline = fcl_line;
	    retval = ED_DIFF;
	    goto scan_exit;
	}
	if ((diff_type == NO_DIFF || diff_type == UNI_DIFF)
	    && strnEQ(s, "@@ -", 4)) {

	    /* `name' and `p_timestamp' are backwards; swap them.  */
	    time_t ti = p_timestamp[OLD];
	    p_timestamp[OLD] = p_timestamp[NEW];
	    p_timestamp[NEW] = ti;
	    t = name[OLD];
	    name[OLD] = name[NEW];
	    name[NEW] = t;

	    s += 4;
	    if (s[0] == '0' && !ISDIGIT (s[1]))
	      p_says_nonexistent[OLD] = 1 + ! p_timestamp[OLD];
	    while (*s != ' ' && *s != '\n')
	      s++;
	    while (*s == ' ')
	      s++;
	    if (s[0] == '+' && s[1] == '0' && !ISDIGIT (s[2]))
	      p_says_nonexistent[NEW] = 1 + ! p_timestamp[NEW];
	    p_indent = indent;
	    p_start = this_line;
	    p_sline = p_input_line;
	    retval = UNI_DIFF;
	    if (! ((name[OLD] || ! p_timestamp[OLD])
		   && (name[NEW] || ! p_timestamp[NEW]))
		&& ! name[INDEX])
	      {
		char numbuf[LINENUM_LENGTH_BOUND + 1];
		say ("missing header for unified diff at line %s of patch\n",
		     format_linenum (numbuf, p_sline));
	      }
	    goto scan_exit;
	}
	stars_this_line = strnEQ(s, "********", 8);
	if ((diff_type == NO_DIFF
	     || diff_type == CONTEXT_DIFF
	     || diff_type == NEW_CONTEXT_DIFF)
	    && stars_last_line && strnEQ (s, "*** ", 4)) {
	    s += 4;
	    if (s[0] == '0' && !ISDIGIT (s[1]))
	      p_says_nonexistent[OLD] = 1 + ! p_timestamp[OLD];
	    /* if this is a new context diff the character just before */
	    /* the newline is a '*'. */
	    while (*s != '\n')
		s++;
	    p_indent = indent;
	    p_strip_trailing_cr = strip_trailing_cr;
	    p_start = previous_line;
	    p_sline = p_input_line - 1;
	    retval = (*(s-1) == '*' ? NEW_CONTEXT_DIFF : CONTEXT_DIFF);

	    {
	      /* Scan the first hunk to see whether the file contents
		 appear to have been deleted.  */
	      file_offset saved_p_base = p_base;
	      LINENUM saved_p_bline = p_bline;
	      Fseek (pfp, previous_line, SEEK_SET);
	      p_input_line -= 2;
	      if (another_hunk (retval, false)
		  && ! p_repl_lines && p_newfirst == 1)
		p_says_nonexistent[NEW] = 1 + ! p_timestamp[NEW];
	      next_intuit_at (saved_p_base, saved_p_bline);
	    }

	    if (! ((name[OLD] || ! p_timestamp[OLD])
		   && (name[NEW] || ! p_timestamp[NEW]))
		&& ! name[INDEX])
	      {
		char numbuf[LINENUM_LENGTH_BOUND + 1];
		say ("missing header for context diff at line %s of patch\n",
		     format_linenum (numbuf, p_sline));
	      }
	    goto scan_exit;
	}
	if ((diff_type == NO_DIFF || diff_type == NORMAL_DIFF) &&
	  last_line_was_command &&
	  (strnEQ(s, "< ", 2) || strnEQ(s, "> ", 2)) ) {
	    p_start = previous_line;
	    p_sline = p_input_line - 1;
	    p_indent = indent;
	    p_strip_trailing_cr = strip_trailing_cr;
	    retval = NORMAL_DIFF;
	    goto scan_exit;
	}
    }

  scan_exit:

    /* To intuit `inname', the name of the file to patch,
       use the algorithm specified by POSIX 1003.1-2001 XCU lines 25680-26599
       (with some modifications if posixly_correct is zero):

       - Take the old and new names from the context header if present,
	 and take the index name from the `Index:' line if present and
	 if either the old and new names are both absent
	 or posixly_correct is nonzero.
	 Consider the file names to be in the order (old, new, index).
       - If some named files exist, use the first one if posixly_correct
	 is nonzero, the best one otherwise.
       - If patch_get is nonzero, and no named files exist,
	 but an RCS or SCCS master file exists,
	 use the first named file with an RCS or SCCS master.
       - If no named files exist, no RCS or SCCS master was found,
	 some names are given, posixly_correct is zero,
	 and the patch appears to create a file, then use the best name
	 requiring the creation of the fewest directories.
       - Otherwise, report failure by setting `inname' to 0;
	 this causes our invoker to ask the user for a file name.  */

    i = NONE;

    if (!inname)
      {
	enum nametype i0 = NONE;

	if (! posixly_correct && (name[OLD] || name[NEW]) && name[INDEX])
	  {
	    free (name[INDEX]);
	    name[INDEX] = 0;
	  }

	for (i = OLD;  i <= INDEX;  i++)
	  if (name[i])
	    {
	      if (i0 != NONE && strcmp (name[i0], name[i]) == 0)
		{
		  /* It's the same name as before; reuse stat results.  */
		  stat_errno[i] = stat_errno[i0];
		  if (! stat_errno[i])
		    st[i] = st[i0];
		}
	      else if (stat (name[i], &st[i]) != 0)
		stat_errno[i] = errno;
	      else
		{
		  stat_errno[i] = 0;
		  if (posixly_correct)
		    break;
		}
	      i0 = i;
	    }

	if (! posixly_correct)
	  {
	    bool is_empty;

	    i = best_name (name, stat_errno);

	    if (i == NONE && patch_get)
	      {
		enum nametype nope = NONE;

		for (i = OLD;  i <= INDEX;  i++)
		  if (name[i])
		    {
		      char const *cs;
		      char *getbuf;
		      char *diffbuf;
		      bool readonly = (outfile
				       && strcmp (outfile, name[i]) != 0);

		      if (nope == NONE || strcmp (name[nope], name[i]) != 0)
			{
			  cs = (version_controller
			        (name[i], readonly, (struct stat *) 0,
				 &getbuf, &diffbuf));
			  version_controlled[i] = !! cs;
			  if (cs)
			    {
			      if (version_get (name[i], cs, false, readonly,
					       getbuf, &st[i]))
				stat_errno[i] = 0;
			      else
				version_controlled[i] = 0;

			      free (getbuf);
			      if (diffbuf)
				free (diffbuf);

			      if (! stat_errno[i])
				break;
			    }
			}

		      nope = i;
		    }
	      }

	    is_empty = i == NONE || st[i].st_size == 0;
	    if ((! is_empty) < p_says_nonexistent[reverse ^ is_empty])
	      {
		assert (i0 != NONE);
		reverse ^=
		  ok_to_reverse
		    ("The next patch%s would %s the file %s,\nwhich %s!",
		     reverse ? ", when reversed," : "",
		     (i == NONE ? "delete"
		      : st[i].st_size == 0 ? "empty out"
		      : "create"),
		     quotearg (name[i == NONE || st[i].st_size == 0 ? i0 : i]),
		     (i == NONE ? "does not exist"
		      : st[i].st_size == 0 ? "is already empty"
		      : "already exists"));
	      }

	    if (i == NONE && p_says_nonexistent[reverse])
	      {
		int newdirs[3];
		int newdirs_min = INT_MAX;
		int distance_from_minimum[3];

		for (i = OLD;  i <= INDEX;  i++)
		  if (name[i])
		    {
		      newdirs[i] = (prefix_components (name[i], false)
				    - prefix_components (name[i], true));
		      if (newdirs[i] < newdirs_min)
			newdirs_min = newdirs[i];
		    }

		for (i = OLD;  i <= INDEX;  i++)
		  if (name[i])
		    distance_from_minimum[i] = newdirs[i] - newdirs_min;

		i = best_name (name, distance_from_minimum);
	      }
	  }
      }

    if (i == NONE)
      inerrno = -1;
    else
      {
	inname = name[i];
	name[i] = 0;
	inerrno = stat_errno[i];
	invc = version_controlled[i];
	instat = st[i];
      }

    for (i = OLD;  i <= INDEX;  i++)
      if (name[i])
	free (name[i]);

    return retval;
}

/* Count the path name components in FILENAME's prefix.
   If CHECKDIRS is true, count only existing directories.  */
static int
prefix_components (char *filename, bool checkdirs)
{
  int count = 0;
  struct stat stat_buf;
  int stat_result;
  char *f = filename + FILESYSTEM_PREFIX_LEN (filename);

  if (*f)
    while (*++f)
      if (ISSLASH (f[0]) && ! ISSLASH (f[-1]))
	{
	  if (checkdirs)
	    {
	      *f = '\0';
	      stat_result = stat (filename, &stat_buf);
	      *f = '/';
	      if (! (stat_result == 0 && S_ISDIR (stat_buf.st_mode)))
		break;
	    }

	  count++;
	}

  return count;
}

/* Return the index of the best of NAME[OLD], NAME[NEW], and NAME[INDEX].
   Ignore null names, and ignore NAME[i] if IGNORE[i] is nonzero.
   Return NONE if all names are ignored.  */
static enum nametype
best_name (char *const *name, int const *ignore)
{
  enum nametype i;
  int components[3];
  int components_min = INT_MAX;
  size_t basename_len[3];
  size_t basename_len_min = SIZE_MAX;
  size_t len[3];
  size_t len_min = SIZE_MAX;

  for (i = OLD;  i <= INDEX;  i++)
    if (name[i] && !ignore[i])
      {
	/* Take the names with the fewest prefix components.  */
	components[i] = prefix_components (name[i], false);
	if (components_min < components[i])
	  continue;
	components_min = components[i];

	/* Of those, take the names with the shortest basename.  */
	basename_len[i] = strlen (base_name (name[i]));
	if (basename_len_min < basename_len[i])
	  continue;
	basename_len_min = basename_len[i];

	/* Of those, take the shortest names.  */
	len[i] = strlen (name[i]);
	if (len_min < len[i])
	  continue;
	len_min = len[i];
      }

  /* Of those, take the first name.  */
  for (i = OLD;  i <= INDEX;  i++)
    if (name[i] && !ignore[i]
	&& components[i] == components_min
	&& basename_len[i] == basename_len_min
	&& len[i] == len_min)
      break;

  return i;
}

/* Remember where this patch ends so we know where to start up again. */

static void
next_intuit_at (file_offset file_pos, LINENUM file_line)
{
    p_base = file_pos;
    p_bline = file_line;
}

/* Basically a verbose fseek() to the actual diff listing. */

static void
skip_to (file_offset file_pos, LINENUM file_line)
{
    register FILE *i = pfp;
    register FILE *o = stdout;
    register int c;

    assert(p_base <= file_pos);
    if ((verbosity == VERBOSE || !inname) && p_base < file_pos) {
	Fseek (i, p_base, SEEK_SET);
	say ("The text leading up to this was:\n--------------------------\n");

	while (file_tell (i) < file_pos)
	  {
	    putc ('|', o);
	    do
	      {
		if ((c = getc (i)) == EOF)
		  read_fatal ();
		putc (c, o);
	      }
	    while (c != '\n');
	  }

	say ("--------------------------\n");
    }
    else
	Fseek (i, file_pos, SEEK_SET);
    p_input_line = file_line - 1;
}

/* Make this a function for better debugging.  */
static void
malformed (void)
{
    char numbuf[LINENUM_LENGTH_BOUND + 1];
    fatal ("malformed patch at line %s: %s",
	   format_linenum (numbuf, p_input_line), buf);
		/* about as informative as "Syntax error" in C */
}

/* Parse a line number from a string.
   Return the address of the first char after the number.  */
static char *
scan_linenum (char *s0, LINENUM *linenum)
{
  char *s;
  LINENUM n = 0;
  bool overflow = false;
  char numbuf[LINENUM_LENGTH_BOUND + 1];

  for (s = s0;  ISDIGIT (*s);  s++)
    {
      LINENUM new_n = 10 * n + (*s - '0');
      overflow |= new_n / 10 != n;
      n = new_n;
    }

  if (s == s0)
    fatal ("missing line number at line %s: %s",
	   format_linenum (numbuf, p_input_line), buf);

  if (overflow)
    fatal ("line number %.*s is too large at line %s: %s",
	   (int) (s - s0), s0, format_linenum (numbuf, p_input_line), buf);

  *linenum = n;
  return s;
}

/* 1 if there is more of the current diff listing to process;
   0 if not; -1 if ran out of memory. */

int
another_hunk (enum diff difftype, bool rev)
{
    register char *s;
    register LINENUM context = 0;
    register size_t chars_read;
    char numbuf0[LINENUM_LENGTH_BOUND + 1];
    char numbuf1[LINENUM_LENGTH_BOUND + 1];
    char numbuf2[LINENUM_LENGTH_BOUND + 1];
    char numbuf3[LINENUM_LENGTH_BOUND + 1];

    while (p_end >= 0) {
	if (p_end == p_efake)
	    p_end = p_bfake;		/* don't free twice */
	else
	    free(p_line[p_end]);
	p_end--;
    }
    assert(p_end == -1);
    p_efake = -1;

    p_max = hunkmax;			/* gets reduced when --- found */
    if (difftype == CONTEXT_DIFF || difftype == NEW_CONTEXT_DIFF) {
	file_offset line_beginning = file_tell (pfp);
					/* file pos of the current line */
	LINENUM repl_beginning = 0;	/* index of --- line */
	register LINENUM fillcnt = 0;	/* #lines of missing ptrn or repl */
	register LINENUM fillsrc;	/* index of first line to copy */
	register LINENUM filldst;	/* index of first missing line */
	bool ptrn_spaces_eaten = false;	/* ptrn was slightly misformed */
	bool some_context = false;	/* (perhaps internal) context seen */
	register bool repl_could_be_missing = true;
	bool ptrn_missing = false;	/* The pattern was missing.  */
	bool repl_missing = false;	/* Likewise for replacement.  */
	file_offset repl_backtrack_position = 0;
					/* file pos of first repl line */
	LINENUM repl_patch_line;	/* input line number for same */
	LINENUM repl_context;		/* context for same */
	LINENUM ptrn_prefix_context = -1; /* lines in pattern prefix context */
	LINENUM ptrn_suffix_context = -1; /* lines in pattern suffix context */
	LINENUM repl_prefix_context = -1; /* lines in replac. prefix context */
	LINENUM ptrn_copiable = 0;	/* # of copiable lines in ptrn */
	LINENUM repl_copiable = 0;	/* Likewise for replacement.  */

	/* Pacify `gcc -Wall'.  */
	fillsrc = filldst = repl_patch_line = repl_context = 0;

	chars_read = get_line ();
	if (chars_read == (size_t) -1
	    || chars_read <= 8
	    || strncmp (buf, "********", 8) != 0) {
	    next_intuit_at(line_beginning,p_input_line);
	    return chars_read == (size_t) -1 ? -1 : 0;
	}
	p_hunk_beg = p_input_line + 1;
	while (p_end < p_max) {
	    chars_read = get_line ();
	    if (chars_read == (size_t) -1)
	      return -1;
	    if (!chars_read) {
		if (repl_beginning && repl_could_be_missing) {
		    repl_missing = true;
		    goto hunk_done;
		}
		if (p_max - p_end < 4) {
		    strcpy (buf, "  \n");  /* assume blank lines got chopped */
		    chars_read = 3;
		} else {
		    fatal ("unexpected end of file in patch");
		}
	    }
	    p_end++;
	    if (p_end == hunkmax)
	      fatal ("unterminated hunk starting at line %s; giving up at line %s: %s",
		     format_linenum (numbuf0, pch_hunk_beg ()),
		     format_linenum (numbuf1, p_input_line), buf);
	    assert(p_end < hunkmax);
	    p_Char[p_end] = *buf;
	    p_len[p_end] = 0;
	    p_line[p_end] = 0;
	    switch (*buf) {
	    case '*':
		if (strnEQ(buf, "********", 8)) {
		    if (repl_beginning && repl_could_be_missing) {
			repl_missing = true;
			goto hunk_done;
		    }
		    else
		      fatal ("unexpected end of hunk at line %s",
			     format_linenum (numbuf0, p_input_line));
		}
		if (p_end != 0) {
		    if (repl_beginning && repl_could_be_missing) {
			repl_missing = true;
			goto hunk_done;
		    }
		    fatal ("unexpected `***' at line %s: %s",
			   format_linenum (numbuf0, p_input_line), buf);
		}
		context = 0;
		p_len[p_end] = strlen (buf);
		if (! (p_line[p_end] = savestr (buf))) {
		    p_end--;
		    return -1;
		}
		for (s = buf;  *s && !ISDIGIT (*s);  s++)
		  continue;
		if (strnEQ(s,"0,0",3))
		    remove_prefix (s, 2);
		s = scan_linenum (s, &p_first);
		if (*s == ',') {
		    while (*s && !ISDIGIT (*s))
		      s++;
		    scan_linenum (s, &p_ptrn_lines);
		    p_ptrn_lines += 1 - p_first;
		}
		else if (p_first)
		    p_ptrn_lines = 1;
		else {
		    p_ptrn_lines = 0;
		    p_first = 1;
		}
		p_max = p_ptrn_lines + 6;	/* we need this much at least */
		while (p_max >= hunkmax)
		    if (! grow_hunkmax ())
			return -1;
		p_max = hunkmax;
		break;
	    case '-':
		if (buf[1] != '-')
		  goto change_line;
		if (ptrn_prefix_context == -1)
		  ptrn_prefix_context = context;
		ptrn_suffix_context = context;
		if (repl_beginning
		    || (p_end
			!= p_ptrn_lines + 1 + (p_Char[p_end - 1] == '\n')))
		  {
		    if (p_end == 1)
		      {
			/* `Old' lines were omitted.  Set up to fill
			   them in from `new' context lines.  */
			ptrn_missing = true;
			p_end = p_ptrn_lines + 1;
			ptrn_prefix_context = ptrn_suffix_context = -1;
			fillsrc = p_end + 1;
			filldst = 1;
			fillcnt = p_ptrn_lines;
		      }
		    else if (! repl_beginning)
		      fatal ("%s `---' at line %s; check line numbers at line %s",
			     (p_end <= p_ptrn_lines
			      ? "Premature"
			      : "Overdue"),
			     format_linenum (numbuf0, p_input_line),
			     format_linenum (numbuf1, p_hunk_beg));
		    else if (! repl_could_be_missing)
		      fatal ("duplicate `---' at line %s; check line numbers at line %s",
			     format_linenum (numbuf0, p_input_line),
			     format_linenum (numbuf1,
					     p_hunk_beg + repl_beginning));
		    else
		      {
			repl_missing = true;
			goto hunk_done;
		      }
		  }
		repl_beginning = p_end;
		repl_backtrack_position = file_tell (pfp);
		repl_patch_line = p_input_line;
		repl_context = context;
		p_len[p_end] = strlen (buf);
		if (! (p_line[p_end] = savestr (buf)))
		  {
		    p_end--;
		    return -1;
		  }
		p_Char[p_end] = '=';
		for (s = buf;  *s && ! ISDIGIT (*s);  s++)
		  continue;
		s = scan_linenum (s, &p_newfirst);
		if (*s == ',')
		  {
		    do
		      {
			if (!*++s)
			  malformed ();
		      }
		    while (! ISDIGIT (*s));
		    scan_linenum (s, &p_repl_lines);
		    p_repl_lines += 1 - p_newfirst;
		  }
		else if (p_newfirst)
		  p_repl_lines = 1;
		else
		  {
		    p_repl_lines = 0;
		    p_newfirst = 1;
		  }
		p_max = p_repl_lines + p_end;
		while (p_max >= hunkmax)
		  if (! grow_hunkmax ())
		    return -1;
		if (p_repl_lines != ptrn_copiable
		    && (p_prefix_context != 0
			|| context != 0
			|| p_repl_lines != 1))
		  repl_could_be_missing = false;
		context = 0;
		break;
	    case '+':  case '!':
		repl_could_be_missing = false;
	      change_line:
		s = buf + 1;
		chars_read--;
		if (*s == '\n' && canonicalize) {
		    strcpy (s, " \n");
		    chars_read = 2;
		}
		if (*s == ' ' || *s == '\t') {
		    s++;
		    chars_read--;
		} else if (repl_beginning && repl_could_be_missing) {
		    repl_missing = true;
		    goto hunk_done;
		}
		if (! repl_beginning)
		  {
		    if (ptrn_prefix_context == -1)
		      ptrn_prefix_context = context;
		  }
		else
		  {
		    if (repl_prefix_context == -1)
		      repl_prefix_context = context;
		  }
		chars_read -=
		  (1 < chars_read
		   && p_end == (repl_beginning ? p_max : p_ptrn_lines)
		   && incomplete_line ());
		p_len[p_end] = chars_read;
		if (! (p_line[p_end] = savebuf (s, chars_read))) {
		    p_end--;
		    return -1;
		}
		context = 0;
		break;
	    case '\t': case '\n':	/* assume spaces got eaten */
		s = buf;
		if (*buf == '\t') {
		    s++;
		    chars_read--;
		}
		if (repl_beginning && repl_could_be_missing &&
		    (!ptrn_spaces_eaten || difftype == NEW_CONTEXT_DIFF) ) {
		    repl_missing = true;
		    goto hunk_done;
		}
		chars_read -=
		  (1 < chars_read
		   && p_end == (repl_beginning ? p_max : p_ptrn_lines)
		   && incomplete_line ());
		p_len[p_end] = chars_read;
		if (! (p_line[p_end] = savebuf (buf, chars_read))) {
		    p_end--;
		    return -1;
		}
		if (p_end != p_ptrn_lines + 1) {
		    ptrn_spaces_eaten |= (repl_beginning != 0);
		    some_context = true;
		    context++;
		    if (repl_beginning)
			repl_copiable++;
		    else
			ptrn_copiable++;
		    p_Char[p_end] = ' ';
		}
		break;
	    case ' ':
		s = buf + 1;
		chars_read--;
		if (*s == '\n' && canonicalize) {
		    strcpy (s, "\n");
		    chars_read = 2;
		}
		if (*s == ' ' || *s == '\t') {
		    s++;
		    chars_read--;
		} else if (repl_beginning && repl_could_be_missing) {
		    repl_missing = true;
		    goto hunk_done;
		}
		some_context = true;
		context++;
		if (repl_beginning)
		    repl_copiable++;
		else
		    ptrn_copiable++;
		chars_read -=
		  (1 < chars_read
		   && p_end == (repl_beginning ? p_max : p_ptrn_lines)
		   && incomplete_line ());
		p_len[p_end] = chars_read;
		if (! (p_line[p_end] = savebuf (buf + 2, chars_read))) {
		    p_end--;
		    return -1;
		}
		break;
	    default:
		if (repl_beginning && repl_could_be_missing) {
		    repl_missing = true;
		    goto hunk_done;
		}
		malformed ();
	    }
	}

    hunk_done:
	if (p_end >=0 && !repl_beginning)
	  fatal ("no `---' found in patch at line %s",
		 format_linenum (numbuf0, pch_hunk_beg ()));

	if (repl_missing) {

	    /* reset state back to just after --- */
	    p_input_line = repl_patch_line;
	    context = repl_context;
	    for (p_end--; p_end > repl_beginning; p_end--)
		free(p_line[p_end]);
	    Fseek (pfp, repl_backtrack_position, SEEK_SET);

	    /* redundant 'new' context lines were omitted - set */
	    /* up to fill them in from the old file context */
	    fillsrc = 1;
	    filldst = repl_beginning+1;
	    fillcnt = p_repl_lines;
	    p_end = p_max;
	}
	else if (! ptrn_missing && ptrn_copiable != repl_copiable)
	  fatal ("context mangled in hunk at line %s",
		 format_linenum (numbuf0, p_hunk_beg));
	else if (!some_context && fillcnt == 1) {
	    /* the first hunk was a null hunk with no context */
	    /* and we were expecting one line -- fix it up. */
	    while (filldst < p_end) {
		p_line[filldst] = p_line[filldst+1];
		p_Char[filldst] = p_Char[filldst+1];
		p_len[filldst] = p_len[filldst+1];
		filldst++;
	    }
#if 0
	    repl_beginning--;		/* this doesn't need to be fixed */
#endif
	    p_end--;
	    p_first++;			/* do append rather than insert */
	    fillcnt = 0;
	    p_ptrn_lines = 0;
	}

	p_prefix_context = ((repl_prefix_context == -1
			     || (ptrn_prefix_context != -1
				 && ptrn_prefix_context < repl_prefix_context))
			    ? ptrn_prefix_context : repl_prefix_context);
	p_suffix_context = ((ptrn_suffix_context != -1
			     && ptrn_suffix_context < context)
			    ? ptrn_suffix_context : context);
	assert (p_prefix_context != -1 && p_suffix_context != -1);

	if (difftype == CONTEXT_DIFF
	    && (fillcnt
		|| (p_first > 1
		    && p_prefix_context + p_suffix_context < ptrn_copiable))) {
	    if (verbosity == VERBOSE)
		say ("%s\n%s\n%s\n",
"(Fascinating -- this is really a new-style context diff but without",
"the telltale extra asterisks on the *** line that usually indicate",
"the new style...)");
	    diff_type = difftype = NEW_CONTEXT_DIFF;
	}

	/* if there were omitted context lines, fill them in now */
	if (fillcnt) {
	    p_bfake = filldst;		/* remember where not to free() */
	    p_efake = filldst + fillcnt - 1;
	    while (fillcnt-- > 0) {
		while (fillsrc <= p_end && fillsrc != repl_beginning
		       && p_Char[fillsrc] != ' ')
		    fillsrc++;
		if (p_end < fillsrc || fillsrc == repl_beginning)
		  {
		    fatal ("replacement text or line numbers mangled in hunk at line %s",
			   format_linenum (numbuf0, p_hunk_beg));
		  }
		p_line[filldst] = p_line[fillsrc];
		p_Char[filldst] = p_Char[fillsrc];
		p_len[filldst] = p_len[fillsrc];
		fillsrc++; filldst++;
	    }
	    while (fillsrc <= p_end && fillsrc != repl_beginning)
	      {
		if (p_Char[fillsrc] == ' ')
		  fatal ("replacement text or line numbers mangled in hunk at line %s",
			 format_linenum (numbuf0, p_hunk_beg));
		fillsrc++;
	      }
	    if (debug & 64)
	      printf ("fillsrc %s, filldst %s, rb %s, e+1 %s\n",
		      format_linenum (numbuf0, fillsrc),
		      format_linenum (numbuf1, filldst),
		      format_linenum (numbuf2, repl_beginning),
		      format_linenum (numbuf3, p_end + 1));
	    assert(fillsrc==p_end+1 || fillsrc==repl_beginning);
	    assert(filldst==p_end+1 || filldst==repl_beginning);
	}
    }
    else if (difftype == UNI_DIFF) {
	file_offset line_beginning = file_tell (pfp);
					/* file pos of the current line */
	register LINENUM fillsrc;	/* index of old lines */
	register LINENUM filldst;	/* index of new lines */
	char ch = '\0';

	chars_read = get_line ();
	if (chars_read == (size_t) -1
	    || chars_read <= 4
	    || strncmp (buf, "@@ -", 4) != 0) {
	    next_intuit_at(line_beginning,p_input_line);
	    return chars_read == (size_t) -1 ? -1 : 0;
	}
	s = scan_linenum (buf + 4, &p_first);
	if (*s == ',')
	    s = scan_linenum (s + 1, &p_ptrn_lines);
	else
	    p_ptrn_lines = 1;
	if (*s == ' ') s++;
	if (*s != '+')
	    malformed ();
	s = scan_linenum (s + 1, &p_newfirst);
	if (*s == ',')
	    s = scan_linenum (s + 1, &p_repl_lines);
	else
	    p_repl_lines = 1;
	if (*s == ' ') s++;
	if (*s != '@')
	    malformed ();
	if (!p_ptrn_lines)
	    p_first++;			/* do append rather than insert */
	if (!p_repl_lines)
	    p_newfirst++;
	p_max = p_ptrn_lines + p_repl_lines + 1;
	while (p_max >= hunkmax)
	    if (! grow_hunkmax ())
		return -1;
	fillsrc = 1;
	filldst = fillsrc + p_ptrn_lines;
	p_end = filldst + p_repl_lines;
	sprintf (buf, "*** %s,%s ****\n",
		 format_linenum (numbuf0, p_first),
		 format_linenum (numbuf1, p_first + p_ptrn_lines - 1));
	p_len[0] = strlen (buf);
	if (! (p_line[0] = savestr (buf))) {
	    p_end = -1;
	    return -1;
	}
	p_Char[0] = '*';
	sprintf (buf, "--- %s,%s ----\n",
		 format_linenum (numbuf0, p_newfirst),
		 format_linenum (numbuf1, p_newfirst + p_repl_lines - 1));
	p_len[filldst] = strlen (buf);
	if (! (p_line[filldst] = savestr (buf))) {
	    p_end = 0;
	    return -1;
	}
	p_Char[filldst++] = '=';
	p_prefix_context = -1;
	p_hunk_beg = p_input_line + 1;
	while (fillsrc <= p_ptrn_lines || filldst <= p_end) {
	    chars_read = get_line ();
	    if (!chars_read) {
		if (p_max - filldst < 3) {
		    strcpy (buf, " \n");  /* assume blank lines got chopped */
		    chars_read = 2;
		} else {
		    fatal ("unexpected end of file in patch");
		}
	    }
	    if (chars_read == (size_t) -1)
		s = 0;
	    else if (*buf == '\t' || *buf == '\n') {
		ch = ' ';		/* assume the space got eaten */
		s = savebuf (buf, chars_read);
	    }
	    else {
		ch = *buf;
		s = savebuf (buf+1, --chars_read);
	    }
	    if (!s) {
		while (--filldst > p_ptrn_lines)
		    free(p_line[filldst]);
		p_end = fillsrc-1;
		return -1;
	    }
	    switch (ch) {
	    case '-':
		if (fillsrc > p_ptrn_lines) {
		    free(s);
		    p_end = filldst-1;
		    malformed ();
		}
		chars_read -= fillsrc == p_ptrn_lines && incomplete_line ();
		p_Char[fillsrc] = ch;
		p_line[fillsrc] = s;
		p_len[fillsrc++] = chars_read;
		break;
	    case '=':
		ch = ' ';
		/* FALL THROUGH */
	    case ' ':
		if (fillsrc > p_ptrn_lines) {
		    free(s);
		    while (--filldst > p_ptrn_lines)
			free(p_line[filldst]);
		    p_end = fillsrc-1;
		    malformed ();
		}
		context++;
		chars_read -= fillsrc == p_ptrn_lines && incomplete_line ();
		p_Char[fillsrc] = ch;
		p_line[fillsrc] = s;
		p_len[fillsrc++] = chars_read;
		s = savebuf (s, chars_read);
		if (!s) {
		    while (--filldst > p_ptrn_lines)
			free(p_line[filldst]);
		    p_end = fillsrc-1;
		    return -1;
		}
		/* FALL THROUGH */
	    case '+':
		if (filldst > p_end) {
		    free(s);
		    while (--filldst > p_ptrn_lines)
			free(p_line[filldst]);
		    p_end = fillsrc-1;
		    malformed ();
		}
		chars_read -= filldst == p_end && incomplete_line ();
		p_Char[filldst] = ch;
		p_line[filldst] = s;
		p_len[filldst++] = chars_read;
		break;
	    default:
		p_end = filldst;
		malformed ();
	    }
	    if (ch != ' ') {
		if (p_prefix_context == -1)
		    p_prefix_context = context;
		context = 0;
	    }
	}/* while */
	if (p_prefix_context == -1)
	  malformed ();
	p_suffix_context = context;
    }
    else {				/* normal diff--fake it up */
	char hunk_type;
	register int i;
	LINENUM min, max;
	file_offset line_beginning = file_tell (pfp);

	p_prefix_context = p_suffix_context = 0;
	chars_read = get_line ();
	if (chars_read == (size_t) -1 || !chars_read || !ISDIGIT (*buf)) {
	    next_intuit_at(line_beginning,p_input_line);
	    return chars_read == (size_t) -1 ? -1 : 0;
	}
	s = scan_linenum (buf, &p_first);
	if (*s == ',') {
	    s = scan_linenum (s + 1, &p_ptrn_lines);
	    p_ptrn_lines += 1 - p_first;
	}
	else
	    p_ptrn_lines = (*s != 'a');
	hunk_type = *s;
	if (hunk_type == 'a')
	    p_first++;			/* do append rather than insert */
	s = scan_linenum (s + 1, &min);
	if (*s == ',')
	    scan_linenum (s + 1, &max);
	else
	    max = min;
	if (hunk_type == 'd')
	    min++;
	p_end = p_ptrn_lines + 1 + max - min + 1;
	while (p_end >= hunkmax)
	  if (! grow_hunkmax ())
	    {
	      p_end = -1;
	      return -1;
	    }
	p_newfirst = min;
	p_repl_lines = max - min + 1;
	sprintf (buf, "*** %s,%s\n",
		 format_linenum (numbuf0, p_first),
		 format_linenum (numbuf1, p_first + p_ptrn_lines - 1));
	p_len[0] = strlen (buf);
	if (! (p_line[0] = savestr (buf))) {
	    p_end = -1;
	    return -1;
	}
	p_Char[0] = '*';
	for (i=1; i<=p_ptrn_lines; i++) {
	    chars_read = get_line ();
	    if (chars_read == (size_t) -1)
	      {
		p_end = i - 1;
		return -1;
	      }
	    if (!chars_read)
	      fatal ("unexpected end of file in patch at line %s",
		     format_linenum (numbuf0, p_input_line));
	    if (buf[0] != '<' || (buf[1] != ' ' && buf[1] != '\t'))
	      fatal ("`<' expected at line %s of patch",
		     format_linenum (numbuf0, p_input_line));
	    chars_read -= 2 + (i == p_ptrn_lines && incomplete_line ());
	    p_len[i] = chars_read;
	    if (! (p_line[i] = savebuf (buf + 2, chars_read))) {
		p_end = i-1;
		return -1;
	    }
	    p_Char[i] = '-';
	}
	if (hunk_type == 'c') {
	    chars_read = get_line ();
	    if (chars_read == (size_t) -1)
	      {
		p_end = i - 1;
		return -1;
	      }
	    if (! chars_read)
	      fatal ("unexpected end of file in patch at line %s",
		     format_linenum (numbuf0, p_input_line));
	    if (*buf != '-')
	      fatal ("`---' expected at line %s of patch",
		     format_linenum (numbuf0, p_input_line));
	}
	sprintf (buf, "--- %s,%s\n",
		 format_linenum (numbuf0, min),
		 format_linenum (numbuf1, max));
	p_len[i] = strlen (buf);
	if (! (p_line[i] = savestr (buf))) {
	    p_end = i-1;
	    return -1;
	}
	p_Char[i] = '=';
	for (i++; i<=p_end; i++) {
	    chars_read = get_line ();
	    if (chars_read == (size_t) -1)
	      {
		p_end = i - 1;
		return -1;
	      }
	    if (!chars_read)
	      fatal ("unexpected end of file in patch at line %s",
		     format_linenum (numbuf0, p_input_line));
	    if (buf[0] != '>' || (buf[1] != ' ' && buf[1] != '\t'))
	      fatal ("`>' expected at line %s of patch",
		     format_linenum (numbuf0, p_input_line));
	    chars_read -= 2 + (i == p_end && incomplete_line ());
	    p_len[i] = chars_read;
	    if (! (p_line[i] = savebuf (buf + 2, chars_read))) {
		p_end = i-1;
		return -1;
	    }
	    p_Char[i] = '+';
	}
    }
    if (rev)				/* backwards patch? */
	if (!pch_swap())
	    say ("Not enough memory to swap next hunk!\n");
    if (debug & 2) {
	LINENUM i;
	char special;

	for (i=0; i <= p_end; i++) {
	    if (i == p_ptrn_lines)
		special = '^';
	    else
		special = ' ';
	    fprintf (stderr, "%s %c %c ", format_linenum (numbuf0, i),
		     p_Char[i], special);
	    pch_write_line (i, stderr);
	    fflush (stderr);
	}
    }
    if (p_end+1 < hunkmax)	/* paranoia reigns supreme... */
	p_Char[p_end+1] = '^';  /* add a stopper for apply_hunk */
    return 1;
}

static size_t
get_line (void)
{
   return pget_line (p_indent, p_rfc934_nesting, p_strip_trailing_cr,
		     p_pass_comments_through);
}

/* Input a line from the patch file, worrying about indentation.
   Strip up to INDENT characters' worth of leading indentation.
   Then remove up to RFC934_NESTING instances of leading "- ".
   If STRIP_TRAILING_CR is true, remove any trailing carriage-return.
   Unless PASS_COMMENTS_THROUGH is true, ignore any resulting lines
   that begin with '#'; they're comments.
   Ignore any partial lines at end of input, but warn about them.
   Succeed if a line was read; it is terminated by "\n\0" for convenience.
   Return the number of characters read, including '\n' but not '\0'.
   Return -1 if we ran out of memory.  */

static size_t
pget_line (int indent, int rfc934_nesting, bool strip_trailing_cr,
	   bool pass_comments_through)
{
  register FILE *fp = pfp;
  register int c;
  register int i;
  register char *b;
  register size_t s;

  do
    {
      i = 0;
      for (;;)
	{
	  c = getc (fp);
	  if (c == EOF)
	    {
	      if (ferror (fp))
		read_fatal ();
	      return 0;
	    }
	  if (indent <= i)
	    break;
	  if (c == ' ' || c == 'X')
	    i++;
	  else if (c == '\t')
	    i = (i + 8) & ~7;
	  else
	    break;
	}

      i = 0;
      b = buf;

      while (c == '-' && 0 <= --rfc934_nesting)
	{
	  c = getc (fp);
	  if (c == EOF)
	    goto patch_ends_in_middle_of_line;
	  if (c != ' ')
	    {
	      i = 1;
	      b[0] = '-';
	      break;
	    }
	  c = getc (fp);
	  if (c == EOF)
	    goto patch_ends_in_middle_of_line;
	}

      s = bufsize;

      for (;;)
	{
	  if (i == s - 1)
	    {
	      s *= 2;
	      b = realloc (b, s);
	      if (!b)
		{
		  if (!using_plan_a)
		    memory_fatal ();
		  return (size_t) -1;
		}
	      buf = b;
	      bufsize = s;
	    }
	  b[i++] = c;
	  if (c == '\n')
	    break;
	  c = getc (fp);
	  if (c == EOF)
	    goto patch_ends_in_middle_of_line;
	}

      p_input_line++;
    }
  while (*b == '#' && !pass_comments_through);

  if (strip_trailing_cr && 2 <= i && b[i - 2] == '\r')
    b[i-- - 2] = '\n';
  b[i] = '\0';
  return i;

 patch_ends_in_middle_of_line:
  if (ferror (fp))
    read_fatal ();
  say ("patch unexpectedly ends in middle of line\n");
  return 0;
}

static bool
incomplete_line (void)
{
  register FILE *fp = pfp;
  register int c;
  register file_offset line_beginning = file_tell (fp);

  if (getc (fp) == '\\')
    {
      while ((c = getc (fp)) != '\n'  &&  c != EOF)
	continue;
      return true;
    }
  else
    {
      /* We don't trust ungetc.  */
      Fseek (pfp, line_beginning, SEEK_SET);
      return false;
    }
}

/* Reverse the old and new portions of the current hunk. */

bool
pch_swap (void)
{
    char **tp_line;		/* the text of the hunk */
    size_t *tp_len;		/* length of each line */
    char *tp_char;		/* +, -, and ! */
    register LINENUM i;
    register LINENUM n;
    bool blankline = false;
    register char *s;

    i = p_first;
    p_first = p_newfirst;
    p_newfirst = i;

    /* make a scratch copy */

    tp_line = p_line;
    tp_len = p_len;
    tp_char = p_Char;
    p_line = 0;	/* force set_hunkmax to allocate again */
    p_len = 0;
    p_Char = 0;
    set_hunkmax();
    if (!p_line || !p_len || !p_Char) {
	if (p_line)
	  free (p_line);
	p_line = tp_line;
	if (p_len)
	  free (p_len);
	p_len = tp_len;
	if (p_Char)
	  free (p_Char);
	p_Char = tp_char;
	return false;		/* not enough memory to swap hunk! */
    }

    /* now turn the new into the old */

    i = p_ptrn_lines + 1;
    if (tp_char[i] == '\n') {		/* account for possible blank line */
	blankline = true;
	i++;
    }
    if (p_efake >= 0) {			/* fix non-freeable ptr range */
	if (p_efake <= i)
	    n = p_end - i + 1;
	else
	    n = -i;
	p_efake += n;
	p_bfake += n;
    }
    for (n=0; i <= p_end; i++,n++) {
	p_line[n] = tp_line[i];
	p_Char[n] = tp_char[i];
	if (p_Char[n] == '+')
	    p_Char[n] = '-';
	p_len[n] = tp_len[i];
    }
    if (blankline) {
	i = p_ptrn_lines + 1;
	p_line[n] = tp_line[i];
	p_Char[n] = tp_char[i];
	p_len[n] = tp_len[i];
	n++;
    }
    assert(p_Char[0] == '=');
    p_Char[0] = '*';
    for (s=p_line[0]; *s; s++)
	if (*s == '-')
	    *s = '*';

    /* now turn the old into the new */

    assert(tp_char[0] == '*');
    tp_char[0] = '=';
    for (s=tp_line[0]; *s; s++)
	if (*s == '*')
	    *s = '-';
    for (i=0; n <= p_end; i++,n++) {
	p_line[n] = tp_line[i];
	p_Char[n] = tp_char[i];
	if (p_Char[n] == '-')
	    p_Char[n] = '+';
	p_len[n] = tp_len[i];
    }
    assert(i == p_ptrn_lines + 1);
    i = p_ptrn_lines;
    p_ptrn_lines = p_repl_lines;
    p_repl_lines = i;
    if (tp_line)
      free (tp_line);
    if (tp_len)
      free (tp_len);
    if (tp_char)
      free (tp_char);
    return true;
}

/* Return whether file WHICH (false = old, true = new) appears to nonexistent.
   Return 1 for empty, 2 for nonexistent.  */

int
pch_says_nonexistent (bool which)
{
  return p_says_nonexistent[which];
}

/* Return timestamp of patch header for file WHICH (false = old, true = new),
   or -1 if there was no timestamp or an error in the timestamp.  */

time_t
pch_timestamp (bool which)
{
  return p_timestamp[which];
}

/* Return the specified line position in the old file of the old context. */

LINENUM
pch_first (void)
{
    return p_first;
}

/* Return the number of lines of old context. */

LINENUM
pch_ptrn_lines (void)
{
    return p_ptrn_lines;
}

/* Return the probable line position in the new file of the first line. */

LINENUM
pch_newfirst (void)
{
    return p_newfirst;
}

/* Return the number of lines in the replacement text including context. */

LINENUM
pch_repl_lines (void)
{
    return p_repl_lines;
}

/* Return the number of lines in the whole hunk. */

LINENUM
pch_end (void)
{
    return p_end;
}

/* Return the number of context lines before the first changed line. */

LINENUM
pch_prefix_context (void)
{
    return p_prefix_context;
}

/* Return the number of context lines after the last changed line. */

LINENUM
pch_suffix_context (void)
{
    return p_suffix_context;
}

/* Return the length of a particular patch line. */

size_t
pch_line_len (LINENUM line)
{
    return p_len[line];
}

/* Return the control character (+, -, *, !, etc) for a patch line. */

char
pch_char (LINENUM line)
{
    return p_Char[line];
}

/* Return a pointer to a particular patch line. */

char *
pfetch (LINENUM line)
{
    return p_line[line];
}

/* Output a patch line.  */

bool
pch_write_line (LINENUM line, FILE *file)
{
  bool after_newline = p_line[line][p_len[line] - 1] == '\n';
  if (! fwrite (p_line[line], sizeof (*p_line[line]), p_len[line], file))
    write_fatal ();
  return after_newline;
}

/* Return where in the patch file this hunk began, for error messages. */

LINENUM
pch_hunk_beg (void)
{
    return p_hunk_beg;
}

/* Is the newline-terminated line a valid `ed' command for patch
   input?  If so, return the command character; if not, return 0.
   This accepts accepts just a subset of the valid commands, but it's
   good enough in practice.  */

static char
get_ed_command_letter (char const *line)
{
  char const *p = line;
  char letter;
  bool pair = false;
  if (! ISDIGIT (*p))
    return 0;
  while (ISDIGIT (*++p))
    continue;
  if (*p == ',')
    {
      if (! ISDIGIT (*++p))
	return 0;
      while (ISDIGIT (*++p))
	continue;
      pair = true;
    }
  letter = *p++;

  switch (letter)
    {
    case 'a':
    case 'i':
      if (pair)
	return 0;
      break;

    case 'c':
    case 'd':
      break;

    case 's':
      if (strncmp (p, "/.//", 4) != 0)
	return 0;
      p += 4;
      break;

    default:
      return 0;
    }

  while (*p == ' ' || *p == '\t')
    p++;
  if (*p == '\n')
    return letter;
  return 0;
}

/* Apply an ed script by feeding ed itself. */

void
do_ed_script (FILE *ofp)
{
    static char const ed_program[] = ed_PROGRAM;

    register file_offset beginning_of_this_line;
    register FILE *pipefp = 0;
    register size_t chars_read;

    if (! dry_run && ! skip_rest_of_patch) {
	int exclusive = TMPOUTNAME_needs_removal ? 0 : O_EXCL;
	assert (! inerrno);
	TMPOUTNAME_needs_removal = 1;
	copy_file (inname, TMPOUTNAME, exclusive, instat.st_mode);
	sprintf (buf, "%s %s%s", ed_program, verbosity == VERBOSE ? "" : "- ",
		 TMPOUTNAME);
	fflush (stdout);
	pipefp = popen(buf, binary_transput ? "wb" : "w");
	if (!pipefp)
	  pfatal ("Can't open pipe to %s", quotearg (buf));
    }
    for (;;) {
	char ed_command_letter;
	beginning_of_this_line = file_tell (pfp);
	chars_read = get_line ();
	if (! chars_read) {
	    next_intuit_at(beginning_of_this_line,p_input_line);
	    break;
	}
	ed_command_letter = get_ed_command_letter (buf);
	if (ed_command_letter) {
	    if (pipefp)
		if (! fwrite (buf, sizeof *buf, chars_read, pipefp))
		    write_fatal ();
	    if (ed_command_letter != 'd' && ed_command_letter != 's') {
	        p_pass_comments_through = true;
		while ((chars_read = get_line ()) != 0) {
		    if (pipefp)
			if (! fwrite (buf, sizeof *buf, chars_read, pipefp))
			    write_fatal ();
		    if (chars_read == 2  &&  strEQ (buf, ".\n"))
			break;
		}
		p_pass_comments_through = false;
	    }
	}
	else {
	    next_intuit_at(beginning_of_this_line,p_input_line);
	    break;
	}
    }
    if (!pipefp)
      return;
    if (fwrite ("w\nq\n", sizeof (char), (size_t) 4, pipefp) == 0
	|| fflush (pipefp) != 0)
      write_fatal ();
    if (pclose (pipefp) != 0)
      fatal ("%s FAILED", ed_program);

    if (ofp)
      {
	FILE *ifp = fopen (TMPOUTNAME, binary_transput ? "rb" : "r");
	int c;
	if (!ifp)
	  pfatal ("can't open `%s'", TMPOUTNAME);
	while ((c = getc (ifp)) != EOF)
	  if (putc (c, ofp) == EOF)
	    write_fatal ();
	if (ferror (ifp) || fclose (ifp) != 0)
	  read_fatal ();
      }
}
