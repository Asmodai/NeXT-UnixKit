/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

/*
 * ex_getln.c: Functions for entering and editing an Ex command line.
 */

#include "vim.h"

/*
 * Variables shared between getcmdline(), redrawcmdline() and others.
 * These need to be saved when using CTRL-R |, that's why they are in a
 * structure.
 */
struct cmdline_info
{
    char_u	*cmdbuff;	/* pointer to command line buffer */
    int		cmdbufflen;	/* length of cmdbuff */
    int		cmdlen;		/* number of chars in command line */
    int		cmdpos;		/* current cursor position */
    int		cmdspos;	/* cursor column on screen */
    int		cmdfirstc;	/* ':', '/', '?', '=' or NUL */
    int		cmdindent;	/* number of spaces before cmdline */
    char_u	*cmdprompt;	/* message in front of cmdline */
    int		cmdattr;	/* attributes for prompt */
    int		overstrike;	/* Typing mode on the command line.  Shared by
				   getcmdline() and put_on_cmdline(). */
};

static struct cmdline_info ccline;	/* current cmdline_info */

static int	cmd_showtail;		/* Only show path tail in lists ? */

#ifdef FEAT_EVAL
static int	new_cmdpos;	/* position set by set_cmdline_pos() */
#endif

#ifdef FEAT_CMDHIST
typedef struct hist_entry
{
    int		hisnum;		/* identifying number */
    char_u	*hisstr;	/* actual entry, separator char after the NUL */
} histentry_T;

static histentry_T *(history[HIST_COUNT]) = {NULL, NULL, NULL, NULL, NULL};
static int	hisidx[HIST_COUNT] = {-1, -1, -1, -1, -1};  /* lastused entry */
static int	hisnum[HIST_COUNT] = {0, 0, 0, 0, 0};
		    /* identifying (unique) number of newest history entry */
static int	hislen = 0;		/* actual length of history tables */

static int	hist_char2type __ARGS((int c));
static void	init_history __ARGS((void));

static int	in_history __ARGS((int, char_u *, int));
# ifdef FEAT_EVAL
static int	calc_hist_idx __ARGS((int histype, int num));
# endif
#endif

#ifdef FEAT_RIGHTLEFT
static int	cmd_hkmap = 0;	/* Hebrew mapping during command line */
#endif

#ifdef FEAT_FKMAP
static int	cmd_fkmap = 0;	/* Farsi mapping during command line */
#endif

static int	cmdline_charsize __ARGS((int idx));
static void	set_cmdspos __ARGS((void));
static void	set_cmdspos_cursor __ARGS((void));
#ifdef FEAT_MBYTE
static void	correct_cmdspos __ARGS((int idx, int cells));
#endif
static void	alloc_cmdbuff __ARGS((int len));
static int	realloc_cmdbuff __ARGS((int len));
static void	draw_cmdline __ARGS((int start, int len));
#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
static void	redrawcmd_preedit __ARGS((void));
#endif
#ifdef FEAT_WILDMENU
static void	cmdline_del __ARGS((int from));
#endif
static void	redrawcmdprompt __ARGS((void));
static void	cursorcmd __ARGS((void));
static int	ccheck_abbr __ARGS((int));
static int	nextwild __ARGS((expand_T *xp, int type, int options));
static int	showmatches __ARGS((expand_T *xp, int wildmenu));
static void	set_expand_context __ARGS((expand_T *xp));
static int	ExpandFromContext __ARGS((expand_T *xp, char_u *, int *, char_u ***, int));
static int	expand_showtail __ARGS((expand_T *xp));
#ifdef FEAT_CMDL_COMPL
static int	ExpandRTDir __ARGS((char_u *pat, int *num_file, char_u ***file, char *dirname));
# if defined(FEAT_USR_CMDS) && defined(FEAT_EVAL)
static int	ExpandUserDefined __ARGS((expand_T *xp, regmatch_T *regmatch, int *num_file, char_u ***file));
# endif
#endif

#ifdef FEAT_CMDWIN
static int	ex_window __ARGS((void));
#endif

/*
 * getcmdline() - accept a command line starting with firstc.
 *
 * firstc == ':'	    get ":" command line.
 * firstc == '/' or '?'	    get search pattern
 * firstc == '='	    get expression
 * firstc == '@'	    get text for input() function
 * firstc == '>'	    get text for debug mode
 * firstc == NUL	    get text for :insert command
 * firstc == -1		    like NUL, and break on CTRL-C
 *
 * The line is collected in ccline.cmdbuff, which is reallocated to fit the
 * command line.
 *
 * Careful: getcmdline() can be called recursively!
 *
 * Return pointer to allocated string if there is a commandline, NULL
 * otherwise.
 */
/*ARGSUSED*/
    char_u *
