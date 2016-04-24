/*
 * Copyright 2008 - 2013 Chris Young <chris@unsatisfactorysoftware.co.uk>
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

#include "amiga/os3support.h"

#include <assert.h>

#ifndef __amigaos4__
#include <proto/bullet.h>
#endif
#include <proto/diskfont.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/Picasso96API.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include <diskfont/diskfonttag.h>
#include <diskfont/oterrors.h>
#include <graphics/rpattr.h>

#ifdef __amigaos4__
#include <graphics/blitattr.h>
#endif

#include "utils/log.h"
#include "utils/utf8.h"
#include "utils/utils.h"
#include "utils/nsoption.h"
#include "desktop/browser.h"
#include "desktop/font.h"
#include "desktop/gui_window.h"

#include "amiga/font.h"
#include "amiga/font_scan.h"
#include "amiga/gui.h"
#include "amiga/utf8.h"
#include "amiga/object.h"
#include "amiga/schedule.h"

#define NSA_UNICODE_FONT PLOT_FONT_FAMILY_COUNT

#define NSA_NORMAL 0
#define NSA_ITALIC 1
#define NSA_BOLD 2
#define NSA_BOLDITALIC 3
#define NSA_OBLIQUE 4
#define NSA_BOLDOBLIQUE 6

#define NSA_VALUE_BOLDX (1 << 12)
#define NSA_VALUE_BOLDY 0
#define NSA_VALUE_SHEARSIN (1 << 14)
#define NSA_VALUE_SHEARCOS (1 << 16)

#define NSA_FONT_EMWIDTH(s) (s / FONT_SIZE_SCALE) * (ami_xdpi / 72.0)

struct ami_font_node
{
#ifdef __amigaos4__
	struct SkipNode skip_node;
#endif
	struct OutlineFont *font;
	char *bold;
	char *italic;
	char *bolditalic;
	struct TimeVal lastused;
};

const uint16 sc_table[] = {
		0x0061, 0x1D00, /* a */
		0x0062, 0x0299, /* b */
		0x0063, 0x1D04, /* c */
		0x0064, 0x1D05, /* d */
		0x0065, 0x1D07, /* e */
		0x0066, 0xA730, /* f */
		0x0067, 0x0262, /* g */
		0x0068, 0x029C, /* h */
		0x0069, 0x026A, /* i */
		0x006A, 0x1D0A, /* j */
		0x006B, 0x1D0B, /* k */
		0x006C, 0x029F, /* l */
		0x006D, 0x1D0D, /* m */
		0x006E, 0x0274, /* n */
		0x006F, 0x1D0F, /* o */
		0x0070, 0x1D18, /* p */
		0x0071, 0xA7EE, /* q (proposed) (Adobe codepoint 0xF771) */
		0x0072, 0x0280, /* r */
		0x0073, 0xA731, /* s */
		0x0074, 0x1D1B, /* t */
		0x0075, 0x1D1C, /* u */
		0x0076, 0x1D20, /* v */
		0x0077, 0x1D21, /* w */
		0x0078, 0xA7EF, /* x (proposed) (Adobe codepoint 0xF778) */
		0x0079, 0x028F, /* y */
		0x007A, 0x1D22, /* z */

		0x00C6, 0x1D01, /* ae */
		0x0153, 0x0276, /* oe */

#if 0
/* TODO: fill in the non-small caps character ids for these */
		0x0000, 0x1D03, /* barred b */
		0x0000, 0x0281, /* inverted r */
		0x0000, 0x1D19, /* reversed r */
		0x0000, 0x1D1A, /* turned r */
		0x0000, 0x029B, /* g with hook */
		0x0000, 0x1D06, /* eth � */
		0x0000, 0x1D0C, /* l with stroke */
		0x0000, 0xA7FA, /* turned m */
		0x0000, 0x1D0E, /* reversed n */
		0x0000, 0x1D10, /* open o */
		0x0000, 0x1D15, /* ou */
		0x0000, 0x1D23, /* ezh */
		0x0000, 0x1D26, /* gamma */
		0x0000, 0x1D27, /* lamda */
		0x0000, 0x1D28, /* pi */
		0x0000, 0x1D29, /* rho */
		0x0000, 0x1D2A, /* psi */
		0x0000, 0x1D2B, /* el */
		0x0000, 0xA776, /* rum */

		0x0000, 0x1DDB, /* combining g */
		0x0000, 0x1DDE, /* combining l */
		0x0000, 0x1DDF, /* combining m */
		0x0000, 0x1DE1, /* combining n */
		0x0000, 0x1DE2, /* combining r */

		0x0000, 0x1DA6, /* modifier i */
		0x0000, 0x1DA7, /* modifier i with stroke */
		0x0000, 0x1DAB, /* modifier l */
		0x0000, 0x1DB0, /* modifier n */
		0x0000, 0x1DB8, /* modifier u */
