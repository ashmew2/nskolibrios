/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdbool.h>
#include <stdlib.h>

#ifndef _WIN32
#include <endian.h>
#else
#define __BYTE_ORDER __BYTE_ORDER__
#define __BIG_ENDIAN __ORDER_BIG_ENDIAN__
#endif

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_plot_util.h"

#include "nsfb.h"
#include "plot.h"


#define UNUSED __attribute__((unused)) 

static inline uint32_t *get_xy_loc(nsfb_t *nsfb, int x, int y)
{
        return (void *)(nsfb->ptr + (y * nsfb->linelen) + (x << 2));
}

#if __BYTE_ORDER == __BIG_ENDIAN
static inline nsfb_colour_t pixel_to_colour(UNUSED nsfb_t *nsfb, uint32_t pixel)
{
	/* TODO: FIX */
        return (pixel >> 8) & ~0xFF000000U;
}

/* convert a colour value to a 32bpp pixel value ready for screen output */
static inline uint32_t colour_to_pixel(UNUSED nsfb_t *nsfb, nsfb_colour_t c)
{
	/* TODO: FIX */
	c = c<<8;
	return ( ((c & 0xFF000000U) >> 16) | (c & 0x00FF0000U) | ((c & 0x0000FF00U) << 16 )) & 0xFFFFFF00U;
}
#else /* __BYTE_ORDER == __BIG_ENDIAN */
static inline nsfb_colour_t pixel_to_colour(UNUSED nsfb_t *nsfb, uint32_t pixel)

        return pixel | 0xFF000000U;
}

/* convert a colour value to a 32bpp pixel value ready for screen output */
static inline uint32_t colour_to_pixel(UNUSED nsfb_t *nsfb, nsfb_colour_t c)
{
        return c;
}
#endif

#define PLOT_TYPE uint32_t
#define PLOT_LINELEN(ll) ((ll) >> 2)

#include "32bpp-common.c"

const nsfb_plotter_fns_t _nsfb_32bpp_xbgr8888_plotters = {
        .line = line,
        .fill = fill,
        .point = point,
        .bitmap = bitmap,
        .bitmap_tiles = bitmap_tiles,
        .glyph8 = glyph8,
        .glyph1 = glyph1,
        .readrect = readrect,
};

/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
