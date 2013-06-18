/*
 * Copyright 2012 - 2013 Michael Drake <tlsa@netsurf-browser.org>
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
 
#ifndef _NETSURF_DESKTOP_GLOBAL_HISTORY_H_
#define _NETSURF_DESKTOP_GLOBAL_HISTORY_H_

#include <stdbool.h>

#include "desktop/core_window.h"


/**
 * Initialise the global history.
 *
 * This iterates through the URL database, generating the global history data,
 * and creates a treeview.
 *
 * This must be called before any other global_history_* function.
 *
 * \param cw_t		Callback table for core_window containing the treeview
 * \param cw		The core_window in which the treeview is shown
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror global_history_init(struct core_window_callback_table *cw_t,
		void *core_window_handle);

/**
 * Finalise the global history.
 *
 * This destroys the global history treeview and the global history module's
 * internal data.  After calling this if global history is required again,
 * global_history_init must be called.
 *
 * \param cw_t		Callback table for core_window containing the treeview
 * \param cw		The core_window in which the treeview is shown
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror global_history_fini(struct core_window_callback_table *cw_t,
		void *core_window_handle);

/**
 * Redraw the global history.
 *
 * \param x		X coordinate to render treeview at
 * \param x		Y coordinate to render treeview at
 * \param clip		Current clip rectangle (wrt tree origin)
 * \param ctx		Current redraw context
 */
void global_history_redraw(int x, int y, struct rect *clip,
		const struct redraw_context *ctx);

/**
 * Handles all kinds of mouse action
 *
 * \param mouse		The current mouse state
 * \param x		X coordinate
 * \param y		Y coordinate
 */
void global_history_mouse_action(browser_mouse_state mouse, int x, int y);

#endif