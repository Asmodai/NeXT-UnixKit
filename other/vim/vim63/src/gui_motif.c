/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *				GUI/Motif support by Robert Webb
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/CascadeB.h>
#include <Xm/ScrollBar.h>
#include <Xm/MenuShell.h>
#include <Xm/DrawingA.h>
#if (XmVersion >= 1002)
# include <Xm/RepType.h>
#endif
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/ToggleBG.h>
#include <Xm/SeparatoG.h>

#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>

#include "vim.h"

#ifdef HAVE_X11_XPM_H
# include <X11/xpm.h>
#else
# ifdef HAVE_XM_XPMP_H
#  include <Xm/XpmP.h>
# endif
#endif

#if defined(FEAT_GUI_DIALOG) && defined(HAVE_XPM)
# include "../pixmaps/alert.xpm"
# include "../pixmaps/error.xpm"
# include "../pixmaps/generic.xpm"
# include "../pixmaps/info.xpm"
# include "../pixmaps/quest.xpm"
#endif

#define MOTIF_POPUP

extern Widget vimShell;

static Widget vimForm;
static Widget textAreaForm;
Widget textArea;
#ifdef FEAT_TOOLBAR
static Widget toolBarFrame;
static Widget toolBar;
#endif
#ifdef FEAT_FOOTER
static Widget footer;
#endif
#ifdef FEAT_MENU
# if (XmVersion >= 1002)
/* remember the last set value for the tearoff item */
static int tearoff_val = (int)XmTEAR_OFF_ENABLED;
# endif
static Widget menuBar;
#endif

static void scroll_cb __ARGS((Widget w, XtPointer client_data, XtPointer call_data));
#ifdef FEAT_TOOLBAR
# if 0
static void toolbar_enter_cb __ARGS((Widget, XtPointer, XEvent *, Boolean *));
static void toolbar_leave_cb __ARGS((Widget, XtPointer, XEvent *, Boolean *));
# endif
# ifdef FEAT_FOOTER
static void toolbarbutton_enter_cb __ARGS((Widget, XtPointer, XEvent *, Boolean *));
static void toolbarbutton_leave_cb __ARGS((Widget, XtPointer, XEvent *, Boolean *));
# endif
static void gui_mch_reset_focus __ARGS((void));
#endif
#ifdef FEAT_FOOTER
static int gui_mch_compute_footer_height __ARGS((void));
#endif
#ifdef WSDEBUG
static void attachDump(Widget, char *);
#endif

static void gui_motif_menu_colors __ARGS((Widget id));
static void gui_motif_scroll_colors __ARGS((Widget id));
#ifdef FEAT_MENU
static void gui_motif_menu_fontlist __ARGS((Widget id));
#endif

#if (XmVersion >= 1002)
# define STRING_TAG  XmFONTLIST_DEFAULT_TAG
#else
# define STRING_TAG  XmSTRING_DEFAULT_CHARSET
#endif

/*
 * Call-back routines.
 */

/* ARGSUSED */
    static void
