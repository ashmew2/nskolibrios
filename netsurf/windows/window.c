/*
 * Copyright 2011 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils/config.h"

#include <stdbool.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "utils/errors.h"
#include "utils/log.h"
#include "utils/utils.h"
#include "utils/nsoption.h"
#include "utils/nsurl.h"
#include "utils/messages.h"
#include "content/content.h"
#include "desktop/browser.h"
#include "desktop/gui_window.h"
#include "desktop/browser_history.h"
#include "desktop/textinput.h"

#include "windows/gui.h"
#include "windows/pointers.h"
#include "windows/about.h"
#include "windows/resourceid.h"
#include "windows/findfile.h"
#include "windows/windbg.h"
#include "windows/drawable.h"
#include "windows/font.h"
#include "windows/prefs.h"
#include "windows/localhistory.h"
#include "windows/window.h"

/** List of all our gui windows */
static struct gui_window *window_list = NULL;

/** The main window class name */
static const char windowclassname_main[] = "nswsmainwindow";

/** width of the throbber element */
#define NSWS_THROBBER_WIDTH 24

/** Number of open windows */
static int open_windows = 0;


/**
 * Obtain the DPI of the display.
 */
static int get_window_dpi(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	int dpi = GetDeviceCaps(hdc, LOGPIXELSY);

	if (dpi <= 10) {
		dpi = 96; /* 96DPI is the default */
	}

	ReleaseDC(hwnd, hdc);

	LOG("FIX DPI %d", dpi);

	return dpi;
}


/**
 * set accelerators
 */
static void nsws_window_set_accels(struct gui_window *w)
{
	int i, nitems = 13;
	ACCEL accels[nitems];

	for (i = 0; i < nitems; i++) {
		accels[i].fVirt = FCONTROL | FVIRTKEY;
	}

	accels[0].key = 0x51; /* Q */
	accels[0].cmd = IDM_FILE_QUIT;
	accels[1].key = 0x4E; /* N */
	accels[1].cmd = IDM_FILE_OPEN_WINDOW;
	accels[2].key = VK_LEFT;
	accels[2].cmd = IDM_NAV_BACK;
	accels[3].key = VK_RIGHT;
	accels[3].cmd = IDM_NAV_FORWARD;
	accels[4].key = VK_UP;
	accels[4].cmd = IDM_NAV_HOME;
	accels[5].key = VK_BACK;
	accels[5].cmd = IDM_NAV_STOP;
	accels[6].key = VK_SPACE;
	accels[6].cmd = IDM_NAV_RELOAD;
	accels[7].key = 0x4C; /* L */
	accels[7].cmd = IDM_FILE_OPEN_LOCATION;
	accels[8].key = 0x57; /* w */
	accels[8].cmd = IDM_FILE_CLOSE_WINDOW;
	accels[9].key = 0x41; /* A */
	accels[9].cmd = IDM_EDIT_SELECT_ALL;
	accels[10].key = VK_F8;
	accels[10].cmd = IDM_VIEW_SOURCE;
	accels[11].key = VK_RETURN;
	accels[11].fVirt = FVIRTKEY;
	accels[11].cmd = IDC_MAIN_LAUNCH_URL;
	accels[12].key = VK_F11;
	accels[12].fVirt = FVIRTKEY;
	accels[12].cmd = IDM_VIEW_FULLSCREEN;

	w->acceltable = CreateAcceleratorTable(accels, nitems);
}


/**
 * creation of a new full browser window
 */
static HWND nsws_window_create(struct gui_window *gw)
{
	HWND hwnd;
	INITCOMMONCONTROLSEX icc;

	LOG("GUI window %p", gw);

	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_BAR_CLASSES | ICC_WIN95_CLASSES;
#if WINVER > 0x0501
	icc.dwICC |= ICC_STANDARD_CLASSES;
#endif
	InitCommonControlsEx(&icc);

	gw->mainmenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_MAIN));
	gw->rclick = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_CONTEXT));

	LOG("creating window for hInstance %p", hInstance);
	hwnd = CreateWindowEx(0,
			      windowclassname_main,
			      "NetSurf Browser",
			      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CS_DBLCLKS,
			      CW_USEDEFAULT,
			      CW_USEDEFAULT,
			      gw->width,
			      gw->height,
			      NULL,
			      gw->mainmenu,
			      hInstance,
			      NULL);

	if (hwnd == NULL) {
		LOG("Window create failed");
		return NULL;
	}

	/* set the gui window associated with this browser */
	SetProp(hwnd, TEXT("GuiWnd"), (HANDLE)gw);

	browser_set_dpi(get_window_dpi(hwnd));

	if ((nsoption_int(window_width) >= 100) &&
	    (nsoption_int(window_height) >= 100) &&
	    (nsoption_int(window_x) >= 0) &&
	    (nsoption_int(window_y) >= 0)) {
		LOG("Setting Window position %d,%d %d,%d", nsoption_int(window_x), nsoption_int(window_y), nsoption_int(window_width), nsoption_int(window_height));
		SetWindowPos(hwnd, HWND_TOP,
			     nsoption_int(window_x), nsoption_int(window_y),
			     nsoption_int(window_width), nsoption_int(window_height),
			     SWP_SHOWWINDOW);
	}

	nsws_window_set_accels(gw);

	return hwnd;
}


/**
 * calculate the dimensions of the url bar relative to the parent toolbar
 */
static void
urlbar_dimensions(HWND hWndParent,
		  int toolbuttonsize,
		  int buttonc,
		  int *x,
		  int *y,
		  int *width,
		  int *height)
{
	RECT rc;
	const int cy_edit = 23;

	GetClientRect(hWndParent, &rc);
	*x = (toolbuttonsize + 1) * (buttonc + 1) + (NSWS_THROBBER_WIDTH>>1);
	*y = ((((rc.bottom - 1) - cy_edit) >> 1) * 2) / 3;
	*width = (rc.right - 1) - *x - (NSWS_THROBBER_WIDTH>>1) - NSWS_THROBBER_WIDTH;
	*height = cy_edit;
}



