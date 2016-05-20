/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2005 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2005 John M Bell <jmb202@ecs.soton.ac.uk>
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
 * \brief Implementation of URL parsing and joining operations.
 */

#include <ctype.h>
#include <string.h>
#include "kolibrios/kolibri_http.h"

#include "utils/config.h"
#include "utils/log.h"
#include "utils/utils.h"
#include "utils/url.h"


/* exported interface documented in utils/url.h */
nserror url_unescape(const char *str, char **result)
{
	char *curlstr;
	char *retstr;

	curlstr = http_unescape_asm(str, strlen(str));
	if (curlstr == NULL) {
		return NSERROR_NOMEM;
	}

	retstr = strdup(curlstr);

	/* free crashes for favicon.png.. */

	int val;
	__asm__ __volatile__(
			     "int $0x40"
			     :"=a"(val)
			     :"a"(68),"b"(13),"c"(curlstr));

	if(val != 1) {
	  debug_board_write_str("Failed to free block with SF 68,13\n");
	  __asm__ __volatile__("int3");
	}
	
	/* free(curlstr); */
	/* FIXME: Investigate why free crashes for only that string and enable it
	   Otherwise we might run out of mem completely when we run for a long time */

	if (retstr == NULL) {
		return NSERROR_NOMEM;
	}

	*result = retstr;
	return NSERROR_OK;
}


/* exported interface documented in utils/url.h */
nserror url_escape(const char *unescaped, size_t toskip,
		bool sptoplus, const char *escexceptions, char **result)
{
	size_t len;
	char *escaped, *d, *tmpres;
	const char *c;

	if (!unescaped || !result)
		return NSERROR_NOT_FOUND;

	*result = NULL;

	len = strlen(unescaped);
	if (len < toskip)
		return NSERROR_NOT_FOUND;
	len -= toskip;

	escaped = malloc(len * 3 + 1);
	if (!escaped)
		return NSERROR_NOMEM;

	for (c = unescaped + toskip, d = escaped; *c; c++) {
		/* Check if we should escape this byte.
		 * '~' is unreserved and should not be percent encoded, if
		 * you believe the spec; however, leaving it unescaped
		 * breaks a bunch of websites, so we escape it anyway. */
		if (!isascii(*c)
			|| (strchr(":/?#[]@" /* gen-delims */
				  "!$&'()*+,;=" /* sub-delims */
				  "<>%\"{}|\\^`~" /* others */,	*c)
				&& (!escexceptions || !strchr(escexceptions, *c)))
			|| *c <= 0x20 || *c == 0x7f) {
			if (*c == 0x20 && sptoplus) {
				*d++ = '+';
			} else {
				*d++ = '%';
				*d++ = "0123456789ABCDEF"[((*c >> 4) & 0xf)];
				*d++ = "0123456789ABCDEF"[(*c & 0xf)];
			}
		} else {
			/* unreserved characters: [a-zA-Z0-9-._] */
			*d++ = *c;
		}
	}
	*d++ = '\0';

	tmpres = malloc(d - escaped + toskip);
	if (!tmpres) {
		free(escaped);
		return NSERROR_NOMEM;
	}

	memcpy(tmpres, unescaped, toskip); 
	memcpy(tmpres + toskip, escaped, d - escaped);
	*result = tmpres;

	free(escaped);

	return NSERROR_OK;
}
