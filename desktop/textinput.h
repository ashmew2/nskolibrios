/*
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004 Andrew Timmins <atimmins@blueyonder.co.uk>
 * Copyright 2004 John Tytgat <joty@netsurf-browser.org>
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

/** \file
 * Textual input handling (interface)
 */

#ifndef _NETSURF_DESKTOP_TEXTINPUT_H_
#define _NETSURF_DESKTOP_TEXTINPUT_H_

struct browser_window;

enum input_key {

	NS_KEY_SELECT_ALL = 1,
	NS_KEY_COPY_SELECTION = 3,

	NS_KEY_DELETE_LEFT = 8,
	NS_KEY_TAB = 9,

	NS_KEY_NL = 10,
	NS_KEY_SHIFT_TAB = 11,
	NS_KEY_CR = 13,

	NS_KEY_DELETE_LINE = 21,
	NS_KEY_PASTE = 22,
	NS_KEY_CUT_SELECTION = 24,
	NS_KEY_CLEAR_SELECTION = 26,

	NS_KEY_ESCAPE = 27,

	/* cursor movement keys */
	NS_KEY_LEFT = 28,
	NS_KEY_RIGHT,
	NS_KEY_UP,
	NS_KEY_DOWN,

	NS_KEY_DELETE_RIGHT = 127,

	NS_KEY_LINE_START = 128,
	NS_KEY_LINE_END,
	NS_KEY_TEXT_START,
	NS_KEY_TEXT_END,
	NS_KEY_WORD_LEFT,
	NS_KEY_WORD_RIGHT,
	NS_KEY_PAGE_UP,
	NS_KEY_PAGE_DOWN,
	NS_KEY_DELETE_LINE_END,
	NS_KEY_DELETE_LINE_START,

	NS_KEY_UNDO,
	NS_KEY_REDO
};


/**
 * Position the caret and assign a callback for key presses.
 *
 * \param bw		The browser window in which to place the caret
 * \param x		X coordinate of the caret
 * \param y		Y coordinate
 * \param height	Height of caret
 * \param clip		Clip rectangle for caret, or NULL if none
 */
void browser_window_place_caret(struct browser_window *bw, int x, int y,
		int height, const struct rect *clip);

/**
 * Removes the caret and callback for key process.
 *
 * \param bw The browser window from which to remove caret.
 * \param only_hide Revove the caret but leave the textinput editable.
 */
void browser_window_remove_caret(struct browser_window *bw, bool only_hide);

/**
 * Handle key presses in a browser window.
 *
 * \param bw   The root browser window
 * \param key  The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool browser_window_key_press(struct browser_window *bw, uint32_t key);

#endif