static LRESULT
nsws_window_toolbar_command(struct gui_window *gw,
		    int notification_code,
		    int identifier,
		    HWND ctrl_window)
{
	LOG("notification_code %d identifier %d ctrl_window %p", notification_code, identifier, ctrl_window);

	switch(identifier) {

	case IDC_MAIN_URLBAR:
		switch (notification_code) {
		case EN_CHANGE:
			LOG("EN_CHANGE");
			break;

		case EN_ERRSPACE:
			LOG("EN_ERRSPACE");
			break;

		case EN_HSCROLL:
			LOG("EN_HSCROLL");
			break;

		case EN_KILLFOCUS:
			LOG("EN_KILLFOCUS");
			break;

		case EN_MAXTEXT:
			LOG("EN_MAXTEXT");
			break;

		case EN_SETFOCUS:
			LOG("EN_SETFOCUS");
			break;

		case EN_UPDATE:
			LOG("EN_UPDATE");
			break;

		case EN_VSCROLL:
			LOG("EN_VSCROLL");
			break;

		default:
			LOG("Unknown notification_code");
			break;
		}
		break;

	default:
		return 1; /* unhandled */

	}
	return 0; /* control message handled */
}


/**
 * callback for toolbar events
 */
static LRESULT CALLBACK
nsws_window_toolbar_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	struct gui_window *gw;
	int urlx, urly, urlwidth, urlheight;
	WNDPROC toolproc;

	LOG_WIN_MSG(hwnd, msg, wparam, lparam);

	gw = nsws_get_gui_window(hwnd);

	switch (msg) {
	case WM_SIZE:
		urlbar_dimensions(hwnd,
				  gw->toolbuttonsize,
				  gw->toolbuttonc,
				  &urlx, &urly, &urlwidth, &urlheight);

		/* resize url */
		if (gw->urlbar != NULL) {
			MoveWindow(gw->urlbar,
				   urlx, urly,
				   urlwidth, urlheight,
				   true);
		}

		/* move throbber */
		if (gw->throbber != NULL) {
			MoveWindow(gw->throbber,
				   LOWORD(lparam) - NSWS_THROBBER_WIDTH - 4, 8,
				   NSWS_THROBBER_WIDTH, NSWS_THROBBER_WIDTH,
				   true);
		}
		break;

	case WM_COMMAND:
		if (nsws_window_toolbar_command(gw,
						HIWORD(wparam),
						LOWORD(wparam),
						(HWND)lparam) == 0) {
			return 0;
		}
		break;
	}

	/* remove properties if window is being destroyed */
	if (msg == WM_NCDESTROY) {
		RemoveProp(hwnd, TEXT("GuiWnd"));
		toolproc = (WNDPROC)RemoveProp(hwnd, TEXT("OrigMsgProc"));
	} else {
		toolproc = (WNDPROC)GetProp(hwnd, TEXT("OrigMsgProc"));
	}

	if (toolproc == NULL) {
		/* the original toolbar procedure is not available */
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	/* chain to the next handler */
	return CallWindowProc(toolproc, hwnd, msg, wparam, lparam);
}