getcmdline(firstc, count, indent)
    int		firstc;
    long	count;		/* only used for incremental search */
    int		indent;		/* indent for inside conditionals */
{
    int		c;
    int		i;
    int		j;
    int		gotesc = FALSE;		/* TRUE when <ESC> just typed */
    int		do_abbr;		/* when TRUE check for abbr. */
#ifdef FEAT_CMDHIST
    char_u	*lookfor = NULL;	/* string to match */
    int		hiscnt;			/* current history line in use */
    int		histype;		/* history type to be used */
#endif
#ifdef FEAT_SEARCH_EXTRA
    pos_T	old_cursor;
    colnr_T	old_curswant;
    colnr_T	old_leftcol;
    linenr_T	old_topline;
# ifdef FEAT_DIFF
    int		old_topfill;
# endif
    linenr_T	old_botline;
    int		did_incsearch = FALSE;
    int		incsearch_postponed = FALSE;
#endif
    int		did_wild_list = FALSE;	/* did wild_list() recently */
    int		wim_index = 0;		/* index in wim_flags[] */
    int		res;
    int		save_msg_scroll = msg_scroll;
    int		save_State = State;	/* remember State when called */
    int		some_key_typed = FALSE;	/* one of the keys was typed */
#ifdef FEAT_MOUSE
    /* mouse drag and release events are ignored, unless they are
     * preceded with a mouse down event */
    int		ignore_drag_release = TRUE;
#endif
#ifdef FEAT_EVAL
    int		break_ctrl_c = FALSE;
#endif
    expand_T	xpc;
    long	*b_im_ptr = NULL;

#ifdef FEAT_SNIFF
    want_sniff_request = 0;
#endif
#ifdef FEAT_EVAL
    if (firstc == -1)
    {
	firstc = NUL;
	break_ctrl_c = TRUE;
    }
#endif
#ifdef FEAT_RIGHTLEFT
    /* start without Hebrew mapping for a command line */
    if (firstc == ':' || firstc == '=' || firstc == '>')
	cmd_hkmap = 0;
#endif

    ccline.overstrike = FALSE;		    /* always start in insert mode */
#ifdef FEAT_SEARCH_EXTRA
    old_cursor = curwin->w_cursor;	    /* needs to be restored later */
    old_curswant = curwin->w_curswant;
    old_leftcol = curwin->w_leftcol;
    old_topline = curwin->w_topline;
# ifdef FEAT_DIFF
    old_topfill = curwin->w_topfill;
# endif
    old_botline = curwin->w_botline;
#endif

    /*
     * set some variables for redrawcmd()
     */
    ccline.cmdfirstc = (firstc == '@' ? 0 : firstc);
    ccline.cmdindent = indent;
    alloc_cmdbuff(exmode_active ? 250 : 0); /* alloc initial ccline.cmdbuff */
    if (ccline.cmdbuff == NULL)
	return NULL;			    /* out of memory */
    ccline.cmdlen = ccline.cmdpos = 0;
    ccline.cmdbuff[0] = NUL;

    ExpandInit(&xpc);

#ifdef FEAT_RIGHTLEFT
    if (curwin->w_p_rl && *curwin->w_p_rlc == 's'
					  && (firstc == '/' || firstc == '?'))
	cmdmsg_rl = TRUE;
    else
	cmdmsg_rl = FALSE;
#endif

    redir_off = TRUE;		/* don't redirect the typed command */
    if (!cmd_silent)
    {
	i = msg_scrolled;
	msg_scrolled = 0;		/* avoid wait_return message */
	gotocmdline(TRUE);
	msg_scrolled += i;
	redrawcmdprompt();		/* draw prompt or indent */
	set_cmdspos();
    }
    xpc.xp_context = EXPAND_NOTHING;
    xpc.xp_backslash = XP_BS_NONE;

    /*
     * Avoid scrolling when called by a recursive do_cmdline(), e.g. when
     * doing ":@0" when register 0 doesn't contain a CR.
     */
    msg_scroll = FALSE;

    State = CMDLINE;

    if (firstc == '/' || firstc == '?' || firstc == '@')
    {
	/* Use ":lmap" mappings for search pattern and input(). */
	if (curbuf->b_p_imsearch == B_IMODE_USE_INSERT)
	    b_im_ptr = &curbuf->b_p_iminsert;
	else
	    b_im_ptr = &curbuf->b_p_imsearch;
	if (*b_im_ptr == B_IMODE_LMAP)
	    State |= LANGMAP;
#ifdef USE_IM_CONTROL
	im_set_active(*b_im_ptr == B_IMODE_IM);
#endif
    }
#ifdef USE_IM_CONTROL
    else if (p_imcmdline)
	im_set_active(TRUE);
#endif

#ifdef FEAT_MOUSE
    setmouse();
#endif
#ifdef CURSOR_SHAPE
    ui_cursor_shape();		/* may show different cursor shape */
#endif

#ifdef FEAT_CMDHIST
    init_history();
    hiscnt = hislen;		/* set hiscnt to impossible history value */
    histype = hist_char2type(firstc);
#endif

#ifdef FEAT_DIGRAPHS
    do_digraph(-1);		/* init digraph typahead */
#endif

    /*
     * Collect the command string, handling editing keys.
     */
    for (;;)
    {
#ifdef USE_ON_FLY_SCROLL
	dont_scroll = FALSE;	/* allow scrolling here */
#endif
	quit_more = FALSE;	/* reset after CTRL-D which had a more-prompt */

	cursorcmd();		/* set the cursor on the right spot */
	c = safe_vgetc();
	if (KeyTyped)
	{
	    some_key_typed = TRUE;
#ifdef FEAT_RIGHTLEFT
	    if (cmd_hkmap)
		c = hkmap(c);
# ifdef FEAT_FKMAP
	    if (cmd_fkmap)
		c = cmdl_fkmap(c);
# endif
	    if (cmdmsg_rl && !KeyStuffed)
	    {
		/* Invert horizontal movements and operations.  Only when
		 * typed by the user directly, not when the result of a
		 * mapping. */
		switch (c)
		{
		    case K_RIGHT:   c = K_LEFT; break;
		    case K_S_RIGHT: c = K_S_LEFT; break;
		    case K_C_RIGHT: c = K_C_LEFT; break;
		    case K_LEFT:    c = K_RIGHT; break;
		    case K_S_LEFT:  c = K_S_RIGHT; break;
		    case K_C_LEFT:  c = K_C_RIGHT; break;
		}
	    }
#endif
	}

	/*
	 * Ignore got_int when CTRL-C was typed here.
	 * Don't ignore it in :global, we really need to break then, e.g., for
	 * ":g/pat/normal /pat" (without the <CR>).
	 * Don't ignore it for the input() function.
	 */
	if ((c == Ctrl_C
#ifdef UNIX
		|| c == intr_char
#endif
				)
#if defined(FEAT_EVAL) || defined(FEAT_CRYPT)
		&& firstc != '@'
#endif
#ifdef FEAT_EVAL
		&& !break_ctrl_c
#endif
		&& !global_busy)
	    got_int = FALSE;

#ifdef FEAT_CMDHIST
	/* free old command line when finished moving around in the history
	 * list */
	if (lookfor != NULL
		&& c != K_S_DOWN && c != K_S_UP && c != K_DOWN && c != K_UP
		&& c != K_PAGEDOWN && c != K_PAGEUP
		&& c != K_KPAGEDOWN && c != K_KPAGEUP
		&& c != K_LEFT && c != K_RIGHT
		&& (xpc.xp_numfiles > 0 || (c != Ctrl_P && c != Ctrl_N)))
	{
	    vim_free(lookfor);
	    lookfor = NULL;
	}
#endif

	/*
	 * <S-Tab> works like CTRL-P (unless 'wc' is <S-Tab>).
	 */
	if (c != p_wc && c == K_S_TAB && xpc.xp_numfiles != -1)
	    c = Ctrl_P;

#ifdef FEAT_WILDMENU
	/* Special translations for 'wildmenu' */
	if (did_wild_list && p_wmnu)
	{
	    if (c == K_LEFT)
		c = Ctrl_P;
	    else if (c == K_RIGHT)
		c = Ctrl_N;
	}
	/* Hitting CR after "emenu Name.": complete submenu */
	if (xpc.xp_context == EXPAND_MENUNAMES && p_wmnu
		&& ccline.cmdpos > 1
		&& ccline.cmdbuff[ccline.cmdpos - 1] == '.'
		&& ccline.cmdbuff[ccline.cmdpos - 2] != '\\'
		&& (c == '\n' || c == '\r' || c == K_KENTER))
	    c = K_DOWN;
#endif

	/* free expanded names when finished walking through matches */
	if (xpc.xp_numfiles != -1
		&& !(c == p_wc && KeyTyped) && c != p_wcm
		&& c != Ctrl_N && c != Ctrl_P && c != Ctrl_A
		&& c != Ctrl_L)
	{
	    (void)ExpandOne(&xpc, NULL, NULL, 0, WILD_FREE);
	    did_wild_list = FALSE;
#ifdef FEAT_WILDMENU
	    if (!p_wmnu || (c != K_UP && c != K_DOWN))
#endif
		xpc.xp_context = EXPAND_NOTHING;
	    wim_index = 0;
#ifdef FEAT_WILDMENU
	    if (p_wmnu && wild_menu_showing != 0)
	    {
		int skt = KeyTyped;

		if (wild_menu_showing == WM_SCROLLED)
		{
		    /* Entered command line, move it up */
		    cmdline_row--;
		    redrawcmd();
		}
		else if (save_p_ls != -1)
		{
		    /* restore 'laststatus' and 'winminheight' */
		    p_ls = save_p_ls;
		    p_wmh = save_p_wmh;
		    last_status(FALSE);
		    update_screen(VALID);	/* redraw the screen NOW */
		    redrawcmd();
		    save_p_ls = -1;
		}
		else
		{
# ifdef FEAT_VERTSPLIT
		    win_redraw_last_status(topframe);
# else
		    lastwin->w_redr_status = TRUE;
# endif
		    redraw_statuslines();
		}
		KeyTyped = skt;
		wild_menu_showing = 0;
	    }
#endif
	}

#ifdef FEAT_WILDMENU
	/* Special translations for 'wildmenu' */
	if (xpc.xp_context == EXPAND_MENUNAMES && p_wmnu)
	{
	    /* Hitting <Down> after "emenu Name.": complete submenu */
	    if (ccline.cmdbuff[ccline.cmdpos - 1] == '.' && c == K_DOWN)
		c = p_wc;
	    else if (c == K_UP)
	    {
		/* Hitting <Up>: Remove one submenu name in front of the
		 * cursor */
		int found = FALSE;

		j = (int)(xpc.xp_pattern - ccline.cmdbuff);
		i = 0;
		while (--j > 0)
		{
		    /* check for start of menu name */
		    if (ccline.cmdbuff[j] == ' '
			    && ccline.cmdbuff[j - 1] != '\\')
		    {
			i = j + 1;
			break;
		    }
		    /* check for start of submenu name */
		    if (ccline.cmdbuff[j] == '.'
			    && ccline.cmdbuff[j - 1] != '\\')
		    {
			if (found)
			{
			    i = j + 1;
			    break;
			}
			else
			    found = TRUE;
		    }
		}
		if (i > 0)
		    cmdline_del(i);
		c = p_wc;
		xpc.xp_context = EXPAND_NOTHING;
	    }
	}
	if (xpc.xp_context == EXPAND_FILES && p_wmnu)
	{
	    char_u upseg[5];

	    upseg[0] = PATHSEP;
	    upseg[1] = '.';
	    upseg[2] = '.';
	    upseg[3] = PATHSEP;
	    upseg[4] = NUL;

	    if (ccline.cmdbuff[ccline.cmdpos - 1] == PATHSEP
		    && c == K_DOWN
		    && (ccline.cmdbuff[ccline.cmdpos - 2] != '.'
			|| ccline.cmdbuff[ccline.cmdpos - 3] != '.'))
	    {
		/* go down a directory */
		c = p_wc;
	    }
	    else if (STRNCMP(xpc.xp_pattern, upseg + 1, 3) == 0 && c == K_DOWN)
	    {
		/* If in a direct ancestor, strip off one ../ to go down */
		int found = FALSE;

		j = ccline.cmdpos;
		i = (int)(xpc.xp_pattern - ccline.cmdbuff);
		while (--j > i)
		{
		    if (vim_ispathsep(ccline.cmdbuff[j]))
		    {
			found = TRUE;
			break;
		    }
		}
		if (found
			&& ccline.cmdbuff[j - 1] == '.'
			&& ccline.cmdbuff[j - 2] == '.'
			&& (vim_ispathsep(ccline.cmdbuff[j - 3]) || j == i + 2))
		{
		    cmdline_del(j - 2);
		    c = p_wc;
		}
	    }
	    else if (c == K_UP)
	    {
		/* go up a directory */
		int found = FALSE;

		j = ccline.cmdpos - 1;
		i = (int)(xpc.xp_pattern - ccline.cmdbuff);
		while (--j > i)
		{
#ifdef FEAT_MBYTE
		    if (has_mbyte)
			j -= (*mb_head_off)(ccline.cmdbuff, ccline.cmdbuff + j);
#endif
		    if (vim_ispathsep(ccline.cmdbuff[j])
#ifdef BACKSLASH_IN_FILENAME
			    && vim_strchr(" *?[{`$%#", ccline.cmdbuff[j + 1])
			       == NULL
#endif
		       )
		    {
			if (found)
			{
			    i = j + 1;
			    break;
			}
			else
			    found = TRUE;
		    }
		}

		if (!found)
		    j = i;
		else if (STRNCMP(ccline.cmdbuff + j, upseg, 4) == 0)
		    j += 4;
		else if (STRNCMP(ccline.cmdbuff + j, upseg + 1, 3) == 0
			     && j == i)
		    j += 3;
		else
		    j = 0;
		if (j > 0)
		{
		    /* TODO this is only for DOS/UNIX systems - need to put in
		     * machine-specific stuff here and in upseg init */
		    cmdline_del(j);
		    put_on_cmdline(upseg + 1, 3, FALSE);
		}
		else if (ccline.cmdpos > i)
		    cmdline_del(i);
		c = p_wc;
	    }
	}
#if 0 /* If enabled <Down> on a file takes you _completely_ out of wildmenu */
	if (p_wmnu
		&& (xpc.xp_context == EXPAND_FILES
		    || xpc.xp_context == EXPAND_MENUNAMES)
		&& (c == K_UP || c == K_DOWN))
	    xpc.xp_context = EXPAND_NOTHING;
#endif

#endif	/* FEAT_WILDMENU */

	/* CTRL-\ CTRL-N goes to Normal mode, CTRL-\ CTRL-G goes to Insert
	 * mode when 'insertmode' is set, CTRL-\ e prompts for an expression. */
	if (c == Ctrl_BSL)
	{
	    ++no_mapping;
	    ++allow_keys;
	    c = safe_vgetc();
	    --no_mapping;
	    --allow_keys;
	    /* CTRL-\ e doesn't work when obtaining an expression. */
	    if (c != Ctrl_N && c != Ctrl_G
				     && (c != 'e' || ccline.cmdfirstc == '='))
	    {
		vungetc(c);
		c = Ctrl_BSL;
	    }
#ifdef FEAT_EVAL
	    else if (c == 'e')
	    {
		struct cmdline_info	    save_ccline;
		char_u		    *p;

		/*
		 * Replace the command line with the result of an expression.
		 * Need to save the current command line, to be able to enter
		 * a new one...
		 */
		if (ccline.cmdpos == ccline.cmdlen)
		    new_cmdpos = 99999;	/* keep it at the end */
		else
		    new_cmdpos = ccline.cmdpos;
		save_ccline = ccline;
		ccline.cmdbuff = NULL;
		ccline.cmdprompt = NULL;
		c = get_expr_register();
		ccline = save_ccline;
		if (c == '=')
		{
		    p = get_expr_line();
		    if (p != NULL
			     && realloc_cmdbuff((int)STRLEN(p) + 1) == OK)
		    {
			ccline.cmdlen = STRLEN(p);
			STRCPY(ccline.cmdbuff, p);
			vim_free(p);

			/* Restore the cursor or use the position set with
			 * set_cmdline_pos(). */
			if (new_cmdpos > ccline.cmdlen)
			    ccline.cmdpos = ccline.cmdlen;
			else
			    ccline.cmdpos = new_cmdpos;

			KeyTyped = FALSE;	/* Don't do p_wc completion. */
			redrawcmd();
			goto cmdline_changed;
		    }
		}
		beep_flush();
		c = ESC;
	    }
#endif
	    else
	    {
		if (c == Ctrl_G && p_im && restart_edit == 0)
		    restart_edit = 'a';
		gotesc = TRUE;	/* will free ccline.cmdbuff after putting it
				   in history */
		goto returncmd;	/* back to Normal mode */
	    }
	}

#ifdef FEAT_CMDWIN
	if (c == cedit_key || c == K_CMDWIN)
	{
	    /*
	     * Open a window to edit the command line (and history).
	     */
	    c = ex_window();
	    some_key_typed = TRUE;
	}
# ifdef FEAT_DIGRAPHS
	else
# endif
#endif
#ifdef FEAT_DIGRAPHS
	    c = do_digraph(c);
#endif

	if (c == '\n' || c == '\r' || c == K_KENTER || (c == ESC
			&& (!KeyTyped || vim_strchr(p_cpo, CPO_ESC) != NULL)))
	{
	    gotesc = FALSE;	/* Might have typed ESC previously, don't
				   truncate the cmdline now. */
	    if (ccheck_abbr(c + ABBR_OFF))
		goto cmdline_changed;
	    if (!cmd_silent)
	    {
		windgoto(msg_row, 0);
		out_flush();
	    }
	    break;
	}

	/*
	 * Completion for 'wildchar' or 'wildcharm' key.
	 * - hitting <ESC> twice means: abandon command line.
	 * - wildcard expansion is only done when the 'wildchar' key is really
	 *   typed, not when it comes from a macro
	 */
	if ((c == p_wc && !gotesc && KeyTyped) || c == p_wcm)
	{
	    if (xpc.xp_numfiles > 0)   /* typed p_wc at least twice */
	    {
		/* if 'wildmode' contains "list" may still need to list */
		if (xpc.xp_numfiles > 1
			&& !did_wild_list
			&& (wim_flags[wim_index] & WIM_LIST))
		{
		    (void)showmatches(&xpc, FALSE);
		    redrawcmd();
		    did_wild_list = TRUE;
		}
		if (wim_flags[wim_index] & WIM_LONGEST)
		    res = nextwild(&xpc, WILD_LONGEST, WILD_NO_BEEP);
		else if (wim_flags[wim_index] & WIM_FULL)
		    res = nextwild(&xpc, WILD_NEXT, WILD_NO_BEEP);
		else
		    res = OK;	    /* don't insert 'wildchar' now */
	    }
	    else		    /* typed p_wc first time */
	    {
		wim_index = 0;
		j = ccline.cmdpos;
		/* if 'wildmode' first contains "longest", get longest
		 * common part */
		if (wim_flags[0] & WIM_LONGEST)
		    res = nextwild(&xpc, WILD_LONGEST, WILD_NO_BEEP);
		else
		    res = nextwild(&xpc, WILD_EXPAND_KEEP, WILD_NO_BEEP);

		/* if interrupted while completing, behave like it failed */
		if (got_int)
		{
		    (void)vpeekc();	/* remove <C-C> from input stream */
		    got_int = FALSE;	/* don't abandon the command line */
		    (void)ExpandOne(&xpc, NULL, NULL, 0, WILD_FREE);
#ifdef FEAT_WILDMENU
		    xpc.xp_context = EXPAND_NOTHING;
#endif
		    goto cmdline_changed;
		}

		/* when more than one match, and 'wildmode' first contains
		 * "list", or no change and 'wildmode' contains "longest,list",
		 * list all matches */
		if (res == OK && xpc.xp_numfiles > 1)
		{
		    /* a "longest" that didn't do anything is skipped (but not
		     * "list:longest") */
		    if (wim_flags[0] == WIM_LONGEST && ccline.cmdpos == j)
			wim_index = 1;
		    if ((wim_flags[wim_index] & WIM_LIST)
#ifdef FEAT_WILDMENU
			    || (p_wmnu && (wim_flags[wim_index] & WIM_FULL) != 0)
#endif
			    )
		    {
			if (!(wim_flags[0] & WIM_LONGEST))
			{
#ifdef FEAT_WILDMENU
			    int p_wmnu_save = p_wmnu;
			    p_wmnu = 0;
#endif
			    nextwild(&xpc, WILD_PREV, 0); /* remove match */
#ifdef FEAT_WILDMENU
			    p_wmnu = p_wmnu_save;
#endif
			}
#ifdef FEAT_WILDMENU
			(void)showmatches(&xpc, p_wmnu
				&& ((wim_flags[wim_index] & WIM_LIST) == 0));
#else
			(void)showmatches(&xpc, FALSE);
#endif
			redrawcmd();
			did_wild_list = TRUE;
			if (wim_flags[wim_index] & WIM_LONGEST)
			    nextwild(&xpc, WILD_LONGEST, WILD_NO_BEEP);
			else if (wim_flags[wim_index] & WIM_FULL)
			    nextwild(&xpc, WILD_NEXT, WILD_NO_BEEP);
		    }
		    else
			vim_beep();
		}
#ifdef FEAT_WILDMENU
		else if (xpc.xp_numfiles == -1)
		    xpc.xp_context = EXPAND_NOTHING;
#endif
	    }
	    if (wim_index < 3)
		++wim_index;
	    if (c == ESC)
		gotesc = TRUE;
	    if (res == OK)
		goto cmdline_changed;
	}

	gotesc = FALSE;

	/* <S-Tab> goes to last match, in a clumsy way */
	if (c == K_S_TAB && KeyTyped)
	{
	    if (nextwild(&xpc, WILD_EXPAND_KEEP, 0) == OK
		    && nextwild(&xpc, WILD_PREV, 0) == OK
		    && nextwild(&xpc, WILD_PREV, 0) == OK)
		goto cmdline_changed;
	}

	if (c == NUL || c == K_ZERO)	    /* NUL is stored as NL */
	    c = NL;

	do_abbr = TRUE;		/* default: check for abbreviation */

	/*
	 * Big switch for a typed command line character.
	 */
	switch (c)
	{
	case K_BS:
	case Ctrl_H:
	case K_DEL:
	case K_KDEL:
	case Ctrl_W:
#ifdef FEAT_FKMAP
		if (cmd_fkmap && c == K_BS)
		    c = K_DEL;
#endif
		if (c == K_KDEL)
		    c = K_DEL;

		/*
		 * delete current character is the same as backspace on next
		 * character, except at end of line
		 */
		if (c == K_DEL && ccline.cmdpos != ccline.cmdlen)
		    ++ccline.cmdpos;
#ifdef FEAT_MBYTE
		if (has_mbyte && c == K_DEL)
		    ccline.cmdpos += mb_off_next(ccline.cmdbuff,
					      ccline.cmdbuff + ccline.cmdpos);
#endif
		if (ccline.cmdpos > 0)
		{
		    char_u *p;

		    j = ccline.cmdpos;
		    p = ccline.cmdbuff + j;
#ifdef FEAT_MBYTE
		    if (has_mbyte)
		    {
			p = mb_prevptr(ccline.cmdbuff, p);
			if (c == Ctrl_W)
			{
			    while (p > ccline.cmdbuff && vim_isspace(*p))
				p = mb_prevptr(ccline.cmdbuff, p);
			    i = mb_get_class(p);
			    while (p > ccline.cmdbuff && mb_get_class(p) == i)
				p = mb_prevptr(ccline.cmdbuff, p);
			    if (mb_get_class(p) != i)
				p += (*mb_ptr2len_check)(p);
			}
		    }
		    else
#endif
			if (c == Ctrl_W)
		    {
			while (p > ccline.cmdbuff && vim_isspace(p[-1]))
			    --p;
			i = vim_iswordc(p[-1]);
			while (p > ccline.cmdbuff && !vim_isspace(p[-1])
				&& vim_iswordc(p[-1]) == i)
			    --p;
		    }
		    else
			--p;
		    ccline.cmdpos = (int)(p - ccline.cmdbuff);
		    ccline.cmdlen -= j - ccline.cmdpos;
		    i = ccline.cmdpos;
		    while (i < ccline.cmdlen)
			ccline.cmdbuff[i++] = ccline.cmdbuff[j++];

		    /* Truncate at the end, required for multi-byte chars. */
		    ccline.cmdbuff[ccline.cmdlen] = NUL;
		    redrawcmd();
		}
		else if (ccline.cmdlen == 0 && c != Ctrl_W
				   && ccline.cmdprompt == NULL && indent == 0)
		{
		    /* In ex and debug mode it doesn't make sense to return. */
		    if (exmode_active
#ifdef FEAT_EVAL
			    || ccline.cmdfirstc == '>'
#endif
			    )
			goto cmdline_not_changed;

		    vim_free(ccline.cmdbuff);	/* no commandline to return */
		    ccline.cmdbuff = NULL;
		    if (!cmd_silent)
		    {
#ifdef FEAT_RIGHTLEFT
			if (cmdmsg_rl)
			    msg_col = Columns;
			else
#endif
			    msg_col = 0;
			msg_putchar(' ');		/* delete ':' */
		    }
		    redraw_cmdline = TRUE;
		    goto returncmd;		/* back to cmd mode */
		}
		goto cmdline_changed;

	case K_INS:
	case K_KINS:
#ifdef FEAT_FKMAP
		/* if Farsi mode set, we are in reverse insert mode -
		   Do not change the mode */
		if (cmd_fkmap)
		    beep_flush();
		else
#endif
		ccline.overstrike = !ccline.overstrike;
#ifdef CURSOR_SHAPE
		ui_cursor_shape();	/* may show different cursor shape */
#endif
		goto cmdline_not_changed;

	case Ctrl_HAT:
		if (map_to_exists_mode((char_u *)"", LANGMAP))
		{
		    /* ":lmap" mappings exists, toggle use of mappings. */
		    State ^= LANGMAP;
#ifdef USE_IM_CONTROL
		    im_set_active(FALSE);	/* Disable input method */
#endif
		    if (b_im_ptr != NULL)
		    {
			if (State & LANGMAP)
			    *b_im_ptr = B_IMODE_LMAP;
			else
			    *b_im_ptr = B_IMODE_NONE;
		    }
		}
#ifdef USE_IM_CONTROL
		else
		{
		    /* There are no ":lmap" mappings, toggle IM.  When
		     * 'imdisable' is set don't try getting the status, it's
		     * always off. */
		    if ((p_imdisable && b_im_ptr != NULL)
			    ? *b_im_ptr == B_IMODE_IM : im_get_status())
		    {
			im_set_active(FALSE);	/* Disable input method */
			if (b_im_ptr != NULL)
			    *b_im_ptr = B_IMODE_NONE;
		    }
		    else
		    {
			im_set_active(TRUE);	/* Enable input method */
			if (b_im_ptr != NULL)
			    *b_im_ptr = B_IMODE_IM;
		    }
		}
#endif
		if (b_im_ptr != NULL)
		{
		    if (b_im_ptr == &curbuf->b_p_iminsert)
			set_iminsert_global();
		    else
			set_imsearch_global();
		}
#ifdef CURSOR_SHAPE
		ui_cursor_shape();	/* may show different cursor shape */
#endif
#if defined(FEAT_WINDOWS) && defined(FEAT_KEYMAP)
		/* Show/unshow value of 'keymap' in status lines later. */
		status_redraw_curbuf();
#endif
		goto cmdline_not_changed;

/*	case '@':   only in very old vi */
	case Ctrl_U:
		/* delete all characters left of the cursor */
		j = ccline.cmdpos;
		ccline.cmdlen -= j;
		i = ccline.cmdpos = 0;
		while (i < ccline.cmdlen)
		    ccline.cmdbuff[i++] = ccline.cmdbuff[j++];
		/* Truncate at the end, required for multi-byte chars. */
		ccline.cmdbuff[ccline.cmdlen] = NUL;
		redrawcmd();
		goto cmdline_changed;

#ifdef FEAT_CLIPBOARD
	case Ctrl_Y:
		/* Copy the modeless selection, if there is one. */
		if (clip_star.state != SELECT_CLEARED)
		{
		    if (clip_star.state == SELECT_DONE)
			clip_copy_modeless_selection(TRUE);
		    goto cmdline_not_changed;
		}
		break;
#endif

	case ESC:	/* get here if p_wc != ESC or when ESC typed twice */
	case Ctrl_C:
		/* In exmode it doesn't make sense to return. */
		if (exmode_active)
		    goto cmdline_not_changed;

		gotesc = TRUE;		/* will free ccline.cmdbuff after
					   putting it in history */
		goto returncmd;		/* back to cmd mode */

	case Ctrl_R:			/* insert register */
#ifdef USE_ON_FLY_SCROLL
		dont_scroll = TRUE;	/* disallow scrolling here */
#endif
		putcmdline('"', TRUE);
		++no_mapping;
		i = c = safe_vgetc();	/* CTRL-R <char> */
		if (i == Ctrl_O)
		    i = Ctrl_R;		/* CTRL-R CTRL-O == CTRL-R CTRL-R */
		if (i == Ctrl_R)
		    c = safe_vgetc();	/* CTRL-R CTRL-R <char> */
		--no_mapping;
#ifdef FEAT_EVAL
		/*
		 * Insert the result of an expression.
		 * Need to save the current command line, to be able to enter
		 * a new one...
		 */
		new_cmdpos = -1;
		if (c == '=')
		{
		    struct cmdline_info	    save_ccline;

		    if (ccline.cmdfirstc == '=')/* can't do this recursively */
		    {
			beep_flush();
			c = ESC;
		    }
		    else
		    {
			save_ccline = ccline;
			ccline.cmdbuff = NULL;
			ccline.cmdprompt = NULL;
			c = get_expr_register();
			ccline = save_ccline;
		    }
		}
#endif
		if (c != ESC)	    /* use ESC to cancel inserting register */
		{
		    cmdline_paste(c, i == Ctrl_R);
		    KeyTyped = FALSE;	/* Don't do p_wc completion. */
#ifdef FEAT_EVAL
		    if (new_cmdpos >= 0)
		    {
			/* set_cmdline_pos() was used */
			if (new_cmdpos > ccline.cmdlen)
			    ccline.cmdpos = ccline.cmdlen;
			else
			    ccline.cmdpos = new_cmdpos;
		    }
#endif
		}
		redrawcmd();
		goto cmdline_changed;

	case Ctrl_D:
		if (showmatches(&xpc, FALSE) == EXPAND_NOTHING)
		    break;	/* Use ^D as normal char instead */

		redrawcmd();
		continue;	/* don't do incremental search now */

	case K_RIGHT:
	case K_S_RIGHT:
	case K_C_RIGHT:
		do
		{
		    if (ccline.cmdpos >= ccline.cmdlen)
			break;
		    i = cmdline_charsize(ccline.cmdpos);
		    if (KeyTyped && ccline.cmdspos + i >= Columns * Rows)
			break;
		    ccline.cmdspos += i;
#ifdef FEAT_MBYTE
		    if (has_mbyte)
			ccline.cmdpos += (*mb_ptr2len_check)(ccline.cmdbuff
							     + ccline.cmdpos);
		    else
#endif
			++ccline.cmdpos;
		}
		while ((c == K_S_RIGHT || c == K_C_RIGHT)
			&& ccline.cmdbuff[ccline.cmdpos] != ' ');
#ifdef FEAT_MBYTE
		if (has_mbyte)
		    set_cmdspos_cursor();
#endif
		goto cmdline_not_changed;

	case K_LEFT:
	case K_S_LEFT:
	case K_C_LEFT:
		do
		{
		    if (ccline.cmdpos == 0)
			break;
		    --ccline.cmdpos;
#ifdef FEAT_MBYTE
		    if (has_mbyte)	/* move to first byte of char */
			ccline.cmdpos -= (*mb_head_off)(ccline.cmdbuff,
					      ccline.cmdbuff + ccline.cmdpos);
#endif
		    ccline.cmdspos -= cmdline_charsize(ccline.cmdpos);
		}
		while ((c == K_S_LEFT || c == K_C_LEFT)
			&& ccline.cmdbuff[ccline.cmdpos - 1] != ' ');
#ifdef FEAT_MBYTE
		if (has_mbyte)
		    set_cmdspos_cursor();
#endif
		goto cmdline_not_changed;

	case K_IGNORE:
		goto cmdline_not_changed;	/* Ignore mouse */

#ifdef FEAT_MOUSE
	case K_MIDDLEDRAG:
	case K_MIDDLERELEASE:
		goto cmdline_not_changed;	/* Ignore mouse */

	case K_MIDDLEMOUSE:
# ifdef FEAT_GUI
		/* When GUI is active, also paste when 'mouse' is empty */
		if (!gui.in_use)
# endif
		    if (!mouse_has(MOUSE_COMMAND))
			goto cmdline_not_changed;   /* Ignore mouse */
#ifdef FEAT_CLIPBOARD
		if (clip_star.available)
		    cmdline_paste('*', TRUE);
		else
#endif
		    cmdline_paste(0, TRUE);
		redrawcmd();
		goto cmdline_changed;

#ifdef FEAT_DND
	case K_DROP:
		cmdline_paste('~', TRUE);
		redrawcmd();
		goto cmdline_changed;
#endif

	case K_LEFTDRAG:
	case K_LEFTRELEASE:
	case K_RIGHTDRAG:
	case K_RIGHTRELEASE:
		/* Ignore drag and release events when the button-down wasn't
		 * seen before. */
		if (ignore_drag_release)
		    goto cmdline_not_changed;
		/* FALLTHROUGH */
	case K_LEFTMOUSE:
	case K_RIGHTMOUSE:
		if (c == K_LEFTRELEASE || c == K_RIGHTRELEASE)
		    ignore_drag_release = TRUE;
		else
		    ignore_drag_release = FALSE;
# ifdef FEAT_GUI
		/* When GUI is active, also move when 'mouse' is empty */
		if (!gui.in_use)
# endif
		    if (!mouse_has(MOUSE_COMMAND))
			goto cmdline_not_changed;   /* Ignore mouse */
# ifdef FEAT_CLIPBOARD
		if (mouse_row < cmdline_row && clip_star.available)
		{
		    int	    button, is_click, is_drag;

		    /*
		     * Handle modeless selection.
		     */
		    button = get_mouse_button(KEY2TERMCAP1(c),
							 &is_click, &is_drag);
		    if (mouse_model_popup() && button == MOUSE_LEFT
					       && (mod_mask & MOD_MASK_SHIFT))
		    {
			/* Translate shift-left to right button. */
			button = MOUSE_RIGHT;
			mod_mask &= ~MOD_MASK_SHIFT;
		    }
		    clip_modeless(button, is_click, is_drag);
		    goto cmdline_not_changed;
		}
# endif

		set_cmdspos();
		for (ccline.cmdpos = 0; ccline.cmdpos < ccline.cmdlen;
							      ++ccline.cmdpos)
		{
		    i = cmdline_charsize(ccline.cmdpos);
		    if (mouse_row <= cmdline_row + ccline.cmdspos / Columns
				  && mouse_col < ccline.cmdspos % Columns + i)
			break;
#ifdef FEAT_MBYTE
		    if (has_mbyte)
		    {
			/* Count ">" for double-wide char that doesn't fit. */
			correct_cmdspos(ccline.cmdpos, i);
			ccline.cmdpos += (*mb_ptr2len_check)(ccline.cmdbuff
							 + ccline.cmdpos) - 1;
		    }
#endif
		    ccline.cmdspos += i;
		}
		goto cmdline_not_changed;

	/* Mouse scroll wheel: ignored here */
	case K_MOUSEDOWN:
	case K_MOUSEUP:
	/* Alternate buttons ignored here */
	case K_X1MOUSE:
	case K_X1DRAG:
	case K_X1RELEASE:
	case K_X2MOUSE:
	case K_X2DRAG:
	case K_X2RELEASE:
		goto cmdline_not_changed;

#endif	/* FEAT_MOUSE */

#ifdef FEAT_GUI
	case K_LEFTMOUSE_NM:	/* mousefocus click, ignored */
	case K_LEFTRELEASE_NM:
		goto cmdline_not_changed;

	case K_VER_SCROLLBAR:
		if (!msg_scrolled)
		{
		    gui_do_scroll();
		    redrawcmd();
		}
		goto cmdline_not_changed;

	case K_HOR_SCROLLBAR:
		if (!msg_scrolled)
		{
		    gui_do_horiz_scroll();
		    redrawcmd();
		}
		goto cmdline_not_changed;
#endif
	case K_SELECT:	    /* end of Select mode mapping - ignore */
		goto cmdline_not_changed;

	case Ctrl_B:	    /* begin of command line */
	case K_HOME:
	case K_KHOME:
	case K_XHOME:
	case K_S_HOME:
	case K_C_HOME:
		ccline.cmdpos = 0;
		set_cmdspos();
		goto cmdline_not_changed;

	case Ctrl_E:	    /* end of command line */
	case K_END:
	case K_KEND:
	case K_XEND:
	case K_S_END:
	case K_C_END:
		ccline.cmdpos = ccline.cmdlen;
		set_cmdspos_cursor();
		goto cmdline_not_changed;

	case Ctrl_A:	    /* all matches */
		if (nextwild(&xpc, WILD_ALL, 0) == FAIL)
		    break;
		goto cmdline_changed;

	case Ctrl_L:	    /* longest common part */
		if (nextwild(&xpc, WILD_LONGEST, 0) == FAIL)
		    break;
		goto cmdline_changed;

	case Ctrl_N:	    /* next match */
	case Ctrl_P:	    /* previous match */
		if (xpc.xp_numfiles > 0)
		{
		    if (nextwild(&xpc, (c == Ctrl_P) ? WILD_PREV : WILD_NEXT, 0)
								      == FAIL)
			break;
		    goto cmdline_changed;
		}

#ifdef FEAT_CMDHIST
	case K_UP:
	case K_DOWN:
	case K_S_UP:
	case K_S_DOWN:
	case K_PAGEUP:
	case K_KPAGEUP:
	case K_PAGEDOWN:
	case K_KPAGEDOWN:
		if (hislen == 0 || firstc == NUL)	/* no history */
		    goto cmdline_not_changed;

		i = hiscnt;

		/* save current command string so it can be restored later */
		if (lookfor == NULL)
		{
		    if ((lookfor = vim_strsave(ccline.cmdbuff)) == NULL)
			goto cmdline_not_changed;
		    lookfor[ccline.cmdpos] = NUL;
		}

		j = (int)STRLEN(lookfor);
		for (;;)
		{
		    /* one step backwards */
		    if (c == K_UP || c == K_S_UP || c == Ctrl_P ||
			    c == K_PAGEUP || c == K_KPAGEUP)
		    {
			if (hiscnt == hislen)	/* first time */
			    hiscnt = hisidx[histype];
			else if (hiscnt == 0 && hisidx[histype] != hislen - 1)
			    hiscnt = hislen - 1;
			else if (hiscnt != hisidx[histype] + 1)
			    --hiscnt;
			else			/* at top of list */
			{
			    hiscnt = i;
			    break;
			}
		    }
		    else    /* one step forwards */
		    {
			/* on last entry, clear the line */
			if (hiscnt == hisidx[histype])
			{
			    hiscnt = hislen;
			    break;
			}

			/* not on a history line, nothing to do */
			if (hiscnt == hislen)
			    break;
			if (hiscnt == hislen - 1)   /* wrap around */
			    hiscnt = 0;
			else
			    ++hiscnt;
		    }
		    if (hiscnt < 0 || history[histype][hiscnt].hisstr == NULL)
		    {
			hiscnt = i;
			break;
		    }
		    if ((c != K_UP && c != K_DOWN) || hiscnt == i
			    || STRNCMP(history[histype][hiscnt].hisstr,
						    lookfor, (size_t)j) == 0)
			break;
		}

		if (hiscnt != i)	/* jumped to other entry */
		{
		    char_u	*p;
		    int		len;
		    int		old_firstc;

		    vim_free(ccline.cmdbuff);
		    if (hiscnt == hislen)
			p = lookfor;	/* back to the old one */
		    else
			p = history[histype][hiscnt].hisstr;

		    if (histype == HIST_SEARCH
			    && p != lookfor
			    && (old_firstc = p[STRLEN(p) + 1]) != firstc)
		    {
			/* Correct for the separator character used when
			 * adding the history entry vs the one used now.
			 * First loop: count length.
			 * Second loop: copy the characters. */
			for (i = 0; i <= 1; ++i)
			{
			    len = 0;
			    for (j = 0; p[j] != NUL; ++j)
			    {
				/* Replace old sep with new sep, unless it is
				 * escaped. */
				if (p[j] == old_firstc
					      && (j == 0 || p[j - 1] != '\\'))
				{
				    if (i > 0)
					ccline.cmdbuff[len] = firstc;
				}
				else
				{
				    /* Escape new sep, unless it is already
				     * escaped. */
				    if (p[j] == firstc
					      && (j == 0 || p[j - 1] != '\\'))
				    {
					if (i > 0)
					    ccline.cmdbuff[len] = '\\';
					++len;
				    }
				    if (i > 0)
					ccline.cmdbuff[len] = p[j];
				}
				++len;
			    }
			    if (i == 0)
			    {
				alloc_cmdbuff(len);
				if (ccline.cmdbuff == NULL)
				    goto returncmd;
			    }
			}
			ccline.cmdbuff[len] = NUL;
		    }
		    else
		    {
			alloc_cmdbuff((int)STRLEN(p));
			if (ccline.cmdbuff == NULL)
			    goto returncmd;
			STRCPY(ccline.cmdbuff, p);
		    }

		    ccline.cmdpos = ccline.cmdlen = (int)STRLEN(ccline.cmdbuff);
		    redrawcmd();
		    goto cmdline_changed;
		}
		beep_flush();
		goto cmdline_not_changed;
#endif

	case Ctrl_V:
	case Ctrl_Q:
#ifdef FEAT_MOUSE
		ignore_drag_release = TRUE;
#endif
		putcmdline('^', TRUE);
		c = get_literal();	    /* get next (two) character(s) */
		do_abbr = FALSE;	    /* don't do abbreviation now */
#ifdef FEAT_MBYTE
		/* may need to remove ^ when composing char was typed */
		if (enc_utf8 && utf_iscomposing(c) && !cmd_silent)
		{
		    draw_cmdline(ccline.cmdpos, ccline.cmdlen - ccline.cmdpos);
		    msg_putchar(' ');
		    cursorcmd();
		}
#endif
		break;

#ifdef FEAT_DIGRAPHS
	case Ctrl_K:
#ifdef FEAT_MOUSE
		ignore_drag_release = TRUE;
#endif
		putcmdline('?', TRUE);
#ifdef USE_ON_FLY_SCROLL
		dont_scroll = TRUE;	    /* disallow scrolling here */
#endif
		c = get_digraph(TRUE);
		if (c != NUL)
		    break;

		redrawcmd();
		goto cmdline_not_changed;
#endif /* FEAT_DIGRAPHS */

#ifdef FEAT_RIGHTLEFT
	case Ctrl__:	    /* CTRL-_: switch language mode */
		if (!p_ari)
		    break;
#ifdef FEAT_FKMAP
		if (p_altkeymap)
		{
		    cmd_fkmap = !cmd_fkmap;
		    if (cmd_fkmap)	/* in Farsi always in Insert mode */
			ccline.overstrike = FALSE;
		}
		else			    /* Hebrew is default */
#endif
		    cmd_hkmap = !cmd_hkmap;
		goto cmdline_not_changed;
#endif

	default:
#ifdef UNIX
		if (c == intr_char)
		{
		    gotesc = TRUE;	/* will free ccline.cmdbuff after
					   putting it in history */
		    goto returncmd;	/* back to Normal mode */
		}
#endif
		/*
		 * Normal character with no special meaning.  Just set mod_mask
		 * to 0x0 so that typing Shift-Space in the GUI doesn't enter
		 * the string <S-Space>.  This should only happen after ^V.
		 */
		if (!IS_SPECIAL(c))
		    mod_mask = 0x0;
		break;
	}
	/*
	 * End of switch on command line character.
	 * We come here if we have a normal character.
	 */

	if (do_abbr && (IS_SPECIAL(c) || !vim_iswordc(c)) && ccheck_abbr(
#ifdef FEAT_MBYTE
			/* Add ABBR_OFF for characters above 0x100, this is
			 * what check_abbr() expects. */
			(has_mbyte && c >= 0x100) ? (c + ABBR_OFF) :
#endif
									c))
	    goto cmdline_changed;

	/*
	 * put the character in the command line
	 */
	if (IS_SPECIAL(c) || mod_mask != 0)
	    put_on_cmdline(get_special_key_name(c, mod_mask), -1, TRUE);
	else
	{
#ifdef FEAT_MBYTE
	    if (has_mbyte)
	    {
		j = (*mb_char2bytes)(c, IObuff);
		IObuff[j] = NUL;	/* exclude composing chars */
		put_on_cmdline(IObuff, j, TRUE);
	    }
	    else
#endif
	    {
		IObuff[0] = c;
		put_on_cmdline(IObuff, 1, TRUE);
	    }
	}
	goto cmdline_changed;

/*
 * This part implements incremental searches for "/" and "?"
 * Jump to cmdline_not_changed when a character has been read but the command
 * line did not change. Then we only search and redraw if something changed in
 * the past.
 * Jump to cmdline_changed when the command line did change.
 * (Sorry for the goto's, I know it is ugly).
 */
cmdline_not_changed:
#ifdef FEAT_SEARCH_EXTRA
	if (!incsearch_postponed)
	    continue;
#endif

cmdline_changed:
#ifdef FEAT_SEARCH_EXTRA
	/*
	 * 'incsearch' highlighting.
	 */
	if (p_is && !cmd_silent && (firstc == '/' || firstc == '?'))
	{
	    /* if there is a character waiting, search and redraw later */
	    if (char_avail())
	    {
		incsearch_postponed = TRUE;
		continue;
	    }
	    incsearch_postponed = FALSE;
	    curwin->w_cursor = old_cursor;  /* start at old position */

	    /* If there is no command line, don't do anything */
	    if (ccline.cmdlen == 0)
		i = 0;
	    else
	    {
		cursor_off();		/* so the user knows we're busy */
		out_flush();
		++emsg_off;    /* So it doesn't beep if bad expr */
		i = do_search(NULL, firstc, ccline.cmdbuff, count,
			SEARCH_KEEP + SEARCH_OPT + SEARCH_NOOF + SEARCH_PEEK);
		--emsg_off;
		/* if interrupted while searching, behave like it failed */
		if (got_int)
		{
		    (void)vpeekc();	/* remove <C-C> from input stream */
		    got_int = FALSE;	/* don't abandon the command line */
		    i = 0;
		}
		else if (char_avail())
		    /* cancelled searching because a char was typed */
		    incsearch_postponed = TRUE;
	    }
	    if (i)
		highlight_match = TRUE;		/* highlight position */
	    else
		highlight_match = FALSE;	/* remove highlight */

	    /* first restore the old curwin values, so the screen is
	     * positioned in the same way as the actual search command */
	    curwin->w_leftcol = old_leftcol;
	    curwin->w_topline = old_topline;
# ifdef FEAT_DIFF
	    curwin->w_topfill = old_topfill;
# endif
	    curwin->w_botline = old_botline;
	    changed_cline_bef_curs();
	    update_topline();

	    if (i != 0)
	    {
		/*
		 * First move cursor to end of match, then to start.  This
		 * moves the whole match onto the screen when 'nowrap' is set.
		 */
		i = curwin->w_cursor.col;
		curwin->w_cursor.lnum += search_match_lines;
		curwin->w_cursor.col = search_match_endcol;
		validate_cursor();
		curwin->w_cursor.lnum -= search_match_lines;
		curwin->w_cursor.col = i;
	    }
	    validate_cursor();

	    update_screen(NOT_VALID);
	    msg_starthere();
	    redrawcmdline();
	    did_incsearch = TRUE;
	}
#else /* FEAT_SEARCH_EXTRA */
	;
#endif

#ifdef FEAT_RIGHTLEFT
	if (cmdmsg_rl
# ifdef FEAT_ARABIC
		|| p_arshape
# endif
		)
	    /* Always redraw the whole command line to fix shaping and
	     * right-left typing.  Not efficient, but it works. */
	    redrawcmd();
#endif
    }

returncmd:

#ifdef FEAT_RIGHTLEFT
    cmdmsg_rl = FALSE;
#endif

#ifdef FEAT_FKMAP
    cmd_fkmap = 0;
#endif

    ExpandCleanup(&xpc);

#ifdef FEAT_SEARCH_EXTRA
    if (did_incsearch)
    {
	curwin->w_cursor = old_cursor;
	curwin->w_curswant = old_curswant;
	curwin->w_leftcol = old_leftcol;
	curwin->w_topline = old_topline;
# ifdef FEAT_DIFF
	curwin->w_topfill = old_topfill;
# endif
	curwin->w_botline = old_botline;
	highlight_match = FALSE;
	validate_cursor();	/* needed for TAB */
	redraw_later(NOT_VALID);
    }
#endif

    if (ccline.cmdbuff != NULL)
    {
	/*
	 * Put line in history buffer (":" and "=" only when it was typed).
	 */
#ifdef FEAT_CMDHIST
	if (ccline.cmdlen && firstc != NUL
		&& (some_key_typed || histype == HIST_SEARCH))
	{
	    add_to_history(histype, ccline.cmdbuff, TRUE,
				       histype == HIST_SEARCH ? firstc : NUL);
	    if (firstc == ':')
	    {
		vim_free(new_last_cmdline);
		new_last_cmdline = vim_strsave(ccline.cmdbuff);
	    }
	}
#endif

	if (gotesc)	    /* abandon command line */
	{
	    vim_free(ccline.cmdbuff);
	    ccline.cmdbuff = NULL;
	    if (msg_scrolled == 0)
		compute_cmdrow();
	    MSG("");
	    redraw_cmdline = TRUE;
	}
    }

    /*
     * If the screen was shifted up, redraw the whole screen (later).
     * If the line is too long, clear it, so ruler and shown command do
     * not get printed in the middle of it.
     */
    msg_check();
    msg_scroll = save_msg_scroll;
    redir_off = FALSE;

    /* When the command line was typed, no need for a wait-return prompt. */
    if (some_key_typed)
	need_wait_return = FALSE;

    State = save_State;
#ifdef USE_IM_CONTROL
    if (b_im_ptr != NULL && *b_im_ptr != B_IMODE_LMAP)
	im_save_status(b_im_ptr);
    im_set_active(FALSE);
#endif
#ifdef FEAT_MOUSE
    setmouse();
#endif
#ifdef CURSOR_SHAPE
    ui_cursor_shape();		/* may show different cursor shape */
#endif

    return ccline.cmdbuff;
}