#endif
		0, 0};

#ifdef __amigaos4__
struct SkipList *ami_font_list = NULL;
#else
struct MinList *ami_font_list = NULL;
#endif
struct List ami_diskfontlib_list;
lwc_string *glypharray[0xffff + 1];
ULONG ami_devicedpi;
ULONG ami_xdpi;
static struct Hook ami_font_cache_hook;

static inline int32 ami_font_plot_glyph(struct OutlineFont *ofont, struct RastPort *rp,
		uint16 *char1, uint16 *char2, uint32 x, uint32 y, uint32 emwidth, bool aa);
static inline int32 ami_font_width_glyph(struct OutlineFont *ofont, 
		const uint16 *char1, const uint16 *char2, uint32 emwidth);
static struct OutlineFont *ami_open_outline_font(const plot_font_style_t *fstyle,
		const uint16 *codepoint);
static inline ULONG ami_font_unicode_width(const char *string, ULONG length,
		const plot_font_style_t *fstyle, ULONG x, ULONG y, bool aa);

static inline int amiga_nsfont_utf16_char_length(const uint16 *char1)
{
	if (__builtin_expect(((*char1 < 0xD800) || (0xDBFF < *char1)), 1)) {
		return 1;
	} else {
		return 2;
	}
}

static inline uint32 amiga_nsfont_decode_surrogate(const uint16 *char1)
{
	if(__builtin_expect((amiga_nsfont_utf16_char_length(char1) == 2), 0)) {
		return ((uint32)char1[0] << 10) + char1[1] - 0x35FDC00;
	} else {
		return (uint32)*char1;
	}
}

static inline bool amiga_nsfont_width(const plot_font_style_t *fstyle,
		const char *string, size_t length,
		int *width)
{
	*width = ami_font_unicode_width(string, length, fstyle, 0, 0, false);

	if(*width <= 0) *width == length; // fudge

	return true;
}

/**
 * Find the position in a string where an x coordinate falls.
 *
 * \param  fstyle       style for this text
 * \param  string       UTF-8 string to measure
 * \param  length       length of string
 * \param  x            x coordinate to search for
 * \param  char_offset  updated to offset in string of actual_x, [0..length]
 * \param  actual_x     updated to x coordinate of character closest to x
 * \return  true on success, false on error and error reported
 */

static inline bool amiga_nsfont_position_in_string(const plot_font_style_t *fstyle,
		const char *string, size_t length,
		int x, size_t *char_offset, int *actual_x)
{
	uint16 *utf16 = NULL, *outf16 = NULL;
	uint16 *utf16next = NULL;
	struct OutlineFont *ofont, *ufont = NULL;
	int tx = 0;
	uint32 utf8_pos = 0;
	int utf16charlen;
	ULONG emwidth = (ULONG)NSA_FONT_EMWIDTH(fstyle->size);
	int32 tempx;

	if(utf8_to_enc(string,"UTF-16",length,(char **)&utf16) != NSERROR_OK) return false;
	outf16 = utf16;
	if(!(ofont = ami_open_outline_font(fstyle, 0))) return false;

	*char_offset = 0;
	*actual_x = 0;

	while (utf8_pos < length) {
		utf16charlen = amiga_nsfont_utf16_char_length(utf16);
		utf16next = &utf16[utf16charlen];

		tempx = ami_font_width_glyph(ofont, utf16, utf16next, emwidth);

		if (tempx == 0) {
			if (ufont == NULL)
				ufont = ami_open_outline_font(fstyle, utf16);

			if (ufont)
				tempx = ami_font_width_glyph(ufont, utf16,
						utf16next, emwidth);
		}

		tx += tempx;
		utf16 = utf16next;
		utf8_pos = utf8_next(string, length, utf8_pos);

		if(tx < x) {
			*actual_x = tx;
			*char_offset = utf8_pos;
		} else {
			if((x - *actual_x) > (tx - x)) {
				*actual_x = tx;
				*char_offset = utf8_pos;
			}
			free(outf16);
			return true;
		}
	}

	*actual_x = tx;
	*char_offset = length;

	free(outf16);
	return true;
}


/**
 * Find where to split a string to make it fit a width.
 *
 * \param  fstyle       style for this text
 * \param  string       UTF-8 string to measure
 * \param  length       length of string
 * \param  x            width available
 * \param  char_offset  updated to offset in string of actual_x, [1..length]
 * \param  actual_x     updated to x coordinate of character closest to x
 * \return  true on success, false on error and error reported
 *
 * On exit, char_offset indicates first character after split point.
 *
 * Note: char_offset of 0 should never be returned.
 *
 * Returns:
 * char_offset giving split point closest to x, where actual_x <= x
 * else
 * char_offset giving split point closest to x, where actual_x > x
 *
 * Returning char_offset == length means no split possible
 */

