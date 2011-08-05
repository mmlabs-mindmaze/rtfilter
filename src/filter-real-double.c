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

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "rtfilter.h"
#include "filter-internal.h"


/**************************************************************************
 *                                                                        *
 *                     Real double precision version                      *
 *                               ( double )                               *
 *                                                                        *
 **************************************************************************/
#ifdef __SSE2__
# include <emmintrin.h>
# define USE_SIMD
#endif

#define add_vec(v1,v2)			_mm_add_pd(v1,v2)
#define mul_vec(v1,v2)			_mm_mul_pd(v1,v2)
#define mul_in_vec(v1,v2,part)		_mm_mul_pd(v1,v2)
#define zero_vec()			_mm_setzero_pd()
#define set1_vec(data)			_mm_set1_pd(data)
#define TYPEIN				double
#define TYPEIN_V			__m128d
#define TYPEOUT				double
#define TYPEOUT_V			__m128d
#define FILTER_ALIGNED_FUNC		filter_da
#define FILTER_UNALIGNED_FUNC		filter_du
#define FILTER_FUNC			filter_d
#define COPY_NUMDENUM			copy_numdenum_d
#define DINTYPE				RTF_DOUBLE
#define DOUTTYPE			RTF_DOUBLE

#include "templates.c"