#if (defined(FEAT_CRYPT) || defined(FEAT_EVAL)) || defined(PROTO)
/*
 * Get a command line with a prompt.
 * This is prepared to be called recursively from getcmdline() (e.g. by
 * f_input() when evaluating an expression from CTRL-R =).
 * Returns the command line in allocated memory, or NULL.
 */
    char_u *
getcmdline_prompt(firstc, prompt, attr)
    int		firstc;
    char_u	*prompt;	/* command line prompt */
    int		attr;		/* attributes for prompt */
{
    char_u		*s;
    struct cmdline_info	save_ccline;
    int			msg_col_save = msg_col;

    save_ccline = ccline;
    ccline.cmdbuff = NULL;
    ccline.cmdprompt = prompt;
    ccline.cmdattr = attr;
    s = getcmdline(firstc, 1L, 0);
    ccline = save_ccline;
    /* Restore msg_col, the prompt from input() may have changed it. */
    msg_col = msg_col_save;

    return s;
}
#endif

    static int
cmdline_charsize(idx)
    int		idx;
{
#if defined(FEAT_CRYPT) || defined(FEAT_EVAL)
    if (cmdline_star > 0)	    /* showing '*', always 1 position */
	return 1;
#endif
    return ptr2cells(ccline.cmdbuff + idx);
}

/*
 * Compute the offset of the cursor on the command line for the prompt and
 * indent.
 */
    static void
set_cmdspos()
{
    if (ccline.cmdfirstc)
	ccline.cmdspos = 1 + ccline.cmdindent;
    else
	ccline.cmdspos = 0 + ccline.cmdindent;
}

/*
 * Compute the screen position for the cursor on the command line.
 */
    static void
set_cmdspos_cursor()
{
    int		i, m, c;

    set_cmdspos();
    if (KeyTyped)
	m = Columns * Rows;
    else
	m = MAXCOL;
    for (i = 0; i < ccline.cmdlen && i < ccline.cmdpos; ++i)
    {
	c = cmdline_charsize(i);
#ifdef FEAT_MBYTE
	/* Count ">" for double-wide multi-byte char that doesn't fit. */
	if (has_mbyte)
	    correct_cmdspos(i, c);
#endif
	/* If the cmdline doesn't fit, put cursor on last visible char. */
	if ((ccline.cmdspos += c) >= m)
	{
	    ccline.cmdpos = i - 1;
	    ccline.cmdspos -= c;
	    break;
	}
#ifdef FEAT_MBYTE
	if (has_mbyte)
	    i += (*mb_ptr2len_check)(ccline.cmdbuff + i) - 1;
#endif
    }
}

#ifdef FEAT_MBYTE
/*
 * Check if the character at "idx", which is "cells" wide, is a multi-byte
 * character that doesn't fit, so that a ">" must be displayed.
 */
    static void
correct_cmdspos(idx, cells)
    int		idx;
    int		cells;
{
    if ((*mb_ptr2len_check)(ccline.cmdbuff + idx) > 1
		&& (*mb_ptr2cells)(ccline.cmdbuff + idx) > 1
		&& ccline.cmdspos % Columns + cells > Columns)
	ccline.cmdspos++;
}
#endif

/*
 * Get an Ex command line for the ":" command.
 */
/* ARGSUSED */
    char_u *
getexline(c, dummy, indent)
    int		c;		/* normally ':', NUL for ":append" */
    void	*dummy;		/* cookie not used */
    int		indent;		/* indent for inside conditionals */
{
    /* When executing a register, remove ':' that's in front of each line. */
    if (exec_from_reg && vpeekc() == ':')
	(void)vgetc();
    return getcmdline(c, 1L, indent);
}

/*
 * Get an Ex command line for Ex mode.
 * In Ex mode we only use the OS supplied line editing features and no
 * mappings or abbreviations.
 */
/* ARGSUSED */
    char_u *