scroll_cb(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data, call_data;
{
    scrollbar_T *sb;
    long	value;
    int		dragging;

    sb = gui_find_scrollbar((long)client_data);

    value = ((XmScrollBarCallbackStruct *)call_data)->value;
    dragging = (((XmScrollBarCallbackStruct *)call_data)->reason ==
							      (int)XmCR_DRAG);
    gui_drag_scrollbar(sb, value, dragging);
}


/*
 * End of call-back routines
 */

/*
 * Create all the motif widgets necessary.
 */
    void
gui_x11_create_widgets()
{
    /*
     * Start out by adding the configured border width into the border offset
     */
    gui.border_offset = gui.border_width;

    /*
     * Install the tearOffModel resource converter.
     */
#if (XmVersion >= 1002)
    XmRepTypeInstallTearOffModelConverter();
#endif

    /* Make sure the "Quit" menu entry of the window manager is ignored */
    XtVaSetValues(vimShell, XmNdeleteResponse, XmDO_NOTHING, NULL);

    vimForm = XtVaCreateManagedWidget("vimForm",
	xmFormWidgetClass, vimShell,
	XmNborderWidth, 0,
	XmNhighlightThickness, 0,
	XmNshadowThickness, 0,
	XmNmarginWidth, 0,
	XmNmarginHeight, 0,
	XmNresizePolicy, XmRESIZE_ANY,
	NULL);
    gui_motif_menu_colors(vimForm);

#ifdef FEAT_MENU
    {
	Arg al[7]; /* Make sure there is enough room for arguments! */
	int ac = 0;

# if (XmVersion >= 1002)
	XtSetArg(al[ac], XmNtearOffModel, tearoff_val); ac++;
# endif
	XtSetArg(al[ac], XmNleftAttachment,  XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNtopAttachment,   XmATTACH_FORM); ac++;
	XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
# ifndef FEAT_TOOLBAR
	/* Always stick to right hand side. */
	XtSetArg(al[ac], XmNrightOffset, 0); ac++;
# endif
	menuBar = XmCreateMenuBar(vimForm, "menuBar", al, ac);
	XtManageChild(menuBar);

	/* Remember the default colors, needed for ":hi clear". */
	XtVaGetValues(menuBar,
	    XmNbackground, &gui.menu_def_bg_pixel,
	    XmNforeground, &gui.menu_def_fg_pixel,
	    NULL);
	gui_motif_menu_colors(menuBar);
    }
#endif

#ifdef FEAT_TOOLBAR
    /*
     * Create an empty ToolBar. We should get buttons defined from menu.vim.
     */
    toolBarFrame = XtVaCreateWidget("toolBarFrame",
	xmFrameWidgetClass, vimForm,
	XmNshadowThickness, 0,
	XmNmarginHeight, 0,
	XmNmarginWidth, 0,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	NULL);
    gui_motif_menu_colors(toolBarFrame);

    toolBar = XtVaCreateManagedWidget("toolBar",
	xmRowColumnWidgetClass, toolBarFrame,
	XmNchildType, XmFRAME_WORKAREA_CHILD,
	XmNrowColumnType, XmWORK_AREA,
	XmNorientation, XmHORIZONTAL,
	XmNtraversalOn, False,
	XmNisHomogeneous, False,
	XmNpacking, XmPACK_TIGHT,
	XmNspacing, 0,
	XmNshadowThickness, 0,
	XmNhighlightThickness, 0,
	XmNmarginHeight, 0,
	XmNmarginWidth, 0,
	XmNadjustLast, True,
	NULL);
    gui_motif_menu_colors(toolBar);

# if 0	/* these don't work, because of the XmNtraversalOn above. */
    XtAddEventHandler(toolBar, EnterWindowMask, False,
	    toolbar_enter_cb, NULL);
    XtAddEventHandler(toolBar, LeaveWindowMask, False,
	    toolbar_leave_cb, NULL);
# endif
#endif

    textAreaForm = XtVaCreateManagedWidget("textAreaForm",
	xmFormWidgetClass, vimForm,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_FORM,
	XmNmarginWidth, 0,
	XmNmarginHeight, 0,
	XmNresizePolicy, XmRESIZE_ANY,
	NULL);
    gui_motif_scroll_colors(textAreaForm);

    textArea = XtVaCreateManagedWidget("textArea",
	xmDrawingAreaWidgetClass, textAreaForm,
	XmNforeground, gui.norm_pixel,
	XmNbackground, gui.back_pixel,
	XmNleftAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,

	/*
	 * These take some control away from the user, but avoids making them
	 * add resources to get a decent looking setup.
	 */
	XmNborderWidth, 0,
	XmNhighlightThickness, 0,
	XmNshadowThickness, 0,
	NULL);

#ifdef FEAT_FOOTER
    /*
     * Create the Footer.
     */
    footer = XtVaCreateWidget("footer",
	xmLabelGadgetClass, vimForm,
	XmNalignment, XmALIGNMENT_BEGINNING,
	XmNmarginHeight, 0,
	XmNmarginWidth, 0,
	XmNtraversalOn, False,
	XmNrecomputeSize, False,
	XmNleftAttachment, XmATTACH_FORM,
	XmNleftOffset, 5,
	XmNrightAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL);
    gui_mch_set_footer((char_u *) "");
#endif

    /*
     * Install the callbacks.
     */
    gui_x11_callbacks(textArea, vimForm);

    /* Pretend we don't have input focus, we will get an event if we do. */
    gui.in_focus = FALSE;
}

/*
 * Called when the GUI is not going to start after all.
 */
    void
gui_x11_destroy_widgets()
{
    textArea = NULL;
#ifdef FEAT_MENU
    menuBar = NULL;
#endif
}

/*ARGSUSED*/
    void
gui_mch_set_text_area_pos(x, y, w, h)
    int	    x;
    int	    y;
    int	    w;
    int	    h;
{
#ifdef FEAT_TOOLBAR
    /* Give keyboard focus to the textArea instead of the toolbar. */
    gui_mch_reset_focus();
#endif
}

    void
gui_x11_set_back_color()
{
    if (textArea != NULL)
#if (XmVersion >= 1002)
	XmChangeColor(textArea, gui.back_pixel);
#else
	XtVaSetValues(textArea,
		  XmNbackground, gui.back_pixel,
		  NULL);
#endif
}

/*
 * Manage dialog centered on pointer. This could be used by the Athena code as
 * well.
 */
static void manage_centered __ARGS((Widget dialog_child));

static void
manage_centered(dialog_child)
    Widget dialog_child;
{
    Widget shell = XtParent(dialog_child);
    Window root, child;
    unsigned int mask;
    unsigned int width, height, border_width, depth;
    int x, y, win_x, win_y, maxX, maxY;
    Boolean mappedWhenManaged;

    /* Temporarily set value of XmNmappedWhenManaged
       to stop the dialog from popping up right away */
    XtVaGetValues(shell, XmNmappedWhenManaged, &mappedWhenManaged, 0);
    XtVaSetValues(shell, XmNmappedWhenManaged, False, 0);

    XtManageChild(dialog_child);

    /* Get the pointer position (x, y) */
    XQueryPointer(XtDisplay(shell), XtWindow(shell), &root, &child,
		  &x, &y, &win_x, &win_y, &mask);

    /* Translate the pointer position (x, y) into a position for the new
       window that will place the pointer at its center */
    XGetGeometry(XtDisplay(shell), XtWindow(shell), &root, &win_x, &win_y,
		 &width, &height, &border_width, &depth);
    width += 2 * border_width;
    height += 2 * border_width;
    x -= width / 2;
    y -= height / 2;

    /* Ensure that the dialog remains on screen */
    maxX = XtScreen(shell)->width - width;
    maxY = XtScreen(shell)->height - height;
    if (x < 0)
	x = 0;
    if (x > maxX)
	x = maxX;
    if (y < 0)
	y = 0;
    if (y > maxY)
	y = maxY;

    /* Set desired window position in the DialogShell */
    XtVaSetValues(shell, XmNx, x, XmNy, y, NULL);

    /* Map the widget */
    XtMapWidget(shell);

    /* Restore the value of XmNmappedWhenManaged */
    XtVaSetValues(shell, XmNmappedWhenManaged, mappedWhenManaged, 0);
}

#if defined(FEAT_MENU) || defined(FEAT_SUN_WORKSHOP) \
	|| defined(FEAT_GUI_DIALOG) || defined(PROTO)

/*
 * Encapsulate the way an XmFontList is created.
 */
    XmFontList
gui_motif_create_fontlist(font)
    XFontStruct    *font;
{
    XmFontList font_list;

# if (XmVersion <= 1001)
    /* Motif 1.1 method */
    font_list = XmFontListCreate(font, STRING_TAG);
# else
    /* Motif 1.2 method */
    XmFontListEntry font_list_entry;

    font_list_entry = XmFontListEntryCreate(STRING_TAG, XmFONT_IS_FONT,
					    (XtPointer)font);
    font_list = XmFontListAppendEntry(NULL, font_list_entry);
    XmFontListEntryFree(&font_list_entry);
# endif
    return font_list;
}

# if ((XmVersion > 1001) && defined(FEAT_XFONTSET)) || defined(PROTO)
    XmFontList
gui_motif_fontset2fontlist(fontset)
    XFontSet	*fontset;
{
    XmFontList font_list;

    /* Motif 1.2 method */
    XmFontListEntry font_list_entry;

    font_list_entry = XmFontListEntryCreate(STRING_TAG,
					    XmFONT_IS_FONTSET,
					    (XtPointer)*fontset);
    font_list = XmFontListAppendEntry(NULL, font_list_entry);
    XmFontListEntryFree(&font_list_entry);
    return font_list;
}
# endif

#endif

#if defined(FEAT_MENU) || defined(PROTO)
/*
 * Menu stuff.
 */

static void gui_motif_add_actext __ARGS((vimmenu_T *menu));
#if (XmVersion >= 1002)
static void toggle_tearoff __ARGS((Widget wid));
static void gui_mch_recurse_tearoffs __ARGS((vimmenu_T *menu));
#endif
static void gui_mch_submenu_change __ARGS((vimmenu_T *mp, int colors));

static void do_set_mnemonics __ARGS((int enable));
static int menu_enabled = TRUE;

    void
gui_mch_enable_menu(flag)
    int	    flag;
{
    if (flag)
    {
	XtManageChild(menuBar);
#ifdef FEAT_TOOLBAR
	if (XtIsManaged(XtParent(toolBar)))
	{
	    /* toolBar is attached to top form */
	    XtVaSetValues(XtParent(toolBar),
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, menuBar,
		NULL);
	    XtVaSetValues(textAreaForm,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, XtParent(toolBar),
		NULL);
	}
	else
#endif
	{
	    XtVaSetValues(textAreaForm,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, menuBar,
		NULL);
	}
    }
    else
    {
	XtUnmanageChild(menuBar);
#ifdef FEAT_TOOLBAR
	if (XtIsManaged(XtParent(toolBar)))
	{
	    XtVaSetValues(XtParent(toolBar),
		XmNtopAttachment, XmATTACH_FORM,
		NULL);
	    XtVaSetValues(textAreaForm,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, XtParent(toolBar),
		NULL);
	}
	else
#endif
	{
	    XtVaSetValues(textAreaForm,
		XmNtopAttachment, XmATTACH_FORM,
		NULL);
	}
    }

}

/*
 * Enable or disable mnemonics for the toplevel menus.
 */
    void
gui_motif_set_mnemonics(enable)
    int		enable;
{
    /*
     * Don't enable menu mnemonics when the menu bar is disabled, LessTif
     * crashes when using a mnemonic then.
     */
    if (!menu_enabled)
	enable = FALSE;
    do_set_mnemonics(enable);
}

    static void
do_set_mnemonics(enable)
    int		enable;
{
    vimmenu_T	*menu;

    for (menu = root_menu; menu != NULL; menu = menu->next)
	if (menu->id != (Widget)0)
	    XtVaSetValues(menu->id,
		    XmNmnemonic, enable ? menu->mnemonic : NUL,
		    NULL);
}

    void
gui_mch_add_menu(menu, idx)
    vimmenu_T	*menu;
    int		idx;
{
    XmString	label;
    Widget	shell;
    vimmenu_T	*parent = menu->parent;

#ifdef MOTIF_POPUP
    if (menu_is_popup(menu->name))
    {
	Arg arg[2];
	int n = 0;

	/* Only create the popup menu when it's actually used, otherwise there
	 * is a delay when using the right mouse button. */
# if (XmVersion <= 1002)
	if (mouse_model_popup())
# endif
	{
	    if (gui.menu_bg_pixel != INVALCOLOR)
	    {
		XtSetArg(arg[0], XmNbackground, gui.menu_bg_pixel); n++;
	    }
	    if (gui.menu_fg_pixel != INVALCOLOR)
	    {
		XtSetArg(arg[1], XmNforeground, gui.menu_fg_pixel); n++;
	    }
	    menu->submenu_id = XmCreatePopupMenu(textArea, "contextMenu",
								      arg, n);
	    menu->id = (Widget)0;
	}
	return;
    }
#endif

    if (!menu_is_menubar(menu->name)
	    || (parent != NULL && parent->submenu_id == (Widget)0))
	return;

    label = XmStringCreate((char *)menu->dname, STRING_TAG);
    if (label == NULL)
	return;
    menu->id = XtVaCreateWidget("subMenu",
	    xmCascadeButtonWidgetClass,
	    (parent == NULL) ? menuBar : parent->submenu_id,
	    XmNlabelString, label,
	    XmNmnemonic, p_wak[0] == 'n' ? NUL : menu->mnemonic,
#if (XmVersion >= 1002)
	    /* submenu: count the tearoff item (needed for LessTif) */
	    XmNpositionIndex, idx + (parent != NULL
			   && tearoff_val == (int)XmTEAR_OFF_ENABLED ? 1 : 0),
#endif
	    NULL);
    gui_motif_menu_colors(menu->id);
    gui_motif_menu_fontlist(menu->id);
    XmStringFree(label);

    if (menu->id == (Widget)0)		/* failed */
	return;

    /* add accelerator text */
    gui_motif_add_actext(menu);

    shell = XtVaCreateWidget("subMenuShell",
	xmMenuShellWidgetClass, menu->id,
	XmNwidth, 1,
	XmNheight, 1,
	NULL);
    gui_motif_menu_colors(shell);
    menu->submenu_id = XtVaCreateWidget("rowColumnMenu",
	xmRowColumnWidgetClass, shell,
	XmNrowColumnType, XmMENU_PULLDOWN,
	NULL);
    gui_motif_menu_colors(menu->submenu_id);

    if (menu->submenu_id == (Widget)0)		/* failed */
	return;

#if (XmVersion >= 1002)
    /* Set the colors for the tear off widget */
    toggle_tearoff(menu->submenu_id);
#endif

    XtVaSetValues(menu->id,
	XmNsubMenuId, menu->submenu_id,
	NULL);

    /*
     * The "Help" menu is a special case, and should be placed at the far
     * right hand side of the menu-bar.  It's recognized by its high priority.
     */
    if (parent == NULL && menu->priority >= 9999)
	XtVaSetValues(menuBar,
		XmNmenuHelpWidget, menu->id,
		NULL);

    /*
     * When we add a top-level item to the menu bar, we can figure out how
     * high the menu bar should be.
     */
    if (parent == NULL)
	gui_mch_compute_menu_height(menu->id);
}


/*
 * Add mnemonic and accelerator text to a menu button.
 */
    static void
gui_motif_add_actext(menu)
    vimmenu_T	*menu;
{
    XmString	label;

    /* Add accelrator text, if there is one */
    if (menu->actext != NULL && menu->id != (Widget)0)
    {
	label = XmStringCreate((char *)menu->actext, STRING_TAG);
	if (label == NULL)
	    return;
	XtVaSetValues(menu->id, XmNacceleratorText, label, NULL);
	XmStringFree(label);
    }
}

    void
gui_mch_toggle_tearoffs(enable)
    int		enable;
{
#if (XmVersion >= 1002)
    if (enable)
	tearoff_val = (int)XmTEAR_OFF_ENABLED;
    else
	tearoff_val = (int)XmTEAR_OFF_DISABLED;
    toggle_tearoff(menuBar);
    gui_mch_recurse_tearoffs(root_menu);
#endif
}

#if (XmVersion >= 1002)
/*
 * Set the tearoff for one menu widget on or off, and set the color of the
 * tearoff widget.
 */
    static void
toggle_tearoff(wid)
    Widget	wid;
{
    Widget	w;

    XtVaSetValues(wid, XmNtearOffModel, tearoff_val, NULL);
    if (tearoff_val == (int)XmTEAR_OFF_ENABLED
	    && (w = XmGetTearOffControl(wid)) != (Widget)0)
	gui_motif_menu_colors(w);
}

    static void
gui_mch_recurse_tearoffs(menu)
    vimmenu_T	*menu;
{
    while (menu != NULL)
    {
	if (!menu_is_popup(menu->name))
	{
	    if (menu->submenu_id != (Widget)0)
		toggle_tearoff(menu->submenu_id);
	    gui_mch_recurse_tearoffs(menu->children);
	}
	menu = menu->next;
    }
}
#endif

    int
gui_mch_text_area_extra_height()
{
    Dimension	shadowHeight;

    XtVaGetValues(textAreaForm, XmNshadowThickness, &shadowHeight, NULL);
    return shadowHeight;
}

/*
 * Compute the height of the menu bar.
 * We need to check all the items for their position and height, for the case
 * there are several rows, and/or some characters extend higher or lower.
 */
    void
gui_mch_compute_menu_height(id)
    Widget	id;		    /* can be NULL when deleting menu */
{
    Dimension	y, maxy;
    Dimension	margin, shadow;
    vimmenu_T	*mp;
    static Dimension	height = 21;	/* normal height of a menu item */

    /*
     * Get the height of the new item, before managing it, because it will
     * still reflect the font size.  After managing it depends on the menu
     * height, which is what we just wanted to get!.
     */
    if (id != (Widget)0)
	XtVaGetValues(id, XmNheight, &height, NULL);

    /* Find any menu Widget, to be able to call XtManageChild() */
    else
	for (mp = root_menu; mp != NULL; mp = mp->next)
	    if (mp->id != (Widget)0 && menu_is_menubar(mp->name))
	    {
		id = mp->id;
		break;
	    }

    /*
     * Now manage the menu item, to make them all be positioned (makes an
     * extra row when needed, removes it when not needed).
     */
    if (id != (Widget)0)
	XtManageChild(id);

    /*
     * Now find the menu item that is the furthest down, and get it's position.
     */
    maxy = 0;
    for (mp = root_menu; mp != NULL; mp = mp->next)
    {
	if (mp->id != (Widget)0 && menu_is_menubar(mp->name))
	{
	    XtVaGetValues(mp->id, XmNy, &y, NULL);
	    if (y > maxy)
		maxy = y;
	}
    }

    XtVaGetValues(menuBar,
	XmNmarginHeight, &margin,
	XmNshadowThickness, &shadow,
	NULL);

    /*
     * This computation is the result of trial-and-error:
     * maxy =	The maximum position of an item; required for when there are
     *		two or more rows
     * height = height of an item, before managing it;	Hopefully this will
     *		change with the font height.  Includes shadow-border.
     * shadow =	shadow-border; must be subtracted from the height.
     * margin = margin around the menu buttons;  Must be added.
     * Add 4 for the underlining of shortcut keys.
     */
    gui.menu_height = maxy + height - 2 * shadow + 2 * margin + 4;

#ifdef LESSTIF_VERSION
    /* Somehow the menu bar doesn't resize automatically.  Set it here,
     * even though this is a catch 22.  Don't do this when starting up,
     * somehow the menu gets very high then. */
    if (gui.shell_created)
	XtVaSetValues(menuBar, XmNheight, gui.menu_height, NULL);
#endif
}

    void
gui_mch_add_menu_item(menu, idx)
    vimmenu_T	*menu;
    int		idx;
{
    XmString	label;
    vimmenu_T	*parent = menu->parent;

# ifdef EBCDIC
    menu->mnemonic = 0;
# endif

# if (XmVersion <= 1002)
    /* Don't add Popup menu items when the popup menu isn't used. */
    if (menu_is_child_of_popup(menu) && !mouse_model_popup())
	return;
# endif

# ifdef FEAT_TOOLBAR
    if (menu_is_toolbar(parent->name))
    {
	WidgetClass	type;
	XmString	xms = NULL;    /* fallback label if pixmap not found */
	int		n;
	Arg		args[18];

	n = 0;
	if (menu_is_separator(menu->name))
	{
	    char	*cp;
	    Dimension	wid;

	    /*
	     * A separator has the format "-sep%d[:%d]-". The optional :%d is
	     * a width specifier. If no width is specified then we choose one.
	     */
	    cp = (char *)vim_strchr(menu->name, ':');
	    if (cp != NULL)
		wid = (Dimension)atoi(++cp);
	    else
		wid = 4;

#if 0
	    /* We better use a FormWidget here, since it's far more
	     * flexible in terms of size.  */
	    type = xmFormWidgetClass;
	    XtSetArg(args[n], XmNwidth, wid); n++;
#else
	    type = xmSeparatorWidgetClass;
	    XtSetArg(args[n], XmNwidth, wid); n++;
	    XtSetArg(args[n], XmNminWidth, wid); n++;
	    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
	    XtSetArg(args[n], XmNseparatorType, XmNO_LINE); n++;
#endif
	}
	else
	{
	    get_toolbar_pixmap(menu, &menu->image, &menu->image_ins);
	    /* Set the label here, so that we can switch between icons/text
	     * by changing the XmNlabelType resource. */
	    xms = XmStringCreate((char *)menu->dname, STRING_TAG);
	    XtSetArg(args[n], XmNlabelString, xms); n++;

#ifndef FEAT_SUN_WORKSHOP

	    /* Without shadows one can't sense whatever the button has been
	     * pressed or not! However we wan't to save a bit of space...
	     */
	    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
	    XtSetArg(args[n], XmNhighlightOnEnter, True); n++;
	    XtSetArg(args[n], XmNmarginWidth, 0); n++;
	    XtSetArg(args[n], XmNmarginHeight, 0); n++;
#endif
	    if (menu->image == 0)
	    {
		XtSetArg(args[n], XmNlabelType, XmSTRING); n++;
		XtSetArg(args[n], XmNlabelPixmap, 0); n++;
		XtSetArg(args[n], XmNlabelInsensitivePixmap, 0); n++;
	    }
	    else
	    {
		XtSetArg(args[n], XmNlabelPixmap, menu->image); n++;
		XtSetArg(args[n], XmNlabelInsensitivePixmap, menu->image_ins); n++;
		XtSetArg(args[n], XmNlabelType, XmPIXMAP); n++;
	    }
	    type = xmPushButtonWidgetClass;
	    XtSetArg(args[n], XmNwidth, 80); n++;
	}

	XtSetArg(args[n], XmNpositionIndex, idx); n++;
	if (menu->id == NULL)
	{
	    menu->id = XtCreateManagedWidget((char *)menu->dname,
			type, toolBar, args, n);
	    if (menu->id != NULL && type == xmPushButtonWidgetClass)
	    {
		XtAddCallback(menu->id,
			XmNactivateCallback, gui_x11_menu_cb, menu);

# ifdef FEAT_FOOTER
		XtAddEventHandler(menu->id, EnterWindowMask, False,
			toolbarbutton_enter_cb, menu);
		XtAddEventHandler(menu->id, LeaveWindowMask, False,
			toolbarbutton_leave_cb, menu);
# endif
	    }
	}
	else
	    XtSetValues(menu->id, args, n);
	if (xms != NULL)
	    XmStringFree(xms);

#ifdef FEAT_BEVAL
	gui_mch_menu_set_tip(menu);
#endif

	menu->parent = parent;
	menu->submenu_id = NULL;
	/* When adding first item to toolbar it might have to be enabled .*/
	if (!XtIsManaged(XtParent(toolBar))
		    && vim_strchr(p_go, GO_TOOLBAR) != NULL)
	    gui_mch_show_toolbar(TRUE);
	gui.toolbar_height = gui_mch_compute_toolbar_height();
	return;
    } /* toolbar menu item */
# endif

    /* No parent, must be a non-menubar menu */
    if (parent->submenu_id == (Widget)0)
	return;

    menu->submenu_id = (Widget)0;

    /* Add menu separator */
    if (menu_is_separator(menu->name))
    {
	menu->id = XtVaCreateWidget("subMenu",
		xmSeparatorGadgetClass, parent->submenu_id,
#if (XmVersion >= 1002)
		/* count the tearoff item (needed for LessTif) */
		XmNpositionIndex, idx + (tearoff_val == (int)XmTEAR_OFF_ENABLED
								     ? 1 : 0),
#endif
		NULL);
	gui_motif_menu_colors(menu->id);
	return;
    }

    label = XmStringCreate((char *)menu->dname, STRING_TAG);
    if (label == NULL)
	return;
    menu->id = XtVaCreateWidget("subMenu",
	xmPushButtonWidgetClass, parent->submenu_id,
	XmNlabelString, label,
	XmNmnemonic, menu->mnemonic,
#if (XmVersion >= 1002)
	/* count the tearoff item (needed for LessTif) */
	XmNpositionIndex, idx + (tearoff_val == (int)XmTEAR_OFF_ENABLED
								     ? 1 : 0),
#endif
	NULL);
    gui_motif_menu_colors(menu->id);
    gui_motif_menu_fontlist(menu->id);
    XmStringFree(label);

    if (menu->id != (Widget)0)
    {
	XtAddCallback(menu->id, XmNactivateCallback, gui_x11_menu_cb,
		(XtPointer)menu);
	/* add accelerator text */
	gui_motif_add_actext(menu);
    }
}

#if (XmVersion <= 1002) || defined(PROTO)
/*
 * This function will destroy/create the popup menus dynamically,
 * according to the value of 'mousemodel'.
 * This will fix the "right mouse button freeze" that occurs when
 * there exists a popup menu but it isn't managed.
 */
    void
gui_motif_update_mousemodel(menu)
    vimmenu_T	*menu;
{
    int		idx = 0;

    /* When GUI hasn't started the menus have not been created. */
    if (!gui.in_use)
      return;

    while (menu)
    {
      if (menu->children != NULL)
      {
	  if (menu_is_popup(menu->name))
	  {
	      if (mouse_model_popup())
	      {
		  /* Popup menu will be used.  Create the popup menus. */
		  gui_mch_add_menu(menu, idx);
		  gui_motif_update_mousemodel(menu->children);
	      }
	      else
	      {
		  /* Popup menu will not be used.  Destroy the popup menus. */
		  gui_motif_update_mousemodel(menu->children);
		  gui_mch_destroy_menu(menu);
	      }
	  }
      }
      else if (menu_is_child_of_popup(menu))
      {
	  if (mouse_model_popup())
	      gui_mch_add_menu_item(menu, idx);
	  else
	      gui_mch_destroy_menu(menu);
      }
      menu = menu->next;
      ++idx;
    }
}
#endif

    void
gui_mch_new_menu_colors()
{
    if (menuBar == (Widget)0)
	return;
    gui_motif_menu_colors(menuBar);
#ifdef FEAT_TOOLBAR
    gui_motif_menu_colors(toolBarFrame);
    gui_motif_menu_colors(toolBar);
#endif

    gui_mch_submenu_change(root_menu, TRUE);
}

    void
gui_mch_new_menu_font()
{
    if (menuBar == (Widget)0)
	return;
    gui_mch_submenu_change(root_menu, FALSE);
    {
	Dimension   height;
	Position w, h;

	XtVaGetValues(menuBar, XmNheight, &height, NULL);
	gui.menu_height = height;

	XtVaGetValues(vimShell, XtNwidth, &w, XtNheight, &h, NULL);
	gui_resize_shell(w, h
#ifdef FEAT_XIM
		- xim_get_status_area_height()
#endif
		     );
    }
    gui_set_shellsize(FALSE, TRUE);
    ui_new_shellsize();
}

#if defined(FEAT_BEVAL) || defined(PROTO)
    void
gui_mch_new_tooltip_font()
{
# ifdef FEAT_TOOLBAR
    vimmenu_T   *menu;

    if (toolBar == (Widget)0)
	return;

    menu = gui_find_menu((char_u *)"ToolBar");
    if (menu != NULL)
	gui_mch_submenu_change(menu, FALSE);
# endif
}

    void
gui_mch_new_tooltip_colors()
{
# ifdef FEAT_TOOLBAR
    vimmenu_T   *toolbar;

    if (toolBar == (Widget)0)
	return;

    toolbar = gui_find_menu((char_u *)"ToolBar");
    if (toolbar != NULL)
	gui_mch_submenu_change(toolbar, TRUE);
# endif
}
#endif

    static void
gui_mch_submenu_change(menu, colors)
    vimmenu_T	*menu;
    int		colors;		/* TRUE for colors, FALSE for font */
{
    vimmenu_T	*mp;

    for (mp = menu; mp != NULL; mp = mp->next)
    {
	if (mp->id != (Widget)0)
	{
	    if (colors)
	    {
		gui_motif_menu_colors(mp->id);
#ifdef FEAT_TOOLBAR
		/* For a toolbar item: Free the pixmap and allocate a new one,
		 * so that the background color is right. */
		if (mp->image != (Pixmap)0)
		{
		    XFreePixmap(gui.dpy, mp->image);
		    XFreePixmap(gui.dpy, mp->image_ins);
		    get_toolbar_pixmap(mp, &mp->image, &mp->image_ins);
		    if (mp->image != (Pixmap)0)
			XtVaSetValues(mp->id,
				XmNlabelPixmap, mp->image,
				XmNlabelInsensitivePixmap, mp->image_ins,
				NULL);
		}
# ifdef FEAT_BEVAL
		/* If we have a tooltip, then we need to change it's font */
		if (mp->tip != NULL)
		{
		    Arg args[2];

		    args[0].name = XmNbackground;
		    args[0].value = gui.tooltip_bg_pixel;
		    args[1].name = XmNforeground;
		    args[1].value = gui.tooltip_fg_pixel;
		    XtSetValues(mp->tip->balloonLabel, &args[0], XtNumber(args));
		}
# endif
#endif
	    }
	    else
	    {
		gui_motif_menu_fontlist(mp->id);
#ifdef FEAT_BEVAL
		/* If we have a tooltip, then we need to change it's font */
		if (mp->tip != NULL)
		{
		    Arg args[1];

		    args[0].name = XmNfontList;
		    args[0].value = (XtArgVal)gui_motif_fontset2fontlist(
						    &gui.tooltip_fontset);
		    XtSetValues(mp->tip->balloonLabel, &args[0], XtNumber(args));
		}
#endif
	    }
	}

	if (mp->children != NULL)
	{
#if (XmVersion >= 1002)
	    /* Set the colors/font for the tear off widget */
	    if (mp->submenu_id != (Widget)0)
	    {
		if (colors)
		    gui_motif_menu_colors(mp->submenu_id);
		else
		    gui_motif_menu_fontlist(mp->submenu_id);
		toggle_tearoff(mp->submenu_id);
	    }
#endif
	    /* Set the colors for the children */
	    gui_mch_submenu_change(mp->children, colors);
	}
    }
}

/*
 * Destroy the machine specific menu widget.
 */
    void
gui_mch_destroy_menu(menu)
    vimmenu_T	*menu;
{
    /* Please be sure to destroy the parent widget first (i.e. menu->id).
     * On the other hand, problems have been reported that the submenu must be
     * deleted first...
     *
     * This code should be basically identical to that in the file gui_athena.c
     * because they are both Xt based.
     */
    if (menu->submenu_id != (Widget)0)
    {
	XtDestroyWidget(menu->submenu_id);
	menu->submenu_id = (Widget)0;
    }

    if (menu->id != (Widget)0)
    {
	Widget	    parent;

	parent = XtParent(menu->id);
#if defined(FEAT_TOOLBAR) && defined(FEAT_BEVAL)
	if ((parent == toolBar) && (menu->tip != NULL))
	{
	    /* We try to destroy this before the actual menu, because there are
	     * callbacks, etc. that will be unregistered during the tooltip
	     * destruction.
	     *
	     * If you call "gui_mch_destroy_beval_area()" after destroying
	     * menu->id, then the tooltip's window will have already been
	     * deallocated by Xt, and unknown behaviour will ensue (probably
	     * a core dump).
	     */
	    gui_mch_destroy_beval_area(menu->tip);
	    menu->tip = NULL;
	}
#endif
	XtDestroyWidget(menu->id);
	menu->id = (Widget)0;
	if (parent == menuBar)
	    gui_mch_compute_menu_height((Widget)0);
#ifdef FEAT_TOOLBAR
	else if (parent == toolBar)
	{
	    Cardinal    num_children;

	    /* When removing last toolbar item, don't display the toolbar. */
	    XtVaGetValues(toolBar, XmNnumChildren, &num_children, NULL);
	    if (num_children == 0)
		gui_mch_show_toolbar(FALSE);
	    else
		gui.toolbar_height = gui_mch_compute_toolbar_height();
	}
#endif
    }
}

/* ARGSUSED */
    void
gui_mch_show_popupmenu(menu)
    vimmenu_T *menu;
{
#ifdef MOTIF_POPUP
    XmMenuPosition(menu->submenu_id, gui_x11_get_last_mouse_event());
    XtManageChild(menu->submenu_id);
#endif
}

#endif /* FEAT_MENU */

/*
 * Set the menu and scrollbar colors to their default values.
 */
    void
gui_mch_def_colors()
{
    if (gui.in_use)
    {
	/* Use the values saved when starting up.  These should come from the
	 * window manager or a resources file. */
	gui.menu_fg_pixel = gui.menu_def_fg_pixel;
	gui.menu_bg_pixel = gui.menu_def_bg_pixel;
	gui.scroll_fg_pixel = gui.scroll_def_fg_pixel;
	gui.scroll_bg_pixel = gui.scroll_def_bg_pixel;
#ifdef FEAT_BEVAL
	gui.tooltip_fg_pixel =
			gui_get_color((char_u *)gui.rsrc_tooltip_fg_name);
	gui.tooltip_bg_pixel =
			gui_get_color((char_u *)gui.rsrc_tooltip_bg_name);
#endif
    }
}


/*
 * Scrollbar stuff.
 */

    void
gui_mch_set_scrollbar_thumb(sb, val, size, max)
    scrollbar_T *sb;
    long	val;
    long	size;
    long	max;
{
    if (sb->id != (Widget)0)
	XtVaSetValues(sb->id,
		  XmNvalue, val,
		  XmNsliderSize, size,
		  XmNpageIncrement, (size > 2 ? size - 2 : 1),
		  XmNmaximum, max + 1,	    /* Motif has max one past the end */
		  NULL);
}

    void
gui_mch_set_scrollbar_pos(sb, x, y, w, h)
    scrollbar_T *sb;
    int		x;
    int		y;
    int		w;
    int		h;
{
    if (sb->id != (Widget)0)
    {
	if (sb->type == SBAR_LEFT || sb->type == SBAR_RIGHT)
	{
	    if (y == 0)
		h -= gui.border_offset;
	    else
		y -= gui.border_offset;
	    XtVaSetValues(sb->id,
			  XmNtopOffset, y,
			  XmNbottomOffset, -y - h,
			  XmNwidth, w,
			  NULL);
	}
	else
	    XtVaSetValues(sb->id,
			  XmNtopOffset, y,
			  XmNleftOffset, x,
			  XmNrightOffset, gui.which_scrollbars[SBAR_RIGHT]
						    ? gui.scrollbar_width : 0,
			  XmNheight, h,
			  NULL);
	XtManageChild(sb->id);
    }
}

    void
gui_mch_enable_scrollbar(sb, flag)
    scrollbar_T *sb;
    int		flag;
{
    Arg		args[16];
    int		n;

    if (sb->id != (Widget)0)
    {
	n = 0;
	if (flag)
	{
	    switch (sb->type)
	    {
		case SBAR_LEFT:
		    XtSetArg(args[n], XmNleftOffset, gui.scrollbar_width); n++;
		    break;

		case SBAR_RIGHT:
		    XtSetArg(args[n], XmNrightOffset, gui.scrollbar_width); n++;
		    break;

		case SBAR_BOTTOM:
		    XtSetArg(args[n], XmNbottomOffset, gui.scrollbar_height);n++;
		    break;
	    }
	    XtSetValues(textArea, args, n);
	    XtManageChild(sb->id);
	}
	else
	{
	    if (!gui.which_scrollbars[sb->type])
	    {
		/* The scrollbars of this type are all disabled, adjust the
		 * textArea attachment offset. */
		switch (sb->type)
		{
		    case SBAR_LEFT:
			XtSetArg(args[n], XmNleftOffset, 0); n++;
			break;

		    case SBAR_RIGHT:
			XtSetArg(args[n], XmNrightOffset, 0); n++;
			break;

		    case SBAR_BOTTOM:
			XtSetArg(args[n], XmNbottomOffset, 0);n++;
			break;
		}
		XtSetValues(textArea, args, n);
	    }
	    XtUnmanageChild(sb->id);
	}
    }
}

    void
gui_mch_create_scrollbar(sb, orient)
    scrollbar_T *sb;
    int		orient;	/* SBAR_VERT or SBAR_HORIZ */
{
    Arg		args[16];
    int		n;

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 1); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNorientation,
	    (orient == SBAR_VERT) ? XmVERTICAL : XmHORIZONTAL); n++;

    switch (sb->type)
    {
	case SBAR_LEFT:
	    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_OPPOSITE_FORM); n++;
	    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    break;

	case SBAR_RIGHT:
	    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_OPPOSITE_FORM); n++;
	    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    break;

	case SBAR_BOTTOM:
	    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    break;
    }

    sb->id = XtCreateWidget("scrollBar",
	    xmScrollBarWidgetClass, textAreaForm, args, n);

    /* Remember the default colors, needed for ":hi clear". */
    if (gui.scroll_def_bg_pixel == (guicolor_T)0
	    && gui.scroll_def_fg_pixel == (guicolor_T)0)
	XtVaGetValues(sb->id,
		XmNbackground, &gui.scroll_def_bg_pixel,
		XmNforeground, &gui.scroll_def_fg_pixel,
		NULL);

    if (sb->id != (Widget)0)
    {
	gui_mch_set_scrollbar_colors(sb);
	XtAddCallback(sb->id, XmNvalueChangedCallback,
		      scroll_cb, (XtPointer)sb->ident);
	XtAddCallback(sb->id, XmNdragCallback,
		      scroll_cb, (XtPointer)sb->ident);
	XtAddEventHandler(sb->id, KeyPressMask, FALSE, gui_x11_key_hit_cb,
	    (XtPointer)0);
    }
}

