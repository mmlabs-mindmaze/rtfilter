/*
      Copyright (C) 2010 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This file is part of the rtfilter library

    The eegpanel library is free software: you can redistribute it and/or
    modify it under the terms of the version 3 of the GNU General Public
    License as published by the Free Software Foundation.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <memory.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include "filter.h"
#include "filter-internal.h"


/**************************************************************************
 *                                                                        *
 *                      Complex double precision version                  *
 *                            ( complex double )                          *
 *                                                                        *
 **************************************************************************/
#if 0
#ifdef __SSE3__
# include <pmmintrin.h>
# define USE_SIMD
#endif
#endif

typedef complex double cdouble;

#define add_vec(v1,v2)			_mm_add_pd(v1,v2)
#define mul_vec(v1,v2)			_mm_mul_pd(v1,v2)
#define mul_in_vec(v1,v2)		_mm_mul_pd(v1,v2)
#define zero_vec()			_mm_setzero_pd()
#define set1_vec(data)			_mm_set1_pd(data)
#define TYPEIN				double
#define TYPEIN_V			__m128d
#define TYPEOUT				cdouble
#define TYPEOUT_V			__m128d
#define FILTER_ALIGNED_FUNC		filter_dcda
#define FILTER_UNALIGNED_FUNC		filter_dcdu
#define FILTER_FUNC			filter_dcd
#define DINTYPE				RTF_DOUBLE
#define DOUTTYPE			RTF_CDOUBLE



#include "templates.c"