getexmodeline(c, dummy, indent)
    int		c;		/* normally ':', NUL for ":append" */
    void	*dummy;		/* cookie not used */
    int		indent;		/* indent for inside conditionals */
{
    garray_T		line_ga;
    int			len;
    int			off = 0;
    char_u		*p;
    int			finished = FALSE;
#if defined(FEAT_GUI) || defined(NO_COOKED_INPUT)
    int			startcol = 0;
    int			c1;
    int			escaped = FALSE;	/* CTRL-V typed */
    int			vcol = 0;
#endif

    /* Switch cursor on now.  This avoids that it happens after the "\n", which
     * confuses the system function that computes tabstops. */
    cursor_on();

    /* always start in column 0; write a newline if necessary */
    compute_cmdrow();
    if (msg_col)
	msg_putchar('\n');
    if (c == ':')
    {
	msg_putchar(':');
	while (indent-- > 0)
	    msg_putchar(' ');
#if defined(FEAT_GUI) || defined(NO_COOKED_INPUT)
	startcol = msg_col;
#endif
    }

    ga_init2(&line_ga, 1, 30);

    /*
     * Get the line, one character at a time.
     */
    got_int = FALSE;
    while (!got_int && !finished)
    {
	if (ga_grow(&line_ga, 40) == FAIL)
	    break;
	p = (char_u *)line_ga.ga_data + line_ga.ga_len;

	/* Get one character (inchar gets a third of maxlen characters!) */
	len = inchar(p + off, 3, -1L, 0);
	if (len < 0)
	    continue;	    /* end of input script reached */
	/* for a special character, we need at least three characters */
	if ((*p == K_SPECIAL || *p == CSI) && off + len < 3)
	{
	    off += len;
	    continue;
	}
	len += off;
	off = 0;

	/*
	 * When using the GUI, and for systems that don't have cooked input,
	 * handle line editing here.
	 */
#if defined(FEAT_GUI) || defined(NO_COOKED_INPUT)
# ifndef NO_COOKED_INPUT
	if (gui.in_use)
# endif
	{
	    if (got_int)
	    {
		msg_putchar('\n');
		break;
	    }

	    while (len > 0)
	    {
		c1 = *p++;
		--len;
		if ((c1 == K_SPECIAL
#  if !defined(NO_COOKED_INPUT) || defined(FEAT_GUI)
			    || c1 == CSI
#  endif
		    ) && len >= 2)
		{
		    c1 = TO_SPECIAL(p[0], p[1]);
		    p += 2;
		    len -= 2;
		}

		if (!escaped)
		{
		    /* CR typed means "enter", which is NL */
		    if (c1 == '\r')
			c1 = '\n';

		    if (c1 == BS || c1 == K_BS
				  || c1 == DEL || c1 == K_DEL || c1 == K_KDEL)
		    {
			if (line_ga.ga_len > 0)
			{
			    int		i, v;
			    char_u	*q;

			    --line_ga.ga_len;
			    ++line_ga.ga_room;
			    /* compute column that cursor should be in */
			    v = 0;
			    q = ((char_u *)line_ga.ga_data);
			    for (i = 0; i < line_ga.ga_len; ++i)
			    {
				if (*q == TAB)
				    v += 8 - v % 8;
				else
				    v += ptr2cells(q);
				++q;
			    }
			    /* erase characters to position cursor */
			    while (vcol > v)
			    {
				msg_putchar('\b');
				msg_putchar(' ');
				msg_putchar('\b');
				--vcol;
			    }
			}
			continue;
		    }

		    if (c1 == Ctrl_U)
		    {
			msg_col = startcol;
			msg_clr_eos();
			line_ga.ga_room += line_ga.ga_len;
			line_ga.ga_len = 0;
			continue;
		    }

		    if (c1 == Ctrl_V)
		    {
			escaped = TRUE;
			continue;
		    }
		}

		if (IS_SPECIAL(c1))
		    c1 = '?';
		((char_u *)line_ga.ga_data)[line_ga.ga_len] = c1;
		if (c1 == '\n')
		    msg_putchar('\n');
		else if (c1 == TAB)
		{
		    /* Don't use chartabsize(), 'ts' can be different */
		    do
		    {
			msg_putchar(' ');
		    } while (++vcol % 8);
		}
		else
		{
		    msg_outtrans_len(
			     ((char_u *)line_ga.ga_data) + line_ga.ga_len, 1);
		    vcol += char2cells(c1);
		}
		++line_ga.ga_len;
		--line_ga.ga_room;
		escaped = FALSE;
	    }
	    windgoto(msg_row, msg_col);
	}
# ifndef NO_COOKED_INPUT
	else
# endif
#endif
#ifndef NO_COOKED_INPUT
	{
	    line_ga.ga_len += len;
	    line_ga.ga_room -= len;
	}
#endif
	p = (char_u *)(line_ga.ga_data) + line_ga.ga_len;
	if (line_ga.ga_len && p[-1] == '\n')
	{
	    finished = TRUE;
	    --line_ga.ga_len;
	    --p;
	    *p = NUL;
	}
    }

    /* note that cursor has moved, because of the echoed <CR> */
    screen_down();
    /* make following messages go to the next line */
    msg_didout = FALSE;
    msg_col = 0;
    if (msg_row < Rows - 1)
	++msg_row;
    emsg_on_display = FALSE;		/* don't want ui_delay() */

    if (got_int)
	ga_clear(&line_ga);

    return (char_u *)line_ga.ga_data;
}

#ifdef CURSOR_SHAPE
/*
 * Return TRUE if ccline.overstrike is on.
 */
    int
cmdline_overstrike()
{
    return ccline.overstrike;
}

/*
 * Return TRUE if the cursor is at the end of the cmdline.
 */
    int
cmdline_at_end()
{
    return (ccline.cmdpos >= ccline.cmdlen);
}
#endif

#if (defined(FEAT_XIM) && defined(FEAT_GUI_GTK)) || defined(PROTO)
/*
 * Return the virtual column number at the current cursor position.
 * This is used by the IM code to obtain the start of the preedit string.
 */
    colnr_T
cmdline_getvcol_cursor()
{
    if (ccline.cmdbuff == NULL || ccline.cmdpos > ccline.cmdlen)
	return MAXCOL;

# ifdef FEAT_MBYTE
    if (has_mbyte)
    {
	colnr_T	col;
	int	i = 0;

	for (col = 0; i < ccline.cmdpos; ++col)
	    i += (*mb_ptr2len_check)(ccline.cmdbuff + i);

	return col;
    }
    else
# endif
	return ccline.cmdpos;
}
#endif

#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
/*
 * If part of the command line is an IM preedit string, redraw it with
 * IM feedback attributes.  The cursor position is restored after drawing.
 */
    static void
redrawcmd_preedit()
{
    if ((State & CMDLINE)
	    && xic != NULL
	    && im_get_status()
	    && !p_imdisable
	    && im_is_preediting())
    {
	int	cmdpos = 0;
	int	cmdspos;
	int	old_row;
	int	old_col;
	colnr_T	col;

	old_row = msg_row;
	old_col = msg_col;
	cmdspos = ((ccline.cmdfirstc) ? 1 : 0) + ccline.cmdindent;

# ifdef FEAT_MBYTE
	if (has_mbyte)
	{
	    for (col = 0; col < preedit_start_col
			  && cmdpos < ccline.cmdlen; ++col)
	    {
		cmdspos += (*mb_ptr2cells)(ccline.cmdbuff + cmdpos);
		cmdpos  += (*mb_ptr2len_check)(ccline.cmdbuff + cmdpos);
	    }
	}
	else
# endif
	{
	    cmdspos += preedit_start_col;
	    cmdpos  += preedit_start_col;
	}

	msg_row = cmdline_row + (cmdspos / (int)Columns);
	msg_col = cmdspos % (int)Columns;
	if (msg_row >= Rows)
	    msg_row = Rows - 1;

	for (col = 0; cmdpos < ccline.cmdlen; ++col)
	{
	    int char_len;
	    int char_attr;

	    char_attr = im_get_feedback_attr(col);
	    if (char_attr < 0)
		break; /* end of preedit string */

# ifdef FEAT_MBYTE
	    if (has_mbyte)
		char_len = (*mb_ptr2len_check)(ccline.cmdbuff + cmdpos);
	    else
# endif
		char_len = 1;

	    msg_outtrans_len_attr(ccline.cmdbuff + cmdpos, char_len, char_attr);
	    cmdpos += char_len;
	}

	msg_row = old_row;
	msg_col = old_col;
    }
}
#endif /* FEAT_XIM && FEAT_GUI_GTK */

/*
 * Allocate a new command line buffer.
 * Assigns the new buffer to ccline.cmdbuff and ccline.cmdbufflen.
 * Returns the new value of ccline.cmdbuff and ccline.cmdbufflen.
 */
    static void
alloc_cmdbuff(len)
    int	    len;
{
    /*
     * give some extra space to avoid having to allocate all the time
     */
    if (len < 80)
	len = 100;
    else
	len += 20;

    ccline.cmdbuff = alloc(len);    /* caller should check for out-of-memory */
    ccline.cmdbufflen = len;
}

/*
 * Re-allocate the command line to length len + something extra.
 * return FAIL for failure, OK otherwise
 */
    static int
realloc_cmdbuff(len)
    int	    len;
{
    char_u	*p;

    p = ccline.cmdbuff;
    alloc_cmdbuff(len);			/* will get some more */
    if (ccline.cmdbuff == NULL)		/* out of memory */
    {
	ccline.cmdbuff = p;		/* keep the old one */
	return FAIL;
    }
    mch_memmove(ccline.cmdbuff, p, (size_t)ccline.cmdlen + 1);
    vim_free(p);
    return OK;
}

/*
 * Draw part of the cmdline at the current cursor position.  But draw stars
 * when cmdline_star is TRUE.
 */
    static void
draw_cmdline(start, len)
    int		start;
    int		len;
{
#if defined(FEAT_CRYPT) || defined(FEAT_EVAL)
    int		i;

    if (cmdline_star > 0)
	for (i = 0; i < len; ++i)
	{
	    msg_putchar('*');
# ifdef FEAT_MBYTE
	    if (has_mbyte)
		i += (*mb_ptr2len_check)(ccline.cmdbuff + start + i) - 1;
# endif
	}
    else
#endif
#ifdef FEAT_ARABIC
	if (p_arshape && !p_tbidi && enc_utf8 && len > 0)
    {
	static char_u	*buf;
	static int	buflen = 0;
	char_u		*p;
	int		j;
	int		newlen = 0;
	int		mb_l;
	int		pc, pc1;
	int		prev_c = 0;
	int		prev_c1 = 0;
	int		u8c, u8c_c1, u8c_c2;
	int		nc = 0;
	int		dummy;

	/*
	 * Do arabic shaping into a temporary buffer.  This is very
	 * inefficient!
	 */
	if (len * 2 > buflen)
	{
	    /* Re-allocate the buffer.  We keep it around to avoid a lot of
	     * alloc()/free() calls. */
	    vim_free(buf);
	    buflen = len * 2;
	    buf = alloc(buflen);
	    if (buf == NULL)
		return;	/* out of memory */
	}

	for (j = start; j < start + len; j += mb_l)
	{
	    p = ccline.cmdbuff + j;
	    u8c = utfc_ptr2char_len(p, &u8c_c1, &u8c_c2, start + len - j);
	    mb_l = utfc_ptr2len_check_len(p, start + len - j);
	    if (ARABIC_CHAR(u8c))
	    {
		/* Do Arabic shaping. */
		if (cmdmsg_rl)
		{
		    /* displaying from right to left */
		    pc = prev_c;
		    pc1 = prev_c1;
		    prev_c1 = u8c_c1;
		    if (j + mb_l >= start + len)
			nc = NUL;
		    else
			nc = utf_ptr2char(p + mb_l);
		}
		else
		{
		    /* displaying from left to right */
		    if (j + mb_l >= start + len)
			pc = NUL;
		    else
			pc = utfc_ptr2char_len(p + mb_l, &pc1, &dummy,
						      start + len - j - mb_l);
		    nc = prev_c;
		}
		prev_c = u8c;

		u8c = arabic_shape(u8c, NULL, &u8c_c1, pc, pc1, nc);

		newlen += (*mb_char2bytes)(u8c, buf + newlen);
		if (u8c_c1 != 0)
		{
		    newlen += (*mb_char2bytes)(u8c_c1, buf + newlen);
		    if (u8c_c2 != 0)
			newlen += (*mb_char2bytes)(u8c_c2, buf + newlen);
		}
	    }
	    else
	    {
		prev_c = u8c;
		mch_memmove(buf + newlen, p, mb_l);
		newlen += mb_l;
	    }
	}

	msg_outtrans_len(buf, newlen);
    }
    else
#endif
	msg_outtrans_len(ccline.cmdbuff + start, len);
}

/*
 * Put a character on the command line.  Shifts the following text to the
 * right when "shift" is TRUE.  Used for CTRL-V, CTRL-K, etc.
 * "c" must be printable (fit in one display cell)!
 */
    void
putcmdline(c, shift)
    int		c;
    int		shift;
{
    if (cmd_silent)
	return;
    msg_no_more = TRUE;
    msg_putchar(c);
    if (shift)
	draw_cmdline(ccline.cmdpos, ccline.cmdlen - ccline.cmdpos);
    msg_no_more = FALSE;
    cursorcmd();
}

/*
 * Undo a putcmdline(c, FALSE).
 */
    void
unputcmdline()
{
    if (cmd_silent)
	return;
    msg_no_more = TRUE;
    if (ccline.cmdlen == ccline.cmdpos)
	msg_putchar(' ');
    else
	draw_cmdline(ccline.cmdpos, 1);
    msg_no_more = FALSE;
    cursorcmd();
}

/*
 * Put the given string, of the given length, onto the command line.
 * If len is -1, then STRLEN() is used to calculate the length.
 * If 'redraw' is TRUE then the new part of the command line, and the remaining
 * part will be redrawn, otherwise it will not.  If this function is called
 * twice in a row, then 'redraw' should be FALSE and redrawcmd() should be
 * called afterwards.
 */
    int
put_on_cmdline(str, len, redraw)
    char_u	*str;
    int		len;
    int		redraw;
{
    int		retval;
    int		i;
    int		m;
    int		c;

    if (len < 0)
	len = (int)STRLEN(str);

    /* Check if ccline.cmdbuff needs to be longer */
    if (ccline.cmdlen + len + 1 >= ccline.cmdbufflen)
	retval = realloc_cmdbuff(ccline.cmdlen + len);
    else
	retval = OK;
    if (retval == OK)
    {
	if (!ccline.overstrike)
	{
	    mch_memmove(ccline.cmdbuff + ccline.cmdpos + len,
					       ccline.cmdbuff + ccline.cmdpos,
				     (size_t)(ccline.cmdlen - ccline.cmdpos));
	    ccline.cmdlen += len;
	}
	else
	{
#ifdef FEAT_MBYTE
	    if (has_mbyte)
	    {
		/* Count nr of characters in the new string. */
		m = 0;
		for (i = 0; i < len; i += (*mb_ptr2len_check)(str + i))
		    ++m;
		/* Count nr of bytes in cmdline that are overwritten by these
		 * characters. */
		for (i = ccline.cmdpos; i < ccline.cmdlen && m > 0;
				 i += (*mb_ptr2len_check)(ccline.cmdbuff + i))
		    --m;
		if (i < ccline.cmdlen)
		{
		    mch_memmove(ccline.cmdbuff + ccline.cmdpos + len,
			    ccline.cmdbuff + i, (size_t)(ccline.cmdlen - i));
		    ccline.cmdlen += ccline.cmdpos + len - i;
		}
		else
		    ccline.cmdlen = ccline.cmdpos + len;
	    }
	    else
#endif
	    if (ccline.cmdpos + len > ccline.cmdlen)
		ccline.cmdlen = ccline.cmdpos + len;
	}
	mch_memmove(ccline.cmdbuff + ccline.cmdpos, str, (size_t)len);
	ccline.cmdbuff[ccline.cmdlen] = NUL;

#ifdef FEAT_MBYTE
	if (enc_utf8)
	{
	    /* When the inserted text starts with a composing character,
	     * backup to the character before it.  There could be two of them.
	     */
	    i = 0;
	    c = utf_ptr2char(ccline.cmdbuff + ccline.cmdpos);
	    while (ccline.cmdpos > 0 && utf_iscomposing(c))
	    {
		i = (*mb_head_off)(ccline.cmdbuff,
				      ccline.cmdbuff + ccline.cmdpos - 1) + 1;
		ccline.cmdpos -= i;
		len += i;
		c = utf_ptr2char(ccline.cmdbuff + ccline.cmdpos);
	    }
# ifdef FEAT_ARABIC
	    if (i == 0 && ccline.cmdpos > 0 && arabic_maycombine(c))
	    {
		/* Check the previous character for Arabic combining pair. */
		i = (*mb_head_off)(ccline.cmdbuff,
				      ccline.cmdbuff + ccline.cmdpos - 1) + 1;
		if (arabic_combine(utf_ptr2char(ccline.cmdbuff
						     + ccline.cmdpos - i), c))
		{
		    ccline.cmdpos -= i;
		    len += i;
		}
		else
		    i = 0;
	    }
# endif
	    if (i != 0)
	    {
		/* Also backup the cursor position. */
		i = ptr2cells(ccline.cmdbuff + ccline.cmdpos);
		ccline.cmdspos -= i;
		msg_col -= i;
		if (msg_col < 0)
		{
		    msg_col += Columns;
		    --msg_row;
		}
	    }
	}
#endif

	if (redraw && !cmd_silent)
	{
	    msg_no_more = TRUE;
	    i = cmdline_row;
	    draw_cmdline(ccline.cmdpos, ccline.cmdlen - ccline.cmdpos);
	    /* Avoid clearing the rest of the line too often. */
	    if (cmdline_row != i || ccline.overstrike)
		msg_clr_eos();
	    msg_no_more = FALSE;
	}
#ifdef FEAT_FKMAP
	/*
	 * If we are in Farsi command mode, the character input must be in
	 * Insert mode. So do not advance the cmdpos.
	 */
	if (!cmd_fkmap)
#endif
	{
	    if (KeyTyped)
		m = Columns * Rows;
	    else
		m = MAXCOL;
	    for (i = 0; i < len; ++i)
	    {
		c = cmdline_charsize(ccline.cmdpos);
#ifdef FEAT_MBYTE
		/* count ">" for a double-wide char that doesn't fit. */
		if (has_mbyte)
		    correct_cmdspos(ccline.cmdpos, c);
#endif
		/* Stop cursor at the end of the screen */
		if (ccline.cmdspos + c >= m)
		    break;
		ccline.cmdspos += c;
#ifdef FEAT_MBYTE
		if (has_mbyte)
		{
		    c = (*mb_ptr2len_check)(ccline.cmdbuff + ccline.cmdpos) - 1;
		    if (c > len - i - 1)
			c = len - i - 1;
		    ccline.cmdpos += c;
		    i += c;
		}
#endif
		++ccline.cmdpos;
	    }
	}
    }
    if (redraw)
	msg_check();
    return retval;
}

#ifdef FEAT_WILDMENU
/*
 * Delete characters on the command line, from "from" to the current
 * position.
 */
    static void
cmdline_del(from)
    int from;
{
    mch_memmove(ccline.cmdbuff + from, ccline.cmdbuff + ccline.cmdpos,
	    (size_t)(ccline.cmdlen - ccline.cmdpos + 1));
    ccline.cmdlen -= ccline.cmdpos - from;
    ccline.cmdpos = from;
}
#endif

/*
 * this fuction is called when the screen size changes and with incremental
 * search
 */
    void
redrawcmdline()
{
    if (cmd_silent)
	return;
    need_wait_return = FALSE;
    compute_cmdrow();
    redrawcmd();
    cursorcmd();
}

    static void
redrawcmdprompt()
{
    int		i;

    if (cmd_silent)
	return;
    if (ccline.cmdfirstc)
	msg_putchar(ccline.cmdfirstc);
    if (ccline.cmdprompt != NULL)
    {
	msg_puts_attr(ccline.cmdprompt, ccline.cmdattr);
	ccline.cmdindent = msg_col + (msg_row - cmdline_row) * Columns;
	/* do the reverse of set_cmdspos() */
	if (ccline.cmdfirstc)
	    --ccline.cmdindent;
    }
    else
	for (i = ccline.cmdindent; i > 0; --i)
	    msg_putchar(' ');
}

/*
 * Redraw what is currently on the command line.
 */
    void
redrawcmd()
{
    if (cmd_silent)
	return;

    msg_start();
    redrawcmdprompt();

    /* Don't use more prompt, truncate the cmdline if it doesn't fit. */
    msg_no_more = TRUE;
    draw_cmdline(0, ccline.cmdlen);
    msg_clr_eos();
    msg_no_more = FALSE;

    set_cmdspos_cursor();

    /*
     * An emsg() before may have set msg_scroll. This is used in normal mode,
     * in cmdline mode we can reset them now.
     */
    msg_scroll = FALSE;		/* next message overwrites cmdline */

    /* Typing ':' at the more prompt may set skip_redraw.  We don't want this
     * in cmdline mode */
    skip_redraw = FALSE;
}

    void