#if defined(FEAT_WINDOWS) || defined(PROTO)
    void
gui_mch_destroy_scrollbar(sb)
    scrollbar_T *sb;
{
    if (sb->id != (Widget)0)
	XtDestroyWidget(sb->id);
}
#endif

    void
gui_mch_set_scrollbar_colors(sb)
    scrollbar_T *sb;
{
    if (sb->id != (Widget)0)
    {
	if (gui.scroll_bg_pixel != INVALCOLOR)
	{
#if (XmVersion>=1002)
	    XmChangeColor(sb->id, gui.scroll_bg_pixel);
#else
	    XtVaSetValues(sb->id,
		    XmNtroughColor, gui.scroll_bg_pixel,
		    NULL);
#endif
	}

	if (gui.scroll_fg_pixel != INVALCOLOR)
	    XtVaSetValues(sb->id,
		    XmNforeground, gui.scroll_fg_pixel,
#if (XmVersion<1002)
		    XmNbackground, gui.scroll_fg_pixel,
#endif
		    NULL);
    }

    /* This is needed for the rectangle below the vertical scrollbars. */
    if (sb == &gui.bottom_sbar && textAreaForm != (Widget)0)
	gui_motif_scroll_colors(textAreaForm);
}

/*
 * Miscellaneous stuff:
 */

    Window