static HIMAGELIST
get_imagelist(int resid, int bsize, int bcnt)
{
	HIMAGELIST hImageList;
	HBITMAP hScrBM;

	LOG("resource id %d, bzize %d, bcnt %d", resid, bsize, bcnt);

	hImageList = ImageList_Create(bsize, bsize, ILC_COLOR24 | ILC_MASK, 0,
				      bcnt);
	if (hImageList == NULL)
		return NULL;

	hScrBM = LoadImage(hInstance, MAKEINTRESOURCE(resid),
			   IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

	if (hScrBM == NULL) {
		win_perror("LoadImage");
		return NULL;
	}

	if (ImageList_AddMasked(hImageList, hScrBM, 0xcccccc) == -1) {
		/* failed to add masked bitmap */
		ImageList_Destroy(hImageList);
		hImageList = NULL;
	}
	DeleteObject(hScrBM);

	return hImageList;
}


/**
 * callback for url bar events
 */
static LRESULT CALLBACK
nsws_window_urlbar_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	struct gui_window *gw;
	WNDPROC urlproc;
	HFONT hFont;

	LOG_WIN_MSG(hwnd, msg, wparam, lparam);

	gw = nsws_get_gui_window(hwnd);

	urlproc = (WNDPROC)GetProp(hwnd, TEXT("OrigMsgProc"));

	/* override messages */
	switch (msg) {
	case WM_CHAR:
		if (wparam == 13) {
			SendMessage(gw->main, WM_COMMAND, IDC_MAIN_LAUNCH_URL, 0);
			return 0;
		}
		break;

	case WM_DESTROY:
		hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
		if (hFont != NULL) {
			LOG("Destroyed font object");
			DeleteObject(hFont);
		}


	case WM_NCDESTROY:
		/* remove properties if window is being destroyed */
		RemoveProp(hwnd, TEXT("GuiWnd"));
		RemoveProp(hwnd, TEXT("OrigMsgProc"));
		break;
	}

	if (urlproc == NULL) {
		/* the original toolbar procedure is not available */
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	/* chain to the next handler */
	return CallWindowProc(urlproc, hwnd, msg, wparam, lparam);
}


/**
 * create a urlbar and message handler
 *
 * Create an Edit control for enerting urls
 */
static HWND
nsws_window_urlbar_create(struct gui_window *gw, HWND hwndparent)
{
	int urlx, urly, urlwidth, urlheight;
	HWND hwnd;
	WNDPROC	urlproc;
	HFONT hFont;

	urlbar_dimensions(hwndparent,
			  gw->toolbuttonsize,
			  gw->toolbuttonc,
			  &urlx, &urly, &urlwidth, &urlheight);

	/* Create the edit control */
	hwnd = CreateWindowEx(0L,
			      TEXT("Edit"),
			      NULL,
			      WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
			      urlx,
			      urly,
			      urlwidth,
			      urlheight,
			      hwndparent,
			      (HMENU)IDC_MAIN_URLBAR,
			      hInstance,
			      0);

	if (hwnd == NULL) {
		return NULL;
	}

	/* set the gui window associated with this control */
	SetProp(hwnd, TEXT("GuiWnd"), (HANDLE)gw);

	/* subclass the message handler */
	urlproc = (WNDPROC)SetWindowLongPtr(hwnd,
					    GWLP_WNDPROC,
					    (LONG_PTR)nsws_window_urlbar_callback);

	/* save the real handler  */
	SetProp(hwnd, TEXT("OrigMsgProc"), (HANDLE)urlproc);

	hFont = CreateFont(urlheight - 4, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
	if (hFont != NULL) {
		LOG("Setting font object");
		SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, 0);
	}

	LOG("Created url bar hwnd:%p, x:%d, y:%d, w:%d, h:%d", hwnd, urlx, urly, urlwidth, urlheight);

	return hwnd;
}


/**
 * creation of throbber
 */
static HWND
nsws_window_throbber_create(struct gui_window *w)
{
	HWND hwnd;
	char avi[PATH_MAX];

	hwnd = CreateWindow(ANIMATE_CLASS,
			    "",
			    WS_CHILD | WS_VISIBLE | ACS_TRANSPARENT,
			    w->width - NSWS_THROBBER_WIDTH - 4,
			    8,
			    NSWS_THROBBER_WIDTH,
			    NSWS_THROBBER_WIDTH,
			    w->main,
			    (HMENU) IDC_MAIN_THROBBER,
			    hInstance,
			    NULL);

	nsws_find_resource(avi, "throbber.avi", "windows/res/throbber.avi");
	LOG("setting throbber avi as %s", avi);
	Animate_Open(hwnd, avi);
	if (w->throbbing)
		Animate_Play(hwnd, 0, -1, -1);
	else
		Animate_Seek(hwnd, 0);
	ShowWindow(hwnd, SW_SHOWNORMAL);
	return hwnd;
}


/* create a toolbar add controls and message handler */
static HWND
nsws_window_create_toolbar(struct gui_window *gw, HWND hWndParent)
{
	HIMAGELIST hImageList;
	HWND hWndToolbar;
	/* Toolbar buttons */
	TBBUTTON tbButtons[] = {
		{0, IDM_NAV_BACK, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
		{1, IDM_NAV_FORWARD, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
		{2, IDM_NAV_HOME, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
		{3, IDM_NAV_RELOAD, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
		{4, IDM_NAV_STOP, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
	};
	WNDPROC	toolproc;

	/* Create the toolbar window and subclass its message handler. */
	hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, "Toolbar",
				     WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT,
				     0, 0, 0, 0,
				     hWndParent, NULL, HINST_COMMCTRL, NULL);
	if (!hWndToolbar) {
		return NULL;
	}

	/* set the gui window associated with this toolbar */
	SetProp(hWndToolbar, TEXT("GuiWnd"), (HANDLE)gw);

	/* subclass the message handler */
	toolproc = (WNDPROC)SetWindowLongPtr(hWndToolbar,
				GWLP_WNDPROC,
				(LONG_PTR)nsws_window_toolbar_callback);

	/* save the real handler  */
	SetProp(hWndToolbar, TEXT("OrigMsgProc"), (HANDLE)toolproc);

	/* remember how many buttons are being created */
	gw->toolbuttonc = sizeof(tbButtons) / sizeof(TBBUTTON);

	/* Create the standard image list and assign to toolbar. */
	hImageList = get_imagelist(IDR_TOOLBAR_BITMAP,
				   gw->toolbuttonsize,
				   gw->toolbuttonc);
	if (hImageList != NULL) {
		SendMessage(hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
	}

	/* Create the disabled image list and assign to toolbar. */
	hImageList = get_imagelist(IDR_TOOLBAR_BITMAP_GREY,
				   gw->toolbuttonsize,
				   gw->toolbuttonc);
	if (hImageList != NULL) {
		SendMessage(hWndToolbar, TB_SETDISABLEDIMAGELIST, 0,
			    (LPARAM)hImageList);
	}

	/* Create the hot image list and assign to toolbar. */
	hImageList = get_imagelist(IDR_TOOLBAR_BITMAP_HOT,
				   gw->toolbuttonsize,
				   gw->toolbuttonc);
	if (hImageList != NULL) {
		SendMessage(hWndToolbar, TB_SETHOTIMAGELIST, 0,
			    (LPARAM)hImageList);
	}

	/* Add buttons. */
	SendMessage(hWndToolbar,
		    TB_BUTTONSTRUCTSIZE,
		    (WPARAM)sizeof(TBBUTTON),
		    0);
	SendMessage(hWndToolbar,
		    TB_ADDBUTTONS,
		    (WPARAM)gw->toolbuttonc,
		    (LPARAM)&tbButtons);

	gw->urlbar = nsws_window_urlbar_create(gw, hWndToolbar);

	gw->throbber = nsws_window_throbber_create(gw);

	return hWndToolbar;
}


/**
 * creation of status bar
 */
static HWND nsws_window_create_statusbar(struct gui_window *w)
{
	HWND hwnd = CreateWindowEx(0,
				   STATUSCLASSNAME,
				   NULL,
				   WS_CHILD | WS_VISIBLE,
				   0, 0, 0, 0,
				   w->main,
				   (HMENU)IDC_MAIN_STATUSBAR,
				   hInstance,
				   NULL);
	SendMessage(hwnd, SB_SETTEXT, 0, (LPARAM)"NetSurf");
	return hwnd;
}


static void nsws_update_edit(struct gui_window *w)
{
	browser_editor_flags editor_flags = (w->bw == NULL) ?
			BW_EDITOR_NONE : browser_window_get_editor_flags(w->bw);
	bool paste, copy, del;
	bool sel = (editor_flags & BW_EDITOR_CAN_COPY);
	if (GetFocus() == w->urlbar) {
		DWORD i, ii;
		SendMessage(w->urlbar, EM_GETSEL, (WPARAM)&i, (LPARAM)&ii);
		paste = true;
		copy = (i != ii);
		del = (i != ii);

	} else if (sel){
		paste = (editor_flags & BW_EDITOR_CAN_PASTE);
		copy = sel;
		del = (editor_flags & BW_EDITOR_CAN_CUT);
	} else {
		paste = false;
		copy = false;
		del = false;
	}
	EnableMenuItem(w->mainmenu,
		       IDM_EDIT_PASTE,
		       (paste ? MF_ENABLED : MF_GRAYED));

	EnableMenuItem(w->rclick,
		       IDM_EDIT_PASTE,
		       (paste ? MF_ENABLED : MF_GRAYED));

	EnableMenuItem(w->mainmenu,
		       IDM_EDIT_COPY,
		       (copy ? MF_ENABLED : MF_GRAYED));

	EnableMenuItem(w->rclick,
		       IDM_EDIT_COPY,
		       (copy ? MF_ENABLED : MF_GRAYED));

	if (del == true) {
		EnableMenuItem(w->mainmenu, IDM_EDIT_CUT, MF_ENABLED);
		EnableMenuItem(w->mainmenu, IDM_EDIT_DELETE, MF_ENABLED);
		EnableMenuItem(w->rclick, IDM_EDIT_CUT, MF_ENABLED);
		EnableMenuItem(w->rclick, IDM_EDIT_DELETE, MF_ENABLED);
	} else {
		EnableMenuItem(w->mainmenu, IDM_EDIT_CUT, MF_GRAYED);
		EnableMenuItem(w->mainmenu, IDM_EDIT_DELETE, MF_GRAYED);
		EnableMenuItem(w->rclick, IDM_EDIT_CUT, MF_GRAYED);
		EnableMenuItem(w->rclick, IDM_EDIT_DELETE, MF_GRAYED);
	}
}


static bool
nsws_ctx_menu(struct gui_window *w, HWND hwnd, int x, int y)
{
	RECT rc; /* client area of window */
	POINT pt = { x, y }; /* location of mouse click */

	/* Get the bounding rectangle of the client area. */
	GetClientRect(hwnd, &rc);

	/* Convert the mouse position to client coordinates. */
	ScreenToClient(hwnd, &pt);

	/* If the position is in the client area, display a shortcut menu. */
	if (PtInRect(&rc, pt)) {
		ClientToScreen(hwnd, &pt);
		nsws_update_edit(w);
		TrackPopupMenu(GetSubMenu(w->rclick, 0),
			       TPM_CENTERALIGN | TPM_TOPALIGN,
			       x,
			       y,
			       0,
			       hwnd,
			       NULL);

		return true;
	}

	/* Return false if no menu is displayed. */
	return false;
}


/**
 * update state of forward/back buttons/menu items when page changes
 */
static void nsws_window_update_forward_back(struct gui_window *w)
{
	if (w->bw == NULL)
		return;

	bool forward = browser_window_history_forward_available(w->bw);
	bool back = browser_window_history_back_available(w->bw);

	if (w->mainmenu != NULL) {
		EnableMenuItem(w->mainmenu, IDM_NAV_FORWARD,
			       (forward ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(w->mainmenu, IDM_NAV_BACK,
			       (back ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(w->rclick, IDM_NAV_FORWARD,
			       (forward ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(w->rclick, IDM_NAV_BACK,
			       (back ? MF_ENABLED : MF_GRAYED));
	}

	if (w->toolbar != NULL) {
		SendMessage(w->toolbar, TB_SETSTATE,
			    (WPARAM) IDM_NAV_FORWARD,
			    MAKELONG((forward ? TBSTATE_ENABLED :
				      TBSTATE_INDETERMINATE), 0));
		SendMessage(w->toolbar, TB_SETSTATE,
			    (WPARAM) IDM_NAV_BACK,
			    MAKELONG((back ? TBSTATE_ENABLED :
				      TBSTATE_INDETERMINATE), 0));
	}
}


static bool win32_window_get_scroll(struct gui_window *w, int *sx, int *sy)
{
	LOG("get scroll");
	if (w == NULL)
		return false;

	*sx = w->scrollx;
	*sy = w->scrolly;

	return true;
}


static void nsws_set_scale(struct gui_window *gw, float scale)
{
	assert(gw != NULL);

	if (gw->scale == scale)
		return;

	gw->scale = scale;

	if (gw->bw == NULL)
		return;

	browser_window_set_scale(gw->bw, scale, true);
}


/**
 * redraw the whole window
 */
static void win32_window_redraw_window(struct gui_window *gw)
{
	/* LOG("gw:%p", gw); */
	if (gw != NULL) {
		RedrawWindow(gw->drawingarea, NULL, NULL,
			     RDW_INVALIDATE | RDW_NOERASE);
	}
}


/**
 * scroll the window
 *
 * \param w The win32 gui window to scroll.
 * \param sx the new 'absolute' horizontal scroll location
 * \param sy the new 'absolute' vertical scroll location
 */
void win32_window_set_scroll(struct gui_window *w, int sx, int sy)
{
	SCROLLINFO si;
	nserror err;
	int height;
	int width;
	POINT p;

	if ((w == NULL) || (w->bw == NULL))
		return;

	err = browser_window_get_extents(w->bw, true, &width, &height);
	if (err != NSERROR_OK) {
		return;
	}

	/*LOG("scroll sx,sy:%d,%d x,y:%d,%d w.h:%d,%d",sx,sy,w->scrollx,w->scrolly, width,height);*/

	/* The resulting gui window scroll must remain withn the
	 * windows bounding box.
	 */
	if (sx < 0) {
		w->requestscrollx = -w->scrollx;
	} else if (sx > (width - w->width)) {
		w->requestscrollx = (width - w->width) - w->scrollx;
	} else {
		w->requestscrollx = sx - w->scrollx;
	}
	if (sy < 0) {
		w->requestscrolly = -w->scrolly;
	} else if (sy > (height - w->height)) {
		w->requestscrolly = (height - w->height) - w->scrolly;
	} else {
		w->requestscrolly = sy - w->scrolly;
	}

	/*LOG("requestscroll x,y:%d,%d", w->requestscrollx, w->requestscrolly);*/

	/* set the vertical scroll offset */
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = height - 1;
	si.nPage = w->height;
	si.nPos = max(w->scrolly + w->requestscrolly, 0);
	si.nPos = min(si.nPos, height - w->height);
	SetScrollInfo(w->drawingarea, SB_VERT, &si, TRUE);
	/*LOG("SetScrollInfo VERT min:%d max:%d page:%d pos:%d", si.nMin, si.nMax, si.nPage, si.nPos);*/

	/* set the horizontal scroll offset */
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = width -1;
	si.nPage = w->width;
	si.nPos = max(w->scrollx + w->requestscrollx, 0);
	si.nPos = min(si.nPos, width - w->width);
	SetScrollInfo(w->drawingarea, SB_HORZ, &si, TRUE);
	/*LOG("SetScrollInfo HORZ min:%d max:%d page:%d pos:%d", si.nMin, si.nMax, si.nPage, si.nPos);*/

	/* Set caret position */
	GetCaretPos(&p);
	HideCaret(w->drawingarea);
	SetCaretPos(p.x - w->requestscrollx, p.y - w->requestscrolly);
	ShowCaret(w->drawingarea);

	RECT r, redraw;
	r.top = 0;
	r.bottom = w->height + 1;
	r.left = 0;
	r.right = w->width + 1;
	ScrollWindowEx(w->drawingarea, - w->requestscrollx, - w->requestscrolly, &r, NULL, NULL, &redraw, SW_INVALIDATE);
	/*LOG("ScrollWindowEx %d, %d", - w->requestscrollx, - w->requestscrolly);*/
	w->scrolly += w->requestscrolly;
	w->scrollx += w->requestscrollx;
	w->requestscrollx = 0;
	w->requestscrolly = 0;

}


static LRESULT
nsws_window_command(HWND hwnd,
		    struct gui_window *gw,
		    int notification_code,
		    int identifier,
		    HWND ctrl_window)
{
	nserror ret;

	LOG("notification_code %x identifier %x ctrl_window %p", notification_code, identifier, ctrl_window);

	switch(identifier) {

	case IDM_FILE_QUIT:
	{
		struct gui_window *w;
		w = window_list;
		while (w != NULL) {
			PostMessage(w->main, WM_CLOSE, 0, 0);
			w = w->next;
		}
		break;
	}

	case IDM_FILE_OPEN_LOCATION:
		SetFocus(gw->urlbar);
		break;

	case IDM_FILE_OPEN_WINDOW:
		ret = browser_window_create(BW_CREATE_NONE,
				      NULL,
				      NULL,
				      gw->bw,
				      NULL);
		if (ret != NSERROR_OK) {
			warn_user(messages_get_errorcode(ret), 0);
		}
		break;

	case IDM_FILE_CLOSE_WINDOW:
		PostMessage(gw->main, WM_CLOSE, 0, 0);
		break;

	case IDM_FILE_SAVE_PAGE:
		break;

	case IDM_FILE_SAVEAS_TEXT:
		break;

	case IDM_FILE_SAVEAS_PDF:
		break;

	case IDM_FILE_SAVEAS_POSTSCRIPT:
		break;

	case IDM_FILE_PRINT_PREVIEW:
		break;

	case IDM_FILE_PRINT:
		break;

	case IDM_EDIT_CUT:
		OpenClipboard(gw->main);
		EmptyClipboard();
		CloseClipboard();
		if (GetFocus() == gw->urlbar) {
			SendMessage(gw->urlbar, WM_CUT, 0, 0);
		} else if (gw->bw != NULL) {
			browser_window_key_press(gw->bw, NS_KEY_CUT_SELECTION);
		}
		break;

	case IDM_EDIT_COPY:
		OpenClipboard(gw->main);
		EmptyClipboard();
		CloseClipboard();
		if (GetFocus() == gw->urlbar) {
			SendMessage(gw->urlbar, WM_COPY, 0, 0);
		} else if (gw->bw != NULL) {
			browser_window_key_press(gw->bw, NS_KEY_COPY_SELECTION);
		}
		break;

	case IDM_EDIT_PASTE: {
		OpenClipboard(gw->main);
		HANDLE h = GetClipboardData(CF_TEXT);
		if (h != NULL) {
			char *content = GlobalLock(h);
			LOG("pasting %s\n", content);
			GlobalUnlock(h);
		}
		CloseClipboard();
		if (GetFocus() == gw->urlbar)
			SendMessage(gw->urlbar, WM_PASTE, 0, 0);
		else
			browser_window_key_press(gw->bw, NS_KEY_PASTE);
		break;
	}

	case IDM_EDIT_DELETE:
		if (GetFocus() == gw->urlbar)
			SendMessage(gw->urlbar, WM_CUT, 0, 0);
		else
			browser_window_key_press(gw->bw, NS_KEY_DELETE_RIGHT);
		break;

	case IDM_EDIT_SELECT_ALL:
		if (GetFocus() == gw->urlbar)
			SendMessage(gw->urlbar, EM_SETSEL, 0, -1);
		else
			browser_window_key_press(gw->bw, NS_KEY_SELECT_ALL);
		break;

	case IDM_EDIT_SEARCH:
		break;

	case IDM_EDIT_PREFERENCES:
		nsws_prefs_dialog_init(hInstance, gw->main);
		break;

	case IDM_NAV_BACK:
		if ((gw->bw != NULL) &&
		    (browser_window_history_back_available(gw->bw))) {
			browser_window_history_back(gw->bw, false);
		}
		nsws_window_update_forward_back(gw);
		break;

	case IDM_NAV_FORWARD:
		if ((gw->bw != NULL) &&
		    (browser_window_history_forward_available(gw->bw))) {
			browser_window_history_forward(gw->bw, false);
		}
		nsws_window_update_forward_back(gw);
		break;

	case IDM_NAV_HOME:
	{
		nsurl *url;

		if (nsurl_create(nsoption_charp(homepage_url), &url) != NSERROR_OK) {
			warn_user("NoMemory", 0);
		} else {
			browser_window_navigate(gw->bw,
						url,
						NULL,
						BW_NAVIGATE_HISTORY,
						NULL,
						NULL,
						NULL);
			nsurl_unref(url);
		}
		break;
	}

	case IDM_NAV_STOP:
		browser_window_stop(gw->bw);
		break;

	case IDM_NAV_RELOAD:
		browser_window_reload(gw->bw, true);
		break;

	case IDM_NAV_LOCALHISTORY:
		gw->localhistory = nsws_window_create_localhistory(gw);
		break;

	case IDM_NAV_GLOBALHISTORY:
		break;

	case IDM_VIEW_ZOOMPLUS: {
		int x, y;
		win32_window_get_scroll(gw, &x, &y);
		if (gw->bw != NULL) {
			nsws_set_scale(gw, gw->scale * 1.1);
		}
		win32_window_redraw_window(gw);
		win32_window_set_scroll(gw, x, y);
		break;
	}

	case IDM_VIEW_ZOOMMINUS: {
		int x, y;
		win32_window_get_scroll(gw, &x, &y);
		if (gw->bw != NULL) {
			nsws_set_scale(gw, gw->scale * 0.9);
		}
		win32_window_redraw_window(gw);
		win32_window_set_scroll(gw, x, y);
		break;
	}

	case IDM_VIEW_ZOOMNORMAL: {
		int x, y;
		win32_window_get_scroll(gw, &x, &y);
		if (gw->bw != NULL) {
			nsws_set_scale(gw, 1.0);
		}
		win32_window_redraw_window(gw);
		win32_window_set_scroll(gw, x, y);
		break;
	}

	case IDM_VIEW_SOURCE:
		break;

	case IDM_VIEW_SAVE_WIN_METRICS: {
		RECT r;
		GetWindowRect(gw->main, &r);
		nsoption_set_int(window_x, r.left);
		nsoption_set_int(window_y, r.top);
		nsoption_set_int(window_width, r.right - r.left);
		nsoption_set_int(window_height, r.bottom - r.top);
		nsoption_write(options_file_location, NULL, NULL);
		break;
	}

	case IDM_VIEW_FULLSCREEN: {
		RECT rdesk;
		if (gw->fullscreen == NULL) {
			HWND desktop = GetDesktopWindow();
			gw->fullscreen = malloc(sizeof(RECT));
			if ((desktop == NULL) ||
			    (gw->fullscreen == NULL)) {
				warn_user("NoMemory", 0);
				break;
			}
			GetWindowRect(desktop, &rdesk);
			GetWindowRect(gw->main, gw->fullscreen);
			DeleteObject(desktop);
			SetWindowLong(gw->main, GWL_STYLE, 0);
			SetWindowPos(gw->main, HWND_TOPMOST, 0, 0,
				     rdesk.right - rdesk.left,
				     rdesk.bottom - rdesk.top,
				     SWP_SHOWWINDOW);
		} else {
			SetWindowLong(gw->main, GWL_STYLE,
				      WS_OVERLAPPEDWINDOW |
				      WS_HSCROLL | WS_VSCROLL |
				      WS_CLIPCHILDREN |
				      WS_CLIPSIBLINGS | CS_DBLCLKS);
			SetWindowPos(gw->main, HWND_TOPMOST,
				     gw->fullscreen->left,
				     gw->fullscreen->top,
				     gw->fullscreen->right -
				     gw->fullscreen->left,
				     gw->fullscreen->bottom -
				     gw->fullscreen->top,
				     SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			free(gw->fullscreen);
			gw->fullscreen = NULL;
		}
		break;
	}

	case IDM_VIEW_DOWNLOADS:
		break;

	case IDM_VIEW_TOGGLE_DEBUG_RENDERING:
		if (gw->bw != NULL) {
			browser_window_debug(gw->bw, CONTENT_DEBUG_REDRAW);
			/* TODO: This should only redraw, not reformat.
			 * (Layout doesn't change, so reformat is a waste of time) */
			browser_window_reformat(gw->bw, false, gw->width, gw->height);
		}
		break;

	case IDM_VIEW_DEBUGGING_SAVE_BOXTREE:
		break;

	case IDM_VIEW_DEBUGGING_SAVE_DOMTREE:
		break;

	case IDM_HELP_CONTENTS:
		nsws_window_go(hwnd,
			       "http://www.netsurf-browser.org/documentation/");
		break;

	case IDM_HELP_GUIDE:
		nsws_window_go(hwnd,
			       "http://www.netsurf-browser.org/documentation/guide");
		break;

	case IDM_HELP_INFO:
		nsws_window_go(hwnd,
			       "http://www.netsurf-browser.org/documentation/info");
		break;

	case IDM_HELP_ABOUT:
		nsws_about_dialog_init(hInstance, gw->main);
		break;

	case IDC_MAIN_LAUNCH_URL:
	{
		nsurl *url;

		if (GetFocus() != gw->urlbar)
			break;

		int len = SendMessage(gw->urlbar, WM_GETTEXTLENGTH, 0, 0);
		char addr[len + 1];
		SendMessage(gw->urlbar, WM_GETTEXT, (WPARAM)(len + 1), (LPARAM)addr);
		LOG("launching %s\n", addr);

		if (nsurl_create(addr, &url) != NSERROR_OK) {
			warn_user("NoMemory", 0);
		} else {
			browser_window_navigate(gw->bw,
						url,
						NULL,
						BW_NAVIGATE_HISTORY,
						NULL,
						NULL,
						NULL);
			nsurl_unref(url);
		}

		break;
	}


	default:
		return 1; /* unhandled */

	}
	return 0; /* control message handled */
}


static LRESULT
nsws_window_resize(struct gui_window *gw,
		   HWND hwnd,
		   WPARAM wparam,
		   LPARAM lparam)
{
	int x, y;
	RECT rstatus, rtool;

	if ((gw->toolbar == NULL) ||
	    (gw->urlbar == NULL) ||
	    (gw->statusbar == NULL))
		return 0;

	SendMessage(gw->statusbar, WM_SIZE, wparam, lparam);
	SendMessage(gw->toolbar, WM_SIZE, wparam, lparam);

	GetClientRect(gw->toolbar, &rtool);
	GetWindowRect(gw->statusbar, &rstatus);
	win32_window_get_scroll(gw, &x, &y);
	gw->width = LOWORD(lparam);
	gw->height = HIWORD(lparam) - (rtool.bottom - rtool.top) - (rstatus.bottom - rstatus.top);

	if (gw->drawingarea != NULL) {
		MoveWindow(gw->drawingarea,
			   0,
			   rtool.bottom,
			   gw->width,
			   gw->height,
			   true);
	}
	nsws_window_update_forward_back(gw);

	win32_window_set_scroll(gw, x, y);

	if (gw->toolbar != NULL) {
		SendMessage(gw->toolbar, TB_SETSTATE,
			    (WPARAM) IDM_NAV_STOP,
			    MAKELONG(TBSTATE_INDETERMINATE, 0));
	}

	return 0;
}


/**
 * callback for window events generally
 */
static LRESULT CALLBACK
nsws_window_event_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	struct gui_window *gw;
	RECT rmain;

	LOG_WIN_MSG(hwnd, msg, wparam, lparam);

	/* deal with window creation as a special case */
	if (msg == WM_CREATE) {
		/* To cause all the component child windows to be
		 * re-sized correctly a WM_SIZE message of the actual
		 * created size must be sent.
		 *
		 * The message must be posted here because the actual
		 * size values of the component windows are not known
		 * until after the WM_CREATE message is dispatched.
		 */
		GetClientRect(hwnd, &rmain);
		PostMessage(hwnd, WM_SIZE, 0, MAKELPARAM(rmain.right, rmain.bottom));
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}


	gw = nsws_get_gui_window(hwnd);
	if (gw == NULL) {
		LOG("Unable to find gui window structure for hwnd %p", hwnd);
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	switch (msg) {

	case WM_CONTEXTMENU:
		if (nsws_ctx_menu(gw, hwnd, GET_X_LPARAM(lparam),
				  GET_Y_LPARAM(lparam))) {
			return 0;
		}
		break;

	case WM_COMMAND:
		if (nsws_window_command(hwnd, gw, HIWORD(wparam),
					LOWORD(wparam), (HWND)lparam) == 0) {
			return 0;
		}
		break;

	case WM_SIZE:
		return nsws_window_resize(gw, hwnd, wparam, lparam);

	case WM_NCDESTROY:
		RemoveProp(hwnd, TEXT("GuiWnd"));
		browser_window_destroy(gw->bw);
		if (--open_windows <= 0) {
			win32_set_quit(true);
		}
		break;

	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}


/**
 * Create the main window class.
 */
nserror
nsws_create_main_class(HINSTANCE hinstance) {
	nserror ret = NSERROR_OK;
	WNDCLASSEX w;

	/* main window */
	w.cbSize = sizeof(WNDCLASSEX);
	w.style	= 0;
	w.lpfnWndProc = nsws_window_event_callback;
	w.cbClsExtra = 0;
	w.cbWndExtra = 0;
	w.hInstance = hinstance;
	w.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(IDR_NETSURF_ICON));
	w.hCursor = NULL;
	w.hbrBackground	= (HBRUSH)(COLOR_MENU + 1);
	w.lpszMenuName = NULL;
	w.lpszClassName = windowclassname_main;
	w.hIconSm = LoadIcon(hinstance, MAKEINTRESOURCE(IDR_NETSURF_ICON));

	if (RegisterClassEx(&w) == 0) {
		win_perror("DrawableClass");
		ret = NSERROR_INIT_FAILED;
	}

	hInstance = hinstance;

	return ret;
}






/**
 * create a new gui_window to contain a browser_window.
 *
 * \param bw the browser_window to connect to the new gui_window
 * \param existing An existing window.
 * \param flags The flags controlling the construction.
 * \return The new win32 gui window or NULL on error.
 */
static struct gui_window *
win32_window_create(struct browser_window *bw,
		struct gui_window *existing,
		gui_window_create_flags flags)
{
	struct gui_window *gw;

	LOG("Creating gui window for browser window %p", bw);

	gw = calloc(1, sizeof(struct gui_window));

	if (gw == NULL) {
		return NULL;
	}

	/* connect gui window to browser window */
	gw->bw = bw;

	gw->width = 800;
	gw->height = 600;
	gw->scale = 1.0;
	gw->toolbuttonsize = 24;
	gw->requestscrollx = 0;
	gw->requestscrolly = 0;
	gw->localhistory = NULL;

	gw->mouse = malloc(sizeof(struct browser_mouse));
	if (gw->mouse == NULL) {
		free(gw);
		LOG("Unable to allocate mouse state");
		return NULL;
	}
	gw->mouse->gui = gw;
	gw->mouse->state = 0;
	gw->mouse->pressed_x = 0;
	gw->mouse->pressed_y = 0;

	/* add window to list */
	if (window_list != NULL)
		window_list->prev = gw;
	gw->next = window_list;
	window_list = gw;

	gw->main = nsws_window_create(gw);
	gw->toolbar = nsws_window_create_toolbar(gw, gw->main);
	gw->statusbar = nsws_window_create_statusbar(gw);
	gw->drawingarea = nsws_window_create_drawable(hInstance, gw->main, gw);

	LOG("new window: main:%p toolbar:%p statusbar %p drawingarea %p", gw->main, gw->toolbar, gw->statusbar, gw->drawingarea);

	font_hwnd = gw->drawingarea;
	open_windows++;
	ShowWindow(gw->main, SW_SHOWNORMAL);

	return gw;
}


/**
 * window cleanup code
 */
static void win32_window_destroy(struct gui_window *w)
{
	if (w == NULL)
		return;

	if (w->prev != NULL)
		w->prev->next = w->next;
	else
		window_list = w->next;

	if (w->next != NULL)
		w->next->prev = w->prev;

	DestroyAcceleratorTable(w->acceltable);

	free(w);
	w = NULL;
}




static void
win32_window_update_box(struct gui_window *gw, const struct rect *rect)
{
	/* LOG("gw:%p %f,%f %f,%f", gw, data->redraw.x, data->redraw.y, data->redraw.width, data->redraw.height); */

	if (gw == NULL)
		return;

	RECT redrawrect;

	redrawrect.left = (long)rect->x0 - (gw->scrollx / gw->scale);
	redrawrect.top = (long)rect->y0 - (gw->scrolly / gw->scale);
	redrawrect.right =(long)rect->x1;
	redrawrect.bottom = (long)rect->y1;

	RedrawWindow(gw->drawingarea, &redrawrect, NULL,
		     RDW_INVALIDATE | RDW_NOERASE);

}





static void win32_window_get_dimensions(struct gui_window *w, int *width, int *height,
			       bool scaled)
{
	if (w == NULL)
		return;

	LOG("get dimensions %p w=%d h=%d", w, w->width, w->height);

	*width = w->width;
	*height = w->height;
}

static void win32_window_update_extent(struct gui_window *w)
{

}

/**
 * callback from core to reformat a window.
 *
 * \param gw The win32 gui window to reformat.
 */
static void win32_window_reformat(struct gui_window *gw)
{
	if (gw != NULL) {
		browser_window_reformat(gw->bw, false, gw->width, gw->height);
	}
}


/**
 * set window title
 *
 * \param w the Windows gui window.
 * \param title to set on window
 */
static void win32_window_set_title(struct gui_window *w, const char *title)
{
	if (w == NULL)
		return;
	LOG("%p, title %s", w, title);
	char *fulltitle = malloc(strlen(title) +
				 SLEN("  -  NetSurf") + 1);
	if (fulltitle == NULL) {
		warn_user("NoMemory", 0);
		return;
	}
	strcpy(fulltitle, title);
	strcat(fulltitle, "  -  NetSurf");
	SendMessage(w->main, WM_SETTEXT, 0, (LPARAM)fulltitle);
	free(fulltitle);
}


static nserror win32_window_set_url(struct gui_window *w, nsurl *url)
{
	SendMessage(w->urlbar, WM_SETTEXT, 0, (LPARAM) nsurl_access(url));

	return NSERROR_OK;
}


/**
 * set the status bar message
 */
static void win32_window_set_status(struct gui_window *w, const char *text)
{
	if (w == NULL)
		return;
	SendMessage(w->statusbar, WM_SETTEXT, 0, (LPARAM)text);
}


/**
 * set the pointer shape
 */
static void win32_window_set_pointer(struct gui_window *w,
				     gui_pointer_shape shape)
{
	SetCursor(nsws_get_pointer(shape));
}


/**
 * place caret in window
 */
static void win32_window_place_caret(struct gui_window *w, int x, int y,
				   int height, const struct rect *clip)
{
	if (w == NULL)
		return;
	CreateCaret(w->drawingarea, (HBITMAP)NULL, 1, height * w->scale);
	SetCaretPos(x * w->scale - w->scrollx,
		    y * w->scale - w->scrolly);
	ShowCaret(w->drawingarea);
}

/**
 * clear window caret
 */
static void win32_window_remove_caret(struct gui_window *w)
{
	if (w == NULL)
		return;
	HideCaret(w->drawingarea);
}




static void win32_window_start_throbber(struct gui_window *w)
{
	if (w == NULL)
		return;
	nsws_window_update_forward_back(w);

	if (w->mainmenu != NULL) {
		EnableMenuItem(w->mainmenu, IDM_NAV_STOP, MF_ENABLED);
		EnableMenuItem(w->mainmenu, IDM_NAV_RELOAD, MF_GRAYED);
	}
	if (w->rclick != NULL) {
		EnableMenuItem(w->rclick, IDM_NAV_STOP, MF_ENABLED);
		EnableMenuItem(w->rclick, IDM_NAV_RELOAD, MF_GRAYED);
	}
	if (w->toolbar != NULL) {
		SendMessage(w->toolbar, TB_SETSTATE, (WPARAM) IDM_NAV_STOP,
			    MAKELONG(TBSTATE_ENABLED, 0));
		SendMessage(w->toolbar, TB_SETSTATE,
			    (WPARAM) IDM_NAV_RELOAD,
			    MAKELONG(TBSTATE_INDETERMINATE, 0));
	}
	w->throbbing = true;
	Animate_Play(w->throbber, 0, -1, -1);
}



static void win32_window_stop_throbber(struct gui_window *w)
{
	if (w == NULL)
		return;

	nsws_window_update_forward_back(w);
	if (w->mainmenu != NULL) {
		EnableMenuItem(w->mainmenu, IDM_NAV_STOP, MF_GRAYED);
		EnableMenuItem(w->mainmenu, IDM_NAV_RELOAD, MF_ENABLED);
	}

	if (w->rclick != NULL) {
		EnableMenuItem(w->rclick, IDM_NAV_STOP, MF_GRAYED);
		EnableMenuItem(w->rclick, IDM_NAV_RELOAD, MF_ENABLED);
	}

	if (w->toolbar != NULL) {
		SendMessage(w->toolbar, TB_SETSTATE, (WPARAM) IDM_NAV_STOP,
			    MAKELONG(TBSTATE_INDETERMINATE, 0));
		SendMessage(w->toolbar, TB_SETSTATE,
			    (WPARAM) IDM_NAV_RELOAD,
			    MAKELONG(TBSTATE_ENABLED, 0));
	}

	w->throbbing = false;
	Animate_Stop(w->throbber);
	Animate_Seek(w->throbber, 0);
}

static struct gui_window_table window_table = {
	.create = win32_window_create,
	.destroy = win32_window_destroy,
	.redraw = win32_window_redraw_window,
	.update = win32_window_update_box,
	.get_scroll = win32_window_get_scroll,
	.set_scroll = win32_window_set_scroll,
	.get_dimensions = win32_window_get_dimensions,
	.update_extent = win32_window_update_extent,
	.reformat = win32_window_reformat,

	.set_title = win32_window_set_title,
	.set_url = win32_window_set_url,
	.set_status = win32_window_set_status,
	.set_pointer = win32_window_set_pointer,
	.place_caret = win32_window_place_caret,
	.remove_caret = win32_window_remove_caret,
	.start_throbber = win32_window_start_throbber,
	.stop_throbber = win32_window_stop_throbber,
};

struct gui_window_table *win32_window_table = &window_table;

/* exported interface documented in windows/window.h */
struct gui_window *nsws_get_gui_window(HWND hwnd)
{
	struct gui_window *gw = NULL;
	HWND phwnd = hwnd;

	/* scan the window hierachy for gui window */
	while (phwnd != NULL) {
		gw = GetProp(phwnd, TEXT("GuiWnd"));
		if (gw != NULL)
			break;
		phwnd = GetParent(phwnd);
	}

	if (gw == NULL) {
		/* try again looking for owner windows instead */
		phwnd = hwnd;
		while (phwnd != NULL) {
			gw = GetProp(phwnd, TEXT("GuiWnd"));
			if (gw != NULL)
				break;
			phwnd = GetWindow(phwnd, GW_OWNER);
		}
	}

	return gw;
}


/* exported interface documented in windows/window.h */
bool nsws_window_go(HWND hwnd, const char *urltxt)
{
	struct gui_window *gw;
	nsurl *url;

	gw = nsws_get_gui_window(hwnd);
	if (gw == NULL)
		return false;

	if (nsurl_create(urltxt, &url) != NSERROR_OK) {
		warn_user("NoMemory", 0);
	} else {
		browser_window_navigate(gw->bw,
					url,
					NULL,
					BW_NAVIGATE_HISTORY,
					NULL,
					NULL,
					NULL);
		nsurl_unref(url);
	}

	return true;
}


/* exported interface documented in windows/window.h */
HWND gui_window_main_window(struct gui_window *w)
{
	if (w == NULL)
		return NULL;
	return w->main;
}


/* exported interface documented in windows/window.h */
struct nsws_localhistory *gui_window_localhistory(struct gui_window *w)
{
	if (w == NULL)
		return NULL;
	return w->localhistory;
}