compute_cmdrow()
{
    if (exmode_active || msg_scrolled)
	cmdline_row = Rows - 1;
    else
	cmdline_row = W_WINROW(lastwin) + lastwin->w_height
						   + W_STATUS_HEIGHT(lastwin);
}

    static void
cursorcmd()
{
    if (cmd_silent)
	return;

#ifdef FEAT_RIGHTLEFT
    if (cmdmsg_rl)
    {
	msg_row = cmdline_row  + (ccline.cmdspos / (int)(Columns - 1));
	msg_col = (int)Columns - (ccline.cmdspos % (int)(Columns - 1)) - 1;
	if (msg_row <= 0)
	    msg_row = Rows - 1;
    }
    else
#endif
    {
	msg_row = cmdline_row + (ccline.cmdspos / (int)Columns);
	msg_col = ccline.cmdspos % (int)Columns;
	if (msg_row >= Rows)
	    msg_row = Rows - 1;
    }

    windgoto(msg_row, msg_col);
#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
    redrawcmd_preedit();
#endif
#ifdef MCH_CURSOR_SHAPE
    mch_update_cursor();
#endif
}

    void
gotocmdline(clr)
    int		    clr;
{
    msg_start();
#ifdef FEAT_RIGHTLEFT
    if (cmdmsg_rl)
	msg_col = Columns - 1;
    else
#endif
	msg_col = 0;	    /* always start in column 0 */
    if (clr)		    /* clear the bottom line(s) */
	msg_clr_eos();	    /* will reset clear_cmdline */
    windgoto(cmdline_row, 0);
}

/*
 * Check the word in front of the cursor for an abbreviation.
 * Called when the non-id character "c" has been entered.
 * When an abbreviation is recognized it is removed from the text with
 * backspaces and the replacement string is inserted, followed by "c".
 */
    static int
ccheck_abbr(c)
    int c;
{
    if (p_paste || no_abbr)	    /* no abbreviations or in paste mode */
	return FALSE;

    return check_abbr(c, ccline.cmdbuff, ccline.cmdpos, 0);
}

/*
 * Return FAIL if this is not an appropriate context in which to do
 * completion of anything, return OK if it is (even if there are no matches).
 * For the caller, this means that the character is just passed through like a
 * normal character (instead of being expanded).  This allows :s/^I^D etc.
 */
    static int
nextwild(xp, type, options)
    expand_T	*xp;
    int		type;
    int		options;	/* extra options for ExpandOne() */
{
    int		i, j;
    char_u	*p1;
    char_u	*p2;
    int		oldlen;
    int		difflen;
    int		v;

    if (xp->xp_numfiles == -1)
    {
	set_expand_context(xp);
	cmd_showtail = expand_showtail(xp);
    }

    if (xp->xp_context == EXPAND_UNSUCCESSFUL)
    {
	beep_flush();
	return OK;  /* Something illegal on command line */
    }
    if (xp->xp_context == EXPAND_NOTHING)
    {
	/* Caller can use the character as a normal char instead */
	return FAIL;
    }

    MSG_PUTS("...");	    /* show that we are busy */
    out_flush();

    i = (int)(xp->xp_pattern - ccline.cmdbuff);
    oldlen = ccline.cmdpos - i;

    if (type == WILD_NEXT || type == WILD_PREV)
    {
	/*
	 * Get next/previous match for a previous expanded pattern.
	 */
	p2 = ExpandOne(xp, NULL, NULL, 0, type);
    }
    else
    {
	/*
	 * Translate string into pattern and expand it.
	 */
	if ((p1 = addstar(&ccline.cmdbuff[i], oldlen, xp->xp_context)) == NULL)
	    p2 = NULL;
	else
	{
	    p2 = ExpandOne(xp, p1, vim_strnsave(&ccline.cmdbuff[i], oldlen),
		    WILD_HOME_REPLACE|WILD_ADD_SLASH|WILD_SILENT|WILD_ESCAPE
							      |options, type);
	    vim_free(p1);
	    /* longest match: make sure it is not shorter (happens with :help */
	    if (p2 != NULL && type == WILD_LONGEST)
	    {
		for (j = 0; j < oldlen; ++j)
		     if (ccline.cmdbuff[i + j] == '*'
			     || ccline.cmdbuff[i + j] == '?')
			 break;
		if ((int)STRLEN(p2) < j)
		{
		    vim_free(p2);
		    p2 = NULL;
		}
	    }
	}
    }

    if (p2 != NULL && !got_int)
    {
	difflen = (int)STRLEN(p2) - oldlen;
	if (ccline.cmdlen + difflen > ccline.cmdbufflen - 4)
	{
	    v = realloc_cmdbuff(ccline.cmdlen + difflen);
	    xp->xp_pattern = ccline.cmdbuff + i;
	}
	else
	    v = OK;
	if (v == OK)
	{
	    vim_strncpy(&ccline.cmdbuff[ccline.cmdpos + difflen],
					       &ccline.cmdbuff[ccline.cmdpos],
		    ccline.cmdlen - ccline.cmdpos + 1);
	    STRNCPY(&ccline.cmdbuff[i], p2, STRLEN(p2));
	    ccline.cmdlen += difflen;
	    ccline.cmdpos += difflen;
	}
    }
    vim_free(p2);

    redrawcmd();

    /* When expanding a ":map" command and no matches are found, assume that
     * the key is supposed to be inserted literally */
    if (xp->xp_context == EXPAND_MAPPINGS && p2 == NULL)
	return FAIL;

    if (xp->xp_numfiles <= 0 && p2 == NULL)
	beep_flush();
    else if (xp->xp_numfiles == 1)
	/* free expanded pattern */
	(void)ExpandOne(xp, NULL, NULL, 0, WILD_FREE);

    return OK;
}

/*
 * Do wildcard expansion on the string 'str'.
 * Chars that should not be expanded must be preceded with a backslash.
 * Return a pointer to alloced memory containing the new string.
 * Return NULL for failure.
 *
 * Results are cached in xp->xp_files and xp->xp_numfiles.
 *
 * mode = WILD_FREE:	    just free previously expanded matches
 * mode = WILD_EXPAND_FREE: normal expansion, do not keep matches
 * mode = WILD_EXPAND_KEEP: normal expansion, keep matches
 * mode = WILD_NEXT:	    use next match in multiple match, wrap to first
 * mode = WILD_PREV:	    use previous match in multiple match, wrap to first
 * mode = WILD_ALL:	    return all matches concatenated
 * mode = WILD_LONGEST:	    return longest matched part
 *
 * options = WILD_LIST_NOTFOUND:    list entries without a match
 * options = WILD_HOME_REPLACE:	    do home_replace() for buffer names
 * options = WILD_USE_NL:	    Use '\n' for WILD_ALL
 * options = WILD_NO_BEEP:	    Don't beep for multiple matches
 * options = WILD_ADD_SLASH:	    add a slash after directory names
 * options = WILD_KEEP_ALL:	    don't remove 'wildignore' entries
 * options = WILD_SILENT:	    don't print warning messages
 * options = WILD_ESCAPE:	    put backslash before special chars
 *
 * The variables xp->xp_context and xp->xp_backslash must have been set!
 */
    char_u *
ExpandOne(xp, str, orig, options, mode)
    expand_T	*xp;
    char_u	*str;
    char_u	*orig;	    /* allocated copy of original of expanded string */
    int		options;
    int		mode;
{
    char_u	*ss = NULL;
    static int	findex;
    static char_u *orig_save = NULL;	/* kept value of orig */
    int		i;
    long_u	len;
    int		non_suf_match;		/* number without matching suffix */

    /*
     * first handle the case of using an old match
     */
    if (mode == WILD_NEXT || mode == WILD_PREV)
    {
	if (xp->xp_numfiles > 0)
	{
	    if (mode == WILD_PREV)
	    {
		if (findex == -1)
		    findex = xp->xp_numfiles;
		--findex;
	    }
	    else    /* mode == WILD_NEXT */
		++findex;

	    /*
	     * When wrapping around, return the original string, set findex to
	     * -1.
	     */
	    if (findex < 0)
	    {
		if (orig_save == NULL)
		    findex = xp->xp_numfiles - 1;
		else
		    findex = -1;
	    }
	    if (findex >= xp->xp_numfiles)
	    {
		if (orig_save == NULL)
		    findex = 0;
		else
		    findex = -1;
	    }
#ifdef FEAT_WILDMENU
	    if (p_wmnu)
		win_redr_status_matches(xp, xp->xp_numfiles, xp->xp_files,
							findex, cmd_showtail);
#endif
	    if (findex == -1)
		return vim_strsave(orig_save);
	    return vim_strsave(xp->xp_files[findex]);
	}
	else
	    return NULL;
    }

/* free old names */
    if (xp->xp_numfiles != -1 && mode != WILD_ALL && mode != WILD_LONGEST)
    {
	FreeWild(xp->xp_numfiles, xp->xp_files);
	xp->xp_numfiles = -1;
	vim_free(orig_save);
	orig_save = NULL;
    }
    findex = 0;

    if (mode == WILD_FREE)	/* only release file name */
	return NULL;

    if (xp->xp_numfiles == -1)
    {
	vim_free(orig_save);
	orig_save = orig;

	/*
	 * Do the expansion.
	 */
	if (ExpandFromContext(xp, str, &xp->xp_numfiles, &xp->xp_files,
							     options) == FAIL)
	{
#ifdef FNAME_ILLEGAL
	    /* Illegal file name has been silently skipped.  But when there
	     * are wildcards, the real problem is that there was no match,
	     * causing the pattern to be added, which has illegal characters.
	     */
	    if (!(options & WILD_SILENT) && (options & WILD_LIST_NOTFOUND))
		EMSG2(_(e_nomatch2), str);
#endif
	}
	else if (xp->xp_numfiles == 0)
	{
	    if (!(options & WILD_SILENT))
		EMSG2(_(e_nomatch2), str);
	}
	else
	{
	    /* Escape the matches for use on the command line. */
	    ExpandEscape(xp, str, xp->xp_numfiles, xp->xp_files, options);

	    /*
	     * Check for matching suffixes in file names.
	     */
	    if (mode != WILD_ALL && mode != WILD_LONGEST)
	    {
		if (xp->xp_numfiles)
		    non_suf_match = xp->xp_numfiles;
		else
		    non_suf_match = 1;
		if ((xp->xp_context == EXPAND_FILES
			    || xp->xp_context == EXPAND_DIRECTORIES)
			&& xp->xp_numfiles > 1)
		{
		    /*
		     * More than one match; check suffix.
		     * The files will have been sorted on matching suffix in
		     * expand_wildcards, only need to check the first two.
		     */
		    non_suf_match = 0;
		    for (i = 0; i < 2; ++i)
			if (match_suffix(xp->xp_files[i]))
			    ++non_suf_match;
		}
		if (non_suf_match != 1)
		{
		    /* Can we ever get here unless it's while expanding
		     * interactively?  If not, we can get rid of this all
		     * together. Don't really want to wait for this message
		     * (and possibly have to hit return to continue!).
		     */
		    if (!(options & WILD_SILENT))
			EMSG(_(e_toomany));
		    else if (!(options & WILD_NO_BEEP))
			beep_flush();
		}
		if (!(non_suf_match != 1 && mode == WILD_EXPAND_FREE))
		    ss = vim_strsave(xp->xp_files[0]);
	    }
	}
    }

    /* Find longest common part */
    if (mode == WILD_LONGEST && xp->xp_numfiles > 0)
    {
	for (len = 0; xp->xp_files[0][len]; ++len)
	{
	    for (i = 0; i < xp->xp_numfiles; ++i)
	    {
#ifdef CASE_INSENSITIVE_FILENAME
		if (xp->xp_context == EXPAND_DIRECTORIES
			|| xp->xp_context == EXPAND_FILES
			|| xp->xp_context == EXPAND_BUFFERS)
		{
		    if (TOLOWER_LOC(xp->xp_files[i][len]) !=
					    TOLOWER_LOC(xp->xp_files[0][len]))
			break;
		}
		else
#endif
		     if (xp->xp_files[i][len] != xp->xp_files[0][len])
		    break;
	    }
	    if (i < xp->xp_numfiles)
	    {
		if (!(options & WILD_NO_BEEP))
		    vim_beep();
		break;
	    }
	}
	ss = alloc((unsigned)len + 1);
	if (ss)
	{
	    STRNCPY(ss, xp->xp_files[0], len);
	    ss[len] = NUL;
	}
	findex = -1;			    /* next p_wc gets first one */
    }

    /* Concatenate all matching names */
    if (mode == WILD_ALL && xp->xp_numfiles > 0)
    {
	len = 0;
	for (i = 0; i < xp->xp_numfiles; ++i)
	    len += (long_u)STRLEN(xp->xp_files[i]) + 1;
	ss = lalloc(len, TRUE);
	if (ss != NULL)
	{
	    *ss = NUL;
	    for (i = 0; i < xp->xp_numfiles; ++i)
	    {
		STRCAT(ss, xp->xp_files[i]);
		if (i != xp->xp_numfiles - 1)
		    STRCAT(ss, (options & WILD_USE_NL) ? "\n" : " ");
	    }
	}
    }

    if (mode == WILD_EXPAND_FREE || mode == WILD_ALL)
	ExpandCleanup(xp);

    return ss;
}

/*
 * Prepare an expand structure for use.
 */
    void
ExpandInit(xp)
    expand_T	*xp;
{
    xp->xp_backslash = XP_BS_NONE;
    xp->xp_numfiles = -1;
    xp->xp_files = NULL;
}

/*
 * Cleanup an expand structure after use.
 */
    void
ExpandCleanup(xp)
    expand_T	*xp;
{
    if (xp->xp_numfiles >= 0)
    {
	FreeWild(xp->xp_numfiles, xp->xp_files);
	xp->xp_numfiles = -1;
    }
}

    void
ExpandEscape(xp, str, numfiles, files, options)
    expand_T	*xp;
    char_u	*str;
    int		numfiles;
    char_u	**files;
    int		options;
{
    int		i;
    char_u	*p;

    /*
     * May change home directory back to "~"
     */
    if (options & WILD_HOME_REPLACE)
	tilde_replace(str, numfiles, files);

    if (options & WILD_ESCAPE)
    {
	if (xp->xp_context == EXPAND_FILES
		|| xp->xp_context == EXPAND_BUFFERS
		|| xp->xp_context == EXPAND_DIRECTORIES)
	{
	    /*
	     * Insert a backslash into a file name before a space, \, %, #
	     * and wildmatch characters, except '~'.
	     */
	    for (i = 0; i < numfiles; ++i)
	    {
		/* for ":set path=" we need to escape spaces twice */
		if (xp->xp_backslash == XP_BS_THREE)
		{
		    p = vim_strsave_escaped(files[i], (char_u *)" ");
		    if (p != NULL)
		    {
			vim_free(files[i]);
			files[i] = p;
#if defined(BACKSLASH_IN_FILENAME) || defined(COLON_AS_PATHSEP)
			p = vim_strsave_escaped(files[i], (char_u *)" ");
			if (p != NULL)
			{
			    vim_free(files[i]);
			    files[i] = p;
			}
#endif
		    }
		}
#ifdef BACKSLASH_IN_FILENAME
		{
		    char_u	buf[20];
		    int		j = 0;

		    /* Don't escape '[' and '{' if they are in 'isfname'. */
		    for (p = PATH_ESC_CHARS; *p != NUL; ++p)
			if ((*p != '[' && *p != '{') || !vim_isfilec(*p))
			    buf[j++] = *p;
		    buf[j] = NUL;
		    p = vim_strsave_escaped(files[i], buf);
		}
#else
		p = vim_strsave_escaped(files[i], PATH_ESC_CHARS);
#endif
		if (p != NULL)
		{
		    vim_free(files[i]);
		    files[i] = p;
		}

		/* If 'str' starts with "\~", replace "~" at start of
		 * files[i] with "\~". */
		if (str[0] == '\\' && str[1] == '~' && files[i][0] == '~')
		{
		    p = alloc((unsigned)(STRLEN(files[i]) + 2));
		    if (p != NULL)
		    {
			p[0] = '\\';
			STRCPY(p + 1, files[i]);
			vim_free(files[i]);
			files[i] = p;
		    }
		}
	    }
	    xp->xp_backslash = XP_BS_NONE;
	}
	else if (xp->xp_context == EXPAND_TAGS)
	{
	    /*
	     * Insert a backslash before characters in a tag name that
	     * would terminate the ":tag" command.
	     */
	    for (i = 0; i < numfiles; ++i)
	    {
		p = vim_strsave_escaped(files[i], (char_u *)"\\|\"");
		if (p != NULL)
		{
		    vim_free(files[i]);
		    files[i] = p;
		}
	    }
	}
    }
}

/*
 * For each file name in files[num_files]:
 * If 'orig_pat' starts with "~/", replace the home directory with "~".
 */
    void
tilde_replace(orig_pat, num_files, files)
    char_u  *orig_pat;
    int	    num_files;
    char_u  **files;
{
    int	    i;
    char_u  *p;

    if (orig_pat[0] == '~' && vim_ispathsep(orig_pat[1]))
    {
	for (i = 0; i < num_files; ++i)
	{
	    p = home_replace_save(NULL, files[i]);
	    if (p != NULL)
	    {
		vim_free(files[i]);
		files[i] = p;
	    }
	}
    }
}

/*
 * Show all matches for completion on the command line.
 * Returns EXPAND_NOTHING when the character that triggered expansion should
 * be inserted like a normal character.
 */
/*ARGSUSED*/
    static int
showmatches(xp, wildmenu)
    expand_T	*xp;
    int		wildmenu;
{
#define L_SHOWFILE(m) (showtail ? sm_gettail(files_found[m]) : files_found[m])
    int		num_files;
    char_u	**files_found;
    int		i, j, k;
    int		maxlen;
    int		lines;
    int		columns;
    char_u	*p;
    int		lastlen;
    int		attr;
    int		showtail;

    if (xp->xp_numfiles == -1)
    {
	set_expand_context(xp);
	i = expand_cmdline(xp, ccline.cmdbuff, ccline.cmdpos,
						    &num_files, &files_found);
	showtail = expand_showtail(xp);
	if (i != EXPAND_OK)
	    return i;

    }
    else
    {
	num_files = xp->xp_numfiles;
	files_found = xp->xp_files;
	showtail = cmd_showtail;
    }

#ifdef FEAT_WILDMENU
    if (!wildmenu)
    {
#endif
	msg_didany = FALSE;		/* lines_left will be set */
	msg_start();			/* prepare for paging */
	msg_putchar('\n');
	out_flush();
	cmdline_row = msg_row;
	msg_didany = FALSE;		/* lines_left will be set again */
	msg_start();			/* prepare for paging */
#ifdef FEAT_WILDMENU
    }
#endif

    if (got_int)
	got_int = FALSE;	/* only int. the completion, not the cmd line */
#ifdef FEAT_WILDMENU
    else if (wildmenu)
	win_redr_status_matches(xp, num_files, files_found, 0, showtail);
#endif
    else
    {
	/* find the length of the longest file name */
	maxlen = 0;
	for (i = 0; i < num_files; ++i)
	{
	    if (!showtail && (xp->xp_context == EXPAND_FILES
			  || xp->xp_context == EXPAND_BUFFERS))
	    {
		home_replace(NULL, files_found[i], NameBuff, MAXPATHL, TRUE);
		j = vim_strsize(NameBuff);
	    }
	    else
		j = vim_strsize(L_SHOWFILE(i));
	    if (j > maxlen)
		maxlen = j;
	}

	if (xp->xp_context == EXPAND_TAGS_LISTFILES)
	    lines = num_files;
	else
	{
	    /* compute the number of columns and lines for the listing */
	    maxlen += 2;    /* two spaces between file names */
	    columns = ((int)Columns + 2) / maxlen;
	    if (columns < 1)
		columns = 1;
	    lines = (num_files + columns - 1) / columns;
	}

	attr = hl_attr(HLF_D);	/* find out highlighting for directories */

	if (xp->xp_context == EXPAND_TAGS_LISTFILES)
	{
	    MSG_PUTS_ATTR(_("tagname"), hl_attr(HLF_T));
	    msg_clr_eos();
	    msg_advance(maxlen - 3);
	    MSG_PUTS_ATTR(_(" kind file\n"), hl_attr(HLF_T));
	}

	/* list the files line by line */
	for (i = 0; i < lines; ++i)
	{
	    lastlen = 999;
	    for (k = i; k < num_files; k += lines)
	    {
		if (xp->xp_context == EXPAND_TAGS_LISTFILES)
		{
		    msg_outtrans_attr(files_found[k], hl_attr(HLF_D));
		    p = files_found[k] + STRLEN(files_found[k]) + 1;
		    msg_advance(maxlen + 1);
		    msg_puts(p);
		    msg_advance(maxlen + 3);
		    msg_puts_long_attr(p + 2, hl_attr(HLF_D));
		    break;
		}
		for (j = maxlen - lastlen; --j >= 0; )
		    msg_putchar(' ');
		if (xp->xp_context == EXPAND_FILES
					  || xp->xp_context == EXPAND_BUFFERS)
		{
			    /* highlight directories */
		    j = (mch_isdir(files_found[k]));
		    if (showtail)
			p = L_SHOWFILE(k);
		    else
		    {
			home_replace(NULL, files_found[k], NameBuff, MAXPATHL,
									TRUE);
			p = NameBuff;
		    }
		}
		else
		{
		    j = FALSE;
		    p = L_SHOWFILE(k);
		}
		lastlen = msg_outtrans_attr(p, j ? attr : 0);
	    }
	    if (msg_col > 0)	/* when not wrapped around */
	    {
		msg_clr_eos();
		msg_putchar('\n');
	    }
	    out_flush();		    /* show one line at a time */
	    if (got_int)
	    {
		got_int = FALSE;
		break;
	    }
	}

	/*
	 * we redraw the command below the lines that we have just listed
	 * This is a bit tricky, but it saves a lot of screen updating.
	 */
	cmdline_row = msg_row;	/* will put it back later */
    }

    if (xp->xp_numfiles == -1)
	FreeWild(num_files, files_found);

    return EXPAND_OK;
}

