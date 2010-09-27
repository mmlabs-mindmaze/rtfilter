/*
      Copyright (C) 2010 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This file is part of the rtfilter library

    The rtfilter library is free software: you can redistribute it and/or
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
#include "rtfilter.h"
#include "filter-internal.h"


/**************************************************************************
 *                                                                        *
 *                     Complex Single precision version                   *
 *                            ( complex float )                           *
 *                                                                        *
 **************************************************************************/
#if 0
#ifdef __SSE3__
# include <pmmintrin.h>
# define USE_SIMD
#endif
#endif

typedef complex float cfloat;

#define add_vec(v1,v2)			_mm_add_ps(v1,v2)
#define mul_vec(v1,v2)			_mm_mul_ps(v1,v2)
#define mul_in_vec(v1,v2)		_mm_mul_ps(v1,v2)
#define zero_vec()			_mm_setzero_ps()
#define set1_vec(data)			_mm_set1_ps(data)
#define TYPEIN				float
#define TYPEIN_V			__m128
#define TYPEOUT				cfloat
#define TYPEOUT_V			__m128
#define FILTER_ALIGNED_FUNC		filter_fcfa
#define FILTER_UNALIGNED_FUNC		filter_fcfu
#define FILTER_FUNC			filter_fcf
#define DINTYPE				RTF_FLOAT
#define DOUTTYPE			RTF_CFLOAT



#include "templates.c"