static inline bool amiga_nsfont_split(const plot_font_style_t *fstyle,
		const char *string, size_t length,
		int x, size_t *char_offset, int *actual_x)
{
	uint16 *utf16_str = NULL;
	const uint16 *utf16 = NULL;
	const uint16 *utf16next = NULL;
	struct OutlineFont *ofont, *ufont = NULL;
	int tx = 0;
	uint32 utf8_pos = 0;
	int32 tempx = 0;
	ULONG emwidth = (ULONG)NSA_FONT_EMWIDTH(fstyle->size);

	/* Get utf16 conversion of string for glyph measuring routines */
	if (utf8_to_enc(string, "UTF-16", length, (char **)&utf16_str) !=
			NSERROR_OK)
		return false;

	utf16 = utf16_str;
	if (!(ofont = ami_open_outline_font(fstyle, 0)))
		return false;

	*char_offset = 0;
	*actual_x = 0;

	if (*utf16 == 0xFEFF) utf16++;

	while (utf8_pos < length) {
		if ((*utf16 < 0xD800) || (0xDBFF < *utf16))
			utf16next = utf16 + 1;
		else
			utf16next = utf16 + 2;

		tempx = ami_font_width_glyph(ofont, utf16, utf16next, emwidth);

		if (tempx == 0) {
			if (ufont == NULL)
				ufont = ami_open_outline_font(fstyle, utf16);

			if (ufont)
				tempx = ami_font_width_glyph(ufont, utf16,
						utf16next, emwidth);
		}

		/* Check whether we have a space */
		if (*(string + utf8_pos) == ' ') {
			/* Got a space */
			*actual_x = tx;
			*char_offset = utf8_pos;
		}

		tx += tempx;
		if ((x < tx) && (*char_offset != 0)) {
			/* Reached available width, and a space was found;
			 * split there. */
			free(utf16_str);
			return true;
		}

		utf16 = utf16next;
		utf8_pos = utf8_next(string, length, utf8_pos);
	}

	free(utf16_str);

	/* No spaces to split at, or everything fits */
	assert(*char_offset == 0 || x >= tx);

	*char_offset = length;
	*actual_x = tx;
	return true;
}

/**
 * Search for a font in the list and load from disk if not present
 */
static struct ami_font_node *ami_font_open(const char *font, bool critical)
{
	struct ami_font_node *nodedata = NULL;

#ifdef __amigaos4__
	nodedata = (struct ami_font_node *)FindSkipNode(ami_font_list, (APTR)font);		
#else
	struct nsObject *node = (struct nsObject *)FindIName((struct List *)ami_font_list, font);
	if(node) nodedata = node->objstruct;
#endif

	if(nodedata) {
		GetSysTime(&nodedata->lastused);
		return nodedata;
	}

	LOG("Font cache miss: %s", font);

#ifdef __amigaos4__
	nodedata = (struct ami_font_node *)InsertSkipNode(ami_font_list, (APTR)font, sizeof(struct ami_font_node));
#else
	nodedata = AllocVecTagList(sizeof(struct ami_font_node), NULL);
#endif

	if(nodedata == NULL) {
		warn_user("NoMemory", "");
		return NULL;
	}

	nodedata->font = OpenOutlineFont(font, &ami_diskfontlib_list, OFF_OPEN);
	
	if(!nodedata->font)
	{
		LOG("Requested font not found: %s", font);
		if(critical == true) warn_user("CompError", font);
		FreeVec(nodedata);
		return NULL;
	}

	nodedata->bold = (char *)GetTagData(OT_BName, 0, nodedata->font->olf_OTagList);
	if(nodedata->bold)
		LOG("Bold font defined for %s is %s", font, nodedata->bold);
	else
		LOG("Warning: No designed bold font defined for %s", font);

	nodedata->italic = (char *)GetTagData(OT_IName, 0, nodedata->font->olf_OTagList);
	if(nodedata->italic)
		LOG("Italic font defined for %s is %s", font, nodedata->italic);
	else
		LOG("Warning: No designed italic font defined for %s", font);

	nodedata->bolditalic = (char *)GetTagData(OT_BIName, 0, nodedata->font->olf_OTagList);
	if(nodedata->bolditalic)
		LOG("Bold-italic font defined for %s is %s", font, nodedata->bolditalic);
	else
		LOG("Warning: No designed bold-italic font defined for %s", font);

	GetSysTime(&nodedata->lastused);

#ifndef __amigaos4__
	node = AddObject(ami_font_list, AMINS_FONT);
	if(node) {
		node->objstruct = nodedata;
		node->dtz_Node.ln_Name = strdup(font);
	}
#endif