/*
 * Private gettail for showmatches() (and win_redr_status_matches()):
 * Find tail of file name path, but ignore trailing "/".
 */
    char_u *
sm_gettail(s)
    char_u	*s;
{
    char_u	*p;
    char_u	*t = s;
    int		had_sep = FALSE;

    for (p = s; *p != NUL; )
    {
	if (vim_ispathsep(*p)
#ifdef BACKSLASH_IN_FILENAME
		&& !rem_backslash(p)
#endif
	   )
	    had_sep = TRUE;
	else if (had_sep)
	{
	    t = p;
	    had_sep = FALSE;
	}
#ifdef FEAT_MBYTE
	if (has_mbyte)
	    p += (*mb_ptr2len_check)(p);
	else
#endif
	    ++p;
    }
    return t;
}

/*
 * Return TRUE if we only need to show the tail of completion matches.
 * When not completing file names or there is a wildcard in the path FALSE is
 * returned.
 */
    static int
expand_showtail(xp)
    expand_T	*xp;
{
    char_u	*s;
    char_u	*end;

    /* When not completing file names a "/" may mean something different. */
    if (xp->xp_context != EXPAND_FILES && xp->xp_context != EXPAND_DIRECTORIES)
	return FALSE;

    end = gettail(xp->xp_pattern);
    if (end == xp->xp_pattern)		/* there is no path separator */
	return FALSE;

    for (s = xp->xp_pattern; s < end; s++)
    {
	/* Skip escaped wildcards.  Only when the backslash is not a path
	 * separator, on DOS the '*' "path\*\file" must not be skipped. */
	if (rem_backslash(s))
	    ++s;
	else if (vim_strchr((char_u *)"*?[", *s) != NULL)
	    return FALSE;
    }
    return TRUE;
}

/*
 * Prepare a string for expansion.
 * When expanding file names: The string will be used with expand_wildcards().
 * Copy the file name into allocated memory and add a '*' at the end.
 * When expanding other names: The string will be used with regcomp().  Copy
 * the name into allocated memory and prepend "^".
 */
    char_u *
addstar(fname, len, context)
    char_u	*fname;
    int		len;
    int		context;	/* EXPAND_FILES etc. */
{
    char_u	*retval;
    int		i, j;
    int		new_len;
    char_u	*tail;

    if (context != EXPAND_FILES && context != EXPAND_DIRECTORIES)
    {
	/*
	 * Matching will be done internally (on something other than files).
	 * So we convert the file-matching-type wildcards into our kind for
	 * use with vim_regcomp().  First work out how long it will be:
	 */

	/* For help tags the translation is done in find_help_tags().
	 * For a tag pattern starting with "/" no translation is needed. */
	if (context == EXPAND_HELP
		|| context == EXPAND_COLORS
		|| context == EXPAND_COMPILER
		|| (context == EXPAND_TAGS && fname[0] == '/'))
	    retval = vim_strnsave(fname, len);
	else
	{
	    new_len = len + 2;		/* +2 for '^' at start, NUL at end */
	    for (i = 0; i < len; i++)
	    {
		if (fname[i] == '*' || fname[i] == '~')
		    new_len++;		/* '*' needs to be replaced by ".*"
					   '~' needs to be replaced by "\~" */

		/* Buffer names are like file names.  "." should be literal */
		if (context == EXPAND_BUFFERS && fname[i] == '.')
		    new_len++;		/* "." becomes "\." */

		/* Custom expansion takes care of special things, match
		 * backslashes literally (perhaps also for other types?) */
		if (context == EXPAND_USER_DEFINED && fname[i] == '\\')
		    new_len++;		/* '\' becomes "\\" */
	    }
	    retval = alloc(new_len);
	    if (retval != NULL)
	    {
		retval[0] = '^';
		j = 1;
		for (i = 0; i < len; i++, j++)
		{
		    /* Skip backslash.  But why?  At least keep it for custom
		     * expansion. */
		    if (context != EXPAND_USER_DEFINED
					    && fname[i] == '\\' && ++i == len)
			break;

		    switch (fname[i])
		    {
			case '*':   retval[j++] = '.';
				    break;
			case '~':   retval[j++] = '\\';
				    break;
			case '?':   retval[j] = '.';
				    continue;
			case '.':   if (context == EXPAND_BUFFERS)
					retval[j++] = '\\';
				    break;
			case '\\':  if (context == EXPAND_USER_DEFINED)
					retval[j++] = '\\';
				    break;
		    }
		    retval[j] = fname[i];
		}
		retval[j] = NUL;
	    }
	}
    }
    else
    {
	retval = alloc(len + 4);
	if (retval != NULL)
	{
	    STRNCPY(retval, fname, len);
	    retval[len] = NUL;

	    /*
	     * Don't add a star to ~, ~user, $var or `cmd`.
	     * ~ would be at the start of the file name, but not the tail.
	     * $ could be anywhere in the tail.
	     * ` could be anywhere in the file name.
	     */
	    tail = gettail(retval);
	    if ((*retval != '~' || tail != retval)
		    && vim_strchr(tail, '$') == NULL
		    && vim_strchr(retval, '`') == NULL)
		retval[len++] = '*';
	    retval[len] = NUL;
	}
    }
    return retval;
}

/*
 * Must parse the command line so far to work out what context we are in.
 * Completion can then be done based on that context.
 * This routine sets the variables:
 *  xp->xp_pattern	    The start of the pattern to be expanded within
 *				the command line (ends at the cursor).
 *  xp->xp_context	    The type of thing to expand.  Will be one of:
 *
 *  EXPAND_UNSUCCESSFUL	    Used sometimes when there is something illegal on
 *			    the command line, like an unknown command.	Caller
 *			    should beep.
 *  EXPAND_NOTHING	    Unrecognised context for completion, use char like
 *			    a normal char, rather than for completion.	eg
 *			    :s/^I/
 *  EXPAND_COMMANDS	    Cursor is still touching the command, so complete
 *			    it.
 *  EXPAND_BUFFERS	    Complete file names for :buf and :sbuf commands.
 *  EXPAND_FILES	    After command with XFILE set, or after setting
 *			    with P_EXPAND set.	eg :e ^I, :w>>^I
 *  EXPAND_DIRECTORIES	    In some cases this is used instead of the latter
 *			    when we know only directories are of interest.  eg
 *			    :set dir=^I
 *  EXPAND_SETTINGS	    Complete variable names.  eg :set d^I
 *  EXPAND_BOOL_SETTINGS    Complete boolean variables only,  eg :set no^I
 *  EXPAND_TAGS		    Complete tags from the files in p_tags.  eg :ta a^I
 *  EXPAND_TAGS_LISTFILES   As above, but list filenames on ^D, after :tselect
 *  EXPAND_HELP		    Complete tags from the file 'helpfile'/tags
 *  EXPAND_EVENTS	    Complete event names
 *  EXPAND_SYNTAX	    Complete :syntax command arguments
 *  EXPAND_HIGHLIGHT	    Complete highlight (syntax) group names
 *  EXPAND_AUGROUP	    Complete autocommand group names
 *  EXPAND_USER_VARS	    Complete user defined variable names, eg :unlet a^I
 *  EXPAND_MAPPINGS	    Complete mapping and abbreviation names,
 *			      eg :unmap a^I , :cunab x^I
 *  EXPAND_FUNCTIONS	    Complete internal or user defined function names,
 *			      eg :call sub^I
 *  EXPAND_USER_FUNC	    Complete user defined function names, eg :delf F^I
 *  EXPAND_EXPRESSION	    Complete internal or user defined function/variable
 *			    names in expressions, eg :while s^I
 *  EXPAND_ENV_VARS	    Complete environment variable names
 */
    static void
set_expand_context(xp)
    expand_T	*xp;
{
    /* only expansion for ':' and '>' commands */
    if (ccline.cmdfirstc != ':'
#ifdef FEAT_EVAL
	    && ccline.cmdfirstc != '>'
#endif
	    )
    {
	xp->xp_context = EXPAND_NOTHING;
	return;
    }
    set_cmd_context(xp, ccline.cmdbuff, ccline.cmdlen, ccline.cmdpos);
}

    void
set_cmd_context(xp, str, len, col)
    expand_T	*xp;
    char_u	*str;	    /* start of command line */
    int		len;	    /* length of command line (excl. NUL) */
    int		col;	    /* position of cursor */
{
    int		old_char = NUL;
    char_u	*nextcomm;

    /*
     * Avoid a UMR warning from Purify, only save the character if it has been
     * written before.
     */
    if (col < len)
	old_char = str[col];
    str[col] = NUL;
    nextcomm = str;
    while (nextcomm != NULL)
	nextcomm = set_one_cmd_context(xp, nextcomm);
    str[col] = old_char;
}

/*
 * Expand the command line "str" from context "xp".
 * "xp" must have been set by set_cmd_context().
 * xp->xp_pattern points into "str", to where the text that is to be expanded
 * starts.
 * Returns EXPAND_UNSUCCESSFUL when there is something illegal before the
 * cursor.
 * Returns EXPAND_NOTHING when there is nothing to expand, might insert the
 * key that triggered expansion literally.
 * Returns EXPAND_OK otherwise.
 */
    int
expand_cmdline(xp, str, col, matchcount, matches)
    expand_T	*xp;
    char_u	*str;		/* start of command line */
    int		col;		/* position of cursor */
    int		*matchcount;	/* return: nr of matches */
    char_u	***matches;	/* return: array of pointers to matches */
{
    char_u	*file_str = NULL;

    if (xp->xp_context == EXPAND_UNSUCCESSFUL)
    {
	beep_flush();
	return EXPAND_UNSUCCESSFUL;  /* Something illegal on command line */
    }
    if (xp->xp_context == EXPAND_NOTHING)
    {
	/* Caller can use the character as a normal char instead */
	return EXPAND_NOTHING;
    }

    /* add star to file name, or convert to regexp if not exp. files. */
    file_str = addstar(xp->xp_pattern,
			   (int)(str + col - xp->xp_pattern), xp->xp_context);
    if (file_str == NULL)
	return EXPAND_UNSUCCESSFUL;

    /* find all files that match the description */
    if (ExpandFromContext(xp, file_str, matchcount, matches,
					  WILD_ADD_SLASH|WILD_SILENT) == FAIL)
    {
	*matchcount = 0;
	*matches = NULL;
    }
    vim_free(file_str);

    return EXPAND_OK;
}

#ifdef FEAT_MULTI_LANG
/*
 * Cleanup matches for help tags: remove "@en" if "en" is the only language.
 */
static void	cleanup_help_tags __ARGS((int num_file, char_u **file));

    static void
cleanup_help_tags(num_file, file)
    int		num_file;
    char_u	**file;
{
    int		i, j;
    int		len;

    for (i = 0; i < num_file; ++i)
    {
	len = (int)STRLEN(file[i]) - 3;
	if (len > 0 && STRCMP(file[i] + len, "@en") == 0)
	{
	    /* Sorting on priority means the same item in another language may
	     * be anywhere.  Search all items for a match up to the "@en". */
	    for (j = 0; j < num_file; ++j)
		if (j != i
			&& (int)STRLEN(file[j]) == len + 3
			&& STRNCMP(file[i], file[j], len + 1) == 0)
		    break;
	    if (j == num_file)
		file[i][len] = NUL;
	}
    }
}
#endif

/*
 * Do the expansion based on xp->xp_context and "pat".
 */
    static int
ExpandFromContext(xp, pat, num_file, file, options)
    expand_T	*xp;
    char_u	*pat;
    int		*num_file;
    char_u	***file;
    int		options;
{
#ifdef FEAT_CMDL_COMPL
    regmatch_T	regmatch;
#endif
    int		ret;
    int		flags;

    flags = EW_DIR;	/* include directories */
    if (options & WILD_LIST_NOTFOUND)
	flags |= EW_NOTFOUND;
    if (options & WILD_ADD_SLASH)
	flags |= EW_ADDSLASH;
    if (options & WILD_KEEP_ALL)
	flags |= EW_KEEPALL;
    if (options & WILD_SILENT)
	flags |= EW_SILENT;

    if (xp->xp_context == EXPAND_FILES || xp->xp_context == EXPAND_DIRECTORIES)
    {
	/*
	 * Expand file or directory names.
	 */
	int	free_pat = FALSE;
	int	i;

	/* for ":set path=" and ":set tags=" halve backslashes for escaped
	 * space */
	if (xp->xp_backslash != XP_BS_NONE)
	{
	    free_pat = TRUE;
	    pat = vim_strsave(pat);
	    for (i = 0; pat[i]; ++i)
		if (pat[i] == '\\')
		{
		    if (xp->xp_backslash == XP_BS_THREE
			    && pat[i + 1] == '\\'
			    && pat[i + 2] == '\\'
			    && pat[i + 3] == ' ')
			STRCPY(pat + i, pat + i + 3);
		    if (xp->xp_backslash == XP_BS_ONE
			    && pat[i + 1] == ' ')
			STRCPY(pat + i, pat + i + 1);
		}
	}

	if (xp->xp_context == EXPAND_FILES)
	    flags |= EW_FILE;
	else
	    flags = (flags | EW_DIR) & ~EW_FILE;
	ret = expand_wildcards(1, &pat, num_file, file, flags);
	if (free_pat)
	    vim_free(pat);
	return ret;
    }

    *file = (char_u **)"";
    *num_file = 0;
    if (xp->xp_context == EXPAND_HELP)
    {
	if (find_help_tags(pat, num_file, file, FALSE) == OK)
	{
#ifdef FEAT_MULTI_LANG
	    cleanup_help_tags(*num_file, *file);
#endif
	    return OK;
	}
	return FAIL;
    }

#ifndef FEAT_CMDL_COMPL
    return FAIL;
#else
    if (xp->xp_context == EXPAND_OLD_SETTING)
	return ExpandOldSetting(num_file, file);
    if (xp->xp_context == EXPAND_BUFFERS)
	return ExpandBufnames(pat, num_file, file, options);
    if (xp->xp_context == EXPAND_TAGS
	    || xp->xp_context == EXPAND_TAGS_LISTFILES)
	return expand_tags(xp->xp_context == EXPAND_TAGS, pat, num_file, file);
    if (xp->xp_context == EXPAND_COLORS)
	return ExpandRTDir(pat, num_file, file, "colors");
    if (xp->xp_context == EXPAND_COMPILER)
	return ExpandRTDir(pat, num_file, file, "compiler");

    regmatch.regprog = vim_regcomp(pat, p_magic ? RE_MAGIC : 0);
    if (regmatch.regprog == NULL)
	return FAIL;

    /* set ignore-case according to p_ic, p_scs and pat */
    regmatch.rm_ic = ignorecase(pat);

    if (xp->xp_context == EXPAND_SETTINGS
	    || xp->xp_context == EXPAND_BOOL_SETTINGS)
	ret = ExpandSettings(xp, &regmatch, num_file, file);
    else if (xp->xp_context == EXPAND_MAPPINGS)
	ret = ExpandMappings(&regmatch, num_file, file);
# if defined(FEAT_USR_CMDS) && defined(FEAT_EVAL)
    else if (xp->xp_context == EXPAND_USER_DEFINED)
	ret = ExpandUserDefined(xp, &regmatch, num_file, file);
# endif
    else
    {
	static struct expgen
	{
	    int		context;
	    char_u	*((*func)__ARGS((expand_T *, int)));
	    int		ic;
	} tab[] =
	{
	    {EXPAND_COMMANDS, get_command_name, FALSE},
#ifdef FEAT_USR_CMDS
	    {EXPAND_USER_COMMANDS, get_user_commands, FALSE},
	    {EXPAND_USER_CMD_FLAGS, get_user_cmd_flags, FALSE},
	    {EXPAND_USER_NARGS, get_user_cmd_nargs, FALSE},
	    {EXPAND_USER_COMPLETE, get_user_cmd_complete, FALSE},
#endif
#ifdef FEAT_EVAL
	    {EXPAND_USER_VARS, get_user_var_name, FALSE},
	    {EXPAND_FUNCTIONS, get_function_name, FALSE},
	    {EXPAND_USER_FUNC, get_user_func_name, FALSE},
	    {EXPAND_EXPRESSION, get_expr_name, FALSE},
#endif
#ifdef FEAT_MENU
	    {EXPAND_MENUS, get_menu_name, FALSE},
	    {EXPAND_MENUNAMES, get_menu_names, FALSE},
#endif
#ifdef FEAT_SYN_HL
	    {EXPAND_SYNTAX, get_syntax_name, TRUE},
#endif
	    {EXPAND_HIGHLIGHT, get_highlight_name, TRUE},
#ifdef FEAT_AUTOCMD
	    {EXPAND_EVENTS, get_event_name, TRUE},
	    {EXPAND_AUGROUP, get_augroup_name, TRUE},
#endif
#if (defined(HAVE_LOCALE_H) || defined(X_LOCALE)) \
	&& (defined(FEAT_GETTEXT) || defined(FEAT_MBYTE))
	    {EXPAND_LANGUAGE, get_lang_arg, TRUE},
#endif
	    {EXPAND_ENV_VARS, get_env_name, TRUE},
	};
	int	i;

	/*
	 * Find a context in the table and call the ExpandGeneric() with the
	 * right function to do the expansion.
	 */
	ret = FAIL;
	for (i = 0; i < sizeof(tab) / sizeof(struct expgen); ++i)
	    if (xp->xp_context == tab[i].context)
	    {
		if (tab[i].ic)
		    regmatch.rm_ic = TRUE;
		ret = ExpandGeneric(xp, &regmatch, num_file, file, tab[i].func);
		break;
	    }
    }

    vim_free(regmatch.regprog);

    return ret;
#endif /* FEAT_CMDL_COMPL */
}

#if defined(FEAT_CMDL_COMPL) || defined(PROTO)
/*
 * Expand a list of names.
 *
 * Generic function for command line completion.  It calls a function to
 * obtain strings, one by one.	The strings are matched against a regexp
 * program.  Matching strings are copied into an array, which is returned.
 *
 * Returns OK when no problems encountered, FAIL for error (out of memory).
 */
    int
