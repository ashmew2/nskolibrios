/*
 * Copyright 2008 François Revol <mmu_man@users.sourceforge.net>
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

#ifndef NS_BEOS_BITMAP_H
#define NS_BEOS_BITMAP_H

#include <Bitmap.h>

extern struct gui_bitmap_table *beos_bitmap_table;

BBitmap *nsbeos_bitmap_get_primary(struct bitmap*);
BBitmap *nsbeos_bitmap_get_pretile_x(struct bitmap*);
BBitmap *nsbeos_bitmap_get_pretile_y(struct bitmap*);
BBitmap *nsbeos_bitmap_get_pretile_xy(struct bitmap*);

void bitmap_modified(void *vbitmap);

#endif /* NS_BEOS_BITMAP_H */