	return nodedata;
}

/**
 * Open an outline font in the specified size and style
 *
 * \param fstyle font style structure
 * \param codepoint open a default font instead of the one specified by fstyle
 * \return outline font or NULL on error
 */
static struct OutlineFont *ami_open_outline_font(const plot_font_style_t *fstyle,
		const uint16 *codepoint)
{
	struct ami_font_node *node;
	struct ami_font_node *designed_node = NULL;
	struct OutlineFont *ofont;
	char *fontname;
	ULONG ysize;
	int tstyle = 0;
	plot_font_generic_family_t fontfamily;
	ULONG emboldenx = 0;
	ULONG emboldeny = 0;
	ULONG shearsin = 0;
	ULONG shearcos = (1 << 16);

	if(codepoint) fontfamily = NSA_UNICODE_FONT;
		else fontfamily = fstyle->family;

	switch(fontfamily)
	{
		case PLOT_FONT_FAMILY_SANS_SERIF:
			fontname = nsoption_charp(font_sans);
		break;
		case PLOT_FONT_FAMILY_SERIF:
			fontname = nsoption_charp(font_serif);
		break;
		case PLOT_FONT_FAMILY_MONOSPACE:
			fontname = nsoption_charp(font_mono);
		break;
		case PLOT_FONT_FAMILY_CURSIVE:
			fontname = nsoption_charp(font_cursive);
		break;
		case PLOT_FONT_FAMILY_FANTASY:
			fontname = nsoption_charp(font_fantasy);
		break;
		case NSA_UNICODE_FONT:
		default:
			if(__builtin_expect((amiga_nsfont_utf16_char_length(codepoint) == 2), 0)) {
				/* Multi-byte character */
				fontname = nsoption_charp(font_surrogate);
			} else {
				fontname = (char *)ami_font_scan_lookup(codepoint, glypharray);
			}
			if(fontname == NULL) return NULL;
		break;
	}

	node = ami_font_open(fontname, true);
	if(!node) return NULL;

	if (fstyle->flags & FONTF_OBLIQUE)
		tstyle = NSA_OBLIQUE;

	if (fstyle->flags & FONTF_ITALIC)
		tstyle = NSA_ITALIC;

	if (fstyle->weight >= 700)
		tstyle += NSA_BOLD;

	switch(tstyle)
	{
		case NSA_ITALIC:
			if(node->italic) designed_node = ami_font_open(node->italic, false);

			if(designed_node == NULL) {
				shearsin = NSA_VALUE_SHEARSIN;
				shearcos = NSA_VALUE_SHEARCOS;
			}
		break;

		case NSA_OBLIQUE:
			shearsin = NSA_VALUE_SHEARSIN;
			shearcos = NSA_VALUE_SHEARCOS;
		break;

		case NSA_BOLD:
			if(node->bold) designed_node = ami_font_open(node->bold, false);

			if(designed_node == NULL) {
				emboldenx = NSA_VALUE_BOLDX;
				emboldeny = NSA_VALUE_BOLDY;
			}
		break;

		case NSA_BOLDOBLIQUE:
			shearsin = NSA_VALUE_SHEARSIN;
			shearcos = NSA_VALUE_SHEARCOS;

			if(node->bold) designed_node = ami_font_open(node->bold, false);

			if(designed_node == NULL) {
				emboldenx = NSA_VALUE_BOLDX;
				emboldeny = NSA_VALUE_BOLDY;
			}
		break;

		case NSA_BOLDITALIC:
			if(node->bolditalic) designed_node = ami_font_open(node->bolditalic, false);

			if(designed_node == NULL) {
				emboldenx = NSA_VALUE_BOLDX;
				emboldeny = NSA_VALUE_BOLDY;
				shearsin = NSA_VALUE_SHEARSIN;
				shearcos = NSA_VALUE_SHEARCOS;
			}
		break;
	}

	/* Scale to 16.16 fixed point */
	ysize = fstyle->size * ((1 << 16) / FONT_SIZE_SCALE);

	if(designed_node == NULL) {
		ofont = node->font;
	} else {
		ofont = designed_node->font;
	}

#ifndef __amigaos4__
	struct BulletBase *BulletBase = ofont->BulletBase;
#endif

	if(ESetInfo(AMI_OFONT_ENGINE,
			OT_DeviceDPI,   ami_devicedpi,
			OT_PointHeight, ysize,
			OT_EmboldenX,   emboldenx,
			OT_EmboldenY,   emboldeny,
			OT_ShearSin,    shearsin,
			OT_ShearCos,    shearcos,
			TAG_END) == OTERR_Success)
		return ofont;

	return NULL;
}