ExpandGeneric(xp, regmatch, num_file, file, func)
    expand_T	*xp;
    regmatch_T	*regmatch;
    int		*num_file;
    char_u	***file;
    char_u	*((*func)__ARGS((expand_T *, int)));
					  /* returns a string from the list */
{
    int		i;
    int		count = 0;
    int		loop;
    char_u	*str;

    /* do this loop twice:
     * loop == 0: count the number of matching names
     * loop == 1: copy the matching names into allocated memory
     */
    for (loop = 0; loop <= 1; ++loop)
    {
	for (i = 0; ; ++i)
	{
	    str = (*func)(xp, i);
	    if (str == NULL)	    /* end of list */
		break;
	    if (*str == NUL)	    /* skip empty strings */
		continue;

	    if (vim_regexec(regmatch, str, (colnr_T)0))
	    {
		if (loop)
		{
		    str = vim_strsave_escaped(str, (char_u *)" \t\\.");
		    (*file)[count] = str;
#ifdef FEAT_MENU
		    if (func == get_menu_names && str != NULL)
		    {
			/* test for separator added by get_menu_names() */
			str += STRLEN(str) - 1;
			if (*str == '\001')
			    *str = '.';
		    }
#endif
		}
		++count;
	    }
	}
	if (loop == 0)
	{
	    if (count == 0)
		return OK;
	    *num_file = count;
	    *file = (char_u **)alloc((unsigned)(count * sizeof(char_u *)));
	    if (*file == NULL)
	    {
		*file = (char_u **)"";
		return FAIL;
	    }
	    count = 0;
	}
    }
    return OK;
}

# if defined(FEAT_USR_CMDS) && defined(FEAT_EVAL)
/*
 * Expand names with a function defined by the user.
 */
    static int
ExpandUserDefined(xp, regmatch, num_file, file)
    expand_T	*xp;
    regmatch_T	*regmatch;
    int		*num_file;
    char_u	***file;
{
    char_u	*args[3];
    char_u	*all;
    char_u	*s;
    char_u	*e;
    char_u      keep;
    char_u      num[50];
    garray_T	ga;
    int		save_current_SID = current_SID;

    if (xp->xp_arg == NULL || xp->xp_arg[0] == '\0')
	return FAIL;
    *num_file = 0;
    *file = NULL;

    keep = ccline.cmdbuff[ccline.cmdlen];
    ccline.cmdbuff[ccline.cmdlen] = 0;
    sprintf((char *)num, "%d", ccline.cmdpos);
    args[0] = xp->xp_pattern;
    args[1] = ccline.cmdbuff;
    args[2] = num;

    current_SID = xp->xp_scriptID;
    all = call_vim_function(xp->xp_arg, 3, args, FALSE);
    current_SID = save_current_SID;
    ccline.cmdbuff[ccline.cmdlen] = keep;
    if (all == NULL)
	return FAIL;

    ga_init2(&ga, (int)sizeof(char *), 3);
    for (s = all; *s != NUL; s = e)
    {
	e = vim_strchr(s, '\n');
	if (e == NULL)
	    e = s + STRLEN(s);
	keep = *e;
	*e = 0;

	if (xp->xp_pattern[0] && vim_regexec(regmatch, s, (colnr_T)0) == 0)
	{
	    *e = keep;
	    if (*e != NUL)
		++e;
	    continue;
	}

	if (ga_grow(&ga, 1) == FAIL)
	    break;

	((char_u **)ga.ga_data)[ga.ga_len] = vim_strnsave(s, (int)(e - s));
	++ga.ga_len;
	--ga.ga_room;

	*e = keep;
	if (*e != NUL)
	    ++e;
    }
    vim_free(all);
    *file = ga.ga_data;
    *num_file = ga.ga_len;
    return OK;
}
#endif

/*
 * Expand color scheme names: 'runtimepath'/colors/{pat}.vim
 * or compiler names.
 */
    static int
ExpandRTDir(pat, num_file, file, dirname)
    char_u	*pat;
    int		*num_file;
    char_u	***file;
    char	*dirname;	/* "colors" or "compiler" */
{
    char_u	*all;
    char_u	*s;
    char_u	*e;
    garray_T	ga;

    *num_file = 0;
    *file = NULL;
    s = alloc((unsigned)(STRLEN(pat) + STRLEN(dirname) + 7));
    if (s == NULL)
	return FAIL;
    sprintf((char *)s, "%s/%s*.vim", dirname, pat);
    all = globpath(p_rtp, s);
    vim_free(s);
    if (all == NULL)
	return FAIL;

    ga_init2(&ga, (int)sizeof(char *), 3);
    for (s = all; *s != NUL; s = e)
    {
	e = vim_strchr(s, '\n');
	if (e == NULL)
	    e = s + STRLEN(s);
	if (ga_grow(&ga, 1) == FAIL)
	    break;
	if (e - 4 > s && STRNICMP(e - 4, ".vim", 4) == 0)
	{
	    for (s = e - 4; s > all; --s)
		if (*s == '\n' || vim_ispathsep(*s))
		    break;
	    ++s;
	    ((char_u **)ga.ga_data)[ga.ga_len] =
					    vim_strnsave(s, (int)(e - s - 4));
	    ++ga.ga_len;
	    --ga.ga_room;
	}
	if (*e != NUL)
	    ++e;
    }
    vim_free(all);
    *file = ga.ga_data;
    *num_file = ga.ga_len;
    return OK;
}

#endif

#if defined(FEAT_CMDL_COMPL) || defined(FEAT_EVAL) || defined(PROTO)
/*
 * Expand "file" for all comma-separated directories in "path".
 * Returns an allocated string with all matches concatenated, separated by
 * newlines.  Returns NULL for an error or no matches.
 */
    char_u *
globpath(path, file)
    char_u	*path;
    char_u	*file;
{
    expand_T	xpc;
    char_u	*buf;
    garray_T	ga;
    int		i;
    int		len;
    int		num_p;
    char_u	**p;
    char_u	*cur = NULL;

    buf = alloc(MAXPATHL);
    if (buf == NULL)
	return NULL;

    xpc.xp_context = EXPAND_FILES;
    xpc.xp_backslash = XP_BS_NONE;
    ga_init2(&ga, 1, 100);

    /* Loop over all entries in {path}. */
    while (*path != NUL)
    {
	/* Copy one item of the path to buf[] and concatenate the file name. */
	copy_option_part(&path, buf, MAXPATHL, ",");
	if (STRLEN(buf) + STRLEN(file) + 2 < MAXPATHL)
	{
	    add_pathsep(buf);
	    STRCAT(buf, file);
	    if (ExpandFromContext(&xpc, buf, &num_p, &p, WILD_SILENT) != FAIL
								 && num_p > 0)
	    {
		ExpandEscape(&xpc, buf, num_p, p, WILD_SILENT);
		for (len = 0, i = 0; i < num_p; ++i)
		    len += (long_u)STRLEN(p[i]) + 1;

		/* Concatenate new results to previous ones. */
		if (ga_grow(&ga, len) == OK)
		{
		    cur = (char_u *)ga.ga_data + ga.ga_len;
		    for (i = 0; i < num_p; ++i)
		    {
			STRCPY(cur, p[i]);
			cur += STRLEN(p[i]);
			*cur++ = '\n';
		    }
		    ga.ga_len += len;
		    ga.ga_room -= len;
		}
		FreeWild(num_p, p);
	    }
	}
    }
    if (cur != NULL)
	*--cur = 0; /* Replace trailing newline with NUL */

    vim_free(buf);
    return (char_u *)ga.ga_data;
}

#endif

#if defined(FEAT_CMDHIST) || defined(PROTO)

/*********************************
 *  Command line history stuff	 *
 *********************************/

/*
 * Translate a history character to the associated type number.
 */
    static int
hist_char2type(c)
    int	    c;
{
    if (c == ':')
	return HIST_CMD;
    if (c == '=')
	return HIST_EXPR;
    if (c == '@')
	return HIST_INPUT;
    if (c == '>')
	return HIST_DEBUG;
    return HIST_SEARCH;	    /* must be '?' or '/' */
}

/*
 * Table of history names.
 * These names are used in :history and various hist...() functions.
 * It is sufficient to give the significant prefix of a history name.
 */

static char *(history_names[]) =
{
    "cmd",
    "search",
    "expr",
    "input",
    "debug",
    NULL
};

/*
 * init_history() - Initialize the command line history.
 * Also used to re-allocate the history when the size changes.
 */
    static void
init_history()
{
    int		newlen;	    /* new length of history table */
    histentry_T	*temp;
    int		i;
    int		j;
    int		type;

    /*
     * If size of history table changed, reallocate it
     */
    newlen = (int)p_hi;
    if (newlen != hislen)			/* history length changed */
    {
	for (type = 0; type < HIST_COUNT; ++type)   /* adjust the tables */
	{
	    if (newlen)
	    {
		temp = (histentry_T *)lalloc(
				(long_u)(newlen * sizeof(histentry_T)), TRUE);
		if (temp == NULL)   /* out of memory! */
		{
		    if (type == 0)  /* first one: just keep the old length */
		    {
			newlen = hislen;
			break;
		    }
		    /* Already changed one table, now we can only have zero
		     * length for all tables. */
		    newlen = 0;
		    type = -1;
		    continue;
		}
	    }
	    else
		temp = NULL;
	    if (newlen == 0 || temp != NULL)
	    {
		if (hisidx[type] < 0)		/* there are no entries yet */
		{
		    for (i = 0; i < newlen; ++i)
		    {
			temp[i].hisnum = 0;
			temp[i].hisstr = NULL;
		    }
		}
		else if (newlen > hislen)	/* array becomes bigger */
		{
		    for (i = 0; i <= hisidx[type]; ++i)
			temp[i] = history[type][i];
		    j = i;
		    for ( ; i <= newlen - (hislen - hisidx[type]); ++i)
		    {
			temp[i].hisnum = 0;
			temp[i].hisstr = NULL;
		    }
		    for ( ; j < hislen; ++i, ++j)
			temp[i] = history[type][j];
		}
		else				/* array becomes smaller or 0 */
		{
		    j = hisidx[type];
		    for (i = newlen - 1; ; --i)
		    {
			if (i >= 0)		/* copy newest entries */
			    temp[i] = history[type][j];
			else			/* remove older entries */
			    vim_free(history[type][j].hisstr);
			if (--j < 0)
			    j = hislen - 1;
			if (j == hisidx[type])
			    break;
		    }
		    hisidx[type] = newlen - 1;
		}
		vim_free(history[type]);
		history[type] = temp;
	    }
	}
	hislen = newlen;
    }
}

/*
 * Check if command line 'str' is already in history.
 * If 'move_to_front' is TRUE, matching entry is moved to end of history.
 */
    static int
in_history(type, str, move_to_front)
    int	    type;
    char_u  *str;
    int	    move_to_front;	/* Move the entry to the front if it exists */
{
    int	    i;
    int	    last_i = -1;

    if (hisidx[type] < 0)
	return FALSE;
    i = hisidx[type];
    do
    {
	if (history[type][i].hisstr == NULL)
	    return FALSE;
	if (STRCMP(str, history[type][i].hisstr) == 0)
	{
	    if (!move_to_front)
		return TRUE;
	    last_i = i;
	    break;
	}
	if (--i < 0)
	    i = hislen - 1;
    } while (i != hisidx[type]);

    if (last_i >= 0)
    {
	str = history[type][i].hisstr;
	while (i != hisidx[type])
	{
	    if (++i >= hislen)
		i = 0;
	    history[type][last_i] = history[type][i];
	    last_i = i;
	}
	history[type][i].hisstr = str;
	history[type][i].hisnum = ++hisnum[type];
	return TRUE;
    }
    return FALSE;
}

/*
 * Convert history name (from table above) to its HIST_ equivalent.
 * When "name" is empty, return "cmd" history.
 * Returns -1 for unknown history name.
 */
    int
get_histtype(name)
    char_u	*name;
{
    int		i;
    int		len = (int)STRLEN(name);

    /* No argument: use current history. */
    if (len == 0)
	return hist_char2type(ccline.cmdfirstc);

    for (i = 0; history_names[i] != NULL; ++i)
	if (STRNICMP(name, history_names[i], len) == 0)
	    return i;

    if (vim_strchr((char_u *)":=@>?/", name[0]) != NULL && name[1] == NUL)
	return hist_char2type(name[0]);

    return -1;
}

static int	last_maptick = -1;	/* last seen maptick */

/*
 * Add the given string to the given history.  If the string is already in the
 * history then it is moved to the front.  "histype" may be one of he HIST_
 * values.
 */
    void
add_to_history(histype, new_entry, in_map, sep)
    int		histype;
    char_u	*new_entry;
    int		in_map;		/* consider maptick when inside a mapping */
    int		sep;		/* separator character used (search hist) */
{
    histentry_T	*hisptr;
    int		len;

    if (hislen == 0)		/* no history */
	return;

    /*
     * Searches inside the same mapping overwrite each other, so that only
     * the last line is kept.  Be careful not to remove a line that was moved
     * down, only lines that were added.
     */
    if (histype == HIST_SEARCH && in_map)
    {
	if (maptick == last_maptick)
	{
	    /* Current line is from the same mapping, remove it */
	    hisptr = &history[HIST_SEARCH][hisidx[HIST_SEARCH]];
	    vim_free(hisptr->hisstr);
	    hisptr->hisstr = NULL;
	    hisptr->hisnum = 0;
	    --hisnum[histype];
	    if (--hisidx[HIST_SEARCH] < 0)
		hisidx[HIST_SEARCH] = hislen - 1;
	}
	last_maptick = -1;
    }
    if (!in_history(histype, new_entry, TRUE))
    {
	if (++hisidx[histype] == hislen)
	    hisidx[histype] = 0;
	hisptr = &history[histype][hisidx[histype]];
	vim_free(hisptr->hisstr);

	/* Store the separator after the NUL of the string. */
	len = STRLEN(new_entry);
	hisptr->hisstr = vim_strnsave(new_entry, len + 2);
	if (hisptr->hisstr != NULL)
	    hisptr->hisstr[len + 1] = sep;

	hisptr->hisnum = ++hisnum[histype];
	if (histype == HIST_SEARCH && in_map)
	    last_maptick = maptick;
    }
}

#if defined(FEAT_EVAL) || defined(PROTO)

/*
 * Get identifier of newest history entry.
 * "histype" may be one of the HIST_ values.
 */
    int
get_history_idx(histype)
    int	    histype;
{
    if (hislen == 0 || histype < 0 || histype >= HIST_COUNT
		    || hisidx[histype] < 0)
	return -1;

    return history[histype][hisidx[histype]].hisnum;
}

/*
 * Get the current command line in allocated memory.
 * Only works when the command line is being edited.
 * Returns NULL when something is wrong.
 */
    char_u *
get_cmdline_str()
{
    if (ccline.cmdbuff == NULL || (State & CMDLINE) == 0)
	return NULL;
    return vim_strnsave(ccline.cmdbuff, ccline.cmdlen);
}

/*
 * Get the current command line position, counted in bytes.
 * Zero is the first position.
 * Only works when the command line is being edited.
 * Returns -1 when something is wrong.
 */
    int
get_cmdline_pos()
{
    if (ccline.cmdbuff == NULL || (State & CMDLINE) == 0)
	return -1;
    return ccline.cmdpos;
}

/*
 * Set the command line byte position to "pos".  Zero is the first position.
 * Only works when the command line is being edited.
 * Returns 1 when failed, 0 when OK.
 */
    int
set_cmdline_pos(pos)
    int		pos;
{
    if (ccline.cmdbuff == NULL || (State & CMDLINE) == 0)
	return 1;

    /* The position is not set directly but after CTRL-\ e or CTRL-R = has
     * changed the command line. */
    if (pos < 0)
	new_cmdpos = 0;
    else
	new_cmdpos = pos;
    return 0;
}

/*
 * Calculate history index from a number:
 *   num > 0: seen as identifying number of a history entry
 *   num < 0: relative position in history wrt newest entry
 * "histype" may be one of the HIST_ values.
 */
    static int
calc_hist_idx(histype, num)
    int		histype;
    int		num;
{
    int		i;
    histentry_T	*hist;
    int		wrapped = FALSE;

    if (hislen == 0 || histype < 0 || histype >= HIST_COUNT
		    || (i = hisidx[histype]) < 0 || num == 0)
	return -1;

    hist = history[histype];
    if (num > 0)
    {
	while (hist[i].hisnum > num)
	    if (--i < 0)
	    {
		if (wrapped)
		    break;
		i += hislen;
		wrapped = TRUE;
	    }
	if (hist[i].hisnum == num && hist[i].hisstr != NULL)
	    return i;
    }
    else if (-num <= hislen)
    {
	i += num + 1;
	if (i < 0)
	    i += hislen;
	if (hist[i].hisstr != NULL)
	    return i;
    }
    return -1;
}

/*
 * Get a history entry by its index.
 * "histype" may be one of the HIST_ values.
 */
    char_u *
get_history_entry(histype, idx)
    int	    histype;
    int	    idx;
{
    idx = calc_hist_idx(histype, idx);
    if (idx >= 0)
	return history[histype][idx].hisstr;
    else
	return (char_u *)"";
}

/*
 * Clear all entries of a history.
 * "histype" may be one of the HIST_ values.
 */
    int
clr_history(histype)
    int		histype;
{
    int		i;
    histentry_T	*hisptr;

    if (hislen != 0 && histype >= 0 && histype < HIST_COUNT)
    {
	hisptr = history[histype];
	for (i = hislen; i--;)
	{
	    vim_free(hisptr->hisstr);
	    hisptr->hisnum = 0;
	    hisptr++->hisstr = NULL;
	}
	hisidx[histype] = -1;	/* mark history as cleared */
	hisnum[histype] = 0;	/* reset identifier counter */
	return OK;
    }
    return FAIL;
}

/*
 * Remove all entries matching {str} from a history.
 * "histype" may be one of the HIST_ values.
 */
    int
del_history_entry(histype, str)
    int		histype;
    char_u	*str;
{
    regmatch_T	regmatch;
    histentry_T	*hisptr;
    int		idx;
    int		i;
    int		last;
    int		found = FALSE;

    regmatch.regprog = NULL;
    regmatch.rm_ic = FALSE;	/* always match case */
    if (hislen != 0
	    && histype >= 0
	    && histype < HIST_COUNT
	    && *str != NUL
	    && (idx = hisidx[histype]) >= 0
	    && (regmatch.regprog = vim_regcomp(str, RE_MAGIC + RE_STRING))
								      != NULL)
    {
	i = last = idx;
	do
	{
	    hisptr = &history[histype][i];
	    if (hisptr->hisstr == NULL)
		break;
	    if (vim_regexec(&regmatch, hisptr->hisstr, (colnr_T)0))
	    {
		found = TRUE;
		vim_free(hisptr->hisstr);
		hisptr->hisstr = NULL;
		hisptr->hisnum = 0;
	    }
	    else
	    {
		if (i != last)
		{
		    history[histype][last] = *hisptr;
		    hisptr->hisstr = NULL;
		    hisptr->hisnum = 0;
		}
		if (--last < 0)
		    last += hislen;
	    }
	    if (--i < 0)
		i += hislen;
	} while (i != idx);
	if (history[histype][idx].hisstr == NULL)
	    hisidx[histype] = -1;
    }
    vim_free(regmatch.regprog);
    return found;
}

/*
 * Remove an indexed entry from a history.
 * "histype" may be one of the HIST_ values.
 */
    int
del_history_idx(histype, idx)
    int	    histype;
    int	    idx;
{
    int	    i, j;

    i = calc_hist_idx(histype, idx);
    if (i < 0)
	return FALSE;
    idx = hisidx[histype];
    vim_free(history[histype][i].hisstr);

    /* When deleting the last added search string in a mapping, reset
     * last_maptick, so that the last added search string isn't deleted again.
     */
    if (histype == HIST_SEARCH && maptick == last_maptick && i == idx)
	last_maptick = -1;

    while (i != idx)
    {
	j = (i + 1) % hislen;
	history[histype][i] = history[histype][j];
	i = j;
    }
    history[histype][i].hisstr = NULL;
    history[histype][i].hisnum = 0;
    if (--i < 0)
	i += hislen;
    hisidx[histype] = i;
    return TRUE;
}

#endif /* FEAT_EVAL */

#if defined(FEAT_CRYPT) || defined(PROTO)
/*
 * Very specific function to remove the value in ":set key=val" from the
 * history.
 */
    void