gui_x11_get_wid()
{
    return(XtWindow(textArea));
}


#if defined(FEAT_BROWSE) || defined(PROTO)

/*
 * file selector related stuff
 */

#include <Xm/FileSB.h>
#include <Xm/XmStrDefs.h>

typedef struct dialog_callback_arg
{
    char *  args;   /* not used right now */
    int	    id;
} dcbarg_T;

static Widget dialog_wgt;
static char *browse_fname = NULL;
static XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
				/* used to set up XmStrings */

static void DialogCancelCB __ARGS((Widget, XtPointer, XtPointer));
static void DialogAcceptCB __ARGS((Widget, XtPointer, XtPointer));

/*
 * This function is used to translate the predefined label text of the
 * precomposed dialogs. We do this explicitly to allow:
 *
 * - usage of gettext for translation, as in all the other places.
 *
 * - equalize the messages between different GUI implementations as far as
 * possible.
 */
static void set_predefined_label __ARGS((Widget parent, String name, char * new_label));

static void
set_predefined_label(parent, name, new_label)
    Widget parent;
    String name;
    char * new_label;
{
    XmString str;
    Widget w;

    w = XtNameToWidget(parent, name);

    if (!w)
	return;

    str = XmStringCreate(new_label, STRING_TAG);

    if (str)
    {
	XtVaSetValues(w, XmNlabelString, str, NULL);
	XmStringFree(str);
    }
}

/*
 * Put up a file requester.
 * Returns the selected name in allocated memory, or NULL for Cancel.
 */
/* ARGSUSED */
    char_u *