static inline int32 ami_font_plot_glyph(struct OutlineFont *ofont, struct RastPort *rp,
		uint16 *char1, uint16 *char2, uint32 x, uint32 y, uint32 emwidth, bool aa)
{
	struct GlyphMap *glyph;
	UBYTE *glyphbm;
	int32 char_advance = 0;
	FIXED kern = 0;
	ULONG glyphmaptag;
	ULONG template_type;
	uint32 long_char_1 = 0, long_char_2 = 0;
#ifndef __amigaos4__
	struct BulletBase *BulletBase = ofont->BulletBase;
#endif

#ifndef __amigaos4__
	if (__builtin_expect(((*char1 >= 0xD800) && (*char1 <= 0xDBFF)), 0)) {
		/* We don't support UTF-16 surrogates yet, so just return. */
		return 0;
	}
	
	if (__builtin_expect(((*char2 >= 0xD800) && (*char2 <= 0xDBFF)), 0)) {
		/* Don't attempt to kern a UTF-16 surrogate */
		*char2 = 0;
	}
#endif

#ifdef __amigaos4__
	if(__builtin_expect(aa == true, 1)) {
		glyphmaptag = OT_GlyphMap8Bit;
		template_type = BLITT_ALPHATEMPLATE;
	} else {
#endif
		glyphmaptag = OT_GlyphMap;
#ifdef __amigaos4__
		template_type = BLITT_TEMPLATE;
	}
#endif
 
	long_char_1 = amiga_nsfont_decode_surrogate(char1);
	long_char_2 = amiga_nsfont_decode_surrogate(char2);
	/**\todo use OT_GlyphCode_32 so we get an error for old font engines */

	if(ESetInfo(AMI_OFONT_ENGINE,
			OT_GlyphCode, long_char_1,
			OT_GlyphCode2, long_char_2,
			TAG_END) == OTERR_Success)
	{
		if(EObtainInfo(AMI_OFONT_ENGINE,
			glyphmaptag, &glyph,
			TAG_END) == 0)
		{
			glyphbm = glyph->glm_BitMap;
			if(!glyphbm) return 0;

			if(rp) {
#ifdef __amigaos4__
				BltBitMapTags(BLITA_SrcX, glyph->glm_BlackLeft,
					BLITA_SrcY, glyph->glm_BlackTop,
					BLITA_DestX, x - glyph->glm_X0 + glyph->glm_BlackLeft,
					BLITA_DestY, y - glyph->glm_Y0 + glyph->glm_BlackTop,
					BLITA_Width, glyph->glm_BlackWidth,
					BLITA_Height, glyph->glm_BlackHeight,
					BLITA_Source, glyphbm,
					BLITA_SrcType, template_type,
					BLITA_Dest, rp,
					BLITA_DestType, BLITT_RASTPORT,
					BLITA_SrcBytesPerRow, glyph->glm_BMModulo,
					TAG_DONE);
#else
				/* On OS3 the glyph needs to be in chip RAM */
				void *chip_glyph = AllocVec(glyph->glm_BMModulo * glyph->glm_BMRows, MEMF_CHIP);
				if(chip_glyph != NULL) {
					CopyMem(glyphbm, chip_glyph, glyph->glm_BMModulo * glyph->glm_BMRows);

					BltTemplate(chip_glyph + (glyph->glm_BMModulo * glyph->glm_BlackTop) +
						((glyph->glm_BlackLeft >> 4) << 1),
						glyph->glm_BlackLeft & 0xF, glyph->glm_BMModulo, rp,
						x - glyph->glm_X0 + glyph->glm_BlackLeft,
						y - glyph->glm_Y0 + glyph->glm_BlackTop,
						glyph->glm_BlackWidth, glyph->glm_BlackHeight);

					FreeVec(chip_glyph);
				}
#endif
			}

			kern = 0;

			if(*char2) EObtainInfo(AMI_OFONT_ENGINE,
								OT_TextKernPair, &kern,
								TAG_END);

			char_advance = (ULONG)(((glyph->glm_Width - kern) * emwidth) / 65536);

			EReleaseInfo(AMI_OFONT_ENGINE,
				glyphmaptag, glyph,
				TAG_END);
				
			if(*char2) EReleaseInfo(AMI_OFONT_ENGINE,
				OT_TextKernPair, kern,
				TAG_END);
		}
	}

	return char_advance;
}

