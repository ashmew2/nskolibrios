/*
 * Copyright 2008 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2009 Mark Benjamin <netsurf-browser.org.MarkBenjamin@dfgh.net>
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

#ifndef _NETSURF_WINDOWS_GUI_H_
#define _NETSURF_WINDOWS_GUI_H_

struct gui_window;
struct gui_clipboard_table *win32_clipboard_table;

extern HINSTANCE hInstance;

/* bounding box */
typedef struct bbox_s {
        int x0;
        int y0;
        int x1;
        int y1;
} bbox_t;



extern char *options_file_location;




/**
 * Run the win32 message loop with scheduling
 */
void win32_run(void);

/** cause the main message loop to exit */
void win32_set_quit(bool q);

#endif 
