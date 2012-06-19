/*
 * Copyright 2012 Vincent Sanders <vince@netsurf-browser.org>
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
 * Interface to javascript engine functions.
 */

#ifndef _NETSURF_JAVASCRIPT_JS_H_
#define _NETSURF_JAVASCRIPT_JS_H_

typedef struct jscontext jscontext;
typedef struct jsobject jsobject;

void js_initialise(void);
void js_finalise(void);

jscontext *js_newcontext(void);
void js_destroycontext(jscontext *ctx);

jsobject *js_newcompartment(jscontext *ctx, struct content* c);

bool js_exec(jscontext *ctx, const char *txt, int txtlen);

#endif /* _NETSURF_JAVASCRIPT_JS_H_ */