static inline int32 ami_font_width_glyph(struct OutlineFont *ofont, 
		const uint16 *char1, const uint16 *char2, uint32 emwidth)
{
	int32 char_advance = 0;
	FIXED kern = 0;
	struct MinList *gwlist = NULL;
	FIXED char1w = 0;
	struct GlyphWidthEntry *gwnode;
	bool skip_c2 = false;
	uint32 long_char_1 = 0;
	uint32 long_char_2;
#ifndef __amigaos4__
	struct BulletBase *BulletBase = ofont->BulletBase;
#endif

#ifndef __amigaos4__
	if (__builtin_expect(((*char1 >= 0xD800) && (*char1 <= 0xDBFF)), 0)) {
		/* We don't support UTF-16 surrogates yet, so just return. */
		return 0;
	}
	
	if (__builtin_expect(((*char2 >= 0xD800) && (*char2 <= 0xDBFF)), 0)) {
		/* Don't attempt to kern a UTF-16 surrogate */
		skip_c2 = true;
	}
#endif

	if (*char2 < 0x0020) skip_c2 = true;

	long_char_1 = amiga_nsfont_decode_surrogate(char1);
	/**\todo use OT_GlyphCode_32 so we get an error for old font engines */

	if(ESetInfo(AMI_OFONT_ENGINE,
			OT_GlyphCode, long_char_1,
			OT_GlyphCode2, long_char_1,
			TAG_END) == OTERR_Success)
	{
		if(EObtainInfo(AMI_OFONT_ENGINE,
			OT_WidthList, &gwlist,
			TAG_END) == 0)
		{
			gwnode = (struct GlyphWidthEntry *)GetHead((struct List *)gwlist);
			if(gwnode) char1w = gwnode->gwe_Width;

			kern = 0;

			if(!skip_c2) {
				long_char_2 = amiga_nsfont_decode_surrogate(char2);
				if(ESetInfo(AMI_OFONT_ENGINE,
						OT_GlyphCode, long_char_1,
						OT_GlyphCode2, long_char_2,
						TAG_END) == OTERR_Success)
				{
					EObtainInfo(AMI_OFONT_ENGINE,
								OT_TextKernPair, &kern,
								TAG_END);
				}
			}
			char_advance = (ULONG)(((char1w - kern) * emwidth) / 65536);
			
			if(!skip_c2) EReleaseInfo(AMI_OFONT_ENGINE,
				OT_TextKernPair, kern,
				TAG_END);
				
			EReleaseInfo(AMI_OFONT_ENGINE,
				OT_WidthList, gwlist,
				TAG_END);
		}
	}

	return char_advance;
}

static const uint16 *ami_font_translate_smallcaps(uint16 *utf16char)
{
	const uint16 *p;
	p = &sc_table[0];

	while (*p != 0)
	{
		if(*p == *utf16char) return &p[1];
		p++;
	}

	return utf16char;
}

ULONG ami_font_unicode_text(struct RastPort *rp, const char *string, ULONG length,
			const plot_font_style_t *fstyle, ULONG dx, ULONG dy, bool aa)
{
	uint16 *utf16 = NULL, *outf16 = NULL;
	uint16 *utf16charsc = 0, *utf16nextsc = 0;
	uint16 *utf16next = 0;
	int utf16charlen;
	struct OutlineFont *ofont, *ufont = NULL;
	uint32 x=0;
	int32 tempx = 0;
	ULONG emwidth = (ULONG)NSA_FONT_EMWIDTH(fstyle->size);

	if(!string || string[0]=='\0') return 0;
	if(!length) return 0;
	if(rp == NULL) return 0;

	if(__builtin_expect(nsoption_bool(use_diskfont) == true, 0)) {
		return ami_font_bm_text(rp, string, length, fstyle, dx, dy);
	}

	if(utf8_to_enc(string,"UTF-16",length,(char **)&utf16) != NSERROR_OK) return 0;
	outf16 = utf16;
	if(!(ofont = ami_open_outline_font(fstyle, 0))) return 0;

	while(*utf16 != 0)
	{
		utf16charlen = amiga_nsfont_utf16_char_length(utf16);
		utf16next = &utf16[utf16charlen];

		if(fstyle->flags & FONTF_SMALLCAPS)
		{
			utf16charsc = (uint16 *)ami_font_translate_smallcaps(utf16);
			utf16nextsc = (uint16 *)ami_font_translate_smallcaps(utf16next);

			tempx = ami_font_plot_glyph(ofont, rp, utf16charsc, utf16nextsc,
								dx + x, dy, emwidth, aa);
		}
		else tempx = 0;

		if(tempx == 0) {
			tempx = ami_font_plot_glyph(ofont, rp, utf16, utf16next,
								dx + x, dy, emwidth, aa);
		}

		if(tempx == 0)
		{
			if(ufont == NULL)
			{
				ufont = ami_open_outline_font(fstyle, utf16);
			}

			if(ufont) {
				tempx = ami_font_plot_glyph(ufont, rp, utf16, utf16next,
											dx + x, dy, emwidth, aa);
			}
		}

		x += tempx;

		utf16 += utf16charlen;
	}

	free(outf16);
	return x;
}