gui_mch_browse(saving, title, dflt, ext, initdir, filter)
    int		saving;		/* select file to write */
    char_u	*title;		/* title for the window */
    char_u	*dflt;		/* default name */
    char_u	*ext;		/* not used (extension added) */
    char_u	*initdir;	/* initial directory, NULL for current dir */
    char_u	*filter;	/* file name filter */
{
    char_u	dirbuf[MAXPATHL];
    char_u	dfltbuf[MAXPATHL];
    char_u	*pattern;
    char_u	*tofree = NULL;

    dialog_wgt = XmCreateFileSelectionDialog(vimShell, (char *)title, NULL, 0);

    if (initdir == NULL || *initdir == NUL)
    {
	mch_dirname(dirbuf, MAXPATHL);
	initdir = dirbuf;
    }

    if (dflt == NULL)
	dflt = (char_u *)"";
    else if (STRLEN(initdir) + STRLEN(dflt) + 2 < MAXPATHL)
    {
	/* The default selection should be the full path, "dflt" is only the
	 * file name. */
	STRCPY(dfltbuf, initdir);
	add_pathsep(dfltbuf);
	STRCAT(dfltbuf, dflt);
	dflt = dfltbuf;
    }

    /* Can only use one pattern for a file name.  Get the first pattern out of
     * the filter.  An empty pattern means everything matches. */
    if (filter == NULL)
	pattern = (char_u *)"";
    else
    {
	char_u	*s, *p;

	s = filter;
	for (p = filter; *p != NUL; ++p)
	{
	    if (*p == '\t')	/* end of description, start of pattern */
		s = p + 1;
	    if (*p == ';' || *p == '\n')	/* end of (first) pattern */
		break;
	}
	pattern = vim_strnsave(s, p - s);
	tofree = pattern;
	if (pattern == NULL)
	    pattern = (char_u *)"";
    }

    XtVaSetValues(dialog_wgt,
	XtVaTypedArg,
	    XmNdirectory, XmRString, (char *)initdir, STRLEN(initdir) + 1,
	XtVaTypedArg,
	    XmNdirSpec,	XmRString, (char *)dflt, STRLEN(dflt) + 1,
	XtVaTypedArg,
	    XmNpattern,	XmRString, (char *)pattern, STRLEN(pattern) + 1,
	XtVaTypedArg,
	    XmNdialogTitle, XmRString, (char *)title, STRLEN(title) + 1,
	NULL);

    set_predefined_label(dialog_wgt, "Apply", _("Filter"));
    set_predefined_label(dialog_wgt, "Cancel", _("Cancel"));
    set_predefined_label(dialog_wgt, "Dir", _("Directories"));
    set_predefined_label(dialog_wgt, "FilterLabel", _("Filter"));
    set_predefined_label(dialog_wgt, "Help", _("Help"));
    set_predefined_label(dialog_wgt, "Items", _("Files"));
    set_predefined_label(dialog_wgt, "OK", _("OK"));
    set_predefined_label(dialog_wgt, "Selection", _("Selection"));

    gui_motif_menu_colors(dialog_wgt);
    if (gui.scroll_bg_pixel != INVALCOLOR)
	XtVaSetValues(dialog_wgt, XmNtroughColor, gui.scroll_bg_pixel, NULL);

    XtAddCallback(dialog_wgt, XmNokCallback, DialogAcceptCB, (XtPointer)0);
    XtAddCallback(dialog_wgt, XmNcancelCallback, DialogCancelCB, (XtPointer)0);
    /* We have no help in this window, so hide help button */
    XtUnmanageChild(XmFileSelectionBoxGetChild(dialog_wgt,
					(unsigned char)XmDIALOG_HELP_BUTTON));

    manage_centered(dialog_wgt);

    /* sit in a loop until the dialog box has gone away */
    do
    {
	XtAppProcessEvent(XtWidgetToApplicationContext(dialog_wgt),
	    (XtInputMask)XtIMAll);
    } while (XtIsManaged(dialog_wgt));

    XtDestroyWidget(dialog_wgt);
    vim_free(tofree);

    if (browse_fname == NULL)
	return NULL;
    return vim_strsave((char_u *)browse_fname);
}

/*
 * The code below was originally taken from
 *	/usr/examples/motif/xmsamplers/xmeditor.c
 * on Digital Unix 4.0d, but heavily modified.
 */

/*
 * Process callback from Dialog cancel actions.
 */
/* ARGSUSED */
    static void
DialogCancelCB(w, client_data, call_data)
    Widget	w;		/*  widget id		*/
    XtPointer	client_data;	/*  data from application   */
    XtPointer	call_data;	/*  data from widget class  */
{
    if (browse_fname != NULL)
    {
	XtFree(browse_fname);
	browse_fname = NULL;
    }
    XtUnmanageChild(dialog_wgt);
}

/*
 * Process callback from Dialog actions.
 */
/* ARGSUSED */
    static void
DialogAcceptCB(w, client_data, call_data)
    Widget	w;		/*  widget id		*/
    XtPointer	client_data;	/*  data from application   */
    XtPointer	call_data;	/*  data from widget class  */
{
    XmFileSelectionBoxCallbackStruct *fcb;

    if (browse_fname != NULL)
    {
	XtFree(browse_fname);
	browse_fname = NULL;
    }
    fcb = (XmFileSelectionBoxCallbackStruct *)call_data;

    /* get the filename from the file selection box */
    XmStringGetLtoR(fcb->value, charset, &browse_fname);

    /* popdown the file selection box */
    XtUnmanageChild(dialog_wgt);
}

#endif /* FEAT_BROWSE */

#if defined(FEAT_GUI_DIALOG) || defined(PROTO)

static int	dialogStatus;

static void keyhit_callback __ARGS((Widget w, XtPointer client_data, XEvent *event, Boolean *cont));
static void butproc __ARGS((Widget w, XtPointer client_data, XtPointer call_data));

/*
 * Callback function for the textfield.  When CR is hit this works like
 * hitting the "OK" button, ESC like "Cancel".
 */
/* ARGSUSED */
    static void
keyhit_callback(w, client_data, event, cont)
    Widget		w;
    XtPointer		client_data;
    XEvent		*event;
    Boolean		*cont;
{
    char	buf[2];
    KeySym	key_sym;

    if (XLookupString(&(event->xkey), buf, 2, &key_sym, NULL) == 1)
    {
	if (*buf == CAR)
	    dialogStatus = 1;
	else if (*buf == ESC)
	    dialogStatus = 2;
    }
    if ((key_sym == XK_Left || key_sym == XK_Right)
	    && !(event->xkey.state & ShiftMask))
	XmTextFieldClearSelection(w, XtLastTimestampProcessed(gui.dpy));
}

/* ARGSUSED */
    static void
butproc(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    dialogStatus = (int)(long)client_data + 1;
}

static void gui_motif_set_fontlist __ARGS((Widget wg));

/*
 * Use the 'guifont' or 'guifontset' as a fontlist for a dialog widget.
 */
    static void
gui_motif_set_fontlist(wg)
    Widget wg;
{
    XmFontList fl;

    fl =
#ifdef FEAT_XFONTSET
	    gui.fontset != NOFONTSET ?
		    gui_motif_fontset2fontlist((XFontSet *)&gui.fontset)
				     :
#endif
		    gui_motif_create_fontlist((XFontStruct *)gui.norm_font);
    if (fl != NULL)
    {
	XtVaSetValues(wg, XmNfontList, fl, NULL);
	XmFontListFree(fl);
    }
}

#ifdef HAVE_XPM

static Widget create_pixmap_label(Widget parent, String name, char **data, ArgList args, Cardinal arg);

    static Widget
create_pixmap_label(parent, name, data, args, arg)
    Widget	parent;
    String	name;
    char	**data;
    ArgList	args;
    Cardinal	arg;
{
    Widget		label;
    Display		*dsp;
    Screen		*scr;
    int			depth;
    Pixmap		pixmap = 0;
    XpmAttributes	attr;
    Boolean		rs;
    XpmColorSymbol	color[5] =
    {
	{"none", NULL, 0},
	{"iconColor1", NULL, 0},
	{"bottomShadowColor", NULL, 0},
	{"topShadowColor", NULL, 0},
	{"selectColor", NULL, 0}
    };

    label = XmCreateLabelGadget(parent, name, args, arg);

    /*
     * We need to be carefull here, since in case of gadgets, there is
     * no way to get the background color directly from the widget itself.
     * In such cases we get it from The Core part of his parent instead.
     */
    dsp = XtDisplayOfObject(label);
    scr = XtScreenOfObject(label);
    XtVaGetValues(XtIsSubclass(label, coreWidgetClass)
	    ?  label : XtParent(label),
		  XmNdepth, &depth,
		  XmNbackground, &color[0].pixel,
		  XmNforeground, &color[1].pixel,
		  XmNbottomShadowColor, &color[2].pixel,
		  XmNtopShadowColor, &color[3].pixel,
		  XmNhighlight, &color[4].pixel,
		  NULL);

    attr.valuemask = XpmColorSymbols | XpmCloseness | XpmDepth;
    attr.colorsymbols = color;
    attr.numsymbols = 5;
    attr.closeness = 65535;
    attr.depth = depth;
    XpmCreatePixmapFromData(dsp, RootWindowOfScreen(scr),
		    data, &pixmap, NULL, &attr);

    XtVaGetValues(label, XmNrecomputeSize, &rs, NULL);
    XtVaSetValues(label, XmNrecomputeSize, True, NULL);
    XtVaSetValues(label,
	    XmNlabelType, XmPIXMAP,
	    XmNlabelPixmap, pixmap,
	    NULL);
    XtVaSetValues(label, XmNrecomputeSize, rs, NULL);

    return label;
}
#endif

/* ARGSUSED */
    int
