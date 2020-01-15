/*
 *  Copyright (C) 2010-2012 Nicolas Bourdaud <nicolas.bourdaud@gmail.com>
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

#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "filter-funcs.h"
#include "filter-internal.h"
#include "probesimd.h"
#include "rtfilter.h"


/**************************************************************************
 *                     Complex double precision version                   *
 *                            ( complex double )                          *
 **************************************************************************/
#define TYPEIN cdouble
#define TYPEOUT cdouble
#define add_dat(d1, d2)                  ((d1)+(d2))
#define mul_in_dat(d1, d2, part)          ((d1)*(d2))
#define mul_dat(d1, d2)                  ((d1)*(d2))
#define zero_dat()                      (0)
#define set1_dat(data)                  (data)
#define TYPEIN_LOCAL TYPEIN
#define TYPEOUT_LOCAL TYPEOUT
#define FILTER_DATADEP_FUNC filter_cd_noop
#define INIT_DATADEP_FUNC init_filter_cd
#include "filter-func-template.c"
#include "init-filter-func-template.c"

static HOTSPOT
int filtfunc(hfilter filt, const void* x, void* y, int ns)
{
#if SUPPORT_SSE3_SET
	if (  (filt->dispatch_code == 1)
	   && !(((uintptr_t)x) % (2*sizeof(double)))
	   && !(((uintptr_t)y) % (2*sizeof(double))) )
		filter_cd_sse3(filt, x, y, ns);
	else
#endif //SUPPORT_SSE3_SET
	filter_cd_noop(filt, x, y, ns);
	return ns;
}


LOCAL_FN
void set_filterfn_cd(struct rtf_filter* filt)
{
	filt->filter_fn = filtfunc;
	filt->dispatch_code = cputest_sse3() ? 1 : 0;
}