static inline ULONG ami_font_unicode_width(const char *string, ULONG length,
			const plot_font_style_t *fstyle, ULONG dx, ULONG dy, bool aa)
{
	uint16 *utf16 = NULL, *outf16 = NULL;
	uint16 *utf16charsc = 0, *utf16nextsc = 0;
	uint16 *utf16next = 0;
	int utf16charlen;
	struct OutlineFont *ofont, *ufont = NULL;
	uint32 x=0;
	int32 tempx = 0;
	ULONG emwidth = (ULONG)NSA_FONT_EMWIDTH(fstyle->size);

	if(!string || string[0]=='\0') return 0;
	if(!length) return 0;

	if(utf8_to_enc(string,"UTF-16",length,(char **)&utf16) != NSERROR_OK) return 0;
	outf16 = utf16;
	if(!(ofont = ami_open_outline_font(fstyle, 0))) return 0;

	while(*utf16 != 0)
	{
		utf16charlen = amiga_nsfont_utf16_char_length(utf16);
		utf16next = &utf16[utf16charlen];

		if(fstyle->flags & FONTF_SMALLCAPS)
		{
			utf16charsc = (uint16 *)ami_font_translate_smallcaps(utf16);
			utf16nextsc = (uint16 *)ami_font_translate_smallcaps(utf16next);

			tempx = ami_font_width_glyph(ofont, utf16charsc, utf16nextsc, emwidth);
		}
		else tempx = 0;

		if(tempx == 0) {
			tempx = ami_font_width_glyph(ofont, utf16, utf16next, emwidth);
		}

		if(tempx == 0)
		{
			if(ufont == NULL)
			{
				ufont = ami_open_outline_font(fstyle, utf16);
			}

			if(ufont)
			{
				tempx = ami_font_width_glyph(ufont, utf16, utf16next, emwidth);
			}
		}

		x += tempx;

		utf16 += utf16charlen;
	}

	free(outf16);
	return x;
}

void ami_font_initscanner(bool force, bool save)
{
	ami_font_scan_init(nsoption_charp(font_unicode_file), force, save, glypharray);
}

void ami_font_finiscanner(void)
{
	ami_font_scan_fini(glypharray);
}

void ami_font_savescanner(void)
{
	ami_font_scan_save(nsoption_charp(font_unicode_file), glypharray);
}

#ifdef __amigaos4__
static LONG ami_font_cache_sort(struct Hook *hook, APTR key1, APTR key2)
{
	return stricmp(key1, key2);
}
#endif

#ifdef __amigaos4__
static void ami_font_cleanup(struct SkipList *skiplist)
{
	struct ami_font_node *node;
	struct ami_font_node *nnode;
	struct TimeVal curtime;

	node = (struct ami_font_node *)GetFirstSkipNode(skiplist);
	if(node == NULL) return;

	do {
		nnode = (struct ami_font_node *)GetNextSkipNode(skiplist, (struct SkipNode *)node);
		GetSysTime(&curtime);
		SubTime(&curtime, &node->lastused);
		if(curtime.Seconds > 300)
		{
			LOG("Freeing %s not used for %ld seconds", node->skip_node.sn_Key, curtime.Seconds);
			ami_font_close(node);
			RemoveSkipNode(skiplist, node->skip_node.sn_Key);
		}
	} while((node = nnode));

	/* reschedule to run in five minutes */
	ami_schedule(300000, (void *)ami_font_cleanup, ami_font_list);
}
#else
static void ami_font_cleanup(struct MinList *ami_font_list)
{
	struct nsObject *node;
	struct nsObject *nnode;
	struct ami_font_node *fnode;
	struct TimeVal curtime;

	if(IsMinListEmpty(ami_font_list)) return;

	node = (struct nsObject *)GetHead((struct List *)ami_font_list);

	do
	{
		nnode=(struct nsObject *)GetSucc((struct Node *)node);
		fnode = node->objstruct;
		GetSysTime(&curtime);
		SubTime(&curtime, &fnode->lastused);
		if(curtime.Seconds > 300)
		{
			LOG("Freeing %s not used for %ld seconds", node->dtz_Node.ln_Name, curtime.Seconds);
			DelObject(node);
		}
	} while((node=nnode));

	/* reschedule to run in five minutes */
	ami_schedule(300000, (void *)ami_font_cleanup, ami_font_list);
}
#endif

void ami_init_fonts(void)
{
	/* Initialise Unicode font scanner */
	ami_font_initscanner(false, true);

	/* Initialise font caching etc lists */
#ifdef __amigaos4__
	ami_font_cache_hook.h_Entry = (HOOKFUNC)ami_font_cache_sort;
	ami_font_cache_hook.h_Data = 0;
	ami_font_list = CreateSkipList(&ami_font_cache_hook, 8);
#else
	ami_font_list = NewObjList();
#endif

	NewList(&ami_diskfontlib_list);

	/* run first cleanup in ten minutes */
	ami_schedule(600000, (void *)ami_font_cleanup, ami_font_list);
}