gui_mch_dialog(type, title, message, button_names, dfltbutton, textfield)
    int		type;
    char_u	*title;
    char_u	*message;
    char_u	*button_names;
    int		dfltbutton;
    char_u	*textfield;		/* buffer of size IOSIZE */
{
    char_u		*buts;
    char_u		*p, *next;
    XtAppContext	app;
    XmString		label;
    int			butcount;
    Widget		dialogform = NULL;
    Widget		form = NULL;
    Widget		dialogtextfield = NULL;
    Widget		*buttons;
    Widget		sep_form = NULL;
    Boolean		vertical;
    Widget		separator = NULL;
    int			n;
    Arg			args[6];
#ifdef HAVE_XPM
    char		**icon_data = NULL;
    Widget		dialogpixmap = NULL;
#endif

    if (title == NULL)
	title = (char_u *)_("Vim dialog");

    /* if our pointer is currently hidden, then we should show it. */
    gui_mch_mousehide(FALSE);

    dialogform = XmCreateFormDialog(vimShell, (char *)"dialog", NULL, 0);

    /* Check 'v' flag in 'guioptions': vertical button placement. */
    vertical = (vim_strchr(p_go, GO_VERTICAL) != NULL);

    /* Set the title of the Dialog window */
    label = XmStringCreateSimple((char *)title);
    if (label == NULL)
	return -1;
    XtVaSetValues(dialogform,
	    XmNdialogTitle, label,
	    XmNhorizontalSpacing, 4,
	    XmNverticalSpacing, vertical ? 0 : 4,
	    NULL);
    XmStringFree(label);

    /* make a copy, so that we can insert NULs */
    buts = vim_strsave(button_names);
    if (buts == NULL)
	return -1;

    /* Count the number of buttons and allocate buttons[]. */
    butcount = 1;
    for (p = buts; *p; ++p)
	if (*p == DLG_BUTTON_SEP)
	    ++butcount;
    buttons = (Widget *)alloc((unsigned)(butcount * sizeof(Widget)));
    if (buttons == NULL)
    {
	vim_free(buts);
	return -1;
    }

    /*
     * Create the buttons.
     */
    sep_form = (Widget) 0;
    p = buts;
    for (butcount = 0; *p; ++butcount)
    {
	for (next = p; *next; ++next)
	{
	    if (*next == DLG_HOTKEY_CHAR)
		mch_memmove(next, next + 1, STRLEN(next));
	    if (*next == DLG_BUTTON_SEP)
	    {
		*next++ = NUL;
		break;
	    }
	}
	label = XmStringCreate(_((char *)p), STRING_TAG);
	if (label == NULL)
	    break;

	buttons[butcount] = XtVaCreateManagedWidget("button",
		xmPushButtonWidgetClass, dialogform,
		XmNlabelString, label,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNbottomOffset, 4,
		XmNshowAsDefault, butcount == dfltbutton - 1,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	XmStringFree(label);

	/* Layout properly. */

	if (butcount > 0)
	{
	    if (vertical)
		XtVaSetValues(buttons[butcount],
			XmNtopWidget, buttons[butcount - 1],
			NULL);
	    else
	    {
		if (*next == NUL)
		{
		    XtVaSetValues(buttons[butcount],
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNrightOffset, 4,
			    NULL);

		    /* fill in a form as invisible separator */
		    sep_form = XtVaCreateWidget("separatorForm",
			    xmFormWidgetClass,	dialogform,
			    XmNleftAttachment, XmATTACH_WIDGET,
			    XmNleftWidget, buttons[butcount - 1],
			    XmNrightAttachment, XmATTACH_WIDGET,
			    XmNrightWidget, buttons[butcount],
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNbottomOffset, 4,
			    NULL);
		    XtManageChild(sep_form);
		}
		else
		{
		    XtVaSetValues(buttons[butcount],
			    XmNleftAttachment, XmATTACH_WIDGET,
			    XmNleftWidget, buttons[butcount - 1],
			    NULL);
		}
	    }
	}
	else if (!vertical)
	{
	    if (*next == NUL)
	    {
		XtVaSetValues(buttons[0],
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 4,
			NULL);

		/* fill in a form as invisible separator */
		sep_form = XtVaCreateWidget("separatorForm",
			xmFormWidgetClass, dialogform,
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 4,
			XmNrightAttachment, XmATTACH_WIDGET,
			XmNrightWidget, buttons[0],
			XmNbottomAttachment, XmATTACH_FORM,
			XmNbottomOffset, 4,
			NULL);
		XtManageChild(sep_form);
	    }
	    else
		XtVaSetValues(buttons[0],
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 4,
			NULL);
	}

	XtAddCallback(buttons[butcount], XmNactivateCallback,
			  (XtCallbackProc)butproc, (XtPointer)(long)butcount);
	p = next;
    }
    vim_free(buts);

    separator = (Widget) 0;
    if (butcount > 0)
    {
	/* Create the separator for beauty. */
	n = 0;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomWidget, buttons[0]); n++;
	XtSetArg(args[n], XmNbottomOffset, 4); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	separator = XmCreateSeparatorGadget(dialogform, "separator", args, n);
	XtManageChild(separator);
    }

    if (textfield != NULL)
    {
	dialogtextfield = XtVaCreateWidget("textField",
		xmTextFieldWidgetClass, dialogform,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);
	if (butcount > 0)
	    XtVaSetValues(dialogtextfield,
		    XmNbottomAttachment, XmATTACH_WIDGET,
		    XmNbottomWidget, separator,
		    NULL);
	else
	    XtVaSetValues(dialogtextfield,
		    XmNbottomAttachment, XmATTACH_FORM,
		    NULL);

	gui_motif_set_fontlist(dialogtextfield);
	XmTextFieldSetString(dialogtextfield, (char *)textfield);
	XtManageChild(dialogtextfield);
	XtAddEventHandler(dialogtextfield, KeyPressMask, False,
			    (XtEventHandler)keyhit_callback, (XtPointer)NULL);
    }

    /* Form holding both message and pixmap labels */
    form = XtVaCreateWidget("separatorForm",
	    xmFormWidgetClass, dialogform,
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNtopAttachment, XmATTACH_FORM,
	    NULL);
    XtManageChild(form);

#ifdef HAVE_XPM
    /* Add a pixmap, left of the message. */
    switch (type)
    {
	case VIM_GENERIC:
	    icon_data = generic_xpm;
	    break;
	case VIM_ERROR:
	    icon_data = error_xpm;
	    break;
	case VIM_WARNING:
	    icon_data = alert_xpm;
	    break;
	case VIM_INFO:
	    icon_data = info_xpm;
	    break;
	case VIM_QUESTION:
	    icon_data = quest_xpm;
	    break;
	default:
	    icon_data = generic_xpm;
    }

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 8); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 8); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 8); n++;

    dialogpixmap = create_pixmap_label(form, "dialogPixmap",
	    icon_data, args, n);
    XtManageChild(dialogpixmap);
#endif

    /* Create the dialog message. */
    label = XmStringLtoRCreate((char *)message, STRING_TAG);
    if (label == NULL)
	return -1;
    (void)XtVaCreateManagedWidget("dialogMessage",
				xmLabelGadgetClass, form,
				XmNlabelString, label,
				XmNtopAttachment, XmATTACH_FORM,
				XmNtopOffset, 8,
#ifdef HAVE_XPM
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, dialogpixmap,
#else
				XmNleftAttachment, XmATTACH_FORM,
#endif
				XmNleftOffset, 8,
				XmNrightAttachment, XmATTACH_FORM,
				XmNrightOffset, 8,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNbottomOffset, 8,
				NULL);
    XmStringFree(label);

    if (textfield != NULL)
    {
	XtVaSetValues(form,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, dialogtextfield,
		NULL);
    }
    else
    {
	if (butcount > 0)
	    XtVaSetValues(form,
		    XmNbottomAttachment, XmATTACH_WIDGET,
		    XmNbottomWidget, separator,
		    NULL);
	else
	    XtVaSetValues(form,
		    XmNbottomAttachment, XmATTACH_FORM,
		    NULL);
    }

    if (dfltbutton < 1)
	dfltbutton = 1;
    if (dfltbutton > butcount)
	dfltbutton = butcount;
    XtVaSetValues(dialogform,
	    XmNdefaultButton, buttons[dfltbutton - 1], NULL);
    if (textfield != NULL)
	XtVaSetValues(dialogform, XmNinitialFocus, dialogtextfield, NULL);
    else
	XtVaSetValues(dialogform, XmNinitialFocus, buttons[dfltbutton - 1],
									NULL);

    manage_centered(dialogform);

    if (textfield != NULL && *textfield != NUL)
    {
	/* This only works after the textfield has been realised. */
	XmTextFieldSetSelection(dialogtextfield,
			 (XmTextPosition)0, (XmTextPosition)STRLEN(textfield),
					   XtLastTimestampProcessed(gui.dpy));
	XmTextFieldSetCursorPosition(dialogtextfield,
					   (XmTextPosition)STRLEN(textfield));
    }

    app = XtWidgetToApplicationContext(dialogform);

    /* Loop until a button is pressed or the dialog is killed somehow. */
    dialogStatus = -1;
    for (;;)
    {
	XtAppProcessEvent(app, (XtInputMask)XtIMAll);
	if (dialogStatus >= 0 || !XtIsManaged(dialogform))
	    break;
    }

    vim_free(buttons);

    if (textfield != NULL)
    {
	p = (char_u *)XmTextGetString(dialogtextfield);
	if (p == NULL || dialogStatus < 0)
	    *textfield = NUL;
	else
	{
	    STRNCPY(textfield, p, IOSIZE);
	    textfield[IOSIZE - 1] = NUL;
	}
    }

    XtDestroyWidget(dialogform);

    return dialogStatus;
}
#endif /* FEAT_GUI_DIALOG */

#if defined(FEAT_FOOTER) || defined(PROTO)

    static int
gui_mch_compute_footer_height()
{
    Dimension	height;		    /* total Toolbar height */
    Dimension	top;		    /* XmNmarginTop */
    Dimension	bottom;		    /* XmNmarginBottom */
    Dimension	shadow;		    /* XmNshadowThickness */

    XtVaGetValues(footer,
	    XmNheight, &height,
	    XmNmarginTop, &top,
	    XmNmarginBottom, &bottom,
	    XmNshadowThickness, &shadow,
	    NULL);

    return (int) height + top + bottom + (shadow << 1);
}

#if 0	    /* not used */
    void
gui_mch_set_footer_pos(h)
    int	    h;			    /* textArea height */
{
    XtVaSetValues(footer,
		  XmNtopOffset, h + 7,
		  NULL);
}
#endif

    void
gui_mch_enable_footer(showit)
    int		showit;
{
    if (showit)
    {
	gui.footer_height = gui_mch_compute_footer_height();
	XtManageChild(footer);
    }
    else
    {
	gui.footer_height = 0;
	XtUnmanageChild(footer);
    }
    XtVaSetValues(textAreaForm, XmNbottomOffset, gui.footer_height, NULL);
}

    void
gui_mch_set_footer(s)
    char_u	*s;
{
    XmString	xms;

    xms = XmStringCreate((char *)s, STRING_TAG);
    XtVaSetValues(footer, XmNlabelString, xms, NULL);
    XmStringFree(xms);
}

#endif


#if defined(FEAT_TOOLBAR) || defined(PROTO)
    void
gui_mch_show_toolbar(int showit)
{
    Cardinal	numChildren;	    /* how many children toolBar has */

    if (toolBar == (Widget)0)
	return;
    XtVaGetValues(toolBar, XmNnumChildren, &numChildren, NULL);
    if (showit && numChildren > 0)
    {
	/* Assume that we want to show the toolbar if p_toolbar contains
	 * valid option settings, therefore p_toolbar must not be NULL.
	 */
	WidgetList  children;

	XtVaGetValues(toolBar, XmNchildren, &children, NULL);
	{
	    void    (*action)(BalloonEval *);
	    int	    text = 0;

	    if (strstr((const char *)p_toolbar, "tooltips"))
		action = &gui_mch_enable_beval_area;
	    else
		action = &gui_mch_disable_beval_area;
	    if (strstr((const char *)p_toolbar, "text"))
		text = 1;
	    else if (strstr((const char *)p_toolbar, "icons"))
		text = -1;
	    if (text != 0)
	    {
		vimmenu_T   *toolbar;
		vimmenu_T   *cur;

		for (toolbar = root_menu; toolbar; toolbar = toolbar->next)
		    if (menu_is_toolbar(toolbar->dname))
			break;
		/* Assumption: toolbar is NULL if there is no toolbar,
		 *	       otherwise it contains the toolbar menu structure.
		 *
		 * Assumption: "numChildren" == the number of items in the list
		 *	       of items beginning with toolbar->children.
		 */
		if (toolbar)
		{
		    for (cur = toolbar->children; cur; cur = cur->next)
		    {
			Arg	    args[1];
			int	    n = 0;

			/* Enable/Disable tooltip (OK to enable while
			 * currently enabled)
			 */
			if (cur->tip != NULL)
			    (*action)(cur->tip);
			if (!menu_is_separator(cur->name))
			{
			    if (text == 1 || cur->image == 0)
				XtSetArg(args[n], XmNlabelType, XmSTRING);
			    else
				XtSetArg(args[n], XmNlabelType, XmPIXMAP);
			    n++;
			    if (cur->id != NULL)
			    {
				XtUnmanageChild(cur->id);
				XtSetValues(cur->id, args, n);
				XtManageChild(cur->id);
			    }
			}
		    }
		}
	    }
	}
	gui.toolbar_height = gui_mch_compute_toolbar_height();
	XtManageChild(XtParent(toolBar));
	XtVaSetValues(textAreaForm,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, XtParent(toolBar),
		NULL);
	if (XtIsManaged(menuBar))
	    XtVaSetValues(XtParent(toolBar),
		    XmNtopAttachment, XmATTACH_WIDGET,
		    XmNtopWidget, menuBar,
		    NULL);
	else
	    XtVaSetValues(XtParent(toolBar),
		    XmNtopAttachment, XmATTACH_FORM,
		    NULL);
    }
    else
    {
	gui.toolbar_height = 0;
	if (XtIsManaged(menuBar))
	    XtVaSetValues(textAreaForm,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, menuBar,
		NULL);
	else
	    XtVaSetValues(textAreaForm,
		XmNtopAttachment, XmATTACH_FORM,
		NULL);

	XtUnmanageChild(XtParent(toolBar));
    }
    gui_set_shellsize(FALSE, FALSE);
}

