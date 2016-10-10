/*
    Copyright (C) 2010-2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This file is part of the rtfilter library

    The rtfilter library is free software: you can redistribute it and/or
    modify it under the terms of the version 3 of the GNU Lesser General
    Public License as published by the Free Software Foundation.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <stdint.h>
#include "rtfilter.h"
#include "filter-internal.h"
#include "filter-funcs.h"
#include "probesimd.h"
#include "complex-arithmetic.h"


/**************************************************************************
 *          complex output real input double precision version            *
 *                    ( complex double out double in)                     *
 **************************************************************************/
#define TYPEIN				double
#define TYPEOUT				complex_double_t
#define add_dat(d1,d2)			cadd_d(d1, d2)
#define mul_in_dat(d1,d2,part)		cscale_d(d1, d2)
#define mul_dat(d1,d2)			cmul_d(d1, d2)
#define zero_dat()			(complex_double_t){0.0}
#define set1_dat(data)			(data)
#define TYPEIN_LOCAL			TYPEIN
#define TYPEOUT_LOCAL			TYPEOUT
#define FILTER_DATADEP_FUNC		filter_dcd_noop
#include "filter-func-template.c"

static HOTSPOT
unsigned int filtfunc(hfilter filt, const void* x, void* y, unsigned int ns)
{
#if SUPPORT_SSE3_SET
	if ( (filt->dispatch_code == 1)
	  && !(((uintptr_t)x) % (2*sizeof(double)))
	  && !(((uintptr_t)y) % (2*sizeof(double))) )
		filter_dcd_sse3(filt, x, y, ns);
	else 
#endif //SUPPORT_SSE3_SET
	filter_dcd_noop(filt, x, y, ns);
	return ns;
}


LOCAL_FN
void set_filterfn_dcd(struct rtf_filter* filt)
{
	filt->filter_fn = filtfunc;

#if SUPPORT_SSE3_SET
	// Check that sample can be aligned on 16 byte boundaries
	if (cputest_sse3() && !(filt->num_chann%4))
		filt->dispatch_code = 1;
#endif //SUPPORT_SSE3_SET
}

