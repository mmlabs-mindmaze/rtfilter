/*
 *  Copyright (C) 2009-2011 Nicolas Bourdaud <nicolas.bourdaud@gmail.com>
 *
 *  This file is part of the rtfilter library
 *
 *  The rtfilter library is free software: you can redistribute it and/or
 *  modify it under the terms of the version 3 of the GNU Lesser General
 *  Public License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "filter-funcs.h"
#include "filter-internal.h"
#include "probesimd.h"
#include "rtfilter.h"


/**************************************************************************
 *                     Real single precision version                      *
 *                               ( float )                                *
 **************************************************************************/
#define TYPEIN float
#define TYPEOUT float
#define add_dat(d1, d2)                  ((d1)+(d2))
#define mul_in_dat(d1, d2, part)          ((d1)*(d2))
#define mul_dat(d1, d2)                  ((d1)*(d2))
#define zero_dat()                      (0)
#define set1_dat(data)                  (data)
#define TYPEIN_LOCAL TYPEIN
#define TYPEOUT_LOCAL TYPEOUT
#define FILTER_DATADEP_FUNC filter_f_noop
#define INIT_DATADEP_FUNC init_filter_f
#include "filter-func-template.c"
#include "init-filter-func-template.c"


static HOTSPOT
int filtfunc(hfilter filt, const void* x, void* y, int ns)
{
#if SUPPORT_SSE_SET
	if (  (filt->dispatch_code == 1)
	   && !(((uintptr_t)x) % (4*sizeof(float)))
	   && !(((uintptr_t)y) % (4*sizeof(float))) )
		filter_f_sse(filt, x, y, ns);
	else
#endif //SUPPORT_SSE_SET
	filter_f_noop(filt, x, y, ns);
	return ns;
}


LOCAL_FN
void set_filterfn_f(struct rtf_filter* filt)
{
	filt->filter_fn = filtfunc;

#if SUPPORT_SSE_SET
	// Check that sample can be aligned on 16 byte boundaries
	if (cputest_sse() && !(filt->num_chann%4))
		filt->dispatch_code = 1;

#endif //SUPPORT_SSE_SET
}