/*
 * A toolbar button has been pushed; now reset the input focus
 * such that the user can type page up/down etc. and have the
 * input go to the editor window, not the button
 */
    static void
gui_mch_reset_focus()
{
    if (textArea != NULL)
	XmProcessTraversal(textArea, XmTRAVERSE_CURRENT);
}

    int
gui_mch_compute_toolbar_height()
{
    Dimension	height;		    /* total Toolbar height */
    Dimension	whgt;		    /* height of each widget */
    Dimension	marginHeight;	    /* XmNmarginHeight of toolBar */
    Dimension	shadowThickness;    /* thickness of Xtparent(toolBar) */
    WidgetList	children;	    /* list of toolBar's children */
    Cardinal	numChildren;	    /* how many children toolBar has */
    int		i;

    height = 0;
    shadowThickness = 0;
    marginHeight = 0;
    if (toolBar != (Widget)0 && toolBarFrame != (Widget)0)
    {				    /* get height of XmFrame parent */
	XtVaGetValues(toolBarFrame,
		XmNshadowThickness, &shadowThickness,
		NULL);
	XtVaGetValues(toolBar,
		XmNmarginHeight, &marginHeight,
		XmNchildren, &children,
		XmNnumChildren, &numChildren, NULL);
	for (i = 0; i < numChildren; i++)
	{
	    whgt = 0;
	    XtVaGetValues(children[i], XmNheight, &whgt, NULL);
	    if (height < whgt)
		height = whgt;
	}
    }

    return (int)(height + (marginHeight << 1) + (shadowThickness << 1));
}

#if 0 /* these are never called. */
/*
 * The next toolbar enter/leave callbacks make sure the text area gets the
 * keyboard focus when the pointer is not in the toolbar.
 */
/*ARGSUSED*/
    static void
toolbar_enter_cb(w, client_data, event, cont)
    Widget	w;
    XtPointer	client_data;
    XEvent	*event;
    Boolean	*cont;
{
    XmProcessTraversal(toolBar, XmTRAVERSE_CURRENT);
}

/*ARGSUSED*/
    static void
toolbar_leave_cb(w, client_data, event, cont)
    Widget	w;
    XtPointer	client_data;
    XEvent	*event;
    Boolean	*cont;
{
    XmProcessTraversal(textArea, XmTRAVERSE_CURRENT);
}
#endif

# ifdef FEAT_FOOTER
/*
 * The next toolbar enter/leave callbacks should really do balloon help.  But
 * I have to use footer help for backwards compatability.  Hopefully both will
 * get implemented and the user will have a choice.
 */
/*ARGSUSED*/
    static void
toolbarbutton_enter_cb(w, client_data, event, cont)
    Widget	w;
    XtPointer	client_data;
    XEvent	*event;
    Boolean	*cont;
{
    vimmenu_T	*menu = (vimmenu_T *) client_data;

    if (menu->strings[MENU_INDEX_TIP] != NULL)
    {
	if (vim_strchr(p_go, GO_FOOTER) != NULL)
	    gui_mch_set_footer(menu->strings[MENU_INDEX_TIP]);
    }
}

/*ARGSUSED*/
    static void
toolbarbutton_leave_cb(w, client_data, event, cont)
    Widget	w;
    XtPointer	client_data;
    XEvent	*event;
    Boolean	*cont;
{
    gui_mch_set_footer((char_u *) "");
}
# endif

    void
gui_mch_get_toolbar_colors(bgp, fgp, bsp, tsp, hsp)
    Pixel	*bgp;
    Pixel	*fgp;
    Pixel       *bsp;
    Pixel	*tsp;
    Pixel	*hsp;
{
    XtVaGetValues(toolBar,
	    XmNbackground, bgp,
	    XmNforeground, fgp,
	    XmNbottomShadowColor, bsp,
	    XmNtopShadowColor, tsp,
	    XmNhighlightColor, hsp,
	    NULL);
}
#endif

/*
 * Set the colors of Widget "id" to the menu colors.
 */
    static void
gui_motif_menu_colors(id)
    Widget  id;
{
    if (gui.menu_bg_pixel != INVALCOLOR)
#if (XmVersion >= 1002)
	XmChangeColor(id, gui.menu_bg_pixel);
#else
	XtVaSetValues(id, XmNbackground, gui.menu_bg_pixel, NULL);
#endif
    if (gui.menu_fg_pixel != INVALCOLOR)
	XtVaSetValues(id, XmNforeground, gui.menu_fg_pixel, NULL);
}

/*
 * Set the colors of Widget "id" to the scrollbar colors.
 */
    static void
gui_motif_scroll_colors(id)
    Widget  id;
{
    if (gui.scroll_bg_pixel != INVALCOLOR)
#if (XmVersion >= 1002)
	XmChangeColor(id, gui.scroll_bg_pixel);
#else
	XtVaSetValues(id, XmNbackground, gui.scroll_bg_pixel, NULL);
#endif
    if (gui.scroll_fg_pixel != INVALCOLOR)
	XtVaSetValues(id, XmNforeground, gui.scroll_fg_pixel, NULL);
}

#ifdef FEAT_MENU
/*
 * Set the fontlist for Widget "id" to use gui.menu_fontset or gui.menu_font.
 */
    static void
gui_motif_menu_fontlist(id)
    Widget  id;
{
#ifdef FONTSET_ALWAYS
    if (gui.menu_fontset != NOFONTSET)
    {
	XmFontList fl;

	fl = gui_motif_fontset2fontlist((XFontSet *)&gui.menu_fontset);
	if (fl != NULL)
	{
	    if (XtIsManaged(id))
	    {
		XtUnmanageChild(id);
		XtVaSetValues(id, XmNfontList, fl, NULL);
		/* We should force the widget to recalculate it's
		 * geometry now. */
		XtManageChild(id);
	    }
	    else
		XtVaSetValues(id, XmNfontList, fl, NULL);
	    XmFontListFree(fl);
	}
    }
#else
    if (gui.menu_font != NOFONT)
    {
	XmFontList fl;

	fl = gui_motif_create_fontlist((XFontStruct *)gui.menu_font);
	if (fl != NULL)
	{
	    if (XtIsManaged(id))
	    {
		XtUnmanageChild(id);
		XtVaSetValues(id, XmNfontList, fl, NULL);
		/* We should force the widget to recalculate it's
		 * geometry now. */
		XtManageChild(id);
	    }
	    else
		XtVaSetValues(id, XmNfontList, fl, NULL);
	    XmFontListFree(fl);
	}
    }
#endif
}

#endif

/*
 * We don't create it twice for the sake of speed.
 */

typedef struct _SharedFindReplace
{
    Widget dialog;	/* the main dialog widget */
    Widget wword;	/* 'Exact match' check button */
    Widget mcase;	/* 'match case' check button */
    Widget up;		/* search direction 'Up' radio button */
    Widget down;	/* search direction 'Down' radio button */
    Widget what;	/* 'Find what' entry text widget */
    Widget with;	/* 'Replace with' entry text widget */
    Widget find;	/* 'Find Next' action button */
    Widget replace;	/* 'Replace With' action button */
    Widget all;		/* 'Replace All' action button */
    Widget undo;	/* 'Undo' action button */

    Widget cancel;
} SharedFindReplace;

static SharedFindReplace find_widgets = { NULL };
static SharedFindReplace repl_widgets = { NULL };

static void find_replace_destroy_callback __ARGS((Widget w, XtPointer client_data, XtPointer call_data));
static void find_replace_dismiss_callback __ARGS((Widget w, XtPointer client_data, XtPointer call_data));
static void entry_activate_callback __ARGS((Widget w, XtPointer client_data, XtPointer call_data));
static void find_replace_callback __ARGS((Widget w, XtPointer client_data, XtPointer call_data));
static void find_replace_keypress __ARGS((Widget w, SharedFindReplace * frdp, XKeyEvent * event));
static void find_replace_dialog_create __ARGS((char_u *entry_text, int do_replace));

/*ARGSUSED*/
    static void
find_replace_destroy_callback(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    SharedFindReplace *cd = (SharedFindReplace *)client_data;

    if (cd != NULL)
	cd->dialog = (Widget)0;
}

/*ARGSUSED*/
    static void
find_replace_dismiss_callback(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    SharedFindReplace *cd = (SharedFindReplace *)client_data;

    if (cd != NULL)
	XtUnmanageChild(cd->dialog);
}

/*ARGSUSED*/
    static void
entry_activate_callback(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    XmProcessTraversal((Widget)client_data, XmTRAVERSE_CURRENT);
}

/*ARGSUSED*/
    static void
find_replace_callback(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    long_u	flags = (long_u)client_data;
    char	*find_text, *repl_text;
    Boolean	direction_down = TRUE;
    Boolean	wword;
    Boolean	mcase;
    SharedFindReplace *sfr;

    if (flags == FRD_UNDO)
    {
	char_u	*save_cpo = p_cpo;

	/* No need to be Vi compatible here. */
	p_cpo = (char_u *)"";
	u_undo(1);
	p_cpo = save_cpo;
	gui_update_screen();
	return;
    }

    /* Get the search/replace strings from the dialog */
    if (flags == FRD_FINDNEXT)
    {
	repl_text = NULL;
	sfr = &find_widgets;
    }
    else
    {
	repl_text = XmTextFieldGetString(repl_widgets.with);
	sfr = &repl_widgets;
    }
    find_text = XmTextFieldGetString(sfr->what);
    XtVaGetValues(sfr->down, XmNset, &direction_down, NULL);
    XtVaGetValues(sfr->wword, XmNset, &wword, NULL);
    XtVaGetValues(sfr->mcase, XmNset, &mcase, NULL);
    if (wword)
	flags |= FRD_WHOLE_WORD;
    if (mcase)
	flags |= FRD_MATCH_CASE;

    (void)gui_do_findrepl((int)flags, (char_u *)find_text, (char_u *)repl_text,
							      direction_down);

    if (find_text != NULL)
	XtFree(find_text);
    if (repl_text != NULL)
	XtFree(repl_text);
}

/*ARGSUSED*/
    static void
