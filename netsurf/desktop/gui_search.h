/*
 * Copyright 2014 Vincent Sanders <vince@netsurf-browser.org>
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

/**
 * \file
 *
 * Interface to platform-specific search operations.
 */

#ifndef _NETSURF_DESKTOP_GUI_SEARCH_H_
#define _NETSURF_DESKTOP_GUI_SEARCH_H_

/**
 * function table for page text search.
 */
struct gui_search_table {
	/**
	 * Change the displayed search status.
	 *
	 * \param found search pattern matched in text
	 * \param p gui private data pointer provided with search callbacks
	 */
	void (*status)(bool found, void *p);

	/**
	 * display hourglass while searching.
	 *
	 * \param active start/stop indicator
	 * \param p gui private data pointer provided with search callbacks
	 */
	void (*hourglass)(bool active, void *p);

	/**
	 * add search string to recent searches list
	 * front has full liberty how to implement the bare notification;
	 * core gives no guarantee of the integrity of the string
	 *
	 * \param string search pattern
	 * \param p gui private data pointer provided with search callbacks
	 */
	void (*add_recent)(const char *string, void *p);

	/**
	 * activate search forwards button in gui
	 *
	 * \param active activate/inactivate
	 * \param p gui private data pointer provided with search callbacks
	 */
	void (*forward_state)(bool active, void *p);

	/**
	 * activate search back button in gui
	 *
	 * \param active activate/inactivate
	 * \param p gui private data pointer provided with search callbacks
	 */
	void (*back_state)(bool active, void *p);
};

#endif