remove_key_from_history()
{
    char_u	*p;
    int		i;

    i = hisidx[HIST_CMD];
    if (i < 0)
	return;
    p = history[HIST_CMD][i].hisstr;
    if (p != NULL)
	for ( ; *p; ++p)
	    if (STRNCMP(p, "key", 3) == 0 && !isalpha(p[3]))
	    {
		p = vim_strchr(p + 3, '=');
		if (p == NULL)
		    break;
		++p;
		for (i = 0; p[i] && !vim_iswhite(p[i]); ++i)
		    if (p[i] == '\\' && p[i + 1])
			++i;
		mch_memmove(p, p + i, STRLEN(p + i) + 1);
		--p;
	    }
}
#endif

#endif /* FEAT_CMDHIST */

#if defined(FEAT_QUICKFIX) || defined(FEAT_CMDHIST) || defined(PROTO)
/*
 * Get indices "num1,num2" that specify a range within a list (not a range of
 * text lines in a buffer!) from a string.  Used for ":history" and ":clist".
 * Returns OK if parsed successfully, otherwise FAIL.
 */
    int
get_list_range(str, num1, num2)
    char_u	**str;
    int		*num1;
    int		*num2;
{
    int		len;
    int		first = FALSE;
    long	num;

    *str = skipwhite(*str);
    if (**str == '-' || vim_isdigit(**str))  /* parse "from" part of range */
    {
	vim_str2nr(*str, NULL, &len, FALSE, FALSE, &num, NULL);
	*str += len;
	*num1 = (int)num;
	first = TRUE;
    }
    *str = skipwhite(*str);
    if (**str == ',')			/* parse "to" part of range */
    {
	*str = skipwhite(*str + 1);
	vim_str2nr(*str, NULL, &len, FALSE, FALSE, &num, NULL);
	if (len > 0)
	{
	    *num2 = (int)num;
	    *str = skipwhite(*str + len);
	}
	else if (!first)		/* no number given at all */
	    return FAIL;
    }
    else if (first)			/* only one number given */
	*num2 = *num1;
    return OK;
}
#endif

#if defined(FEAT_CMDHIST) || defined(PROTO)
/*
 * :history command - print a history
 */
    void
ex_history(eap)
    exarg_T	*eap;
{
    histentry_T	*hist;
    int		histype1 = HIST_CMD;
    int		histype2 = HIST_CMD;
    int		hisidx1 = 1;
    int		hisidx2 = -1;
    int		idx;
    int		i, j, k;
    char_u	*end;
    char_u	*arg = eap->arg;

    if (hislen == 0)
    {
	MSG(_("'history' option is zero"));
	return;
    }

    if (!(VIM_ISDIGIT(*arg) || *arg == '-' || *arg == ','))
    {
	end = arg;
	while (ASCII_ISALPHA(*end)
		|| vim_strchr((char_u *)":=@>/?", *end) != NULL)
	    end++;
	i = *end;
	*end = NUL;
	histype1 = get_histtype(arg);
	if (histype1 == -1)
	{
	    if (STRICMP(arg, "all") == 0)
	    {
		histype1 = 0;
		histype2 = HIST_COUNT-1;
	    }
	    else
	    {
		*end = i;
		EMSG(_(e_trailing));
		return;
	    }
	}
	else
	    histype2 = histype1;
	*end = i;
    }
    else
	end = arg;
    if (!get_list_range(&end, &hisidx1, &hisidx2) || *end != NUL)
    {
	EMSG(_(e_trailing));
	return;
    }

    for (; !got_int && histype1 <= histype2; ++histype1)
    {
	STRCPY(IObuff, "\n      #  ");
	STRCAT(STRCAT(IObuff, history_names[histype1]), " history");
	MSG_PUTS_TITLE(IObuff);
	idx = hisidx[histype1];
	hist = history[histype1];
	j = hisidx1;
	k = hisidx2;
	if (j < 0)
	    j = (-j > hislen) ? 0 : hist[(hislen+j+idx+1) % hislen].hisnum;
	if (k < 0)
	    k = (-k > hislen) ? 0 : hist[(hislen+k+idx+1) % hislen].hisnum;
	if (idx >= 0 && j <= k)
	    for (i = idx + 1; !got_int; ++i)
	    {
		if (i == hislen)
		    i = 0;
		if (hist[i].hisstr != NULL
			&& hist[i].hisnum >= j && hist[i].hisnum <= k)
		{
		    msg_putchar('\n');
		    sprintf((char *)IObuff, "%c%6d  ", i == idx ? '>' : ' ',
							      hist[i].hisnum);
		    if (vim_strsize(hist[i].hisstr) > (int)Columns - 10)
			trunc_string(hist[i].hisstr, IObuff + STRLEN(IObuff),
							   (int)Columns - 10);
		    else
			STRCAT(IObuff, hist[i].hisstr);
		    msg_outtrans(IObuff);
		    out_flush();
		}
		if (i == idx)
		    break;
	    }
    }
}
#endif

#if (defined(FEAT_VIMINFO) && defined(FEAT_CMDHIST)) || defined(PROTO)
static char_u **viminfo_history[HIST_COUNT] = {NULL, NULL, NULL, NULL};
static int	viminfo_hisidx[HIST_COUNT] = {0, 0, 0, 0};
static int	viminfo_hislen[HIST_COUNT] = {0, 0, 0, 0};
static int	viminfo_add_at_front = FALSE;

static int	hist_type2char __ARGS((int type, int use_question));

/*
 * Translate a history type number to the associated character.
 */
    static int
hist_type2char(type, use_question)
    int	    type;
    int	    use_question;	    /* use '?' instead of '/' */
{
    if (type == HIST_CMD)
	return ':';
    if (type == HIST_SEARCH)
    {
	if (use_question)
	    return '?';
	else
	    return '/';
    }
    if (type == HIST_EXPR)
	return '=';
    return '@';
}

/*
 * Prepare for reading the history from the viminfo file.
 * This allocates history arrays to store the read history lines.
 */
    void
prepare_viminfo_history(asklen)
    int	    asklen;
{
    int	    i;
    int	    num;
    int	    type;
    int	    len;

    init_history();
    viminfo_add_at_front = (asklen != 0);
    if (asklen > hislen)
	asklen = hislen;

    for (type = 0; type < HIST_COUNT; ++type)
    {
	/*
	 * Count the number of empty spaces in the history list.  If there are
	 * more spaces available than we request, then fill them up.
	 */
	for (i = 0, num = 0; i < hislen; i++)
	    if (history[type][i].hisstr == NULL)
		num++;
	len = asklen;
	if (num > len)
	    len = num;
	if (len <= 0)
	    viminfo_history[type] = NULL;
	else
	    viminfo_history[type] =
		   (char_u **)lalloc((long_u)(len * sizeof(char_u *)), FALSE);
	if (viminfo_history[type] == NULL)
	    len = 0;
	viminfo_hislen[type] = len;
	viminfo_hisidx[type] = 0;
    }
}

/*
 * Accept a line from the viminfo, store it in the history array when it's
 * new.
 */
    int
read_viminfo_history(virp)
    vir_T	*virp;
{
    int		type;
    long_u	len;
    char_u	*val;
    char_u	*p;

    type = hist_char2type(virp->vir_line[0]);
    if (viminfo_hisidx[type] < viminfo_hislen[type])
    {
	val = viminfo_readstring(virp, 1, TRUE);
	if (val != NULL && *val != NUL)
	{
	    if (!in_history(type, val + (type == HIST_SEARCH),
							viminfo_add_at_front))
	    {
		/* Need to re-allocate to append the separator byte. */
		len = STRLEN(val);
		p = lalloc(len + 2, TRUE);
		if (p != NULL)
		{
		    if (type == HIST_SEARCH)
		    {
			/* Search entry: Move the separator from the first
			 * column to after the NUL. */
			mch_memmove(p, val + 1, (size_t)len);
			p[len] = (*val == ' ' ? NUL : *val);
		    }
		    else
		    {
			/* Not a search entry: No separator in the viminfo
			 * file, add a NUL separator. */
			mch_memmove(p, val, (size_t)len + 1);
			p[len + 1] = NUL;
		    }
		    viminfo_history[type][viminfo_hisidx[type]++] = p;
		}
	    }
	}
	vim_free(val);
    }
    return viminfo_readline(virp);
}

    void
finish_viminfo_history()
{
    int idx;
    int i;
    int	type;

    for (type = 0; type < HIST_COUNT; ++type)
    {
	if (history[type] == NULL)
	    return;
	idx = hisidx[type] + viminfo_hisidx[type];
	if (idx >= hislen)
	    idx -= hislen;
	else if (idx < 0)
	    idx = hislen - 1;
	if (viminfo_add_at_front)
	    hisidx[type] = idx;
	else
	{
	    if (hisidx[type] == -1)
		hisidx[type] = hislen - 1;
	    do
	    {
		if (history[type][idx].hisstr != NULL)
		    break;
		if (++idx == hislen)
		    idx = 0;
	    } while (idx != hisidx[type]);
	    if (idx != hisidx[type] && --idx < 0)
		idx = hislen - 1;
	}
	for (i = 0; i < viminfo_hisidx[type]; i++)
	{
	    vim_free(history[type][idx].hisstr);
	    history[type][idx].hisstr = viminfo_history[type][i];
	    if (--idx < 0)
		idx = hislen - 1;
	}
	idx += 1;
	idx %= hislen;
	for (i = 0; i < viminfo_hisidx[type]; i++)
	{
	    history[type][idx++].hisnum = ++hisnum[type];
	    idx %= hislen;
	}
	vim_free(viminfo_history[type]);
	viminfo_history[type] = NULL;
    }
}

    void
write_viminfo_history(fp)
    FILE    *fp;
{
    int	    i;
    int	    type;
    int	    num_saved;
    char_u  *p;
    int	    c;

    init_history();
    if (hislen == 0)
	return;
    for (type = 0; type < HIST_COUNT; ++type)
    {
	num_saved = get_viminfo_parameter(hist_type2char(type, FALSE));
	if (num_saved == 0)
	    continue;
	if (num_saved < 0)  /* Use default */
	    num_saved = hislen;
	fprintf(fp, _("\n# %s History (newest to oldest):\n"),
			    type == HIST_CMD ? _("Command Line") :
			    type == HIST_SEARCH ? _("Search String") :
			    type == HIST_EXPR ?  _("Expression") :
					_("Input Line"));
	if (num_saved > hislen)
	    num_saved = hislen;
	i = hisidx[type];
	if (i >= 0)
	    while (num_saved--)
	    {
		p = history[type][i].hisstr;
		if (p != NULL)
		{
		    putc(hist_type2char(type, TRUE), fp);
		    /* For the search history: put the separator in the second
		     * column; use a space if there isn't one. */
		    if (type == HIST_SEARCH)
		    {
			c = p[STRLEN(p) + 1];
			putc(c == NUL ? ' ' : c, fp);
		    }
		    viminfo_writestring(fp, p);
		}
		if (--i < 0)
		    i = hislen - 1;
	    }
    }
}
#endif /* FEAT_VIMINFO */

#if defined(FEAT_FKMAP) || defined(PROTO)
/*
 * Write a character at the current cursor+offset position.
 * It is directly written into the command buffer block.
 */
    void
cmd_pchar(c, offset)
    int	    c, offset;
{
    if (ccline.cmdpos + offset >= ccline.cmdlen || ccline.cmdpos + offset < 0)
    {
	EMSG(_("E198: cmd_pchar beyond the command length"));
	return;
    }
    ccline.cmdbuff[ccline.cmdpos + offset] = (char_u)c;
    ccline.cmdbuff[ccline.cmdlen] = NUL;
}

    int
cmd_gchar(offset)
    int	    offset;
{
    if (ccline.cmdpos + offset >= ccline.cmdlen || ccline.cmdpos + offset < 0)
    {
	/*  EMSG(_("cmd_gchar beyond the command length")); */
	return NUL;
    }
    return (int)ccline.cmdbuff[ccline.cmdpos + offset];
}
#endif

#if defined(FEAT_CMDWIN) || defined(PROTO)
/*
 * Open a window on the current command line and history.  Allow editing in
 * the window.  Returns when the window is closed.
 * Returns:
 *	CR	 if the command is to be executed
 *	Ctrl_C	 if it is to be abandoned
 *	K_IGNORE if editing continues
 */
    static int
ex_window()
{
    struct cmdline_info	save_ccline;
    buf_T		*old_curbuf = curbuf;
    win_T		*old_curwin = curwin;
    buf_T		*bp;
    win_T		*wp;
    int			i;
    linenr_T		lnum;
    int			histtype;
    garray_T		winsizes;
    char_u		typestr[2];
    int			save_restart_edit = restart_edit;
    int			save_State = State;
    int			save_exmode = exmode_active;

    /* Can't do this recursively.  Can't do it when typing a password. */
    if (cmdwin_type != 0
# if defined(FEAT_CRYPT) || defined(FEAT_EVAL)
	    || cmdline_star > 0
# endif
	    )
    {
	beep_flush();
	return K_IGNORE;
    }

    /* Save current window sizes. */
    win_size_save(&winsizes);

# ifdef FEAT_AUTOCMD
    /* Don't execute autocommands while creating the window. */
    ++autocmd_block;
# endif
    /* Create a window for the command-line buffer. */
    if (win_split((int)p_cwh, WSP_BOT) == FAIL)
    {
	beep_flush();
	return K_IGNORE;
    }
    cmdwin_type = ccline.cmdfirstc;
    if (cmdwin_type == NUL)
	cmdwin_type = '-';

    /* Create the command-line buffer empty. */
    (void)do_ecmd(0, NULL, NULL, NULL, ECMD_ONE, ECMD_HIDE);
    (void)setfname(curbuf, (char_u *)"command-line", NULL, TRUE);
    set_option_value((char_u *)"bt", 0L, (char_u *)"nofile", OPT_LOCAL);
    set_option_value((char_u *)"swf", 0L, NULL, OPT_LOCAL);
    curbuf->b_p_ma = TRUE;
# ifdef FEAT_RIGHTLEFT
    curwin->w_p_rl = FALSE;
# endif
# ifdef FEAT_SCROLLBIND
    curwin->w_p_scb = FALSE;
# endif

# ifdef FEAT_AUTOCMD
    /* Do execute autocommands for setting the filetype (load syntax). */
    --autocmd_block;
# endif

    histtype = hist_char2type(ccline.cmdfirstc);
    if (histtype == HIST_CMD || histtype == HIST_DEBUG)
    {
	if (p_wc == TAB)
	{
	    add_map((char_u *)"<buffer> <Tab> <C-X><C-V>", INSERT);
	    add_map((char_u *)"<buffer> <Tab> a<C-X><C-V>", NORMAL);
	}
	set_option_value((char_u *)"ft", 0L, (char_u *)"vim", OPT_LOCAL);
    }

    /* Fill the buffer with the history. */
    init_history();
    if (hislen > 0)
    {
	i = hisidx[histtype];
	if (i >= 0)
	{
	    lnum = 0;
	    do
	    {
		if (++i == hislen)
		    i = 0;
		if (history[histtype][i].hisstr != NULL)
		    ml_append(lnum++, history[histtype][i].hisstr,
							   (colnr_T)0, FALSE);
	    }
	    while (i != hisidx[histtype]);
	}
    }

    /* Replace the empty last line with the current command-line and put the
     * cursor there. */
    ml_replace(curbuf->b_ml.ml_line_count, ccline.cmdbuff, TRUE);
    curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
    curwin->w_cursor.col = ccline.cmdpos;
    redraw_later(NOT_VALID);

    /* Save the command line info, can be used recursively. */
    save_ccline = ccline;
    ccline.cmdbuff = NULL;
    ccline.cmdprompt = NULL;

    /* No Ex mode here! */
    exmode_active = 0;

    State = NORMAL;
# ifdef FEAT_MOUSE
    setmouse();
# endif

# ifdef FEAT_AUTOCMD
    /* Trigger CmdwinEnter autocommands. */
    typestr[0] = cmdwin_type;
    typestr[1] = NUL;
    apply_autocmds(EVENT_CMDWINENTER, typestr, typestr, FALSE, curbuf);
# endif

    i = RedrawingDisabled;
    RedrawingDisabled = 0;

    /*
     * Call the main loop until <CR> or CTRL-C is typed.
     */
    cmdwin_result = 0;
    main_loop(TRUE);

    RedrawingDisabled = i;

# ifdef FEAT_AUTOCMD
    /* Trigger CmdwinLeave autocommands. */
    apply_autocmds(EVENT_CMDWINLEAVE, typestr, typestr, FALSE, curbuf);
# endif

    /* Restore the comand line info. */
    ccline = save_ccline;
    cmdwin_type = 0;

    exmode_active = save_exmode;

    /* Safety check: The old window or buffer was deleted: It's a a bug when
     * this happens! */
    if (!win_valid(old_curwin) || !buf_valid(old_curbuf))
    {
	cmdwin_result = Ctrl_C;
	EMSG(_("E199: Active window or buffer deleted"));
    }
    else
    {
# if defined(FEAT_AUTOCMD) && defined(FEAT_EVAL)
	/* autocmds may abort script processing */
	if (aborting() && cmdwin_result != K_IGNORE)
	    cmdwin_result = Ctrl_C;
# endif
	/* Set the new command line from the cmdline buffer. */
	vim_free(ccline.cmdbuff);
	if (cmdwin_result == K_XF1)		/* :qa! typed */
	{
	    ccline.cmdbuff = vim_strsave((char_u *)"qa!");
	    cmdwin_result = CAR;
	}
	else if (cmdwin_result == K_XF2)	/* :qa typed */
	{
	    ccline.cmdbuff = vim_strsave((char_u *)"qa");
	    cmdwin_result = CAR;
	}
	else
	    ccline.cmdbuff = vim_strsave(ml_get_curline());
	if (ccline.cmdbuff == NULL)
	    cmdwin_result = Ctrl_C;
	else
	{
	    ccline.cmdlen = (int)STRLEN(ccline.cmdbuff);
	    ccline.cmdbufflen = ccline.cmdlen + 1;
	    ccline.cmdpos = curwin->w_cursor.col;
	    if (ccline.cmdpos > ccline.cmdlen)
		ccline.cmdpos = ccline.cmdlen;
	    if (cmdwin_result == K_IGNORE)
	    {
		set_cmdspos_cursor();
		redrawcmd();
	    }
	}

# ifdef FEAT_AUTOCMD
	/* Don't execute autocommands while deleting the window. */
	++autocmd_block;
# endif
	wp = curwin;
	bp = curbuf;
	win_goto(old_curwin);
	win_close(wp, TRUE);
	close_buffer(NULL, bp, DOBUF_WIPE);

	/* Restore window sizes. */
	win_size_restore(&winsizes);

# ifdef FEAT_AUTOCMD
	--autocmd_block;
# endif
    }

    ga_clear(&winsizes);
    restart_edit = save_restart_edit;

    State = save_State;
# ifdef FEAT_MOUSE
    setmouse();
# endif

    return cmdwin_result;
}
#endif /* FEAT_CMDWIN */

/*
 * Used for commands that either take a simple command string argument, or:
 *	cmd << endmarker
 *	  {script}
 *	endmarker
 * Returns a pointer to allocated memory with {script} or NULL.
 */
    char_u *
script_get(eap, cmd)
    exarg_T	*eap;
    char_u	*cmd;
{
    char_u	*theline;
    char	*end_pattern = NULL;
    char	dot[] = ".";
    garray_T	ga;

    if (cmd[0] != '<' || cmd[1] != '<' || eap->getline == NULL)
	return NULL;

    ga_init2(&ga, 1, 0x400);

    if (cmd[2] != NUL)
	end_pattern = (char *)skipwhite(cmd + 2);
    else
	end_pattern = dot;

    for (;;)
    {
	theline = eap->getline(
#ifdef FEAT_EVAL
	    eap->cstack->cs_whilelevel > 0 ? -1 :
#endif
	    NUL, eap->cookie, 0);

	if (theline == NULL || STRCMP(end_pattern, theline) == 0)
	    break;

	ga_concat(&ga, theline);
	ga_append(&ga, '\n');
	vim_free(theline);
    }

    return (char_u *)ga.ga_data;
}