find_replace_keypress(w, frdp, event)
    Widget		w;
    SharedFindReplace	*frdp;
    XKeyEvent		*event;
{
    KeySym keysym;

    if (frdp == NULL)
	return;

    keysym = XLookupKeysym(event, 0);

    /* the scape key pops the whole dialog down */
    if (keysym == XK_Escape)
	XtUnmanageChild(frdp->dialog);
}

    static void
find_replace_dialog_create(arg, do_replace)
    char_u	*arg;
    int		do_replace;
{
    SharedFindReplace	*frdp;
    Widget		separator;
    Widget		input_form;
    Widget		button_form;
    Widget		toggle_form;
    Widget		frame;
    XmString		str;
    int			n;
    Arg			args[6];
    int			wword = FALSE;
    int			mcase = !p_ic;
    Dimension		width;
    Dimension		widest;
    char_u		*entry_text;

    frdp = do_replace ? &repl_widgets : &find_widgets;

    /* Get the search string to use. */
    entry_text = get_find_dialog_text(arg, &wword, &mcase);

    /* If the dialog already exists, just raise it. */
    if (frdp->dialog)
    {
	/* If the window is already up, just pop it to the top */
	if (XtIsManaged(frdp->dialog))
	    XMapRaised(XtDisplay(frdp->dialog),
					    XtWindow(XtParent(frdp->dialog)));
	else
	    XtManageChild(frdp->dialog);
	XtPopup(XtParent(frdp->dialog), XtGrabNone);
	XmProcessTraversal(frdp->what, XmTRAVERSE_CURRENT);

	if (entry_text != NULL)
	    XmTextFieldSetString(frdp->what, (char *)entry_text);
	vim_free(entry_text);

	XtVaSetValues(frdp->wword, XmNset, wword, NULL);
	return;
    }

    /* Create a fresh new dialog window */
    if (do_replace)
	 str = XmStringCreateSimple(_("VIM - Search and Replace..."));
    else
	 str = XmStringCreateSimple(_("VIM - Search..."));

    n = 0;
    XtSetArg(args[n], XmNautoUnmanage, False); n++;
    XtSetArg(args[n], XmNnoResize, True); n++;
    XtSetArg(args[n], XmNdialogTitle, str); n++;

    frdp->dialog = XmCreateFormDialog(vimShell, "findReplaceDialog", args, n);
    XmStringFree(str);
    XtAddCallback(frdp->dialog, XmNdestroyCallback,
	    find_replace_destroy_callback, frdp);

    button_form = XtVaCreateWidget("buttonForm",
	    xmFormWidgetClass,	frdp->dialog,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNrightOffset, 4,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNtopOffset, 4,
	    XmNbottomAttachment, XmATTACH_FORM,
	    XmNbottomOffset, 4,
	    NULL);

    str = XmStringCreateSimple(_("Find Next"));
    frdp->find = XtVaCreateManagedWidget("findButton",
	    xmPushButtonWidgetClass, button_form,
	    XmNlabelString, str,
	    XmNsensitive, True,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_FORM,
	    NULL);
    XmStringFree(str);

    XtAddCallback(frdp->find, XmNactivateCallback,
	    find_replace_callback,
	    (XtPointer) (do_replace ? FRD_R_FINDNEXT : FRD_FINDNEXT));

    if (do_replace)
    {
	str = XmStringCreateSimple(_("Replace"));
	frdp->replace = XtVaCreateManagedWidget("replaceButton",
		xmPushButtonWidgetClass, button_form,
		XmNlabelString, str,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, frdp->find,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);
	XmStringFree(str);
	XtAddCallback(frdp->replace, XmNactivateCallback,
		find_replace_callback, (XtPointer)FRD_REPLACE);

	str = XmStringCreateSimple(_("Replace All"));
	frdp->all = XtVaCreateManagedWidget("replaceAllButton",
		xmPushButtonWidgetClass, button_form,
		XmNlabelString, str,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, frdp->replace,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);
	XmStringFree(str);
	XtAddCallback(frdp->all, XmNactivateCallback,
		find_replace_callback, (XtPointer)FRD_REPLACEALL);

	str = XmStringCreateSimple(_("Undo"));
	frdp->undo = XtVaCreateManagedWidget("undoButton",
		xmPushButtonWidgetClass, button_form,
		XmNlabelString, str,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, frdp->all,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);
	XmStringFree(str);
	XtAddCallback(frdp->undo, XmNactivateCallback,
		find_replace_callback, (XtPointer)FRD_UNDO);
    }

    str = XmStringCreateSimple(_("Cancel"));
    frdp->cancel = XtVaCreateManagedWidget("closeButton",
	    xmPushButtonWidgetClass, button_form,
	    XmNlabelString, str,
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM,
	    NULL);
    XmStringFree(str);
    XtAddCallback(frdp->cancel, XmNactivateCallback,
	    find_replace_dismiss_callback, frdp);

    XtManageChild(button_form);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, button_form); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    separator = XmCreateSeparatorGadget(frdp->dialog, "separator", args, n);
    XtManageChild(separator);

    input_form = XtVaCreateWidget("inputForm",
	    xmFormWidgetClass,	frdp->dialog,
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNleftOffset, 4,
	    XmNrightAttachment, XmATTACH_WIDGET,
	    XmNrightWidget, separator,
	    XmNrightOffset, 4,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNtopOffset, 4,
	    NULL);

    {
	Widget label_what;
	Widget label_with = (Widget)0;

	str = XmStringCreateSimple(_("Find what:"));
	label_what = XtVaCreateManagedWidget("whatLabel",
		xmLabelGadgetClass, input_form,
		XmNlabelString, str,
		XmNleftAttachment,	XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		XmNtopOffset, 4,
		NULL);
	XmStringFree(str);

	frdp->what = XtVaCreateManagedWidget("whatText",
		xmTextFieldWidgetClass, input_form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		NULL);

	if (do_replace)
	{
	    frdp->with = XtVaCreateManagedWidget("withText",
		    xmTextFieldWidgetClass,	input_form,
		    XmNtopAttachment, XmATTACH_WIDGET,
		    XmNtopWidget, frdp->what,
		    XmNtopOffset, 4,
		    XmNleftAttachment, XmATTACH_FORM,
		    XmNrightAttachment, XmATTACH_FORM,
		    XmNbottomAttachment, XmATTACH_FORM,
		    NULL);

	    XtAddCallback(frdp->with, XmNactivateCallback,
		    find_replace_callback, (XtPointer) FRD_R_FINDNEXT);

	    str = XmStringCreateSimple(_("Replace with:"));
	    label_with = XtVaCreateManagedWidget("withLabel",
		    xmLabelGadgetClass, input_form,
		    XmNlabelString, str,
		    XmNleftAttachment, XmATTACH_FORM,
		    XmNtopAttachment, XmATTACH_WIDGET,
		    XmNtopWidget, frdp->what,
		    XmNtopOffset, 4,
		    XmNbottomAttachment, XmATTACH_FORM,
		    NULL);
	    XmStringFree(str);

	    /*
	     * Make the entry activation only change the input focus onto the
	     * with item.
	     */
	    XtAddCallback(frdp->what, XmNactivateCallback,
		    entry_activate_callback, frdp->with);
	    XtAddEventHandler(frdp->with, KeyPressMask, False,
			    (XtEventHandler)find_replace_keypress,
			    (XtPointer) frdp);

	}
	else
	{
	    /*
	     * Make the entry activation do the search.
	     */
	    XtAddCallback(frdp->what, XmNactivateCallback,
		    find_replace_callback, (XtPointer)FRD_FINDNEXT);
	}
	XtAddEventHandler(frdp->what, KeyPressMask, False,
			    (XtEventHandler)find_replace_keypress,
			    (XtPointer)frdp);

	/* Get the maximum width between the label widgets and line them up.
	 */
	n = 0;
	XtSetArg(args[n], XmNwidth, &width); n++;
	XtGetValues(label_what, args, n);
	widest = width;
	if (do_replace)
	{
	    XtGetValues(label_with, args, n);
	    if (width > widest)
		widest = width;
	}

	XtVaSetValues(frdp->what, XmNleftOffset, widest, NULL);
	if (do_replace)
	    XtVaSetValues(frdp->with, XmNleftOffset, widest, NULL);

    }

    XtManageChild(input_form);

    {
	Widget radio_box;

	frame = XtVaCreateWidget("directionFrame",
		xmFrameWidgetClass, frdp->dialog,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, input_form,
		XmNtopOffset, 4,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNbottomOffset, 4,
		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		XmNrightWidget, input_form,
		NULL);

	str = XmStringCreateSimple(_("Direction"));
	(void)XtVaCreateManagedWidget("directionFrameLabel",
		xmLabelGadgetClass, frame,
		XmNlabelString, str,
		XmNchildHorizontalAlignment, XmALIGNMENT_BEGINNING,
		XmNchildType, XmFRAME_TITLE_CHILD,
		NULL);
	XmStringFree(str);

	radio_box = XmCreateRadioBox(frame, "radioBox",
		(ArgList)NULL, 0);

	str = XmStringCreateSimple( _("Up"));
	frdp->up = XtVaCreateManagedWidget("upRadioButton",
		xmToggleButtonGadgetClass, radio_box,
		XmNlabelString, str,
		XmNset, False,
		NULL);
	XmStringFree(str);

	str = XmStringCreateSimple(_("Down"));
	frdp->down = XtVaCreateManagedWidget("downRadioButton",
		xmToggleButtonGadgetClass, radio_box,
		XmNlabelString, str,
		XmNset, True,
		NULL);
	XmStringFree(str);

	XtManageChild(radio_box);
	XtManageChild(frame);
    }

    toggle_form = XtVaCreateWidget("toggleForm",
	    xmFormWidgetClass,	frdp->dialog,
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNleftOffset, 4,
	    XmNrightAttachment, XmATTACH_WIDGET,
	    XmNrightWidget, frame,
	    XmNrightOffset, 4,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, input_form,
	    XmNtopOffset, 4,
	    XmNbottomAttachment, XmATTACH_FORM,
	    XmNbottomOffset, 4,
	    NULL);

    str = XmStringCreateSimple(_("Match whole word only"));
    frdp->wword = XtVaCreateManagedWidget("wordToggle",
	    xmToggleButtonGadgetClass, toggle_form,
	    XmNlabelString, str,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNtopOffset, 4,
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNleftOffset, 4,
	    XmNset, wword,
	    NULL);
    XmStringFree(str);

    str = XmStringCreateSimple(_("Match case"));
    frdp->mcase = XtVaCreateManagedWidget("caseToggle",
	    xmToggleButtonGadgetClass, toggle_form,
	    XmNlabelString, str,
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNleftOffset, 4,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, frdp->wword,
	    XmNtopOffset, 4,
	    XmNset, mcase,
	    NULL);
    XmStringFree(str);

    XtManageChild(toggle_form);

    if (entry_text != NULL)
	XmTextFieldSetString(frdp->what, (char *)entry_text);
    vim_free(entry_text);

    XtManageChild(frdp->dialog);
    XmProcessTraversal(frdp->what, XmTRAVERSE_CURRENT);
}

   void
gui_mch_find_dialog(eap)
    exarg_T	*eap;
{
    if (!gui.in_use)
	return;

    find_replace_dialog_create(eap->arg, FALSE);
}


    void
gui_mch_replace_dialog(eap)
    exarg_T	*eap;
{
    if (!gui.in_use)
	return;

    find_replace_dialog_create(eap->arg, TRUE);
}