#ifdef __amigaos4__
static void ami_font_del_skiplist(struct SkipList *skiplist)
{
	struct SkipNode *node;
	struct SkipNode *nnode;

	node = GetFirstSkipNode(skiplist);
	if(node == NULL) return;

	do {
		nnode = GetNextSkipNode(skiplist, node);
		ami_font_close((struct ami_font_node *)node);
		
	} while((node = nnode));

	DeleteSkipList(skiplist);
}
#endif

void ami_close_fonts(void)
{
	LOG("Cleaning up font cache");
	ami_schedule(-1, (void *)ami_font_cleanup, ami_font_list);
#ifdef __amigaos4__
	ami_font_del_skiplist(ami_font_list);
#else
	FreeObjList(ami_font_list);
#endif
	ami_font_list = NULL;
	ami_font_finiscanner();
}

void ami_font_close(struct ami_font_node *node)
{
	/* Called from FreeObjList if node type is AMINS_FONT */

	CloseOutlineFont(node->font, &ami_diskfontlib_list);
}

void ami_font_setdevicedpi(int id)
{
	DisplayInfoHandle dih;
	struct DisplayInfo dinfo;
	ULONG ydpi = nsoption_int(screen_ydpi);
	ULONG xdpi = nsoption_int(screen_ydpi);

	if(nsoption_bool(use_diskfont) == true) {
		LOG("WARNING: Using diskfont.library for text. Forcing DPI to 72.");
		nsoption_int(screen_ydpi) = 72;
	}

	browser_set_dpi(nsoption_int(screen_ydpi));

	if(id && (nsoption_int(monitor_aspect_x) != 0) && (nsoption_int(monitor_aspect_y) != 0))
	{
		if((dih = FindDisplayInfo(id)))
		{
			if(GetDisplayInfoData(dih, &dinfo, sizeof(struct DisplayInfo),
				DTAG_DISP, 0))
			{
				int xres = dinfo.Resolution.x;
				int yres = dinfo.Resolution.y;

				if((nsoption_int(monitor_aspect_x) != 4) || (nsoption_int(monitor_aspect_y) != 3))
				{
					/* AmigaOS sees 4:3 modes as square in the DisplayInfo database,
					 * so we correct other modes to "4:3 equiv" here. */
					xres = (xres * nsoption_int(monitor_aspect_x)) / 4;
					yres = (yres * nsoption_int(monitor_aspect_y)) / 3;
				}

				xdpi = (yres * ydpi) / xres;

				LOG("XDPI = %ld, YDPI = %ld (DisplayInfo resolution %d x %d, corrected %d x %d)", xdpi, ydpi, dinfo.Resolution.x, dinfo.Resolution.y, xres, yres);
			}
		}
	}

	ami_xdpi = xdpi;
	ami_devicedpi = (xdpi << 16) | ydpi;
}

/* The below are simple font routines which should not be used for page rendering */

struct TextFont *ami_font_open_disk_font(struct TextAttr *tattr)
{
	struct TextFont *tfont = OpenDiskFont(tattr);
	return tfont;
}

void ami_font_close_disk_font(struct TextFont *tfont)
{
	CloseFont(tfont);
}


/* Stub entry points */
static bool nsfont_width(const plot_font_style_t *fstyle,
		const char *string, size_t length,
		int *width)
{
	if(__builtin_expect(nsoption_bool(use_diskfont) == false, 1)) {
		return amiga_nsfont_width(fstyle, string, length, width);
	} else {
		return amiga_bm_nsfont_width(fstyle, string, length, width);
	}
}

static bool nsfont_position_in_string(const plot_font_style_t *fstyle,
		const char *string, size_t length,
		int x, size_t *char_offset, int *actual_x)
{
	if(__builtin_expect(nsoption_bool(use_diskfont) == false, 1)) {
		return amiga_nsfont_position_in_string(fstyle, string, length, x,
				char_offset, actual_x);
	} else {
		return amiga_bm_nsfont_position_in_string(fstyle, string, length, x,
				char_offset, actual_x);
	}
}

static bool nsfont_split(const plot_font_style_t *fstyle,
		const char *string, size_t length,
		int x, size_t *char_offset, int *actual_x)
{
	if(__builtin_expect(nsoption_bool(use_diskfont) == false, 1)) {
		return amiga_nsfont_split(fstyle, string, length, x, char_offset, actual_x);
	} else {
		return amiga_bm_nsfont_split(fstyle, string, length, x, char_offset, actual_x);
	}
}

const struct font_functions nsfont = {
	nsfont_width,
	nsfont_position_in_string,
	nsfont_split
};

